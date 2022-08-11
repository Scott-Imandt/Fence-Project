//Scott Imandt
// Code based on Example nrf24_reliable_datagram_client.pde
// Code doc for classes used: https://github.com/adafruit/RadioHead

// Need to include files for NRF24 and class variables

//Change Gate ID, Channel 

#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1
#define CLIENT_ADDRESS3 3
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
//RH_NRF24 driver;

RH_NRF24 driver(8,7); // ce, csn

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

#define Gate 0 // Gate Number // 0,1
#define inputPin 4 // Pinout of inputPin

int State; // Inital state value to change for PIN READ
int currentState;

int batteryLevel;

uint8_t data[3]; // [Gate#, State, Battery Level] STATE 0 == close ; 1 == open

// Dont put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
//


void setup() {
  
    Serial.begin(115200);
  if (!manager.init()){
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  }
  else{
    Serial.println("Set DataRate: 1Mbps, TP: 0dBm");
    driver.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm);
  }
  
   driver.setModeIdle(); 
   
 //-----------------

 pinMode(inputPin, INPUT);

 // Batery Percent setup

 int batteryLevel = batteryPercent();
 Serial.print("init battery Level: ");
 Serial.println(batteryLevel);


 // Inital Gate status check
 Serial.println("intit Gate Status");
 Serial.print(GateStatus());
 if(GateStatus()){
  SendStatusChange(Gate, 1, batteryLevel);
 }
 else{
  SendStatusChange(Gate, 0, batteryLevel);
 }
}

void loop() {
  boolean temp = GateStatus(); // True == CLOSE ; False == OPEN
   
  if(temp & currentState == 0){
    SendStatusChange(Gate, 1, batteryLevel);  
  }
  else if(!temp & currentState == 1){
    SendStatusChange(Gate, 0, batteryLevel);
  }

  batteryCheck();

  delay(500);   
}


boolean GateStatus(){
  State = digitalRead(inputPin);

  if( State == HIGH){
    //Serial.println("Close");
    return true;
  }
  else if(State != HIGH){
    //Serial.println("Open");  
    return false;
  } 
}



boolean SendStatusChange(int gate, int state, int batteryLevel){
 
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
  data[2] = batteryLevel;
  
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

int batteryPercent(){

  int sensorValue = analogRead(A6);
  int percentRaw = map(sensorValue, 754, 988, 0, 100);
  int percentConstrain = constrain(percentRaw,0,100);

  return percentConstrain; 
}

void batteryCheck(){
  // Battery Percent Checker
  
  int temp = batteryPercent();

  if(batteryLevel - temp  >= 5 || temp - batteryLevel >= 5){
    batteryLevel = temp;
    SendStatusChange(Gate, currentState, batteryLevel);  
  }
  
}
