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
// Other game state information here:
float msp432_paddle, other_paddle;
int ball_x, ball_y, ball_angle;

// Reading from MSP432 giving the position of the potentiometer:
unsigned int msp432_potent = 0;

// Signals that the game is over/ not over:
byte game_on_signal = 0xAA;
byte game_over_signal = 0xFF;

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
  msp432Comm.write(game_over_signal);
}

int havent_replied = 100;
int counter = 0;
void loop() {
  // listen for incoming clients but also for serial communication
  EthernetClient client = server.available();
  int receiver_available = msp432Comm.available();

  if (receiver_available && counter < havent_replied) {
    counter++;
    byte first_reading = msp432Comm.read();
    // Wait until the next character is sent
    if (game_over == 0) {
      msp432Comm.write(game_on_signal);
    }
    else {
      msp432Comm.write(game_over_signal);
    }
    while (!msp432Comm.available()) ;
    byte second_reading = msp432Comm.read();
    msp432_potent = first_reading | (second_reading << 8);
    // Give the paddle's position as a "percentage"
    if (msp432_potent <= 4096) {
      msp432_paddle = ((float) msp432_potent / 4096.0) * 100.0;
    }
    if (game_over == 0) {
      msp432Comm.write(game_on_signal);
    }
    else {
      msp432Comm.write(game_over_signal);
    }
  }
  if (client) {
    counter = 0;
    //Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    // Everything after here is yet to be coded...
    char c;
    bool is_player_1 = false; // only player 1 determines where the ball is
    bool is_player_2 = false;
    while (client.connected()) {
      if (client.available()) {
        char last_char = c;
        c = client.read();
        // Read in the player number and game_over headers from
        // the python program indicating how the game is going
        // Have to start from the first letter of the line (has
        // to be unique. For example, only the "player" header
        // starts with p and only the "game_over" header starts
        // with g)
        if (c == 'g' && last_char == '\n' && is_player_1) {
          while (c != '\r') {
            last_char = c;
            c = client.read();
          }
          currentLineIsBlank = false;
          if (last_char == '0')
            game_over = 0;
          else {
            game_over = 1;
            //Serial.println("Got game over");
          }
        }
        else if (c == 'p' && last_char == '\n') {
          while (c != '\r') {
            last_char = c;
            c = client.read();
          }
          currentLineIsBlank = false;
          if (last_char == '1')
            is_player_1 = true;
          else
            is_player_2 = true;
        }
        else if (c == 'x' && last_char == '\n' && is_player_1) {
          client.read();  // ":"
          client.read();  // " "
          String reading = "";
          while (c != '\r') {
            last_char = c;
            c = client.read();
            reading += c;
          }
          ball_x = reading.toInt();
          currentLineIsBlank = false;
        }
        else if (c == 'y' && last_char == '\n' && is_player_1) {
          client.read();  // ":"
          client.read();  // " "
          String reading = "";
          while (c != '\r') {
            last_char = c;
            c = client.read();
            reading += c;
          }
          ball_y = reading.toInt();
          currentLineIsBlank = false;
        }
        else if (c == 'a' && last_char == '\n' && is_player_1) {
          client.read();  // "n"
          client.read();  // "g"
          client.read();  // "l"
          client.read();  // "e"
          client.read();  // ":"
          client.read();  // " "
          String reading = "";
          while (c != '\r') {
            last_char = c;
            c = client.read();
            reading += c;
          }
          ball_angle = reading.toInt();
          currentLineIsBlank = false;
        }
        else if (c == 'o' && last_char == '\n' && is_player_2) {
          // strip the other stuff out
          while (c != ':') {
            last_char = c;
            c = client.read();
          }
          client.read();  // " "
          String reading = "";
          while (c != '\r') {
            last_char = c;
            c = client.read();
            reading += c;
          }
          other_paddle = reading.toFloat();
          currentLineIsBlank = false;
        }
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
          client.print("x ");
          client.print(ball_x);
          client.print("<br/>");
          client.print("y ");
          client.print(ball_y);
          client.print("<br/>");
          client.print("angle ");
          client.print(ball_angle);
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
    //Serial.println("client disonnected");
  }
}
