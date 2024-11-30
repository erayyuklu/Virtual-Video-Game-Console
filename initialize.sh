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


