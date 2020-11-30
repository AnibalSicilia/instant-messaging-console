#include "instant_messaging.h"

void syserr(char *msg) {
	perror(msg);
	exit(-1);
}

void clean_buffer(char *buffer, int start, int limit) {
	int u;
	for(u=start; u<limit; u++)
		buffer[u] = '\0';
}

/*

// Returns a pointer to last node on the list 
struct client * get_last_node(struct client *h, struct client *ln) 
{
	if(h == NULL) {
		ln = h;
		return ln;
	}
    //printf("I pased first test in get_last...\n");
	struct client *tmp;
	tmp = h;
	while(tmp->next != NULL) {
		tmp = tmp->next;
    }
	ln = tmp;
    printf("Lets see if i make it before return value\n");
	return ln;
}

// a function to delete the specified client
struct client * delete_client (struct client *h, int client_no )
{	
   	struct client *prev, *curr;

   	if (h == NULL ) {
      	printf("List is empty. Nothing to be deleted \n");
			return h;
   	}
        
	curr = h;

	if(h->as_number == client_no) {
        h = h->next;
		free(curr);
		return h;
	}

	prev = curr;
	curr = curr->next;
   	while (curr != NULL)	//returns the last item to aClient struct
	{
		if(curr->as_number == client_no) {
				prev->next = curr->next;
				free(curr);
				break;
		}
		prev = curr;
        curr = curr->next;
    }
	return h;
}

*/

// find a name in the linked list of clients
void find_name(struct client *h, int client_no, char name_t [] ) {
	int j;
	struct client *tmp;
	tmp = h;
	if(tmp == NULL) {
		return;
		//return NULL;
	}
	while(tmp != NULL) {
		if(tmp->as_number == client_no) {
			for(j=0; j<strlen(tmp->as_name); j++)
				name_t[j] = tmp->as_name[j];
			clean_buffer(name_t, j, 30);
			return;
			//return tmp->as_name;
		}
		tmp = tmp->next;
	}
	//return NULL;
	return;
}

int assign_name(struct client *h, char *name) {

	struct client *tmp;
	tmp = h;
	int result;
	if(tmp == NULL)
		return 0;
	while(tmp != NULL) {
		result = strcmp(tmp->as_name, name);
		if(result == 0) {
			return 1;
		}
		tmp = tmp->next;
	}
	return 0;
}


//void send_list_of_connected_clients(struct client *h, int clt_num, int skfd, char *buf)
void send_list_of_connected_clients(struct client *h, int skfd, char *buf)
{
	int w = 0;
	ssize_t n;
	struct client *tmp;
	tmp = h;

	usleep(100);
	clean_buffer(buf, 0, MAXLINE );
	if(tmp->next == NULL)
	{
		n = sprintf(buf, "Up to now, you are the only person connected. So,\n"
		"you better wait for more people to enter the chat\n");
		buf[n + 1] = '\0';
		n = send(skfd, buf, n + 1, 0);
		if(n < 0)
			syserr("Could not send to client");
		return;
	}

	n =	sprintf(buf, "These are the connected clients:");
	buf[n + 1] = '\0';
	usleep(100);
	n = send(skfd, buf, n + 1, 0);
	if(n < 0)
	syserr("Could not send to client");

    while (tmp != NULL)
    {
		//if(tmp->as_number != clt_num)
		if(tmp->as_number != skfd)
		{
				usleep(200);	// avoid packets dropping
				clean_buffer(buf, 0, MAXLINE);
				n = sprintf(buf, " - %s ", tmp->as_name);
				buf[n + 1] = '\0';

				n = send(skfd, buf, n + 1, 0);
				if(n < 0)
					syserr("Could not send to client");
		}
        tmp = tmp->next;
		w++;
    }
	//printf("linked list length is: %d\n\n", w);
}


// find client id in the linked list of clients
int find_client_id(char *name, struct client *h)
{	struct client *tmp;
	tmp = h;
	int result;
	int id;
	if(tmp == NULL)
		return 0;
	while(tmp != NULL) {
		result = strcmp(tmp->as_name, name);
		if(result == 0) {
			id = tmp->as_number;
			return id;
		}
		tmp = tmp->next;
	}
	return -1;
}

