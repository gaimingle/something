/**
 * file name: ls.c
 * 一个ls的简单实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>		/* for stat() */
#include <dirent.h>		/* for dir operations */
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include "ls.h"

static void print_stat(const char *path, struct stat, int verbose);
static void err_sys(const char *msg);
static void usage(const char *msg);

int
main(int argc, char *argv[])
{
    int option = 0;
    char ch, path[PATH_SIZE];
    struct stat st;

    /* 处理选项 */
    opterr = 0;
    while ((ch = getopt(argc, argv, "ald")) != -1) {
	switch (ch) {
	case 'a':
	    option |= LS_ALL;
	    break;
	case 'l':
	    option |= LS_LIST;
	    break;
	case 'd':
	    option |= LS_DIR;
	    break;
	case '?':
	    usage("Unknown option");
	}
    }
    
    /* 确定path */
    if (optind == argc)
	getcwd(path, sizeof(path));
    else
	strcpy(path, argv[optind]);
    
    /* 打开文件 */
    if (lstat(path, &st) == -1)
	err_sys("first lstat error");

    if (!S_ISDIR(st.st_mode) ||	/* 如果不是目录或是目录，但指定了-d选项 */
	(S_ISDIR(st.st_mode) && (option & LS_DIR))) { 
	/* 打印文件信息 */
	print_stat(path, st, option & LS_LIST);
	
    } else if (S_ISDIR(st.st_mode)) { /* 如果是目录并且没有指定-d选项 */
	size_t len;
	DIR *dir;
	struct dirent *dirent;

	/* 打开目录 */
	if ((dir = opendir(path)) == NULL)
	    err_sys("opendir error");

	len = strlen(path);
	if (path[len-1] != '/') {
	    path[len] = '/';
	    path[len+1] = 0;
	}
	/* 遍历该目录下的所有文件 */
	while ((dirent = readdir(dir)) != NULL) {

	    /* 如果没有指定-a选项，则跳过点文件 */
	    if (!(option & LS_ALL) && dirent->d_name[0] == '.')
		continue;
	    
	    strcat(path, dirent->d_name);
	    /* 打开文件并打印文件信息 */
	    if (lstat(path, &st) == -1)
		err_sys("lstat error");
	    print_stat(strrchr(path, '/')+1, st, option & LS_LIST);

	    /* 恢复path */
	    len = strlen(dirent->d_name);
	    path[strlen(path)-len] = 0;
	}
	closedir(dir);
    }
    
    return 0;
}

/*
 * 函数名: print_stat
 * 功能： 打印一个文件的信息
 */
void
print_stat(const char *path, struct stat st, int verbose)
{
    struct passwd *psw;
    struct group  *grp;
    struct tm *tmp;
    
    if (verbose) {		/* 如果指定了verbose */
	/* 文件类型 */
	if (S_ISREG(st.st_mode))
	    printf("-");
	else if (S_ISDIR(st.st_mode))
	    printf("d");
	else
	    printf("?");
	
	/* 文件权限位 */
	printf("%c", (st.st_mode & S_IRUSR)? 'r' : '-');
	printf("%c", (st.st_mode & S_IWUSR)? 'w' : '-');
	printf("%c", (st.st_mode & S_IXUSR)? 'x' : '-');
	printf("%c", (st.st_mode & S_IRGRP)? 'r' : '-');
	printf("%c", (st.st_mode & S_IWGRP)? 'w' : '-');
	printf("%c", (st.st_mode & S_IXGRP)? 'x' : '-');
	printf("%c", (st.st_mode & S_IROTH)? 'r' : '-');
	printf("%c", (st.st_mode & S_IWOTH)? 'w' : '-');
	printf("%c", (st.st_mode & S_IXOTH)? 'x' : '-');

	/* 所有者和所有组 */
	if ((psw = getpwuid(st.st_uid)) == NULL)
	    err_sys("getpwuid error");
	if ((grp = getgrgid(st.st_gid)) == NULL)
	    err_sys("getgrgid error");
	printf(" %s %s", psw->pw_name, grp->gr_name);

	/* 文件大小 */
	if (S_ISREG(st.st_mode))
	    printf(" %-5d", (int) st.st_size);
	else if (S_ISDIR(st.st_mode))
	    printf(" %-5d", (int) st.st_blksize);

	/* 最后一次修改时间 */
	tmp = localtime(&st.st_mtime);
	printf(" %d.%d.%d %02d:%02d ", tmp->tm_year+1900, tmp->tm_mon+1,
	       tmp->tm_mday, tmp->tm_hour, tmp->tm_min);
    }
    
    /* 文件名 */
    printf("%s\n", path);

}

void err_sys(const char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

void
usage(const char *msg)
{
    printf("error: %s\n", msg);
    printf(
	"usage: ls [-ald] [pathname]\n"
	"       -a 列出隐藏文件\n"
	"       -l 列出详细信息\n"
	"       -d 参数为一个目录\n"
	);
    
    exit(1);
}
