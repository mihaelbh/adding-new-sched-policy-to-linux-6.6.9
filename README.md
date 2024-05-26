# adding-new-sched-policy-to-linux-6.6.9
Adding new policy to linux-6.6.9 kernel

For detailed kernel compile guide see: https://itsfoss.com/compile-linux-kernel/ 

## Ubuntu

### Download and compile
1. Clone this repository with `git clone https://github.com/mihaelbh/adding-new-sched-policy-to-linux-6.6.9.git`
2. Enter new directory with `cd adding-new-sched-policy-to-linux-6.6.9`
3. Get .config file with `cp /boot/config-"$(uname -r)" .config`
4. Run `make oldconfig` to add new configuration options to .config
5. Run `scripts/config --disable SYSTEM_TRUSTED_KEYS` and `scripts/config --disable SYSTEM_REVOCATION_KEYS` (because it isn't signed so make will complain otherwise)
6. Start compiling with `make -j$(nproc) 2>&1 | tee log`
	- if it asks for SYSTEM_TRUSTED_KEYS or SYSTEM_REVOCATION_KEYS just press Enter
	- if an error happens you can find it easy with `cat log | grep -i error`
	- if an error happens run `make mrproper`, fix the error and start again from step 3
7. Run `sudo make modules_install -j$(nproc)` to load modules
8. Run `sudo make install` to install kernel
9. Reboot and select new kernel ( during boot spam shift to get GRUB boot menu and select "Advanced options for Ubuntu" )

### Run
1. Check if right kernel is loaded with `uname -r`
2. Before using the scheduler in file /usr/include/x86_64-linux-gnu/bits/types/struct_sched_param.h add line `int new_sched_prio;` in struct sched_param
3. To use new scheduler policy 
	- use function `sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)`
	- set policy to 7
	- set priority in sched_param to 1
	- set new_sched_prio in sched_param to a number between 0 and 9


## Arch

### Download and compile
1. Clone this repository with `git clone https://github.com/mihaelbh/adding-new-sched-policy-to-linux-6.6.9.git`
2. Enter new directory with `cd adding-new-sched-policy-to-linux-6.6.9`
3. Get .config file with `cp /proc/config.gz ./`, unzip it with `gunzip config.gz` and raname it with `mv config .config`
4. Run `make oldconfig` to add new configuration options to .config
5. Disable CONFIG_DEBUG_INFO_BTF in .config ( from my experience if it is enabled it gives error during compile )
6. Change CONFIG_EXT4_FS=m from in .config from m to y if you use ext4 filesystem
7. Start compiling with `make -j$(nproc) 2>&1 | tee log`
	- if an error happens you can find it easy with `cat log | grep -i error`
	- if an error happens run `make mrproper`, fix the error and start again from step 3
8. Run `sudo make modules_install -j$(nproc)` to load modules
9. Run `sudo install -Dm644 "$(make -s image_name)" /boot/vmlinuz-6.6.9-localversion` to install the kernel, replace localversion with whatever you want to distinguish it from other kernels
10. Make file /etc/mkinitcpio.d/linux-localversion.preset and add (replace localversion with whatever you replaced it with in step before):
```
	ALL_config="/etc/mkinitcpio.conf"
	ALL_kver="/boot/vmlinuz-6.6.9-localversion"

	PRESETS=('default' 'fallback')

	default_image="/boot/initramfs-6.6.9-localversion.img"
	fallback_options="-S autodetect"
```

11. Create initial ramdisk with `sudo mkinitcpio -P`
12. Update bootloader ( for GRUB run `sudo grub-mkconfig -o /boot/grub/grub.cfg` )
13. Reboot and select new kernel ( select "Advanced options for Arch Linux" in GRUB boot menu )

### Run
1. Check if right kernel is loaded with `uname -r`
2. Before using the scheduler in file /usr/include/bits/types/struct_sched_param.h add line `int new_sched_prio;` in struct sched_param
3. To use new scheduler policy 
	- use function `sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)`
	- set policy to 7
	- set priority in sched_param to 1
	- set new_sched_prio in sched_param to a number between 0 and 9