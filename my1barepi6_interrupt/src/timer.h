/*----------------------------------------------------------------------------*/
#ifndef __MY1TIMERH__
#define __MY1TIMERH__
/*----------------------------------------------------------------------------*/
void timer_init(void);
unsigned int timer_read(void);
/*----------------------------------------------------------------------------*/
/* timer_wait is usleep equivalent */
#define TIMER_MS 1000
#define TIMER_S 1000000
/*----------------------------------------------------------------------------*/
void timer_wait(unsigned int wait);
/*----------------------------------------------------------------------------*/
void timer_setirq(int enable);
void timer_active(int enable);
void timer_load(int value);
void timer_reload(int value);
void timer_irq_clear(void); /** clears pending irq bit */
unsigned int timer_irq_raw(void); /** returns pending irq bit */
unsigned int timer_irq_masked(void); /** returns pending irq bit & enable bit */
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
