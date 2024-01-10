# 构建 ELF 文件：预处理、汇编、编译、链接
# 硬链接复用：ln -f $workingDir/learn-make/build.elf.mk build.elf.mk

# CFLAGS+=-g # 生成调试信息

# 预处理文件
$(BUILD)/%.i: $(SRC)/%.c $(BUILD)
	$(CC) $(CFLAGS) -E -o $@ $<
$(BUILD)/%.i: $(SRC)/%.cpp $(BUILD)
	$(CXX) $(CFLAGS) -E -o $@ $<
# 汇编文件
$(BUILD)/%.s: $(BUILD)/%.i
	$(CC) $(CFLAGS) -S -o $@ $<
# 对象文件
$(BUILD)/%.o: $(BUILD)/%.s
	$(CC) $(CFLAGS) -c -o $@ $<

# 默认直接构建可执行文件，使用 SECONDARY 指定为间接构建
DEBUG_VARS+=SECONDARY
ifndef SECONDARY
# 直接构建可执行文件，从源文件编译
$(BUILD)/%.bin: $(SRC)/%.c $(BUILD)
	$(CC) $(CFLAGS)    -o $@ $<
$(BUILD)/%.bin: $(SRC)/%.cpp $(BUILD)
	$(CXX) $(CFLAGS)   -o $@ $<
else
# 间接构建可执行文件，从对象文件编译
$(BUILD)/%.bin: $(BUILD)/%.o
	$(CC) $(CFLAGS)    -o $@ $<
endif

%.bin: $(BUILD)/%.bin;
