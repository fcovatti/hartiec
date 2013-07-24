// hartcli.cpp : Defines the entry point for the console application.
//
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "hartip.h"
#include "hartcli.h"
#include "string.h"
static void print_addr(hip_addr_t addr);
static void print_result(int rv);
//static int exists(const char *fname);
static void loop_question(struct hip_sess *sess);
static void loop_question2(struct hip_sess *sess);
static void loop_question3(struct hip_sess *sess);
static void loop_question4(struct hip_sess *sess);
static void loop_question5(struct hip_sess *sess);
static void loop_question6(struct hip_sess *sess);
static void loop_question7(struct hip_sess *sess);
static void loop_question8(struct hip_sess *sess);
static void loop_question9(struct hip_sess *sess);
static void loop_question10(struct hip_sess *sess); //links
//void mainMenu();  teste
#define NODE_ADDR       0xe0ff000101
#define _tprintf printf
#define _T
int k=0;
static TCHAR *gwname;
static hip_u16 gwport = 20004; //emerson
//static hip_u16 gwport = 10100; //nivis

int main(int argc, char* argv[])
{
	int rv, ent=0;
	struct hip_sess *sess;
    TCHAR c;
    //BOOL sat;
	BOOL quit;
	quit = FALSE;

 //   if (argc != 2) {
 //       _tprintf(_T("Usage: hartcli gwname\n"));
 //       return 1;
 //   }
 //   gwname = argv[1];
	 gwname = _T("192.168.1.102"); //nivis
//     gwname = _T("192.168.1.103");
	_tprintf(_T("\n\n"));  	
	_tprintf(_T("**************"));    
	_tprintf(_T("SOFTWARE PARA ANALISE DE DISPOSITIVOS WIRELESSHART"));
	_tprintf(_T("**************\n\n"));
	_tprintf(_T("Conectando com %s..."), gwname);
	rv = hip_connect(&sess, gwname, gwport);
    if (rv != HIP_OK) {
        _tprintf(_T("Falha na conexão.\n"));
        return 1;
    }
	int unsigned tmps=140;
	while (ent<5){
    _tprintf(_T("> "));
	_sleep(tmps);		
    ent++;
	}
	_tprintf(_T("conectado!!\n"));


menu: 
	_tprintf(_T("\n MENU:"));
	_tprintf(_T("\n"));
    _tprintf(_T("e: Enumerar Dispositivos\n"));
    _tprintf(_T("i: Informacao dos Vizinhos\n"));//loop1
	_tprintf(_T("l: Informacao dos Links\n"));//loop10 links
//	_tprintf(_T("s: loop 783 superframe register\n"));//loop4
//	_tprintf(_T("l: loop 784 links register\n"));//loop4
//	_tprintf(_T("m: loop 840 register\n"));//loop2
//	_tprintf(_T("g: loop 802 register\n"));//loop3
//	_tprintf(_T("S: Estatistica dos Dispositivos\n"));//loop3
	_tprintf(_T("a: Analise da Rede\n"));//loop3
	_tprintf(_T("q: quit (Pressione 'q' e aguarde a finalização!)\n"));
    quit = FALSE;
	//mainMenu(); teste
    do {
        c = _getwch();
        switch (c) {
            case _T('e'):
                handle_enum(sess);
                break;
		   	case _T('i'):
                loop_question8(sess);
                break;
		   	case _T('l'): // Analise links
                loop_question10(sess);
                break;
				//		case _T('m'):
	//			loop_question2(sess);
      //          break;
		//	case _T('g'):
//				loop_question3(sess);
  //              break;
	//		case _T('s'):
	//			loop_question4(sess);
      //          break;
		//	case _T('l'):
		//		loop_question5(sess);
          //      break;
			case _T('a'): //analise rede
				loop_question6(sess);
				break;
			case _T('q'):
                quit = TRUE;
                break;
            default:
                _tprintf(_T("comando desconhecido\n"));
                continue;
        }
	
   } while (!quit);
	if (quit) {
	_tprintf(_T("Exiting...\n"));
	hip_close(sess);
	return 0;
				}
	goto menu;

}

void
handle_set_position(struct hip_sess *sess, hip_s16 pos)
{
    int rv;

    _tprintf(_T("Sending set position command...   "));
    fflush(stdout);
    rv = hip_set_position(sess, NODE_ADDR, pos);
    print_result(rv);
}

void
handle_clear_alarms(struct hip_sess *sess)
{
    int rv;

    _tprintf(_T("Clearing alarms...   "));
    fflush(stdout);
    rv = hip_clear_alarms(sess, NODE_ADDR);
    print_result(rv);
}

void
handle_read(struct hip_sess *sess)
{
    int rv;
    struct hip_node_data data;

    _tprintf(_T("Reading node...   "));
    fflush(stdout);
    rv = hip_read_node(sess, NODE_ADDR, &data);
    print_result(rv);
    if (rv != HIP_OK) {
        goto done;
    }

    _tprintf(_T("status: %#x\n"), data.status);
    _tprintf(_T("alarms: %#x\n"), data.alarms);
    _tprintf(_T("position: %d %%\n"), data.position);
    _tprintf(_T("torque: %u Nm\n"), data.torque);

done:
    return;
}

void
handle_enum(struct hip_sess *sess)
{
    int rv;
    hip_addr_t *nodes;
    size_t i;
    size_t nnodes;
    nodes = NULL;
    _tprintf(_T("\nVerificando dispositivos na rede...  "));
    fflush(stdout);
    rv = hip_enum_nodes(sess, &nodes, &nnodes);
    print_result(rv);
    if (rv != HIP_OK) {
        goto done;
    }
    _tprintf(_T("\nNumero de nos: %zu\n\n"), nnodes);
	_tprintf(_T("Endereco dos nos:\n\n"));   

    for (i = 0; i < nnodes; ++i) {
		print_addr(nodes[i]);
        _tprintf(_T("\n"));
    }

done:
    if (nodes) {
        hip_free_node_list(sess, nodes);
    }
    return;
}

