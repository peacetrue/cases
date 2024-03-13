#include "gtest/gtest.h"
#include <fcntl.h>
#include "io.h"
// 信号被处理后，当前进程是否还能执行： https://stackoverflow.com/questions/14233464/can-a-c-program-continue-execution-after-a-signal-is-handled
// https://stackoverflow.com/questions/19866754/sigio-arriving-for-file-descriptors-i-did-not-set-it-for-and-when-no-io-is-possi

static int server_socket_fd;

// 信号处理函数
static void handle_signal(int sig_no, siginfo_t *info, void *context) {
    if (sig_no == SIGIO) {
        int socket_fd = info->si_fd;
        if (server_socket_fd == socket_fd) {
            int client_socket_fd = accept_client(server_socket_fd);
            assert(client_socket_fd != -1);
            set_fd_async(client_socket_fd);
        } else {
            handle_client(socket_fd);
        }
    }
}

static void start_server() {
    server_socket_fd = listen_server();
    set_fd_async(server_socket_fd);

    // 注册信号处理函数
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = handle_signal;
    sigaction(SIGIO, &sa, NULL);
}

TEST(io, sigio) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    start_server();
    start_client();
    start_client();
}

