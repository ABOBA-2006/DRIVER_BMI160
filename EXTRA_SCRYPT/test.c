#include <stdio.h>
#include <stdint.h>

// emulate kernel types
typedef int16_t s16;

// IOCTL macros for userspace
#define _IOC_NRBITS    8
#define _IOC_TYPEBITS  8
#define _IOC_SIZEBITS  14
#define _IOC_DIRBITS   2

#define _IOC_NRSHIFT   0
#define _IOC_TYPESHIFT (_IOC_NRSHIFT + _IOC_NRBITS)
#define _IOC_SIZESHIFT (_IOC_TYPESHIFT + _IOC_TYPEBITS)
#define _IOC_DIRSHIFT  (_IOC_SIZESHIFT + _IOC_SIZEBITS)

#define _IOC(dir,type,nr,size) (((dir)  << _IOC_DIRSHIFT) | \
                                ((type) << _IOC_TYPESHIFT) | \
                                ((nr)   << _IOC_NRSHIFT) | \
                                ((size) << _IOC_SIZESHIFT))

#define _IOR(type,nr,size) _IOC(2,(type),(nr),sizeof(size)) // 2 = read

#define BMI160_IOCTL_MAGIC 'B'

// Now define your IOCTLs
#define IOCTL_GET_ACCEL_X _IOR(BMI160_IOCTL_MAGIC, 1, s16)
#define IOCTL_GET_ACCEL_Y _IOR(BMI160_IOCTL_MAGIC, 2, s16)
#define IOCTL_GET_ACCEL_Z _IOR(BMI160_IOCTL_MAGIC, 3, s16)

int main() {
    printf("IOCTL_GET_ACCEL_X = 0x%X\n", IOCTL_GET_ACCEL_X);
    printf("IOCTL_GET_ACCEL_Y = 0x%X\n", IOCTL_GET_ACCEL_Y);
    printf("IOCTL_GET_ACCEL_Z = 0x%X\n", IOCTL_GET_ACCEL_Z);
    return 0;
}
