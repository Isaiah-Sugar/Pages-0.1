
const byte muxSize = 3; //3 select lines
const byte numSelectLines = int(pow(2,muxSize));
int muxSelectPins[muxSize] = {3,4,5};

const byte numMuxes = 9;
int muxSignalPins[numMuxes] = {6,7,8,9,14,15,16,17,18}; //pins carrying signals from each mux
enum muxType {encoderCC=0, encoderCW=1, key=10}; //values for different devices connected to the muxes
muxType whatsOnThatMux [numMuxes] = {encoderCC,encoderCW,encoderCC,encoderCW,key,key,key,key,key};





//represents an address on a specific mux (specific pin on a specific mux)
class muxAddress { 
  public:
  int muxNumber; //which physical multiplexer is it connected to
  byte muxLine; //which select line is it connected to

  muxAddress (int number, byte line) { //constructor
    muxNumber = number;
    muxLine = line;
  }

  virtual bool operator== (const muxAddress &other) const {
    return ((muxNumber == other.muxNumber) && (muxLine == other.muxLine));
  }
};



//move this into the encoder file when that exists
//addresses of all the encoders' counterclockwise pins
const muxAddress encoderAddressesCC [numEncoders] = {
  muxAddress(0, 0),
  muxAddress(0, 1),
  muxAddress(0, 2),  
  muxAddress(0, 3), 
  muxAddress(0, 4), 
  muxAddress(0, 5),  
  muxAddress(0, 6), 
  muxAddress(0, 7),
  muxAddress(2, 0),
  muxAddress(2, 1),
  muxAddress(2, 2),  
  muxAddress(2, 3), 
  muxAddress(2, 4), 
  muxAddress(2, 5),  
  muxAddress(2, 6), 
  muxAddress(2, 7),
  };

//addresses of all the encoders' clockwise pins
const muxAddress encoderAddressesCW [numEncoders] = {
  muxAddress(1, 0),
  muxAddress(1, 1),
  muxAddress(1, 2),  
  muxAddress(1, 3), 
  muxAddress(1, 4), 
  muxAddress(1, 5),  
  muxAddress(1, 6), 
  muxAddress(1, 7),
  muxAddress(3, 0),
  muxAddress(3, 1),
  muxAddress(3, 2),  
  muxAddress(3, 3), 
  muxAddress(3, 4), 
  muxAddress(3, 5),  
  muxAddress(3, 6), 
  muxAddress(3, 7),
  };



static byte encoderIndicies [numMuxes] [numSelectLines];

void initializeEncoderIndicies() { //run this in setup
  //generates a 2d array which allows the findEncoderByAddress function to run faster.
  for (int i = 0; i<numEncoders; i++) {
    encoderIndicies[encoderAddressesCC[i].muxNumber] [encoderAddressesCC[i].muxLine] = i;
    encoderIndicies[encoderAddressesCW[i].muxNumber] [encoderAddressesCW[i].muxLine] = i;
  }
}



int findEncoderByAddress (int number, int line) {
  return encoderIndicies[number][line];
}



  
