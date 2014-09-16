#ifndef  __AC_EVENT_H__
#define  __AC_EVENT_H__
/**
 * @file ac_event.h
 * @author Angluca
 * @date 2012-12-26
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ac_function)(void*, ...);

struct ac_socket;
struct ac_event;

struct ac_socket* socket_listener_create(struct ac_event* event, int port, int backlog, void* recv_func, void* close_func);

struct ac_socket* socket_connecter_create(struct ac_event* event, char *ip, int port, void* recv_func, void* close_func);


struct ac_event* event_listener_create(int *out_fd, int port, int backlog, int max, void* recv_func, void* close_func);

int event_run(struct ac_event* event, int timeout);
struct ac_event* event_create(int max_socket /*, void* recv_func, void* close_func*/);
void event_close(struct ac_event* event);

unsigned char event_is_shutdown(struct ac_event* event);
void event_shutdown(struct ac_event* event);

/* logic function use them */
struct ac_socket* socket_get_handle(struct ac_event* event, int fd);
int socket_fd(struct ac_socket* sk);
void socket_close(struct ac_socket* sk);
void socket_close_fd(struct ac_event* event, int fd);

int socket_id(struct ac_socket* sk);
void socket_set_id(struct ac_socket* sk, int id);

struct ac_buffer;
struct ac_callback;
//#include "ac_callback.h"

//int socket_send(struct ac_socket *sk, struct ac_buffer *buffer, struct ac_callback *callback);
int socket_send(struct ac_socket *sk, uint16_t id, uint16_t result, struct ac_buffer *buffer, struct ac_callback *callback);
int socket_head_send(struct ac_socket *sk, uint16_t size, uint16_t id, uint16_t result, struct ac_callback *callback);


#ifdef __cplusplus
}
#endif

#endif  /*__AC_EVENT_H__*/
