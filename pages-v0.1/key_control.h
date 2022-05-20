
//location of every button on the multiplexers 
const muxAddress keyAddresses [numButtons] = {
  muxAddress(4, 7),
  muxAddress(4, 6),
  muxAddress(4, 5),
  muxAddress(4, 4),
  muxAddress(4, 3),
  muxAddress(4, 2),
  muxAddress(4, 1),
  muxAddress(4, 0),
  muxAddress(5, 7),
  muxAddress(5, 6),
  muxAddress(5, 5),
  muxAddress(5, 4),
  muxAddress(5, 3),
  muxAddress(5, 2),
  muxAddress(5, 1),
  muxAddress(5, 0),
  muxAddress(6, 7),
  muxAddress(6, 6),
  muxAddress(6, 5),
  muxAddress(6, 4),
  muxAddress(6, 3),
  muxAddress(6, 2),
  muxAddress(6, 1),
  muxAddress(6, 0),
  muxAddress(7, 7),
  muxAddress(7, 6),
  muxAddress(7, 5),
  muxAddress(7, 4),
  muxAddress(7, 3),
  muxAddress(7, 2),
  muxAddress(7, 1),
  muxAddress(8, 0),
  muxAddress(8, 7),
  muxAddress(8, 6),
  muxAddress(8, 5),
  muxAddress(8, 4),
  muxAddress(8, 3),
  muxAddress(8, 2),
  muxAddress(8, 1),
  muxAddress(8, 0),
  };
//yes that's really the best way to do that


static int buttonIndicies [numMuxes] [numSelectLines];

enum keyType {pagesButton = 2, basicMidi=3, options=10};

keyType keyTypesArray [numButtons] = {
  basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, 
  basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, 
  basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, 
  pagesButton, pagesButton, pagesButton, pagesButton, pagesButton, pagesButton, pagesButton, pagesButton, 
  basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi, basicMidi
};




//there's gonna be a base class for 'button'
//there's gonna be a subclass for each 'type' of button, based on their functionality
//virtual function 'pushed', 'released', or whatever. Subclasses override that to add their functionality
//pass led library object into object, probably as a reference. only modify led value when needed

class baseButton { //base class for buttons, make a subclass for specific button types
  protected:

  int pinToRead;
  muxAddress buttonAddress = muxAddress(0,0);
  int assignedLedIndex;
  int assignedButtonIndex;
  
  bool currentState = false;
  byte offColor [3] = {0,0,0};
  byte onColor [3] = {255,255,255};
  float brightness = .15;
  ledController* theLedController;

  void updateLed (byte newColor[3]) {
    byte brightenedColor [3] =
    {byte(float(newColor[0]) * brightness), byte(float(newColor[1]) * brightness), byte(float(newColor[2]) * brightness)};

    theLedController->writeLedColor(assignedLedIndex, brightenedColor);
  }

  
  void updateLed (byte newColor_1, byte newColor_2, byte newColor_3) {
    byte brightenedColor [3] =
    {byte(float(newColor_1) * brightness), byte(float(newColor_2) * brightness), byte(float(newColor_3) * brightness)};

    theLedController->writeLedColor(assignedLedIndex, brightenedColor);
  }

  public:
  
  baseButton(const muxAddress& address, ledController* ledControllerIn, int ledIndex, int buttonIndex) {
    buttonAddress = address;
    assignedLedIndex = ledIndex;
    assignedButtonIndex = buttonIndex;
    pinToRead = muxSignalPins[address.muxNumber];
    theLedController = ledControllerIn;
  };


  //called repeatedly when on this button's select line
  void updateButton() {
      
    bool newState = digitalRead(pinToRead);
    if (currentState != newState){
      currentState = newState;
      //call a function based on the state
      if(!currentState) {
        Pressed();
      }
      else {
        Released();
      }
    }
  };

  
  void setKeyBrightness (float newBrightness) {
    brightness = newBrightness;
    #ifdef USB_SERIAL
    Serial.println(assignedButtonIndex);
    #endif
  }
  
