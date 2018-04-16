
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

#define SERVER_PORT 6662

//Request types
typedef enum {
  CLIENT_JOIN,
  REQUEST_NEW_PEER,
  CLIENT_EXIT
} request_t;

//Client node
typedef struct client_node
{
  struct client_node * next;
  size_t cid;
  int client_num;
  char ipstr[INET_ADDRSTRLEN]; //holds ip address of client
  size_t port; //holds listening port for client
} client_node_t;

//Client list 
typedef struct client_list {
  client_node_t * head; //pointer to list of client nodes
  int cur_num_clients;
} client_list_t;

//Transmitted data packet
typedef struct packet
{
  request_t request;
  size_t client_id;  
  size_t port;
  char ipstr[INET_ADDRSTRLEN];
} info_packet_t;

//List of potential parents for new clients
typedef struct server_info
{
  int num_clients;
} server_info_t;

int cur_num_clients = 0;  //tracks unique ids for clients
client_list_t* dir_list;
int list_size = 0;

//create parents list array
//client_node_t potential_clients[10];

void send_num_total_clients(int client_socket);

//Creates a new empty client node
client_node_t* create_client_node(info_packet_t * client_packet){
  client_node_t * new_node = malloc (sizeof(client_node_t));
  new_node->next = NULL;        
  if (new_node == NULL){
    perror("Malloc failed!");
  }
  new_node->cid = cur_num_clients++;
  new_node->port = client_packet->port;
  strcpy(new_node->ipstr, client_packet->ipstr);
  return new_node;
}

//List of possible parents
client_list_t* return_list_clients(){
  return dir_list;
}

//Appends a new client node to the end of the list
void append_node(client_node_t* node) {

  if(dir_list->head == NULL) {
    dir_list->head = node;
      ++list_size;
    return;
  }

  client_node_t* cur = dir_list->head;

  //traverse to one node before end of the list
  while(cur->next != NULL) {
    cur = cur->next;
  }  

  cur->next  = node;
  ++list_size;
}

int accept_incoming_connection(int server_socket) {

//accept an incoming connection
  struct sockaddr_in client_addr;
  socklen_t client_addr_length = sizeof(struct sockaddr_in);
  int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_length);

  if(client_socket == -1) {
    perror("accept failed");
    exit(2);
  }

  return client_socket;
}

void send_all_parents_list(int client_socket) {

//send client expected number of clients_nodes to receive
send_num_total_clients(client_socket);

//create array to send to client nodes
 client_node_t potential_clients[list_size];
 
//add all the clients from linked list to this array
 client_node_t* cur = dir_list->head;
 int i = 0;
 while(cur != NULL) {
 	potential_clients[i++] = *cur;
	cur = cur->next;
 }

 //send array of client nodes to client
 send(client_socket,(void *) &potential_clients, sizeof(potential_clients), 0);
} 

//Sends the total number of clients in the directory to the client
void send_num_total_clients(int client_socket) {
 server_info_t server_info;
 server_info.num_clients = list_size,
 send(client_socket,(void *) &server_info, sizeof(server_info_t), 0);
}


//Send num possible clients i.e total num of clients whose cid is less than given cid
int send_num_potential_parents(int client_socket, int cid) {
int num_potential_parents = 0;

//Count the number of clients whose cid is less than given cid
client_node_t* cur = dir_list->head;
while(cur != NULL && cur->cid < cid) {
	 ++num_potential_parents;
   cur = cur->next;
}

//printf("potential parents %d\n", num_potential_parents);

//send num of new parents to potentially join to the requesting client
 server_info_t server_info;
 server_info.num_clients = num_potential_parents;
 send(client_socket,(void *) &server_info, sizeof(server_info_t), 0);
 return num_potential_parents;
}

//Sends an array of potential parent nodes to the requesting client
void send_potential_parents(int client_socket, int cid) {

//send requesting client the expected number of client nodes to receive
int num_potential_parents = send_num_potential_parents(client_socket, cid);

//create array of client nodes to send
 client_node_t potential_clients[num_potential_parents];
 
//add all the clients from linked list to this array
 client_node_t* cur = dir_list->head;
 int i = 0;

 while(cur != NULL && cid < cur->cid) {
  potential_clients[i++] = *cur;
  cur = cur->next;
 }

 printf("potential parents %d\n", num_potential_parents);
 //send array of client nodes to client
// send(client_socket,(void *) &potential_clients, sizeof(potential_clients), 0);

}

/*
//Prints the cids of all the clients in our list (Only for testing purposes)
void print_list(client_list_t* lst) {

  client_node_t* cur = lst->head;

  while(cur != NULL) {          
    printf("cid: %zu\n",  cur->cid);
    printf("port: %zu\n",  cur->port);
    printf("ipstr: %s\n",  cur->ipstr);
    cur = cur->next;
  }

  return;
}
*/

//Removes the client that wants to exit from the system
void update_directory_server(size_t  port){

  client_node_t* cur = dir_list->head;
  client_node_t* prev = dir_list->head;

  while(cur != NULL) {
    //if we find the cid we are looking for, just change the pointers
    if(cur->port == port) {
      if(cur == dir_list->head) { 
        dir_list->head = dir_list->head->next;
      } else {
        //otherwise delete normally
        prev->next = cur->next;
      }
      --list_size;
      return;
    }
    //otherwise, keep traversing
    prev = cur;
    cur = cur->next;
  }
}

void init_list() {
  dir_list = malloc(sizeof(client_list_t));
  dir_list->head = NULL;
}

int setup_server() {
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
  addr.sin_port = htons(SERVER_PORT);

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

  return s;
}

int main(int argc, char const *argv[]) {

  init_list();     
  int server = setup_server();
  int client_socket = accept_incoming_connection(server);

  //Get the client information from the connecting client node
  info_packet_t packet_info;
  if (recv(client_socket, (info_packet_t*) &packet_info, sizeof(packet_info), 0) < 0){
    perror("There was a problem in reading the data\n");
    exit(2);
  }

  //handle the appropriate request from the client node
  switch (packet_info.request){
  case CLIENT_JOIN:
    //add client to dir list, give it potential parents to join
    append_node(create_client_node(&packet_info));
  	send_all_parents_list(client_socket);
    break;
  case REQUEST_NEW_PEER:
  append_node(create_client_node(&packet_info));
  append_node(create_client_node(&packet_info));
  append_node(create_client_node(&packet_info));
  send_potential_parents(client_socket, 2);
     break;
  case CLIENT_EXIT:
  //remove client with given port num
  update_directory_server(packet_info.port);
  break;
  } 
    
  close(server);

       
  return 0; 
}





