#!/bin/bash

KERNEL=/home/mateusz/zso/linux-5.5.5/arch/x86/boot/bzImage
QEMU_PATH=$PWD

${QEMU_PATH}/qemu/x86_64-softmmu/qemu-system-x86_64 \
-kernel ${KERNEL} \
-drive file=zso2020.qcow2,if=none,id=drive0 \
-append root=/dev/sda1 \
-device virtio-scsi-pci,id=scsi0 \
-device scsi-hd,bus=scsi0.0,drive=drive0 \
-enable-kvm \
-smp 4 \
-m 1G \
-net nic,model=virtio -net user \
-net user,hostfwd=tcp::2222-:22 \
-fsdev local,id=hshare,path=hshare/,security_model=none \
-device virtio-9p-pci,fsdev=hshare,mount_tag=hshare \
-device adlerdev \
-chardev stdio,id=cons,signal=off -device virtio-serial-pci \
-device virtconsole,chardev=cons \
-soundhw hda \
-usb -device usb-mouse
