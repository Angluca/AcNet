/**
 * @file ac_event.c
 * @author Angluca
 * @date 2012-12-26
 */
#include "ac_event.h"
#include "utils.h"
#include "ac_net.h"
#include "ac_buffer.h"
#include "ac_callback.h"
#include "ac_package.h"

#include "rwlock.h"

#ifdef __linux__
#define  _USE_EPOLL_ 0
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#define  _USE_KQUEUE_ 1
#else
#error("must unix, linux or mac os.");
#endif

#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/queue.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

#ifdef _USE_EPOLL_
#include <sys/epoll.h>
#elif _USE_KQUEUE_
#include <sys/event.h>
#include <time.h>
#endif

#include <assert.h>


#define  BACKLOG 32
#define  SOCKETS 64
#define  EVQUEUE 16

#define  SOCKET_INVALID 0
#define  SOCKET_CLOSE 1
#define  SOCKET_READ 2
#define  SOCKET_WRITE 4
#define  SOCKET_POLLIN 8
#define  SOCKET_POLLOUT 16

#define  SOCKET_BLOCKING 128
#define  MAX_BUFFER_SIZE 65534

#define  DEFAULT_SOCKETS_SIZE 64

int _connect_recv(struct ac_event* event, struct ac_socket* sk);
int _connect_send(struct ac_event* event, struct ac_socket* sk);

struct _inter_functions
{
	ac_function recv_func;
	ac_function send_func;
};

struct _custom_functions
{
	ac_function recv_func;
	ac_function close_func;
};

/*STAILQ_HEAD(_rb, ac_buffer);*/
STAILQ_HEAD(_rb, ac_package);

struct ac_socket
{
	int fd;
	int id;
	struct sockaddr_in addr;
	uint8_t status;
	struct ac_event *event;

	struct _inter_functions functions;

	struct ac_package* recv_package;
	struct _custom_functions custom_funcions;

	struct _rb rb;
	struct rwlock lock;
};

struct ac_event
{
	int evfd;
	uint8_t shut_down;
	uint16_t max_connection;
	struct ac_socket* sockets;
#ifdef _USE_EPOLL_
	struct epoll_event ev_queue[EVQUEUE];
#elif _USE_KQUEUE_
	struct kevent ev_queue[EVQUEUE];
#endif
};



#define   _ac_socket_size  sizeof(struct ac_socket)
#define   _ac_event_size  sizeof(struct ac_event)
#define   _ac_addr_size  sizeof(struct sockaddr_in)
#define   _ac_custom_functions_size  sizeof(struct _custom_functions)


static int _head_size = sizeof(struct ac_package_head);

uint8_t event_is_shutdown(struct ac_event* event)
{
	return event->shut_down;
}
void event_shutdown(struct ac_event* event)
{
	event->shut_down = 1;
}

static inline void _invalid_socket_flag(struct ac_socket* sk) {
	sk->status = SOCKET_INVALID;
}
static inline void _set_socket_flag(struct ac_socket* sk, uint8_t flag) {
	sk->status |= flag;
}
static inline void _del_socket_flag(struct ac_socket* sk, uint8_t flag) {
	sk->status &= ~flag;
}
/* -1:invalid 0:false 1:true */
static inline int _get_socket_flag(struct ac_socket* sk, uint8_t flag) {
	/* if(SOCKET_INVALID == sk->status) return SOCKET_INVALID; */
	return sk->status & flag?1:0;
}

