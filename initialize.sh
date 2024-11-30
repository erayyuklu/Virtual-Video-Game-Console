#!/bin/bash

# Variables
VIRTUAL_DISK_IMAGE="storage_vgc.img"
DISK_SIZE_MB=20  # Disk size in MB
BIN_DIR="./bin"  # Directory where the game files are located

# Function to check for errors and exit if any
check_error() {
    if [ $? -ne 0 ]; then
        echo "Error occurred. Exiting."
        exit 1
    fi
}

#  Check if the disk image already exists and delete if needed
if [ -f $VIRTUAL_DISK_IMAGE ]; then
    rm $VIRTUAL_DISK_IMAGE
fi

# Create a new disk image
dd if=/dev/zero of=$VIRTUAL_DISK_IMAGE bs=1M count=$DISK_SIZE_MB  >/dev/null 2>&1
check_error

# Associate the disk image with a loop device
LOOP_DEVICE=$(sudo losetup --find --show $VIRTUAL_DISK_IMAGE) >/dev/null 2>&1
check_error

# Create a partition table and partition
sudo parted $LOOP_DEVICE mklabel msdos >/dev/null 2>&1
sudo parted $LOOP_DEVICE mkpart primary ext4 1MiB 100% >/dev/null 2>&1
check_error

# Inform the kernel of the partition changes
sudo partprobe $LOOP_DEVICE >/dev/null 2>&1
check_error

#Format the partition as ext4"
sudo mkfs.ext4 ${LOOP_DEVICE}p1 >/dev/null 2>&1
check_error

# Mount the partition temporarily to copy files
TEMP_MOUNT_DIR=$(mktemp -d)
sudo mount ${LOOP_DEVICE}p1 $TEMP_MOUNT_DIR >/dev/null 2>&1
check_error

# Copy files from BIN_DIR to the mounted partition
if [ -d "$BIN_DIR" ]; then
    sudo cp -r $BIN_DIR/* $TEMP_MOUNT_DIR >/dev/null 2>&1
    check_error
fi

# Unmount the partition
sudo umount $TEMP_MOUNT_DIR
check_error

# Detach the loop device
sudo losetup -d $LOOP_DEVICE
check_error

# Clean up temporary mount directory
rmdir $TEMP_MOUNT_DIR


