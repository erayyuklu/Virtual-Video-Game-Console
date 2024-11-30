#!/bin/bash

# Variables
IMAGE_FILE="./storage_vgc.img"
MOUNT_DIR="./"
SYMLINK_DEVICE="./my_device"  # The symbolic link for the device file in the same directory as the script
MOUNT_POINT="${MOUNT_DIR}/mount"  # This is where the disk will be mounted

# Function to check for errors and exit if any
check_error() {
    if [ $? -ne 0 ]; then
        echo "Error occurred. Exiting."
        exit 1
    fi
}



# Create the virtual disk mount point directory if it doesn't exist
if [ ! -d "$MOUNT_POINT" ]; then
    sudo mkdir -p $MOUNT_POINT
    check_error
fi

#  Set up the loop device for the disk image file
LOOP_DEVICE=$(sudo losetup --find --show $IMAGE_FILE)
check_error

#  Inform the kernel of the partition table changes
sudo partprobe $LOOP_DEVICE
check_error

#  Mount the partition
PARTITION="${LOOP_DEVICE}p1"
sudo mount $PARTITION $MOUNT_POINT
check_error

# Create the symbolic link for the device file
sudo ln -sf $LOOP_DEVICE $SYMLINK_DEVICE
check_error


