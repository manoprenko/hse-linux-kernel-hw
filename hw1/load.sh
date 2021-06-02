#!/bin/bash

make

sudo rm /dev/phonebook 2> /dev/null
sudo rmmod phonebook 2> /dev/null
sudo dmesg -C
sudo insmod phonebook.ko
sudo mknod /dev/phonebook c $(dmesg | grep "lkm_phonebook: module loaded with device major number" | cut -d " " -f9) 0
sudo chmod 777 /dev/phonebook
