#include "../include/net.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../include/stdio.h"
#include "../include/timer.h"

/* Global NIC */
nic_t* g_nic = NULL;

/* Static NIC (simulated for QEMU's default networking) */
static nic_t virtual_nic;
static arp_cache_entry_t arp_cache[ARP_CACHE_SIZE];
static tcp_socket_t tcp_sockets[MAX_TCP_SOCKETS];

/* ==============================================================================
 * Utility Functions
 * ============================================================================== */

uint32_t net_make_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d;
}

void net_ip_to_str(uint32_t ip, char* str) {
    sprintf(str, "%d.%d.%d.%d",
            (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF, ip & 0xFF);
}

uint32_t net_str_to_ip(const char* str) {
    uint8_t parts[4] = {0};
    int idx = 0;
    while (*str && idx < 4) {
        while (*str >= '0' && *str <= '9') {
            parts[idx] = parts[idx] * 10 + (*str - '0');
            str++;
        }
        idx++;
        if (*str == '.') str++;
    }
    return net_make_ip(parts[0], parts[1], parts[2], parts[3]);
}

void net_mac_to_str(const uint8_t* mac, char* str) {
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

uint16_t ip_checksum(const void* data, uint32_t len) {
    const uint16_t* d = (const uint16_t*)data;
    uint32_t sum = 0;
    while (len > 1) {
        sum += *d++;
        len -= 2;
    }
    if (len == 1) {
        sum += *(uint8_t*)d;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return ~sum;
}

static uint16_t htons(uint16_t v) {
    return ((v & 0xFF) << 8) | ((v >> 8) & 0xFF);
}

static uint32_t htonl(uint32_t v) {
    return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) |
           ((v >> 8) & 0xFF00) | ((v >> 24) & 0xFF);
}

/* ==============================================================================
 * ARP Implementation
 * ============================================================================== */

void arp_init(void) {
    memset(arp_cache, 0, sizeof(arp_cache));
}

void arp_process(arp_header_t* arp, uint32_t len UNUSED) {
    uint16_t op = htons(arp->op);

    if (op == ARP_OP_REQUEST) {
        /* Check if it's for our IP */
        if (htonl(arp->dst_ip) == g_nic->ip) {
            /* Send ARP reply */
            uint8_t reply[42];
            eth_header_t* eth = (eth_header_t*)reply;
            memcpy(eth->dst_mac, arp->src_mac, ETH_ALEN);
            memcpy(eth->src_mac, g_nic->mac, ETH_ALEN);
            eth->type = htons(ETH_TYPE_ARP);

            arp_header_t* rarp = (arp_header_t*)(reply + 14);
            rarp->htype = htons(ARP_HTYPE_ETH);
            rarp->ptype = htons(ARP_PTYPE_IP);
            rarp->hlen = ETH_ALEN;
            rarp->plen = 4;
            rarp->op = htons(ARP_OP_REPLY);
            memcpy(rarp->src_mac, g_nic->mac, ETH_ALEN);
            rarp->src_ip = htonl(g_nic->ip);
            memcpy(rarp->dst_mac, arp->src_mac, ETH_ALEN);
            rarp->dst_ip = arp->src_ip;

            if (g_nic->send) g_nic->send(reply, 42);
        }
    } else if (op == ARP_OP_REPLY) {
        /* Add to ARP cache */
        uint32_t src_ip = htonl(arp->src_ip);
        for (int i = 0; i < ARP_CACHE_SIZE; i++) {
            if (!arp_cache[i].valid || arp_cache[i].ip == src_ip) {
                arp_cache[i].ip = src_ip;
                memcpy(arp_cache[i].mac, arp->src_mac, ETH_ALEN);
                arp_cache[i].valid = true;
                arp_cache[i].timestamp = timer_get_ticks();
                break;
            }
        }
    }
}

int arp_resolve(uint32_t ip, uint8_t* mac) {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip) {
            memcpy(mac, arp_cache[i].mac, ETH_ALEN);
            return 0;
        }
    }
    /* Send ARP request */
    arp_send_request(ip);
    return -1;
}

void arp_send_request(uint32_t ip) {
    uint8_t packet[42];
    eth_header_t* eth = (eth_header_t*)packet;
    memset(eth->dst_mac, 0xFF, ETH_ALEN);  /* Broadcast */
    memcpy(eth->src_mac, g_nic->mac, ETH_ALEN);
    eth->type = htons(ETH_TYPE_ARP);

    arp_header_t* arp = (arp_header_t*)(packet + 14);
    arp->htype = htons(ARP_HTYPE_ETH);
    arp->ptype = htons(ARP_PTYPE_IP);
    arp->hlen = ETH_ALEN;
    arp->plen = 4;
    arp->op = htons(ARP_OP_REQUEST);
    memcpy(arp->src_mac, g_nic->mac, ETH_ALEN);
    arp->src_ip = htonl(g_nic->ip);
    memset(arp->dst_mac, 0x00, ETH_ALEN);
    arp->dst_ip = htonl(ip);

    if (g_nic->send) g_nic->send(packet, 42);
}

