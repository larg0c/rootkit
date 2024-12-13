To install :
```bash
chmod +x scripts/install.sh
./scripts/install.sh
```

Requirements :

Having linux-6.11.5 folder in the root directory

If you want to use another kernel then change:
  install.sh sript the line 19 : sudo cp ./linux-6.11.5/arch/x86/boot/bzImage /tmp/my-rootfs/boot/vmlinuz
  Makefile in files/.rootkit : change the KERNEL_DIR


To code kernel module : 
 Put your C files in files/src in which there is a Makefile then do ```make```

To execute in kernel:
  After compiling use the ./update.sh to upload the file in your vm, then laucnh the vm
  In the vm as root you just need to do : ```insmod my_file.ko```
