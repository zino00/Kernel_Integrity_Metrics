#! bash
pushd ~/kernel/modules
make clean
make
cp kernel.ko ../linux-2.6.18/busybox-1.23.2
popd
pushd busybox-1.23.2/
find . | cpio -o --format=newc > ../rootfs.img
popd

qemu-system-x86_64 \
    -m 64M \
    -nographic \
    -kernel ./arch/x86_64/boot/bzImage \
    -initrd  ./rootfs.img \
    -append "root=/dev/ram console=ttyS0 oops=panic panic=1 nokaslr" \
    -smp cores=2,threads=1 \
    -cpu kvm64 \
    -gdb tcp::1234