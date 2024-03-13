#include "gtest/gtest.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "io.h"

static void *handle_client_func(void *socket_fd_ptr) {
    handle_client((int) (long) socket_fd_ptr);
    return socket_fd_ptr;
}

static void start_server() {
    int socket_fd = listen_server();

    // 循环等待连接
    while (true) {
        // 接受客户端连接
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }

        printf("server: accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // 处理客户端连接
        pthread_t thread;
        pthread_create(&thread, nullptr, handle_client_func, (void *) (long) client_socket);
    }
}

static void *start_server_func(void *arg) {
    start_server();
    return arg;
}


TEST(io, bio) {
    pthread_t thread;
    pthread_create(&thread, nullptr, start_server_func, nullptr);
    usleep(1000);//等待服务端完成监听
    start_client();
    start_client();
}

