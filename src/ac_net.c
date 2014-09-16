/**
 * @file ac_net.c
 * @author Angluca
 * @date 2012-12-26
 */
#include "ac_net.h"

#include <stdio.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include <netdb.h>
#include <assert.h>

#include "utils.h"

int	net_setnonblock(int	fd)
{
	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)|O_NONBLOCK) == -1) {
		return -1;
	}

	return 0;
}

void net_setaddress(struct sockaddr_in* addr, const char* ip, int port)
{
	assert(ip);
	bzero(addr,sizeof(struct sockaddr_in));
	addr->sin_family=AF_INET;
	inet_pton(AF_INET,ip,&(addr->sin_addr));
	addr->sin_port=htons(port);

	/*addr->sin_family=AF_INET;*/
	/*addr->sin_port=htons(port);*/
	/*addr->sin_addr.s_addr = inet_addr(ip);*/
	/*bzero(&(addr->sin_zero),8);*/
}

int	net_addr2ip(char* out_ip, struct sockaddr_in* addr)
{
	assert(out_ip);
	char ip[30];
	if(!inet_ntop(AF_INET,&(addr->sin_addr),ip,sizeof(ip))) {
		perror("fail to addr convert ip");
		return -1;
	}
	sprintf(out_ip,"%s:%d",ip,ntohs(addr->sin_port));
	return	0;
}
int	net_ip2addr(struct sockaddr_in* out_addr, char* ip)
{
	if(inet_pton(AF_INET,ip,&out_addr)<0) {
		perror("fail to ip convert addr");
		return -1;
	}
	return 0;
}

int	new_tcp_server(int port,struct sockaddr_in* paddr /* NULL */, int backlog )
{
	int	fd	=	socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(fd < 0) {
		return -1;
	}

	struct sockaddr_in	addr;
	struct sockaddr_in *p;
	if(paddr == NULL) p	=	&addr;
	else	p	=	paddr;

	int ok=1;
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&ok,sizeof(ok));

	if(net_setnonblock(fd) < 0)
	{
		close(fd);
		return	-1;
	}

	p->sin_family	=	AF_INET;
	p->sin_port	=	htons(port);
	p->sin_addr.s_addr = INADDR_ANY;

	if( bind(fd,(struct sockaddr*)p,sizeof(struct sockaddr)) == -1 )
	{
		ERROR_MSG("bind failed");
		return	-1;
	}

	if( listen(fd,backlog) == -1 )
	{
		ERROR_MSG("listen failed");
		return	-1;
	}

	return	fd;
}

#define TIMEOUT 4
int	new_tcp_client(const char* ip, int port, struct sockaddr_in* paddr /*= 0*/, int is_block)
{
	/*int	fd	=	socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);*/
	int	fd	=	socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in	addr;
	struct sockaddr_in *taddr = paddr;
	if(NULL == taddr) {
		taddr = &addr;
	}
	net_setaddress(taddr, ip ,port);

	if(!is_block) {
		if(net_setnonblock(fd) < 0)
		{
			close(fd);
			return	-1;
		}
	}

	struct timeval timeo = { TIMEOUT,0};
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
	int ret;
	if((ret = connect(fd,(struct sockaddr*)taddr, sizeof(struct sockaddr_in))) == -1)
	{
		if(errno != EINPROGRESS)
		{
			ERROR_MSG("connect failed");
			return	-1;
		}
	}
	/*usleep(1000);*/
    
//    fd_set set;
//    /*socklen_t len = sizeof(timeo);*/
//	if(ret!=0) {
//		FD_ZERO(&set);
//		FD_SET(fd, &set);
//		int tret = select(fd+1, NULL, &set, NULL, &timeo);
//		if(tret < 0) {
//			ERROR_MSG("select error");
//			return -1;
//		} else if(tret == 0) {
//			ERROR_MSG("select timeout");
//			return -1;
//		}
//
//		if( FD_ISSET( fd, &set))
//		{
//			int error = 0;
//			socklen_t len = sizeof( error);
//			if( getsockopt( fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
//			{
//				fprintf( stderr, "getsockopt fail,connected fail\n");
//				return -1;
//			}
//
//			if( error == ETIMEDOUT)
//			{
//				fprintf( stderr, "connecte timeout\n");
//				return -1;
//			}
//
//			if( error == 0) {
//
//			}
//			else
//			{
//				fprintf( stderr, "connect error.\n ");
//				return -1;
//			}
//		}
//	}

	return	fd;
}

int	net_accept(int listen_fd, struct sockaddr_in * addr, int is_block)
{
	int	clt_fd, errno_code;
	static	socklen_t	addr_len	=	sizeof(struct sockaddr_in);

	do {
		clt_fd	=	accept(listen_fd, (struct sockaddr*)addr, &addr_len);
		if(clt_fd > 0)	break;

		if(clt_fd < 0){
			errno_code	=	errno;
			if(errno_code == EINTR){
				continue;
			}else if(errno_code == EAGAIN || errno_code == EWOULDBLOCK){
				return	0;
			}else{
				ERROR_MSG("Failed to accept errno: %s",strerror(errno_code));
				return	-1;
			}

		}
	}while(1);

	if(!is_block) {
		if(net_setnonblock(clt_fd) < 0)
		{
			close(clt_fd);
			return	-1;
		}
	}

	return	clt_fd;
}

