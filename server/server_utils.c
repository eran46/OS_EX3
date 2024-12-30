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

