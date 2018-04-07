#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ui.h"

int main(int argc, char** argv) {
  if(argc != 3) {
    fprintf(stderr, "Usage: %s <username> <server address>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  char* local_user = argv[1];
  char* server_address = argv[2];
  
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

