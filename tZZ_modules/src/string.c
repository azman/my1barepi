/*----------------------------------------------------------------------------*/
#include "string.h"
/*----------------------------------------------------------------------------*/
char* strword(char* src, char* del, int* chk)
{
	char* ptr = 0x0, *buf = src;
	int loop = 0, test;
	while (buf[loop])
	{
		src++; test = 0;
		while (del[test])
		{
			if (buf[loop]==del[test])
			{
				buf[loop] = 0x0;
				ptr = src;
				break;
			}
			test++;
		}
		if (ptr) break;
		loop++;
	}
	if (loop>0)
	{
		if (chk) *chk = loop;
	}
	return ptr;
}
/*----------------------------------------------------------------------------*/
int strncmp(char* src1, char* src2, int size)
{
	int loop, stat = 0;

	/* strncmp has a size limit */
	for (loop=0;loop<size;loop++)
	{
		/* check if null comes first */
		if (!(*src1))
		{
			if ((*src2)) stat = -1; /* src2 longer string */
			/* else? stop at same time => equal! */
			break;
		}
		if (!(*src2))
		{
			if ((*src1)) stat = 1; /* src1 longer string */
			/* else? stop at same time => equal! */
			break;
		}
		/* actual strcmp! */
		if ((unsigned char)(*src1)>(unsigned char)(*src2))
		{
			stat = 1; break;
		}
		if ((unsigned char)(*src1)<(unsigned char)(*src2))
		{
			stat = -1; break;
		}
		src1++; src2++;
	}
	return stat;
}
/*----------------------------------------------------------------------------*/
int strncpy(char* dst, char* src, int size)
{
	int temp = sizeof(unsigned int);
	int test = size/temp, init = 0, loop;
	unsigned int* chk1 = (unsigned int*) src;
	unsigned int* chk2 = (unsigned int*) dst;
	// just in case!
	if(size<=0) return 0;
	// start with 32-bit transfer -> sizeof(unsigned int)
	for(loop=0;loop<test;loop++)
	{
		*chk2 = *chk1;
		chk2++; chk1++;
		init += temp;
	}
	// remaining with 8-bit transfer
	test = size%temp;
	for(loop=0;loop<test;loop++)
	{
		*dst = *src;
		dst++; src++;
		init++;
	}
	return init;
}
/*----------------------------------------------------------------------------*/
char is_whitespace(char achar)
{
	switch(achar)
	{
		case ' ':
		case '\t':
			break;
		default:
			achar = 0x0;
	}
	return achar;
}
/*----------------------------------------------------------------------------*/
int trimws(char *astring, int norepeat)
{
	int count = 0, index = 0, loop;
	char prev = ' ';
	/* count */
	while(astring[count])
		count++;
	/* trim right */
	count--;
	while(is_whitespace(astring[count]))
		count--;
	/* replace null at last whitespace */
	astring[count++] = 0x0;
	/* trim left */
	loop = 0;
	while(is_whitespace(astring[loop]))
		loop++;
	/* copy, trim left & multiple sequence */
	for(;loop<count;loop++)
	{
		if (is_whitespace(astring[loop]))
		{
			if(norepeat&&is_whitespace(prev))
				continue;
			/* all whitespace convert to 0x20 */
			astring[loop] = ' ';
		}
		astring[index++] = astring[loop];
		prev = astring[loop];
	}
	/* terminate string */
	astring[index] = 0x0;
	/* return new length */
	return index;
}
/*----------------------------------------------------------------------------*/
char uppercase(char *achar)
{
	if(*achar>=0x61&&*achar<=0x7a)
		*achar -= 0x20; /* make uppercase */
	return *achar;
}
/*----------------------------------------------------------------------------*/
char* str2upper(char *astring)
{
	int loop = 0;
	while(uppercase(&astring[loop++]));
	return astring;
}
/*----------------------------------------------------------------------------*/
char lowercase(char *achar)
{
	if(*achar>=0x41&&*achar<=0x5a)
		*achar += 0x20; /* make lowercase */
	return *achar;
}
/*----------------------------------------------------------------------------*/
char* str2lower(char *astring)
{
	int loop = 0;
	while(lowercase(&astring[loop++]));
	return astring;
}
/*----------------------------------------------------------------------------*/