int	net_send(int* out_len, int fd, uint8_t* p, int n)
{
	char*	pbuf	=	(char*)p;
	int	ret;
	int	total	=	n;
	int	errno_code;
	while(total > 0)
	{
		ret	=	(int)send(fd,pbuf,total,0);
		if(ret > 0)
		{
			total	-=	ret;
			pbuf	+=	ret;
			continue;
		}
		/*if(ret	==	0){*/
			/*DEBUG_MSG("send close: %s", strerror(errno_code));*/
			/*return	0;*/
			/*//return	-1;*/
		/*}*/

		errno_code	=	errno;

		if(errno_code == EINTR)
			continue;
		if(errno_code  ==  EAGAIN ||  errno_code  ==  EWOULDBLOCK) {
			/*usleep(100);*/
			/*continue;*/
			break;
		}

		/*ERROR_MSG("fd:%d send errno:%d,%s, ret:%d",fd,errno_code,strerror(errno_code),ret);*/
		return	-1;
	}
	*out_len = n-total;
	return	0;
}

int	net_recv(int* out_len, int fd, uint8_t* p, int n)
{
	ssize_t	nread;
	size_t	nleft = n;
	char	*ptr = (char*)p;

	/*ptr = (char*)p;*/
	/*nleft = n;*/

	int		errno_code;

	while (nleft > 0) {
		nread = read(fd, ptr, nleft);

		if(nread > 0)
		{
			nleft -= nread;
			ptr   += nread;
			continue;
		}
		errno_code	=	errno;

		if (nread == 0){
			/*DEBUG_MSG("client close: %s", strerror(errno_code));*/
			/*return 0;				[> EOF <]*/
			return -1;				/* EOF */
			/*break;*/
		}


		if (errno_code  ==  EAGAIN  ||  errno_code  ==  EWOULDBLOCK) {
			break ;
		}

		if (errno_code == EINTR) {
			continue;		/* and call read() again */
		}

		/*ERROR_MSG("fd:%d recv errno:%d,%s, ret:%d",fd,errno_code,strerror(errno_code),(int)nread);*/
		return(-1);
	}
	*out_len = (int)n - (int)nleft;		/* return >= 0 */
	return 0;
}

int net_recvn(int fd, uint8_t *buf, int n)
{
	int nread, totnread, myerrno;

	if(n==0)
	{
		ERROR_MSG("readn: richiesti ZERO bytes\n");
		fflush(stderr);
		return(0);
	}
	if(n<0)
	{
		ERROR_MSG("readn failed: chiesto un numero NEGATIVO di bytes\n");
		fflush(stderr);
		return(-1);
	}
	totnread=0;
	while (totnread<n)
	{
		nread = (int)read(fd, buf+totnread, n-totnread );
		myerrno=errno;
		if(nread==0)
		{
			fprintf(stderr,"readn failed: connection closed from peer\n");
			fflush(stderr);
			return(-1);
		}
		else if(nread<0)
		{
			if(errno==EINTR) {
                /* nread = 0;  /\* and call read() again*/
                continue;
            }
			else
			{
				errno=myerrno;
				perror("read failed: ");
				errno=myerrno;
				return(-1);       /* error */
			}
		}
		else /* nread>0 */
		{
			totnread+=nread;
			if(totnread==n) return(n);
		}
	}
	return(n);
}

int net_sendn(int fd, uint8_t *buf, int n)
{
   size_t nleft;   ssize_t nwritten;   char *ptr;

   ptr = (char*) buf;  nleft = n;
   while (nleft > 0)
   {
     if ( (nwritten = send(fd, ptr, nleft, MSG_WAITALL)) <= 0)
		 {
        if(errno == EINTR)
						nwritten = 0;   /* and call write() again*/
        else
						return(-1);       /* error */
     }
     nleft -= nwritten;         ptr   += nwritten;
   }
   return(n);
}

int net_host2ip(const char* hostname, char* ip, int len)
{
	struct hostent *host;
	do {
		host = gethostbyname(hostname);
		if(0 == host) {
			if(TRY_AGAIN == h_errno) {
				/*usleep(1);*/
				continue;
			}
			return -1;
		} else break;
	}while(1);

	snprintf(ip, len, "%d.%d.%d.%d",
			( host->h_addr_list[0][0] & 0xff ),
			( host->h_addr_list[0][1] & 0xff ),
			( host->h_addr_list[0][2] & 0xff ),
			( host->h_addr_list[0][3] & 0xff ));
	return 0;
}

