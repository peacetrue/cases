container.test: container.init.a;

CONTAINERS_DIR=$(BUILD)/containers
# 容器初始化，%=容器名
container.init.%: image.init $(CONTAINERS_DIR)/%;
$(CONTAINERS_DIR)/%:
#	如果模块 overlay 未加载，加载 overlay 模块。overlay 模块用于支持联合文件系统
	if ! command -v modinfo overlay > /dev/null; then modprobe overlay; fi
	mkdir -p $@/workdir $@/tmp
#	-t 文件系统类型为 overlay，-o 指定 overlay 文件系统的配置
	sudo mount -t overlay -o lowerdir=$(BUILD)/$(MINIROOTFS),upperdir=$@/workdir,workdir=$@/tmp overlay $(BUILD)/$(MINIROOTFS)


container.login:
	chroot $(BUILD)/$(MINIROOTFS) $(SHELL) --login

MINIROOTFS=alpine-minirootfs-3.11.0-x86_64
image.init: image.install;
# 安装镜像
image.install: $(BUILD)/$(MINIROOTFS);
# 解压 rootfs 文件
$(BUILD)/$(MINIROOTFS): $(BUILD)/$(MINIROOTFS).tar.gz
	if [ ! -f $@ ]; then  mkdir -p $@; $(SUDO_HOST) tar -xzf $< -C $@; fi
# 如果 rootfs 不存在，下载 rootfs
$(BUILD)/$(MINIROOTFS).tar.gz: $(BUILD)
	if [ ! -f $@ ]; then wget -o $@.log -O $@ https://mirrors.tuna.tsinghua.edu.cn/alpine/v3.11/releases/x86_64/$(MINIROOTFS).tar.gz; fi

