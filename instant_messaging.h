#ifndef __instant_messaging_h
#define __instant_messaging_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>	// definitions for internet operations
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>

#define MAXLINE 4096

struct client {
	int as_number;
	int mc;
	char as_name[30];
	struct client *next;
};
 
#endif
