#include "gtest/gtest.h"
#include <netinet/in.h>
#include <aio.h>
#include <fcntl.h>
#include "io.h"

// 异步I/O -- posix aio 从入门到放弃的吐血实践 https://zhuanlan.zhihu.com/p/112162403

static int server_socket_fd;
struct sigaction sa;
struct aiocb aiocb_i;

// 信号处理函数
void handle_signal(int sig_no, siginfo_t *info, void *context) {
    if (sig_no == SIGIO) {
        int socket_fd = info->si_fd;
        if (server_socket_fd == socket_fd) {
            int client_socket_fd = accept_client(server_socket_fd);
            assert(client_socket_fd != -1);

            char buf[BUFFER_SIZE];
            aiocb_i.aio_fildes = client_socket_fd;
            aiocb_i.aio_buf = buf;
            aiocb_i.aio_nbytes = BUFFER_SIZE;
            aiocb_i.aio_offset = 0; // start offset
            aiocb_i.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
            aiocb_i.aio_sigevent.sigev_signo = SIGIO;
            aiocb_i.aio_sigevent.sigev_value.sival_ptr = &sa;

            int ret = aio_read(&aiocb_i);
            if (ret < 0) {
                fprintf(stderr, "aio_read() errno=%d\n", errno);
                return;
            }
        } else {
            char *buffer = (char *) aiocb_i.aio_buf;
            printf("server: received '%s'\n", buffer);
//            buffer[strlen(buffer)] = '!';
            printf("server: send '%s'\n", buffer);
//            aio_write(&aiocb_i);
            send(socket_fd, buffer, strlen(buffer), 0);
        }
    }
    printf("11");
}


static void start_server() {
    server_socket_fd = listen_server();
    set_fd_async(server_socket_fd);

    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = handle_signal;
    sigaction(SIGIO, &sa, NULL);
}

TEST(io, aio) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    start_server();
    start_client();
//    start_client();
}

TEST(aio, block) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    char buf[BUFFER_SIZE];
    struct aiocb my;
    memset(&my, 0, sizeof(struct aiocb));
    int fd = open("/media/sf_cases/io/src/test/cpp/aio_test.cpp", O_RDONLY);
    my.aio_fildes = fd;
    my.aio_buf = buf;
    my.aio_nbytes = BUFFER_SIZE;
    my.aio_offset = 0;

    int ret = aio_read(&my);
    if (ret != 0)
        fprintf(stderr, "aio_read() failed. errno = %d\n", errno);

    while (aio_error(&my) == EINPROGRESS)
        sleep(1);

    printf("----------\n%s\n", (char *) (my.aio_buf));
}

struct aiocb my;
// 信号处理函数
void handle_signal_file(int sig_no, siginfo_t *info, void *context) {
    if (sig_no == SIGIO) {
        printf("----------\n%s\n", (char *) (my.aio_buf));
    }
}

TEST(aio, sigaction) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = handle_signal_file;
    sigaction(SIGIO, &sa, NULL);

    char buf[BUFFER_SIZE];
    memset(&my, 0, sizeof(struct aiocb));
    int fd = open("/media/sf_cases/io/src/test/cpp/aio_test.cpp", O_RDONLY);
    my.aio_fildes = fd;
    my.aio_buf = buf;
    my.aio_nbytes = BUFFER_SIZE;
    my.aio_offset = 0;
    my.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    my.aio_sigevent.sigev_signo = SIGIO;
    my.aio_sigevent.sigev_value.sival_ptr = &sa;

    int ret = aio_read(&my);
    if (ret != 0)
        fprintf(stderr, "aio_read() failed. errno = %d\n", errno);

    while (aio_error(&my) == EINPROGRESS)
        sleep(1);
}