void
handle_get_long_tag(struct hip_sess *sess)
{
    int rv;
    TCHAR buf[64];

    _tprintf(_T("Getting long tag...   "));
    fflush(stdout);
    rv = hip_get_long_tag(sess, NODE_ADDR, buf, HIP_DIM(buf));
    print_result(rv);
    if (rv != HIP_OK) {
        goto done;
    }

    _tprintf(_T("Long Tag: %s\n"), buf);

done:
    return;
}

void
handle_set_long_tag(struct hip_sess *sess)
{
    int rv;
    TCHAR buf[64];
    TCHAR *s;
    size_t len;

    _tprintf(_T("Please enter the long tag: "));
    s = _fgetts(buf, HIP_DIM(buf), stdin);
    if (!s) {
        _tprintf(_T("fgets failed\n"));
        goto done;
    }
    len = _tcslen(s);
    if (s[len - 1] != _T('\n')) {
        _tprintf(_T("invalid long tag\n"));
        goto done;
    }
    s[len - 1] = _T('\0');

    _tprintf(_T("Setting long tag...   "));
    fflush(stdout);
    rv = hip_set_long_tag(sess, NODE_ADDR, buf);
    print_result(rv);

done:
    return;
}


void 
handle_get_nickname(struct hip_sess *sess)
{
	unsigned int nick;
    int rv=0;
    //TCHAR buf[64];

    _tprintf(_T("Getting nickname...   "));
    fflush(stdout);
    rv = hip_get_nickname(sess, NODE_ADDR, &nick);
    print_result(rv);
    if (rv != HIP_OK) {
        goto done;
    }

    _tprintf(_T("NickName: %u\n"), nick);

done:
    return;
}
//
void 
handle_get_netid(struct hip_sess *sess)
{
	unsigned int net;
    int rv=0;
    //TCHAR buf[64];

    _tprintf(_T("Getting NetworkID...   "));
    fflush(stdout);
    rv = hip_get_netid(sess, NODE_ADDR, &net);
    print_result(rv);
    if (rv != HIP_OK) {
        goto done;
    }

    _tprintf(_T("NetworkID: %u\n"), net);

done:
    return;
}
//
static void
print_addr(hip_addr_t addr)
{
    _tprintf(_T("%010I64x"), addr);
}

