#include "instant_messaging.h"

#define	max(a,b)	((a) > (b) ? (a) : (b))	// (line 73) distinguish between STDIN or socket message
#define STDIN 0 // standard input is 0

void syserr(char* msg) { perror(msg); exit(-1); }

void clean_buffer(char *buffer) {
	int u;
	for(u=0; u<MAXLINE; u++)
		buffer[u] = 0;
}

int main(int argc, char* argv[])
{
    int sockfd, portno, n;
    struct hostent* server;
    struct sockaddr_in serv_socket;
    char *name;
    char sendline[MAXLINE], recvline[MAXLINE];
	int maxfdp1;
	fd_set rset;
	FILE *fp;
	char message[MAXLINE/2];

    if(argc != 4) {
		fprintf(stderr, "Usage: %s <server-ip> <server-port> <name>\n", argv[0]);
        return 1;
    }

	server = gethostbyname(argv[1]); // get server's ip address
	if(!server) {
		fprintf(stderr, "ERROR: no such host: %s\n", argv[1]);
    	return 2;
	}
	portno = atoi(argv[2]);	// get port number


	name = argv[3]; // get client's name
 	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
 	if(sockfd < 0) 
		syserr("can't open socket");

  	memset(&serv_socket, 0, sizeof(serv_socket));
  	serv_socket.sin_family = AF_INET;
  	serv_socket.sin_addr = *((struct in_addr*)server->h_addr);
  	serv_socket.sin_port = htons(portno);

  	if(connect(sockfd, (struct sockaddr*)&serv_socket, sizeof(serv_socket)) < 0)
    	syserr("can't connect to server");

	// in steven's book now comes str_cli function str_cli(stdin, sockfd)
	int i;
	for(i=0; i<strlen(name); i++) {
		sendline[i] = *(name + i);
	}
	sendline[i] = '\0';

	n = strlen(sendline); 
  	n = send(sockfd, sendline, strlen(sendline), 0);
	if(n < 0) 
		syserr("can't send to server");

	FD_ZERO(&rset);

/*=================================================================================================================================*/

	for(;;) {
		FD_CLR(sockfd, &rset);
		FD_SET(STDIN, &rset);	// this is to send input
		FD_SET(sockfd, &rset);	// this one is to receive input
		maxfdp1 = max(fileno(stdin), sockfd) + 1;
		select(maxfdp1, &rset, NULL, NULL, NULL);

		// this is to send
		if(FD_ISSET(STDIN, &rset)) { // socket is readable			
		clean_buffer(sendline);	

			fgets(sendline, MAXLINE/2 - 1, stdin);	
			strcat(sendline, fgets(message, MAXLINE/2 - 1, stdin));
			n = strlen(sendline);
	  		n = send(sockfd, sendline, strlen(sendline), 0);
	  		if(n < 0) syserr("can't send to server");
	  			printf("send...\n");
		}

		// this is to receive
		else if(FD_ISSET(sockfd, &rset)) { // socket is readable
				 
		  	n = recv(sockfd, recvline, MAXLINE - 1, 0);
			if (n == 0) {
				shutdown(sockfd, SHUT_WR); /* send FIN */
				FD_CLR(sockfd, &rset);
				break;			
			}
		  	else if(n < 0) 
				syserr("can't receive from server");
			else
				recvline[n] = '\0';
		  	printf("%s\n", recvline);
		}
	} // end for(;;)

/*=================================================================================================================================*/
  	close(sockfd);
	close(maxfdp1);
	printf("sockets were closed off\n");
  	return 0;
}








