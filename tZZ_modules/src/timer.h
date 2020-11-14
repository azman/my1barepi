/*----------------------------------------------------------------------------*/
#ifndef __MY1TIMER_H__
#define __MY1TIMER_H__
/*----------------------------------------------------------------------------*/
#include "raspi.h"
#include "boot.h"
/*----------------------------------------------------------------------------*/
void timer_init(void); /* configure timer for 1mhz */
unsigned int timer_read(void);
/*----------------------------------------------------------------------------*/
/* timer_wait is usleep equivalent */
#define TIMER_US 1
#define TIMER_MS 1000
#define TIMER_S 1000000
/*----------------------------------------------------------------------------*/
void timer_wait(unsigned int wait);
/*----------------------------------------------------------------------------*/
void timer_setirq(int enable);
void timer_active(int enable);
void timer_load(unsigned int value);
void timer_reload(unsigned int value);
void timer_irq_clear(void); /** clears pending irq bit */
unsigned int timer_value(void);
unsigned int timer_irq_raw(void); /** returns pending irq bit */
unsigned int timer_irq_masked(void); /** returns pending irq bit & enable bit */
/*----------------------------------------------------------------------------*/
#endif /* __MY1TIMER_H__ */
/*----------------------------------------------------------------------------*/
