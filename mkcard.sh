#!/bin/bash

# This script is used to flash the sd-card.
# Customize DEV/UDISK with your environment.
# @DEV: device name of your sdcard
# @UDISK: mount point of your sdcard

if [[ -z "${DEV}" ]]; then
	DEV=/dev/sdb1
fi

if [[ -z "${UDISK}" ]]; then
	UDISK=/home/u/ws/u-disk
fi

ROOT=`pwd`

sudo umount $UDISK
sudo mount $DEV $UDISK
if [ 0 -ne $? ]; then
	echo "Mount failed, please check and retry!"
	exit -1
fi

echo "Removing ......"
sudo rm $UDISK/fip.bin
echo "Removing Done!"

echo "Copying to sd-card ......"
sudo cp $ROOT/release_out/fip.bin $UDISK
echo "Copying Done!"

sudo umount $UDISK

echo "Done!"

