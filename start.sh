#! bash
pushd ~/kernel/Kernel_Integrity_Metrics
make clean
make
gcc user.c -o user -static
cp netlink.ko ~/kernel/busybox-1.35.0/_install
cp user ~/kernel/busybox-1.35.0/_install
# cp getsys.ko ~/kernel/busybox-1.35.0/_install
popd
pushd  ~/kernel/busybox-1.35.0/_install
find . | cpio -o --format=newc > ../rootfs.img
popd

sleep 3
qemu-system-x86_64 \
    -nographic \
    -kernel ~/kernel/linux-5.4.98/arch/x86/boot/bzImage \
    -initrd  ~/kernel/busybox-1.35.0/rootfs.img \
    -append "root=/dev/ram console=ttyS0 oops=panic panic=1 nokaslr" \
    -smp cores=2,threads=1 \
    -cpu kvm64 \
    -no-reboot
    -gdb tcp::1234