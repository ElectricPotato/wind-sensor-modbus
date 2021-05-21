/*
  Modbus sensor
*/

#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

#define ledPin LED_BUILTIN

//Jumpers to set bus ID
#define IDJumperPin0 7 //LSB
#define IDJumperPin1 8 //MSB

 //using an IO pin as ground for the jumpers means less wiring
#define JmprPinGND0 9
#define JmprPinGND1 6


// wind sensor pins
#define analogPinForRV    1
#define analogPinForTMP   0

// wind sensor vars
const float zeroWindAdjustment =  .2; // negative numbers yield smaller wind speeds and vice versa.

int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;    //RV output from wind sensor 
float RV_Wind_Volts;
unsigned long lastMillis;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
int readingX500;

//return wind sensor value multiplied by 500 for better resolution
int getWindSpeedMPHx500(){
    TMP_Therm_ADunits = analogRead(analogPinForTMP);
    RV_Wind_ADunits = analogRead(analogPinForRV);
    RV_Wind_Volts = (RV_Wind_ADunits *  0.0048828125);

    zeroWind_ADunits = -0.0006*((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39

    zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;  

    // This from a regression from data in the form of 
    // Vraw = V0 + b * WindSpeed ^ c
    // V0 is zero wind at a particular temperature
    // The constants b and c were determined by some Excel wrangling with the solver.

    readingX500 = 500*pow(((RV_Wind_Volts - zeroWind_volts) /.2300) , 2.7265);
    if(readingX500>65500){
      readingX500=65500;//set max value for sending using 16 bits
    }
    
   return readingX500;   
}


//get the bus ID of the device from the jumpers on each module
//no jumper = logic high = !1 = 0
//jumper = pulled low = !0 = 1
int getJumperID(){
  //return 1;
  return !digitalRead(IDJumperPin0) + 2*!digitalRead(IDJumperPin1); //+4*!digitalRead(IDJumperPin2)+8*...
}

void setup() {
  //Serial.begin(9600);//debug

  //Serial.println("BEGIN");//debug
  pinMode(IDJumperPin0,INPUT_PULLUP);
  pinMode(IDJumperPin1,INPUT_PULLUP);

  pinMode(JmprPinGND0,OUTPUT);
  pinMode(JmprPinGND1,OUTPUT);
  digitalWrite(JmprPinGND0, LOW);
  digitalWrite(JmprPinGND1, LOW);
  
  delay(100);
  int ID = getJumperID()+1;
  // start the Modbus RTU server, with id set by jumpers (from 0 to 3)
  if (!ModbusRTUServer.begin(ID, 9600)) {
    //Serial.println("Failed to start Modbus RTU Server!");//debug
    while (1);
  }

  // configure the LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // at start adress 0x00, configure 1 input register
  ModbusRTUServer.configureInputRegisters(0x00, 1);
}



void loop() {
  // poll for Modbus RTU requests
  ModbusRTUServer.poll();
  uint16_t sensorVal = getWindSpeedMPHx500();
  ModbusRTUServer.inputRegisterWrite(0x00, sensorVal);
}
