kernel version: 6.2.0-37-generic  

insmod sample.ko  
==> load kernel driver module  
./poll_test &  
==> user space that polling deivce node  
./ioctl_test  
==> send command to driver then driver return POLLIN event to user space  

