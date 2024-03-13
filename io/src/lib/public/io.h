#define PORT 8885
#define BUFFER_SIZE 256

int listen_server();

void set_fd_flags(int socket_fd, int flags);

void set_fd_nonblocking(int socket_fd);

void set_fd_async(int socket_fd);

int accept_client(int socket_fd);

void handle_client(int socket_fd);

void start_client();
