#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <limits.h>

#include "ui.h"

#define SERVER_PORT 6662

size_t client_id = 0; //default client id

//Request types
typedef enum {
  CLIENT_JOIN,
  REQUEST_NEW_PEER,
  CLIENT_EXIT
} request_t;

//Transmission data packet
typedef struct packet
{
  request_t request;
  size_t client_id;
  int port;
  char ipstr[INET_ADDRSTRLEN];
} info_packet_t;

//Thread args
typedef struct _client_thread_args {
  int s;
} client_thread_args_t;

//Client node
typedef struct client_node
{
  struct client_node * next;
  size_t cid;
  int client_num;
  char ipstr[INET_ADDRSTRLEN]; //holds ip address of client
  int port; //holds listening port for client
} client_node_t;

//Client list 
typedef struct client_list {
  client_node_t * head; //pointer to list of client nodes
} client_list_t;

//List of potential parents for new clients
typedef struct server_info
{
  int num_clients;
  size_t cid;
} server_info_t;



void* client_fn(void* arg) {
  

  while (1) {
    //accept an incoming connection
    client_thread_args_t* real_args = (client_thread_args_t*)arg;
    int s = real_args->s;
    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(struct sockaddr_in);
    int client_socket = accept(s, (struct sockaddr*)&client_addr,
                               &client_addr_length);
  }
    return NULL;
}

//Prints the cids of all the clients in our list (Only for testing purposes)
void print_list(client_list_t* lst) {

  client_node_t* cur = lst->head;

  while(cur != NULL) {          
    printf("cid: %zu\n",  cur->cid);
    printf("port: %d\n",  cur->port);
    printf("ipstr: %s\n",  cur->ipstr);
    cur = cur->next;
    }

  return;
}


