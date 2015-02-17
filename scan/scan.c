/**
 * file name: scan.c
 */

#include <gm_net.h>
#include "scan.h"

static int
get_max_fds()
{
    int n;

    if ((n = sysconf(_SC_OPEN_MAX)) == -1)
	err_sys("sysconf error");
    return (n - 3);		/* stdin，stdout，stderr */
}

static void
do_scan(const char *host, int min, int nports)
{
    int fd, nleftports, n, i, flag, error, maxfd;
    struct port *ports;
    struct sockaddr_in dstaddr;
    fd_set rset, wset, rs, ws;
    
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    
    bzero(&dstaddr, sizeof(struct sockaddr_in));
    dstaddr.sin_family = AF_INET;
    Inet_pton(AF_INET, host, &dstaddr.sin_addr);

    ports = Malloc(nports * sizeof(struct port));
    for (i = 0; i < nports; i++) {
	/* 创建套结口 */
	ports[i].p_fd = Socket(AF_INET, SOCK_STREAM, 0);
	ports[i].p_flags = P_CONNECTING;
	fd = ports[i].p_fd;
	
	/* 把套结口设置为非阻塞 */
	flag = Fcntl(fd, F_GETFL, 0);
	Fcntl(fd, F_SETFL, flag | O_NONBLOCK);
	
	/* 设置端口，然后发起连接 */
	dstaddr.sin_port = htons(min + i);
	connect(fd, (SA *) &dstaddr, sizeof(struct sockaddr_in));
	if (errno != EINPROGRESS)
	    err_sys("nonblocking connect error");
	
	/* 设置相应的位 */
	FD_SET(fd, &rset);
	FD_SET(fd, &wset);
    }

    maxfd = fd;
    nleftports = nports;
    while (nleftports > 0) {
	rs = rset;
	ws = wset;
	Select(maxfd + 1, &rs, &ws, NULL, NULL);

	for (i = 0; i < nports; i++) {
	    /* 忽略已经有结果的端口 */
	    flag = ports[i].p_flags;
	    if (flag == P_OPEN || flag == P_CLOSE)
		continue;
	    
	    fd = ports[i].p_fd;
	    if (FD_ISSET(fd, &rs) || FD_ISSET(fd, &ws)) {
		/* 根据套结口上的错误进行判断 */
		n = sizeof(error);
		Getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &n);
		if (error != 0)
		    ports[i].p_flags = P_CLOSE;
		else 
		    ports[i].p_flags = P_OPEN;
		
		/* 清除相应的位 */
		FD_CLR(fd, &rset);
		FD_CLR(fd, &wset);
		
		nleftports--;
	    }
	}
    }
    
    /* 打印结果 */
    for (i = 0; i < nports; i++) {
	if (ports[i].p_flags == P_OPEN)
	    printf("port %d open\n", min + i);
    }
    /* 等待所有子进程退出 */
    while (wait(NULL) != -1);
}

int
main(int argc, char **argv)
{
    pid_t pid = 0;
    int i, min, max, max_fds, nports;
    char maxbuf[PORT_LEN], minbuf[PORT_LEN];

    if (argc != 4)
	err_quit("Usage: scanport <ip> <min port> <max port>");

    min = atoi(argv[2]);	/* 最小端口号 */
    max = atoi(argv[3]);	/* 最大端口号 */
    max_fds = get_max_fds();	/* 进程的最大打开文件数-3 */
    nports = max - min + 1;	/* 需要扫描的端口数 */

    if (max < min || min < 1 || max > 65535)
	err_quit("invalid argument");
    
    /* 根据需要扫描的端口数fork相应数量的子进程 */
    while (nports / (max_fds + 1) > 0) {
	if ((pid = Fork()) == 0) {
	    snprintf(minbuf, sizeof(minbuf), "%d", min);
	    snprintf(maxbuf, sizeof(maxbuf), "%d", min + max_fds -1);
	    execl(PROGRAM, PROGRAM, argv[1], minbuf, maxbuf, NULL);
	    err_sys("execl error");
	}
	min += max_fds;
	nports -= max_fds;
    }
    /* 开始扫描 */
    do_scan(argv[1], min, nports);

    return 0;
}
