#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <pthread.h>
#include <sys/socket.h>

#define MAX_CLIENT_QUEUE 50 // max listen() queue
#define BUFFER_SIZE 1024 // buffer size for recv() message
#define MAX_CONNECTED_CLIENTS 100

typedef struct client{
    struct sockaddr_in* client_address;
    int client_socket;
    int client_index;
    char* client_name;
} Client;

extern Client* clients[MAX_CONNECTED_CLIENTS];

// Mutex and condition variable for clients[] synchronization
extern pthread_mutex_t clients_mutex;
extern pthread_cond_t clients_cond;

int incorrect_arg_num(int argc);
void free_client_mem(Client* client);
int bind_socket_to_addr(int socket,struct sockaddr_in *server_address);
void init_clients(Client* clients);
int is_clients_full();
int insert_client(Client* new_client);
void remove_client(int index);
int get_client_index(int client_socket);
int check_message_type(const char *input_string);
int find_client_index_by_name(char* name);
void echo_msg_to_all_clients(Client* client, char* out_message);

#endif
