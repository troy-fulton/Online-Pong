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
byte game_over = 1;

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(1,2,3,4);

// Initialize the Ethernet server library at port 80
EthernetServer server(80);

// Initialize Serial (UART) communication with MSP432
SoftwareSerial msp432Serial(10, 11); // RX, TX

// Reading from MSP432 giving the position of the potentiometer:
unsigned int msp432_potent = 0;

// Signals that the game is over/ not over:
byte game_start_signal = 0xAA;
byte game_over_signal  = 0xFF;

void setup() {
  // Open serial communications with computer just in case:
  Serial.begin(9600);

  // Start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield hardware not found.");
    while (true) {
      delay(1); // do nothing
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  // Open the communication channel with the MSP432 Device:
  msp432Serial.begin(9600);   // 9600 baud rate
  msp432Serial.write(game_start_signal);
}


void loop() {
  // listen for incoming clients but also for serial communication
  EthernetClient client = server.available();
  int receiver_available = msp432Serial.available();

  if (receiver_available) {
    byte first_reading = msp432.read();
    if (msp432.available()) {
      byte second_reading = msp432.read();
      msp432_potent = first_reading | (second_reading << 8);
    } else {
      Serial.println("Only got one read() from serial MSP432 communication.");
    }
  }
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    // Everything after here is yet to be coded...
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
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
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");       
          }
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
