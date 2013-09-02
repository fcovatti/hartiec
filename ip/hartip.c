
// hartip.cpp : Defines the exported functions for the DLL application.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "hartip.h"

static int hip_get_asc_name(char *buf, size_t bufsz, const TCHAR *name);
static int hip_send_pkt(struct hip_sess *sess,
                        const struct sockaddr_in *toaddr,
                        hip_u8 msgid, const hip_u8 *req, size_t reqlen);
static int hip_recv_pkt(struct hip_sess *sess,
                        struct sockaddr_in *from_addr,
                        hip_u8 *status, hip_u8 *rsp, size_t *rsplen);
 
int hip_connect(struct hip_sess **sess_out, const TCHAR *name, hip_u16 port)
{
    int rv;
    struct hip_sess *sess;
    //WSADATA wsadata;
    char name_str[80];
    char port_str[8];
    struct addrinfo hints;
    struct addrinfo *ai;
    hip_u8 init_req[5];
    hip_u8 init_rsp[16];
    size_t init_rsp_len;
    hip_u8 status;

    sess = NULL;
    ai = NULL;

    sess = (struct hip_sess *)malloc(sizeof (*sess));
    if (!sess) {
        rv = HIP_OUT_OF_MEM;
        goto done;
    }

    memset(sess, 0, sizeof (*sess));
    sess->sock = INVALID_SOCKET;

/*
    rv = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (rv != 0) {
        rv = HIP_FAILED;
        goto done;
    }
    sess->wsastarted = TRUE;
*/

    rv = hip_get_asc_name(name_str, HIP_DIM(name_str), name);
    if (rv != HIP_OK) {
        goto done;
    }
    //StringCchPrintfA(port_str, HIP_DIM(port_str), "%u", port);
	snprintf(port_str, HIP_DIM(port_str), "%u", port);

    memset(&hints, 0, sizeof (hints));
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    rv = getaddrinfo(name_str, port_str, &hints, &ai);
    if (rv != 0) {
        rv = HIP_NAME_LOOKUP_ERR;
        goto done;
    }

    sess->sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sess->sock == INVALID_SOCKET) {
        rv = HIP_FAILED;
        goto done;
    }

    init_rsp_len = sizeof (init_rsp);
    rv = hip_exec_hip_req(sess, (struct sockaddr_in *)ai->ai_addr,
            HIP_MSGID_OPEN, init_req, sizeof (init_req),
            &sess->srvaddr, &status, init_rsp, &init_rsp_len);
    if (rv != HIP_OK) {
        goto done;
    }
    sess->sess_opened = TRUE;

    rv = HIP_OK;

done:
    if (ai) {
        freeaddrinfo(ai);
    }

    if (rv == HIP_OK) {
        *sess_out = sess;
    } else {
        hip_close(sess);
    }

	return rv;
}

static int
hip_get_asc_name(char *buf, size_t bufsz, const TCHAR *name)
{
    int rv;
    size_t i;
    const TCHAR *p;
    TCHAR ch;

    p = name;
    for (i = 0; i < (bufsz - 1); ++i) {
        ch = *(p++);
     //   if (ch == _T('\0')) {
	 
        if (ch == '\0') {
            break;
        }
        if ((ch < 0x20) || (ch >= 0x80)) {
            rv = HIP_FAILED;
            goto done;
        }
        buf[i] = (char)ch;
    }
    //if (ch != _T('\0')) {
    if (ch != '\0') {
        rv = HIP_FAILED;
        goto done;
    }
    buf[i] = '\0';
    rv = HIP_OK;

done:
    return rv;
}

int hip_close(struct hip_sess *sess)
{
    if (!sess) {
        goto done;
    }
    if (sess->sess_opened) {
        hip_exec_hip_req(sess, &sess->srvaddr, HIP_MSGID_CLOSE,
            NULL, 0, NULL, NULL, NULL, NULL);
    }
    if (sess->sock != INVALID_SOCKET) {
     //   closesocket(sess->sock);
	 close(sess->sock);
    }
   // if (sess->wsastarted) {
   //     WSACleanup();
   // }
    free(sess);

done:
	return HIP_OK;
}

int hip_enum_nodes(struct hip_sess *sess,hip_addr_t **ppbuf, size_t *pbuflen)
{
    static const hip_u8 req[] = {
        0x1f, 6, 0x3, 0x2e, 0x0, 16, 0, 0 //814
    };

    int rv;
    hip_u8 rspcode;
    hip_u8 rsp[256];
    size_t rsplen;
    hip_addr_t *outbuf;
    size_t i;
    size_t ndevs;
    hip_u8 *p;

    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, HIP_GATEWAY_ADDR, req, sizeof (req),&rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
    if ((rspcode != 0) && (rspcode != 8)) {
        rv = HIP_FAILED;
        goto done;
    }

    if (rsplen < 8) {
        rv = HIP_INVALID_RSP;
        goto done;
    }

    p = rsp + 2;
    rsplen -= 2;
    ndevs = p[1];
    p += 6;
    rsplen -= 6;

    if (ndevs == 0) {
        rv = HIP_OK;
        *ppbuf = NULL;
        *pbuflen = 0;
        goto done;
    }

    if ((ndevs * 5) > rsplen) {
        rv = HIP_INVALID_RSP;
        goto done;
    }

    outbuf = (hip_addr_t *)malloc(sizeof (hip_addr_t) * ndevs);
    if (!outbuf) {
        rv = HIP_OUT_OF_MEM;
        goto done;
    }

    for (i = 0; i < ndevs; ++i) {
        /*
         * The compiler doesn't seem to like the logical-or
         * operator on unsigned 64-bit values.  If you use that,
         * it seems to try to sign-extend the lower 32-bit
         * into the upper 32-bit and overwrite everything
         * that is already there.  This will happen even if you
         * carefully cast everything to unsigned quantities first.
         * Worked around using a simple unsigned addition.
         */
        hip_u32 tmp;
        outbuf[i] = ((hip_u64)p[0]) << 32; // aqui tem um cast
        tmp = (p[1] << 24) | (p[2] << 16) |
                        (p[3] << 8) | p[4];
        outbuf[i] += tmp;
        p += 5;
        rsplen -= 5;
    }

    rv = HIP_OK;
    *ppbuf = outbuf;
    *pbuflen = ndevs;

done:
    return rv;
}

