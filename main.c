#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/*
  Output pseudorandom bits out on B0

  This implements a blazingly fast 24 or 32 bit maximal Galois LFSR.
  For the 24-bit version runs at 8 instructions per bit.
  The 32-bit version takes 9 instructions per bit.

  The period is (2**nbits) - 1
*/

#define LFSR_32_BITS  // Otherwise 24 bits

int main() {
  cli();
  // CLKPSR = 0; // 8 MHz system clock
  CLKPSR = 3; // 1 MHz system clock
  // Disable ADC. Probably not needed.
  ADCSRA = 0;
  // Shut down the ADC. Also probably not needed.
  PRR &= ~(1<<PRADC);
  // Disable the comparator
  ACSR = 0x8;
  DDRB = 1;

  asm(
#ifdef LFSR_32_BITS
    "ldi r19, 0xaf \n\t"
    "ldi r22, 1 \n\t"
#else
    "ldi r19, 0x1b \n\t"
#endif
    "ldi r20, 1 \n\t"
    "ldi r21, 1 \n\t"
    "ldi r23, 1 \n\t"
    "myloop: \n\t"
    "lsl r20 \n\t"
    "rol r21 \n\t"
#ifdef LFSR_32_BITS
    "rol r22 \n\t"
#endif
    "rol r23 \n\t"
    "brcc dont_xor \n\t"
    "eor r20, r19 \n\t"
    "dont_xor: \n\t"
    "out  %0, r23 \n\t"
    "rjmp myloop \n\t"
    ::"I" _SFR_IO_ADDR(PORTB)
   );
}
