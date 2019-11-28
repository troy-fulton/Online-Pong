/*
  Term Project: Online Pong

  For the Arduino device, we will need to be able to serve a simple web page
  so that others can "watch," and we can monitor on our computer, what is
  happening on the board. It will need to receive UART communication from
  the MSP432 device and update the state of the game accordingly. It also 
  needs to update the MSP432 device when the game is over and when it starts.

*/
#include <Ethernet.h>
#include <SoftwareSerial.h>

// Similar to MSP432:
byte game_over = 0;
// Other game state information here:
float msp432_paddle, other_paddle;

// Reading from MSP432 giving the position of the potentiometer:
unsigned int msp432_potent = 0;

// Signals that the game is over/ not over:
byte game_on_signal = 0xAA;
byte game_over_signal  = 0xFF;

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(1,2,3,4);

// Initialize the Ethernet server library at port 80
EthernetServer server(80);

// Initialize Serial (UART) communication with MSP432
SoftwareSerial msp432Comm(8, 9); // RX, TX

void setup() {
  Ethernet.init(10);  // Most Arduino shields init this way
  
  // Open serial communications with computer just in case:
  Serial.begin(9600);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  // Open the communication channel with the MSP432 Device:
  msp432Comm.begin(9600);   // 9600 baud rate
  msp432Comm.write(game_on_signal);
}


void loop() {
  // listen for incoming clients but also for serial communication
  EthernetClient client = server.available();
  int receiver_available = msp432Comm.available();

  if (receiver_available) {
    byte first_reading = msp432Comm.read();
    // Wait until the next character is sent
    msp432Comm.write(game_on_signal);
    while (!msp432Comm.available()) ;
    byte second_reading = msp432Comm.read();
    msp432_potent = first_reading | (second_reading << 8);
    // Give the paddle's position as a "percentage"
    //Serial.println(msp432_potent);
    if (msp432_potent <= 4096) {
      msp432_paddle = ((float) msp432_potent / 4096.0) * 100.0;
    }
    msp432Comm.write(game_on_signal);
  }
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    // Everything after here is yet to be coded...
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // add a meta refresh tag, so the browser pulls again every 5 seconds:
          client.println("<meta http-equiv=\"refresh\" content=\"5\">");

          // For our data, we want to print the game state information:
          client.print("game_over ");
          client.print(game_over);
          client.print("<br/>");
          client.print("msp432_paddle ");
          client.print(msp432_paddle);
          client.print("<br/>");
          client.print("other_paddle ");
          client.print(other_paddle);
          client.print("<br/>");
          
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}
