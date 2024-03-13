.SECONDARY:#保留中间过程文件
include build.common.mk
include build.elf.mk

all: $(BUILD)/data_alignment.bin $(BUILD)/datafile
	$< $(word 2,$^) &> $(BUILD)/result.csv

$(BUILD)/datafile:; dd if=/dev/urandom bs=1M count=10 | base64 > $@
