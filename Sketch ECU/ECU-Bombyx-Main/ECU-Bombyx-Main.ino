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
static String dataString = "000000000000000000000000000000"; // make a string for assembling the data to log
static String dFile = "000000000000";
static int timer = 1441;
static int timer2 = 0;
long t1 =0;
long pouse = 15*1000;

// set up variables using the DS3231 RTC:
RTC_DS3231 rtc; // declaration of the "rtc" object to the class RTC_DS3231

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600); // 9600 bps serial port setting
  delay(2000);
  
  rtcSet();
  sdSet();
  DHTSet();
  timerSet();
  timer2 = timer;
  t1=millis();
}

void loop() {
  long diff =millis() - t1;
  if(diff > pouse){
  timerSet();
  pouse += millis();
  }
  if(timer>=timer2 + 15){
    timer2 = timer;
    rtcMain();
    DHTMain();
    sdMain();
  }
}

//Function setup()
void rtcSet(){
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
  }
  
  DateTime now = rtc.now();
    char buf[] = "YY-MM-DD";
    dFile =(now.toString(buf));
    dFile +=(".csv");
}

void sdSet(){
  Serial.print(F("Initializing SD card..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    while (1);
  }
  Serial.println(F("card initialized."));
  if(! SD.exists(dFile)){
    File dataFile = SD.open(dFile, FILE_WRITE);
    dataFile.println(F("Data & Time;C-Temp;%-Humid"));
    dataFile.close();
    Serial.print(dFile);
    Serial.println(F(" initialized"));
  }
}
void DHTSet(){
  dht.begin();
  Serial.println(F("DHT 22 initialized"));
}

void timerSet(){
  DateTime now = rtc.now();
    char buf1[] = "hh";
    char buf2[] = "mm";
    String s1 =(now.toString(buf1));
    String s2 =(now.toString(buf2));
    int hh= s1.toInt();
    int mm= s2.toInt();
    timer = (hh*60) + mm;
    Serial.print(F("ore in minuti: "));
    Serial.println(hh*60);
    Serial.print(F("minuti: "));
    Serial.println(mm);
    Serial.print(F("timer: "));
    Serial.print(timer);
    Serial.println(F(" update!"));
}

//Function loop()
void rtcMain(){
  DateTime now = rtc.now();
    Serial.println();
    char buf2[] = "YY/MM/DD-hh:mm";
    dataString =(now.toString(buf2));
    dataString +=(";");
}

void DHTMain(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  dataString +=(t);
  dataString +=(";");
  dataString +=(h);
  dataString +=(";");
}

void sdMain(){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(dFile, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println(F("error opening datalog file"));
  }
  Serial.print(dFile);
  Serial.println(F(" update"));
}