static void
print_result(int rv)
{
    if (rv == HIP_OK) {
        _tprintf(_T("ok\n"));
    } else {
        _tprintf(_T("%d\n"), rv);
    }
}
/*
static void 
handle_get_neighbor_list(struct hip_sess *sess)
{
      int rv;
	  struct hip_node_vizinhos_linked *tabela;
//    hip_addr_t *nodes;
      size_t i;
      size_t nnodes;

	tabela = NULL;

    _tprintf(_T("Buscando vizinhos...   "));
    fflush(stdout);
    rv = hip_get_neighbor_health_list(sess, NODE_ADDR, &tabela);
    print_result(rv);
    if (rv == HIP_OK) 
	{
		_tprintf(_T("\nNeighbor_health_list\n"));
		_tprintf(_T("Numero Total de Vizinhos: %u\n"), tabela->total_vizinhos);
		nnodes = tabela->vizinhos_lidos;
		_tprintf(_T("Numero de Vizinhos: %u\n"), nnodes);
		for (i = 0; i < nnodes; ++i) 
			_tprintf(_T("Nickname %d: (RSL %d, Flags: %d, nTX: %d, nRX: %d, nTXfault %d)\n"),
									   tabela->lista[i].nickname,
									   tabela->lista[i].RSL,
									   tabela->lista[i].flags,
									   tabela->lista[i].pacotes_okTX,
									   tabela->lista[i].pacotes_okRX,
									   tabela->lista[i].pacotes_falhaTX);	

	    if (tabela) {
		    free(tabela->lista);
			free(tabela);
		}
	}
    return;
}
*/
//cmd 840
void
handle_get_network_stat(struct hip_sess *sess)
{
      int rv;
	  struct hip_network_devices *tabela;
//    hip_addr_t *nodes;
      //size_t i;
      //size_t nnodes;

	tabela = NULL;

    _tprintf(_T("Buscando estatisticas...   "));
    fflush(stdout);
    rv = hip_get_network_statistics(sess, NODE_ADDR, &tabela);
    print_result(rv);
    if (rv == HIP_OK) 
	{
		_tprintf(_T("\nNetwork Device´s Statistics\n"));
		_tprintf(_T("Unique ID: ") ); print_addr(tabela->unique_id); _tprintf(_T("\n") );
		_tprintf(_T("Numero de Grafos ativos: %u\n"), tabela->n_active_graphs);
		_tprintf(_T("Numero de Frames ativos: %u\n"),tabela->n_active_frames);
		_tprintf(_T("Numero de Links ativos: %u\n"),tabela->n_active_links);
		_tprintf(_T("Numero de vizinhos: %u\n"), tabela->number_neighbors);
		_tprintf(_T("Media de comunicação do GW para este no: %u\n"), tabela->avera_comm);
		_tprintf(_T("Numero de joins: %u\n"), tabela->n_joins);
		_tprintf(_T("Data do ultimo join: %d/%d/%d\n"), 
							tabela->data_join.dia,
							tabela->data_join.mes,
							tabela->data_join.ano);
		_tprintf(_T("Tempo 1/32 joined: %u\n"), tabela->time_ms);
		_tprintf(_T("Pacotes gerados por este dispositivo: %u\n"), tabela->pack_generate);
		_tprintf(_T("Pacotes terminados neste dispositivo: %u\n"), tabela->pack_terminated);
		_tprintf(_T("Erros camada de enlace: %u\n"),tabela->n_mic_failure);
		_tprintf(_T("Numero de sessoes detectadas: %u\n)"),tabela->n_session_fail);
		_tprintf(_T("CRC erros: %u\n"),tabela->n_crc_erros);
		_tprintf(_T("Numeros nonce counter by this dev: %u\n"),tabela->n_counterby);
		_tprintf(_T("Numero nonce counter from this dev: %u\n"),tabela->n_counterfrom);
		_tprintf(_T("Standard Deviation Latency: %u\n"),tabela->deviat_std);
	    if (tabela) {
					free(tabela);
		}
	}
    return;
}
//
void 
handle_get_neighbor_dlist(struct hip_sess *sess)
{
      int rv;
	  struct hip_node_vizinhos_discor *tabela;
      size_t i;
      size_t nnodes;

	tabela = NULL;

    _tprintf(_T("Buscando vizinhos...   "));
    fflush(stdout);
    rv = hip_get_neighbor_discovered_list(sess, NODE_ADDR, &tabela);
    print_result(rv);
    if (rv == HIP_OK) 
	{
		_tprintf(_T("\nNeighbor_discovered_list\n"));
		_tprintf(_T("Numero Total de Vizinhos: %u\n"), tabela->total_vizinhos);
		nnodes = tabela->vizinhos_lidos;
		_tprintf(_T("Numero de Vizinhos: %u\n"), nnodes);
		for (i = 0; i < nnodes; ++i) 
			_tprintf(_T("Nickname %d: (RSL %d dB)\n"),
									   tabela->lista[i].nickname,
									   tabela->lista[i].RSL);
		    if (tabela) {
		    free(tabela->lista);
			free(tabela);
		}
	}
    return;
}
//cmd 802
void 
handle_get_route_list(struct hip_sess *sess)
{
      int rv;
	  struct hip_node_routes *tabela;
      size_t i;
      size_t nnodes;

	tabela = NULL;

    _tprintf(_T("Buscando lista de rotas...   "));
    fflush(stdout);
    rv = hip_get_route_list(sess, NODE_ADDR, &tabela);
    print_result(rv);
    if (rv == HIP_OK) 
	{
		_tprintf(_T("\nRoute_list\n"));
	//	_tprintf(_T("Numero Total de Vizinhos: %u\n"), tabela->total_vizinhos);
	//	nnodes = tabela->vizinhos_lidos;
	//	_tprintf(_T("Numero de Vizinhos: %u\n"), nnodes);
		_tprintf(_T("Indice de rotas: %u\n"), tabela->route_index);
		_tprintf(_T("Numero de rotas Ativas: %u\n"), tabela->number_active_routes);
		_tprintf(_T("Numero de rotas remaining: %u\n"), tabela->routes_remaining);
		nnodes = tabela->entries_read;
		//_tprintf(_T("Numero de Vizinhos: %u\n"), nnodes);
		
		for (i = 0; i < nnodes; ++i) 
			_tprintf(_T("Route ID %d: (Nickname: %d, Graph ID: %d, Source Route: %d)\n"),
									   tabela->lista[i].routeid,
									   tabela->lista[i].nickname,
									   tabela->lista[i].graphid,
									   tabela->lista[i].sourceroute);

	    if (tabela) {
		    free(tabela->lista);
			free(tabela);
		}
	}
    return;
}

/*int exists(const char *fname)
	{
	FILE *file;
	if (file = fopen(fname, "r")){
		fclose(file);
		return 1;
	}
	return 0;
	}*/

