/*********************************************************************************************
 *
 * MACROS / FUNCTIONS / GLOBAL VARIABLES FOR ADDED MUSIC FEATURES
 *
 *********************************************************************************************/

// Timer functions to start/stop the continuous counting by enabling/
// disabling interrupts
void start_timer() {
    // Enable timer interrupts
    TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CCIE;    // Enable CC interrupts - pitch
    TIMER_A0->CCTL[1] |= TIMER_A_CCTLN_CCIE;    // Enable CC interrupts - tempo
    NVIC->ISER[0] |= 1 << ((TA0_0_IRQn) & 31);  // "0" means any of the capture/compare registers (why?)
    __enable_irq();     // enable interrupts
}

void stop_timer() {
    // Disable timer interrupts
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;       // Disable CC interrupts - pitch
    TIMER_A0->CCTL[1] &= ~TIMER_A_CCTLN_CCIE;       // Disable CC interrupts - tempo
    NVIC->ISER[0] &= ~(1 << ((TA0_0_IRQn) & 31));  // "0" means any of the capture/compare registers (why?)
    __disable_irq();    // disable interrupts

}

// Indices into scale structure below:
#define C4  0
#define CS4 1   // C#
#define D4  2
#define DS4 3   // D#, etc...
#define E4  4
#define F4  5
#define FS4 6
#define G4  7
#define GS4 8
#define A4  9
#define AS4 10
#define B4  11
#define C5  12
#define CS5 13
#define D5  14
#define DS5 15
#define E5  16
#define F5  17
#define FS5 18
#define G5  19
#define GS5 20
#define A5  21
#define AS5 22
#define B5  23
#define C6  24

// Notes in a scale
float frequencies[25] = {
                         261.63,  // C4
                         277.18,  // C4#
                         293.66,  // D4
                         311.13,  // D4#
                         329.63,  // E4
                         349.23,  // F4
                         369.99,  // F4#
                         392.00,  // G4
                         415.30,  // G4#
                         440.00,  // A4
                         466.16,  // A4#
                         493.88,  // B4
                         523.25,  // C5
                         554.37,  // C5#
                         587.33,  // D5
                         622.25,  // D5#
                         659.25,  // E5
                         698.46,  // F5
                         739.99,  // F5#
                         783.99,  // G5
                         830.61,  // G5#
                         880.00,  // A5
                         932.33,  // A5#
                         987.77,  // B5
                         1046.50  // C6
};

// Magic number by experimentation
#define SLEEP_MULTIPLE 1500000

// Define the quarter, whole, half, etc. notes
#define QUARTER     80         // THIS is the tempo (inverted in multiples of 10)
#define WHOLE       ((int) (QUARTER*4))
#define HALF        ((int) (QUARTER*2))
#define EIGHTH      ((int) (QUARTER/2))
#define DOT_QUARTER ((int) (QUARTER*1.5))

// Have to count in multiples of 10 since the counter in the
// timer can only go so high
int counter = 0;

struct NOTE {
    int name;   // index into below table
    int period; // as something we can give to the timer (have to initialize)
    int len;    // quarter, half, whole note, etc.
};

// Allocate our song here while keeping track of how many notes we have

#define PLAYING_BACH 0

#if PLAYING_BACH
#define NOTE_COUNT 30
struct NOTE song[NOTE_COUNT] = {
                                {E4, 0, QUARTER},       // bar 1
                                {E4, 0, QUARTER},
                                {F4, 0, QUARTER},
                                {G4, 0, QUARTER},
                                {G4, 0, QUARTER},       // bar 2
                                {F4, 0, QUARTER},
                                {E4, 0, QUARTER},
                                {D4, 0, QUARTER},
                                {C4, 0, QUARTER},       // bar 3
                                {C4, 0, QUARTER},
                                {D4, 0, QUARTER},
                                {E4, 0, QUARTER},
                                {E4, 0, DOT_QUARTER},   // bar 4
                                {D4, 0, EIGHTH},
                                {D4, 0, HALF},
                                {E4, 0, QUARTER},       // bar 5
                                {E4, 0, QUARTER},
                                {F4, 0, QUARTER},
                                {G4, 0, QUARTER},
                                {G4, 0, QUARTER},       // bar 6
                                {F4, 0, QUARTER},
                                {E4, 0, QUARTER},
                                {D4, 0, QUARTER},
                                {C4, 0, QUARTER},       // bar 7
                                {C4, 0, QUARTER},
                                {D4, 0, QUARTER},
                                {E4, 0, QUARTER},
                                {D4, 0, DOT_QUARTER},   // bar 8
                                {C4, 0, EIGHTH},
                                {C4, 0, HALF},
};

