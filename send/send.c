/**
 * file name: nagle.c
 * 用于向某个TCP或UDP端口发送数据，以观察网络中的分组
 */

#include <gm_net.h>
#include <netinet/tcp.h>
#include "send.h"

static void usage(const char *message);

int packet_size = 1024;		/* 缺省每次写1024个字节 */
int packet_num = 10;		/* 缺省写10次数据 */
int sleep_time = 0;		/* 缺省不睡眠 */
int option = 0 | SN_TCP;	/* 缺省使用TCP */

int
main(int argc, char *argv[])
{
    int fd, on;
    socklen_t socklen;
    char ch, sendbuf[MAX_SIZE];
    struct sockaddr_in servaddr;

    /*  处理选项 */
    opterr = 0;
    while ((ch = getopt(argc, argv, "p:s:c:S:CRDN")) != -1) {
	switch (ch) {
	case 'p':
	    if (strcmp(optarg, "tcp") == 0)
		option |= SN_TCP;
	    else if (strcmp(optarg, "udp") == 0)
		option &= ~SN_TCP;
	    else
		usage("unknown protocol");
	    break;
	case 's':
	    packet_size = atoi(optarg);
	    if (packet_size > MAX_SIZE || packet_size < 1)
		usage("packet size too big or too small");
	    break;
	case 'c':
	    packet_num = atoi(optarg);
	    if (packet_num < 1)
		usage("packet num must >0");
	    break;
	case 'S':
	    sleep_time = atoi(optarg);
	    if (sleep_time < 0)
		usage("sleep time must >=0");
	    break;
	case 'C':
	    option |= SN_CONNCT;
	    break;
	case 'R':
	    option |= SN_READ;
	    break;
	case 'D':
	    option |= SN_DF;
	    break;
	case 'N':
	    option |= SN_NODELAY;
	    break;
	case '?':
	    usage("Unknown option");
	}
    }

    if (optind > argc - 2)
	usage("need ip or port");

    memset(sendbuf, 0xa5, sizeof(sendbuf));
    
    bzero(&servaddr, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[optind+1]));
    Inet_pton(AF_INET, argv[optind], &servaddr.sin_addr);

    if (option & SN_TCP) {	/* TCP */
	fd = Socket(AF_INET, SOCK_STREAM, 0);
	if (option & SN_NODELAY) {	    /* 关闭nagle算法 */
	    on = 1;
	    Setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int));
	}
	Connect(fd, (SA *) &servaddr, sizeof(struct sockaddr_in));
    } else {			/* UDP */
	fd = Socket(AF_INET, SOCK_DGRAM, 0);
	if (option & SN_CONNCT) { /* 对UDP套结口调用connect */
	    Connect(fd, (SA *) &servaddr, sizeof(struct sockaddr_in));
	}
    }

    if (option & SN_DF) {	/* 设置DF位 */
	on = IP_PMTUDISC_DO;
	Setsockopt(fd, IPPROTO_IP, IP_MTU_DISCOVER, &on, sizeof(int));
    }

    socklen = sizeof(struct sockaddr_in);
    if (option & SN_READ) {	/* 开启一个子进程读套结口，再写到终端 */
	pid_t pid;
	if ((pid = Fork()) == 0) {
	    int nread;
	    char recvbuf[MAXLINE];
	    
	    while ((nread = Recvfrom(fd, recvbuf, MAXLINE,
				     0, (SA *) &servaddr, &socklen)) > 0)
		Write(STDOUT_FILENO, recvbuf, nread);
	    exit(0);
	}
	
    }
    
    while (packet_num--) {	/* 写相应数目和大小的数据到套结口 */
	Sendto(fd, sendbuf, packet_size, 0, (SA *) &servaddr, socklen);
	if (sleep_time)
	    sleep(sleep_time);
    }
    sleep(1);
    Close(fd);

    return 0;
}

static void
usage(const char *message)
{
    printf("error: %s\n", message);
    printf(
"usage: send [-CRDN] [-p tcp|udp] [-s size] [-c count] [-S sleep_time] <IP> <PORT>\n"
"       -C 如果使用UDP，则对UDP套结口调用connect\n"
"       -R 开启一个子进程从套结口读\n"
"       -D 设置DF位\n"
"       -N 如果使用TCP，则关闭Nagle算法\n"
"       -p proto 使用proto指定的协议\n"
"       -s size 发次发送size大小的数据\n"
"       -c count 发送count次\n"
"       -S sleep_time 每次发送后睡眠sleep_time秒\n"
	);
    
    exit(1);
}
