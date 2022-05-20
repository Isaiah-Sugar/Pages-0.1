


//led indicies for the first led in each led ring (around the encoder)
const int knobLedIndicies[numEncoders]={0,8,16,24,32,40,48,56,64,72,80,88,96,104,112,120};


const int ledPin = 1;
const int ledsPerEnc = 8;
const int numLeds = (numEncoders * ledsPerEnc) + numButtons;
DMAMEM byte displayMemory[numLeds*12]; // 12 bytes per LED

class ledController {

  private: 
  byte drawingMemory[numLeds*3];         //  3 bytes per LED (rgb)

  WS2812Serial leds = WS2812Serial(numLeds, displayMemory, drawingMemory, 1, WS2812_GRB);


  
  public:
  
  ledController() {

    pinMode(ledPin, OUTPUT);
    leds.begin();
    
    }; //constructor
  



  

  //i figure it makes sense to abstract this away, especially when I call it from outside this class
  void writeLedColor(int index, byte newColor[3]) {
    leds.setPixel(index, newColor[0], newColor[1], newColor[2]);
  }
  
  
  //update a ring of led's (buffers led values)
  void knobLedUpdate(int knobIndex, float newValue, byte newColor [3]){
    int firstLed = knobLedIndicies[knobIndex];
    
    for (int i = 0; i<ledsPerEnc; i++) { //each led assigned to the current knob
      
      float lowBound = float(i)/float(ledsPerEnc); //7/8
      float highBound = lowBound + (1.0/ledsPerEnc) - .001; //8/8 = 1.0

      float ledValue = 0;
      if (newValue<lowBound) 
        ledValue = 0;
      else if (newValue>highBound)
        ledValue = 1;
      else
        ledValue = fmod(newValue, (1.0/float(ledsPerEnc))); 


      //float ledValue = max(min((newValue * ledsPerEnc) + i, 1),0);

      
      ledValue *= .05; //otherwise i would be blinded during testing


      byte ledColor [3] = {
        byte(float(newColor[0]) * ledValue), 
        byte(float(newColor[1]) * ledValue), 
        byte(float(newColor[2]) * ledValue) };
      
      writeLedColor(firstLed+i, ledColor);
    }
    //delay(2);
  }
  
  //update the led's from buffered data
  void show(){
    leds.show();
  }

  
}
