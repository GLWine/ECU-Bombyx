/* The following code is used to set the different components 
 * and to check that everyone answers the call, leaving the 
 * answer on the serial monitor. 
 */

// ADD the following Arduino libraries:
#include <SD.h>
#include <SPI.h>
#include <RTClib.h> // RTC:  RTClib by adafruit V.1.5.0
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

// set up variables using the DS3231 RTC:
RTC_DS3231 rtc; // declaration of the "rtc" object to the class RTC_DS3231

// declaration of the Global variables make a string for assembling the data to log
static String dataOnTtheRow = "000000000000000000000000000000";
// Enter the initial company name without spaces followed 
// by a progressive number for each ECU installed,
// also entering ".csv" (maximum 12 characters)
// Ex. Seta Etica ECU 1 => "SE01.csv"
static String company = ""; //fill in here
// variable to which the 15 minutes to take the readings are added
static unsigned long pouse = (15*60*1000);

//*********************************************************************************
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600); // 9600 bps serial port setting
  delay(2000);
  
  rtcSet();
  sdSet();
  DHTSet();
}

void loop() {
  // If 15 minutes of use have passed a reading
   if(millis()>pouse){
    pouse+=(15*60*1000); // Further increase of 15 the pause value
    delay(2000); // Safety delay in case of DHT reset
    rtcMain();
    DHTMain();
    sdMain();
  }  
}
//---------------------------------------------------------------------------------

// Group of functions for setting the modules
//*********************************************************************************

// The function takes care of starting and if necessary 
// setting the date and time of the DS3231
void rtcSet(){
  if (! rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }
}

// The function takes care of starting the SD module and, if necessary,
// creating a "CSV" file with the current date
void sdSet(){
  Serial.print(F("Initializing SD card..."));

  // See if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    while (1);
  }
  Serial.println(F("card initialized."));
  
// Check if there is a file with the name given by the variable "company",
// otherwise it creates and initializes it
  if(! SD.exists(company)){
    File dataFile = SD.open(company, FILE_WRITE);
    dataFile.println(F("Data & Time;C-Temp;%-Humid"));
    dataFile.close();
    Serial.print(company);
    Serial.println(F(" file created"));
  }
}

// initialazied DHT22
void DHTSet(){
dht.begin();
Serial.println(F("DHT22 initialized"));
}
//---------------------------------------------------------------------------------

// Group of functions for utilized the modules
//*********************************************************************************

// The function starts writing in the string that forms the first line
void rtcMain(){
  // I create a DateTime object and call it now 
  // and pass it the constructor rtc.now();
  DateTime now = rtc.now();
  // I create a char variable to which I pass the scheme 
  // with which to create the date and time
  char buf[] = "YY/MM/DD-hh:mm";
  // dnow.toString () function converts buf and
  // passes the date to the variable "dataOnTtheRow"
  dataOnTtheRow =(now.toString(buf));
  dataOnTtheRow +=(";");
}

// Function for reading humidity and temperature
void DHTMain(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  // Writing of the values read in the two variables
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Passing data to the variable that will write to the SD
  dataOnTtheRow +=(t);
  dataOnTtheRow +=(";");
  dataOnTtheRow +=(h);
  dataOnTtheRow +=(";");
}

// Function that writes the data collected in the SD
void sdMain(){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(company, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataOnTtheRow);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println(F("error opening datalog file"));
  }
  Serial.print(company);
  Serial.println(F(" update"));
}
//---------------------------------------------------------------------------------