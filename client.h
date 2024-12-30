#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>     
#include <arpa/inet.h> 

#define SERVER_PORT 8080  
#define SERVER_IP "127.0.0.1"  
#define BUFFER_SIZE 1024 

void *receive_messages(void *socket_desc);

void run_client();


