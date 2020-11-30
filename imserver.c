#include "instant_messaging.h"


int shutdown_t = 0;	// to handle ctrl-c
// handle ctrl-c
void signal_handler(int signal) {
	//shutdown_t = 1;
	exit(EXIT_SUCCESS);
}


int main(int argc, char **argv) 
{
	int maxfd;	// number of descriptors to be tested.
	int listenfd;	// to store created listening socket
	int maxi, connfd, sockfd, id;	// will hold file descriptors
	int portno;
	fd_set allset;	// master file descriptor list
	fd_set rset;	// temp file descriptor list for select(), with r stands for read
	char buf[MAXLINE]; // buffer for client data
	socklen_t clilen;	// contain the size (in bytes) of the structure pointed to by addr (accept());
	struct sockaddr_in clt_addr;	// client's socket address (ip addres + port number)
	struct sockaddr_in serv_addr;	// server's socket address (ip address + port number)
	int i; // used in for loop at line 84. It will be used to check turned on bits in file descriptor 
	int j, z, n;	// the first two handle for loops (line 113). The third one (n) handle recv() returns
	int nready, client[FD_SETSIZE - 1];
	char name[30];
	char name_t[30]; // used exclusively to find names already in the list
	int cc = 0; // connected clients
	int disconnected;	// get id of who disconnected from chat
	int m_writer;	// get id of who wrote some message
	char wrote[MAXLINE];	// user wrote:
	char *substring; // to read message sent by instant message user.
	char message_b[MAXLINE]; // will hold message when client sends message to a group
	char *users_list; // will hold 'this' group of clients
	char buf_t[MAXLINE]; // to send message to user a client was not found
	char *str, *token, *saveptr; // for use with 'tokenized' elements
	
	// structs (linked lists) declarations
	struct client *head, *aClient, *tmp;
	// some structs to handle deleted nodes
	struct client *prev, *curr;

	aClient = head = NULL;
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <server-port>\n", argv[0]);
		return 1;
	}

	portno = atoi(argv[1]); // retrieve port number as an integer

	listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// open socket
	if(listenfd < 0)
		syserr("Could not open socket");
	memset(&serv_addr, 0, sizeof(serv_addr)); // clear bits on server address (filled up just below)

	// fill up serv_addr struct with address information
	serv_addr.sin_family = AF_INET;	// IPv4 family
	//The htonl() function converts the unsigned integer hostlong from host byte order to network byte order
	serv_addr.sin_addr.s_addr = INADDR_ANY;	// IP address
	//The htons() function converts the unsigned short integer hostshort from host byte order to network byte order
	serv_addr.sin_port = htons(portno); // the port number

	if(bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		syserr("bind() function failed");	// i.e. can't assign address (serv_addr) to socket (listenfd)
	}
//	printf("bind() function successful\n");

	if(listen(listenfd, 10) == -1) {
		syserr("listen() function failed");
	}
//	printf("listen() function sucessful\n");

	maxfd = listenfd;	// initialize amount of file descriptors to be tested (in this case bit 0)
	maxi = -1;
	for(i=0; i<FD_SETSIZE; i++)
		client[i] = -1;	/* -1 indicates available entry */
	FD_ZERO(&allset);	// initialize fd_set: all bits off. Very important!!
	// void FD_SET(int fd, fd_set* fdset);
	FD_SET(listenfd, &allset);	// turn on the bit for listenfd in fdset

	printf("Now, I am listening...\n");

	// handle ctrl-c with a signal handler
	signal(SIGINT, signal_handler);

