#include "bio.h"

char *unpack(char *messages) {
    const char *position = strchr(messages, '\n');
    if (position == NULL) return NULL;

    long length = position - messages + 1; // 计算截取的长度

    char *result = (char *) malloc(length + 1);
    strncpy(result, messages, length);// 截取字符串（包括 \n）
    result[length] = '\0';// 确保结果字符串以 null 终止

    memmove(messages, position + 1, INPUT_SIZE - *position - 1);
    return result;
}

void receive_message(int socket, void (*handle_message)(int socket, char *message)) {
    // buffer 是一次读取到的内容，buffer 可能不是完整的语义信息；input 是累计读取到的内容
    char input[INPUT_SIZE] = {}, buffer[BUFFER_SIZE] = {};
    // 接收数据
    ssize_t bytes_received = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) return;
    printf("Received buffer:\n%s\n", buffer);

    // 将接收到的数据添加到末尾
    strncat(input, buffer, (size_t) bytes_received);
    printf("Current input:\n%s\n", input);

    while (true) {
        char *message = unpack(input);
        if (message == NULL) break;
        handle_message(socket, message);
    }
}

void send_message(int socket, char *message) {
    send(socket, message, strlen(message), 0);
}

void printf_received_message(int socket, char *message) {
    printf("Received message:\n%s\n", message);
}