int hip_free_node_list(struct hip_sess *sess, hip_addr_t *buf)
{
    if (buf) {
        free(buf);
    }
    return HIP_OK;
}

int hip_get_long_tag(struct hip_sess *sess, hip_addr_t addr,
                 TCHAR *buf, size_t buflen)
{
    static const hip_u8 req[] = { 20, 0 };
    int rv;
    hip_u8 rspcode;
    hip_u8 rsp[64];
    size_t rsplen;

    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK)
    {
        goto done;
    }
    if (rspcode != 0)
    {
        rv = HIP_FAILED;
        goto done;
    }
    if (rsplen < 32)
    {
        rv = HIP_INVALID_RSP;
        goto done;
    }
    rsp[32] = '\0';

    //rv = MultiByteToWideChar(28591, 0, (const char *)rsp, -1, buf, buflen);
	//rv=mbtowc(buf, (const char *)rsp, buflen);
    rv = (memcpy(buf, (const char *)rsp, buflen)!=NULL);

    if (rv ==  0)
    {
        rv = HIP_FAILED;
        goto done;
    }

    rv = HIP_OK;

done:
    return rv;
}


// Funcao para pegar o nickname
int hip_get_nickname(struct hip_sess *sess, hip_addr_t addr,
							unsigned int *_nickname)
{
    static const hip_u8 req[] = {
        0x1f, 2, 0x3, 0x0d
    };

    int rv;
    hip_u8 rspcode;
    hip_u8 rsp[64];
    size_t rsplen;
    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req), &rspcode, rsp, &rsplen);
    if (rv != HIP_OK)
    {
        goto done;
    }
    if (rspcode != 0)
    {
        rv = HIP_FAILED;
        goto done;
    }
    if (rsplen < 4)
    {
        rv = HIP_INVALID_RSP;
        goto done;
    }
	
//	printf ("\n Frame recebido{ ");
//	for (int a=0;a<rsplen;a++)
//	{
//		printf ("%d ",rsp[a]);
//	}
//	printf ("}\n");

	(*_nickname)=rsp[2]*256+rsp[3];

    rv = HIP_OK;

done:
    return rv;
}

// funçao para pegar cmd 840
int hip_get_network_statistics(struct hip_sess *sess, hip_addr_t nodeaddr,
										struct  hip_network_devices **list)
{
    static hip_u8 req[9] = {
        0x1f, 7, 0x3, 0x48
    };

    int rv;
    hip_u8 rsp[256];
    size_t rsplen;
    hip_u8 rspcode;
	struct hip_network_devices *tabela; // ver aqui ponteiro
    hip_u8 *p;

	req[4]= ((nodeaddr)&0xFF00000000)>>32;
	req[5]= ((nodeaddr)&0x00FF000000)>>24;
	req[6]= ((nodeaddr)&0x0000FF0000)>>16;
	req[7]= ((nodeaddr)&0x000000FF00)>>8;
	req[8]= ((nodeaddr)&0x00000000FF);

    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, HIP_GATEWAY_ADDR, req, sizeof (req),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
//	printf("\nFrame network devices stats retorno = %d\n",rv);
//	printf("\nFrame network devices stats (%d){",rsplen);
//	for (int aux=0; aux<rsplen; aux++)
//	{
//		printf("%d ",rsp[aux]);
//	}
//	printf("}\n");
	if ((rspcode != 0)) {
        rv = HIP_FAILED;
        goto done;
    }

    if (rsplen < 59) { 
        rv = HIP_INVALID_RSP;
        goto done;
    }

    p = rsp + 2; 
    rsplen -= 2;

	tabela = (struct hip_network_devices*)malloc(sizeof (struct hip_network_devices));
    if (!tabela) {
        rv = HIP_OUT_OF_MEM;
		free(tabela);
        goto done;
    }

	tabela->unique_id= (p[0]*4294967296L) | (((long long)p[1])<<24) | (p[2]<<16) | (p[3]<<8) | p[4];
	tabela->n_active_graphs=p[5]*256+p[6];
	tabela->n_active_frames=p[7]*256+p[8];
	tabela->n_active_links=p[9]*256+p[10];
	tabela->number_neighbors=p[11];
	tabela->avera_comm= (p[12]<<24) | (p[13]<<16) | (p[14]<<8) | p[15];
	tabela->n_joins=p[16]*256+p[17];
	
	tabela->data_join.dia = p[18];
	tabela->data_join.mes = p[19];
	tabela->data_join.ano = p[20] + 1900;
	tabela->time_ms= ((p[21]<<24) | (p[22]<<16) | (p[23]<<8) | p[24]);
	tabela->pack_generate= (p[25]<<24) | (p[26]<<16) | (p[27]<<8) | p[28];
	tabela->pack_terminated= (p[29]<<24) | (p[30]<<16) | (p[31]<<8) | p[32];
	tabela->n_mic_failure= (p[33]<<24) | (p[34]<<16) | (p[35]<<8) | p[36]; 
	tabela->n_session_fail= (p[37]<<24) | (p[38]<<16) | (p[39]<<8) | p[40];
	tabela->n_crc_erros= (p[41]<<24) | (p[42]<<16) | (p[43]<<8) | p[44];
	tabela->n_counterby= (p[45]<<24) | (p[46]<<16) | (p[47]<<8) | p[48];
	tabela->n_counterfrom= (p[49]<<24) | (p[50]<<16) | (p[51]<<8) | p[52];
	tabela->deviat_std= (p[53]<<24) | (p[54]<<16) | (p[55]<<8) | p[56]; 
	rv = HIP_OK;
    *list = tabela;

done:
    return rv;
}

