/*----------------------------------------------------------------------------*/
#ifndef __MY1PWMH__
#define __MY1PWMH__
/*----------------------------------------------------------------------------*/
#include "raspi.h"
#include "boot.h"
/*----------------------------------------------------------------------------*/
/** PWMCH1_PWEN & PWMCH2_PWEN */
#define PWM_CHANNEL_0 0x0001
#define PWM_CHANNEL_1 0x0100
/*----------------------------------------------------------------------------*/
/**
 *  exculsive use of MS (Mark-Space) pwm mode!
 *  pwm clock derived from a 19.2MHz clock?
 *  - div16 will get 1.2MHz
 *  - div192 => 100kHz
 *  - div384 => 50kHz
 *  - div512 => 37.5kHz
 *  - div1920 => 10kHz
 *  - div3840 => 5kHz
 *  - div is only 12-bits (max=4095)
**/
void pwm_main_clock(unsigned int divi, unsigned int divf);
/*----------------------------------------------------------------------------*/
void pwm_init(unsigned int chan);
void pwm_exec(unsigned int chan);
void pwm_stop(unsigned int chan);
void pwm_prep_data(unsigned int chan, unsigned int data);
void pwm_prep_full(unsigned int chan, unsigned int full);
/* get current values for data and range */
unsigned int pwm_curr_data(unsigned int chan);
unsigned int pwm_curr_full(unsigned int chan);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
