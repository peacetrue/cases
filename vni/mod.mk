run/%: ; $* $(ARGS) 2>&1 $(BACK)
log/%: ; $* $(ARGS) >$(BUILD)/$*.log 2>&1 $(BACK)
# 列出模块
mod.case: log/lsmod; make log/modprobe log/modinfo ARGS=tun
