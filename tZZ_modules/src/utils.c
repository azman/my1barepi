/*----------------------------------------------------------------------------*/
#include "utils.h"
/*----------------------------------------------------------------------------*/
unsigned int __aeabi_uidiv(unsigned int dvd, unsigned int dvs)
{
	unsigned int quo, rem, bit;
	/** take care of the obvious */
	if(dvd<dvs) return 0;
	else if(dvd==dvs) return 1;
	/** divide by zero result is invalid! */
	else if(dvs==0) return ~0;
	/** initialize! */
	quo = 0; rem = 0; bit = 0x80000000;
	/** long division - bit by bit */
	while(bit)
	{
		quo <<= 1;
		if(bit&dvd)
			rem |= 1;
		if(rem>=dvs)
		{
			rem -= dvs;
			quo++;
		}
		bit >>= 1;
	}
	return quo;
}
/*----------------------------------------------------------------------------*/
unsigned int __aeabi_uidivmod(unsigned int dvd, unsigned int dvs)
{
	unsigned int quo, rem, bit;
	/** take care of the obvious */
	if(dvd<dvs) return dvd;
	else if(dvd==dvs) return 0;
	/** divide by zero result is invalid! */
	else if(dvs==0) return ~0;
	/** initialize! */
	quo = 0; rem = 0; bit = 0x80000000;
	/** long division - bit by bit */
	while(bit)
	{
		quo <<= 1;
		if(bit&dvd)
			rem |= 1;
		if(rem>=dvs)
		{
			rem -= dvs;
			quo++;
		}
		bit >>= 1;
	}
	return rem;
}
/*----------------------------------------------------------------------------*/
unsigned int str2uint(char* str, unsigned char len)
{
	unsigned int calc = 0, mult = 1;
	/** browse through the array - from right! */
	while(len>0)
	{
		len--;
		/** check if really numeric */
		if(str[len]<0x30||str[len]>0x39)
			return 0;
		/** get digit actual value */
		calc = calc + (str[len]-0x30)*mult;
		/** prepare next iteration */
		mult = mult * 10;
	}
	return calc;
}
/*----------------------------------------------------------------------------*/
int str2int(char* str)
{
	int calc;
	unsigned char len = 0;
	unsigned char neg = 0;
	/** check negative value */
	if(str[len]=='-')
	{
		neg = 1; str++;
	}
	/** get string length */
	while(str[len]) len++;
	/** do your thing! */
	calc = (int) str2uint(str,len);
	/** check overflow */
	if(calc<0) return 0;
	/** apply sign flag */
	if(neg) calc = -calc;
	return calc;
}
/*----------------------------------------------------------------------------*/
float str2float(char* str)
{
	float calc = 0.0, divs = 10.0;
	unsigned int chk;
	unsigned char len = 0;
	char dot = 0xFF;
	unsigned char neg = 0;
	/** check negative value */
	if(str[len]=='-')
	{
		neg = 1; str++;
	}
	/** get string length and get dp position */
	while(str[len])
	{
		if(str[len]=='.') dot = len;
		len++;
	}
	if((unsigned char)dot==0xFF) dot = len;
	/** do your thing! */
	chk = str2uint(str,dot);
	/** loop decimal points */
	while((unsigned char)dot<len)
	{
		dot++;
		calc = calc + (float)(str[len]-0x30)/divs;
		divs *= 10.0;
	}
	/** add up and apply sign flag */
	calc = calc + (float) chk;
	if(neg) calc = -calc;
	return calc;
}
/*----------------------------------------------------------------------------*/
int uint2str(char* str, unsigned int val)
{
	unsigned char idx = 0, tmp;
	unsigned int cmp = 10000;
	unsigned char fst = 0;
	/** look for the 5 digits */
	while(cmp>0)
	{
		if(val>=cmp||fst) /** check fst to get following zero(es)! */
		{
			tmp = val / cmp; /** should be 0-9 */
			val = val % cmp;
			str[idx++] = tmp + 0x30;
			fst = 1;
		}
		cmp /= 10;
	}
	/** check zero value */
	if(idx==0) str[idx++] = '0';
	/** return current position */
	return idx;
}
/*----------------------------------------------------------------------------*/
int int2str(char* str, int val)
{
	unsigned char idx = 0;
	unsigned int chk;
	/** check negative value */
	if(val<0)
	{
		str[idx++] = '-';
		val = -val;
	}
	chk = val;
	/** do your thing! */
	idx += uint2str(&str[idx],chk);
	/** add terminator */
	str[idx++] = 0x0;
	/** return size (excluding null) */
	return (idx-1);
}
/*----------------------------------------------------------------------------*/
int float2str(char* str, float val)
{
	unsigned char tmp, idx = 0;
	unsigned int chk;
	/** check negative value */
	if(val<0)
	{
		str[idx++] = '-';
		val = -val;
	}
	chk = val; /** should be within 16-bit value - NOT checked! */
	val -= (float) chk;
	/** convert integer part! */
	idx += uint2str(&str[idx],chk);
	/** put decimal point in */
	str[idx++] = '.';
	/* show decimal part - 2 digits for now */
	val *= 10;
	tmp = (unsigned char) val;
	str[idx++] = (unsigned char)tmp + 0x30;
	val -= (float) tmp;
	val *= 10;
	tmp = (unsigned char) val;
	str[idx++] = (unsigned char)tmp + 0x30;
	/** add terminator */
	str[idx++] = 0x0;
	/** return size (excluding null) */
	return (idx-1);
}
/*----------------------------------------------------------------------------*/
