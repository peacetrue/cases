#include "io.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <csignal>

int listen_server() {
    printf("server: start\n");

    // 创建套接字
    int socket_fd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if (socket_fd == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    struct sockaddr_in addr;// socket address input
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;// sin: socket input
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = PORT;

    // 绑定地址和端口
    if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // 监听连接。listen(int socket, int backlog);
    if (listen(socket_fd, 5) == -1) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("server: listening on port %d...\n", PORT);
    return socket_fd;
}

void set_fd_flags(int socket_fd, int flags) {
    fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | flags);
}

void set_fd_nonblocking(int socket_fd) {
    set_fd_flags(socket_fd, O_NONBLOCK);
}

void set_fd_async(int socket_fd) {
    // 将服务器套接字设置为异步 I/O
    set_fd_flags(socket_fd, O_NONBLOCK | O_ASYNC);
    // siginfo_t.si_fd 指向触发信号的 fd
    fcntl(socket_fd, F_SETSIG, SIGIO);
    // 设置异步 I/O 所有者（进程ID）
    fcntl(socket_fd, F_SETOWN, getpid());
}

int accept_client(int socket_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);
    if (client_socket != -1) {
        printf("server: accepted connection from %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port)
        );
    }
    return client_socket;
}

void handle_client(int socket_fd) {
    char buffer[BUFFER_SIZE];
    // bytes_received 包含字符串末尾的 \0，即 hello 为 6。
    ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) return;
    printf("server: received '%s'\n", buffer);
    buffer[bytes_received - 1] = '!';
    printf("server: send '%s'\n", buffer);
    send(socket_fd, buffer, bytes_received, 0);
}

void start_client() {
    printf("------------\n");
    printf("client: start\n");

    // 创建套接字
    int socket_fd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if (socket_fd == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = PORT;

    // 连接到服务器
    if (connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("Error connecting to server");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("client: connected to server\n");
    usleep(1000); //等待服务端打印 'accepted connection from'

    for (int i = 0; i < 1; ++i) {
        char message[6] = "hello";
        printf("client: send '%s'\n", message);
        send(socket_fd, message, sizeof(message), 0);

        char buffer[BUFFER_SIZE];
        ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);
        printf("client: receive '%s'\n", buffer);
    }

    close(socket_fd);
}