// Funcao para pegar o networkID cmd 774 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
int hip_get_netid(struct hip_sess *sess, hip_addr_t addr,
							unsigned int *_netid)
{
    static const hip_u8 req[] = {
        0x1f, 2, 0x3, 0x06
    };

    int rv;
    hip_u8 rspcode; //definindo variavel rspcode como do tipo unsigned char
    hip_u8 rsp[64]; //definindo
    size_t rsplen; //definindo unsigned int

    rsplen = sizeof (rsp);  //recebe tamanho de rsp
    rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req), //rv recebe conteudo da funçao cmd Hart
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK)
    {
        goto done;
    }
    if (rspcode != 0)
    {
        rv = HIP_FAILED;
        goto done;
    }
    if (rsplen < 4)
    {
        rv = HIP_INVALID_RSP;
        goto done;
    }

	(*_netid)=rsp[2]*256+rsp[3];

    rv = HIP_OK;

done:
    return rv;
}


//fim comando 774 
//cmd 780
int hip_get_neighbor_health_list(struct hip_sess *sess, hip_addr_t addr,
			                            struct hip_node_vizinhos_linked **list)
{
    static hip_u8 req[] = {
        0x1f, 4, 0x3, 0x0c, 0, 99 //problema numero de devices fixo
    };

    int rv, i;
    hip_u8 rsp[256];
    size_t rsplen;
    hip_u8 rspcode;
    struct hip_node_vizinhos_linked_entry *linha;
	struct hip_node_vizinhos_linked *tabela;
	
	//    size_t i;

    size_t ndevs;
    hip_u8 *p;

    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
    if ((rspcode != 0) && (rspcode != 8)) {
        rv = HIP_FAILED;
        goto done;
    }

    if (rsplen < 15) {
        rv = HIP_INVALID_RSP;
        goto done;
    }

    p = rsp + 2;
    rsplen -= 2;

    ndevs = p[1];
    if (ndevs == 0) {
        rv = HIP_OK;
        list = NULL;
        goto done;
    }

    if ((ndevs*10+3) > rsplen) {
        rv = HIP_INVALID_RSP;
        goto done;
    }

    linha = (struct hip_node_vizinhos_linked_entry *)malloc(sizeof (struct hip_node_vizinhos_linked_entry)*ndevs);
    if (!linha) {
		free(linha);
        rv = HIP_OUT_OF_MEM;
        goto done;
    }

	tabela = (struct hip_node_vizinhos_linked *)malloc(sizeof (struct hip_node_vizinhos_linked));
    if (!tabela) {
        rv = HIP_OUT_OF_MEM;
		free(linha);
		free(tabela);
        goto done;
    }

	tabela->indice_vizinho=p[0];
	tabela->vizinhos_lidos=p[1];
	tabela->total_vizinhos=p[2];
	tabela->lista=linha;

	p=p+3;

    for (i = 0; i < ndevs; i++) 
	{
			linha[i].nickname=p[0]*256+p[1];		
			linha[i].flags=p[2];
			linha[i].RSL=p[3];
			linha[i].pacotes_okTX=p[4]*256+p[5];
			linha[i].pacotes_falhaTX=p[6]*256+p[7];
			linha[i].pacotes_okRX=(p[8] << 8)|p[9]; //p[8]*256+p[9]; //aqui corrigi! ok
			p=p+10;
    }

    rv = HIP_OK;
    *list = tabela;

done:
    return rv;
}


// cmd 782
int hip_get_session_list(struct hip_sess *sess, hip_addr_t addr,
										struct hip_session_list **list)
{
    static hip_u8 req[] = {
        0x1f, 4, 0x3, 0x0e, 0, 99 //problema numero de sessoes estah fixa
    };

	int i;
	//int aux;
	int nsessoes;
	int rv;
	hip_u8 *p;
    hip_u8 rsp[256];
    size_t rsplen;
    hip_u8 rspcode;
	struct hip_session_list_entry *linha;
	struct hip_session_list *tabela;

   rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
    if ((rspcode != 0) && (rspcode != 8)) {
        rv = HIP_FAILED;
        goto done;
    }

//	printf("\nFrame hip_get_session_list retorno = %d\n",rv);
//	printf("\nFrame hip_get_session_list (%d){",rsplen);
//	for (int aux=0; aux<rsplen; aux++)
//	{
//		printf("%d ",rsp[aux]);
//	}
//	printf("}\n");

    if (rsplen < 5) { 
        rv = HIP_INVALID_RSP;
        goto done;
    }

    p = rsp + 2;
    rsplen -= 2;

	nsessoes = p[1];
	// testando frame 
    if (nsessoes == 0) {			// se nao houver sessao
        rv = HIP_OK;
        list = NULL;
        goto done;
    }

    if ((nsessoes*16+3) > rsplen) { //repete 16 bytes por sessao + 3 bytes info geral
        rv = HIP_INVALID_RSP;
        goto done;
    }

    linha = (struct hip_session_list_entry *)malloc(sizeof (struct hip_session_list_entry)*nsessoes);
    if (!linha) {
		free(linha);
        rv = HIP_OUT_OF_MEM;
        goto done;
    }

