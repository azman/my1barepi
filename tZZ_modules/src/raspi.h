/*----------------------------------------------------------------------------*/
#ifndef __RASPI_H
#define __RASPI_H
/*----------------------------------------------------------------------------*/
#define VC_MMU_MAP_L2_ENABLED  0x40000000
#define VC_MMU_MAP_L2_DISABLED 0xC0000000
#define VC_MMU_MAP_DEFAULT VC_MMU_MAP_L2_ENABLED
#define VC_MMU_MAP VC_MMU_MAP_DEFAULT
/*----------------------------------------------------------------------------*/
/* on VideoCore memory map, it is 0x7e000000 (BCM2835 @PiV1) */
#if defined RASPI3
#define PMAP_BASE 0x3F000000
#elif defined RASPI2
#define PMAP_BASE 0x3F000000
#else
#define PMAP_BASE 0x20000000
#endif
/*----------------------------------------------------------------------------*/
#define GPIO_OFFSET 0x00200000
#define TIMER_OFFSET 0x0000B400
#define TIMER_SYS_OFFSET 0x00003000
#define INTR_OFFSET 0x0000B000
#define SPI0_OFFSET 0x00204000
#define AUX_OFFSET 0x00215000
#define UART_OFFSET 0x00215040
#define SPI1_OFFSET 0x00215080
#define SPI2_OFFSET 0x002150C0
#define MAILBOX_OFFSET 0x0000B880
#define CLKMAN_OFFSET 0x00101000
#define PWM_OFFSET 0x0020C000
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
