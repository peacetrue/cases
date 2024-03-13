#include "gtest/gtest.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "io.h"

#define MAX_EVENTS 10


static void start_server() {
    int socket_fd = listen_server();
    set_fd_nonblocking(socket_fd);

    // 创建 epoll 实例
    int epoll_fd = epoll_create1(0);
    assert(epoll_fd != -1);

    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    int ctl = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    assert(ctl != -1);
    // 循环等待连接
    struct epoll_event events[MAX_EVENTS];
    while (true) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        assert(num_events != -1);
        // 处理每个活动的文件描述符
        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == socket_fd) {
                // 接受客户端连接
                int client_socket = accept_client(socket_fd);
                assert(client_socket != -1);
                set_fd_nonblocking(client_socket);
                event.events = EPOLLIN | EPOLLET;  // 使用边缘触发模式
                event.data.fd = client_socket;
                int epollCtl = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event);
                assert(epollCtl != -1);
            } else {
                handle_client(events[i].data.fd);
            }
        }

    }
}

static void *start_server_func(void *arg) {
    start_server();
    return arg;
}

TEST(nio, epoll) {
    pthread_t thread;
    pthread_create(&thread, nullptr, start_server_func, nullptr);
    usleep(1000);//等待服务端完成监听
    start_client();
    start_client();
}

