/* The following code is used to set the various components and to verify that 
 * everyone answers the call. It is written on the right serial monitor. 
 * The data are conveyed into a single variable then written into the SD.
 * 
 * modified 17 May 2020 by Giampiero Leserri
 */

// ADD the following Arduino libraries:
#include <SPI.h>            // standard Arduino library
#include <SD.h>             // standard Arduino library
#include <Wire.h>           // standard Arduino library
#include <Adafruit_SGP30.h> //Adafruit SGP30 Sensor by Adafruit V1.2.0
#include <ThreeWire.h>      // RTC: RTC by Makuna V.2.3.4
#include <RtcDS1302.h>      // RTC: RTC by Makuna V.2.3.4
#include <DHT.h>            /* DHT: DHT sensor librery by Adafruit V.1.3.9
                            *       Adafruit Unified Sensor by Adafruit V.1.1.2                        
                            *       Adafruit ADXL343 by Adafruit V.1.2.0 
                            */

// set up variables using the DHT 22:
#define DHTPIN 2          //Pin select DHT
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE); // Declare the DHT data pin and model to the DHT library

// set up variables using the SD utility library functions:
#define chipSelect 4 // declare the pin that is connected to the chip select
File myFile;         // the myFile object is created for the SD

// set up variables using the DS1302 RTC:
ThreeWire myWire(15, 16, 14); // DAT->pin A1, CLK->pin A2, RS->pin A0
RtcDS1302<ThreeWire> Rtc(myWire);
#define countof(a) (sizeof(a) / sizeof(a[0]))

// set up variables using the sgp30:
Adafruit_SGP30 sgp;
uint16_t TVOC_base, eCO2_base;
float temperature = 0; // [°C]
float humidity = 0;    // [%RH]
uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
  return absoluteHumidityScaled;
}

// declaration of the Global variables make a string
//  for assembling the data to log
String dataOnTtheRow = "";

// Enter the initial company name without spaces followed by a progressive number
// for each ECU installed, also entering ".csv" (maximum 6.3 characters)
// Ex. Seta Etica ECU 1 => "SeEt01.csv"
const String company = "xxxxxx.csv"!;  // this error allows you not to forget to set the company name

// variable to which the 15 minutes to take the readings are added
uint32_t pouse = 0;
// Variable content in the if on line xxx,
// this causes every 4 readings to set the new baseline
int8_t c = 0;

//*********************************************************************************
void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600); // 9600 bps serial port setting
  delay(500);

  rtcSet();
  sdSet();
  DHTSet();
  sgp30Set();
}

void loop()
{
  // If 15 minutes of use have passed a reading
  if (millis() > pouse)
  {
    // Further increase of 15 the pauseMain value, causes overflow
    pouse += (15UL * 60UL * 1000UL);
    heatingSgp30();

    delay(2000); // Safety delay in case of DHT reset
    rtcMain();
    DHTMain();
    sgp30Main();
    sdMain();
    c += 1;
  }
}
//---------------------------------------------------------------------------------

// Group of functions for setting the modules
//*********************************************************************************

// The function takes care of starting and if necessary
// setting the date and time of the DS1302
void rtcSet()
{
  Rtc.Begin();
  Serial.println(F("RTC initialized!"));

  if (Rtc.GetIsWriteProtected())
  {
    //      Serial.println(F("RTC was write protected, enabling writing now"));
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    //      Serial.println(F("RTC was not actively running, starting now"));
    Rtc.SetIsRunning(true);
  }
}

// The function takes care of starting the SD module and, if necessary,
// creating a "CSV" file with the current date
void sdSet()
{
  Serial.print(F("Initializing SD card..."));

  // See if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    Serial.println(F("Card failed, or not present"));
  }
  Serial.println(F("card initialized."));

  // Check if there is a file with the name given by the variable "company",
  // otherwise it creates and initializes it
  if (!SD.exists(company))
  {
    myFile = SD.open(company, FILE_WRITE);
    myFile.println(F("Data & Time;C-Temp;%-Humid;ppm-eCO2;ppb-TVOC;"));
    myFile.close();
    Serial.print(company);
    Serial.println(F(" file created"));
  }
}

// initialazied DHT22
void DHTSet()
{
  dht.begin();
  Serial.println(F("DHT22 initialized!"));
}

