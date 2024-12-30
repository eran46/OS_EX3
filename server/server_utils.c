#include <sys/socket.h>

int get_port(char* argv[]){

    return str_to_int(char* argv[1]);
}

void check_arg_num(int argc){
    if(argc < 2){
    	printf("Wrong number of arguments. (missing port)");
    	return 1;
    }
    else if(argc > 2){
    	printf("Wrong number of arguments. (too many arguments)");
    	return 1;
    }
}

void free_client_mem(Client* client){
	free(client.client_address);
	free(client.client_socket);
	free(client.client_name);
	free(client.client_index);
	free(client);
}

void init_server_socket(int port){
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // IPv4, Stream Socket, 0=default=TCP
        printf("Socket creation failed");
        return 1;
    }
    return server_socket;
}

struct sockaddr_in init_server_address(port){
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address)); // prevent garbage values    
    server_address.sin_family=AF_INET; // IPv4
    server_address.sin_port = htons(port); // port number - ensure Big Endian
    server_address.sin_addr = INADDR_ANY; // (struct in_addr) accept any IPv4 address
    return server_address;
}

int bind_socket_to_addr(int socket,struct sockaddr_in *server_address){
    if(bind(socket, (struct sockaddr_in*)server_address, sizeof(*server_address) == -1){ // bind failed
    	printf("binding socket to server address failed");
    	return -1;
    }
    return 0;
}

void reap_zombies(int sig) {
    (void)sig; // stop unused parameter warning
    int waitpid_ret_status;
    
    while ((waitpid_ret_status = waitpid(-1, NULL, WNOHANG)) > 0); // reap all current zombie proccesses
    if((waitpid_ret_status == -1 && errno != ECHILD){ // error on waitpid and not error-from-no-waiting-zombie-process
                printf("failed to reap zombie client process. (waitpid())\n");
    }
}

void init_clients(Client* clients){
	for (int i = 0; i < size; i++) {
        clients[i] = NULL;
    }
}

// Function to check if the clients array is full
int is_clients_full() {
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        if (clients[i]->client_socket == 0) {  // Socket 0 indicates unused slot
            return 0;  // Not full
        }
    }
    return 1;  // Full
}

// insert a new client into the clients array, trust there is room
int insert_client(Client new_client) {
    // Find an empty slot and insert the client
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        if (clients[i] == NULL) {  // empty slot
            clients[i] = new_client;
            pthread_mutex_unlock(&clients_mutex);
            return i;  // Success
        }
    }

    return -1;  // Shouldn't reach here if array is properly checked
}

// remove a client (disconnect) based on their socket
void remove_client(int index) {
	pthread_mutex_lock(&clients_mutex); 
	close(clients[index]->client_socket); // close client socket
	free_client_mem(clients[index])// free dynamically allocated memory
	clients[index] = NULL;
	pthread_mutex_unlock(&clients_mutex);
	pthread_cond_signal(&clients_cond);
}

// Helper function to get the index of a client in the clients array
int get_client_index(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        if (clients[i]->client_socket && *clients[i]->client_socket == client_socket) {
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

// 0-text, -1-exit,  n-whisper with name with length n
int check_message_type(const char *input_string) {
    // check exit
    if (strcmp(input_string, "!exit") == 0) {
        return -1;
    }
    
    // check whisper
    // A MESSAGE "@name\0" or "@name \0" DOESNT COUNT AS A MESSAGE
    if (input_string[0] == '@') {
        int i = 1;
        // Ensure that the name is at least one character long and consists of letters/numbers
        while (input_string[i] != ' ' && input_string[i] != '\0') {
            i++;
        }
        
        // A MESSAGE @name or 
        // check for space after the name and text afterwards
        if (input_string[i] == ' ' && input_string[i + 1] != '\0') {
            return i;
        }
    }
    
    // Default case: if none of the above, return 1 for any other text
    return 0;
}

int find_client_index_by_name(char* name) {
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0) {
            return i; 
        }
    }
    return -1;  // client not found
}