  void assignColors(byte onColor_in[3], byte offColor_in[3]) {
    for (int i=0; i<3; i++){
      onColor[i] = onColor_in[i];
      offColor[i] = offColor_in[i];
    }
  }

//override in subclasses:
  virtual void Pressed(); 
  virtual void Released();
};


class pageButton : public baseButton { //page switch button subclass
  
  private:
  int assignedPage = 0;
  byte* currentPage;
  byte* pageColors;

  public:
  pageButton(const muxAddress& address, ledController* daController, int ledIndex, int buttonIndex,
  int assignedPage, byte* pagePointer, byte *pageColors_in) : 
  baseButton(address, daController, ledIndex, buttonIndex) {
    assignPage(assignedPage);
    setPagePointer(pagePointer);
    setPageColorArrayPointer(pageColors_in);
    pageUpdate();
    byte newOffColor[3] = {pageColors_in[assignedPage*3],pageColors_in[(assignedPage*3)+1],pageColors_in[(assignedPage*3)+2]};
    Serial.println(newOffColor[2]);
    byte newOnColor[3] = {255,255,255};
    assignColors(newOnColor,newOffColor);
  }

  void Pressed() {
    *currentPage = assignedPage;
    updatePageButtonLeds(); //calls pageUpdate for every pageButton object

    #ifdef USB_SERIAL
    Serial.println(assignedLedIndex);
    #endif
  }
  
  void Released(){ 
    pageUpdate();
  }


  void pageUpdate() {
    if (!currentState | (*currentPage == assignedPage))
        updateLed(onColor);
    else {
        //byte thePageColor[3] = {pageColors[assignedLedIndex*3]
        updateLed(offColor);
    }
  }
  
  void assignPage(int newPage) {
    assignedPage = newPage;
  }

  void setPagePointer(byte* newPointer) {
    currentPage = newPointer;
  }

  void setPageColorArrayPointer(byte* newPageColorsPointer) {
    pageColors = newPageColorsPointer;
  }


};


#ifdef USB_SERIAL
class basicSerialButton : public baseButton {
  public:
  basicSerialButton(const muxAddress& address, ledController* daController, int ledIndex, int buttonIndex) : 
  baseButton(address, daController, ledIndex, buttonIndex) {
  
  }


  void Pressed() {
    Serial.println(assignedButtonIndex);
    byte theColorToUse[3] = {255,255,255};
    updateLed(theColorToUse);
  }
  void Released(){
    
    byte theColorToUse[3] = {0, 0, 0};
      
    updateLed(theColorToUse);
  };
  
};
#endif


#ifdef USB_MIDI
class basicMidiButton : public baseButton {
  
  protected:
  byte onColor[3] = {255,255,255};
  byte offColor[3] = {0,0,0};

  byte noteNumber;
  byte midiChannel;
  
  public:
  basicMidiButton(const muxAddress& address, ledController* daController, int ledIndex, int buttonIndex, byte noteNumber_in, byte midiChannel_in) : 
  baseButton(address, daController, ledIndex, buttonIndex) {
    noteNumber = noteNumber_in;
    midiChannel = midiChannel_in;
  }


  void Pressed() {
    updateLed(onColor);
    usbMIDI.sendNoteOn(noteNumber, 255, midiChannel);
  }
  void Released(){
    updateLed(offColor);
    usbMIDI.sendNoteOff(noteNumber, 255, midiChannel);
  };

  
};
#endif


muxAddress buttonIndexToAddress (int index) {
  return keyAddresses[index];
}

//allows the buttonAddressToIndex function to look data up faster, call in setup()
void initializeButtonIndicies () {
  for (int i=0; i<numButtons; i++) {
    buttonIndicies[keyAddresses[i].muxNumber] [keyAddresses[i].muxLine] = i;
  }
}

int buttonAddressToIndex (int number, int line) {
  return buttonIndicies [number] [line];
}
