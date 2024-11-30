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
    sudo mkdir -p $MOUNT_POINT>/dev/null 2>&1
    check_error
fi

# Create the loop device for the disk image file
LOOP_DEVICE=$(sudo losetup --find --show $IMAGE_FILE)>/dev/null 2>&1
check_error

# Check if the image already has a partition table
if sudo file -s $LOOP_DEVICE | grep -q "ext4"; then
    echo "Filesystem detected on $LOOP_DEVICE. Skipping partitioning and formatting." >/dev/null 2>&1
else
    # Create a new partition table (GPT or MBR) if no partition is detected
    sudo fdisk $IMAGE_FILE <<EOF>/dev/null 2>&1
g         # Create a GPT partition table
n         # Create a new partition
           # Accept default for partition number, first sector, and last sector
w         # Write changes
EOF
    check_error

    # Set up partition after creating it
    sudo partprobe $LOOP_DEVICE>/dev/null 2>&1
    check_error

    # Format the loop device directly (no partition) or the partition itself
    sudo mkfs.ext4 $LOOP_DEVICE>/dev/null 2>&1
    check_error
fi

# Mount the loop device
sudo mount $LOOP_DEVICE $MOUNT_POINT>/dev/null 2>&1
check_error

# Change ownership of the mount point to the current user
# This allows the user to copy files into the mounted directory
sudo chmod -R 777 $MOUNT_POINT >/dev/null 2>&1
check_error

# Create the symbolic link for the device file
sudo ln -sf $LOOP_DEVICE $SYMLINK_DEVICE>/dev/null 2>&1
check_error


