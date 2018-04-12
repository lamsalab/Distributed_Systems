#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#include "ui.h"

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
	size_t port;
} info_packet_t;

typedef struct _client_thread_args {
  int s;
} client_thread_args_t;


void* client_fn(void* arg) {
  while (1) {
    //accept an incoming connection
    client_thread_args_t* real_args = (client_thread_args_t*)arg;
    int s = real_args->s;
    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(struct sockaddr_in);
    int client_socket = accept(s, (struct sockaddr*)&client_addr,
                               &client_addr_length);
    printf("hello");
    void * output;
    return output;
  }
}

int main(int argc, char** argv) {
  if(argc != 3) {
    fprintf(stderr, "Usage: %s <username> <server address>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  char* local_user = argv[1];
  char* server_address = argv[2];

  struct hostent* server;
  server = gethostbyname(server_address);
  if(server == NULL) {
    fprintf(stderr, "Unable to find host %s\n", argv[2]);
    exit(1);
  }

  int s = socket(AF_INET, SOCK_STREAM, 0);
  if(s == -1) {
    perror("socket failed");
    exit(2);
  }

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(4446)
  };
  

  bcopy((char*)server->h_addr, (char*)&addr.sin_addr.s_addr, server->h_length);

  if(connect(s, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
    perror("connect failed");
    exit(2);
  }

request_t request = CLIENT_EXIT;

info_packet_t packet = 
   {
   .request = request,
   .port = 4445
  };

  send(s,(void *)&packet, sizeof(packet), 0);

 /* pthread_t client_thread;
  client_thread_args_t args_client;
  args_client.s = s;

  //A thread to accept connections from other clients
  if(pthread_create(&client_thread, NULL, client_fn, &args_client)) {
    perror("pthread_create failed");
    exit(2);
  }

  pthread_join(client_thread, NULL); */

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

