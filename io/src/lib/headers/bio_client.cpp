#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "bio.h"
#include <netinet/tcp.h>

#define SERVER_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 256

void printf_message(int socket, char *message) {
    printf("Client Received message:\n%s\n", message);
}

void start_client(char *message) {
    int client = start_client();

    printf("Client Send message:\n%s\n", message);
    send_message(client, message);

    receive_message(client, printf_message);
    stop_client(client);
}

int start_client() {
    printf("Start client!\n");

    // 创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if (client_socket == -1) {
        perror("Error creating Client");
        exit(EXIT_FAILURE);
    }

    // 设置连接超时时间 https://stackoverflow.com/questions/2597608/c-socket-connection-timeout
    int syn_retries = 100;
    setsockopt(client_socket, IPPROTO_TCP, TCP_SYNCNT, &syn_retries, sizeof(syn_retries));

    // 设置服务器地址结构
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_port = PORT;

    // 连接到服务器
    if (connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    return client_socket;
}

void stop_client(int socket) {
    printf("Stopping the client.\n");
    close(socket);
}





