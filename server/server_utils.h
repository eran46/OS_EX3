#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "server_utils.c"

#define MAX_CLIENT_QUEUE 50 // max listen() queue
#define BUFFER_SIZE 1024 // buffer size for recv() message
#define MAX_CONENCTED_CLIENTS 100 // 

typedef struct client{
    struct sockaddr_in* client_address;
    int* client_socket;
    int* client_index;
    char* client_name;
} Client;

extern Client* clients[MAX_CONNECTED_CLIENTS];

// Mutex and condition variable for clients[] synchronization
extern pthread_mutex_t clients_mutex;
extern pthread_cond_t clients_cond;

#endif
