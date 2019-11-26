/*********************************************************************************************
 *
 * MACROS / FUNCTIONS / GLOBAL VARIABLES FOR ADC CONVERSIONS
 *
 *********************************************************************************************/
#include "msp.h"
#include "uart.h"

// TODO: Something in here is not right... need to use potentiometer from BH EDU board with
// correct voltage reference


void adc_setup() {
    P4->SEL1 |= BIT2;
    P4->SEL0 |= BIT2;
    ADC14->CTL0 = ADC14_CTL0_SHT0_2 | ADC14_CTL0_SHP | ADC14_CTL0_ON;
    ADC14->CTL1 = ADC14_CTL1_RES_2;
    ADC14->MCTL[0] = ADC14_MCTLN_INCH_11;

    // For reference voltage:
    ADC14->MCTL[0] |= ADC14_MCTLN_VRSEL_14;
    //P5->DIR &= ~BIT6 & ~BIT7;   // Doesn't matter but just in case
    P5->SEL0 |= BIT6 | BIT7;    // Making these VREF+ and
    P5->SEL1 |= BIT6 | BIT7;    // VREF-

    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);
    ADC14->IER0 |= ADC14_IER0_IE0;
    __enable_irq();
    ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
}

// Interrupt handler for receiving a signal from UART channel
void ADC14_IRQHandler(void) {
    __disable_irq();
    adc_raw = ADC14->MEM[0];
    ADC14->CLRIFGR0 |= ADC14_CLRIFGR0_CLRIFG0; // Clear interrupt flag 1
    __enable_irq();
}

