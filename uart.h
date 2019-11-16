/*********************************************************************************************
 *
 * MACROS / FUNCTIONS / GLOBAL VARIABLES FOR UART OPERATION
 *
 *********************************************************************************************/

// We should really change this polling loop to an interrupt-based thing later...

// Receive a character through the serial communication line
char uart_receive() {      // Now includes a wait time to limit polling
    while ((EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) == 0) ;
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;
    return EUSCI_A0->RXBUF;
}

// Send a character to the serial communication line
void uart_send(char ch) {
    EUSCI_A0->IFG &= EUSCI_A_IFG_TXIFG;
    EUSCI_A0->TXBUF = ch;
    while ((EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG) == 0) ;
}

void uart_setup() {
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | EUSCI_B_CTLW0_SSEL__SMCLK;
    // Sets up 9600 baud rate:
    EUSCI_A0->BRW = 19;
    // Using oversampling (see slides):
    EUSCI_A0->MCTLW = (9 << EUSCI_A_MCTLW_BRF_OFS) | EUSCI_A_MCTLW_OS16;
    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;
    __delay_cycles(1000);
}
