# 并发读写文件
.SECONDARY:#保留中间过程文件
include build.common.mk
include build.elf.mk

all:write;

SIZE := $(shell echo $$((1024 * 1024 * 1024)))
$(BUILD)/result.csv: $(BUILD)/write.bin
	$< $(BUILD)/file $(SIZE) $@
write: $(BUILD)/result.csv
	ls -lah $(BUILD) | grep file
	cat $<
	command -v lsblk && lsblk --output NAME,MODEL,TYPE /dev/sda || true
