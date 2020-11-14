/*----------------------------------------------------------------------------*/
#ifndef __MY1STRING_H__
#define __MY1STRING_H__
/*----------------------------------------------------------------------------*/
char* strword(char** src, char* del);
int strncmp(char* src1, char* src2, int size);
int strncpy(char* dst, char* src, int size);
/*----------------------------------------------------------------------------*/
/** codes from my1asm85 project */
char is_whitespace(char achar);
int trimws(char *astring, int norepeat);
char uppercase(char *achar);
char* str2upper(char *astring);
char lowercase(char *achar);
char* str2lower(char *astring);
/*----------------------------------------------------------------------------*/
#endif /* __MY1STRING_H__ */
/*----------------------------------------------------------------------------*/