	tabela = (struct hip_session_list *)malloc(sizeof (struct hip_session_list));
    if (!tabela) {
        rv = HIP_OUT_OF_MEM;
		free(linha);
		free(tabela);
        goto done;
    }

	tabela->indice_sessao=p[0];
	tabela->numero_sessoes=p[1];
	tabela->numero_sessoes_ativas=p[2];
	tabela->lista=linha;
	p=p+3;

    for (i = 0; i < nsessoes; i++) 
	{
		linha[i].tipo_sessao=p[0];
		linha[i].nickname=(hip_u16) ((p[1])<<8)|p[2];
		linha[i].endereco=(hip_u40) (p[3]*4294967296L) | (((long long)p[4])<<24) | (p[5])<<16 | (p[6])<<8 | p[7];
		linha[i].pdncv=(hip_u32) (p[8])<<24 | (p[9])<<16 | (p[10])<<8 | p[11];
		//TODO: fixed a mistake down here....it was <24....added a <
		linha[i].tdncv=(hip_u32) (p[12])<<24 | (p[13])<<16 | (p[14])<<8 | p[15];
		p=p+16;
    }

    rv = HIP_OK;
    *list = tabela;	
	
done:
    return rv;
}
							



//cmd 783

int hip_get_superframe_list(struct hip_sess *sess, hip_addr_t addr,
			                            struct hip_node_superframes **list)
{
    static hip_u8 req[] = {
        0x1f, 4, 0x3, 0x0f, 0, 99 //problema numero de devices fixo
    };

    int rv, i;
    hip_u8 rsp[256];
    size_t rsplen;
    hip_u8 rspcode;
    struct hip_node_superframes_entry *linha;
	struct hip_node_superframes *tabela;
	
	size_t ndevs;
    hip_u8 *p;

    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
    if ((rspcode != 0) && (rspcode != 8)) {
        rv = HIP_FAILED;
        goto done;
    }

//	printf("\nsuperFrame  list {");
//	for (int aux=0; aux<rsplen; aux++)
//	{
//		printf("%d ",rsp[aux]);
	//	printf("%d",len(rsp[aux]));
//	}
	//printf("}\n");

    if (rsplen < 8) { //número de bytes resposta + 2 bytes cmd
        rv = HIP_INVALID_RSP;
        goto done;
    }

    p = rsp + 2;
    rsplen -= 2;

	ndevs = p[1]; // number of entries to read
    if (ndevs == 0) {
        rv = HIP_OK;
        list = NULL;
        goto done;
    }

    if ((ndevs*4+3) > rsplen) { //repete 4 bytes por device + 3 bytes info geral
        rv = HIP_INVALID_RSP;
        goto done;
    }

    linha = (struct hip_node_superframes_entry *)malloc(sizeof (struct hip_node_superframes_entry)*ndevs);
    if (!linha) {
		free(linha);
        rv = HIP_OUT_OF_MEM;
        goto done;
    }

	tabela = (struct hip_node_superframes *)malloc(sizeof (struct hip_node_superframes));
    if (!tabela) {
        rv = HIP_OUT_OF_MEM;
		free(linha);
		free(tabela);
        goto done;
    }

	tabela->superframe_i=p[0];
	tabela->n_entries_r=p[1];
	tabela->n_active_superframe=p[2];
	tabela->lista=linha;

	p=p+3;

    for (i = 0; i < ndevs; i++) 
	{
			linha[i].superframeid=p[0];
			linha[i].n_slots=p[1]*256+p[2];
			linha[i].flag_superframe=p[3];
			p=p+4;
    }

    rv = HIP_OK;
    *list = tabela;

done:
    return rv;
}

//cmd 784
int hip_get_read_list(struct hip_sess *sess, hip_addr_t addr, struct hip_node_links **list, const hip_u8 *req)
{
//	static hip_u8 req[] = {0x1f, 5, 0x3, 0x10, 0, 0, 99}; //problema numero de devices fixo
//printf("\ncheguei no hartIP\n"); //JW - debug
	// printf("tamanho do req: %d \n",sizeof(req)); //JW -debug
//	for (int aux=0; aux<sizeof(req); aux++) //teste
//	{ //teste
//		printf("%d ",req[aux]); //teste
//	}				//teste
//	printf("}\n"); //teste
// coloquei isso pois quando a variavel req vem é acrescentado um valor (ver conversao??)
 static hip_u8 requ[7];
	requ[0] = req[0];
	requ[1] = req[1];
	requ[2] = req[2];
	requ[3] = req[3];
	requ[4] = req[4];
	requ[5] = req[5];
	requ[6] = req[6];
//


	int i, rv;
    hip_u8 rsp[256];
    size_t rsplen;
    hip_u8 rspcode;
    struct hip_node_links_entry *linha;
	struct hip_node_links *tabela;
	size_t ndevs;
    hip_u8 *p;
    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, requ, sizeof (requ),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
    if ((rspcode != 0) && (rspcode != 8)) {
        rv = HIP_FAILED;
        goto done;
    }

//	printf("\nFrame read link list {");
//	for (int aux=0; aux<rsplen; aux++)
//	{
//		printf("%d ",rsp[aux]);
//	}
//	printf("}\n");

    if (rsplen < 14) { 
        rv = HIP_INVALID_RSP;
        goto done;
    }

    p = rsp + 2;
    rsplen -= 2;

    ndevs = p[2];
    if (ndevs == 0) {
        rv = HIP_OK;
        list = NULL;
        goto done;
    }

    if ((ndevs*8+4) > rsplen) {
        rv = HIP_INVALID_RSP;
        goto done;
    }

    linha = (struct hip_node_links_entry *)malloc(sizeof (struct hip_node_links_entry)*ndevs);
    if (!linha) {
		free(linha);
        rv = HIP_OUT_OF_MEM;
        goto done;
    }

	tabela = (struct hip_node_links *)malloc(sizeof (struct hip_node_links));
    if (!tabela) {
        rv = HIP_OUT_OF_MEM;
		free(linha);
		free(tabela);
        goto done;
    }

	tabela->link_index=p[0]*256+p[1];
	tabela->links_read=p[2];
	tabela->links_ativos=p[3]*256+p[4];
	tabela->lista=linha;

	p=p+5;

    for (i = 0; i < ndevs; i++) 
	{
			linha[i].superframe_id=p[0];		
			linha[i].slot_number_frame=p[1]*256+p[2];
			linha[i].channelOff=p[3];
			linha[i].neighbor_link=p[4]*256+p[5];
			linha[i].link_opt=p[6];
			linha[i].link_type=p[7];
			p=p+8;
    }

    rv = HIP_OK;
    *list = tabela;

done:
    return rv;
}
// cmd 787
int hip_get_neighbor_discovered_list(struct hip_sess *sess, hip_addr_t addr,
			                            struct hip_node_vizinhos_discor **list) 
{
	int i;
    static hip_u8 req[] = {
        0x1f, 4, 0x3, 0x13, 0, 99  //VER ISSO FIXO VARIAVEL???
    };
    int rv;
    hip_u8 rsp[256];
    size_t rsplen;
    hip_u8 rspcode;
    struct hip_node_vizinhos_discor_entry *linha;
	struct hip_node_vizinhos_discor *tabela;
	
