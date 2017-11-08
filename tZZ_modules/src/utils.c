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
		rem <<= 1;
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
		rem <<= 1;
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
int __aeabi_idiv(int dvd, int dvs)
{
	int sgn = 0, res;
	if (dvd<0)
	{
		dvd = -dvd;
		sgn++;
	}
	if (dvs<0)
	{
		dvs = -dvs;
		sgn++;
	}
	sgn &= 0x01;
	res = (int) ((unsigned int)dvd/(unsigned int)dvs);
	if (sgn) res = -res;
	return res;
}
/*----------------------------------------------------------------------------*/
int __aeabi_idivmod(int dvd, int dvs)
{
	if (dvd<0) dvd = -dvd;
	if (dvs<0) dvs = -dvs;
	return (int) ((unsigned int)dvd%(unsigned int)dvs);
}
/*----------------------------------------------------------------------------*/
unsigned int str2uint(char* str, int len)
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
	int calc, len = 0, neg = 0;
	/** check negative value */
	if(str[len]=='-') { neg = 1; str++; }
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
	int dot = -1, len = 0, neg = 0;
	/** check negative value */
	if(str[len]=='-') { neg = 1; str++; }
	/** get string length and get dp position */
	while(str[len])
	{
		if(str[len]=='.') dot = len;
		len++;
	}
	if(dot<0) dot = len;
	/** do your thing! */
	chk = str2uint(str,dot);
	/** loop decimal points */
	while(dot<len)
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
	int len = 0, idx;
	unsigned int tmp = val,cmp;
	while(tmp>0)
	{
		cmp = tmp % 10;
		tmp = tmp / 10;
		/* shift digits */
		for(idx=len;idx>0;idx--)
			str[idx] = str[idx-1];
		str[0] = cmp + 0x30;
		len++;
	}
	/** check zero value */
	if(len==0) str[len++] = '0';
	/** return size */
	return len;
}
/*----------------------------------------------------------------------------*/
int int2str(char* str, int val)
{
	int idx = 0;
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
int float2str(char* str, float val, int dpc)
{
	int tmp, idx = 0;
	unsigned int chk;
	/** check negative value */
	if(val<0)
	{
		str[idx++] = '-';
		val = -val;
	}
	chk = val;
	val -= (float) chk;
	/** convert integer part! */
	idx += uint2str(&str[idx],chk);
	/** put decimal point in */
	str[idx++] = '.';
	/* show decimal part - 2 digits for now */
	while(dpc>0)
	{
		val *= 10;
		tmp = val;
		str[idx++] = tmp + 0x30;
		val -= (float) tmp;
		dpc--;
	}
	/** add terminator */
	str[idx++] = 0x0;
	/** return size (excluding null) */
	return (idx-1);
}
/*----------------------------------------------------------------------------*/
