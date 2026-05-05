#ifndef NET_H
#define NET_H

#include "types.h"

/* Ethernet */
#define ETH_ALEN       6
#define ETH_TYPE_IP    0x0800
#define ETH_TYPE_ARP   0x0806

typedef struct {
    uint8_t  dst_mac[ETH_ALEN];
    uint8_t  src_mac[ETH_ALEN];
    uint16_t type;
} PACKED eth_header_t;

/* ARP */
#define ARP_HTYPE_ETH   1
#define ARP_PTYPE_IP    0x0800
#define ARP_OP_REQUEST  1
#define ARP_OP_REPLY    2

typedef struct {
    uint16_t htype;
    uint16_t ptype;
    uint8_t  hlen;
    uint8_t  plen;
    uint16_t op;
    uint8_t  src_mac[ETH_ALEN];
    uint32_t src_ip;
    uint8_t  dst_mac[ETH_ALEN];
    uint32_t dst_ip;
} PACKED arp_header_t;

#define ARP_CACHE_SIZE 32
typedef struct {
    uint32_t ip;
    uint8_t  mac[ETH_ALEN];
    bool     valid;
    uint32_t timestamp;
} arp_cache_entry_t;

/* IP */
#define IP_PROTOCOL_ICMP  1
#define IP_PROTOCOL_TCP   6
#define IP_PROTOCOL_UDP   17

typedef struct {
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t flags_offset;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
} PACKED ip_header_t;

/* ICMP */
#define ICMP_ECHO_REPLY    0
#define ICMP_ECHO_REQUEST  8

typedef struct {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
} PACKED icmp_header_t;

/* UDP */
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} PACKED udp_header_t;

/* TCP */
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_offset;
    uint8_t  flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent;
} PACKED tcp_header_t;

#define TCP_FIN  0x01
#define TCP_SYN  0x02
#define TCP_RST  0x04
#define TCP_PSH  0x08
#define TCP_ACK  0x10
#define TCP_URG  0x20

/* NIC driver interface */
typedef struct {
    char     name[32];
    uint8_t  mac[ETH_ALEN];
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
    bool     link_up;
    int      (*send)(const uint8_t* data, uint32_t len);
    int      (*recv)(uint8_t* buf, uint32_t max_len);
} nic_t;

/* Network stack functions */
void     net_init(void);
void     net_set_ip(uint32_t ip, uint32_t netmask, uint32_t gateway);
void     net_process_packet(uint8_t* data, uint32_t len);
void     net_poll(void);

/* ARP */
void     arp_init(void);
void     arp_process(arp_header_t* arp, uint32_t len);
int      arp_resolve(uint32_t ip, uint8_t* mac);
void     arp_send_request(uint32_t ip);

/* IP */
void     ip_init(void);
void     ip_process(ip_header_t* ip, uint32_t len);
int      ip_send(uint32_t dst_ip, uint8_t protocol, const void* data, uint32_t len);
uint16_t ip_checksum(const void* data, uint32_t len);

/* ICMP */
void     icmp_init(void);
void     icmp_process(ip_header_t* ip, uint32_t len);
int      icmp_ping(uint32_t ip);

/* UDP */
void     udp_init(void);
void     udp_process(ip_header_t* ip, uint32_t len);
int      udp_send(uint32_t dst_ip, uint16_t dst_port, uint16_t src_port, const void* data, uint32_t len);

/* TCP */
#define TCP_STATE_CLOSED      0
#define TCP_STATE_LISTEN      1
#define TCP_STATE_SYN_SENT    2
#define TCP_STATE_SYN_RCVD    3
#define TCP_STATE_ESTABLISHED 4
#define TCP_STATE_FIN_WAIT1   5
#define TCP_STATE_FIN_WAIT2   6
#define TCP_STATE_CLOSE_WAIT  7
#define TCP_STATE_CLOSING     8
#define TCP_STATE_LAST_ACK    9
#define TCP_STATE_TIME_WAIT   10

#define MAX_TCP_SOCKETS 16

typedef struct {
    uint8_t  state;
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;
    uint32_t seq;
    uint32_t ack;
    uint8_t  recv_buf[4096];
    uint32_t recv_len;
    bool     in_use;
} tcp_socket_t;

void     tcp_init(void);
void     tcp_process(ip_header_t* ip, uint32_t len);
int      tcp_connect(uint32_t ip, uint16_t port);
int      tcp_listen(uint16_t port);
int      tcp_send(int sock, const void* data, uint32_t len);
int      tcp_recv(int sock, void* buf, uint32_t max_len);
int      tcp_close(int sock);

/* Utility */
uint32_t net_make_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void     net_ip_to_str(uint32_t ip, char* str);
uint32_t net_str_to_ip(const char* str);
void     net_mac_to_str(const uint8_t* mac, char* str);

/* Global state */
extern nic_t* g_nic;

#endif
