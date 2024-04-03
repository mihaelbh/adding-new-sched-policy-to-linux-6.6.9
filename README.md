# adding-new-sched-policy-to-linux-6.6.9
Adding new policy to linux-6.6.9 kernel


1. Clone this repository with `git clone https://github.com/mihaelbh/adding-new-sched-policy-to-linux-6.6.9.git`
2. Enter new directory with `cd adding-new-sched-policy-to-linux-6.6.9`
3. Compile kernel ( for detailed guide see: https://itsfoss.com/compile-linux-kernel/ )
	1. add .config file to repository
	2. run `make oldconfig` to add new configuration options to .config
	3. start compiling kernel with `make -j$(nproc)` or `make -j$(nproc) 2>&1 | tee log` ( to find errors easier with command `cat log | grep error` )
		- if it throws somthing like `.btf.vmlinux.bin.o: file not recognized: file format not recognized`
    		1. run `make mrproper`
    		2. add .config file again
			3. run `make oldconfig`
    		4. disable CONFIG\_DEBUG\_INFO\_BTF in .config
    		5. start compiling kernel again ( run `make -j$(nproc)` or `make -j$(nproc) 2>&1 | tee log` )
		- if it throws any other error during compilation
			1. run `make mrproper`
			2. fix the error
			3. add .config file again
			4. run `make oldconfig`
    		5. start compiling kernel again ( run `make -j$(nproc)` or `make -j$(nproc) 2>&1 | tee log` )
  	4. instal modules with `make modules_install -j$(nproc)`, `sudo` may be required
  	5. install kernel with `make install`, `sudo` may be required
    	- if you are on Arch linux, this will not work and you need to
    		1. run `install -Dm644 "$(make -s image_name)" /boot/vmlinuz-6.6.9-<localversion>`, `sudo` may be required
    		2. create initial ramdisk with `mkinitcpio -P`, `sudo` may be required
    		3. update bootloader
				- for grub run `grub-mkconfig -o /boot/grub/grub.cfg`, `sudo` may be required
4. Reboot and load new kernel
	- you can check if right kernel is loaded with `uname -r`
5. Before using the scheduler in file /usr/include/bits/types/struct_sched_param.h add line `int new_sched_prio;` in struct sched\_param
6. To use new scheduler policy 
	- use function `sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)`
	- set policy to 7
	- set priority in sched\_param to 1
	- set new\_sched\_prio in sched\_param to a number between 0 and 9