//number of components

const byte pinsPerEnc = 2; //pls dont try to change this
const int numEncoders = 16;

const int numButtons = 40;




//pages or whatever
const byte numPages = 8;
byte currentPage = 0;
byte pageColors[3 * numPages] = //color for each page - r,g,b, for each page
{0,0,255,
0,128,255,
0,255,255,
128,255,128,
255,255,0,
255,128,128,
255,0,255,
128,128,255,
};  


//muxes

#include "mux_connections.h"



//leds
#include <WS2812Serial.h>
#include "led_control.h"

theLedController = ledController();
float overallBrightness = (1/8); //will be replaced when I create options




//encoders
//#define INPUT_PULLUP 1 //tells the library (encoder.h) to use pullup instead of pulldown resistors
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
//#undef INPUT_PULLUP 
#undef ENCODER_DO_NOT_USE_INTERRUPTS

Encoder *encoders[numEncoders];



//keys

void updatePageButtonLeds();

#include "key_control.h"
baseButton *buttons[numButtons];

void updatePageButtonLeds() {
  for (int i=0; i<numButtons; i++) {
    if (keyTypesArray[i] == pagesButton){
      static_cast<pageButton*>(buttons[i])->pageUpdate();
    }
  }
}




//virtual values
const int numControls = numEncoders * numPages;
float sensitivity = .03; //how much to add per encoder step
float theValue[numControls] = {};




void setup() {
  
  #ifdef USB_SERIAL
    while (! Serial); //wait until serial finishes connecting or smthn
    Serial.println("hi hello here we are");
  #endif
  #ifdef USB_MIDI

  #endif

  //initializes an array used when searching for encoder indicies based on mux address
  initializeEncoderIndicies(); 
  initializeButtonIndicies(); //same thing but for buttons
  
  for (int i = 0; i < numEncoders; i++) { //loop through encoder objects
    //setup objects in the encoders array
    encoders[i] = new Encoder( //pretend this isn't two lines :)
      muxSignalPins[encoderAddressesCW[i].muxNumber], muxSignalPins[encoderAddressesCC[i].muxNumber]);
  }

  for (int i = 0; i<numButtons; i++) {

    if (keyTypesArray[i] == basicMidi) {
      #ifdef USB_SERIAL
        buttons[i] = new basicSerialButton(buttonIndexToAddress(i), &theLedController, i+(numEncoders * ledsPerEnc), i);
      #endif
      #ifdef USB_MIDI
        buttons[i] = new basicMidiButton(buttonIndexToAddress(i), &theLedController, i+(numEncoders * ledsPerEnc), i, i+32, 1);
      #endif
    }
    else if (keyTypesArray[i] == pagesButton)
      buttons[i] = new pageButton(buttonIndexToAddress(i), &theLedController, 
      i+(numEncoders * ledsPerEnc), i, i % 8, &currentPage, pageColors);


    #ifdef USB_SERIAL
      Serial.print("select num: "); Serial.print(buttonIndexToAddress(i).muxNumber);
      Serial.print(" select line: "); Serial.print(buttonIndexToAddress(i).muxLine);
      Serial.print(" index: "); Serial.print(i);
      Serial.print(" ledIndex: "); Serial.println(i+(numEncoders * ledsPerEnc));
    #endif
  }
  
  for (int i = 0; i < muxSize; i++) { //loop through number of select pins (shared between all muxes)
    pinMode(muxSelectPins[i], OUTPUT); //set select lines as outputs
  }
  for (int i = 0; i < numMuxes; i++) { //loop through physical muxes
    pinMode(muxSignalPins[i], INPUT_PULLUP); //set signal pins as inputs
  }
  MuxSel (muxSize, muxSelectPins, 0); //start on select line 0
}


