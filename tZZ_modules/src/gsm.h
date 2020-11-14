/*----------------------------------------------------------------------------*/
#ifndef __MY1GSM_H__
#define __MY1GSM_H__
/*----------------------------------------------------------------------------*/
#define GSM_OK "\r\nOK\r\n"
#define GSM_OK_SIZE 6
/*----------------------------------------------------------------------------*/
#define GSM_OK_COMPLETE GSM_OK_SIZE
#define GSM_OK_FOUNDNEW 1
#define GSM_OK_NOTFOUND 0
/*----------------------------------------------------------------------------*/
void gsm_command(char* message);
int gsm_checkok(char* message, int count);
int gsm_timeout(int delay);
int gsm_replies(char* message, int size, unsigned int *okstat);
char* gsm_trim_prefix(char* message);
/*----------------------------------------------------------------------------*/
#endif /* __MY1GSM_H__ */
/*----------------------------------------------------------------------------*/
