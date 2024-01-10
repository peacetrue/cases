#include <unistd.h>

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>
#include <fcntl.h>

// 结果保存在 csv 文件中
char result[1024];

/** 求时间差（纳秒） */
long diff(struct timespec begin, struct timespec end) {
    return (end.tv_sec - begin.tv_sec) * 1000 * 1000 * 1000 + (end.tv_nsec - begin.tv_nsec);
}

/** 计算耗时 */
long time_consume(void *func(void *arg), void *arg) {
    struct timespec begin;
    clock_gettime(CLOCK_REALTIME, &begin);
    func(arg);
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    return diff(begin, end);
}

typedef struct file_s {
    char *name;// 文件名
    long offset; //写入偏移
    long length; //写入长度
    int parallel_count; //并行数量
} file_t;

typedef struct write_s {
    int fd;
    char *content;
    long length;
} write_t;

void *time_consume_write(void *arg) {
    write_t *w = (write_t *) arg;
    ssize_t size = write(w->fd, w->content, w->length);
    if (size == -1) perror("Error write");
    return arg;
}

void *sequential_write(void *arg) {
    file_t *file = (file_t *) arg;
    // 0644: file_t permissions (rw-r--r--)
    int fd = open(file->name, O_CREAT | O_WRONLY, 0644);
//    fcntl(fd, F_NOCACHE, 1); //无内核 buffer
    if (fd == -1) perror("Error fd");
    lseek(fd, file->offset, SEEK_SET);

    char *content = (char *) malloc(file->length);
    memset(content, '1', file->length);

    write_t w = {fd, content, file->length};
    long consume = time_consume(time_consume_write, &w);
    // 此处存在并发问题
    sprintf(result + strlen(result), ", %ld", consume);

    free(content);
    close(fd);
    return arg;
}

void *time_consume_sequential_write(void *arg) {
//    long consume = time_consume(sequential_write, arg);
//    sprintf(result + strlen(result), "| %ld", consume);
//    return arg;
    return sequential_write(arg);
}

void *parallel_write(void *arg) {
    file_t *file = (file_t *) arg;
    int count = file->parallel_count;
    pthread_t threads[count];
    for (int i = 0; i < count; ++i) {
        file_t *files = (file_t *) malloc(sizeof(file_t));
        files->name = file->name;
        files->length = file->length / count;
        files->offset = files->length * i;
        pthread_create(&threads[i], NULL, time_consume_sequential_write, files);
    }

    for (int i = 0; i < count; ++i) {
        pthread_join(threads[i], NULL);
    }

    return arg;
}

/**
 * 写入内容
 * argv[1]=写入文件名
 * argv[2]=文件字节数
 * argv[3]=结果文件名
 */
int main(int argc, char *argv[]) {
    int index = 1; // 从 1 号参数开始

    // 解析参数
    char *file_name = argc > index ? argv[index++] : "file";
    long length = argc > index ? strtol(argv[index++], NULL, 10) : 10 * 1024 * 1024L;
    char *result_name = argc > index ? argv[index++] : "result.csv";
    file_t file = {file_name, 0, length, 0};

    // 并行写入，从 1 核心（串行）到当前电脑最大核心
    // https://stackoverflow.com/questions/4586405/how-to-get-the-number-of-cpus-in-linux-using-c
    long processor_count = sysconf(_SC_NPROCESSORS_ONLN);
    for (int i = 1; i <= processor_count; ++i) {
        char name[strlen(file_name) + 2];
        strcpy(name, file_name);
        sprintf(name + strlen(name), "%d", i);
        file.name = name;
        file.parallel_count = i;
        sprintf(result + strlen(result), "file");
        sprintf(result + strlen(result), "%d", i);
        long parallel_time = time_consume(parallel_write, &file);
        sprintf(result + strlen(result), ", %ld\n", parallel_time);
    }

    //保存结果
    int fd = open(result_name, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    write(fd, result, strlen(result));
    return 0;
}

