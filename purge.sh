#!/bin/bash

# Variables
IMAGE_FILE="./storage_vgc.img"
MOUNT_DIR="./"
SYMLINK_DEVICE="./my_device"  # The symbolic link for the device file
MOUNT_POINT="${MOUNT_DIR}/mount"

# Function to check for errors and exit if any
check_error() {
    if [ $? -ne 0 ]; then
        echo "Error occurred. Exiting."
        exit 1
    fi
}

#  Unmount the partition if it's mounted
if mountpoint -q $MOUNT_POINT; then
    sudo umount $MOUNT_POINT
    check_error
fi

# Remove the mounted virtual disk directory (optional, only if needed)
sudo rm -rf $MOUNT_POINT
check_error

# Detach the loop device (if it is still attached) using symbolic link
if [ -L "$SYMLINK_DEVICE" ]; then
    # Resolve the symlink to the actual device file
    LOOP_DEVICE=$(readlink -f "$SYMLINK_DEVICE")
    
    # Check if the device is a loop device
    if [[ "$LOOP_DEVICE" == /dev/loop* ]]; then
        # Detach the loop device
        sudo losetup -d $LOOP_DEVICE
        check_error
    fi
fi

#  Remove the symbolic link for the device file
if [ -L "$SYMLINK_DEVICE" ]; then

    sudo rm -f $SYMLINK_DEVICE
    check_error

fi

# Remove the disk image file
if [ -f "$IMAGE_FILE" ]; then
    sudo rm -f $IMAGE_FILE
    check_error
fi

