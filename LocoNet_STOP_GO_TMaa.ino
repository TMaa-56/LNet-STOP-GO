/**************************************************************
DESCRIPTION:
This is code for a Loconet OPC_GPON and OPC_GPOFF Switch
Loconet:
- TX pin D7 (may be any pin, LocoNet library mostly defaults to D7)
- RX pin D8 (mandatory pin, ICP1 (Timer/Counter1 Input Capture Pin)

Libraries:
Uses the Bounce2 an Loconet libraries. They need to be installed in order to compile.
Add to libraries by modify libraries, or:
- Bounce2: https://github.com/thomasfredericks/Bounce2
- Loconet: https://github.com/mrrwa/LocoNet

Pin use Atmega 328P-PU

Go Switch    = 0      (D0 - 2)
Stop Switch  = 1      (D1 - 3)
Stop LED     = 3      (D3 - 5)
Go LED       = 4      (D4 - 6)

Copyright (C) 2015-2016 Timo Sariwating
Bron: https://github.com/Merdeka/Arduino-ModelRail & https://youtu.be/awdsWNFs_F0
Adapted for private use by HdG 2019-07
Modifications for layout redesign by TMaa 2022-02

**************************************************************/
 
#include <LocoNet.h>
#include <Bounce2.h>

//Buttons
const byte GreenButtonPin = 0;   //ON, of NO type
const byte RedButtonPin   = 1;   //OFF, of NO type

//LEDs
const byte RedLed   = 3;
const byte GreenLed = 4;

// Loconet
#define LOCONET_TX_PIN 7
static lnMsg *LnPacket;
static LnBuf  LnTxBuffer;

// Variables will change:
boolean OPCSTATE = 0;   //For state control center (on/off)

Bounce buttonRed, buttonGreen; //The button objects of Bounce2 library


/*************************************************************************/
/*          Setup                                                        */
/*************************************************************************/ 
void setup() {
  
  // Setup the buttons
  buttonRed.attach(RedButtonPin, INPUT_PULLUP);
  buttonGreen.attach(GreenButtonPin, INPUT_PULLUP);
  
  // Set up the outputs
  pinMode(RedLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  
  // Initialize the LocoNet interface
  LocoNet.init(LOCONET_TX_PIN); // The TX output in LocoNet library defaults to digital pin 6 for TX
  
}

/*************************************************************************/
/*          Send OPC_GP                                                  */
/*************************************************************************/ 
void sendOPC_GP(byte on) {
  lnMsg SendPacket;
  if (on) {
      SendPacket.data[ 0 ] = OPC_GPON;  
  } else {
      SendPacket.data[ 0 ] = OPC_GPOFF;  
  }
  LocoNet.send( &SendPacket ) ;
}

/*************************************************************************/
/*          Program Loop                                                 */
/*************************************************************************/ 
void loop() {
  // Read the Buttons
  readButtons();
  
  // Check LocoNet for a power state update
  checkLocoNet();
  
  // Set the LEDs
  setLed();  
}

/*************************************************************************/
/*          Read the Red and Green Buttons                               */
/*************************************************************************/ 
void readButtons() {
  //Read the actual buttons
  buttonRed.update();
  buttonGreen.update();
  
  //check for press of the red button (rose, because of the NC type)
  if(buttonRed.rose()){
    sendOPC_GP(0);  //Send new state to controle center
    OPCSTATE = 0;   //and save the new state
  }
  //Check is the green button became pressed (fell, because of NO type)
  else if(buttonGreen.fell()){
    sendOPC_GP(1);  //Send new state to controle center
    OPCSTATE = 1;   //and save the new state
  }
}

/*************************************************************************/
/*          Read the Red and Green Buttons                               */
/*************************************************************************/ 
void checkLocoNet() {
  // Check for any received LocoNet packets
  LnPacket = LocoNet.receive() ;
  if( LnPacket )
  {
    if (LnPacket->sz.command == OPC_GPON)  {        // GLOBAL power ON request 0x83     
      OPCSTATE = 1;
    } else if (LnPacket->sz.command == OPC_GPOFF) { // GLOBAL power OFF req 0x82
      OPCSTATE = 0;
    }
  }
}


/*************************************************************************/
/*          Set the Red and Green Leds                                  */
/*************************************************************************/ 
void setLed() {
  digitalWrite(RedLed, !OPCSTATE);
  digitalWrite(GreenLed, OPCSTATE);
}
