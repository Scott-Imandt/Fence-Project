//Scott Imandt
// Code based on Example nrf24_reliable_datagram_client.pde
// Code doc for classes used: https://github.com/adafruit/RadioHead

// Need to include files for NRF24 and class variables

#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_NRF24 driver;

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

#define Gate 0 // Gate Number 
#define inputPin 4 // Pinout of inputPin

int State; // Inital state value to change for PIN READ
int currentState;

uint8_t data[2]; // [Gate#, State] STATE 0 == close ; 1 == open

// Dont put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
//


void setup() {
  
    Serial.begin(9600);
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm

   Serial.println("Set DataRate: 1Mbps, TP: 0dBm");
  driver.setRF(RH_NRF24::DataRate1Mbps, RH_NRF24::TransmitPower0dBm);

  currentState = 0;
  driver.setModeIdle(); 
   
 //-----------------

 pinMode(inputPin, INPUT);
 
}

void loop() {
  boolean temp = GateStatus(); // True == OPEN ; False == Close
   
  if(temp & currentState == 0){
    SendStatusChange(Gate, 1);  
  }
  else if(!temp & currentState == 1){
    SendStatusChange(Gate, 0);
  }
  delay(500);   
}


boolean GateStatus(){
  State = digitalRead(inputPin);

  if( State == HIGH){
    Serial.println("Open");
    return true;
  }
  else if(State != HIGH){
    Serial.println("Close");  
    return false;
  } 
}



boolean SendStatusChange(int gate, int state){
 
  Serial.print("Gate ");
  Serial.print(gate);
  Serial.print(": Sending state ");
  Serial.print(state);
  Serial.println(" to Server");

  driver.setModeRx();
  delay(500);
  
  //Set data value to send
  data[0] = gate;
  data[1] = state;
  
  // Send a message to manager_server
  if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
      
      currentState = state;
      driver.setModeIdle(); 
    }
    else
    {
      Serial.println("No reply");
       driver.setModeIdle(); 
    }
  }
  else
    Serial.println("sendtoWait failed");
     driver.setModeIdle(); 
  delay(500);
}