static inline int _event_add(struct ac_event* event, struct ac_socket* sk, uint8_t flag)
{
    assert(sk);
	/* if(SOCKET_INVALID == sk->status) return -1; */

#ifdef _USE_EPOLL_
	struct epoll_event ev;
	ev.data.u64 = 0;
	uint32_t evflag = (flag & SOCKET_POLLIN ? EPOLLIN:0) |
		(flag & SOCKET_POLLOUT ? EPOLLOUT : 0);

	ev.events = evflag | EPOLLERR | EPOLLHUP;
	ev.data.fd = sk->fd;
	if(epoll_ctl(event->evfd, EPOLL_CTL_ADD, sk->fd, &ev) == -1) {
		ERROR_MSG("event add failed");
		return -1;
	}
#elif _USE_KQUEUE_
	struct kevent ev;
	/*short evflag = (flag & SOCKET_POLLIN ? EVFILT_READ : 0) |*/
	/*(flag & SOCKET_POLLOUT ? EVFILT_WRITE : 0);*/
	if(flag & SOCKET_POLLIN) {
		EV_SET(&ev, sk->fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ERROR, 0, 0, NULL);
		if(kevent(event->evfd, &ev, 1, NULL, 0, NULL) == -1) {
			ERROR_MSG("event add failed1");
			return -1;
		}
	}
	if(flag & SOCKET_POLLOUT) {
		EV_SET(&ev, sk->fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ERROR, 0, 0, NULL);
		if(kevent(event->evfd, &ev, 1, NULL, 0, NULL) == -1) {
			ERROR_MSG("event add failed2");
			return -1;
		}
	}
#endif
	if(flag & SOCKET_POLLIN) {
		_set_socket_flag(sk, SOCKET_POLLIN);
	} else {
		_del_socket_flag(sk, SOCKET_POLLIN);
	}
	if(flag & SOCKET_POLLOUT) {
		_set_socket_flag(sk, SOCKET_POLLOUT);
	} else {
		_del_socket_flag(sk, SOCKET_POLLOUT);
	}
	return 0;
}

