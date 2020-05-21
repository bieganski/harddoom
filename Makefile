qemu:
	git clone https://github.com/bieganski/qemu
	cd qemu; git checkout uharddoom
	cd qemu; ./configure --target-list=x86_64-softmmu --enable-virtfs

all: qemu


# TODO do it as root
root:
	echo 'KERNEL=="kvm", NAME="%k", GROUP="kvm", MODE="0660"' >  /etc/udev/rules.d/65-kvm.rules
	udevadm control --reload-rules && udevadm trigger
	adduser `id -un` kvm
