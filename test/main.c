#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "ac_event.h"

/*static	void	sighandler(int signal)*/
/*{*/
	/*[> server exit <]*/
	/*exit(1);*/
/*}*/
/*static	void	set_signal()*/
/*{*/
	/*sigset_t	sigset;*/
	/*bzero(&sigset,sizeof(sigset_t));*/

	/*sigemptyset(&sigset);*/
	/*struct	sigaction	siginfo;*/
	/*bzero(&siginfo,sizeof(siginfo));*/
	/*siginfo.sa_handler	=	sighandler;*/
	/*siginfo.sa_mask	=	sigset;*/
	/*siginfo.sa_flags	=	SA_RESTART;*/

	/*signal(SIGPIPE, SIG_IGN);	//dont auto close*/

	/*sigaction(SIGINT, &siginfo, NULL);*/
	/*sigaction(SIGTERM, &siginfo, NULL);*/
/*}*/

/*#include "test_buffer.c"*/
void test_buffer();
void test_queue();
#include <sys/queue.h>

#include "ac_callback.h"

int main(int argc, char *argv[])
{
	/*set_signal();*/
	/*struct ac_event * event = event_listener_create(NULL, 4444, 32, 32);*/
	/*if(!event) {*/
		/*printf("create event failed\n");*/
	/*}*/
	/*while(!event_is_shutdown(event)) {*/
		/*event_run(event, -1);*/
	/*}*/
	/*event_close(event);*/

	test_buffer();
	/*test_queue();*/
	return 0;
}
