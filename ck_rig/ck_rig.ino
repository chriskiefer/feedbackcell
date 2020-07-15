#include <ADC.h>
#include <ADC_Module.h>
//#include <RingBuffer.h>
//#include <RingBufferDMA.h>

#include <Encoder.h>

#include <OctoWS2811.h>


//https://forum.pjrc.com/threads/25395-Teensy-Quick-Reference-Code-Examples-Tips-and-Tricks
//https://forum.pjrc.com/threads/25532-ADC-library-update-now-with-support-for-Teensy-3-1
//info: https://github.com/nox771/i2c_t3
//mpr121 lib https://github.com/sparkfun/MPR121_Capacitive_Touch_Breakout

#include <i2c_t3.h>
#include "mpr121.h"

// Memory
#define MEM_LEN 256
char databuf[MEM_LEN];
int count;

ADC *adc = new ADC();


//encoders
Encoder blueEncoder(0,1);

#define MPR121_IRQ 24

boolean touchStates[12]; //to keep track of the previous touch states


//neopixels

const int ledsPerStrip = 2;

DMAMEM int displayMemory[ledsPerStrip*1];
int drawingMemory[ledsPerStrip*1];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);



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

//  leds.begin();
//  leds.show();
  
    adc->adc0->setAveraging(13); // set number of averages
    adc->adc0->setResolution(13); // set bits of resolution
    adc->adc1->setAveraging(13); // set number of averages
    adc->adc1->setResolution(13); // set bits of resolution

    // it can be any of the ADC_CONVERSION_SPEED enum: VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED_16BITS, HIGH_SPEED or VERY_HIGH_SPEED
    // see the documentation for more information
    // additionally the conversion speed can also be ADACK_2_4, ADACK_4_0, ADACK_5_2 and ADACK_6_2,
    // where the numbers are the frequency of the ADC clock in MHz and are independent on the bus speed.
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_LOW_SPEED); // change the conversion speed
    // it can be any of the ADC_MED_SPEED enum: VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED or VERY_HIGH_SPEED
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_LOW_SPEED); // change the sampling speed

    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_LOW_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_LOW_SPEED); // change the sampling speed

    
    pinMode(LED_BUILTIN,OUTPUT);    // LED
    digitalWrite(LED_BUILTIN,HIGH);  // LED on
    pinMode(MPR121_IRQ, INPUT_PULLUP); //IRQ

    //switches
    pinMode(27, INPUT_PULLUP);
    pinMode(28, INPUT_PULLUP);
    pinMode(29, INPUT_PULLUP);
    pinMode(30, INPUT_PULLUP);

    pinMode(5, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);
    pinMode(7, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);


    //analogue stuff
    // Setup for Master mode, pins 18/19, external pullups, 400kHz, 200ms default timeout
//    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 100000);
//    Wire.setDefaultTimeout(200000); // 200ms

    // Data init
//    memset(databuf, 0, sizeof(databuf));
//    count = 0;
//
//    mpr121_setup();

    
    Serial.begin(115200);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    }

}
  

unsigned int r=100;
unsigned int gr = 50;
unsigned int bl = 150;

int ledCounter = 0;
bool ledOn = 1;
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
  //readTouchInputs();
  //Serial.println(adc->analogRead(A8, ADC_0));
//  Serial.print(blueEncoder.read());
//  Serial.print('\t');
  //Serial.println();
  
//  leds.setPixel(0, (r << 16) + (gr << 8) + bl);
//  r = (r + 1);
//  gr = (gr + 2);
//  bl = (bl + 3);
//  if (r > 255) r -= 255;
//  if (gr > 255) gr -= 255;
//  if (bl > 255) bl -= 255;
//  leds.show();

  //knobs
  unsigned short blueKnob1 = adc->analogRead(A9, ADC_0);
  unsigned short blueKnob2 = adc->analogRead(A8, ADC_0);
  unsigned short blueKnob3 = adc->analogRead(A7, ADC_0);
  unsigned short blueKnob4 = adc->analogRead(A5, ADC_0);
  unsigned short blueKnob5 = adc->analogRead(A6, ADC_0);
  Serial.write(blueKnob1 >> 8);
  Serial.write(blueKnob1 & 255);
  Serial.write(blueKnob2 >> 8);
  Serial.write(blueKnob2 & 255);
  Serial.write(blueKnob3 >> 8);
  Serial.write(blueKnob3 & 255);
  Serial.write(blueKnob4 >> 8);
  Serial.write(blueKnob4 & 255);
  Serial.write(blueKnob5 >> 8);
  Serial.write(blueKnob5 & 255);

  unsigned short redKnob1 = adc->analogRead(A4, ADC_0);
  unsigned short redKnob2 = adc->analogRead(A3, ADC_0);
  unsigned short redKnob3 = adc->analogRead(A2, ADC_0);
  unsigned short redKnob4 = adc->analogRead(A1, ADC_0);
  unsigned short redKnob5 = adc->analogRead(A0, ADC_0);
  Serial.write(redKnob1 >> 8);
  Serial.write(redKnob1 & 255);
  Serial.write(redKnob2 >> 8);
  Serial.write(redKnob2 & 255);
  Serial.write(redKnob3 >> 8);
  Serial.write(redKnob3 & 255);
  Serial.write(redKnob4 >> 8);
  Serial.write(redKnob4 & 255);
  Serial.write(redKnob5 >> 8);
  Serial.write(redKnob5 & 255);

  unsigned short yellowKnob1 = adc->analogRead(A13, ADC_1);
  unsigned short yellowKnob2 = adc->analogRead(A12, ADC_1);
  unsigned short yellowKnob3 = adc->analogRead(A22, ADC_1);
  unsigned short yellowKnob4 = adc->analogRead(A21, ADC_0);
  unsigned short yellowKnob5 = adc->analogRead(A20, ADC_1);
  Serial.write(yellowKnob1 >> 8);
  Serial.write(yellowKnob1 & 255);
  Serial.write(yellowKnob2 >> 8);
  Serial.write(yellowKnob2 & 255);
  Serial.write(yellowKnob3 >> 8);
  Serial.write(yellowKnob3 & 255);
  Serial.write(yellowKnob4 >> 8);
  Serial.write(yellowKnob4 & 255);
  Serial.write(yellowKnob5 >> 8);
  Serial.write(yellowKnob5 & 255);

  //switches
  byte switch1 = digitalRead(30);
  byte switch2 = digitalRead(29);
  byte switch3 = digitalRead(28);
  byte switch4 = digitalRead(27);
  Serial.write(switch1);
  Serial.write(switch2);
  Serial.write(switch3);
  Serial.write(switch4);
  
  byte switch5 = digitalRead(5);
  byte switch6 = digitalRead(6);
  byte switch7 = digitalRead(7);
  byte switch8 = digitalRead(8);
  Serial.write(switch5);
  Serial.write(switch6);
  Serial.write(switch7);
  Serial.write(switch8);

  Serial.write(255);
  Serial.write(255);
  
  digitalWrite(LED_BUILTIN,ledOn);  // LED on
  ledCounter++;
  if (ledCounter == 20) {
    ledOn = !ledOn;
    ledCounter = 0;
  }
  delay(5);
}