static inline int _event_mod(struct ac_event* event, struct ac_socket* sk, uint8_t flag)
{
	/* if(SOCKET_INVALID == sk->status) return -1; */
#ifdef _USE_EPOLL_
	struct epoll_event ev;
	ev.data.u64 = 0;
	uint32_t evflag = (flag & SOCKET_POLLIN ? EPOLLIN:0) |
		(flag & SOCKET_POLLOUT ? EPOLLOUT : 0);

	ev.events = evflag | EPOLLERR | EPOLLHUP;
	ev.data.fd = sk->fd;
	if(epoll_ctl(event->evfd, EPOLL_CTL_MOD, sk->fd, &ev) == -1) {
		ERROR_MSG("event modify failed1");
        socket_close(sk);
		return -1;
	}
#elif _USE_KQUEUE_
	if(( sk->status & SOCKET_POLLIN ) || ( sk->status & SOCKET_POLLOUT )) {
		struct kevent ev;
		if(flag & SOCKET_POLLIN) {
			EV_SET(&ev, sk->fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
			if(kevent(event->evfd, &ev, 1, NULL, 0, NULL) == -1) {
				ERROR_MSG("event add failed");
				return -1;
			}
		} else {
			EV_SET(&ev, sk->fd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
			kevent(event->evfd, &ev, 1, NULL, 0, NULL);
		}

		if(flag & SOCKET_POLLOUT) {
			EV_SET(&ev, sk->fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
			if(kevent(event->evfd, &ev, 1, NULL, 0, NULL) == -1) {
				ERROR_MSG("event add failed");
				return -1;
			}
		} else {
			EV_SET(&ev, sk->fd, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
			kevent(event->evfd, &ev, 1, NULL, 0, NULL);
		}

		/*short evflag = (flag & SOCKET_POLLIN ? EVFILT_READ : 0) |*/
		/*(flag & SOCKET_POLLOUT ? EVFILT_WRITE : 0);*/
		/*EV_SET(&ev, sk->fd, evflag, EV_ADD | EV_ENABLE, 0, 0, NULL);*/
		/*if(kevent(event->evfd, &ev, 1, NULL, 0, NULL) == -1) {*/
			/*ERROR_MSG("event modify failed");*/
			/*return -1;*/
		/*}*/
	} else {
		ERROR_MSG("event modify failed2");
        socket_close(sk);
		return -1;
	}
#endif
	if(flag & SOCKET_POLLIN) {
		_set_socket_flag(sk, SOCKET_POLLIN);
	} else {
		_del_socket_flag(sk, SOCKET_POLLIN);
	}
	if(flag & SOCKET_POLLOUT) {
		_set_socket_flag(sk, SOCKET_POLLOUT);
	} else {
		_del_socket_flag(sk, SOCKET_POLLOUT);
	}
	return 0;
}

static inline int _event_del(struct ac_event* event, struct ac_socket*sk)
{
	/* if(SOCKET_INVALID == sk->status) return -1; */
#ifdef _USE_EPOLL_
	/*struct	epoll_event ev = {0};*/
	if(epoll_ctl(event->evfd, EPOLL_CTL_DEL, sk->fd, NULL) < 0) {
		ERROR_MSG("event del failed");
        socket_close(sk)
		return -1;
	}
#elif _USE_KQUEUE_
	struct kevent ev;
	/* EV_SET(&ev, sk->fd, EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0, NULL) */
	EV_SET(&ev, sk->fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	kevent(event->evfd, &ev, 1, NULL, 0, NULL);
	EV_SET(&ev, sk->fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(event->evfd, &ev, 1, NULL, 0, NULL);
#endif
	_del_socket_flag(sk, SOCKET_POLLIN | SOCKET_POLLOUT | SOCKET_READ | SOCKET_WRITE);
	/*_set_socket_flag(sk, SOCKET_CLOSE);*/
	return 0;
}

static inline struct ac_socket* _sockets_create(uint16_t max)
{
	struct ac_socket *sockets = (struct ac_socket*)calloc(1,max * _ac_socket_size);
	if(sockets == NULL) {
		EXIT_APP("memory allocate failed");
		return NULL;
	}
	int i;
	struct ac_buffer *buffer, *head;
	for(i=0; i<max; ++i) {
		sockets[i].fd = i;
		sockets[i].status = SOCKET_INVALID;

		head = buffer_alloc(_head_size);
		buffer = buffer_alloc(MAX_BUFFER_SIZE);
		sockets[i].recv_package = package_alloc(head, buffer);
		rwlock_init(&sockets[i].lock);
	}
	/*sockets[max-1].fd = -1;*/
	return sockets;
}

static inline struct ac_socket* _get_socket(struct ac_event* event, int fd)
{
	assert(event);
	if(( fd >= event->max_connection ) || (fd < 0)) {
		ERROR_MSG("client full? fd=%d", fd);
		return NULL;
	}
	assert(event->sockets);

	return &event->sockets[fd];
}

struct ac_socket* _socket_init(struct ac_event* event, int fd, struct sockaddr_in * addr, struct _custom_functions *cus_functions)
{
    assert(event);
    /* if(NULL == event) NULL; */
	struct ac_socket* sk = _get_socket(event, fd);
    if(NULL == sk) return NULL;
	sk->id = 0;
	sk->event = event;
	sk->status = SOCKET_INVALID;
	memcpy(&sk->addr, &addr, _ac_addr_size);
	sk->functions.send_func = (ac_function)_connect_send;
	sk->functions.recv_func = (ac_function)_connect_recv;

	memcpy(&sk->custom_funcions, cus_functions, _ac_custom_functions_size);
	/*sk->custom_funcions.recv_func = (ac_function) recv_func;*/
	/*sk->custom_funcions.close_func = (ac_function) close_func;*/
	{
		rwlock_wlock(&sk->lock);
		STAILQ_INIT(&sk->rb);
		rwlock_wunlock(&sk->lock);
		/*__sync_synchronize();*/
	}
	return sk;
}

static inline void _socket_close(struct ac_socket* sk) {

	rwlock_wlock(&sk->lock);

    if(sk->custom_funcions.close_func) {
        sk->custom_funcions.close_func(sk);
    }

	if(sk->status) {
		DEBUG_MSG("close socket %d", sk->fd);
		_event_del(sk->event, sk);
		_invalid_socket_flag(sk);
		int fd = sk->fd;
		shutdown(fd, SHUT_WR);
		close(fd);
		sk->id = 0;
		memset(&sk->addr, 0, _ac_addr_size);
		memset(&sk->custom_funcions, 0, _ac_custom_functions_size);
	}
	struct ac_package* p;
	struct _rb* rb = &sk->rb;
	{
		while((p = STAILQ_FIRST(rb))) {
			STAILQ_REMOVE_HEAD(rb,_link);
			package_free(p);
		}
		/*__sync_synchronize();*/
	}
	package_reset(sk->recv_package);
	rwlock_wunlock(&sk->lock);

	rwlock_init(&sk->lock);
}

struct ac_socket* socket_get_handle(struct ac_event* event, int fd)
{
	return _get_socket(event, fd);
}

int socket_fd(struct ac_socket* sk) {
	return sk->fd;
}

void socket_close(struct ac_socket* sk)
{
	assert(sk);
	_set_socket_flag(sk, SOCKET_CLOSE);
}
void socket_close_fd(struct ac_event *event, int fd)
{
	struct ac_socket *sk = _get_socket(event, fd);
	if(sk) {
		_set_socket_flag(sk, SOCKET_CLOSE);
	}
}
int socket_id(struct ac_socket* sk)
{
	return sk->id;
}
void socket_set_id(struct ac_socket* sk, int id)
{
	sk->id = id;
}

struct ac_event* event_create(int max_socket/*, void* recv_func, void* close_func*/)
{
	assert(max_socket > 0);

	uint16_t max = (uint16_t)max_socket;
	if(max < DEFAULT_SOCKETS_SIZE)
		max = DEFAULT_SOCKETS_SIZE;

	struct ac_socket *sockets = _sockets_create(max);
	struct ac_event *event = (struct ac_event*)malloc(_ac_event_size);
	memset(event, 0, _ac_event_size);
	if(NULL == sockets || NULL == event) {
		EXIT_APP("failed to allocate memory");
	}
#ifdef _USE_EPOLL_
	int evfd = epoll_create(max);
	if(evfd == -1) {
		ERROR_MSG("create epoll failed.");
        free(event);
		return NULL;
	}
#elif _USE_KQUEUE_
	int evfd = kqueue();
	if (evfd == -1) {
		ERROR_MSG("create kqueue failed.");
        free(event);
		return NULL;
	}
#endif
	event->evfd = evfd;

	event->sockets = sockets;
	event->max_connection = max;
	/*event->custom_funcions.recv_func = (ac_function)recv_func;*/
	/*event->custom_funcions.close_func = (ac_function)close_func;*/
	/*memset(event->ev_queue, 0, EVQUEUE*sizeof());*/
	int i=0;
	for(; i<max; ++i) {
		sockets[i].event = event;
	}

	return event;
}

void event_close(struct ac_event *event)
{
	if(!event) return;
	struct ac_socket *sockets = event->sockets;
	int i, max = event->max_connection;
	if(sockets) {
		for(i=1; i<max; ++i) {
			_socket_close(&sockets[i]);
			package_free(sockets[i].recv_package);
		}
		free(sockets);
		event->sockets = NULL;
	}
	if(event->evfd) {
		close(event->evfd);
		event->evfd = 0;
	}
	free(event);
}


int _listener_accept(struct ac_event* event, struct ac_socket* sk)
{
	/*DEBUG_MSG("accept join");*/
	struct sockaddr_in addr;
	int fd = net_accept(sk->fd, &addr, 0);
	if(fd < 0) {
		return -1;
	}
	struct ac_socket * psocket = _socket_init(event, fd, &addr, &sk->custom_funcions);
	if(_event_add(event, psocket, SOCKET_POLLIN) < 0) {
		_socket_close(psocket);
	}

	return 0;
}

int _connect_recv(struct ac_event* event, struct ac_socket* sk)
{
//	DEBUG_MSG("connect_recv");
	struct ac_package *package = sk->recv_package;
	struct ac_buffer *buffer;
	uint8_t *p;
	int check = 0;
	int ok;
	int out_remain;
	int outlen;

	while((buffer = package_write_complete(package, &check))) {
		if(buffer->size == MAX_BUFFER_SIZE) {
			/* recv content */
			struct ac_package_head *phead = (struct ac_package_head*)package->head->buf;
			int pack_size = package_size(phead);
			p = buffer_write_content(buffer, &out_remain, pack_size);
			/*if(out_remain < 0) return -1;*/

			DEBUG_MSG("recv fd:%d head size:%d, id:0x%x, ret:%d", sk->fd, package_size(phead), phead->id, phead->result);
		} else {
			/* recv head */
			p = buffer_write_content(buffer, &out_remain, _head_size);
		}
		if(out_remain < 1) {
			assert(out_remain > -1);
			continue;
		}
		ok = net_recv(&outlen, sk->fd, p, out_remain);
		if(ok < 0) {
			return -1;
		}
		buffer_increase_write_index(buffer, outlen);
		if(outlen < out_remain) break;
	}

	switch(check) {
		case 0: /* again*/
        {
            _set_socket_flag(sk, SOCKET_READ);
			int modRet = _event_mod(event, sk, SOCKET_POLLIN);
            if(modRet < 0) {
                socket_close(sk);
            }
        }
			return 0;
		case 1: /* ok */
			{
				struct ac_package_head *head = (struct ac_package_head*)package->head->buf;
				/*DEBUG_MSG("fd:%d head size:%d, id:0x%x, ret:%d, buf:%s", sk->fd, package_size(head), head->id, head->result, package->buffer->buf);*/

				if(sk->custom_funcions.recv_func) {
					int ret = sk->custom_funcions.recv_func(sk, head, package->buffer);
					if(ret < 0) return -1;
				}
				_del_socket_flag(sk, SOCKET_READ);
				package_reset(package);

				/*if(STAILQ_FIRST(&sk->rb))*/
					/*_event_mod(event, sk, SOCKET_POLLOUT);*/
				/*else*/
					/*_event_mod(event, sk, SOCKET_POLLIN);*/
			}
			return 0;
		default: /* error */
				_del_socket_flag(sk, SOCKET_READ);
			return -1;
	}

	ERROR_MSG("recv not run this row.");
	return 0;
}

int _connect_send(struct ac_event* event, struct ac_socket* sk)
{
//	DEBUG_MSG("connect_send");
	/*DEBUG_MSG("send join");*/

	/* send */
	struct ac_package *package;
	struct ac_buffer* buffer;
	unsigned char *p;

	int out_remain = 0;
	int ok;
	int outlen = 0;
	int check = 0;

	while(1) {
		rwlock_rlock(&sk->lock);
		package = STAILQ_FIRST(&sk->rb);
		rwlock_runlock(&sk->lock);
		if(NULL == package) {
			break;
		}
		while((buffer = package_read_complete(package, &check))) {
			p = buffer_read_content(buffer, &out_remain);
			if(out_remain < 1) {
				assert(out_remain > -1);
				continue;
			}
			ok = net_send(&outlen, sk->fd, p, out_remain);

			if(ok < 0) {
				return -1;
			}
			buffer_increase_read_index(buffer, outlen);
			if(outlen < out_remain) break;
		}

		switch(check) {
			case 0:
            {
                _set_socket_flag(sk, SOCKET_WRITE);
                if(_event_mod(event, sk, SOCKET_POLLOUT) < 0) {
                    socket_close(sk);
                }
            }
            return 0;
				/*break;*/
			case 1:
				{

					struct ac_package_head *head = (struct ac_package_head*)package->head->buf;
					DEBUG_MSG("send fd:%d head size:%d, id:0x%x, ret:%d", sk->fd, package_size(head), head->id, head->result);
					if(package_callback(package, 0) < 0) return -1;

					{
						rwlock_wlock(&sk->lock);
						STAILQ_REMOVE_HEAD(&sk->rb,_link);
						rwlock_wunlock(&sk->lock);
						/*__sync_synchronize();*/
					}
					package_free(package);
					_del_socket_flag(sk, SOCKET_WRITE);

					/*if(STAILQ_FIRST(&sk->rb))*/
						/*_event_mod(event, sk, SOCKET_POLLOUT);*/
					/*else*/
						/*_event_mod(event, sk, SOCKET_POLLIN);*/
				}
				return 0;
			default:
				/*package_callback(package, -1);*/
				_del_socket_flag(sk, SOCKET_WRITE);
				return -1;
		}
	}

	/*if(STAILQ_FIRST(&sk->rb))*/
		/*_event_mod(event, sk, SOCKET_POLLOUT);*/
	/*else*/
		/*_event_mod(event, sk, SOCKET_POLLIN);*/
	return 0;
}

struct ac_socket* socket_listener_create(struct ac_event* event, int port, int backlog, void* recv_func, void* close_func)
{
	assert(event);
	struct sockaddr_in addr = {0};
	int fd = new_tcp_server(port, &addr, backlog);
	if(fd < 0) {
		ERROR_MSG("create server socket failed.");
		return NULL;
	}
	struct _custom_functions funcs;
	funcs.recv_func = (ac_function)recv_func;
	funcs.close_func = (ac_function)close_func;
	struct ac_socket* sk = _socket_init(event, fd, &addr, &funcs);
	if(!sk) return NULL;
	sk->functions.recv_func = (ac_function)_listener_accept;

	sk->custom_funcions.recv_func = (ac_function)recv_func;
	sk->custom_funcions.close_func = (ac_function)close_func;

	if(_event_add(event, sk, SOCKET_POLLIN) < 0) {
		_socket_close(sk);
		return NULL;
	}
	return sk;
}

struct ac_socket* socket_connecter_create(struct ac_event* event, char *ip, int port, void* recv_func, void* close_func)
{
	assert(event);
	struct sockaddr_in addr = {0};
	int fd = new_tcp_client(ip, port, &addr, 0);
	if(fd < 1) {
		ERROR_MSG("create client socket failed.");
		return NULL;
	}
	struct _custom_functions funcs;
	funcs.recv_func = (ac_function)recv_func;
	funcs.close_func = (ac_function)close_func;
	struct ac_socket* sk = _socket_init(event, fd, &addr, &funcs);

    if(NULL == sk) return NULL;
	sk->custom_funcions.recv_func = (ac_function)recv_func;
	sk->custom_funcions.close_func = (ac_function)close_func;

	if(_event_add(event, sk, SOCKET_POLLOUT) < 0) {
		_socket_close(sk);
		return NULL;
	}
	return sk;
}

struct ac_event* event_listener_create(int *out_fd, int port, int backlog, int max, void* recv_func, void* close_func)
{
	struct ac_event* event = event_create(max);
	if(!event) return NULL;
	struct ac_socket* sk = socket_listener_create(event, port, backlog, recv_func, close_func);
	if(!sk) {
		event_close(event);
        return NULL;
	}
	if(out_fd)
		*out_fd = sk->fd;
	return event;
}

int event_run(struct ac_event* event, int timeout)
{
	assert(event);
	if(event->shut_down) return 0;
#ifdef _USE_EPOLL_
	struct epoll_event *ev_queue = event->ev_queue;
	int nfds = epoll_wait(event->evfd, ev_queue, EVQUEUE, timeout);
#elif _USE_KQUEUE_
	struct kevent *ev_queue = event->ev_queue;
	int nfds = 0;
	if(timeout == -1) {
		nfds = kevent(event->evfd, NULL, 0 , ev_queue, EVQUEUE, NULL);
	} else {
		struct timespec timeoutspec;
		timeoutspec.tv_sec = timeout / 1000;
		timeoutspec.tv_nsec = ( timeout % 1000 ) * 1000000;
		nfds = kevent(event->evfd, NULL, 0, ev_queue, EVQUEUE, &timeoutspec);
	}
#endif
//	DEBUG_MSG("nfds:%d", nfds);
	if(nfds < 0) {
		if(errno == EINTR) {
			return 0;
		}
		return -1;
	}
	int i, fd;
	struct ac_socket *psocket;

	for(i=0; i<nfds; ++i) {
#ifdef _USE_EPOLL_
		fd = ev_queue[i].data.fd;
#elif _USE_KQUEUE_
		fd = (int)ev_queue[i].ident;
#endif
		psocket = _get_socket(event, fd);
		if(NULL == psocket) {
			continue;
		}

#ifdef _USE_EPOLL_
		switch(ev_queue[i].events) {
			case EPOLLIN: case EPOLLOUT: break;
			default:
				_set_socket_flag(psocket, SOCKET_CLOSE);
				continue;
		}
#elif _USE_KQUEUE_
		/*events = *( (int*)&ev_queue[i].filter );*/
		if(ev_queue[i].flags & EV_ERROR) {
			_set_socket_flag(psocket, SOCKET_CLOSE);
			continue;
		}
#endif

		/*if(!psocket->status && _get_socket_flag(psocket, SOCKET_CLOSE)) {*/
			/*DEBUG_MSG("socket closed or invalid");*/
			/*continue;*/
		/*}*/

		if(_get_socket_flag(psocket, SOCKET_POLLIN)) {
			if(psocket->functions.recv_func(event, psocket) < 0) {
					_set_socket_flag(psocket, SOCKET_CLOSE);
					continue;
			}
		}
		/*else */if(_get_socket_flag(psocket, SOCKET_POLLOUT | SOCKET_WRITE)) {
			if(psocket->functions.send_func(event, psocket) < 0) {
					_set_socket_flag(psocket, SOCKET_CLOSE);
					continue;
			}
		}
	}

	for(i=0; i<nfds; ++i) {
#ifdef _USE_EPOLL_
        psocket = _get_socket(event, ev_queue[i].data.fd);
#elif _USE_KQUEUE_
        psocket = _get_socket(event, (int)ev_queue[i].ident);
#endif
        assert(psocket);
		if(SOCKET_INVALID != psocket->status && _get_socket_flag(psocket, SOCKET_CLOSE)) {
			_socket_close(psocket);
		} else {
			{
				rwlock_rlock(&psocket->lock);
				long pl = (long) STAILQ_FIRST(&psocket->rb);
				rwlock_runlock(&psocket->lock);

				if(pl) {
					if(_event_mod(event, psocket, SOCKET_POLLOUT) < 0) {
						/* _socket_close has lock, so...*/
                        _socket_close(psocket);
                    }
				}
				else {
					if(_event_mod(event, psocket, SOCKET_POLLIN) < 0) {
                        _socket_close(psocket);
                    }
				}
				/*rwlock_runlock(&psocket->lock);*/
				/*__sync_synchronize();*/
			}
		}
	}
	return nfds;
}


int socket_send(struct ac_socket *sk, uint16_t id, uint16_t result, struct ac_buffer *buffer, struct ac_callback *callback)
{
	assert(buffer);
	if(SOCKET_INVALID == sk->status) return -1;
	struct ac_package_head head;
	head.id = id;
	head.result = result;
	/*head.size = htons(buffer->write_offset);*/
	head.size = buffer->write_offset;

	struct ac_buffer *head_buf = buffer_alloc(_head_size);
	if(buffer_copy(head_buf, head.buf, _head_size) < 0) {
		buffer_free(head_buf);
		return -1;
	}
	struct ac_package* package = package_alloc(head_buf, buffer);
	package_set_send_callback(package, callback);
	{
		rwlock_wlock(&sk->lock);
		STAILQ_INSERT_TAIL(&sk->rb, package, _link);
		rwlock_wunlock(&sk->lock);
		/*__sync_synchronize();*/
	}

	/*if(sk->status & (SOCKET_POLLOUT | SOCKET_POLLIN))*/
		int ret = _event_mod(sk->event, sk, SOCKET_POLLOUT);
    if(ret < 0) {
        socket_close(sk);
    }
    return ret;
	/*else*/
		/*return _event_mod(sk->event, sk, SOCKET_POLLOUT);*/
	/*return 0;*/
}

int socket_head_send(struct ac_socket *sk, uint16_t size, uint16_t id, uint16_t result, struct ac_callback *callback)
{
	if(SOCKET_INVALID == sk->status) return -1;
	struct ac_package_head head;
	head.id = id;
	head.result = result;
	head.size = size;

	struct ac_buffer *head_buf = buffer_alloc(_head_size);
	if(buffer_copy(head_buf, head.buf, _head_size) < 0) {
		buffer_free(head_buf);
		return -1;
	}
	struct ac_package* package = package_alloc(head_buf, NULL);
	package_set_send_callback(package, callback);
	{
		rwlock_wlock(&sk->lock);
		STAILQ_INSERT_TAIL(&sk->rb, package, _link);
		rwlock_wunlock(&sk->lock);
		/*__sync_synchronize();*/
	}
	int ret = _event_mod(sk->event, sk, SOCKET_POLLOUT);
    if(ret < 0) {
        socket_close(sk);
    }
    return ret;
}

