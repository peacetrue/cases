package com.github.peacetrue.cases.reactive;

import com.github.peacetrue.test.SourcePathUtils;
import lombok.SneakyThrows;
import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousFileChannel;
import java.nio.channels.CompletionHandler;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

/**
 * @author peace
 **/
public class AIOTest {

    @Test
    @SneakyThrows
    void basic() {
        Path filePath = Paths.get(SourcePathUtils.getProjectAbsolutePath() + "/settings.gradle");
        // 打开异步文件通道
        AsynchronousFileChannel fileChannel = AsynchronousFileChannel.open(filePath, StandardOpenOption.READ);
        // 分配缓冲区
        ByteBuffer buffer = ByteBuffer.allocate(1024);
        // 读取文件
        fileChannel.read(buffer, 0, buffer, new CompletionHandler<Integer, ByteBuffer>() {
            @Override
            public void completed(Integer result, ByteBuffer attachment) {
                // 读取完成时的处理逻辑
                if (result == -1) {
                    // 读取到文件末尾
                    return;
                }

                // 处理读取的数据
                buffer.flip();
                byte[] data = new byte[buffer.remaining()];
                buffer.get(data);
                System.out.println(new String(data));

                // 清空缓冲区，准备下一次读取
                buffer.clear();
                // 继续异步读取
                fileChannel.read(buffer, 1024, buffer, this);
            }

            @Override
            public void failed(Throwable exc, ByteBuffer attachment) {
                // 读取失败时的处理逻辑
                exc.printStackTrace();
                try {
                    // 关闭文件通道
                    fileChannel.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
    }
}
