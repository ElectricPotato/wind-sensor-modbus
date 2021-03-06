/*
  Modbus Host (client)
*/

#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  //start modbus client at baud 9600. Client does not have an ID, as its not addressed by server
  if (!ModbusRTUClient.begin(9600)) {
    Serial.println("Failed to start Modbus RTU Client! Unplug and replug the USB cable");
    while (1);
  }
}

//the number of sensors connected, with IDs ranging from 1 to nSensorNodes inclusive
#define nSensorNodes 4

float sensorReading[nSensorNodes];
uint16_t sensorVal;

void loop() {
  for(int i=0;i<nSensorNodes;i++){
      //read register 0x00 (sensor reading), on device with ID i+1
      sensorVal=ModbusRTUClient.inputRegisterRead(i+1, 0x00); //read from IDs 1 through nSensorNodes

      //a reading of -1 means the request for a sensor reading timed out or corrupted (bad checksum)
      if(sensorVal==-1){
        //Serial.print("Fail read ID");//debug
        //Serial.print(i+1);//debug
        //Serial.print(" ");//debug
        //Serial.println(ModbusRTUClient.lastError());//debug
        sensorReading[i]=0;
      }else{
        //divide the recieved value by 500, as its multiplied by 500 at the other end
        //1 is added simply so that you see a non-zero value (0.002) when the sensor is connected, and 0 if its not
        sensorReading[i]=(sensorVal+1)/500.0;
      }
      delay(10); //loop delay: if the readings are taken to frequently, the modbus library times out
  }
  
  //output format: 4 numbers seperated by tabs, then newline, nothing else
  //1.234\t1.234\t1.234\t1.234\t\n
  for(int i=0;i<nSensorNodes;i++){
    Serial.print(sensorReading[i],3);
    Serial.print("\t");
  }
  Serial.println();
  delay(1);
}
