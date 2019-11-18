/*
  Term Project: Online Pong

  For the Arduino device, we will need to be able to serve a simple web page
  so that others can "watch," and we can monitor on our computer, what is
  happening on the board. It will need to receive UART communication from
  the MSP432 device and update the state of the game accordingly. It also 
  needs to update the MSP432 device when the game is over and when it starts.

*/

#include <Ethernet.h>

char game_over = 0;   // Similar to MSP432

void setup() {
  // Set up the Web Server Interface
  
  // Set up the UART Interface with our board

  // Set up the Button to indicate the game has
  // started (maybe on the MSP432 device)

  // Set up the web page with an initial bar 
  // setting (changes when the game starts)
}

void loop() {
  // Poll for the button press and set game_over to 0
  // on a single press

  // When game starts, poll UART for bar positions, and
  // serve the game packets

  // Each iteration, do some kind of tick() to move
  // the ball. When the ball goes out of bounds, signal
  // game_over = 1. Then send over UART channel to MSP432
  // and clear the game state. Go back to polling
  // for the start button.
}