	size_t ndevs;
    hip_u8 *p;

    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
    if ((rspcode != 0) && (rspcode != 8)) {
        rv = HIP_FAILED;
        goto done;
    }

//	printf("\nFrame neighbor health list {");
//	for (int aux=0; aux<rsplen; aux++)
//	{
//		printf("%d ",rsp[aux]);
//	}
//	printf("}\n");

    //if (rsplen < 15) { ///	ORIGINAL
	if (rsplen < 8) { ///	AQUI!!!!!!!!!!!!!!!!!!!!
        rv = HIP_INVALID_RSP;
        goto done;
    }

    p = rsp + 2;
    rsplen -= 2;

    ndevs = p[1];
    if (ndevs == 0) {
        rv = HIP_OK;
        list = NULL;
        goto done;
    }

    if ((ndevs*3+3) > rsplen) {
        rv = HIP_INVALID_RSP; //OU AQUI!!!
        goto done;
    }

    linha = (struct hip_node_vizinhos_discor_entry *)malloc(sizeof (struct hip_node_vizinhos_discor_entry)*ndevs);
    if (!linha) {
		free(linha);
        rv = HIP_OUT_OF_MEM;
        goto done;
    }

	tabela = (struct hip_node_vizinhos_discor *)malloc(sizeof (struct hip_node_vizinhos_discor));
    if (!tabela) {
        rv = HIP_OUT_OF_MEM;
		free(linha);
		free(tabela);
        goto done;
    }

	tabela->indice_vizinho=p[0];
	tabela->vizinhos_lidos=p[1];
	tabela->total_vizinhos=p[2];
	tabela->lista=linha;

	p=p+3;

    for (i = 0; i < ndevs; i++) 
	{
			linha[i].nickname=p[0]*256+p[1];		
			linha[i].RSL=p[2];
			p=p+3;
    }
    rv = HIP_OK;
    *list = tabela;

done:
    return rv;
}

										
//cmd 800
int hip_get_service_list(struct hip_sess *sess, hip_addr_t addr,
			                            struct hip_services_linked **list)
{
    static hip_u8 req[] = {
        0x1f, 4, 0x3, 0x20, 0, 99 //problema numero de devices fixo
    };

    int i, rv;
    hip_u8 rsp[256];
    size_t rsplen;
    hip_u8 rspcode;
    struct hip_services_linked_entry *linha;
	struct hip_services_linked *tabela;
	
	size_t ndevs;
    hip_u8 *p;

    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
    if ((rspcode != 0) && (rspcode != 8)) {
        rv = HIP_FAILED;
        goto done;
    }

    if (rsplen < 14) {
        rv = HIP_INVALID_RSP;
        goto done;
    }

    p = rsp + 2;
    rsplen -= 2;

    ndevs = p[1];
    if (ndevs == 0) {
        rv = HIP_OK;
        list = NULL;
        goto done;
    }

    if ((ndevs*10+3) > rsplen) {
        rv = HIP_INVALID_RSP;
        goto done;
    }

    linha = (struct hip_services_linked_entry *)malloc(sizeof (struct hip_services_linked_entry)*ndevs);
    if (!linha) {
		free(linha);
        rv = HIP_OUT_OF_MEM;
        goto done;
    }

	tabela = (struct hip_services_linked *)malloc(sizeof (struct hip_services_linked));
    if (!tabela) {
        rv = HIP_OUT_OF_MEM;
		free(linha);
		free(tabela);
        goto done;
    }

	tabela->indice_servicos=p[0];
	tabela->entradas_lidas=p[1];
	tabela->servicos_ativos=p[2];
	tabela->lista=linha;
	p=p+3;

    for (i = 0; i < ndevs; i++) 
	{
			linha[i].service_id=p[0];		
			linha[i].service_flag=p[1];
			linha[i].service_domains=p[2];
			linha[i].nickname_peer=p[3]*256+p[4];
			linha[i].periodo_latencia=(hip_u40)(p[5] << 24)|(p[6] << 16)|(p[7] << 8)|p[8];
			linha[i].route_id=p[9];
			p=p+10;
    }

    rv = HIP_OK;
    *list = tabela;

done:
    return rv;
}