/* ==============================================================================
 * IP Implementation
 * ============================================================================== */

void ip_init(void) {
    /* Nothing to initialize */
}

void ip_process(ip_header_t* ip, uint32_t len) {
    /* Check if packet is for us */
    uint32_t dst = htonl(ip->dst_ip);
    if (dst != g_nic->ip && dst != 0xFFFFFFFF) return;

    switch (ip->protocol) {
        case IP_PROTOCOL_ICMP:
            icmp_process(ip, len - (ip->version_ihl & 0x0F) * 4);
            break;
        case IP_PROTOCOL_TCP:
            tcp_process(ip, len - (ip->version_ihl & 0x0F) * 4);
            break;
        case IP_PROTOCOL_UDP:
            udp_process(ip, len - (ip->version_ihl & 0x0F) * 4);
            break;
    }
}

int ip_send(uint32_t dst_ip, uint8_t protocol, const void* data, uint32_t len) {
    uint8_t packet[1518];
    eth_header_t* eth = (eth_header_t*)packet;

    /* Resolve MAC address */
    uint8_t dst_mac[ETH_ALEN];
    if (arp_resolve(dst_ip, dst_mac) < 0) {
        /* Default to gateway MAC (simulated) */
        memset(dst_mac, 0x52, ETH_ALEN);
        dst_mac[5] = 0x02;
    }
    memcpy(eth->dst_mac, dst_mac, ETH_ALEN);
    memcpy(eth->src_mac, g_nic->mac, ETH_ALEN);
    eth->type = htons(ETH_TYPE_IP);

    ip_header_t* ip_hdr = (ip_header_t*)(packet + 14);
    ip_hdr->version_ihl = 0x45;  /* IPv4, 20 byte header */
    ip_hdr->tos = 0;
    ip_hdr->total_len = htons(20 + len);
    ip_hdr->id = htons(0x1234);
    ip_hdr->flags_offset = 0;
    ip_hdr->ttl = 64;
    ip_hdr->protocol = protocol;
    ip_hdr->checksum = 0;
    ip_hdr->src_ip = htonl(g_nic->ip);
    ip_hdr->dst_ip = htonl(dst_ip);
    ip_hdr->checksum = ip_checksum(ip_hdr, 20);

    memcpy(packet + 14 + 20, data, len);

    if (g_nic->send) return g_nic->send(packet, 14 + 20 + len);
    return -1;
}

/* ==============================================================================
 * ICMP Implementation
 * ============================================================================== */

void icmp_init(void) {
    /* Nothing to initialize */
}

void icmp_process(ip_header_t* ip, uint32_t len) {
    icmp_header_t* icmp = (icmp_header_t*)((uint8_t*)ip + ((ip->version_ihl & 0x0F) * 4));

    if (icmp->type == ICMP_ECHO_REQUEST) {
        /* Reply to ping */
        uint8_t reply[1500];
        memcpy(reply, icmp, len);
        icmp_header_t* r_icmp = (icmp_header_t*)reply;
        r_icmp->type = ICMP_ECHO_REPLY;
        r_icmp->checksum = 0;
        r_icmp->checksum = ip_checksum(r_icmp, len);

        ip_send(htonl(ip->src_ip), IP_PROTOCOL_ICMP, reply, len);
    }
}

int icmp_ping(uint32_t ip) {
    uint8_t packet[64];
    icmp_header_t* icmp = (icmp_header_t*)packet;
    icmp->type = ICMP_ECHO_REQUEST;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->id = htons(0x1234);
    icmp->seq = htons(1);
    memset(packet + 8, 0xAA, 56);
    icmp->checksum = ip_checksum(packet, 64);

    return ip_send(ip, IP_PROTOCOL_ICMP, packet, 64);
}

/* ==============================================================================
 * UDP Implementation
 * ============================================================================== */

void udp_init(void) {
    /* Nothing to initialize */
}

void udp_process(ip_header_t* ip, uint32_t len) {
    udp_header_t* udp = (udp_header_t*)((uint8_t*)ip + ((ip->version_ihl & 0x0F) * 4));
    uint16_t dst_port = htons(udp->dst_port);

    /* Check for listening sockets, etc. */
    (void)dst_port;
    (void)len;
}

int udp_send(uint32_t dst_ip, uint16_t dst_port, uint16_t src_port, const void* data, uint32_t len) {
    uint8_t packet[1500];
    udp_header_t* udp = (udp_header_t*)packet;
    udp->src_port = htons(src_port);
    udp->dst_port = htons(dst_port);
    udp->length = htons(8 + len);
    udp->checksum = 0;
    memcpy(packet + 8, data, len);

    return ip_send(dst_ip, IP_PROTOCOL_UDP, packet, 8 + len);
}

/* ==============================================================================
 * TCP Implementation (Simplified)
 * ============================================================================== */

void tcp_init(void) {
    memset(tcp_sockets, 0, sizeof(tcp_sockets));
}

