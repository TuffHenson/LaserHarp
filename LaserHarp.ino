/////////////////////////////////////////////////////////////////////////
//                                                                     //
//   Laser Harp I by Johan Berglund, January 2017                      //
//                                                                     //
//   Massive reworking for Tuff Henson's Harp                          //
//   Using 10 lasers and a system of buttons to choose scales          //
//                                                                     //
/////////////////////////////////////////////////////////////////////////
#define MIDI_CH 0            // MIDI channel (0 is MIDI channel 1 on DIN MIDI) 
int VELOCITY = 64;          // MIDI note velocity (64 for medium velocity, 127 for maximum)
#define START_NOTE 0        // MIDI start note (middle C)
#define BEAMS 10             // number of laser beams (up to 16)
#define OCTAVE_UP 10         //Octaves up and down
#define OCTAVE_DOWN 11

int C = 17;                //This is the map of our keyboard
int CSHARP = 16;
int D = 18;
int EFLAT = 15;
int E = 19;
int F = 20;
int FSHARP = 12;
int G = 21;
int AFLAT = 9;
int A = 22;
int BFLAT = 8;
int B = 23;

int MAJOR = 7;             //Map of our scale type buttons
int MINOR = 6;
int CHROM = 5;

#define VOL_PIN A0      // pin for sensitivity adjustment potentiometer/ volume potentiometer
#define CHECK_INTERVAL 5     // interval in ms for matrix check

unsigned long currentMillis = 0L;
unsigned long statusPreviousMillis = 0L;

byte sensorPin[10]       = {A17,A16,A15,A13,A12,A18,A19,A20,A11,A10}; // teensy analog input pins
byte activeNote[10]      = {0,0,0,0,0,0,0,0,0,0}; // keeps track of active notes
byte sensedNote;            // current reading
int noteNumber;             // calculated midi note number
int scale = 1;              // scale setting
int octave = 60;             // octave setting
int transposition = 0;      // transposition setting, used to select 

int thrValue = 880;         // sensitivity value
int offThr;

int ODUp = 0;
int OUUp = 0;

byte scaleNote[3][10] = {
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },  //chromatic
  { 0, 2, 4, 5, 7, 9,11,12,14,16 },  //major/ionian
  { 0, 2, 3, 5, 7, 8,10,12,14,15 },  //natural minor/aeolian                              
};

void setup() {
  pinMode(C, INPUT_PULLUP);
  pinMode(CSHARP, INPUT_PULLUP);
  pinMode(D, INPUT_PULLUP);
  pinMode(EFLAT, INPUT_PULLUP);
  pinMode(E, INPUT_PULLUP);
  pinMode(F, INPUT_PULLUP);
  pinMode(FSHARP, INPUT_PULLUP);
  pinMode(G, INPUT_PULLUP);
  pinMode(AFLAT, INPUT_PULLUP);
  pinMode(A, INPUT_PULLUP);
  pinMode(BFLAT, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);
  pinMode(MAJOR, INPUT_PULLUP);
  pinMode(MINOR, INPUT_PULLUP);
  pinMode(CHROM, INPUT_PULLUP);
  pinMode(OCTAVE_UP, INPUT_PULLUP); 
  pinMode(OCTAVE_DOWN, INPUT_PULLUP);
  Serial1.begin(31250);  // start serial with midi baudrate 31250
  Serial.begin(31250);  // start serial with midi baudrate 31250
  Serial1.flush();
}

