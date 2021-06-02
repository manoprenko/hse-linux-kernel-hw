obj-m += phonebook.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
test:
	-sudo rm /dev/phonebook
	-sudo rmmod phonebook
	sudo dmesg -C
	sudo insmod phonebook.ko
	dmesg -T
	sudo mknod /dev/phonebook c 239 0
	sudo chmod 777 /dev/phonebook

