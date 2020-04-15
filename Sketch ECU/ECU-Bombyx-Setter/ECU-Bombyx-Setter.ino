/* The following code is used to set the different components 
 * and to check that everyone answers the call, leaving the 
 * answer on the serial monitor. 
 */

// ADD the following Arduino libraries:
#include <SD.h>
#include <SPI.h>
#include <RTClib.h> // RTC:  RTClib by adafruit V.1.4.1
#include <DHT.h> /*DHT: DHT sensor librery by Adafruit V.1.3.8
                  *     Adafruit Unified Sensor by Adafruit V.1.1.2                        
                  *     Adafruit ADXL343 by Adafruit V.1.2.0 
                  */
                  
// set up variables using the DHT 22:
#define DHTPIN 2 //Pin select DHT 
#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE); // Declare the DHT data pin and model to the DHT library

// set up variables using the SD utility library functions:
#define chipSelect 8 // declare the pin that is connected to the chip select
Sd2Card card; // standard declaration for microSD operation
SdVolume volume; // standard declaration for microSD operation
SdFile root; // standard declaration for microSD operation

// set up variables using the DS3231 RTC:
RTC_DS3231 rtc; // declaration of the "rtc" object to the class RTC_DS3231

const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}; // array declaration for the days of the week

void setup() {
  Serial.begin(9600); // 9600 bps serial port setting
  delay(2000);
  dhtMain(); // Start the function that contains what DHT should do
  delay(2000);
  sdMain(); // Start the function that contains what the microSD must do
  delay(2000); // Start the function that contains what the real-time clock should do
  rtc3231();
  Serial.println(F("Setup Complete")); // warn in the serial monitor that the code has ended
}

void loop() {

}

void dhtMain(){
  Serial.println(F("DHTxx test!")); // warns that DHT will be tested
  dht.begin(); // standard declaration

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C \n"));  
}

void sdMain(){
  Serial.print(F("\nInitializing SD card..."));

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.println();
  Serial.print(F("Card type:         "));
  switch (card.type()) {
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
  if (!volume.init(card)) {
    Serial.println(F("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card"));
    while (1);
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

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
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

void rtc3231(){
  Serial.println();
  if (! rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println(F("RTC lost power, lets set the time!"));
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    Serial.println();
  }
  
  DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(F(" ("));
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(F(") "));
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(F(" since midnight 1/1/1970 = "));
    Serial.print(now.unixtime());
    Serial.print(F("s = "));
    Serial.print(now.unixtime() / 86400L);
    Serial.println(F("d"));

    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));

    Serial.print(F(" now + 7d + 30s: "));
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();

    Serial.print(F("Temperature: "));
    Serial.print(rtc.getTemperature());
    Serial.println(F(" C"));

    Serial.println();
}
