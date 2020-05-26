.PHONY: qemu

qemu:
	# git clone https://github.com/bieganski/qemu
	cd qemu; git checkout uharddoom
	cd qemu; ./configure --target-list=x86_64-softmmu --enable-virtfs
	cd qemu; make

prboom:
	git clone https://github.com/bieganski/prboom-plus
	sudo apt install libsdl1.2-dev libsdl-mixer1.2-dev libsdl-net1.2-dev
	cd prboom-plus; git checkout udoomdev

all: qemu prboom


# TODO do it as root
root:
	echo 'KERNEL=="kvm", NAME="%k", GROUP="kvm", MODE="0660"' >  /etc/udev/rules.d/65-kvm.rules
	udevadm control --reload-rules && udevadm trigger
	adduser `id -un` kvm