#else

// Festive Overture by Dmitri Shostakovich

#define NOTE_COUNT 70
struct NOTE song[NOTE_COUNT] = {
                                {F4, 0, QUARTER},               // pickup
                                {F5, 0, WHOLE + EIGHTH},        // bar 1     bar 2 on eighth
                                {DS5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {AS4, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {DS5, 0, EIGHTH},
                                {F5, 0, WHOLE + EIGHTH},       // bar 3     bar 4 on eighth
                                {DS5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {AS4, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {DS5, 0, EIGHTH},
                                {F5, 0, QUARTER},              // bar 5
                                {G5, 0, EIGHTH},
                                {A5, 0, EIGHTH},
                                {AS5, 0, QUARTER},
                                {A5, 0, EIGHTH},
                                {G5, 0, EIGHTH},
                                {F5, 0, QUARTER},              // bar 6
                                {DS5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {DS5, 0, EIGHTH},
                                {G5, 0, EIGHTH},
                                {F5, 0, QUARTER},              // bar 7
                                {G5, 0, EIGHTH},
                                {A5, 0, EIGHTH},
                                {AS5, 0, QUARTER},
                                {A5, 0, EIGHTH},
                                {G5, 0, EIGHTH},
                                {F5, 0, QUARTER},              // bar 8
                                {DS5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {DS5, 0, EIGHTH},
                                {G5, 0, EIGHTH},
                                {F5, 0, QUARTER},              // bar 9
                                {AS4, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {D5, 0, HALF + QUARTER},       // bar 10 on quarter
                                {AS4, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {DS5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {AS4, 0, EIGHTH},              // bar 11
                                {C5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {DS5, 0, EIGHTH},
                                {D5, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {AS4, 0, EIGHTH},
                                {A4, 0, EIGHTH},
                                {G4, 0, EIGHTH},               // bar 12
                                {A4, 0, EIGHTH},
                                {G4, 0, EIGHTH},
                                {FS4, 0, EIGHTH},
                                {G4, 0, EIGHTH},
                                {A4, 0, EIGHTH},
                                {AS4, 0, EIGHTH},
                                {C5, 0, EIGHTH},
                                {D5, 0, WHOLE}                 // bar 13

};

#endif  // which song to play (PLAYING_BACH)

// boolean for if the side button has been pressed to play the song
int playing_song = 0;

// For when we are playing the song:
int curr_note = 0;

// For when we are taking input notes from the keypad:
int custom_pitch = 0;

// Calculate how long a sleep should be for a given note using this
// magic number
int find_period(int note_name) {
    return (int) (1/frequencies[note_name] * SLEEP_MULTIPLE);
}

// When we receive keypad input for a note to play
void play_note(int note_name) {
    custom_pitch = find_period(note_name);
}

// When we get the special button to play our song
void play_song() {
    playing_song = 1;
    curr_note = 0;
    start_timer();
    while (curr_note < NOTE_COUNT) ;    // spin until song is over
    stop_timer();
    playing_song = 0;
    P5->OUT |= BIT0;                    // turn off buzzer just in case
}

// ISR for the Timer A CC
void TA0_0_IRQHandler() {

    if (TIMER_A0->CCTL[0] & TIMER_A_CCTLN_CCIFG) {  // "pitch" timer
        TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;  // reset it
        if (playing_song) {     // During the song, use that array
            TIMER_A0->CCR[0] += song[curr_note].period;

            // This condition says "only play 90% of the note" so we can hear articulation
            if (counter*10 >= (song[curr_note].len - 10)) {
                P5->OUT |= BIT0;
            }
            else {
                P5->OUT ^= BIT0;
            }
        }
        else {
            TIMER_A0->CCR[0] += custom_pitch;  // play our custom pitch
            P5->OUT ^= BIT0;    // Toggle note like PWM
        }
    }

    if (TIMER_A0->CCTL[1] & TIMER_A_CCTLN_CCIFG) { // "tempo" timer
        TIMER_A0->CCTL[1] &= ~TIMER_A_CCTLN_CCIFG;
        if (playing_song) {         // Only care about note length here
            counter++;              // Count multiples of 10
            if (counter*10 == song[curr_note].len) {
                curr_note++;    // Toggle note index in song array when done
                counter = 0;
            }
            TIMER_A0->CCR[1] += 10;
        }
    }
}
