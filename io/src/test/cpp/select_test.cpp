#include "gtest/gtest.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "io.h"

static void start_server() {
    int socket_fd = listen_server();
    set_fd_nonblocking(socket_fd);

    fd_set active_fds;
    FD_ZERO(&active_fds);
    FD_SET(socket_fd, &active_fds);
    // 循环等待连接
    while (true) {
        fd_set read_fds = active_fds;
        // 使用 select 检查有活动的文件描述符
        if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Select failed");
            exit(EXIT_FAILURE);
        }

        // 处理每个活动的文件描述符
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == socket_fd) {
                    // 接受客户端连接
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int client_socket = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);
                    assert(client_socket != -1);
                    printf("server: accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port));
                    set_fd_nonblocking(client_socket);
                    FD_SET(client_socket, &active_fds);
                } else {
                    handle_client(i);
                }
            }
        }

    }
}

static void *start_server_func(void *arg) {
    start_server();
    return arg;
}

TEST(io, select) {
    pthread_t thread;
    pthread_create(&thread, nullptr, start_server_func, nullptr);
    usleep(1000);//等待服务端完成监听
    start_client();
    start_client();
}

