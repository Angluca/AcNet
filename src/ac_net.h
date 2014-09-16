#ifndef  __AC_NET_H__
#define  __AC_NET_H__
/**
 * @file ac_net.h
 * @author Angluca
 * @date 2012-12-26
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sockaddr_in;

int new_tcp_server(int port, struct sockaddr_in* out_paddr, int backlog);
int new_tcp_client(const char* ip, int port, struct sockaddr_in* out_paddr, int is_block);

int net_addr2ip(char* out_str, struct sockaddr_in* addr);
int net_ip2addr(struct sockaddr_in* out_addr, char* str);
int net_setnonblock(int fd);
void net_setaddress(struct sockaddr_in* out_addr, const char* ip, int port);
int net_host2ip(const char* hostname, char* ip, int len);

int net_accept(int listen_fd, struct sockaddr_in * addr, int is_block);
int net_send(int *out_len, int fd, uint8_t* p, int n);
int net_recv(int *out_len, int fd, uint8_t* p, int n);

int net_sendn(int fd, uint8_t* buf, int n);
int net_recvn(int fd, uint8_t* buf, int n);

#ifdef __cplusplus
}
#endif

#endif  /*__AC_NET_H__*/
