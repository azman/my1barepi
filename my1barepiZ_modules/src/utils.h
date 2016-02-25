/*----------------------------------------------------------------------------*/
#ifndef __UTILS_H
#define __UTILS_H
/*----------------------------------------------------------------------------*/
int str2int(char* str); /** convert string to integer */
float str2float(char* str); /** convert string to float */
int int2str(char* str, int val); /** convert integer to string */
int float2str(char* str, float val); /** convert float to string (2dp) */
/*----------------------------------------------------------------------------*/
/**
 * This module also provides __aeabi_uidiv and __aeabi_uidivmod to allow
 * integer division (no hardware for this).
 * Note: Not needed if divide by a constant value?
**/
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
