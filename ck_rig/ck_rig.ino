//info: https://github.com/nox771/i2c_t3
//mpr121 lib https://github.com/sparkfun/MPR121_Capacitive_Touch_Breakout

#include <i2c_t3.h>
#include "mpr121.h"

// Memory
#define MEM_LEN 256
char databuf[MEM_LEN];
int count;

#define MPR121_IRQ 24

boolean touchStates[12]; //to keep track of the previous touch states

boolean checkMPR121Interrupt(void){
  return digitalRead(MPR121_IRQ);
}


void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}

void mpr121_setup(void){

  set_register(0x5A, ELE_CFG, 0x00); 

  // Section A - Controls filtering when data is > baseline.
  set_register(0x5A, MHD_R, 0x01);
  set_register(0x5A, NHD_R, 0x01);
  set_register(0x5A, NCL_R, 0x00);
  set_register(0x5A, FDL_R, 0x00);

  // Section B - Controls filtering when data is < baseline.
  set_register(0x5A, MHD_F, 0x01);
  set_register(0x5A, NHD_F, 0x01);
  set_register(0x5A, NCL_F, 0xFF);
  set_register(0x5A, FDL_F, 0x02);

  // Section C - Sets touch and release thresholds for each electrode
  set_register(0x5A, ELE0_T, TOU_THRESH);
  set_register(0x5A, ELE0_R, REL_THRESH);

  set_register(0x5A, ELE1_T, TOU_THRESH);
  set_register(0x5A, ELE1_R, REL_THRESH);

  set_register(0x5A, ELE2_T, TOU_THRESH);
  set_register(0x5A, ELE2_R, REL_THRESH);

  set_register(0x5A, ELE3_T, TOU_THRESH);
  set_register(0x5A, ELE3_R, REL_THRESH);

  set_register(0x5A, ELE4_T, TOU_THRESH);
  set_register(0x5A, ELE4_R, REL_THRESH);

  set_register(0x5A, ELE5_T, TOU_THRESH);
  set_register(0x5A, ELE5_R, REL_THRESH);

  set_register(0x5A, ELE6_T, TOU_THRESH);
  set_register(0x5A, ELE6_R, REL_THRESH);

  set_register(0x5A, ELE7_T, TOU_THRESH);
  set_register(0x5A, ELE7_R, REL_THRESH);

  set_register(0x5A, ELE8_T, TOU_THRESH);
  set_register(0x5A, ELE8_R, REL_THRESH);

  set_register(0x5A, ELE9_T, TOU_THRESH);
  set_register(0x5A, ELE9_R, REL_THRESH);

  set_register(0x5A, ELE10_T, TOU_THRESH);
  set_register(0x5A, ELE10_R, REL_THRESH);

  set_register(0x5A, ELE11_T, TOU_THRESH);
  set_register(0x5A, ELE11_R, REL_THRESH);

  // Section D
  // Set the Filter Configuration
  // Set ESI2
  set_register(0x5A, FIL_CFG, 0x04);

  // Section E
  // Electrode Configuration
  // Set ELE_CFG to 0x00 to return to standby mode
  set_register(0x5A, ELE_CFG, 0x0C);  // Enables all 12 Electrodes


  // Section F
  // Enable Auto Config and auto Reconfig
  /*set_register(0x5A, ATO_CFG0, 0x0B);
  set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
  set_register(0x5A, ATO_CFGT, 0xB5);*/  // Target = 0.9*USL = 0xB5 @3.3V

  set_register(0x5A, ELE_CFG, 0x0C);

}

void readTouchInputs(){
  if(!checkMPR121Interrupt()){

    //read the touch state from the MPR121
    Wire.requestFrom(0x5A,2); 

    byte LSB = Wire.read();
    byte MSB = Wire.read();

    uint16_t touched = ((MSB << 8) | LSB); //16bits that make up the touch states


    for (int i=0; i < 12; i++){  // Check what electrodes were pressed
      if(touched & (1<<i)){

        if(touchStates[i] == 0){
          //pin i was just touched
          Serial.print("pin ");
          Serial.print(i);
          Serial.println(" was just touched");

        }else if(touchStates[i] == 1){
          //pin i is still being touched
        }  

        touchStates[i] = 1;      
      }else{
        if(touchStates[i] == 1){
          Serial.print("pin ");
          Serial.print(i);
          Serial.println(" is no longer being touched");

          //pin i is no longer being touched
       }

        touchStates[i] = 0;
      }

    }

  }
}



void setup()
{
    pinMode(LED_BUILTIN,OUTPUT);    // LED
    digitalWrite(LED_BUILTIN,LOW);  // LED off
    pinMode(MPR121_IRQ, INPUT_PULLUP); //IRQ
    
    // Setup for Master mode, pins 18/19, external pullups, 400kHz, 200ms default timeout
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
    Wire.setDefaultTimeout(200000); // 200ms

    // Data init
    memset(databuf, 0, sizeof(databuf));
    count = 0;

    mpr121_setup();

    Serial.begin(115200);
}



void loop()
{
//    uint8_t target = 0x66; // target Slave address
//    size_t idx;
//
//    // Send string to Slave
//    //
//    if(digitalRead(12) == LOW)
//    {
//        digitalWrite(LED_BUILTIN,HIGH);   // LED on
//
//        // Construct data message
//        sprintf(databuf, "Data Message #%d", count++);
//
//        // Print message
//        Serial.printf("Sending to Slave: '%s' ", databuf);
//        
//        // Transmit to Slave
//        Wire.beginTransmission(target);   // Slave address
//        for(idx = 0; idx <= strlen(databuf); idx++) // Write string to I2C Tx buffer (incl. string null at end)
//            Wire.write(databuf[idx]);
//        Wire.endTransmission();           // Transmit to Slave
//
//        // Check if error occured
//        if(Wire.getError())
//            Serial.print("FAIL\n");
//        else
//            Serial.print("OK\n");
//
//        digitalWrite(LED_BUILTIN,LOW);    // LED off
//        delay(100);                       // Delay to space out tests
//    }
//
//    // Read string from Slave
//    //
//    if(digitalRead(11) == LOW)
//    {
//        digitalWrite(LED_BUILTIN,HIGH);   // LED on
//
//        // Print message
//        Serial.print("Reading from Slave: ");
//        
//        // Read from Slave
//        Wire.requestFrom(target, (size_t)MEM_LEN); // Read from Slave (string len unknown, request full buffer)
//
//        // Check if error occured
//        if(Wire.getError())
//            Serial.print("FAIL\n");
//        else
//        {
//            // If no error then read Rx data into buffer and print
//            idx = 0;
//            while(Wire.available())
//                databuf[idx++] = Wire.readByte();
//            Serial.printf("'%s' OK\n",databuf);
//        }
//
//        digitalWrite(LED_BUILTIN,LOW);    // LED off
//        delay(100);                       // Delay to space out tests
//    }
  readTouchInputs();
}

