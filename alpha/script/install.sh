#!/bin/bash
# Ce script configure une image disque avec GRUB et démarre une VM avec QEMU

# Variables pour le projet
BUILD_DIR="build"
ROOTFS_DIR="$BUILD_DIR/my-rootfs"
DISK_IMG="$BUILD_DIR/disk.img"
GRUB_CFG="files/grub.cfg"
KERNEL_IMAGE="linux-6.11.5/arch/x86/boot/bzImage"
LOOP_DEVICE=""

# Installer les dépendances
sudo apt-get install grub-pc libelf-dev docker -y

# Création du répertoire build
mkdir -p "$BUILD_DIR" "$ROOTFS_DIR"

# Création et partitionnement de l'image disque
truncate -s 450M "$DISK_IMG"
/sbin/parted -s "$DISK_IMG" mktable msdos
/sbin/parted -s "$DISK_IMG" mkpart primary ext4 1 "100%"
/sbin/parted -s "$DISK_IMG" set 1 boot on

# Associer l'image disque à un périphérique de boucle
sudo losetup -Pf "$DISK_IMG"
LOOP_DEVICE=$(losetup -l | grep "$DISK_IMG" | grep -o 'loop[0-9]*')
LOOP_COUNT=$(losetup -l | grep "$DISK_IMG" | grep -o 'loop[0-9]*' | wc -l)

if [ "$LOOP_COUNT" -ne 1 ]; then
    echo "Error: Failed to find the loop device."
    exit 1
fi

# Formatage de la partition et montage du système de fichiers
sudo mkfs.ext4 "/dev/${LOOP_DEVICE}p1"
sudo mount "/dev/${LOOP_DEVICE}p1" "$ROOTFS_DIR"

# Construction de l'image Docker et exécution pour initialiser le rootfs
sudo docker build -t rootkit-vm -f Dockerfile .
sudo docker run --rm -v "$ROOTFS_DIR:/my-rootfs" rootkit-vm /scripts/init.sh

# Copier le noyau et le fichier de configuration GRUB dans le système de fichiers de la VM
sudo mkdir -p "$ROOTFS_DIR/boot/grub"
sudo cp "$KERNEL_IMAGE" "$ROOTFS_DIR/boot/vmlinuz"
sudo cp "$GRUB_CFG" "$ROOTFS_DIR/boot/grub/grub.cfg"

# Installation de GRUB
sudo grub-install --directory=/usr/lib/grub/i386-pc --boot-directory="$ROOTFS_DIR/boot" "/dev/${LOOP_DEVICE}"

# Copier les fichiers supplémentaires dans le système de fichiers de la VM
sudo mkdir -p "$ROOTFS_DIR/home"
sudo cp -r "$BUILD_DIR" "$ROOTFS_DIR/home"

# Nettoyage : démonter et détacher le périphérique de boucle
sudo umount "$ROOTFS_DIR"
sudo losetup -d "/dev/${LOOP_DEVICE}"

# Lancer l'image disque dans QEMU
qemu-system-x86_64 -hda "$DISK_IMG" -nographic