// cmd 802 !!!!
int hip_get_route_list(struct hip_sess *sess, hip_addr_t addr,
			                            struct hip_node_routes **list)
{
    static hip_u8 req[] = {
        0x1f, 4, 0x3, 0x22, 0, 99 //problema numero de devices fixo
    };
    int i, rv;
    hip_u8 rsp[256];
    size_t rsplen;
    hip_u8 rspcode;
    struct hip_node_routes_entry *linha;
	struct hip_node_routes *tabela;
	
	size_t ndevs;
    hip_u8 *p;

    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
    if ((rspcode != 0) && (rspcode != 8)) {
        rv = HIP_FAILED;
        goto done;
    }

    if (rsplen < 10) { //VER DEPOIS!! original 15
        rv = HIP_INVALID_RSP;
        goto done;
    }

	p = rsp + 2; 
    rsplen -= 2;

    ndevs = p[1]; // recebe de req (acho!)
    if (ndevs == 0) {
        rv = HIP_OK;
        list = NULL;
        goto done;
    }

    if ((ndevs*6+4) > rsplen) {
        rv = HIP_INVALID_RSP;
        goto done;
    }

    linha = (struct hip_node_routes_entry *)malloc(sizeof (struct hip_node_routes_entry)*ndevs);
    if (!linha) {
		free(linha);
        rv = HIP_OUT_OF_MEM;
        goto done;
    }
	tabela = (struct hip_node_routes *)malloc(sizeof (struct hip_node_routes));
    if (!tabela) {
        rv = HIP_OUT_OF_MEM;
		free(linha);
		free(tabela);
        goto done;
    }

	tabela->route_index=p[0]; 
	tabela->entries_read=p[1];
	tabela->number_active_routes=p[2];
	tabela->routes_remaining=p[3];
	tabela->lista=linha;

	p=p+4; // 4 primeiros bytes sao geral

    for (i = 0; i < ndevs; i++) 
	{
			linha[i].routeid=p[0];			//cuidar na struct
			linha[i].nickname=p[1]*256+p[2];		
			linha[i].graphid=p[3]*256+p[4];
			linha[i].sourceroute=p[5];
			p=p+6;
    }

    rv = HIP_OK;
    *list = tabela;
done:
    return rv;
}
//

int hip_set_long_tag(struct hip_sess *sess, hip_addr_t addr,
                 const TCHAR *buf)
{
    int rv;
    hip_u8 req[64];
    hip_u8 *p;
    size_t reqlen;
    hip_u8 rspcode;

    p = req;
    *(p++) = 22;
    *(p++) = 32;
    memset(p, 0, 32);
    //rv = WideCharToMultiByte(28591, 0, buf, -1, (char *)p, 32, NULL, NULL);
	//rv=wcstombs((char *)p, buf, 32);
    rv = (memcpy((char *)p, buf, 32)!=NULL);

	if (rv == 0)
    {
        rv = HIP_INVALID_CHAR;
        goto done;
    }
    p += 32;

    reqlen = p - req;
    rv = hip_exec_hart_cmd(sess, addr, req, reqlen, &rspcode, NULL, NULL);
    if (rv != HIP_OK)
    {
        goto done;
    }
    if (rv != 0)
    {
        rv = HIP_FAILED;
        goto done;
    }

    rv = HIP_OK;

done:
    return rv;
}

int hip_read_node(struct hip_sess *sess, hip_addr_t addr,
              struct hip_node_data *data)
{
    static const hip_u8 read_cmd[] = {
        0x83, 0x0
    };

    int rv;
    hip_u8 rspcode;
    hip_u8 buf[64];
    size_t buflen;
    hip_u8 *p;

//	printf("\nFrame Enviado hip_exec_hart_cmd {");
//	for (int aux=0; aux<sizeof (read_cmd); aux++)
//	{
//		printf("%d ",read_cmd[aux]);
//	}
//	printf("}\n");

    buflen = sizeof (buf);
    rv = hip_exec_hart_cmd(sess, addr, read_cmd, sizeof (read_cmd),
            &rspcode, buf, &buflen);

//	printf("\nRetorno hip_exec_hart_cmd(%d)\n",rv);
//	printf("Codigo Resposta hip_exec_hart_cmd(%d)\n",rspcode);
//	printf("Tamanho Resposta hip_exec_hart_cmd(%d)\n",buflen);
//	printf("Frame Resposta hip_exec_hart_cmd {");
//	for (int aux=0; aux<buflen; aux++)
//	{
//		printf("%d ",buf[aux]);
//	}
//	printf("}\n");

    if (rv != HIP_OK) {
        goto done;
    }
    if (rspcode != 0) {
        rv = HIP_FAILED;
        goto done;
    }
    if (buflen < 8) {
        rv = HIP_INVALID_RSP;
        goto done;
    }
    p = buf;

    data->status = (p[0] << 8) | p[1];
    data->alarms = (p[2] << 8) | p[3];
    data->position = (p[4] << 8) | p[5];
    data->torque = (p[6] << 8) | p[7];

    rv = HIP_OK;

done:
    return rv;
}

int hip_set_position(struct hip_sess *sess, hip_addr_t addr,
                 hip_s16 pos)
{
    int rv;
    hip_u8 req[8];
    size_t reqlen;
    hip_u8 rspcode;
	//TODO: rsplen not used - commented by covatti 15/07
	// hip_u8 rsp[8];
    //size_t rsplen;
    hip_u8 *p;

    p = req;
    *(p++) = 0x82;
    *(p++) = 4;
    *(p++) = 0x0;
    *(p++) = 0x10;
    *(p++) = (pos >> 8) & 0xff;
    *(p++) = pos & 0xff;
    reqlen = p - req;

	//TODO: rsplen not used - commented by covatti 15/07
    //rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, req, reqlen, &rspcode, NULL, NULL);
    if (rv != HIP_OK) {
        goto done;
    }
    if (rspcode != 0) {
        rv = HIP_FAILED;
        goto done;
    }

    rv = HIP_OK;

done:
    return rv;
}

