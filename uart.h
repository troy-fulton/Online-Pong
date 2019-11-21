/*********************************************************************************************
 *
 * MACROS / FUNCTIONS / GLOBAL VARIABLES FOR UART OPERATION
 *
 *********************************************************************************************/

char game_over = 1; // a boolean


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

    //enalbe UART RX interrupt
    EUSCI_A2->IE |= EUSCI_A_IE_RXIE;
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);
    __delay_cycles(1000);

   __enable_irq();

}

void EUSCIA0_IRQHandler(void){ //once this is done delete uart_recieve

    __disable_irq();

    if((EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) != 0){

        //clear flag
        EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;

        //read from the buffer
         char temp = EUSCI_A0->RXBUF;

         if (temp == 0xAA){ //if we get 0xAA, the game has started

             game_over = 0;
         }

         else if(temp == 0xFF){ //if we get 0xFF, the game has ended

             game_over = 1;
         }

    }

    __enable_irq();

}
