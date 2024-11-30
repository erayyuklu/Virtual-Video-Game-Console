#!/bin/bash

# Variables
IMAGE_FILE="./storage_vgc.img"
MOUNT_DIR="./mount"
SYMLINK_DEVICE="./my_device"  # The symbolic link for the device file
MOUNT_POINT="${MOUNT_DIR}/virtual_disk"

# Function to check for errors and exit if any
check_error() {
    if [ $? -ne 0 ]; then
        echo "Error occurred. Exiting."
        exit 1
    fi
}

# Step 1: Unmount the partition if it's mounted
echo "Unmounting the partition at $MOUNT_POINT..."
if mountpoint -q $MOUNT_POINT; then
    sudo umount $MOUNT_POINT
    check_error
else
    echo "$MOUNT_POINT is not mounted."
fi

# Step 2: Remove the mounted virtual disk directory (optional, only if needed)
echo "Removing mount point directory $MOUNT_POINT..."
sudo rm -rf $MOUNT_POINT
check_error

# Step 3: Detach the loop device (if it is still attached)
LOOP_DEVICE=$(sudo losetup --find --show $IMAGE_FILE)
if [ -n "$LOOP_DEVICE" ]; then
    echo "Detaching loop device $LOOP_DEVICE..."
    sudo losetup -d $LOOP_DEVICE
    check_error
else
    echo "No loop device found for the image."
fi

# Step 4: Remove the symbolic link for the device file
if [ -L "$SYMLINK_DEVICE" ]; then
    echo "Removing symbolic link for the device file at $SYMLINK_DEVICE..."
    sudo rm -f $SYMLINK_DEVICE
    check_error
else
    echo "No symbolic link found at $SYMLINK_DEVICE."
fi

echo "Termination completed. The virtual disk has been unmounted and all associated files have been removed."