int hip_clear_alarms(struct hip_sess *sess, hip_addr_t addr)
{
    static const hip_u8 req[] = {
        0x82, 4, 0x1, 0x0, 0x0, 0x0
    };

    int rv;
    hip_u8 rspcode;
    hip_u8 rsp[8];
    size_t rsplen;

    rsplen = sizeof (rsp);
    rv = hip_exec_hart_cmd(sess, addr, req, sizeof (req),
            &rspcode, rsp, &rsplen);
    if (rv != HIP_OK) {
        goto done;
    }
    if (rspcode != 0) {
        rv = HIP_FAILED;
        goto done;
    }

    rv = HIP_OK;

done:
    return rv;
}

int hip_exec_hart_cmd(struct hip_sess *sess, hip_addr_t addr,
                  const hip_u8 *req, size_t reqlen,
                  hip_u8 *rspcode_out,
                  hip_u8 *rsp, size_t *rsplen)
{
    int rv;
    hip_u8 inbuf[256];
    size_t inlen;
    hip_u8 outbuf[256];
    size_t outlen;
    hip_u8 *p;
    hip_u8 chksum;
    hip_u8 status;
    size_t i;
    int niter;
    size_t bodylen;
    hip_u8 rspcode;
    size_t tmplen;

	if (reqlen > 200) {
        rv = HIP_FAILED;
        goto done;
    }

	p = inbuf;
    p[0] = 0x82;
    p[1] = ((addr >> 32) & 0x3f) | 0xa0;
    p[2] = (addr >> 24) & 0xff;
    p[3] = (addr >> 16) & 0xff;
    p[4] = (addr >> 8) & 0xff;
    p[5] = addr & 0xff;
    p += 6;
    memcpy(p, req, reqlen);
    p += reqlen;
    inlen = p - inbuf;
    chksum = 0;
    for (i = 0; i < inlen; ++i) {
        chksum ^= inbuf[i];
    }
    *(p++) = chksum;
    ++inlen;

    for (niter = HIP_DR_RETRIES; niter > 0; --niter) {
        outlen = sizeof (outbuf);

//		printf("\nFrame Enviado hip_exec_hip_req {");
//		for (int aux=0; aux<inlen; aux++)
//		{
//			printf("%d ",inbuf[aux]);
//		}
//		printf("}\n");
				
		rv = hip_exec_hip_req(sess, &sess->srvaddr,
                HIP_MSGID_PDU, inbuf, inlen,
                NULL, &status, outbuf, &outlen);

//		printf("\nRetorno hip_exec_hip_req(%d)\n",rv);
//		printf("Status hip_exec_hip_req(%d)\n",status);
//		printf("Tamanho Resposta hip_exec_hip_req(%d)\n",outlen);
//		printf("Frame Resposta hip_exec_hip_req {");
//		for (int aux=0; aux<outlen; aux++)
//		{
//			printf("%d ",outbuf[aux]);
//		}
//		printf("}\n");

        if (rv != HIP_OK) {
            goto done;
        }

		if (status != 0) {
            rv = HIP_FAILED;
            goto done;
        }
        if (outlen < 11) {
            rv = HIP_INVALID_RSP;
            goto done;
        }
        p = outbuf + 6;
        outlen -= 9;
        bodylen = p[1];

//		printf("\np=%d",p);printf("\n[p]=%d",p[0]);
//		printf("\noutlen=%d",outlen);
//		printf("\nbodylen=%d",bodylen);

        if (bodylen > outlen) {
            rv = HIP_INVALID_RSP;
            goto done;
        }
        p += 2;
//		printf("\np=%d",p);printf("\n[p+2]=%d",p[0]);

        if (bodylen < 2) {
            rv = HIP_INVALID_RSP;
            goto done;
        }
        rspcode = p[0];
//		printf("\nrspcode=%d",rspcode);

        p += 2;
        bodylen -= 2;
																	    // 
        if ( (rspcode != HIP_RC_DR) && (rspcode != HIP_RC_DELAYINIT) ) // esquema gambiarra para evitar -1 na resposta (codigo 33)
		{
            break;
        }
        sleep(HIP_DR_DELAY);
    }

//	printf("\nnitter=%d",niter);

    if (niter == 0) {
        rv = HIP_FAILED;
        goto done;
    }

//	printf("\nrspcode_out=%d",rspcode_out);
    if (rspcode_out) {
//		printf("\nrspcode_out=%d",rspcode_out);
//		printf("\n[rspcode_out]=%d",*rspcode_out);
//		printf("\nrspcode=%d",rspcode);
        *rspcode_out = rspcode;
    }

//	printf("\nrsplen=%d",rsplen);
    if (rsplen) {
//		printf("\nAQ");
        tmplen = *rsplen;
        *rsplen = bodylen;
        if (bodylen > tmplen) {
//					printf("\nAQ2");
            rv = HIP_BUF_TOO_SMALL;
            goto done;
        }
        if (bodylen > 0) {
//					printf("\nAQ3");
            memcpy(rsp, p, bodylen);
        }
    }

    rv = HIP_OK;

done:
//	printf("\nrv=%d",rv);
    return rv;
}

