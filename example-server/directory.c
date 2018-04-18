
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
#include <time.h>

#define SERVER_PORT 6662

//Request types
typedef enum {
  CLIENT_JOIN,
  REQUEST_NEW_PEER,
  CLIENT_EXIT,
  ROOT_REQUEST
} request_t;

//Client node
typedef struct client_node
{
  struct client_node * next;
  size_t cid;
  int client_num;
  char ipstr[INET_ADDRSTRLEN]; //holds ip address of client
  int port; //holds listening port for client
  request_t request;
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
  int port;
  char ipstr[INET_ADDRSTRLEN];
} info_packet_t;

//List of potential parents for new clients
typedef struct server_info
{
  int num_clients;
  size_t cid;
  char ipstr[INET_ADDRSTRLEN];
} server_info_t;

//Function signatures
void send_num_total_clients(int client_socket);

int cur_num_clients = 0;  //tracks unique ids for clients
client_list_t* dir_list;
int list_size = 0;


//Creates a new empty client node
client_node_t* create_client_node(info_packet_t * client_packet){
  client_node_t * new_node = malloc (sizeof(client_node_t));
       
  if (new_node == NULL){
    perror("Malloc failed!");
    exit(2);
  }

  strcpy(new_node->ipstr, client_packet->ipstr);
  new_node->next = NULL; 
  new_node->cid = cur_num_clients++;
  new_node->port = client_packet->port;
  printf("new client port :%d\n",new_node->port);
  return new_node;
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

//Accepts an incoming request on the server and returns the client socket
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


//Sends the list of all clients in the directory to the requesting client
void send_all_parents_list(int client_socket) {

//send client expected number of clients_nodes to receive
//send_num_total_clients(client_socket);

  client_node_t ret;

  printf("I am in send all parents\n");
  printf("list size: %d\n", list_size);

  if (list_size > 1){

printf("I am inside loop\n");
  srand(time(0));

  int random = rand() % list_size -1;
  if (random < 0){
    random = 0;
  }
  printf("random %d\n", random);

  client_node_t* cur = dir_list->head;

  int i = 0;
  while(cur != NULL) {

    if(i == random) {
      break;
    }

    cur = cur->next;
    i++;
  }

printf("cur port: %d\n", cur->port);


 ret.cid = cur->cid;
 ret.client_num = cur->client_num;
 strcpy(ret.ipstr, cur->ipstr);
 ret.port = cur->port;

 //ret = *cur;
  printf("ret port: %d\n", ret.port);
  printf("cur ip: %s\n", cur->ipstr);

}

  if (list_size == 1) {
    ret = *dir_list->head;
    ret.request = ROOT_REQUEST;
  }


  //printf("ret port: %d", ret.port);
 write(client_socket,(void *) &ret, sizeof(ret));
 //send array of client nodes to client
 //send(client_socket,(void *) potential_clients, sizeof(client_node_t) * list_size, 0);
 close(client_socket);
}



/*
//Sends the total number of clients in the directory to the client
void send_num_total_clients(int client_socket) {
 server_info_t server_info;
 server_info.num_clients = list_size;
 server_info.cid = cur_num_clients;
 write(client_socket,(void *) &server_info, sizeof(server_info_t));
// send(client_socket,(void *) &server_info, sizeof(server_info_t), 0);
}
*/

//Removes the client that wants to exit from the system
void update_directory_server(size_t cid){

  client_node_t* cur = dir_list->head;
  client_node_t* prev = dir_list->head;

  while(cur != NULL) {
    //if we find the cid we are looking for, just change the pointers
    if(cur->cid == cid) {
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

    
  while(true) {


   struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(struct sockaddr_in);
    int client_socket = accept(server, (struct sockaddr*)&client_addr, &client_addr_length);

    if(client_socket == -1) {
      perror("accept failed");
      exit(2);
    }

  	//Get the ip address of the connecting client
  	struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
  	struct in_addr ipAddr = pV4Addr->sin_addr;
  	char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddr, ipstr, INET_ADDRSTRLEN);


  //Get the rest of the info from connecting client 
  info_packet_t packet_info;
  read(client_socket, (info_packet_t*) &packet_info, sizeof(packet_info));
  /*if (recv(client_socket, (info_packet_t*) &packet_info, sizeof(packet_info), 0) < 0){
    perror("There was a problem in reading the data\n");
    exit(2);
  }*/

  //add the ipstr of client to the received packet info
  strcpy(packet_info.ipstr, ipstr);

  //handle the appropriate request from the client node
  switch (packet_info.request){

  case CLIENT_JOIN:
    printf("client join");
    append_node(create_client_node(&packet_info));
    send_all_parents_list(client_socket);
    break;

  case REQUEST_NEW_PEER:
 	send_all_parents_list(client_socket);
     break;

  case CLIENT_EXIT:
  update_directory_server(packet_info.client_id);
  break;

  case ROOT_REQUEST:
  break;
  } 

  //close(client_socket);
 }
    
  close(server);

       
  return 0; 
}





