DEV_NAME=$(DEV_TYPE)99
DEV_TYPE:=tun
IP_GATEWAY:=10.1.1.2
ip.tun.case: DEV_TYPE=tun
ip.tun.case: ip.tuntap.case;
ip.tap.case: DEV_TYPE=tap
ip.tap.case: ip.tuntap.case;
ip.tuntap.case: ip.tuntap.rebase
	make ip.tuntap.init.$(DEV_NAME) $(BUILD)/$(DEV_NAME).tcpdump.log ARGS="$(DEV_NAME) $(DEV_TYPE)"
	ping $(IP_GATEWAY)0 -w 3 || true
	killall tcpdump || true
	killall -15 tuntap.bin || true
ip.tuntap.rebase: clean/*.log;
ip.tuntap.init.%: ip.tuntap.del.% $(BUILD)/tuntap.bin.log
#	sudo ip tuntap add $* mode tun
	sudo ip addr add $(IP_GATEWAY)/24 dev $*
	sudo ip link set $* up
ip.tuntap.del.%:; sudo ip tuntap del mode $(DEV_TYPE) $*
$(BUILD)/%.bin.log: $(BUILD)/%.bin; $< $(ARGS) >$@ 2>&1 &
# tcpdump suppress console output：https://stackoverflow.com/questions/49226865/tcpdump-suppress-console-output-in-script-write-to-file
$(BUILD)/%.tcpdump.log:
	tcpdump -nnt -i $* -v > $@ 2>&1 &

# 删除 TUN/TAP 设备
ip.tuntap.del: ip.tun.del ip.tap.del;
ip.%.del: $(BUILD)/ip.%.list.name
	cat $< | xargs -I {} ip tuntap del mode $* {}
$(BUILD)/ip.%.list.name: $(BUILD)/ip.%.list
	cat $< | awk -F: '{print $$1}' > $@ || true
$(BUILD)/ip.%.list: $(BUILD)
	sudo ip tuntap list | grep $* > $@ || true
