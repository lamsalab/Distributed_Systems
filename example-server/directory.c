
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

typedef enum {
	CLIENT_JOIN,
	REQUEST_NEW_PEER,
	CLIENT_EXIT
} request_t;

typedef struct client_node
{
	struct client_node * next;
	size_t cid;
	int client_num;
	char ipstr[INET_ADDRSTRLEN]; //holds ip address of client
	size_t port; //holds listening port for client
} client_node_t;

typedef struct client_list {
	client_node_t * head; //pointer to list of client nodes
	int cur_num_clients;
} client_list_t;

typedef struct packet
{
	request_t request;
	size_t client_id;
	size_t port;
}info_packet_t;


int cur_num_clients;
size_t cid;
client_list_t* dir_list;


client_node_t* create_client_node(){
	client_node_t * new_node = malloc (sizeof(client_node_t));
	if (new_node == NULL){
		perror("Malloc failed!");
	}
	return new_node;
}


//List of possible parents
client_list_t* return_list_clients(){
	if (cur_num_clients == 0){
		dir_list->head = NULL;
		dir_list->cur_num_clients = 0;
	}
	return dir_list;
}


//Remove the client that wants to exit from the system
void update_directory_server(size_t cid){
	if (dir_list->head == NULL) {
		return;
	}

	client_node_t* cur = dir_list->head;
	client_node_t* prev = dir_list->head;

	if(cur->cid == cid) {
   dir_list->head = prev->next;
   free(prev);
	}

	while(cur->next != NULL || cur->next->cid != cid) {
		prev = cur;
		cur = cur->next;
	}

	if (cur->next == NULL){
		return;
	}

	prev = cur->next;
	free(cur);
}

int main(int argc, char const *argv[])
{

	dir_list = malloc (sizeof (client_list_t));
	dir_list->head = NULL;


	//creates a socket
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
 	 
	//Check if the client sent the information correctly


	info_packet_t packet_info;

	if (recv(client_socket, &packet_info, sizeof(packet_info), 0) < 0){
        perror("There was a problem in reading the data");
        exit(2);
	}

	printf("%d\n", (int)packet_info.port);
	return 0; 
	}