int main(int argc, char** argv) {
  if(argc != 3) {
    fprintf(stderr, "Usage: %s <username> <server address>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  char* local_user = argv[1];
  char* server_address = argv[2];

/////////////////////// SETUP THIS CLIENT /////////////////////////////////////////////

  //Setup a socket for this client
  int s_client = socket(AF_INET, SOCK_STREAM, 0);
  if(s_client == -1) {
    perror("socket failed");
    exit(2);
  }

  //Assigns random port
  struct sockaddr_in client_addr = {
    .sin_family = AF_INET,
    .sin_port = htons(0)
  };

  //bind socket to address
  if(bind(s_client, (struct sockaddr*)&client_addr, sizeof(struct sockaddr_in))) {
    perror("bind failed");
    exit(2);
  }

   //begin listening on socket
  if(listen(s_client, 2)) {
    perror("listen failed");
    exit(2);
  }

  //Got the idea from https://stackoverflow.com/questions/4046616/sockets-how-to-find-out-what-port-and-address-im-assigned
  //Get the port no.
  socklen_t len = sizeof(client_addr);
  if (getsockname(s_client, (struct sockaddr *)&client_addr, &len) == -1){
    perror("getsockname");
    exit(2);
  }

  char ipstr[INET_ADDRSTRLEN]; //TODO we should not set the ip string from here, rather, this should be done on the directory
  int port_no = ntohs(client_addr.sin_port);
  inet_ntop(AF_INET, &client_addr.sin_addr, ipstr, INET_ADDRSTRLEN);

//////// SETUP CONNECTION TO DIRECTORY SERVER //////////////////////////////////////

  //Getting the host name for the server
  struct hostent* server;
  server = gethostbyname(server_address);
  if(server == NULL) {
    fprintf(stderr, "Unable to find host %s\n", argv[2]);
    exit(1);
  }

  //Create a socket for the server
  int s_server = socket(AF_INET, SOCK_STREAM, 0);
  if(s_server == -1) {
    perror("socket failed");
    exit(2);
  }

  //Server socket address
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(SERVER_PORT)
  };

  //Copy server address
  bcopy((char*)server->h_addr, (char*)&addr.sin_addr.s_addr, server->h_length);

  //Connect with the server
  if(connect(s_server, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
    perror("connect failed");
    exit(2);
  }

/////// SEND REQUEST TO THE DIRECTORY SERVER ////////////////////////////////////////

  //Packet to be sent to the server
  info_packet_t packet;
  packet.request = CLIENT_JOIN;
  packet.port = port_no;

  printf("client port no before sending: %d\n", packet.port);

//Send the packet to the server
  send(s_server,(void *)&packet, sizeof(packet), 0);

 // client_node_t potential_parents[3];
 server_info_t server_info;
  //Get the client information from the connecting client node
  if (recv(s_server, (server_info_t*) &server_info, sizeof(server_info_t), 0) < 0){
    perror("There was a problem in reading the data\n");
    exit(2);
  }

//////////// RECEIVING DATA FOR THIS CLIENT BACK FROM THE DIRECTORY SERVER ///////////

int num_clients = server_info.num_clients;
client_id = server_info.cid;
client_node_t potential_clients[num_clients];

  //Get the client information from the connecting client node
  if (recv(s_server, (client_node_t*) &potential_clients, sizeof(potential_parents), 0) < 0){
    perror("There was a problem in reading the data\n");
    exit(2);
  }

  //Check received data
  printf("Received cid: %d\n", (int) client_id);
  printf("Num parents received: %d\n", num_clients);
  
  printf("PORT NUMBERS OF PARENTS: ");
  for(int i = 0; i < num_clients; i++) {
    printf("%d ", potential_clients[i].port);
  }
  printf("\n");


  if (num_clients > 1) {
    srand(time(0));


//////  START CONNECTING TO NEW PARENT, USING DATA RETURNED FROM DIRECTORY SERVER ///////////

  int random = rand() % num_clients - 1;
  size_t parent_port = potential_clients[random].port; 
  char ipstr[INET_ADDRSTRLEN];
  strcpy(ipstr, potential_clients[random].ipstr);
  printf("Parent port later %zu\n", parent_port);

  //Getting the host name for the server
  struct hostent* parent_server;
  parent_server = gethostbyname(ipstr);
  if(parent_server == NULL) {
    fprintf(stderr, "Unable to find host %s\n", ipstr);
    exit(1);
  }

  //parent socket address
  struct sockaddr_in addr_parent = {
    .sin_family = AF_INET,
    .sin_port = htons(parent_port)
  };

  //Copy server address
  bcopy((char*)parent_server->h_addr, (char*)&addr_parent.sin_addr.s_addr, parent_server->h_length);

  //Create a socket for the parent
  int s_parent = socket(AF_INET, SOCK_STREAM, 0);
  if(s_parent == -1) {
    perror("socket failed");
    exit(2);
  }

  //Connect with the parent
  if(connect(s_parent, (struct sockaddr*)&addr_parent, sizeof(struct sockaddr_in))) {
    perror("connect failed with parent");
    exit(2);
    }
  }

  pthread_t client_thread;
     client_thread_args_t args_client;
     args_client.s = s_client;

          //A thread to accept connections from other clients
     if(pthread_create(&client_thread, NULL, client_fn, &args_client)) {
     perror("pthread_create failed");
     exit(2);
     }

     pthread_join(client_thread, NULL);
     while(true);


  // Initialize the chat client's user interface.
  /* ui_init();
  
  // Add a test message
  ui_add_message(NULL, "Type your message and hit <ENTER> to post.");
  
  // Loop forever
  while(true) {
  // Read a message from the UI
  char* message = ui_read_input();
    
  // If the message is a quit command, shut down. Otherwise print the message
  if(strcmp(message, "\\quit") == 0) {
  break;
  } else if(strlen(message) > 0) {
  // Add the message to the UI
  ui_add_message(local_user, message);
  }
    
  // Free the message
  free(message);
  }
  
  // Clean up the UI
  ui_shutdown(); */
  
  return 0;
}

