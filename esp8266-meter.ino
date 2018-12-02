#include <Wire.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

const char * ssid = "insert your ssid";              //insert your wifi ssid
const char * password = "insert your wifi password"; //insert your wifi password

// Set web server port number to 80
WiFiServer server(80);

const int requestPin = 0;
int incomingByte = 0;
char thislline[54];
String hightariff;
String lowtariff;
String current;
String gas;
int i;
bool h = false;
bool l = false;
bool c = false;
bool g = false;
String thisline;
String store;
// Variable to store the HTTP request
String header;
//Wire.begin(0); //to open the ports on the esp8266
#define SERIAL_RX 0 // pin for SoftwareSerial RX
SoftwareSerial mySerial (SERIAL_RX, -1, true, 700); // (RX, TX, inverted, buffer)

void setup () {
  delay (5000);

  // Start Serial
  mySerial.begin (115200);
  delay (5);
  Serial.begin (115200);
  delay (1000);

  // We start by connecting to a WiFi network
  Serial.println ();
  Serial.println ();
  Serial.print ("Connecting to ");
  Serial.println (ssid);
  WiFi.begin (ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay (500);
    Serial.print (".");
  }
  // Start the server
  server.begin();
  Serial.println("Server started");
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  Serial.println ("Start reading from the P1 port of the smart meter");
}

void loop () {
WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.print("<h4>Meter reading</h4>");
            i=0;
            while (mySerial.available() > 0 && i<54) {
              incomingByte = mySerial.read();
              incomingByte &= ~ (1 << 8);
              char inChar = (char) incomingByte;
              thislline[i] = inChar;
              i++;
                if (inChar=='\n'){
                  i=0;
                  thisline = String(thislline);
                  //Serial.println(thislline);      //uncomment for testing purposes
                  if (thislline[4]=='1' && thislline[6]=='8' && thislline[8]=='2'){  //it should start like this '1-0:1.8.2'
                    if (thislline[12]=='0' && thislline[13]!='0'){                  //this finds where the actual numbers are and that the line was copied correctly
                      hightariff = thisline.substring(13, 19);
                      h = true;
                    }
                    else if (thislline[11]=='0' && thislline[12]!='0'){
                      hightariff = thisline.substring(12, 19);
                      h = true;
                    }
                  }
                  else if (thislline[4]=='1' && thislline[6]=='8' && thislline[8]=='1'){   //it should start like this '1-0:1.8.1'
                    if (thislline[12]=='0' && thislline[13]!='0'){                        //this finds where the actual numbers are and tests that the line was copied correctly
                      lowtariff = thisline.substring(13, 19);
                      l = true;
                    }
                    else if (thislline[11]=='0' && thislline[12]!='0'){
                      lowtariff = thisline.substring(12, 19);
                      l = true;
                    }
                  }
                  else if (thislline[4]=='1' && thislline[6]=='7' && thislline[8]=='0'){                  //it should start like this '1-0:1.7.0'
                    if ((thislline[11]=='0' && thislline[12]!='0') || (thislline[10]=='0' && thislline[11]!='0')){     //this test that the line was copied correctly
                      current = thisline.substring(11, 16);
                      c = true;
                    }
                  }
                  else if (thislline[4]=='2' && thislline[7]=='2' && thislline[9]=='1'){         //it should start like this '0-1:24.2.1'
                    if (thislline[27]=='0' && thislline[28]!='0'){                              //this finds where the actual numbers are and that the line was copied correctly
                      gas = thisline.substring(28, 35);
                      g = true;
                    }
                    else if (thislline[26]=='0' && thislline[27]!='0'){
                      gas = thisline.substring(27, 35);
                      g = true;
                    }
                  }

                 if (h && l && c && g){   //this checks that all the four variables  were received
                  client.print("High tarrif: " ); client.print(hightariff); client.print(" kWh<br>");
                  client.print("Low tarrif: " ); client.print(lowtariff); client.print(" kWh<br>");
                  client.print("Current usage: " ); client.print(current); client.print(" kW<br>");
                  client.print("Gas: " ); client.print(gas); client.println(" m<sup>3</sup><br>");
                  client.print("---------------------------------------------");
                  i = 54;       //to break out of the while loop
                  h = false;
                  l = false;
                  c = false;
                  g = false;
                 }

                }
                //delay (0.0868055550); //this might be the time it takes to read a character



              }
            client.println("</html>");
            client.println();     // The HTTP response ends with another blank line
            break;               // Break out of the while loop
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    header = "";            // Clear the header variable
    client.stop();          // Close the connection
    Serial.println("Client disconnected.");
    Serial.println("");
  }
        yield();




}
