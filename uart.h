/*********************************************************************************************
 *
 * MACROS / FUNCTIONS / GLOBAL VARIABLES FOR UART OPERATION
 *
 *********************************************************************************************/

volatile unsigned int adc_raw;
volatile unsigned int send_buf = 5000;

// Synchronization variables:
volatile char msp432_sent = 0;
volatile char arduino_received = 2;

// Receive a character through the serial communication line
char uart_receive() {      // Now includes a wait time to limit polling
    while ((EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) == 0) ;
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;
    return EUSCI_A0->RXBUF;
}

// Send a character to the serial communication line from send_buf
void uart_send(unsigned int reading) {
    send_buf = reading;     // capture the reading
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_TXIFG;
    EUSCI_A0->TXBUF = send_buf & 0xFF;  // first 8 bits first
    // No longer blocking:
    //while ((EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG) == 0) ;
}

void uart_setup() {
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | EUSCI_B_CTLW0_SSEL__SMCLK;
    // Sets up 9600 baud rate:
    EUSCI_A0->BRW = 19;
    // Using oversampling (see slides):
    EUSCI_A0->MCTLW = (9 << EUSCI_A_MCTLW_BRF_OFS) | EUSCI_A_MCTLW_OS16;
    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;

    // Enable UART pins:
    P1->SEL0 |= BIT2 | BIT3;

    //enable UART RX and TX interrupt
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE | EUSCI_A_IE_TXIE;
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);
    __delay_cycles(1000);

   __enable_irq();
}

void EUSCIA0_IRQHandler(void){ //once this is done delete uart_recieve

    __disable_irq();

    if((EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) != 0){
        //clear flag
        EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;
        arduino_received++;

        // Acknowledgment for half of a message
        if (arduino_received == 1) {
            // Send the other half
            uart_send(send_buf >> 8);
        }

        //read from the buffer
        char temp = EUSCI_A0->RXBUF;
        // Arbitrary message
        if (temp == 0xAA){
            ;
        }
    }
    if((EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG) != 0){
        EUSCI_A0->IFG &= ~EUSCI_A_IFG_TXIFG;
        msp432_sent++;
    }
    __enable_irq();

}
