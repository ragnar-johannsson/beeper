## Beeper

A Linux kernel module that installs a netfilter hook in NF_INET_LOCAL_IN that beeps through the PC speaker for every ICMP echo packet received.

### Usage

Install build dependencies for the kernel (apt-get build-dep linux on Debian) and then simply:

    $ make
    $ sudo insmod beeper.ko
    $ ping localhost

### License

BSD 2-Clause. See the LICENSE file for details.
