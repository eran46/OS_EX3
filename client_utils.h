#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

void *receive_messages(void *socket_desc);

void clean_exit(int sock, pthread_t thread);

#endif
