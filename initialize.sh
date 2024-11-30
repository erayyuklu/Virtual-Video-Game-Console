#!/bin/bash

# Variables
VIRTUAL_DISK_IMAGE="storage_vgc.img"
DISK_SIZE_MB=20  # Disk size in MB
MOUNT_POINT="mount"
BIN_DIR="./bin"  # Directory where the game files are located

# Function to check for errors and exit if any
check_error() {
    if [ $? -ne 0 ]; then
        echo "Error occurred. Exiting."
        exit 1
    fi
}

# Step 1: Check if the disk image already exists and delete if needed
if [ -f $VIRTUAL_DISK_IMAGE ]; then
    echo "Disk image already exists. Overriding it..."
    rm $VIRTUAL_DISK_IMAGE
fi

# Step 2: Create a new disk image
echo "Creating a $DISK_SIZE_MB MB disk image..."
dd if=/dev/zero of=$VIRTUAL_DISK_IMAGE bs=1M count=$DISK_SIZE_MB
check_error

# Step 3: Associate the disk image with a loop device
echo "Setting up loop device..."
LOOP_DEVICE=$(sudo losetup --find --show $VIRTUAL_DISK_IMAGE)
check_error

# Step 4: Create a partition table and partition
echo "Creating partition table and partition..."
sudo parted $LOOP_DEVICE mklabel msdos
sudo parted $LOOP_DEVICE mkpart primary ext4 1MiB 100%
check_error

# Step 5: Inform the kernel of the partition changes
echo "Informing kernel about the partition table..."
sudo partprobe $LOOP_DEVICE
check_error

# Step 6: Format the partition as ext4
echo "Formatting the partition as ext4..."
sudo mkfs.ext4 ${LOOP_DEVICE}p1
check_error

# Step 7: Mount the partition temporarily to copy files
echo "Mounting the partition to copy files..."
TEMP_MOUNT_DIR=$(mktemp -d)
sudo mount ${LOOP_DEVICE}p1 $TEMP_MOUNT_DIR
check_error

# Step 8: Copy files from BIN_DIR to the mounted partition
if [ -d "$BIN_DIR" ]; then
    echo "Copying files from $BIN_DIR to the mounted partition..."
    sudo cp -r $BIN_DIR/* $TEMP_MOUNT_DIR
    check_error
    echo "Files copied successfully to the partition."
else
    echo "$BIN_DIR does not exist. No files to copy."
fi

# Step 9: Unmount the partition
echo "Unmounting the partition..."
sudo umount $TEMP_MOUNT_DIR
check_error

# Step 10: Detach the loop device
echo "Detaching loop device $LOOP_DEVICE..."
sudo losetup -d $LOOP_DEVICE
check_error

# Step 11: Clean up temporary mount directory
rmdir $TEMP_MOUNT_DIR

echo "Virtual disk image $VIRTUAL_DISK_IMAGE created successfully and pre-filled with files from $BIN_DIR."

