#!/bin/bash

# Variables
IMAGE_FILE="./storage_vgc.img"
MOUNT_DIR="./mount"
SYMLINK_DEVICE="./my_device"  # The symbolic link for the device file in the same directory as the script
MOUNT_POINT="${MOUNT_DIR}/virtual_disk"  # This is where the disk will be mounted

# Function to check for errors and exit if any
check_error() {
    if [ $? -ne 0 ]; then
        echo "Error occurred. Exiting."
        exit 1
    fi
}

# Step 1: Create the mount directory if it doesn't exist
if [ ! -d "$MOUNT_DIR" ]; then
    echo "Creating mount directory $MOUNT_DIR..."
    sudo mkdir -p $MOUNT_DIR
    check_error
fi

# Step 2: Create the virtual disk mount point directory if it doesn't exist
if [ ! -d "$MOUNT_POINT" ]; then
    echo "Creating mount point directory $MOUNT_POINT..."
    sudo mkdir -p $MOUNT_POINT
    check_error
fi

# Step 3: Set up the loop device for the disk image file
echo "Setting up loop device for image file $IMAGE_FILE..."
LOOP_DEVICE=$(sudo losetup --find --show $IMAGE_FILE)
check_error

# Step 4: Inform the kernel of the partition table changes
echo "Informing kernel about the partition table..."
sudo partprobe $LOOP_DEVICE
check_error

# Step 5: Mount the partition
PARTITION="${LOOP_DEVICE}p1"
echo "Mounting the partition $PARTITION at $MOUNT_POINT..."
sudo mount $PARTITION $MOUNT_POINT
check_error

# Step 6: Create the symbolic link for the device file
echo "Creating symbolic link for the loop device at $SYMLINK_DEVICE..."
sudo ln -sf $LOOP_DEVICE $SYMLINK_DEVICE
check_error

echo "Virtual disk setup complete. The virtual disk is mounted at $MOUNT_POINT."

