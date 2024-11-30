#!/bin/bash

# Define paths for the disk image, mount point, and loop device
IMAGE_FILE="./storage_vgc.img"
MOUNT_DIR="./mount"
LOOP_DEVICE="/dev/loop0"  # Full path for the loop device
PARTITION="${LOOP_DEVICE}p1"  # The first partition on the loop device
MOUNT_POINT="${MOUNT_DIR}/virtual_disk"
BIN_DIR="./bin"

# Step 1: Detach the loop device if it is already in use
echo "Checking if loop device $LOOP_DEVICE is already in use..."
sudo losetup -d $LOOP_DEVICE 2>/dev/null

# Step 2: Set up the loop device
echo "Setting up loop device $LOOP_DEVICE..."
sudo losetup $LOOP_DEVICE $IMAGE_FILE
if [ $? -ne 0 ]; then
    echo "Failed to set up loop device. Exiting."
    exit 1
fi

# Step 3: Create a partition using fdisk
echo "Creating partition on the loop device..."
echo -e "o\nn\np\n1\n\n\nw\n" | sudo fdisk $LOOP_DEVICE

# Step 4: Inform the kernel of the partition changes
sudo partprobe $LOOP_DEVICE
if [ $? -ne 0 ]; then
    echo "Failed to inform the kernel about the new partition. Exiting."
    exit 1
fi

# Wait for a moment to ensure changes are registered
sleep 2

# Step 5: Verify that the partition exists
if [ ! -b "$PARTITION" ]; then
    echo "Partition $PARTITION does not exist. Exiting."
    exit 1
fi

# Step 6: Format the partition as ext4 forcefully without confirmation
echo "Formatting the partition as ext4..."
sudo mkfs.ext4 -F $PARTITION
if [ $? -ne 0 ]; then
    echo "Failed to format the partition. Exiting."
    exit 1
fi

# Step 7: Create the mount point if it doesn't exist
if [ ! -d "$MOUNT_POINT" ]; then
    echo "Creating mount point $MOUNT_POINT..."
    sudo mkdir -p $MOUNT_POINT
fi

# Step 8: Mount the partition
echo "Mounting the partition..."
sudo mount $PARTITION $MOUNT_POINT
if [ $? -ne 0 ]; then
    echo "Failed to mount the partition. Exiting."
    exit 1
fi

# Step 9: Copy files to the mounted partition
echo "Copying files from $BIN_DIR to $MOUNT_POINT..."
if [ -d "$BIN_DIR" ]; then
    sudo cp -r $BIN_DIR/* $MOUNT_POINT
    if [ $? -eq 0 ]; then
        echo "Files copied successfully to $MOUNT_POINT."
    else
        echo "Failed to copy files. Exiting."
        exit 1
    fi
else
    echo "$BIN_DIR does not exist. No files to copy."
fi

echo "Virtual disk setup complete with files from $BIN_DIR."

