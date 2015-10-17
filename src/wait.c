#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "unused.h"

static pid_t listen_pid = 0;
static volatile unsigned char need_exit = 0;

static int nl_connect()
{
	int nl_sock;
	struct sockaddr_nl sa_nl;
	
	if((nl_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR)) < 0)
	{
		perror("socket");
		return -1;
	}
	
	sa_nl.nl_family = AF_NETLINK;
	sa_nl.nl_groups = CN_IDX_PROC;
	sa_nl.nl_pid = getpid();
	
	if(bind(nl_sock, (struct sockaddr *)&sa_nl, sizeof(sa_nl)) < 0)
	{
		perror("bind");
		close(nl_sock);
		
		return -1;
	}
	
	return nl_sock;
}

static int nl_subscribe(int nl_sock, unsigned char enable)
{
	struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
		struct nlmsghdr nl_hdr;
		struct __attribute__ ((__packed__)) {
		struct cn_msg cn_msg;
			enum proc_cn_mcast_op cn_mcast;
		};
	} nlcn_msg;
	
	memset(&nlcn_msg, 0, sizeof(nlcn_msg));
	nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
	nlcn_msg.nl_hdr.nlmsg_pid = getpid();
	nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;
	
	nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
	nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
	nlcn_msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);
	
	if(enable == 1)
	{
		nlcn_msg.cn_mcast = PROC_CN_MCAST_LISTEN;
	}
	else
	{
		nlcn_msg.cn_mcast = PROC_CN_MCAST_IGNORE;
	}
	
	if(send(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0) < 0)
	{
		perror("netlink send");
		return -1;
	}
	
	return 0;
}

static int nl_handle_events(int nl_sock)
{
	int rc;
	struct __attribute__ ((aligned(NLMSG_ALIGNTO)))
	{
		struct nlmsghdr nl_hdr;
		struct __attribute__ ((__packed__))
		{
			struct cn_msg cn_msg;
			struct proc_event proc_ev;
		};
	} nlcn_msg;
	
	while(!need_exit)
	{
		if((rc = recv(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0)) == 0)
		{
			// shutdown
			return 0;
		}
		else if(rc == -1)
		{
			if(errno == EINTR)
			{
				continue;
			}
			
			perror("netlink recv");
			
			return -1;
		}
		
		switch(nlcn_msg.proc_ev.what)
		{
			case PROC_EVENT_NONE: break;
			case PROC_EVENT_EXIT:
			{
				// printf("exit: tid=%d pid=%d exit_code=%d\n",
				// 	nlcn_msg.proc_ev.event_data.exit.process_pid,
				// 	nlcn_msg.proc_ev.event_data.exit.process_tgid,
				// 	nlcn_msg.proc_ev.event_data.exit.exit_code);
				
				if(nlcn_msg.proc_ev.event_data.exit.process_pid == listen_pid)
				{
					printf("%i\n", nlcn_msg.proc_ev.event_data.exit.exit_code);
					
					return 0;
				}
				
				break;
			}
			default: break;
		}
	}
	
	return 0;
}

static void signal_handler_sigint(int signo)
{
	UNUSED(signo);
	
	need_exit = 1;
}

int main(int argc, char *argv[])
{
	int nl_sock;
	
	if(argc != 2)
	{
		printf("Usage: %s <pid>\n\tWaits for a pid to exit. Outputs the exit code of the pid and also exits.\n", argv[0]);
		
		return -1;
	}
	
	listen_pid = strtoul(argv[1], NULL, 10);
	
	signal(SIGINT, &signal_handler_sigint);
	siginterrupt(SIGINT, 1);
	
	if((nl_sock = nl_connect()) < 0)
	{
		return -1;
	}
	
	if(nl_subscribe(nl_sock, 1) < 0)
	{
		close(nl_sock);
		
		return -1;
	}
	
	if(nl_handle_events(nl_sock) < 0)
	{
		close(nl_sock);
		
		return -1;
	}
	
	nl_subscribe(nl_sock, 0);
	
	close(nl_sock);
	
	return 0;
}