void sgp30Set()
{
  if (!sgp.begin())
  {
    Serial.println(F("Sensor not found :("));
  }
  else
  {
    Serial.println(F("SGP30 initialized!"));
  }
  Serial.print(F("Found SGP30 serial #"));
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // open the ECO2.TXT for reading:
  myFile = SD.open("eCO2.txt");
  if (myFile)
  {
    Serial.println(F("\neCO2.txt read... "));

    // read from the file until there's nothing else in it:
    while (myFile.available())
    {
      dataOnTtheRow = myFile.readStringUntil('\n');
      dataOnTtheRow.replace("\n", "");
      // goes to convert the string hexadecimal into a integer
      eCO2_base = strtoul(dataOnTtheRow.c_str(), NULL, 16);
      Serial.println(F("Print eCO2_base"));
      Serial.println(eCO2_base, HEX);
    }
    // close the file:
    myFile.close();
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println(F("error opening ECO2.TXT"));
  }

  // open the TVOC.TXT for reading:
  myFile = SD.open("TVOC.txt");
  if (myFile)
  {
    Serial.println(F("TVOC.TXT read... "));

    // read from the file until there's nothing else in it:
    while (myFile.available())
    {
      dataOnTtheRow = myFile.readStringUntil('\n');
      dataOnTtheRow.replace("\n", "");
      // goes to convert the string hexadecimal into a integer
      TVOC_base = strtoul(dataOnTtheRow.c_str(), NULL, 16);
      Serial.println(TVOC_base, HEX);
    }

    // close the file:
    myFile.close();
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println(F("error opening TVOC.TXT"));
  }

  // it sets the baseline calculated in 12 hours
  if (!sgp.setIAQBaseline(eCO2_base, TVOC_base))
  {
    Serial.println(F("the baseline was not written!"));
  }
}
//---------------------------------------------------------------------------------

// Group of functions for utilized the modules
//*********************************************************************************

// The function takes care of heating the sgp30 module
void heatingSgp30()
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
  for (int i = 0; i <= 30; i++)
  {
    sgp.IAQmeasure();
    delay(1000);
  }
  // Notice of end of heating
  Serial.println(F("heating finished"));

  if (c == 4)
  {
    sgp.getIAQBaseline(&eCO2_base, &TVOC_base);
    sgp.setIAQBaseline(eCO2_base, TVOC_base);
    c = 0;
  }
}

// The function starts writing in the string that forms the first line
void rtcMain()
{
  // I create a DateTime object and call it now
  // and pass it the constructor rtc.now();
  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);

  if (!now.IsValid())
  {
    Serial.println();
    // Common Causes:
    // the battery on the device is low or even missing and
    // the power line was disconnected
    Serial.println(F("RTC lost confidence in the DateTime!"));
  }
}

// The function converts and writes the time to the dataOnTtheRow variable
void printDateTime(const RtcDateTime &dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%04u/%02u/%02u %02u:%02u"),
             dt.Year(),
             dt.Month(),
             dt.Day(),
             dt.Hour(),
             dt.Minute());
  dataOnTtheRow = String(datestring);
  dataOnTtheRow += (F(";"));
}

// Function for reading humidity and temperature
void DHTMain()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Writing of the values read in the two variables
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
  }

  // Passing data to the variable that will write to the SD
  dataOnTtheRow += (t);
  dataOnTtheRow += (F(";"));
  dataOnTtheRow += (h);
  dataOnTtheRow += (F(";"));
}

void sgp30Main()
{
  c = c + 1;
  delay(2000);
  // If you have a temperature / humidity sensor, you can set the absolute
  // humidity to enable the humditiy compensation for the air quality signals
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
  if (!sgp.IAQmeasure())
  {
    dataOnTtheRow += (F("sgp not read; sgp not read;"));
  }
  else
  {
    dataOnTtheRow += (sgp.eCO2);
    dataOnTtheRow += (F(";"));
    dataOnTtheRow += (sgp.TVOC);
    dataOnTtheRow += (F(";"));
  }
}

// Function that writes the data collected in the SD
void sdMain()
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(company, FILE_WRITE);

  // if the file is available, write to it:
  if (myFile)
  {
    myFile.println(dataOnTtheRow);
    myFile.close();
    Serial.println(F("String:"));
    Serial.println(dataOnTtheRow);
    Serial.print(company);
    Serial.println(F(" update!\n"));
  }
}
//---------------------------------------------------------------------------------
//Sketch uses 25922 bytes (80%) of program storage space. Maximum is 32256 bytes.
//Global variables use 1164 bytes (56%) of dynamic memory,
//leaving 884 bytes for local variables. Maximum is 2048 bytes.
