#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <atomic>
#include "bio.h"
#include <pthread.h>

void *handle_client(void *socket_fd_ptr);

void echo(int client_socket, char *message);

int server_socket;

void start_server() {
    printf("Start server!\n");

    // 创建套接字
    server_socket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if (server_socket == -1) {
        perror("Error creating server socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    struct sockaddr_in server_addr;// socket address input
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;// sin: socket input
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = PORT;

    // 绑定地址和端口
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding server socket");
        exit(EXIT_FAILURE);
    }

    // 监听连接。listen(int socket, int backlog);
    if (listen(server_socket, 5) == -1) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    // 循环等待连接
    while (true) {
        // 接受客户端连接
        int client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 处理客户端连接
        pthread_t thread;
        pthread_create(&thread, nullptr, handle_client, (void *) (long) client_socket);
    }
}

void *handle_client(void *client_socket) {
    while (true) {
        receive_message((int) (long) client_socket, echo);
    }
    return client_socket;
}

void echo(int client_socket, char *message) {
    printf_received_message(client_socket, message);
    // 如果接收到 "bye"，停止服务
    if (strcmp(message, "bye") == 0) {
        stop_server();
    } else {
        // 向客户端发送相同的数据
        send(client_socket, message, strlen(message), 0);
    }
}

void stop_server() {
    close(server_socket);
    printf("Stopping the server.\n");
}




