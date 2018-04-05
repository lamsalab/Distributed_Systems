
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

/*
We need List structure that holds information about the clients.
Each client can be a node in a linked list which holds the information
about the cline including its IP address and its listening port.
*/
#define CLIENT_JOIN = 0;
#define REQUEST_NEW_PEERS = 1;
#define CLIENT_EXIT = 2;


typedef struct client_node
{
	struct client_node * next;
	size_t cid; //client ID
	int client_num; //TODO DO WE REALLY NEED THIS FIELD
	char ipstr[INET_ADDRSTRLEN]; //holds ip address of client
	size_t port; //holds listening port for client
} client_node_t;

typedef struct client_list {
	client_node_t * head; //pointer to list of client nodes
	int cur_num_clients;
} client_list_t;

int cur_num_clients; //provides unique IDs for our clients
client_list_t* dir_list; //directory

//Creates a new client node
client_node_t* create_client_node(size_t cid, int client_num, char* ipstr, size_t port){
	
	client_node_t * new_node = malloc (sizeof(client_node_t));
	
	if (new_node == NULL){
		perror("Malloc failed!");
	}

	new_node->cid = cid;
	new_node->client_num = client_num;
	strcpy(new_node->ipstr, ipstr);
	new_node->port = port;

	return new_node;
}


//Returns a linked list of client nodes
client_list_t* return_list_clients(){
	return dir_list;
}

void print_list(client_node_t* cur) {
	while(cur != NULL) {
	printf("cid: %d %s\n", (int) cur->cid, cur->ipstr);
	cur = cur->next;
	}
}

//Adds a client node to end our dir_list
void append_node(client_node_t* node) {
	client_node_t* cur = dir_list->head;

		//if the list is empty, just put in front
		if(cur == NULL) {
			dir_list->head = node;
			return;
		}

		//otherwise traverse and add node at the end
		while(cur->next != NULL) {
			cur = cur->next;
		}
		cur->next = node;
}

//Removes the client that wants to exit 
void update_directory_server(size_t cid){

	client_node_t* cur = dir_list->head;
	client_node_t* prev = cur;

	while(cur != NULL) {

	//if we find the client id we want to delete
	if(cur->cid == cid) {
	
	//if it is the head, handle the special case
	if(cur == dir_list->head) {
	dir_list->head = cur->next;
	free(cur);
	return;
	}

	//otherwise handle the normal case
	prev->next = cur->next;
	free(cur);
	return;
	}
	prev = cur;
	cur = cur->next;
}

}

int main(int argc, char const *argv[])
{

	dir_list = malloc (sizeof (client_list_t));
	dir_list->head = NULL;
	
	client_node_t* cur = dir_list->head;
	client_node_t* prev = dir_list->head;

 for(int i = 0; i < 5; i++) {
 	char c[2] = "a";
	append_node(create_client_node(i, 0, c, 12));
}

	print_list(dir_list->head);

/*	//creates a socket
	int s = socket(AF_INET, SOCK_STREAM, 0);

	//check if it fails to create
	if(s == -1) {
	  perror("socket failed");
	  exit(2);
	}

	//set up an internet socket address where server listens to 
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4443);

	//bind socket to address
	if(bind(s, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
	  perror("bind failed");
	  exit(2);
	}

	//begin listening
	if(listen(s, 2)) {
	  perror("listen failed");
	  exit(2);
	}

	//accept an incoming connection
	struct sockaddr_in client_addr;
	socklen_t client_addr_length = sizeof(struct sockaddr_in);
	int client_socket = accept(s, (struct sockaddr*)&client_addr, &client_addr_length);

	if(client_socket == -1) {
	  perror("accept failed");
	  exit(2);
	}
	*/
		return 0; 
	}





