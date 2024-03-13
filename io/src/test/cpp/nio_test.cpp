#include "gtest/gtest.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "io.h"

#define MAX_CLIENTS 10

static void *handle_client_func(void *socket_fd_ptr) {
    while (true) {
        handle_client((int) (long) socket_fd_ptr);
    }
    return socket_fd_ptr;
}

static void start_server() {
    int socket_fd = listen_server();

    set_fd_nonblocking(socket_fd);
    int client_index = 0, client_sockets[MAX_CLIENTS];
    // 循环等待连接
    while (true) {
        int client_socket = accept_client(socket_fd);
        if (client_socket != -1) {
            set_fd_nonblocking(client_socket);
            client_sockets[client_index++] = client_socket;
        }

        for (int client: client_sockets) {
            if (client != 0) {
                handle_client(client);
            }
        }
    }
}

static void *start_server_func(void *arg) {
    start_server();
    return arg;
}

TEST(io, nio) {
    pthread_t thread;
    pthread_create(&thread, nullptr, start_server_func, nullptr);
    usleep(1000);//等待服务端完成监听
    start_client();
    start_client();
}