static void 
loop_question(struct hip_sess *sess){
	{
	int rv;
    hip_addr_t *nodes;
    size_t nnodes;
	struct hip_node_vizinhos_linked *tabela_vizinho[20];
	unsigned int nickname[20];
	FILE *arq;

	time_t agora;
	struct tm *ts;
	char timestamp[30];

	int aux,aux2;

	nodes = NULL;
	for (aux=0;aux<10;aux++)
	{
		tabela_vizinho[aux]=NULL;
	}
	_tprintf(_T("Listando Nos ...\n"));
    fflush(stdout);
    rv = hip_enum_nodes(sess, &nodes, &nnodes);	
    if (rv == HIP_OK) 
	{
		for (aux=0;aux<nnodes;aux++)
		{
			printf("Buscando vizinho ... Addr ");
			print_addr(nodes[aux]);	// Escreve endereco
			
 		    rv = hip_get_nickname(sess, nodes[aux], &nickname[aux]);
		    if (rv == HIP_OK) 
		    {
				_tprintf(_T(" NickName: %u\n"), nickname[aux]);
		    }
			else
			{
				printf("\nErro hip_get_nickname (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
	
			rv = hip_get_neighbor_health_list(sess, nodes[aux], &tabela_vizinho[aux]);
			if (rv == HIP_OK) 
			{
			
				if ((arq = fopen("Comando780.csv", "r"))){ 
					fclose(arq); 
					arq = fopen("Comando780.csv","a");
					//printf("Existo");
				}
				else {
					arq = fopen("Comando780.csv","w");
					fprintf(arq,"Date;Hour;Nickname from;Nickname destination;RSL;Packets transmitted;Packets received;Neighbor Flags;Ciclo\n");
					//printf(" Nao Existo\n");
				}
	
				// Cria timestamp
				agora = time(NULL);
				ts = localtime(&agora);
				strftime(timestamp,sizeof(timestamp),"%d/%m/%Y;%H:%M:%S",ts);

				for (aux2 = 0; aux2 < tabela_vizinho[aux]->vizinhos_lidos; aux2++) 
				{
						_tprintf(_T("\t Nickname %d: (RSL %d, Flags: %d, nTX: %d, nRX: %d, nTXfault %d)\n"),
									   tabela_vizinho[aux]->lista[aux2].nickname,
									   tabela_vizinho[aux]->lista[aux2].RSL,
									   tabela_vizinho[aux]->lista[aux2].flags,
									   tabela_vizinho[aux]->lista[aux2].pacotes_okTX,
									   tabela_vizinho[aux]->lista[aux2].pacotes_okRX,
									   tabela_vizinho[aux]->lista[aux2].pacotes_falhaTX);	
						
						fprintf(arq,"%s;%d;%d;%d;%d;%d;%d;%d\n", 
														timestamp,
														nickname[aux],
														tabela_vizinho[aux]->lista[aux2].nickname,
														tabela_vizinho[aux]->lista[aux2].RSL,
														tabela_vizinho[aux]->lista[aux2].pacotes_okTX,
														tabela_vizinho[aux]->lista[aux2].pacotes_okRX,
														tabela_vizinho[aux]->lista[aux2].flags,
														k);
				}
				fclose(arq);
			}
			else
			{
				printf("\nErro hip_get_neighbor_health (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
		}
	}
	else
	{
		printf("\nErro hip_enum_nodes: retorno %d\n",rv);
    }

	if (nodes) 
	{
        hip_free_node_list(sess, nodes);
    }
	
	for (aux=0;aux<9;aux++)
	{
	    if (tabela_vizinho[aux]) 
		{
		    free(tabela_vizinho[aux]->lista);
			free(tabela_vizinho[aux]);
		}
	}

}
	return;
} 
//loop para cmd 840
static void 
loop_question2(struct hip_sess *sess){
{
	int rv;
    hip_addr_t *nodes;
    size_t nnodes;
	struct hip_network_devices *tabela_vizinho[99]; 
	unsigned int nickname[20];
	FILE *arq;

	time_t agora;
	struct tm *ts;
	char timestamp[30];

	int aux;
	//int aux2;

	nodes = NULL;
	for (aux=0;aux<99;aux++)
	{
		tabela_vizinho[aux]=NULL;
	}
	
	_tprintf(_T("Listando Nos ...\n"));
    fflush(stdout);
    rv = hip_enum_nodes(sess, &nodes, &nnodes);	
    if (rv == HIP_OK) 
	{
		for (aux=0;aux<nnodes;aux++)
		{
			print_addr(nodes[aux]);	// Escreve endereco
			rv = hip_get_nickname(sess, nodes[aux], &nickname[aux]);
		    if (rv == HIP_OK) 
		    {
				_tprintf(_T(" NickName: %u...."), nickname[aux]);
		    }
			else
			{
				printf("\nErro hip_get_nickname (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
			printf("Armazenando dados .. \n");
			rv = hip_get_network_statistics(sess, nodes[aux], &tabela_vizinho[aux]);
			if (rv == HIP_OK) 
			{
					if ((arq = fopen("Comando840.csv", "r"))){ 
					fclose(arq); 
					arq = fopen("Comando840.csv","a");
				}
					else {
					arq = fopen("Comando840.csv","w");
					fprintf(arq,"Date;Hour;Unique ID;Nickname;Active Graph;Active Frames;Active Links;Neighbors;Average communication latency;Joins;Date recent join;Time recently joined;Packets generated;Packets terminated;Data-Link layer failures;Network layer failure;CRC errors;Nonce Counter by;Nonce Counter from; D; Ciclo\n");
				}
				// Cria timestamp
				agora = time(NULL);
				ts = localtime(&agora);
				strftime(timestamp,sizeof(timestamp),"%d/%m/%Y;%H:%M:%S",ts);

				fprintf(arq,"%s; %llx; %d; %d; %d; %d ;%d; %0.5f; %d; %d/%d/%d; %u; %d; %d; %d ;%d; %d; %d; %d; %d; %d\n", //;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;;%d;%d\n",	
				timestamp,
				(unsigned long long)tabela_vizinho[aux]->unique_id,
				nickname[aux],
				tabela_vizinho[aux]->n_active_graphs,
				tabela_vizinho[aux]->n_active_frames,
				tabela_vizinho[aux]->n_active_links,
				tabela_vizinho[aux]->number_neighbors,
				(double)(((double)tabela_vizinho[aux]->avera_comm)*0.03125),
				tabela_vizinho[aux]->n_joins,
				tabela_vizinho[aux]->data_join.dia,
				tabela_vizinho[aux]->data_join.mes,
				tabela_vizinho[aux]->data_join.ano,
				tabela_vizinho[aux]->time_ms,
				tabela_vizinho[aux]->pack_generate,
				tabela_vizinho[aux]->pack_terminated,
				tabela_vizinho[aux]->n_mic_failure,
				tabela_vizinho[aux]->n_session_fail,
				tabela_vizinho[aux]->n_crc_erros,
				tabela_vizinho[aux]->n_counterby,
				tabela_vizinho[aux]->n_counterfrom,
				tabela_vizinho[aux]->deviat_std,
				k);
				fclose(arq);

			}
			else
			{
				printf("\nErro hip_get_network_statistics (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
		}
	}
	else
	{
		printf("\nErro hip_enum_nodes: retorno %d\n",rv);
    }

	if (nodes) 
	{
        hip_free_node_list(sess, nodes);
    }
}
	return;
} 
//loop para cmd 802
static void 
loop_question3(struct hip_sess *sess){
{
	int rv;
    hip_addr_t *nodes;
    size_t nnodes;
	struct hip_node_routes *tabela_vizinho[20];
	unsigned int nickname[20];
	FILE *arq;
//Preparando timestamp
	time_t agora;
	struct tm *ts;
	char timestamp[30];
	int aux,aux2;

	nodes = NULL;
	for (aux=0;aux<20;aux++)
	{
		tabela_vizinho[aux]=NULL;
	}
		

	_tprintf(_T("Listando Nos ...\n"));
    fflush(stdout);
    rv = hip_enum_nodes(sess, &nodes, &nnodes);	
    if (rv == HIP_OK) 
	{
		for (aux=0;aux<nnodes;aux++)
		{
			printf("Buscando vizinho ... Addr ");
			print_addr(nodes[aux]);	// Escreve endereco
			
 		    rv = hip_get_nickname(sess, nodes[aux], &nickname[aux]);
		    if (rv == HIP_OK) 
		    {
				_tprintf(_T(" NickName: %u\n"), nickname[aux]);
		    }
			else
			{
				printf("\nErro hip_get_nickname (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
			rv = hip_get_route_list(sess, nodes[aux], &tabela_vizinho[aux]);
			if (rv == HIP_OK) 
			{
				if ((arq = fopen("Comando802.csv", "r"))){ 
					fclose(arq); 
					arq = fopen("Comando802.csv","a");
					}
				else {
					arq = fopen("Comando802.csv","w");
					fprintf(arq,"Date;Hour;Number Active Routes;Source;Destination;Route ID;Graph ID;Source Route(0);Ciclo\n");
					}
				// Cria timestamp
				agora = time(NULL);
				ts = localtime(&agora);
				strftime(timestamp,sizeof(timestamp),"%d/%m/%Y;%H:%M:%S",ts);

				for (aux2 = 0; aux2 < tabela_vizinho[aux]->entries_read; aux2++) 
				{
						_tprintf(_T("\t Rotas ativas %d: (Nick Orig %d, Nick Dest: %d, Route ID %d;Graph ID: %d, Source R.: %d\n"),
									             		tabela_vizinho[aux]->number_active_routes,
														nickname[aux],
														tabela_vizinho[aux]->lista[aux2].nickname,
														tabela_vizinho[aux]->lista[aux2].routeid,
														tabela_vizinho[aux]->lista[aux2].graphid,
														tabela_vizinho[aux]->lista[aux2].sourceroute);
						
						fprintf(arq,"%s;%d;%d;%d;%d;%d;%d;%d\n", 
														timestamp,
														tabela_vizinho[aux]->number_active_routes,
														nickname[aux],
														tabela_vizinho[aux]->lista[aux2].nickname,
														tabela_vizinho[aux]->lista[aux2].routeid,
														tabela_vizinho[aux]->lista[aux2].graphid,
														tabela_vizinho[aux]->lista[aux2].sourceroute,
														k);
														
				}
				fclose(arq);
			}
			else
			{
				printf("\nErro hip_get_route_list (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
		}
	}
	else
	{
		printf("\nErro hip_enum_nodes: retorno %d\n",rv);
    }

	if (nodes) 
	{
        hip_free_node_list(sess, nodes);
    }
	
	for (aux=0;aux<19;aux++)
	{
	    if (tabela_vizinho[aux]) 
		{
		    free(tabela_vizinho[aux]->lista);
			free(tabela_vizinho[aux]);
		}
	}

}
	return;
} 

//loop para cmd 782
static void 
loop_question9(struct hip_sess *sess)
{
	FILE *arq;
	int rv;
	char * pSessionType;
    hip_addr_t *nodes;
    size_t nnodes;
	struct hip_session_list *tabela_sessoes[30];
	unsigned int nickname[20];
	int aux,aux2;
	time_t agora;
	struct tm *ts;
	char timestamp[30];

	nodes = NULL;
	for (aux=0;aux<30;aux++)
	{
		tabela_sessoes[aux]=NULL;
	}

	_tprintf(_T("Listando Nos ...\n"));
    fflush(stdout);
    rv = hip_enum_nodes(sess, &nodes, &nnodes);	
    if (rv == HIP_OK) 
	{
		for (aux=0;aux<nnodes;aux++)
		{
			printf("Buscando vizinho ... Addr ");
			print_addr(nodes[aux]);	// Escreve endereco
			
 		    rv = hip_get_nickname(sess, nodes[aux], &nickname[aux]);
		    if (rv == HIP_OK) 
		    {
				_tprintf(_T(" NickName: %u\n"), nickname[aux]);
		    }
			else
			{
				printf("\nErro hip_get_nickname (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
				continue;
			}

			rv = hip_get_session_list(sess, nodes[aux], &tabela_sessoes[aux]);
			if (rv == HIP_OK) 
			{

				if ((arq = fopen("Comando782.csv", "r"))){ 
					fclose(arq); 
					arq = fopen("Comando782.csv","a");
					}
				else {
					arq = fopen("Comando782.csv","w");
					fprintf(arq,"Date;Hour;Nickname;Number Active Sessions;Session Type;Peer Device Nick; Peer Device Unique ID; Peer Device Nonce CV; The Device Nonce CV;Ciclo\n");
				}
				// Cria timestamp
				agora = time(NULL);
				ts = localtime(&agora);
				strftime(timestamp,sizeof(timestamp),"%d/%m/%Y;%H:%M:%S",ts);

				printf("\t(Total Sessoes %d, Sessoes Ativas %d)\n",
					tabela_sessoes[aux]->numero_sessoes,
					tabela_sessoes[aux]->numero_sessoes_ativas);
				
				for (aux2=0;aux2<tabela_sessoes[aux]->numero_sessoes;aux2++)
				{
					switch (tabela_sessoes[aux]->lista[aux2].tipo_sessao)
					{
						case 0:	 pSessionType = "Unicast"; break;
						case 1:  pSessionType = "Broadcast"; break;
						case 2:  pSessionType = "Join"; break;
						default: pSessionType = "ERR"; break;
					}
					
					printf("\t\t TipoSessao %s, Nickname %X, Addr %010I64X, pdncv %u, tdncv %u\n",
 						pSessionType,
						tabela_sessoes[aux]->lista[aux2].nickname,
						tabela_sessoes[aux]->lista[aux2].endereco,
						tabela_sessoes[aux]->lista[aux2].pdncv,
						tabela_sessoes[aux]->lista[aux2].tdncv);

					fprintf(arq,"%s;%u;%d;%s;%X;%010I64X;%u;%u;%d\n", 
														timestamp,
														nickname[aux],
														tabela_sessoes[aux]->numero_sessoes_ativas,
														pSessionType,
														tabela_sessoes[aux]->lista[aux2].nickname,
														tabela_sessoes[aux]->lista[aux2].endereco,
														tabela_sessoes[aux]->lista[aux2].pdncv,
														tabela_sessoes[aux]->lista[aux2].tdncv,
														k);


				}
				fclose(arq);
			}
			else
			{
				printf("\nErro hip_get_session_list (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
		}
	}
	else
	{
		printf("\nErro hip_enum_nodes: retorno %d\n",rv);
    }

	if (nodes) 
	{
        hip_free_node_list(sess, nodes);
    }
	
	for (aux=0;aux<29;aux++)
	{
	    if (tabela_sessoes[aux]) 
		{
		    free(tabela_sessoes[aux]->lista);
			free(tabela_sessoes[aux]);
		}
	}

}


//loop para cmd 783
static void 
loop_question4(struct hip_sess *sess){
{
	int rv;
    hip_addr_t *nodes;
    size_t nnodes;
	struct hip_node_superframes *tabela_vizinho[20];
	unsigned int nickname[20];
	FILE *arq;
	time_t agora;
	struct tm *ts;
	char timestamp[30];

	int aux,aux2;
	nodes = NULL;
	for (aux=0;aux<10;aux++)
	{
		tabela_vizinho[aux]=NULL;
	}
		

	_tprintf(_T("Listando Nos ...\n"));
    fflush(stdout);
    rv = hip_enum_nodes(sess, &nodes, &nnodes);	
    if (rv == HIP_OK) 
	{
		for (aux=0;aux<nnodes;aux++)
		{
			printf("Buscando vizinho ... Addr ");
			print_addr(nodes[aux]);	// Escreve endereco
			
 		    rv = hip_get_nickname(sess, nodes[aux], &nickname[aux]);
		    if (rv == HIP_OK) 
		    {
				_tprintf(_T(" NickName: %u\n"), nickname[aux]);
		    }
			else
			{
				printf("\nErro hip_get_nickname (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
			rv = hip_get_superframe_list(sess, nodes[aux], &tabela_vizinho[aux]);
			if (rv == HIP_OK) 
			{
				if ((arq = fopen("Comando783.csv", "r"))){ 
					fclose(arq); 
					arq = fopen("Comando783.csv","a");
					}
				else {
					arq = fopen("Comando783.csv","w");
					fprintf(arq,"Date;Hour;Nickname;Number Active Superframes;Superframe ID;Number of slots (in this superframe); Flag Superframe;Ciclo\n");
				}
				// Cria timestamp
				agora = time(NULL);
				ts = localtime(&agora);
				strftime(timestamp,sizeof(timestamp),"%d/%m/%Y;%H:%M:%S",ts);

				for (aux2 = 0; aux2 < tabela_vizinho[aux]->n_entries_r; aux2++) 
				{
					_tprintf(_T("\t Nickname:%d; Número Superframes Ativos: %d; Superframe ID: %d; Numero de Slots: %d; Flag Superframe: %d)\n"),
									             		nickname[aux],
														tabela_vizinho[aux]->n_active_superframe,
														tabela_vizinho[aux]->lista[aux2].superframeid,
														tabela_vizinho[aux]->lista[aux2].n_slots,
														tabela_vizinho[aux]->lista[aux2].flag_superframe);
						
						fprintf(arq,"%s;%d;%d;%d;%d;%d;%d\n", 
														timestamp,
														nickname[aux],
														tabela_vizinho[aux]->n_active_superframe,
														tabela_vizinho[aux]->lista[aux2].superframeid,
														tabela_vizinho[aux]->lista[aux2].n_slots,
														tabela_vizinho[aux]->lista[aux2].flag_superframe,
														k);
																												
				}
				fclose(arq);
			}
			else
			{
				printf("\nErro hip_get_superframe_list (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
		}
	}
	else
	{
		printf("\nErro hip_enum_nodes: retorno %d\n",rv);
    }

	if (nodes) 
	{
        hip_free_node_list(sess, nodes);
    }
	
	for (aux=0;aux<9;aux++)
	{
	    if (tabela_vizinho[aux]) 
		{
		    free(tabela_vizinho[aux]->lista);
			free(tabela_vizinho[aux]);
		}
	}

}
	return;
}
//loop cmd 784
static void 
loop_question5(struct hip_sess *sess){
{	
	char * pLinkOpt;
	char * pLinkType;
	int rv, s=10;
	//int c;
    hip_addr_t *nodes;
    size_t nnodes;
	struct hip_node_links *tabela_vizinho[20];
	//static hip_u8 req[6] = {0x1f, 5, 0x3, 0x10, 0, 0, 99};
	unsigned int nickname[20];
	FILE *arq;
//Preparando timestamp
	time_t agora;
	struct tm *ts;
	char timestamp[30];
	int aux,aux2;

	nodes = NULL;
	for (aux=0;aux<20;aux++)
	{
		tabela_vizinho[aux]=NULL;
	}
		

	_tprintf(_T("Listando Nos ...\n"));
    fflush(stdout);
    rv = hip_enum_nodes(sess, &nodes, &nnodes);	
    if (rv == HIP_OK) 
	{
		for (aux=0;aux<nnodes;aux++)
		{
			int c=1;
			printf("Buscando vizinho ... Addr ");
			print_addr(nodes[aux]);	// Escreve endereco
			
 		    rv = hip_get_nickname(sess, nodes[aux], &nickname[aux]);
		    if (rv == HIP_OK) 
		    {
				_tprintf(_T(" NickName: %u\n"), nickname[aux]);
		    }
			else
			{
				printf("\nErro hip_get_nickname (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
			//inserir outro for para fazer dois ciclos 		
	//		while c==1 //for (int ax=0; ax<3; ax++)
	//		{
			static hip_u8 req[7] = {0x1f, 5, 0x3, 0x10}; //static
		    		req[4] = 0;
					req[5] = 0;
					req[6] = 99;
					do {
					//while (c==1) {
//			if(c==1){
//					req[5] = 10;
//					}		
//			printf("\nNo hartCLI\n"); //teste
//			printf("tamanho do req: %d \n",sizeof(req)); //teste
//			for (int aux=0; aux<sizeof(req); aux++) //teste
//			{
//				printf("%d ",req[aux]);
//			}
//			printf("}\n"); //teste*

			rv = hip_get_read_list(sess, nodes[aux], &tabela_vizinho[aux], req);
				if ((tabela_vizinho[aux]->links_read) > (s-1)){ //Aqui verificar possivel problema!
				c=1;
				req[5]+=10;
				//s=s+10;
				//printf("\n\n%d \n\n",10); //imprime para debug
				}
				else {
				c=0;
				}

			if (rv == HIP_OK) 
			{
					if ((arq = fopen("Comando784.csv", "r"))){ 
					fclose(arq); 
					arq = fopen("Comando784.csv","a");
				}
				else {
					arq = fopen("Comando784.csv","w");
					fprintf(arq,"Date;Hour;Links Ativos;Nickname;Superframe ID;Slot number(link);Channel OffSet;Neighbor;Link Options;Link type;Ciclos\n");
				}
				agora = time(NULL);
				ts = localtime(&agora);
				strftime(timestamp,sizeof(timestamp),"%d/%m/%Y;%H:%M:%S",ts);

				for (aux2 = 0; aux2 < tabela_vizinho[aux]->links_read; aux2++) 
				{
					printf("Nickname:%d; Superframe ID: %d; Numero do Slot (link): %d; Channel OffSet: %d; LinksAtivos: %d\n",
									             		nickname[aux],
														tabela_vizinho[aux]->lista[aux2].superframe_id,
														tabela_vizinho[aux]->lista[aux2].slot_number_frame,
														tabela_vizinho[aux]->lista[aux2].channelOff,
														tabela_vizinho[aux]->links_ativos);////
					
					switch (tabela_vizinho[aux]->lista[aux2].link_type)
					{
						case 0:  pLinkType = "Normal";			break;
						case 1:  pLinkType = "Discovery";		break;
						case 2:  pLinkType = "Broadcast";		break;
						case 3:  pLinkType = "Join";			break;
						default: pLinkType = "ERR";				break;
					}

					switch (tabela_vizinho[aux]->lista[aux2].link_opt)
					{
						case 1:  pLinkOpt = "Transmit";					break;
						case 2:  pLinkOpt = "Receive";					break;
						case 3:  pLinkOpt = "Transmit|Receive";			break;
						case 4:  pLinkOpt = "Share";					break;
						case 5:  pLinkOpt = "Transmit|Share";			break;
						case 6:  pLinkOpt = "Receive|Share";			break;
						case 7:  pLinkOpt = "Transmit|Receive|Share";	break;
						default: pLinkOpt = "ERR";						break;
					}
					
					printf("Vizinho: %d; Link Options: %s; Link tipo: %s)\n",
														tabela_vizinho[aux]->lista[aux2].neighbor_link,
														pLinkOpt,
														pLinkType);
						
					fprintf(arq,"%s;%d;%d;%d;%d;%d;%x;%s;%s;%d\n", 
														timestamp,
														tabela_vizinho[aux]->links_ativos,
														nickname[aux],
														tabela_vizinho[aux]->lista[aux2].superframe_id,
														tabela_vizinho[aux]->lista[aux2].slot_number_frame,
														tabela_vizinho[aux]->lista[aux2].channelOff,
														tabela_vizinho[aux]->lista[aux2].neighbor_link,
														pLinkOpt,
														pLinkType,
														k);
				}
				fclose(arq);
			}
			else
			{
				printf("\nhip_get_read_list (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
			} while (c!=0);//pertence ao "for" dos links
		}
	}
	else
	{
		printf("\nErro hip_enum_nodes: retorno %d\n",rv);
    }

	if (nodes) 
	{
        hip_free_node_list(sess, nodes);
    }
	
	for (aux=0;aux<19;aux++)
	{
	    if (tabela_vizinho[aux]) 
		{
		    free(tabela_vizinho[aux]->lista);
			free(tabela_vizinho[aux]);
		}
	}

}
	return;
}

//cmd 800 - read service´s list
static void
loop_question7(struct hip_sess *sess){
	{
	int rv;
    hip_addr_t *nodes;
    size_t nnodes;
	struct hip_services_linked *tabela_services[20];
	unsigned int nickname[20];
	FILE *arq;

	time_t agora;
	struct tm *ts;
	char timestamp[30];

	int aux,aux2;

	nodes = NULL;
	for (aux=0;aux<20;aux++)
	{
		tabela_services[aux]=NULL;
	}
		

	_tprintf(_T("Listando Nos ...\n"));
    fflush(stdout);
    rv = hip_enum_nodes(sess, &nodes, &nnodes);	
    if (rv == HIP_OK) 
	{
		for (aux=0;aux<nnodes;aux++)
		{
			printf("Buscando vizinho ... Addr ");
			print_addr(nodes[aux]);	// Escreve endereco
			
 		    rv = hip_get_nickname(sess, nodes[aux], &nickname[aux]);
		    if (rv == HIP_OK) 
		    {
				_tprintf(_T(" NickName: %u\n"), nickname[aux]);
		    }
			else
			{
				printf("\nErro hip_get_nickname (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
	
			rv = hip_get_service_list(sess, nodes[aux], &tabela_services[aux]); //substituir tabela_services
			if (rv == HIP_OK) 
			{
			
				if ((arq = fopen("Comando800.csv", "r"))){ 
					fclose(arq); 
					arq = fopen("Comando800.csv","a");
					//printf("Existo");
				}
				else {
					arq = fopen("Comando800.csv","w");
					fprintf(arq,"Date;Hour;Source;Active services;Service ID;Service Req. Flag;Service Domain;Nickname Peer; Periodo; Route ID;Ciclo\n");
					//printf(" Nao Existo\n");
				}
	
				// Cria timestamp
				agora = time(NULL);
				ts = localtime(&agora);
				strftime(timestamp,sizeof(timestamp),"%d/%m/%Y;%H:%M:%S",ts);

				for (aux2 = 0; aux2 < tabela_services[aux]->entradas_lidas; aux2++) 
				{
						//printf("\t Nickname %d; Servicos Ativos %d; Service ID %d; Service Req. Flag %d; Service Domain %d; Nickname Peer %d; Periodo %0.2f)\n",
						  printf("\t Servicos Ativos %d; Service ID %d; Service Req. Flag %d; Service Domain %d; Nickname Peer %d; Periodo %0.2f)\n",
														//nickname[aux],
														tabela_services[aux]->servicos_ativos,
														tabela_services[aux]->lista[aux2].service_id,
														tabela_services[aux]->lista[aux2].service_flag,
														tabela_services[aux]->lista[aux2].service_domains,
														tabela_services[aux]->lista[aux2].nickname_peer,
														(float) tabela_services[aux]->lista[aux2].periodo_latencia*0.03125);	
						
						fprintf(arq,"%s;%d;%d;%d;%d;%d;%X;%0.2f;%d;%d\n", 
														timestamp,
														nickname[aux],
													    tabela_services[aux]->servicos_ativos,
														tabela_services[aux]->lista[aux2].service_id,
														tabela_services[aux]->lista[aux2].service_flag,
														tabela_services[aux]->lista[aux2].service_domains,
														tabela_services[aux]->lista[aux2].nickname_peer,
														(float) tabela_services[aux]->lista[aux2].periodo_latencia*0.03125,
														tabela_services[aux]->lista[aux2].route_id,
														k);	
				}
				fclose(arq);
			}
			else
			{
				printf("\nErro hip_get_service_list (no ");
				print_addr(nodes[aux]);	// Escreve endereco
				printf(" ) retorno %d'\n",rv);
			}
		}
	}
	else
	{
		printf("\nErro hip_enum_nodes: retorno %d\n",rv);
    }

	if (nodes) 
	{
        hip_free_node_list(sess, nodes);
    }
	
	for (aux=0;aux<19;aux++)
	{
	    if (tabela_services[aux]) 
		{
		    free(tabela_services[aux]->lista);
			free(tabela_services[aux]);
		}
	}

}
	return;
}


//implementando loop all comds

static void
loop_question6(struct hip_sess *sess){
char c = 'c';
while (c != 'q')
{
	if (kbhit()){
		c = _getch();
	}
k++;
printf("\nEnviando 780 \n");
loop_question(sess);//cmd 780
printf("\nEnviando 782\n");
loop_question9(sess);//cmd 782
printf("\nEnviando 783\n");
loop_question4(sess);//cmd 783
printf("\nEnviando 784\n");
loop_question5(sess);//cmd 784
printf("\nEnviando 800\n");
loop_question7(sess);//cmd 800
printf("\nEnviando 802\n");
loop_question3(sess);//cmd 802
printf("\nEnviando 840 \n");
loop_question2(sess);//cmd 840

}
}
static void
loop_question8(struct hip_sess *sess){
char c = 'c';
while (c != 'q')
{
	if (kbhit()){
		c = _getch();
	}
printf("\nEnviando 780 \n");
loop_question(sess);//cmd 780
}
}
//para os links metodo pragmatico	
static void
loop_question10(struct hip_sess *sess){
char c = 'c';
while (c != 'q')
{
	if (kbhit()){
		c = _getch();
	}
printf("\nEnviando 784 \n");
loop_question5(sess);//cmd 784
}
}


/*void mainMenu(){
	 BOOL quit, sat;
	_tprintf(_T("\n MENU:"));
	_tprintf(_T("\n"));
    _tprintf(_T("e: enumerar nos (cmd 814)\n"));
    _tprintf(_T("c: loop 780 health register\n"));//loop1
	_tprintf(_T("s: loop 783 superframe register\n"));//loop4
	_tprintf(_T("l: loop 784 links register\n"));//loop4
	_tprintf(_T("m: loop 840 register\n"));//loop2
	_tprintf(_T("g: loop 802 register\n"));//loop3
	_tprintf(_T("t: loop 6 teste\n"));//loop3
	_tprintf(_T("q: quit\n"));
    quit = FALSE;
	return;
}*/