int hip_exec_hip_req(struct hip_sess *sess,
                 const struct sockaddr_in *toaddr,
                 hip_u8 msgid, const hip_u8 *req, size_t reqlen,
                 struct sockaddr_in *fromaddr,
                 hip_u8 *status, hip_u8 *rsp, size_t *rsplen)
{
    int rv;
//	int aux;

//	printf("\nFrame Enviado hip_send_pkt {");
//	for (int aux=0; aux<reqlen; aux++)
//	{
//		printf("%d ",req[aux]);
//	}
//	printf("}\n");
//
    rv = hip_send_pkt(sess, toaddr, msgid, req, reqlen);
   
//	printf("Retorno hip_send_pkt(%d)\n",rv);

	if (rv != HIP_OK) {
        goto done;
    }
	
	rv = hip_recv_pkt(sess, fromaddr, status, rsp, rsplen);
    if (rv != HIP_OK) {
        goto done;
    }

//	printf("\nRetorno hip_recv_pkt(%d)\n",rv);
//	printf("Status hip_recv_pkt(%d)\n",status);
//	printf("Tamanho Resposta hip_recv_pkt(%d)\n",(*rsplen));
//	printf("Frame Resposta hip_recv_pkt {");
//	for (aux=0; aux<(*rsplen); aux++)
//	{
//		printf("%d ",rsp[aux]);
//	}
//	printf("}\n");

    rv = HIP_OK;

done:
    ++sess->seqno;
    return rv;
}

static int
hip_send_pkt(struct hip_sess *sess,
             const struct sockaddr_in *toaddr,
             hip_u8 msgid, const hip_u8 *req, size_t reqlen)
{
    int rv;
	//int aux;
	hip_u8 hdr[8];
	WSABUF wsabuf[2];
    int nbuf;
	hip_u8 *p;
	size_t total_len;

	p = hdr;
	*(p++) = HIP_PROTO_VERSION;
	*(p++) = HIP_MSGTYPE_REQ;
	*(p++) = msgid;
	*(p++) = 0;
	*(p++) = (sess->seqno >> 8) & 0xff;
	*(p++) = sess->seqno & 0xff;
	total_len = sizeof (hdr) + reqlen;
	*(p++) = (total_len >> 8) & 0xff;
	*(p++) = total_len & 0xff;
	wsabuf[0].buf = (char *)hdr;
	wsabuf[0].len = sizeof (hdr);
    nbuf = 1;

    if (reqlen > 0) {
	    wsabuf[1].buf = (char *)req;
	    wsabuf[1].len = reqlen;
        ++nbuf;
    }

	

//	printf("Frame Requisicao WSASendTo (%d){",total_len);
//	for (int aux=0; aux<wsabuf[0].len; aux++)
//	{
//		printf("%d ",hdr[aux]);
//	}
//	if (reqlen>0)
//	{
//		for (int aux=0; aux<wsabuf[1].len; aux++)
//		{
//			printf("%d ",req[aux]);
//		}
//	}
//	printf("}\n");

	//rv = WSASendTo(sess->sock, wsabuf, nbuf, &nwritten, 0, (struct sockaddr *)toaddr, sizeof (*toaddr), NULL, NULL);
	rv = sendto(sess->sock, wsabuf, nbuf, 0, (struct sockaddr *)toaddr, sizeof (*toaddr));

	if (rv != 0) {
        rv = HIP_FAILED;
		goto done;
	}

    rv = HIP_OK;

done:
    return rv;
}

static int
hip_recv_pkt(struct hip_sess *sess,
             struct sockaddr_in *fromaddr,
             hip_u8 *status, hip_u8 *rsp, size_t *rsplen)
{
    int rv;
    int nread;
    hip_u8 inbuf[256];
    fd_set rfd;
    struct timeval tv;
    socklen_t addrlen;
    socklen_t *paddrlen;
    hip_u8 *p;
    hip_u16 seqno;
    hip_u16 count;
    size_t bodylen;
    size_t tmplen;

    FD_ZERO(&rfd);
    FD_SET(sess->sock, &rfd);
    memset(&tv, 0, sizeof (tv));
    tv.tv_sec = HIP_RECV_TIMEOUT;
    rv = select(1, &rfd, NULL, NULL, &tv);

//	printf("\nRetorno select(%d)\n",rv);

    if (rv == SOCKET_ERROR) {
        rv = HIP_FAILED;
        goto done;
    }
    if (rv == 0) {
        rv = HIP_TIMED_OUT;
        goto done;
    }

    if (fromaddr) {
        addrlen = sizeof (*fromaddr);
        paddrlen = &addrlen;
    } else {
        paddrlen = NULL;
    }
    nread = recvfrom(sess->sock, (char *)inbuf, sizeof (inbuf), 0,
                     (struct sockaddr *)fromaddr, paddrlen);

//	printf("Frame Resposta recvfrom (%d){",nread);
//	for (int aux=0; aux<nread; aux++)
//	{
//		printf("%d ",inbuf[aux]);
//	}
//	printf("}\n");


    if (nread == SOCKET_ERROR) {
        rv = HIP_FAILED;
        goto done;
    }
    if (nread == 0) {
        rv = HIP_FAILED;
        goto done;
    }
    if (nread < 8) {
        rv = HIP_INVALID_RSP;
        goto done;
    }
    p = inbuf;
    if (p[1] != HIP_MSGTYPE_RSP) {
        rv = HIP_INVALID_RSP;
        goto done;
    }
    seqno = (p[4] << 8) | p[5];
    if (seqno != sess->seqno) {
        rv = HIP_INVALID_RSP;
        goto done;
    }
    count = (p[6] << 8) | p[7];
    if (count > nread) {
        rv = HIP_INVALID_RSP;
        goto done;
    }
    if (status) {
        *status = p[3];
    }
    p += 8;
    bodylen = count - 8;

    if (rsplen) {
        tmplen = *rsplen;
        *rsplen = bodylen;
        if (bodylen > tmplen) {
            rv = HIP_BUF_TOO_SMALL;
            goto done;
        }
        if (bodylen > 0) {
            memcpy(rsp, p, bodylen);
        }
    }

    rv = HIP_OK;

done:
    return rv;
}
