/*
  Term Project: Online Pong

  For the MSP432 device, all that is necessary is a potentiometer reading
  via ADC, a bit of music to play on the Piezo Buzzer, and a UART connection
  for the Arduino defvice communication.

  This simple code will read the ADC conversion generated from reading the
  potentiometer, place it in the TX buffer, and transmit it along the Serial
  communication line to the Arduino line as fast as possible. The MSP432 will
  also periodically read from the Arduino device to see if the game is over.
  If it is, it will stop playing the current song and switch to a new sound.

  The MSP432 device will begin idle, waiting for a signal from the Arduino
  device to say that the game has started. When it does, it will start playing
  the song and taking potentiometer input. When the game is over, it will play
  the sound indicating the game is over and go back into idle mode.
*/

#include "msp.h"
#include "adc.h"
#include "music.h"

void main(void) {
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    /****************************************
     * SETUP for GPIO
     ****************************************/

    // Added a GPIO device...

    /****************************************
     * SETUP for the song
     ****************************************/

    int i;
    for (i = 0; i < NOTE_COUNT; i++) {
        song[i].period = find_period(song[i].name);
    }

    /****************************************
     * SETUP for Timer (for the song)
     ****************************************/

    /*
     * Ok here is the thing with the timers...
     *
     * You can either use TAIFG timer that just
     * sets an interrupt when you go from the CCR0 register to 0, or you can use one
     * of the capture/compare counters to count when the counter goes from CCRn -1 to
     * CCR, where 0 <= n <= 6. This means you could have a bunch of different time
     * intervals and each time the timer surpasses a certain count, it sets an interrupt.
     *
     * In the end, it doesn't matter which one we use for this project, since we only use
     * the timer on one device, we will have to reset it every time we finish a note
     * anyways.
     *
     */

    // This timer is used as the pitch of the note:
    // With these instructions, the timer will count up to CCR0 for each note
    TIMER_A0->CCR[0] = 5000;                    // value to count up to (capture/compare register 0)
    TIMER_A0->CTL    = TIMER_A_CTL_TASSEL_2;    // Timer A source select: SMCLK (subsystem master clock)
    TIMER_A0->CTL   |= TIMER_A_CTL_MC_2;        // Mode of control: continuous (so we can use different intervals)
    TIMER_A0->CTL   |= TIMER_A_CTL_CLR;         // Clear timer clock division logic

    // This timer is used as the tempo (different intervals than pitch):
    TIMER_A0->CCR[1] = 10;                      // value to count up to (capture/compare register 0)

    /**********************************************************************
     * SETUP FOR eUSCI (UART)
     **********************************************************************/
    uart_setup();   // starts polling for receiver...
    char rxbuf;

    /**********************************************************************
     * SETUP FOR ADC
     **********************************************************************/
    adc_setup();

    /****************************************
     * PROGRAM
     ****************************************/

    // Keep playing the song and let the potentiometer reading be an interruption...
    while (1) {

        /*
         * While this is happening: if rxbuf is filled with, say 0xAA, we know the game has
         * started. Then, set the game_over to 0. If we receive, say a 0xFF, we know the
         * game has ended, so we set game_over to 1.
         */

        /*
         * Transmission will be a little trickier. We are receiving 12-bit values from the
         * ADC unit, so we will need to send the data in two chars at a time. Send the lower
         * 8 bits first, wait for the transmission to complete, then send the upper 4 bits.
         * We can easily write a function for this sort of thing.
         *
         * This all only happens when we get an interrupt from the ADC telling us that
         * the conversion is complete (which we will also have to set up).
         */

        /*
         * Note that the only things that do not operate on interrupts are the song on the
         * Piezo buzzer and (possibly) the transmission of data via UART.
         */

        if (!game_over) {
            play_song();
        }

    }
}
