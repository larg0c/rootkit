#!/bin/bash
# Ce script met à jour les fichiers dans l'image disque sans reconfiguration complète

# Variables pour le projet
BUILD_DIR="build"
DISK_IMG="$BUILD_DIR/disk.img"
ROOTFS_DIR="/tmp/my-rootfs"
LOOP_DEVICE=""

# Associer l'image disque à un périphérique de boucle
sudo losetup -Pf "$DISK_IMG"
LOOP_DEVICE=$(losetup -l | grep "$DISK_IMG" | grep -o 'loop[0-9]*')

if [ -z "$LOOP_DEVICE" ]; then
    echo "Error: Failed to find the loop device."
    exit 1
fi

# Monter la partition du disque
sudo mkdir -p "$ROOTFS_DIR"
sudo mount "/dev/${LOOP_DEVICE}p1" "$ROOTFS_DIR"

# Copier les fichiers depuis le dossier 'files' vers le système de fichiers de la VM
sudo cp -r "$BUILD_DIR" "$ROOTFS_DIR/"

# Nettoyage : démonter et détacher le périphérique de boucle
sudo umount "$ROOTFS_DIR"
sudo losetup -d "/dev/$LOOP_DEVICE"

echo "Update complete: files copied to the disk image."
