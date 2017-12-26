//GPS Software serial
#include <SoftwareSerial.h>
SoftwareSerial gpsSerial = SoftwareSerial(3,4); // Connect the GPS RX/TX to arduino pins 3 and 4
#include "ubx_pvt.h"

int buttonState;               // the current reading from the input pin
int writeSD = false;

// SD Card 
#include <SPI.h>
#include <SD.h>
const int chipSelect = 10;
File dataFile;
char fileName[]="0000.csv";

// BMP280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bme;
float st_pressure = 1000.0;
float alt = 0;

// LCD
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3f, 16, 2);

void setup() 
{
  Serial.begin(9600);
  gpsSerial.begin(38400);

  pinMode(5, OUTPUT); // led pin
  pinMode(2, INPUT_PULLUP);
  
  // initialize the LCD
  lcd.begin();
  lcd.backlight();
  
  Serial.print("Init SD card...");
  lcd.print("Init SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    lcd.clear();
    lcd.print("SD failed...");
  }else  {
    Serial.println("SD Card OK.");
    lcd.clear();
    lcd.print("SD Card OK.");
  }
  delay(1000);
  
  // BMP begin
  
  if (!bme.begin()) {
    lcd.clear();
    lcd.print("No BMP");
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  }
  else { // calibrate
    lcd.clear();
    lcd.print("Calibrating");
    delay(1000);
    do {
      alt=bme.readAltitude(st_pressure);
      Serial.print(F("Approx altitude = ")); Serial.print(alt); Serial.println(" m");
      lcd.clear();
      lcd.print(alt);
       
      st_pressure += 0.1;
    } while (alt < 850.0);
    Serial.print(F("STPressure = "));Serial.println(st_pressure);
    Serial.print(F("Pressure = "));Serial.print(bme.readPressure());Serial.println(" Pa");
  }
  delay(1000);


  // Turn on the blacklight and print a message.
  lcd.clear();
  lcd.print("GPS searching...");
  delay(1000);
}

void loop() {
     
    if (digitalRead(2)==LOW) {
      writeSD = writeSD ^ 1;
      if (writeSD==true) {
        sprintf(fileName, "%02d%02d%02d%02d%", pvt.day, pvt.month, pvt.hour + 2, pvt.minute);
        Serial.println(fileName);
        dataFile = SD.open(fileName, FILE_WRITE);

        lcd.clear();
        lcd.print("New track");
        delay(1000);
      }
      else { 
        dataFile.close();

        lcd.clear();
        lcd.print("End of track");      
        delay(1000);
      }  
    }   

  if ( processGPS() ) {
  
    if (pvt.fixType>=1) {
      digitalWrite(5, HIGH);
      
      Serial.print("#SV: ");      Serial.print(pvt.numSV);
      Serial.print(" fixType: "); Serial.print(pvt.fixType);
      Serial.print(" Date:");     Serial.print(pvt.year); Serial.print("/"); Serial.print(pvt.month); Serial.print("/"); Serial.print(pvt.day); Serial.print(" "); Serial.print(pvt.hour); Serial.print(":"); Serial.print(pvt.minute); Serial.print(":"); Serial.print(pvt.second);
      Serial.print(" lat/lon: "); Serial.print(pvt.lat/10000000.0f); Serial.print(","); Serial.print(pvt.lon/10000000.0f);
      Serial.print(" Altitude: ");  Serial.print(pvt.height/1000.0f);
      Serial.print(" gSpeed: ");  Serial.print(pvt.gSpeed/1000.0f);
      Serial.print(" heading: "); Serial.print(pvt.heading/100000.0f);
      Serial.print(" hAcc: ");    Serial.print(pvt.hAcc/1000.0f);
      Serial.println();
  
      //set new filename
      //if (!strcmp(fileName,"0000.csv")) {
      //     sprintf(fileName, "%02d%02d%", pvt.hour + 2, pvt.minute ); 
      //}
       // data from BMP280
      Serial.print(F("Temperature = "));Serial.print(bme.readTemperature());Serial.println(" *C");
      Serial.print(F("Pressure = "));Serial.print(bme.readPressure());Serial.println(" Pa");
      Serial.print(F("Approx altitude = "));
      alt = bme.readAltitude(st_pressure);
      
      Serial.print(alt); Serial.println(" m");    
      lcd.clear();
      lcd.print(alt);
  
      //---------------------------------------------------------------------------
      // The routine for writing data to SDcard: 
      if (writeSD==true) {
        
        // if the file is available, write to it:
        if (dataFile) {
         
          dataFile.print(pvt.lat/10000000.0f,10);
          dataFile.print(",");
          dataFile.print(pvt.lon/10000000.0f,10);
          dataFile.print(",");
          dataFile.print(pvt.height/1000.0f,2);
          dataFile.print(",");
          dataFile.print(alt, 2);
          dataFile.print(",");
          dataFile.print(pvt.year);
          dataFile.print("-");      
          dataFile.print(pvt.month);
          dataFile.print("-");      
          dataFile.print(pvt.day);
          dataFile.print("T");
          dataFile.print(pvt.hour);
          dataFile.print(":");      
          dataFile.print(pvt.minute);
          dataFile.print(":");      
          dataFile.print(pvt.second);
          dataFile.print("T");
          dataFile.print("\n");
          
          Serial.print("WRITE "); Serial.println(fileName);
          lcd.setCursor(0, 1);
          lcd.print("Rec"); lcd.print(fileName);
        }
        else {
          Serial.print("NO Datafile ");
          lcd.setCursor(0, 1);
          lcd.print("No Datafile");

        }       
      }  
      // if the file isn't open, pop up an error:
      else {
        lcd.setCursor(0, 1);
        lcd.print("No Rec");
      }
    }
    else {
      digitalWrite(5, LOW);
      lcd.setCursor(0, 1);
      lcd.print("GPS LOST");
    }
    
      
  }
}


