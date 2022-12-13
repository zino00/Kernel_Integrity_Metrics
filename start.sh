#! bash
pushd ~/kernel/Kernel_Integrity_Metrics/system_call_table

#compile kernel module
make

#compile netlink user program
gcc user.c -o ~/kernel/busybox-1.35.0/_install/user -static

#cp kernel module to kernel file system
cp netlink.ko ~/kernel/busybox-1.35.0/_install
cp change_scb.ko ~/kernel/busybox-1.35.0/_install

#clean work dir
make clean


popd
pushd  ~/kernel/busybox-1.35.0/_install
find . | cpio -o --format=newc > ../rootfs.img
popd

#starting kernel
qemu-system-x86_64 \
    -nographic \
    -kernel ~/kernel/linux-5.4.98/arch/x86/boot/bzImage \
    -initrd  ~/kernel/busybox-1.35.0/rootfs.img \
    -append "root=/dev/ram console=ttyS0 oops=panic panic=1 nokaslr" \
    -smp cores=2,threads=1 \
    -cpu kvm64 \
    -no-reboot \
    -gdb tcp::1234