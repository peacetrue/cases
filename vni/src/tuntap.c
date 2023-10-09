#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <signal.h>

int dev_alloc(char *dev_name, short flags) {
    int fd;
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("open /dev/net/tun");
        return fd;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = flags;
    strcpy(ifr.ifr_ifrn.ifrn_name, dev_name);
    int err;
    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        perror("ioctl");
        close(fd);
        return err;
    }

    return fd;
}

/** 解析参数 */
char *resolve_arg(int argc, char **argv, int index, char *defaults) {
    return argc >= index + 1 ? argv[index] : defaults;
}

/** kill -15 刷新文件缓冲区：https://stackoverflow.com/questions/41841527/why-printf-does-not-output-to-a-file-when-stdout-is-redirected-to-that-file */
void flush_stdio() {
    fflush(stdout);
    fflush(stderr);
    exit(0);
}

void swap(char *array, int index, int length) {
    char temp[length];
    char *separator = array + index;
    memcpy(temp, separator, length);
    memcpy(separator, separator - length, length);
    memcpy(separator - length, temp, length);
}

int main(int argc, char **argv) {
    char *dev_name = resolve_arg(argc, argv, 1, "tun0");
    char *dev_type = resolve_arg(argc, argv, 2, "tun");
    printf("dev_name: %s, dev_type: %s\n", dev_name, dev_type);
    // 设置信号处理函数来监听 KILL -15 信号
    // -9 无法被处理：https://stackoverflow.com/questions/62896942/sigstop-on-c-invalid-argument
    if (signal(SIGTERM, flush_stdio) == SIG_ERR) {
        perror("Failed to set signal handler");
        flush_stdio();
        return 1;
    }

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *        IFF_NO_PI - Do not provide packet information
     */
    short dev_flags = strcmp(dev_type, "tun") == 0 ? IFF_TUN : IFF_TAP;
    int dev_fd = dev_alloc(dev_name, dev_flags | IFF_NO_PI);
    if (dev_fd < 0) exit(1);

    char buffer[4096];
    while (1) {
        // 收包
        ssize_t size = read(dev_fd, buffer, sizeof(buffer));
        if (size < 0) {
            perror("Reading from interface");
            close(dev_fd);
            exit(1);
        }

        printf("Read %zd bytes from %s device\n", size, dev_type);

        int base_index = 0;
        if (dev_flags == IFF_TAP) {
            // 以太网帧格式：https://en.wikipedia.org/wiki/Ethernet_frame
            // TODO unsupported
            swap(buffer, 14, 6);
            *((unsigned short *) &buffer[36 + 2]) += 8;
            base_index = 28; //以太网帧头末尾索引
        }
        // 简单对收到的包调换一下顺序
        // IP 头格式：https://en.wikipedia.org/wiki/Internet_Protocol_version_4#Header
        swap(buffer, base_index + 16, 4);
        // ICMP 头格式：https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#Control_messages
        buffer[base_index + 20] = 0;// Type=0：ping 应答
        buffer[base_index + 21] = 0;// Subtype=0：ping 应答
        // 校验和计算：https://www.rfc-editor.org/rfc/rfc1071 https://stackoverflow.com/questions/20247551/icmp-echo-checksum
        *((unsigned short *) &buffer[base_index + 22]) += 8;

        // 发包
        size = write(dev_fd, buffer, size);
        printf("Write %zd bytes to %s device\n", size, dev_type);
    }
    return 0;
}
