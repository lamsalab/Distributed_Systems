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

#define SERVER_PORT 6664

size_t client_id = 0; //default client id

//Request types
typedef enum {
  CLIENT_JOIN,
  REQUEST_NEW_PEER,
  CLIENT_EXIT, 
  ROOT_REQUEST
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
  request_t request;
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


//List of children 
typedef struct children_info { 
size_t port; 
char ipstr[INET_ADDRSTRLEN]; 
children_info_t* next; 
} children_info_t; 


//Children list 
typedef struct children_list {
 children_info_t * head; //pointer to list of client nodes 
 } children_list_t; 

 pthread_mutex_t lock; 
 children_list_t * children; 



 void* child_fn(void* arg) { 
  client_thread_args_t* real_args_child (client_thread_args_t*)arg;
  int child_socket = real_args_child->s;
  children_info_t child_info; 

  //Get the client information from the connecting child node 
  if (recv(child_socket, (client_node_t*) &children_info_t, sizeof(children_info_t), 0) < 0) 
    { perror("There was a problem in reading the data\n"); 
  exit(2); } 

  pthread_mutex_lock(&lock); 
  if (children->head == NULL) 
    { children->head = child_info; 
    } 

  else { children_info_t * ptr = children->head; 
    while (ptr->next != NULL) 
      { ptr = ptr->next; } 
    ptr->next = child_info; 
  } pthread_mutex_unlock(&lock); 
  while(true); 
}


void* client_fn(void* arg) {
  
    //accept an incoming connection
    client_thread_args_t* real_args = (client_thread_args_t*)arg;
    int s = real_args->s;
    struct sockaddr_in child_addr;
    socklen_t child_addr_length = sizeof(struct sockaddr_in);
  
  while (1) {

    int child_socket = accept(s, (struct sockaddr*)&child_addr,
                               &child_addr_length);


    pthread_t child_thread;
    client_thread_args args_child;
    args_child.s = child_socket;
    //A thread to accept connections from other clients
    if(pthread_create(&child_thread, NULL, child_fn, &args_child)) {
      perror("pthread_create failed");
      exit(2);
    }
    pthread_join(client_thread, NULL);

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

  //create packet, with client info
  info_packet_t packet;
  packet.request = CLIENT_JOIN;
  packet.port = port_no;

  printf("client port no before sending: %d\n", packet.port);

//Send the packet to the server
 write(s_server,(void *)&packet, sizeof(packet));

client_node_t server_info;
read(s_server, (client_node_t*) &server_info, sizeof(client_node_t));

//////////// RECEIVING DATA FOR THIS CLIENT BACK FROM THE DIRECTORY SERVER ///////////

client_id = server_info.cid;

 //Check received data
  printf("Received cid: %d\n", (int) client_id);
  printf("Parent port: %d\n", server_info.port);


/*  //Get the client information from the connecting client node
  if (recv(s_server, (client_node_t*) potential_clients, sizeof(client_node_t) * num_clients, 0) < 0) {
    perror("There was a problem in reading the data\n");
    exit(2);
  } */

////// CONNECT TO NEW PARENT, USING DATA RETURNED FROM DIRECTORY SERVER ///////////

if (server_info.request != ROOT_REQUEST) {
  int PARENT_PORT = server_info.port;
  char ipstr[INET_ADDRSTRLEN];
  strcpy(ipstr, server_info.ipstr);
  printf("Parent port later %d\n", PARENT_PORT); 

  //Getting the host name for the parent server
  struct hostent* parent_server;
  parent_server = gethostbyname(ipstr);
  if(parent_server == NULL) {
    fprintf(stderr, "Unable to find host %s\n", ipstr);
    exit(1);
  }

 //Create a socket for the parent
  int s_parent = socket(AF_INET, SOCK_STREAM, 0);
  if(s_parent == -1) {
    perror("socket failed");
    exit(2);
  }

  //initialize the socket address
  struct sockaddr_in addr_parent = {
    .sin_family = AF_INET,
    .sin_port = htons(PARENT_PORT)
  };

  //Copy parent server address
  bcopy((char*)parent_server->h_addr, (char*)&addr_parent.sin_addr.s_addr, parent_server->h_length);

 
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
  ui_init();
  
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
  ui_shutdown();
  
  return 0;
}

