#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>

#define PORT 8881
#define INPUT_SIZE 1024
#define BUFFER_SIZE 256

extern int server_socket;

void start_server();

void stop_server();

int start_client();

void stop_client(int socket);

void send_message(int socket, char *message);

void receive_message(int socket, void (*handle_message)(int socket, char *message));

void printf_received_message(int socket, char *message);

void start_client(const char *message);
