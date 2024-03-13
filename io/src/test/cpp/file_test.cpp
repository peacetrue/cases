//#include "gtest/gtest.h"
#include <fcntl.h>

#define BUFFER_SIZE 512

// 标准 IO #include <stdio.h>
static const char *read_buffer(FILE *file) {
    char *content = (char *) malloc(BUFFER_SIZE);
    memset(content, 0, BUFFER_SIZE);
    size_t size = fread(content, sizeof(char), BUFFER_SIZE, file);
    printf("read file buffer(%zu): '%s'\n", size, content);
    return content;
}

// 高级 IO #include <fcntl.h>
// 直接 IO 提速 https://medium.com/@caglayandokme/enhancing-the-file-access-speed-via-direct-i-o-in-linux-f18a46af068f
static const char *read_direct(const char *filename) {
    int fd = open(filename, O_RDONLY | O_DIRECT);
    char *content = (char *) malloc(BUFFER_SIZE);
    memset(content, 0, BUFFER_SIZE);
    size_t size = read(fd, content, BUFFER_SIZE);
    printf("read file directly(%zu): '%s'\n", size, content);
    close(fd);
    return content;
}

// ../../../src/test/cpp/file_test.cpp
TEST(FILE_IO, SYNC) {
    const char *filename = "file.txt";
    FILE *file = fopen(filename, "w+");// 读|写|创建|截断
    ASSERT_EQ(file->_fileno, STDERR_FILENO + 1);// 文件描述符为 3
    ASSERT_EQ(0, strlen(read_buffer(file))); // 文件内容为空

    {
        const char content[] = "Hello World!";
        size_t size = fwrite(content, sizeof(char), strlen(content), file);
        printf("write file buffer(%zu): '%s'\n", size, content);
    }

    ASSERT_EQ(0, strlen(read_direct(filename))); // 文件内容为空（未写入文件）
    fseek(file, 0, SEEK_SET);
    ASSERT_LT(0, strlen(read_buffer(file))); // 文件内容不为空（已写入内核缓存）

    fflush(file);
    ASSERT_LT(0, strlen(read_direct(filename))); // 文件内容不为空（已写入文件）

    fclose(file);
}

#include <sys/mman.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main1(int argc, char *argv[]) {


    if (argc < 3 || argc > 4) {
        fprintf(stderr, "%s file offset [length]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        handle_error("open");

    struct stat sb;
    if (fstat(fd, &sb) == -1)           /* To obtain file size */
        handle_error("fstat");

    off_t offset = atoi(argv[2]);

    off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    /* offset for mmap() must be page aligned */

    if (offset >= sb.st_size) {
        fprintf(stderr, "offset is past end of file\n");
        exit(EXIT_FAILURE);
    }

    size_t length;
    if (argc == 4) {
        length = atoi(argv[3]);
        if (offset + length > sb.st_size)
            length = sb.st_size - offset;
        /* Can't display bytes past end of file */

    } else {    /* No length arg ==> display to end of file */
        length = sb.st_size - offset;
    }

    char *addr = (char *) mmap(NULL, length + offset - pa_offset, PROT_READ, MAP_PRIVATE, fd, pa_offset);
    if (addr == MAP_FAILED)
        handle_error("mmap");

    ssize_t s = write(STDOUT_FILENO, addr + offset - pa_offset, length);
    if (s != length) {
        if (s == -1)
            handle_error("write");

        fprintf(stderr, "partial write");
        exit(EXIT_FAILURE);
    }

    munmap(addr, length + offset - pa_offset);
    close(fd);

    exit(EXIT_SUCCESS);
}

TEST(FILE_IO, MMAP) {
    const char *file_path = "mmap.txt";
    const char *data_to_write = "Hello, mmap!";

    // 打开文件以读写方式
    int fd = open(file_path, O_RDWR | O_CREAT, (mode_t) 0600);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // 获取文件大小
    off_t file_size = lseek(fd, 0, SEEK_END);
    printf("file_size: '%ld'\n", file_size);


    // 调整文件大小
    if (ftruncate(fd, file_size + strlen(data_to_write)) == -1) {
        perror("Error truncating file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 映射文件到内存
    char *mapped_data = (char *) mmap(NULL, file_size + strlen(data_to_write), PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                                      0);
    printf("mapped_data: '%s'\n", mapped_data);
    if (mapped_data == MAP_FAILED) {
        perror("Error mapping file to memory");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 将数据写入映射区域
    strcat(mapped_data, data_to_write);
    printf("mapped_data2: '%s'\n", mapped_data);

    // 刷新映射区域到文件
    if (msync(mapped_data, file_size + strlen(data_to_write), MS_SYNC) == -1) {
        perror("Error syncing file to disk");
    }

    // 解除内存映射
    if (munmap(mapped_data, file_size + strlen(data_to_write)) == -1) {
        perror("Error unmapping file from memory");
    }

    ASSERT_LT(0, strlen(read_direct(file_path))); // 文件内容不为空（已写入文件）

    // 关闭文件
    close(fd);
}