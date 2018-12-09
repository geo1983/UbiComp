#include <SimpleDHT.h>
#include <Wire.h>
#include "ThingSpeak.h"
#include "secrets.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include <SD.h>
#include <WiFiSSLClient.h>
#include <TembooSSL.h>
//#include "TembooAccount.h" // Contains Temboo account information

// set up variables using the SD utility library functions:
SdVolume volume;
SdFile root;

File myFile;
File dataFile;
WiFiClient client2;  
WiFiSSLClient client;
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password

// Add yourThingSpeak channel information here
unsigned long myChannelNumber = 601298;
const char * myWriteAPIKey = "36MBFDA6FMSN6P5B";


// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);

int val = 0; //value for storing moisture value 
int soilPin = A0;//Declare a variable for the soil moisture sensor 
int soilPower = 5;//Variable for Soil moisture Power
int number1 = 0; // reading number which adds up by 1 at every reading
int led = 6; // led pin no

//Rather than powering the sensor through the 3.3V or 5V pins, 
//we'll use a digital pin to power the sensor. This will 
//prevent corrosion of the sensor as it sits in the soil. 

void setup() 
{
  Serial.begin(9600);   // open serial over USB
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  
  ThingSpeak.begin(client2);  // Initialize ThingSpeak
  
  pinMode(soilPower, OUTPUT);//Set D7 as an OUTPUT
  digitalWrite(soilPower, LOW);//Set to LOW so no power is flowing through the sensor
 
 pinMode(led, OUTPUT); // set led pin as output
}

void loop() 
{
int wifiStatus = WL_IDLE_STATUS;// wifissl for temboo
  // Try to connect to the local WiFi network temboo
  while(wifiStatus != WL_CONNECTED) {
    Serial.print("WiFi:");
    wifiStatus = WiFi.begin(ssid, pass);

    if (wifiStatus == WL_CONNECTED) {
      Serial.println("OK");
    } else {
      Serial.println("FAIL");
    }
  }  
//delay(10000);   // Waits 10 seconds to confirm connection 

  

// start working...
Serial.println("=================================");
Serial.print("Soil Moisture = ");    
//get soil moisture value from the function below and print it
Serial.println(readSoil());
if (readSoil()<420){
Serial.println("Dry soil");
digitalWrite(led, HIGH); // turns led on
}
else {
Serial.println("Wet soil");
digitalWrite(led, LOW); // turns led off
}

// read without samples.
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
    return;
 } 
  
  Serial.println("Temperature and Humidity: ");
  Serial.println((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.print(" H");
  Serial.println();
  
// set the fields with the values
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, readSoil());
  


// write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  // DHT11 sampling rate is 1HZ.

// Write to SD card!!! open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("values.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println("Writing to values.txt...");
    myFile.println("================================");
    myFile.print("Reading no: "); myFile.print(number1, DEC);
    myFile.print(". Temperature = "); myFile.print(temperature, DEC); 
    myFile.print("C . Humidity = "); myFile.print(humidity, DEC);
    myFile.print("%. Soil Moisture = "); myFile.print(readSoil(),DEC);
    myFile.println(".");
    
    // close the file:
    myFile.close();
    Serial.println("done file1.");
  } else {
        // if the file didn't open, print an error:
        Serial.println("error opening file1.txt");
      }
//set temboo
TembooChoreoSSL GetWeatherByAddressChoreo(client);

    // Invoke the Temboo client
    GetWeatherByAddressChoreo.begin();

    // Set Temboo account credentials
    GetWeatherByAddressChoreo.setAccountName(TEMBOO_ACCOUNT);
    GetWeatherByAddressChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    GetWeatherByAddressChoreo.setAppKey(TEMBOO_APP_KEY);
    GetWeatherByAddressChoreo.setDeviceType(TEMBOO_DEVICE_TYPE);

    // Set Choreo inputs
    String AddressValue = "277 Calder Road";
    GetWeatherByAddressChoreo.addInput("Address", AddressValue);
    String DayValue = "1";
    GetWeatherByAddressChoreo.addInput("Day", DayValue);
    String UnitsValue = "c";
    GetWeatherByAddressChoreo.addInput("Units", UnitsValue);

    // Identify the Choreo to run
    GetWeatherByAddressChoreo.setChoreo("/Library/Yahoo/Weather/GetWeatherByAddress");

    // add an Output Filter to extract the date and time of the last report.
    GetWeatherByAddressChoreo.addOutputFilter("\tDate ", "//yweather:condition/@date", "Response");
    // add an Output Filter to extract the current temperature
    GetWeatherByAddressChoreo.addOutputFilter("\tTemperature ", "//yweather:condition/@temp", "Response");
    GetWeatherByAddressChoreo.addOutputFilter("\tHumidity ", "//yweather:atmosphere/@humidity", "Response");
    GetWeatherByAddressChoreo.addOutputFilter("\tSunrise ", "//yweather:astronomy/@sunrise", "Response");
    GetWeatherByAddressChoreo.addOutputFilter("\tSunset ", "//yweather:astronomy/@sunset", "Response");
    GetWeatherByAddressChoreo.addOutputFilter("\tCurr condition ", "//yweather:condition/@text", "Response");

   
    // Run the Choreo; when results are available, print them to serial
    GetWeatherByAddressChoreo.run();
    
    if(GetWeatherByAddressChoreo.available()) {
      Serial.print("API worked.");
      //open second file
      dataFile = SD.open("yahoo.txt", FILE_WRITE);
      dataFile.println();
      dataFile.print("=============================");
      dataFile.println();
      dataFile.print("Reading no: "); dataFile.print(number1);
      dataFile.println();
      
      // if the file opened okay, write to it:
      if (dataFile) {
        Serial.println("Writing to yahoo.txt...");
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening file2.txt");
      }
      
      while(GetWeatherByAddressChoreo.available()) {
        char c = GetWeatherByAddressChoreo.read();
        dataFile.print(c); 
      }
        // close the file:
        dataFile.close();
        Serial.println("done file2."); 
      
    }
    GetWeatherByAddressChoreo.close();



// Disconnects from the WiFi
WiFi.disconnect();
//readings delay
//delay(10000); //10 sec 
delay(3600000);//take a reading every hour
number1++;
}

//This is a function used to get the soil moisture content
int readSoil()
{

    digitalWrite(soilPower, HIGH);//turn D7 "On"
    delay(10);//wait 10 milliseconds 
    val = analogRead(soilPin);//Read the SIG value form sensor 
    digitalWrite(soilPower, LOW);//turn D7 "Off"
    return val;//send current moisture value
}

