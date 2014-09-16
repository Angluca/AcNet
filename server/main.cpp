#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "ac_event.h"
#include "ac_buffer.h"
#include "ac_package.h"

static	void	sighandler(int signal)
{
	/* server exit */
	exit(1);
}
static	void	set_signal()
{
	sigset_t	sigset;
	bzero(&sigset,sizeof(sigset_t));

	sigemptyset(&sigset);
	struct	sigaction	siginfo;
	bzero(&siginfo,sizeof(siginfo));
	siginfo.sa_handler	=	sighandler;
	siginfo.sa_mask	=	sigset;
	siginfo.sa_flags	=	SA_RESTART;

	signal(SIGPIPE, SIG_IGN);	//dont auto close

	sigaction(SIGINT, &siginfo, NULL);
	sigaction(SIGTERM, &siginfo, NULL);
}

#include "protocol_def.h"
#define  MAXSIZE 1024
static int svr_recv(struct ac_socket *sk, struct ac_package_head *head, struct ac_buffer *buffer)
{

	struct ac_buffer *buff = buffer_alloc(MAXSIZE);
	const char *buf = "I'm server";
	buffer_copy(buff, (unsigned char*)buf, (int)strlen(buf));
	socket_send(sk, MSG_LOGIN, 10 , buff, NULL);
	/*printf("clt_recv: fd:%d, buf:%s\n", socket_fd(sk), buffer->buf);*/
	return 0;
}
static int svr_close(struct ac_socket *sk)
{
	printf("server fd:%d close=================================\n", socket_fd(sk));
	return 0;
}

int main(int argc, char *argv[])
{
	set_signal();
	struct ac_event * event = event_listener_create(NULL, 4444, 32, 32, (void*)svr_recv, (void*)svr_close);
	if(!event) {
		printf("create event failed\n");
	}
	while(!event_is_shutdown(event)) {
		event_run(event, -1);
	}
	return 0;
}