void loop() {
  currentMillis = millis();
  
  if(digitalRead(OCTAVE_UP) == LOW && digitalRead(OCTAVE_DOWN) == LOW){               //If the octave buttons are both pressed at the same time, adjust sensors
     thrValue = map(analogRead(A0), 0, 1023, 60, 1023);                          // set sensitivity for light sensors
     Serial.println(thrValue);
     offThr = thrValue - 50;
  }
  else if(digitalRead(OCTAVE_UP)== LOW && OUUp == 1 && octave < 85){//octave up button, send octave up
    octave += 12;
    OUUp = 0;
  }
  else if(digitalRead(OCTAVE_DOWN)== LOW && ODUp == 1 && octave > 11){//octave down button, send octave down
    octave -= 12;
    ODUp = 0;
  }

  
  if(digitalRead(MAJOR) == LOW && scale != 1){           //Sets the scale that we will use
    scale = 1;
    Serial.println(scale);
  }
  if(digitalRead(MINOR)== LOW && scale != 2){
    scale = 2;
    Serial.println(scale);
  }
  if(digitalRead(CHROM)== LOW && scale != 0){
    scale = 0;
    Serial.println(scale);
  }

  if(digitalRead(C)== LOW && transposition != 0){
    transposition = 0;
    Serial.println(transposition);
  }
  if(digitalRead(CSHARP)== LOW && transposition != 1){
    transposition = 1;
    Serial.println(transposition);
  }if(digitalRead(D)== LOW && transposition != 2){
    transposition = 2;
    Serial.println(transposition);
  }if(digitalRead(EFLAT)== LOW && transposition != 3){
    transposition = 3;
    Serial.println(transposition);
  }if(digitalRead(E)== LOW && transposition != 4){
    transposition = 4;
    Serial.println(transposition);
  }if(digitalRead(F) == LOW && transposition != 5){
    transposition = 5;
    Serial.println(transposition);
  }if(digitalRead(FSHARP)== LOW && transposition != 6){
    transposition = 6;
    Serial.println(transposition);
  }if(digitalRead(G)== LOW && transposition != 7){
    transposition = 7;
    Serial.println(transposition);
  }if(digitalRead(AFLAT)== LOW && transposition != 8){
    transposition = 8;
    Serial.println(transposition);
  }if(digitalRead(A)== LOW && transposition != 9){
    transposition = 9;
    Serial.println(transposition);
  }if(digitalRead(BFLAT)== LOW && transposition != 10){
    transposition = 10;
    Serial.println(transposition);
  }if(digitalRead(B)== LOW && transposition != 11){
    transposition = 11;
    Serial.println(transposition);
  }
  
  if ((unsigned long)(currentMillis - statusPreviousMillis) >= CHECK_INTERVAL) {

    //This code ensures that one press of an octave button equals one octave shift
    if(digitalRead(OCTAVE_DOWN) == HIGH && ODUp == 0){
    ODUp = 1;
    }
    if(digitalRead(OCTAVE_UP) == HIGH && OUUp == 0){
      OUUp = 1;
    }
    
    for (int scanSensors = 0; scanSensors < BEAMS; scanSensors++) {     // scan matrix for changes and send note on/off accordingly
      if (!activeNote[scanSensors]){
        sensedNote = (analogRead(sensorPin[scanSensors]) > thrValue);   // if note is off, sensedNote gets high if sensor value is higher than thrValue
      } else {
        sensedNote = (analogRead(sensorPin[scanSensors]) > offThr);     // if note is on, sensedNote only goes low if sensor value goes below offThr
      }
      if (sensedNote != activeNote[scanSensors]) {
        noteNumber = scaleNote[scale][scanSensors] + octave + transposition;
        Serial.println(noteNumber);
        if ((noteNumber < 128) && (noteNumber > -1)) {                  // we don't want to send midi out of range
          if (sensedNote){
              VELOCITY = map(analogRead(A0), 0, 1023, 0, 123);
              usbMIDI.sendNoteOn(noteNumber, VELOCITY, MIDI_CH + 1);    // send Note On, USB MIDI
              midiSend((0x90 | MIDI_CH), noteNumber, VELOCITY);         // send Note On, DIN MIDI
          } else {
              VELOCITY = map(analogRead(A0), 0, 1023, 0, 123);
              usbMIDI.sendNoteOff(noteNumber, VELOCITY, MIDI_CH + 1);   // send note Off, USB MIDI
              midiSend((0x80 | MIDI_CH), noteNumber, VELOCITY);         // send Note Off, DIN MIDI
          }
        }  
        activeNote[scanSensors] = sensedNote;         
      } 
    }
    statusPreviousMillis = currentMillis;                               // reset interval timing
  }
}

//  Send a three byte midi message on serial 1 (DIN MIDI) 
void midiSend(byte midistatus, byte data1, byte data2) {
  Serial1.write(midistatus);
  Serial1.write(data1);
  Serial1.write(data2);
}
