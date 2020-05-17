/* The following code is used to set the starting baseline for the SGP30 module.
 * The calibration process lasts 12 hours, every 30 minutes from a partial response.
 * At the end he writes in the SD the two found values of the eCO2 and of the TVOC.
 * 
 * modified 17 May 2020 by Giampiero Leserri
 */

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <DHT.h>

// set up variables using the DHT 22:
#define DHTPIN 2          //Pin select DHT
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE); // Declare the DHT data pin and model to the DHT library

// set up variables using the SGP30:
Adafruit_SGP30 sgp;
uint16_t TVOC_base, eCO2_base;
/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
  return absoluteHumidityScaled;
}

// set up variables using the SD utility library functions:
#define chipSelect 4 // declare the pin that is connected to the chip select
// the myFile object is created for the SD
File myFilex;
File myFiley;

//
const unsigned long timeL = 44100000;  //12 hour
static unsigned long timeS1 = 60000;   // 1 minute
static unsigned long timeS2 = 1800000; //30 minute

//const unsigned long timeL = 600000; //10 minute
//static unsigned long timeS1 = 60000; // 1 minute
//static unsigned long timeS2 = 300000; //5 minute

void setup()
{
  Serial.begin(9600);

  Serial.print(F("Initializing SD card..."));

  if (!SD.begin(4))
  {
    Serial.println(F("initialization failed!"));
    while (1)
      ;
  }

  dht.begin();

  Serial.println(F("initialization done."));

  Serial.println(F("SGP30 test"));

  if (!sgp.begin())
  {
    Serial.println(F("Sensor not found :("));
    while (1)
      ;
  }
  Serial.print(F("Found SGP30 serial #"));
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  Serial.println(F("Calibration start... (duration 12h)\n******************************"));

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
  myFilex = SD.open("Cali.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (myFilex)
  {
    myFilex.println(F("eCO2;TVOC"));
    myFilex.close();
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println(F("error opening Cali.csv"));
  }
}
// creo una varibile per far andare l'if in basso alla riga 138
int i = 0;

void loop()
{
  // make a string for assembling the data to log:
  String dataString = "";

  if (millis() < timeL)
  {
    // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
    float temperature = dht.readTemperature(); // [°C]
    float humidity = dht.readHumidity();       // [%RH]
    sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

    if (!sgp.IAQmeasure())
    {
      Serial.println(F("Measurement failed"));
    }
    //Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
    //Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");

    if (!sgp.IAQmeasureRaw())
    {
      Serial.println(F("Raw Measurement failed"));
    }
    //Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
    //Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");

    delay(2000);

    if (millis() > timeS1)
    {
      Serial.print(F("."));
      timeS1 += 60000;
    }

    if (millis() >= timeS2)
    {
      timeS2 += 1800000; //30 minute
      //timeS2 += 300000; // 5 minute

      if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
      {
        Serial.println(F("Failed to get baseline readings"));
      }
      Serial.println(F("\n30 minutes have passed!"));
      dataString = (F("0x"));
      dataString += String(eCO2_base, HEX);
      dataString += (F(";"));
      dataString += (F("0x"));
      dataString += String(TVOC_base, HEX);

      // open the file. note that only one file can be open at a time,
      // so you have to close this one before opening another.
      myFilex = SD.open("Cali.csv", FILE_WRITE);

      // if the file is available, write to it:
      if (myFilex)
      {
        myFilex.println(dataString);
        myFilex.close();
        // print to the serial port too:
        Serial.println(dataString);
      }
      else
      {
        // if the file didn't open, print an error:
        Serial.println(F("error opening BLP1"));
      }
    }
  }
  else
  {
    if (i < 1)
    {

      if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
      {
        Serial.println(F("Failed to get baseline readings"));
      }
      Serial.println(F("\n\n\nProcess terminated!!!"));
      Serial.print(F("****Baseline values: eCO2: 0x"));
      Serial.print(eCO2_base, HEX);
      Serial.print(F(" & TVOC: 0x"));
      Serial.println(TVOC_base, HEX);

      // delete the file:
      Serial.println(F("Removing old ECO2 and TVOC..."));
      SD.remove("ECO2.TXT");
      SD.remove("TVOC.TXT");

      dataString = (F("0x"));
      dataString += String(eCO2_base, HEX);
      // open the file. note that only one file can be open at a time,
      // so you have to close this one before opening another.
      myFilex = SD.open("ECO2.TXT", FILE_WRITE);

      // if the file is available, write to it:
      if (myFilex)
      {
        myFilex.println(dataString);
        myFilex.close();
        // print to the serial port too:
        Serial.println(dataString);
      }
      else
      {
        // if the file didn't open, print an error:
        Serial.println(F("error opening eCO2"));
      }

      dataString = (F("0x"));
      dataString += String(TVOC_base, HEX);
      // open the file. note that only one file can be open at a time,
      // so you have to close this one before opening another.
      myFiley = SD.open("TVOC.TXT", FILE_WRITE);

      // if the file is available, write to it:
      if (myFiley)
      {
        myFiley.println(dataString);
        myFiley.close();
        // print to the serial port too:
        Serial.println(dataString);
        Serial.println();
      }
      else
      {
        // if the file didn't open, print an error:
        Serial.println(F("error opening TVOC"));
      }
      i = 2;
    }
  }
}
/* Sketch uses 20316 bytes (62%) of program storage space. Maximum is 32256 bytes.
 * Global variables use 1171 bytes (57%) of dynamic memory,
 * leaving 877 bytes for local variables. Maximum is 2048 bytes.
 */