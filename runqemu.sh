#!/bin/bash

KERNEL=/home/mateusz/zso/linux-5.5.5/arch/x86/boot/bzImage
QEMU_PATH=$PWD


# -kernel ${KERNEL} \
# -append root=/dev/sda1 \
# TODO - graphical window support,
${QEMU_PATH}/qemu/x86_64-softmmu/qemu-system-x86_64 \
-display sdl \
-drive file=zso2020.qcow2,if=none,id=drive0 \
-device virtio-scsi-pci,id=scsi0 \
-device scsi-hd,bus=scsi0.0,drive=drive0 \
-enable-kvm \
-smp 4 \
-m 1G \
-net nic,model=virtio \
-net user \
-fsdev local,id=hshare,path=hshare/,security_model=none \
-device virtio-9p-pci,fsdev=hshare,mount_tag=hshare \
-device adlerdev \
-device uharddoom \
-device uharddoom \
-chardev stdio,id=cons,signal=off -device virtio-serial-pci \
-device virtconsole,chardev=cons \
-soundhw hda \
-usb -device usb-mouse


# -netdev user,id=user.0,hostfwd=tcp::8080-:80 \
# -net tap,ifname=tap0,script=no \
# -net user,hostfwd=tcp::2222-:22 \
# -netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no \
# -device e1000,netdev=mynet0,mac=52:55:00:d1:55:01 \
