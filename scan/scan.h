/**
 * file name: scan.h
 */

#ifndef _SCAN_H
#define _SCAN_H

#define PROGRAM "scan"
#define PORT_LEN 6
#define P_CONNECTING 0
#define P_OPEN  1
#define P_CLOSE 2

struct port {
    int p_fd;
    int p_flags;
};

#endif
