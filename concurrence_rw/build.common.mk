# 构建通用文件，声明构建目录的创建和清除
# 硬链接复用：ln -f $workingDir/learn-make/build.common.mk build.common.mk
override src:=src#源始文件目录，作为常量使用，请勿改变该值
SRC?=$(src)#作为变量使用，兼容前置 SRC 的场景
DEBUG_VARS+=src SRC

override build:=build#目标文件目录，作为常量使用，请勿改变该值
BUILD:=$(build)#作为变量使用
DEBUG_VARS+=build BUILD

mkdir: mkdir/$(BUILD);
mkdir/%:; mkdir -p $*
#所有不带食谱的规则，需要以分号结尾。去掉 $(BUILD)/% 后的分号，执行 make common.case，出现以下错误
# make: *** No rule to make target `build/test', needed by `common.case'.  Stop.
#$(BUILD): mkdir;#此写法导致依赖于 $(BUILD) 的目标每次都触发 mkdir -p build，当文件名与目标名相同时的选择策略为何？
$(BUILD):; mkdir -p $(BUILD) #此写法能触发文件缓存
#$(BUILD)/%: mkdir/$(BUILD)/%; #此目标导致 $(BUILD)/%.txt 类写法全部失效
$(SRC):; mkdir -p $(SRC) # 通常生成的文件存储在 BUILD 目录中，但 vdi 需要存储在 SRC 目录中

#如果是删除 build 目录，使用 -f 强制删除
rm/%:; rm -r$(if $(filter $(build)%,$*),f,) $*
rm: rm/$(BUILD);
clean: rm;
clean/%: rm/$(BUILD)/%;

# 从源目录拷贝到目标目录，统一依赖文件的位置
# EXTERNAL_SRC?=$(SRC)，通过 make 外部赋值会替换内部值，此处需要保留 SRC，此命令可能覆盖形如 $(BUILD)/%.txt 的命令
#$(foreach item,$(SRC) $(EXTERNAL_SRC),$(eval $(BUILD)/%: $(item)/% $(BUILD); cp $$< $$@))
$(foreach item, $(EXTERNAL_SRC),$(eval $(BUILD)/%: $(item)/% $(BUILD); cp $$< $$@))
DEBUG_VARS+=EXTERNAL_SRC

# 以 .cache 结尾的文件为缓存文件，实际无用途，表示该目标已执行过，无需再执行
#$(BUILD)/%.cache: $(BUILD)/%
	#touch $@

# 使用超级管理员执行。参数列表：1=密码
SUDO=/usr/bin/echo '$(1)' | /usr/bin/sudo -S
SUDO_SHORT=echo '$(1)' | sudo -S
# SUDO=sudo -S <<< $(US_PWD)

# 文件重置基准：备份文件存在，从备份文件恢复内容到原始文件；否则根据原始文件创建备份文件
# 参数列表：1=文件位置
FILE_REBASE=if [ -f $(1).bak ]; then sudo cp $(1).bak $(1); else sudo cp $(1) $(1).bak; fi
file_rebase/%:; $(call FILE_REBASE,$*)

