/*----------------------------------------------------------------------------*/
#include "gsm.h"
#include "uart.h"
#include "timer.h"
#include "string.h"
/*----------------------------------------------------------------------------*/
#define GSM_WAIT_DELAY (2*TIMER_S)
/*----------------------------------------------------------------------------*/
void gsm_command(char* message)
{
	uart_print(message);
	uart_send(0x0d); /** CR only */
}
/*----------------------------------------------------------------------------*/
int gsm_checkok(char* message, int count)
{
	int ok_found = 0;
	if (count>=GSM_OK_SIZE)
		if (!strncmp(&message[count-GSM_OK_SIZE],GSM_OK,GSM_OK_SIZE))
			ok_found = 1;
	return ok_found;
}
/*----------------------------------------------------------------------------*/
int gsm_timeout(int delay)
{
	unsigned int init = timer_read(), time = 1;
	while(timer_read()-init<delay)
		if (uart_incoming()) { time = 0; break; }
	return time;
}
/*----------------------------------------------------------------------------*/
int gsm_replies(char* message, int size, unsigned int *okstat)
{
	char *pstrok = GSM_OK;
	int count = 0, okay = GSM_OK_NOTFOUND;
	while (1)
	{
		if (gsm_timeout(GSM_WAIT_DELAY)) break;
		message[count] = uart_read();
		count++;
		if (count==size-1) break;
		if (message[count-1]==pstrok[okay])
		{
			okay++;
			if (okay==GSM_OK_COMPLETE) break;
		}
		else if (message[count-1]==pstrok[0]) okay = GSM_OK_FOUNDNEW;
		else okay = GSM_OK_NOTFOUND;
	}
	message[count] = 0x0;
	if (okstat) *okstat = okay;
	return count;
}
/*----------------------------------------------------------------------------*/
char* gsm_trim_prefix(char* message)
{
	/* trim prefixed \r\n, some gsm module do not send these */
	char *pbuff = message;
	while (*pbuff=='\r'||*pbuff=='\n') pbuff++;
	return pbuff;
}
/*----------------------------------------------------------------------------*/
