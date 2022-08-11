#include <WiFi.h>
#include "arduino_secrets.h"

#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

#define LED1 14
#define LED2 27

RH_NRF24 driver(4,5);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;

// 3 == unknown 1== OPEN 0 == CLOSED
int Gate1 = 3;
int Gate2 = 3;

int Gate1Battery = 0;
int Gate2Battery = 0;

WiFiServer server(80);

void setup(){
  
   Serial.begin(115200);

   pinMode(LED1, OUTPUT);
   pinMode(LED2, OUTPUT);

   digitalWrite(LED1, HIGH);
   digitalWrite(LED2, HIGH);
   NrfSetup();
   
   WebsiteSeverSetup();
   digitalWrite(LED1, LOW);
   digitalWrite(LED2, LOW);
   
}

uint8_t data[] = "Data Delivered";
// Dont put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];


void loop(){

  WebsiteServer();
  NrfLoop();
}


void NrfSetup(){
  
    //Serial.begin(9600);
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm

  driver.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm);
}

void NrfLoop(){
  
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      Serial.print("Got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.print(buf[0]);     
      Serial.print(buf[1]);
      Serial.println(buf[2]);

      // Send a reply back to the originator client
      if (!manager.sendtoWait(data, sizeof(data), from))
        Serial.println("sendtoWait failed");

        if(buf[0] == 0){
          if(buf[1] == 0){
            Gate1 = 1;
            digitalWrite(LED1, HIGH); 
          } else {
            Gate1 = 0;
            digitalWrite(LED1, LOW);
          }
          Gate1Battery = buf[2];
         }
         
         if(buf[0] == 1){
          if(buf[1] == 0){
            Gate2 = 1;
            digitalWrite(LED2, HIGH);
          } else {
            Gate2 = 0;
            digitalWrite(LED2, LOW);
          }
         Gate2Battery = buf[2];
         }
       
        // update the webserver with info
    }
  }
}




void WebsiteSeverSetup(){

   
    //pinMode(5, OUTPUT);      // set the LED pin mode

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();
}


void WebsiteServer(){
  
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            
            client.println();

            // the content of the HTTP response follows the header:
            String htmlPageHeader = "<!DOCTYPE html><html>"
            "<head>"
              "<style>"
            
            "h1 {"
              "color: DarkBlue;"
              "text-align:center;"
              "text-decoration-line: underline;"
            "}"
            
            "p {"
            
              "text-align:center;"
              "margin: auto;"
              "padding: 10px;"
            "}"
            
            "footer{"
              "text-align: center;"
              "padding: 30px;"
              "color: gray;"
              "font-size: 10px;"
            "}"
                                   
              "</style>"
            
            "<title>Fence Server</title>"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
            
            "</head>";
            
            String htmlPageBody ="<body><h1>Fence Server</h1>";

            String htmlPageFooter =
            "<footer>"
              "Fence project V 1.1"
            "</footer>"
          "</html>";

            client.print(htmlPageHeader);
            client.print(htmlPageBody);

            client.print("<p>Right Gate:  <strong>");
            if(Gate1 == 0) client.print("CLOSED  ");
            else if(Gate1 == 1) client.print("OPEN  ");
            else if(Gate1 == 3) client.print("UNKNOWN  ");
            client.print(Gate1Battery);
            client.print("%</strong></p>");

            client.print("<p>Left Gate:    <strong>");
            if(Gate2 == 0) client.print("CLOSED  ");
            else if(Gate2 == 1) client.print("OPEN  ");
            else if(Gate2 == 3) client.print("UNKNOWN  ");
            client.print(Gate2Battery);
            client.print("%</strong></p>");

            client.print(htmlPageFooter);

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
       // if (currentLine.endsWith("GET /H")) {
         // digitalWrite(5, HIGH);               // GET /H turns the LED on
       // }
        //if (currentLine.endsWith("GET /L")) {
        //  digitalWrite(5, LOW);                // GET /L turns the LED off
       // }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
