/* The following code is used to check that all installed modules respond,
 * and to set the modules (ex. RTC DS1302).
 * 
 * modified 17 May 2020 by Giampiero Leserri
 */

// ADD the following Arduino libraries:
#include <SPI.h>            // standard Arduino library
#include <SD.h>             // standard Arduino library
#include <Wire.h>           // standard Arduino library
#include <Adafruit_SGP30.h> // SGP30: Adafruit SGP30 Sensor by Adafruit V1.2.0
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
Sd2Card card;        // standard declaration for microSD operation
SdVolume volume;     // standard declaration for microSD operation
SdFile root;         // standard declaration for microSD operation

// set up variables using the DS1302 RTC:
ThreeWire myWire(15, 16, 14); // DAT->pin A1, CLK->pin A2, RS->pin A0
RtcDS1302<ThreeWire> Rtc(myWire);
#define countof(a) (sizeof(a) / sizeof(a[0]))

// set up variables using the SGP30:
Adafruit_SGP30 sgp;
uint16_t TVOC_base, eCO2_base;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600); // 9600 bps serial port setting
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // notice in the serial monitor that the code is about to start
  Serial.println(F("Start initialization of installed components\n...............................................................\n"));

  delay(500);
  dhtMain(); // Start the function that contains what DHT should do
  delay(500);
  sdMain(); // Start the function that contains what the microSD must do
  delay(500);
  rtc1302Set(); // Start the function that contains what the real-time clock should do
  delay(500);
  sgp30Set(); // Start the function that checks that the SGP30 sensor works and is well connected

  // warn in the serial monitor that the code has ended
  Serial.println(F("\n\nSetup Complete!!!\n..............................................................."));
}

void loop()
{
  // no use of the loop since they only need to be set once
}

// Function that realizes the presence and activity of DHT
void dhtMain()
{
  Serial.println(F("DHT22 initializing...")); // warns that DHT will be tested
  dht.begin();                                // standard declaration

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // scrive sul serial monitor prima l'umidita' e dopo la temperatura
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C"));
}

// Function that realizes the presence and activity of the SD
void sdMain()
{
  Serial.print(F("\n\nInitializing SD card..."));

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect))
  {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
    while (1)
      ;
  }
  else
  {
    Serial.println(F("Wiring is correct and a card is present."));
  }

  // print the type of card
  Serial.println();
  Serial.print(F("Card type:         "));
  switch (card.type())
  {
  case SD_CARD_TYPE_SD1:
    Serial.println(F("SD1"));
    break;
  case SD_CARD_TYPE_SD2:
    Serial.println(F("SD2"));
    break;
  case SD_CARD_TYPE_SDHC:
    Serial.println(F("SDHC"));
    break;
  default:
    Serial.println(F("Unknown"));
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card))
  {
    Serial.println(F("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card"));
    while (1)
      ;
  }

  Serial.print(F("Clusters:          "));
  Serial.println(volume.clusterCount());
  Serial.print(F("Blocks x Cluster:  "));
  Serial.println(volume.blocksPerCluster());

  Serial.print(F("Total Blocks:      "));
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print(F("Volume type is:    FAT"));
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster(); // clusters are collections of blocks
  volumesize *= volume.clusterCount();    // we'll have a lot of clusters
  volumesize /= 2;                        // SD card blocks are always 512 bytes (2 blocks are 1KB)
  Serial.print(F("Volume size (Kb):  "));
  Serial.println(volumesize);
  Serial.print(F("Volume size (Mb):  "));
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print(F("Volume size (Gb):  "));
  Serial.println((float)volumesize / 1024.0);

  Serial.println(F("\nFiles found on the card (name, date and size in bytes): "));
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}

// Function that realizes the presence and activity of the RTC
void rtc1302Set()
{
  Rtc.Begin();
  Serial.println(F("\n\nDS1302 initializing..."));
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!Rtc.IsDateTimeValid())
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println(F("RTC lost confidence in the DateTime!"));
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println(F("RTC was write protected, enabling writing now"));
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println(F("RTC was not actively running, starting now"));
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println(F("RTC is older than compile time!  (Updating DateTime)"));
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println(F("RTC is newer than compile time. (this is expected)"));
  }
  else if (now == compiled)
  {
    Serial.println(F("RTC is the same as compile time! (not expected but all is fine)"));
  }

  //
  now = Rtc.GetDateTime();

  printDateTime(now);
  Serial.println();

  if (!now.IsValid())
  {
    // Common Causes:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println(F("RTC lost confidence in the DateTime!"));
  }

  delay(10000); // ten seconds
}

void printDateTime(const RtcDateTime &dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(datestring);
}

void sgp30Set()
{
  Serial.println(F("\n\nSGP30 initializing..."));
  if (!sgp.begin())
  {
    Serial.println(F("SGP30 Sensor not found :("));
    while (1)
      ;
  }

  Serial.print(F("Found SGP30 serial #"));
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  //
  if (!sgp.IAQmeasure())
  {
    Serial.println(F("Measurement failed"));
    return;
  }
  Serial.print(F("TVOC "));
  Serial.print(sgp.TVOC);
  Serial.print(F(" ppb\t"));
  Serial.print(F("eCO2 "));
  Serial.print(sgp.eCO2);
  Serial.println(F(" ppm"));

  if (!sgp.IAQmeasureRaw())
  {
    Serial.println(F("Raw Measurement failed"));
    return;
  }
  Serial.print(F("Raw H2 "));
  Serial.print(sgp.rawH2);
  Serial.print(" \t");
  Serial.print(F("Raw Ethanol "));
  Serial.print(sgp.rawEthanol);
  Serial.println("");

  delay(1000);

  uint16_t TVOC_base, eCO2_base;
  if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
  {
    Serial.println(F("Failed to get baseline readings"));
    return;
  }
  Serial.print(F("****Baseline values: eCO2: 0x"));
  Serial.print(eCO2_base, HEX);
  Serial.print(F(" & TVOC: 0x"));
  Serial.println(TVOC_base, HEX);
}