void tcp_process(ip_header_t* ip, uint32_t len) {
    tcp_header_t* tcp = (tcp_header_t*)((uint8_t*)ip + ((ip->version_ihl & 0x0F) * 4));
    uint16_t dst_port = htons(tcp->dst_port);
    uint16_t src_port = htons(tcp->src_port);
    uint32_t src_ip = htonl(ip->src_ip);

    /* Find matching socket */
    for (int i = 0; i < MAX_TCP_SOCKETS; i++) {
        if (!tcp_sockets[i].in_use) continue;
        if (tcp_sockets[i].local_port == dst_port) {
            if (tcp_sockets[i].state == TCP_STATE_LISTEN) {
                /* SYN received - accept connection */
                if (tcp->flags & TCP_SYN) {
                    tcp_sockets[i].remote_ip = src_ip;
                    tcp_sockets[i].remote_port = src_port;
                    tcp_sockets[i].ack = htonl(tcp->seq_num) + 1;
                    tcp_sockets[i].seq = 1000;  /* Random ISN */
                    tcp_sockets[i].state = TCP_STATE_SYN_RCVD;
                    /* Would send SYN-ACK here */
                }
            }
            break;
        }
    }
    (void)len;
}

int tcp_connect(uint32_t ip, uint16_t port) {
    for (int i = 0; i < MAX_TCP_SOCKETS; i++) {
        if (!tcp_sockets[i].in_use) {
            tcp_sockets[i].in_use = true;
            tcp_sockets[i].state = TCP_STATE_SYN_SENT;
            tcp_sockets[i].remote_ip = ip;
            tcp_sockets[i].remote_port = port;
            tcp_sockets[i].local_port = 1024 + (timer_get_ticks() % 60000);
            tcp_sockets[i].seq = timer_get_ticks();
            return i;
        }
    }
    return -1;
}

int tcp_listen(uint16_t port) {
    for (int i = 0; i < MAX_TCP_SOCKETS; i++) {
        if (!tcp_sockets[i].in_use) {
            tcp_sockets[i].in_use = true;
            tcp_sockets[i].state = TCP_STATE_LISTEN;
            tcp_sockets[i].local_port = port;
            return i;
        }
    }
    return -1;
}

int tcp_send(int sock, const void* data, uint32_t len) {
    if (sock < 0 || sock >= MAX_TCP_SOCKETS) return -1;
    if (tcp_sockets[sock].state != TCP_STATE_ESTABLISHED) return -1;

    /* Would segment and send data */
    tcp_sockets[sock].seq += len;
    return len;
}

int tcp_recv(int sock, void* buf, uint32_t max_len) {
    if (sock < 0 || sock >= MAX_TCP_SOCKETS) return -1;
    tcp_socket_t* s = &tcp_sockets[sock];
    if (s->recv_len == 0) return 0;
    uint32_t n = (max_len < s->recv_len) ? max_len : s->recv_len;
    memcpy(buf, s->recv_buf, n);
    s->recv_len -= n;
    if (s->recv_len > 0) memmove(s->recv_buf, s->recv_buf + n, s->recv_len);
    return n;
}

int tcp_close(int sock) {
    if (sock < 0 || sock >= MAX_TCP_SOCKETS) return -1;
    tcp_sockets[sock].state = TCP_STATE_CLOSED;
    tcp_sockets[sock].in_use = false;
    return 0;
}

/* ==============================================================================
 * Network Stack Init
 * ============================================================================== */

void net_init(void) {
    /* Initialize virtual NIC */
    memset(&virtual_nic, 0, sizeof(nic_t));
    strcpy(virtual_nic.name, "eth0");
    virtual_nic.mac[0] = 0x52;
    virtual_nic.mac[1] = 0x54;
    virtual_nic.mac[2] = 0x00;
    virtual_nic.mac[3] = 0x12;
    virtual_nic.mac[4] = 0x34;
    virtual_nic.mac[5] = 0x56;
    virtual_nic.ip = net_make_ip(10, 0, 2, 15);
    virtual_nic.netmask = net_make_ip(255, 255, 255, 0);
    virtual_nic.gateway = net_make_ip(10, 0, 2, 2);
    virtual_nic.link_up = true;
    virtual_nic.send = NULL;  /* No real NIC driver */
    virtual_nic.recv = NULL;
    g_nic = &virtual_nic;

    arp_init();
    ip_init();
    icmp_init();
    udp_init();
    tcp_init();
}

void net_set_ip(uint32_t ip, uint32_t netmask, uint32_t gateway) {
    if (g_nic) {
        g_nic->ip = ip;
        g_nic->netmask = netmask;
        g_nic->gateway = gateway;
    }
}

void net_process_packet(uint8_t* data, uint32_t len) {
    if (len < 14) return;
    eth_header_t* eth = (eth_header_t*)data;

    uint16_t type = htons(eth->type);
    switch (type) {
        case ETH_TYPE_ARP:
            arp_process((arp_header_t*)(data + 14), len - 14);
            break;
        case ETH_TYPE_IP:
            ip_process((ip_header_t*)(data + 14), len - 14);
            break;
    }
}

void net_poll(void) {
    /* Would poll NIC for received packets */
}