void loop() {
 
  for (int s = 0; s < numSelectLines; s++) { //loop through mux select lines
    //float delta[numEncoders];
    for (int r = 0; r < numMuxes; r++) { //loop through muxes
      
      if (whatsOnThatMux[r] == key){
        //if this mux has keys on it: 
        buttons[buttonAddressToIndex(r,s)]->updateButton();
      }
      
      else if (whatsOnThatMux[r] == encoderCC) {
        //if this mux has counter clockwise encoder pins on it:
        //(We only need to do this for one of the pins, counter clockwise would work as well)
        //The encoder library polls the counter clockwise pin and also the clockwise pin, 
        //assuming that the clockwise pin is on the next multiplexer in the muxSignalPins array.
        
        int encIndex = findEncoderByAddress(r,s);
        //Serial.print("encoderCC "); Serial.print(encIndex);  Serial.print ("  ");
        
        float delta = (sensitivity * float(encoderChange(encoders[encIndex]))); /*
        Serial.print("delta: "); Serial.print(delta);
        Serial.print(" cc: "); Serial.print(digitalRead(muxSignalPins[r]));
        Serial.print(" cw: "); Serial.println(digitalRead(muxSignalPins[r+1])); */
        
        if (delta != 0) {
          int globalValuesIndex = encIndex+(currentPage * numEncoders);
          theValue[globalValuesIndex] = NoWrapZeroToOne(theValue[globalValuesIndex] + delta);

          #ifdef USB_SERIAL
            Serial.print(encIndex); Serial.print("  ");
            Serial.println(theValue[globalValuesIndex]);
          #endif
          #ifdef USB_MIDI
          usbMIDI.sendControlChange(encIndex, 127.0 * theValue[globalValuesIndex], currentPage+1); 
          //midi channels start at one, not zero
          #endif
        }
      }
      
    }//looping through the multiplexers
    MuxSel (muxSize, muxSelectPins, ((s+1) % numSelectLines)); //select the next pin
                                 //modulus ^ sets us back to the 0th select line
    
    
    delayMicroseconds(2);// wait for the mux's transition
//    delay(800);
  }//looping through select lines

  //leds:
  ledUpdate(theLedController, pageColors);


  #ifdef USB_MIDI
    while(usbMIDI.read()) {} //read incoming messages but do nothing about them
  #endif
}

void ledUpdate (ledController& controllerToUpdate, byte currentPageColors[3 * numPages]) {

 //create this variable inside the next for loop to allow different knobs to be different colors
  byte newColor[3] = {currentPageColors[(currentPage*3)], 
  currentPageColors[(currentPage*3)+1], currentPageColors[(currentPage*3)+2]};

 //update all led rings
  for (int i=0; i<numEncoders; i++) { 
    controllerToUpdate.knobLedUpdate(i, theValue[(numEncoders*currentPage)+i], newColor);
    //controllerToUpdate.knobLedUpdate(i, .5, newColor); //test without encoders
  }

 //update all key backlights
  
  
  controllerToUpdate.show();
}

int encoderChange (Encoder* encInput) {
  
  return encInput->readAndReset();
  /*
  int d = encInput->readAndReset();
 
  if (d != 0) {
    Serial.println(d);  }
  
  return d; */
}


byte findControlNumber (byte encoderIndex) {
  return ((currentPage * numEncoders) + encoderIndex);
}

int findEncoderByLed (int ledIndex) {
  return ledIndex / ledsPerEnc;
}


float ledValTransfer (byte val, int ledNumber) {
  static const byte rangePerLed = 255 / ledsPerEnc;
  return (float(constrain(map(val, ledNumber * rangePerLed, ((ledNumber + 1) * rangePerLed), 0, 255), 0, 255)) / 255.0);
}


//stackoverflow.com/questions/4840696/prevent-a-byte-from-wrapping-back-to-0-when-incremented-over-255
byte NoWrapIntToByte(int x)
{
    int tmp;
    tmp = max(x, 0);
    tmp = min(255, tmp);
    return (byte)tmp;
} 
//modified from that
float NoWrapZeroToOne(float x)
{
    float tmp;
    tmp = max(x, 0);
    tmp = min(1, tmp);
    return tmp;
} 


void MuxSel (byte mux_size, int mux_sel_pins[muxSize], byte selection){

  bool sel_bin[mux_size]; //to store selection as binary

  {
    int i = 0;
    int num = selection;
    while (i <= mux_size) {
      sel_bin[i] = bool(num % 2);
      num = num / 2;
      i++;
    }
  }

  for (int i=0; i<mux_size; i++) {
    digitalWrite(mux_sel_pins[i], sel_bin[i]); 
  }
}
