/*----------------------------------------------------------------------------*/
#ifndef __UTILS_H
#define __UTILS_H
/*----------------------------------------------------------------------------*/
/**
 * These are needed for integer division (no hardware for this).
 * Note: Not needed if divide by a constant value?
**/
unsigned int __aeabi_uidiv(unsigned int dvd, unsigned int dvs);
unsigned int __aeabi_uidivmod(unsigned int dvd, unsigned int dvs);
int __aeabi_idiv(int dvd, int dvs);
int __aeabi_idivmod(int dvd, int dvs);
/*----------------------------------------------------------------------------*/
int str2int(char* str); /** convert string to integer */
float str2float(char* str); /** convert string to float */
int int2str(char* str, int val); /** convert integer to string */
int float2str(char* str, float val, int dpc); /** convert float to string */
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
