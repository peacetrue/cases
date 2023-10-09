# 观察 vbox 虚拟机

vbox/%:; make $*.vbox BUILD=$(BUILD)/vbox
%.vbox:$(BUILD)/%.vbox;
$(BUILD)/%.vbox: $(BUILD)
	ip link > $@.link
	ip addr > $@.addr
	ip route > $@.route