/*=================================================================================================================================*/

	// THE FOREVER LOOP
	for(;;) {
		clean_buffer(buf, 0, MAXLINE);

		rset = allset;	/* structure assignment */
		// call to select. Note the final argument (the timevalue struct) is set to NULL since we want
		// the call to block until something is ready
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(nready == -1) {
			syserr("select() function failed");
		}

/*---------------------------------------------------------------------------------------------------------------------------------*/

		// Handle new clients connection
		if(FD_ISSET(listenfd, &rset)) {
			clilen = (socklen_t) sizeof(clt_addr);
			if((connfd = accept(listenfd, (struct sockaddr *) &clt_addr, &clilen)) == -1){
				syserr("Server's accept() failed");
			}
			
			FD_SET(connfd, &allset);
			if(connfd > maxfd)
				maxfd = connfd;
			printf("New client from: %s on socket %d\n",inet_ntoa(clt_addr.sin_addr),connfd);
		
			// loop to find a new space to place new client
			for(i=0; i < FD_SETSIZE; i++) {
				if(client[i] < 0) {
					client[i] = connfd; // save descriptor
					sockfd = client[i];
					break;
				}
			} //end for loop

			if(i == FD_SETSIZE)
				syserr("Too many clients");
			FD_SET(connfd, &allset); 	// add new descriptor to set
			if(connfd > maxfd)
				maxfd = connfd; // for select
			if(i > maxi)
				maxi = i;	// max index in clients array


			// if new connection is received...
			clean_buffer(buf, 0, MAXLINE );
			if((n = recv(sockfd, buf, sizeof(buf), 0)) <= 0) {
				if(n < 0)
					syserr("Cannot receive from client");
				close(sockfd);
				FD_CLR(sockfd, &allset);
				client[i] = -1;
			}
			else	// HANDLE ALL LINKED LIST INSERTING STUFF FOR NEW CLIENTS
			{			
				// Now, the idea is to find if a client with same name exist in linked list
				// and if so, reject the connection
				int fn;
				sprintf(name, "%s", buf);
				// check for reserved words
				if(strcmp(buf, "all")==0) {
						clean_buffer(buf, 0, MAXLINE );
						n = sprintf(buf, "%s is a reserved word, so you have been rejected.\n"
								     "Hint: connect again using a different name.\n", name);
						n = send(sockfd, buf, n + 1, 0);
						if(n < 0)
							syserr("Could not send to client");
						close(sockfd);
						FD_CLR(sockfd, &allset);
						client[i] = -1;	
						printf("New client has been rejected (used reserved word).\n");				
						continue;					
				}
				// if name is used already in the chat notify new client
				// and end the connection
				if(head != NULL) {
					if((fn = assign_name(head, name)) == 1)
					{
						clean_buffer(buf, 0, MAXLINE );
						n = sprintf(buf, "Dear %s. Your name already exists in the chat.\n"
									 "So, you have been rejected\n"
								     "Hint: connect again using a different name.\n", name);
						n = send(sockfd, buf, n + 1, 0);
						if(n < 0)
							syserr("Could not send to client");
						close(sockfd);
						FD_CLR(sockfd, &allset);
						client[i] = -1;	
						printf("New client has been rejected "
							   "(somebody with same name, already in chat)\n");					
						continue;
					}
				}
				// now create space for new client and append it and the end of linked list
				if(head == NULL)
				{
					head = aClient = (struct client *)malloc(sizeof(struct client));
					if(head == NULL)
						syserr("Out of Memory");
				}
				else if(aClient->next == NULL)
				{
					aClient->next = (struct client *)malloc(sizeof(struct client));
      				if(aClient->next == NULL)	// always check if memory is full
            			syserr("Out of Memory\n");
					aClient = aClient->next;
					aClient->next = NULL;
				}

      			aClient->as_number = sockfd;
				for(i=0; i<sizeof(name); i++) {
					aClient->as_name[i] = name[i];
				}
			
				cc++; // update connected clients
				clean_buffer(buf, 0, MAXLINE );
				n = sprintf(buf, "\nWelcome to our instant messaging chat!!\n"
				"Connected clients: %d\nYour name: %s\n", cc, aClient->as_name);
				buf[n + 1] = '\0';
				n = send(sockfd, buf, n + 1, 0);
				if(n < 0)
					syserr("Could not send to client");
				
				// Send new connected client a list with current chat members 
				//send_list_of_connected_clients(head, aClient->as_number, sockfd, buf);
				// Now communicate all chat members, a new client connected
				for(j=0; j <= maxi; j++) {
					if(((sockfd = client[j]) < 0)) // || client[j] == aClient->as_number) // do not send to whom just entered the chat 
						continue;
					clean_buffer(buf, 0, MAXLINE );
					if(!(client[j] == aClient->as_number)) {
					n = sprintf(buf, "%s has entered the chat. ", aClient->as_name);
					buf[n + 1] = '\0';
					n = send(sockfd, buf, n + 1, 0);
					if(n < 0)
						syserr("Could not send to client");
					}
					send_list_of_connected_clients(head, sockfd, buf);
				}			

			}

			if(--nready <= 0)
				continue; // no more readable descriptors
		}	// end for() loop to handle new client connections

/*---------------------------------------------------------------------------------------------------------------------------------*/	

		// Handle received data from clients 
		for(i=0; i <=maxi; i++) { 
			if((sockfd = client[i]) < 0) // if not ready
				continue; 
			if(FD_ISSET(sockfd, &rset)) {
				clean_buffer(buf, 0, MAXLINE );
				if((n = recv(sockfd, buf, sizeof(buf), 0)) <= 0) {
					if(n < 0)
						syserr("Cannot receive from client");
					find_name(head, sockfd, name_t);
                    //printf("size of head is : %d\n", sizeof(head));
                    //printf("...and the size of NULL is %d\n", sizeof(NULL));
					//// remove client from list
					//head = (struct client *) delete_client(head, sockfd); 
                    //printf("I just returned fine from delete_client\n");
					if(head == NULL)
						printf("List is empty. Nothing to be deleted \n");
					else {
						curr = head;

						if(curr->as_number == sockfd) {
        					head = curr->next;
							free(curr);
						}
						else {
							prev = curr;
							curr = curr->next;
   							while (curr != NULL) {
								if(curr->as_number == sockfd) {
									prev->next = curr->next;
									free(curr);
									break;
								}
								prev = curr;
        						curr = curr->next;
    						}							
						}
					}
					// Now put update into last node
					// aClient = (struct client *) get_last_node(head, aClient);
					if(head == NULL)
						aClient = head;
					else {
						tmp = head;
						while(tmp->next != NULL) {
							tmp = tmp->next;
    					}
						aClient = tmp;
					}
                
					disconnected = sockfd;
					close(sockfd);
					printf("%s has been disconnected\n", name_t);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
					cc--;	// update connected clients

					//If somebody disconnect let all the chat members know
					//Then, send all of them a list of remaining members
					for(j=0; j <= maxi; j++) {
						//printf("d_sockfd = %d\n", client[j]);
						if((sockfd = client[j]) < 0)
							continue;
						clean_buffer(buf, 0);
						n = sprintf(buf, "\n%s just disconnected from the chat. ", name_t);
						n = send(sockfd, buf, n, 0);
						if(n < 0)
							syserr("Could not send to client");
						clean_buffer(buf, 0);
						send_list_of_connected_clients(head, sockfd, buf);
					}					
				}
				else {	// server received some message from a client... 
						// get name of whom wrote message
					find_name(head, sockfd, name_t);

					if(strncmp(buf, "msg @ ", 6) == 0)
					{
						substring = buf + 6;
					
						if(strncmp(substring, "all", 3) == 0) {
							while (*(substring) != EOF) {
								substring = substring + sizeof(char);
								if(*(substring) == '\n') {
									substring = substring + sizeof(char);
									break;
								}
							}
							
							for(j=0; j < strlen(substring); j++)
							{
								buf[j] = *(substring + j);
							}
							clean_buffer(buf, strlen(substring), MAXLINE );
	
							for(j=0; j <= maxi; j++) {
								// do not send to client who wrote message
								if(((sockfd = client[j]) < 0) || j == i) {		
									if(name_t == NULL){
										printf("%s has not been found\n", name_t);
										break;							
									}		
									continue;
								}	// end if (do not send whom wrote message)
								clean_buffer(wrote, 0, MAXLINE);
								n = sprintf(wrote, "\n%s wrote: ", name_t);
								n = send(sockfd, wrote, n, 0);
								if(n < 0)
									syserr("Could not send to client");
								usleep(100);
								n = send(sockfd, buf, strlen(buf), 0);
								if(n < 0)
									syserr("Could not send to client");
							} // end for (j) loop
						}
						else // send message to one or group of people
						{
							// Up to here you got 'substring' pointing "user1,
							// user2, ... , user n. Find the message which 
							// is after \n and place it in message_buffer.
   						 	// Also, we need to parse the clients list. 
							// (using strok ????)
							// Remember!!! buf is still untouched!!!!

							// save pointer for users' group
							users_list = substring;	
							// make temporary buffer message ready
							clean_buffer(message_b, 0, MAXLINE ); 
							while (*(substring) != EOF) {
								substring = substring + sizeof(char);
								if(*(substring) == '\n') {
									substring = substring + sizeof(char);
									break;
								}
							}
							for (j=0; j<strlen(substring); j++) {
								//now we got the message
								message_b[j] = *(substring + j);	
							}

							int users_size;
							
							for(j=0; *(users_list + j) != '\n'; j++){
									buf[j] = *(users_list + j);
									//if((users_list + j)== NULL)
										//break;
							}
							users_size = j;
							// eliminate message from original buffer
							clean_buffer(buf, users_size, MAXLINE ); 

							// Now we need to look for clients inside this string
							for (str = buf; ; str = NULL) {
        						token = strtok_r(str, ", ", &saveptr);
        						if (token == NULL)
            							break;
								id = find_client_id(token, head); // find id of user
								// if name was not found ... 
								//send back message to sender
								if(id == -1) { 
									n = sprintf(buf_t, "%s was not found in the list\n", token);
									n = send(sockfd, buf_t, n, 0);
									if(n < 0)
										syserr("Could not send to client");
								}
								else {
									clean_buffer(wrote, 0, MAXLINE);
									n = sprintf(wrote, "\n%s wrote: ", name_t);
									n = send(id, wrote, n, 0);
									if(n < 0)
										syserr("Could not send to client");	
									n = send(id, message_b, strlen(message_b), 0);
									if(n < 0)
										syserr("Could not send to client");
								}
        						//printf("%s\n", token);
							}
						} // end else for sending message to a group of users
					}	// end if(strncmp(buf, "msg @ ", 6) == 0)
					else {
						n = sprintf(buf, "Message from Server:\n"
							  "Dear %s:\nThe valid format is:\n"
							  "msg @ all <newline> message (if sent to everybody), or\n"
							  "msg @ user1, user2, ... , etc <newline> message\n"
							  "(if sent to an individual or group of users.\n"
							  "Please, try again\n", name_t);
						n = send(sockfd, buf, n, 0);
						if(n < 0)
							syserr("Could not send to client");
					}
				}
				if(--nready <= 0)
					break; // no more readable descriptors	
			}
		}
/*---------------------------------------------------------------------------------------------------------------------------------*/
	}
/*=================================================================================================================================*/
	close(sockfd);
	close(maxfd);
	close(listenfd);
	return 0;
}

