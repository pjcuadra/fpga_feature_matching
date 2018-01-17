# Build Arty Z7-20 Petalinux BSP

*This guide is based on the original Petalinux repository at https://github.com/Digilent/Petalinux-Arty-Z7-20*

This guide will walk you through some basic steps to get you booted into Linux
and rebuild the Petalinux project. After completing it, you should refer to the
Petalinux Reference Guide (UG1144) from Xilinx to learn how to do more useful
things with the Petalinux toolset. Also, refer to the Known Issues section above
for a list of problems you may encounter and work arounds.

This guide assumes you are using Ubuntu 16.04.3 LTS. Digilent highly recommends
using Ubuntu 16.04.x LTS, as this is what we are most familiar with, and cannot
guarantee that we will be able to replicate problems you encounter on other
Linux distributions.

## Install the Petalinux tools

Digilent has put together this quick installation guide to make the petalinux
installation process more convenient. Note it is only tested on Ubuntu 16.04.3
LTS.

First install the needed dependencies by opening a terminal and running the
following:

```
sudo -s
apt-get install tofrodos gawk xvfb git libncurses5-dev tftpd zlib1g-dev zlib1g-dev:i386  \
                libssl-dev flex bison chrpath socat autoconf libtool texinfo gcc-multilib \
                libsdl1.2-dev libglib2.0-dev screen pax
reboot
```

Next, install and configure the tftp server (this can be skipped if you are not
interested in booting via TFTP):

```
sudo -s
apt-get install tftpd-hpa
chmod a+w /var/lib/tftpboot/
reboot
```

Create the petalinux installation directory next:

```
sudo -s
mkdir -p /opt/pkg/petalinux
chown <your_user_name> /opt/pkg/
chgrp <your_user_name> /opt/pkg/
chgrp <your_user_name> /opt/pkg/petalinux/
chown <your_user_name> /opt/pkg/petalinux/
exit
```

Finally, download the petalinux installer from Xilinx and run the following (do
not run as root):

```
cd ~/Downloads
./petalinux-v2017.2-final-installer.run /opt/pkg/petalinux
```

Follow the onscreen instructions to complete the installation.

## Build the image

Whenever you want to run any petalinux commands, you will need to first start by
opening a new terminal and "sourcing" the Petalinux environment settings:

```
source /opt/pkg/petalinux/settings.sh
```

Run the following commands to build the petalinux project with the default
options:

```
cd <project-root>/image
petalinux-config --oldconfig
petalinux-build
petalinux-package --boot --force --fsbl images/linux/zynq_fsbl.elf --fpga images/linux/Arty_Z7_20_wrapper.bit --u-boot
```

### Boot the newly built files from SD

Then run petalinux-build to build your system. After the build completes, your
rootfs image will be at images/linux/rootfs.ext4.

Format an SD card with two partitions: The first should be at least 500 MB and
be FAT formatted. The second needs to be at least 1.5 GB (3 GB is preferred) and
formatted as ext4. The second partition will be overwritten, so don't put
anything on it that you don't want to lose. If you are uncertain how to do this
in Ubuntu, gparted is a well documented tool that can make the process easy.

Copy _images/linux/BOOT.BIN_ and _images/linux/image.ub_ to the first partition
of your SD card.

Identify the /dev/ node for the second partition of your SD card using _lsblk_
at the command line. It will likely take the form of /dev/sdX2, where X is
_a_,_b_,_c_,etc.. Then run the following command to copy the filesystem to the
second partition:

```
sudo dd if=images/linux/rootfs.ext4 of=/dev/sdX2
sync
```

The following commands will also stretch the file system so that you can use the
additional space of your SD card. Be sure to replace the block device node as
you did above, and also XG needs to be replaced with the size of the second
partition on your SD card (so if your 2nd partition is 3 GiB, you should use
3G):

```
sudo resize2fs /dev/sdX2 XG
sync
```

Eject the SD card from your computer, then do the following:

1. Insert the microSD into the Arty Z7
2. Attach a power source and select it with JP5 (note that using USB for power
   may not provide sufficient current)
3. If not already done to provide power, attach a microUSB cable between the
   computer and the Arty Z7
