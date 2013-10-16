#ifndef HARTIP_H
#define HARTIP_H

#include <netinet/in.h>
#ifdef __cplusplus
extern "C" {
#endif

//adaptions to Windows code
#define BOOL int
#define SOCKET int
#define TCHAR char
#define TRUE 1
#define FALSE 0
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef struct WSABUF {
	u_long   len;
	char *buf;
} WSABUF ;

#define HIPAPI __stdcall

#define HIP_DIM(arr)        (sizeof (arr) / sizeof (arr[0]))

#define HIP_SESS_TIMEOUT        (5 * 60)    // in sec
#define HIP_RECV_TIMEOUT        60          // in sec - valor original era 10 
//#define HIP_RECV_TIMEOUT        10          // in sec - valor original era 10 


#define HIP_OK				    0
#define HIP_FAILED			    -1
#define HIP_OUT_OF_MEM          -2
#define HIP_NAME_LOOKUP_ERR     -3
#define HIP_TIMED_OUT           -4
#define HIP_INVALID_RSP         -5
#define HIP_BUF_TOO_SMALL       -6
#define HIP_INVALID_CHAR        -7

#define HIP_PROTO_VERSION       1

#define HIP_MSGTYPE_REQ         0
#define HIP_MSGTYPE_RSP         1

#define HIP_MSGID_OPEN          0
#define HIP_MSGID_CLOSE         1
#define HIP_MSGID_KEEPALIVE     2
#define HIP_MSGID_PDU           3

#define HIP_RC_DELAYINIT        33	// 
#define HIP_RC_DR               34

#define HIP_DR_RETRIES          30
#define HIP_DR_DELAY            2

#define HIP_GATEWAY_ADDR        0xf981000002

typedef char hip_s8;
typedef unsigned char hip_u8;
typedef unsigned short hip_u16;
typedef short hip_s16;
typedef unsigned int hip_u32;
typedef unsigned long long hip_u40; //jean
typedef unsigned long long hip_u64;

typedef hip_u64 hip_addr_t;

typedef struct 
{
	hip_u8	dia;
	hip_u8	mes;
	hip_u16 ano;
} hip_date;

struct hip_sess {
    BOOL wsastarted;
    SOCKET sock;
    BOOL sess_opened;
    struct sockaddr_in srvaddr;
    hip_u16 seqno;
};

struct hip_node_data {
    hip_u16 status;
    hip_u16 alarms;
    hip_s16 position;
    hip_u16 torque;
};

// feito aal/Jean - 29/10
struct hip_node_vizinhos_linked_entry {
	hip_u16 nickname;
	hip_u8 flags;
	hip_s8 RSL;
	hip_u16 pacotes_okRX;
	hip_u16 pacotes_falhaTX;
	hip_u16 pacotes_okTX;
};

// feito aal/Jean - 29/10
struct hip_node_vizinhos_linked {
	hip_u8 total_vizinhos;
	hip_u8 vizinhos_lidos;
	hip_u8 indice_vizinho;
	struct hip_node_vizinhos_linked_entry *lista;
};


// cmd 782
struct hip_session_list_entry {
	hip_u8	tipo_sessao;
	hip_u16 nickname;
	hip_u40 endereco;
	hip_u32 pdncv;
	hip_u32 tdncv;
};

struct hip_session_list {
	hip_u8 indice_sessao;
	hip_u8 numero_sessoes;
	hip_u8 numero_sessoes_ativas;
	struct hip_session_list_entry *lista;
};

// cmd 783
struct hip_node_superframes_entry {
	hip_u8 superframeid;
	hip_u16 n_slots;
	hip_u8 flag_superframe;
};

struct hip_node_superframes {
	hip_u8 superframe_i;
	hip_u8 n_entries_r;
	hip_u8 n_active_superframe;
	struct hip_node_superframes_entry *lista;
};
//struct para cmd 784
struct hip_node_links_entry {
	hip_u8 superframe_id;
	hip_u16 slot_number_frame;
	hip_u8 channelOff;
	hip_u16 neighbor_link;
	hip_u8 link_opt; //do tipo bits
	hip_u8 link_type; //do tipo enum
};
//cmd 800
struct hip_services_linked_entry {
	hip_u8 service_id;
	hip_u8 service_flag;
	hip_u8 service_domains;
	hip_u16 nickname_peer;
	hip_u40 periodo_latencia;
	hip_u8 route_id;
};
struct hip_services_linked {
	hip_u8 indice_servicos;
	hip_u8 entradas_lidas;
	hip_u8 servicos_ativos;
	struct hip_services_linked_entry *lista;
};
//
struct hip_node_links {
	hip_u16 link_index;
	hip_u8 links_read;
	hip_u16 links_ativos;
	struct hip_node_links_entry *lista;
};
//struct para cmd 840
struct hip_network_devices {
	hip_u40 unique_id;
    hip_u16 n_active_graphs;
	hip_u16 n_active_frames;
	hip_u16 n_active_links;
	hip_u8 number_neighbors;
	hip_u32 avera_comm; //AQUI O FORMATO EH TEMPO deveria	
	hip_u16 n_joins;
//	hip_u40 date_join;  //AQUI O FORMATO EH TEMPO deveria
	hip_date data_join;
	hip_u32 time_ms;
	hip_u32 pack_generate;
	hip_u32 pack_terminated;
	hip_u32 n_mic_failure;
	hip_u32 n_session_fail;
	hip_u32 n_crc_erros;
	hip_u32 n_counterby;
	hip_u32 n_counterfrom;
	hip_u32	deviat_std;

};

//struct para cmd 787
struct hip_node_vizinhos_discor_entry {
	hip_u16 nickname;
	//hip_u8 flags;
	hip_s8 RSL;
	//hip_u16 pacotes_okRX;
	//hip_u16 pacotes_falhaTX;
	//hip_u16 pacotes_okTX;
};

struct hip_node_vizinhos_discor {
	hip_u8 total_vizinhos;
	hip_u8 vizinhos_lidos;
	hip_u8 indice_vizinho;
	struct hip_node_vizinhos_discor_entry *lista;
};
//802
struct hip_node_routes_entry {
	hip_u8 routeid;
	hip_u16 nickname;
	hip_u16 graphid;
	hip_u8 sourceroute;
};

struct hip_node_routes {
	hip_u8 route_index;
	hip_u8 entries_read;
	hip_u8 number_active_routes;
	hip_u8 routes_remaining;
	struct hip_node_routes_entry *lista;
};

//
//

int hip_connect(struct hip_sess **sess_out,
                const TCHAR *name, hip_u16 port);
int hip_close(struct hip_sess *sess);

int hip_enum_nodes(struct hip_sess *sess,
                   hip_addr_t **ppbuf, size_t *pbuflen);
int hip_free_node_list(struct hip_sess *sess,
                       hip_addr_t *buf);
int hip_get_long_tag(struct hip_sess *sess, hip_addr_t addr,
                     TCHAR *buf, size_t buflen);

int hip_get_nickname(struct hip_sess *sess, hip_addr_t addr,
	 				unsigned int *_nickname);
int hip_get_netid(struct hip_sess *sess, hip_addr_t addr,
	 				unsigned int *_netid);

int hip_get_neighbor_discovered_list(struct hip_sess *sess, hip_addr_t addr,
	 							struct hip_node_vizinhos_discor **list);

int hip_get_neighbor_health_list(struct hip_sess *sess, hip_addr_t addr,
	 							struct hip_node_vizinhos_linked **list);

int hip_get_service_list(struct hip_sess *sess, hip_addr_t addr,
	 							struct hip_services_linked **list);

int hip_get_read_list(struct hip_sess *sess, hip_addr_t addr, struct hip_node_links **list, const hip_u8 *req);

int hip_get_session_list(struct hip_sess *sess, hip_addr_t addr,
	 							struct hip_session_list **list);

int hip_get_superframe_list(struct hip_sess *sess, hip_addr_t addr,
	 							struct hip_node_superframes **list);

int hip_get_route_list(struct hip_sess *sess, hip_addr_t addr,
	 							struct hip_node_routes **list);

int hip_get_network_statistics(struct hip_sess *sess, hip_addr_t nodeaddr,
	 							struct  hip_network_devices **list);

int hip_set_long_tag(struct hip_sess *sess, hip_addr_t addr,
                     const TCHAR *buf);
int hip_read_node(struct hip_sess *sess, hip_addr_t addr,
                  struct hip_node_data *data);
int hip_set_position(struct hip_sess *sess, hip_addr_t addr,
                     hip_s16 pos);
int hip_clear_alarms(struct hip_sess *sess, hip_addr_t addr);

int hip_exec_hart_cmd(struct hip_sess *sess, hip_addr_t addr,
                      const hip_u8 *req, size_t reqlen,
                      hip_u8 *rspcode_out,
                      hip_u8 *rsp, size_t *rsplen);
int hip_exec_hip_req(struct hip_sess *sess,
                            const struct sockaddr_in *toaddr,
                            hip_u8 msgid, const hip_u8 *req, size_t reqlen,
                            struct sockaddr_in *fromaddr,
                            hip_u8 *status, hip_u8 *rsp, size_t *rsplen);

#ifdef __cplusplus
}
#endif

#endif
