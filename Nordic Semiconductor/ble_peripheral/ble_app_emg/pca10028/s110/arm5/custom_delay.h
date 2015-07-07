#ifndef _CUSTOM_DELAY_H
#define _CUSTOM_DELAY_H

#include "nrf.h"

/* At 16 MHz, one NOP executes in 62.5 ns. 12 NOPs = 750 ns delay */

/*lint --e{438, 522} "Variable not used" "Function lacks side-effects" */
#if defined ( __CC_ARM   )
static __ASM void __INLINE short_delay(void)
{
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
}
#elif defined ( __ICCARM__ )
static void __INLINE short_delay(void)
{
__ASM (
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
}
#elif defined   (  __GNUC__  )

static void __INLINE short_delay(void) __attribute__((always_inline));
static void __INLINE short_delay(void)
{
__ASM volatile (
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"   
    " NOP\n"  
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
}
#endif

void nrf_delay_ms(uint32_t volatile number_of_ms);

#endif
