# man：namespaces、unshare、nsenter、setns、mount、croot、switch_root


ns.crud.case: ns.ls.case ns.create.case ns.view.case;
ns.ls.case:
# 	查询 uts 命名空间
	lsns -t uts
ns.create.case:
# 	创建临时命名空间
	unshare --uts lsns -t uts | wc -l
	lsns -t uts | wc -l
# 	创建持久命名空间
	hostname
	touch $(BUILD)/uts-ns
	unshare --uts=$(BUILD)/uts-ns hostname "hostname-$$$$"
#	unshare: mount /proc/self/ns/uts on build/uts-ns
# 	进入持久命名空间
	nsenter --uts=$(BUILD)/uts-ns hostname
#	nsenter: reassociate to namespace 'ns/uts'
# 	删除持久命名空间
	umount $(BUILD)/uts-ns
	rm -rf $(BUILD)/uts-ns
ns.view.case:
# 	查看命名空间
	unshare --uts ls -l /proc/self/ns

ns.cgroup.case: clean/pid
	cat /proc/self/cgroup
	cat /proc/self/mountinfo
	ll /sys/fs/cgroup
	ll /proc/self/cpu
	make ns.cgroup.init
ns.cgroup.init: $(BUILD)/pid
	top -b -e m -n 1 -p $(shell cat $<)

	kill $(shell cat $<)
# 随意运行进程
$(BUILD)/pid: $(BUILD)
	echo $$(( $$$$+1 )) > $@ && while true; do echo "$$$$" > /dev/null; done &


# 观察命名空间。所有命名空间、已用命名空间
ns.case: $(BUILD)/1.ns $(BUILD)/$$$$.ns;
$(BUILD)/%.ns: $(BUILD); ls -l /proc/$*/ns >> $@

ns.mount.case::
	unshare --mount --fork

# 下面的命令创建了一个PID命名空间，使用——fork确保执行的命令是在一个子进程中执行的，这个子进程(作为命名空间中的第一个进程)的PID为1。
# --mount-proc选项确保同时创建一个新的挂载名称空间，并且挂载一个新的proc(5)文件系统，其中包含与新的PID名称空间相对应的信息。
# 当readlink(1)命令终止时，新的命名空间将被自动拆除。
ns.pid.case:
	readlink /proc/self
	unshare --pid --mount-proc 		  readlink /proc/self
	unshare --pid --mount-proc --fork readlink /proc/self
# 作为非特权用户，创建一个新的用户命名空间，将用户的凭据映射到命名空间内的根id
ns.user.case::
	id -u; id -g
	unshare --user --map-root-user \
                   sh -c 'whoami; cat /proc/self/uid_map /proc/self/gid_map'

#以下命令建立了一个持久的 mount 命名空间，由绑定 /root/namespaces/mnt 引用。
#为了确保绑定安装的创建成功，父目录（/root/namespaces）是一个绑定 mount 的传播类型未共享。
ns.mount.case::
	mkdir -p /root/namespaces
	mount --bind /root/namespaces /root/namespaces
	mount --make-private /root/namespaces
	touch /root/namespaces/mnt
	unshare --mount=/root/namespaces/mnt

ns.time.case:
	uptime -p
	unshare --time --boottime 300000000 uptime -p

