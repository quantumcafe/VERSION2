/*
  Pachube sensor client
 
 This sketch connects an analog sensor to Pachube (http://www.pachube.com)
 using a Wiznet Ethernet shield. You can use the Arduino Ethernet shield, or
 the Adafruit Ethernet shield, either one will work, as long as it's got
 a Wiznet Ethernet module on board.
 
 This example has been updated to use version 2.0 of the Pachube.com API. 
 To make it work, create a feed with a datastream, and give it the ID
 sensor1. Or change the code below to match your feed.
 
 
 Circuit:
 * Analog 0 has LDR input. 5V -> 10kR -> (A0) -> LDR -> Ground
 * DHT22 attached on pin 3
 * Etherten: Equivalent to Ethernet shield attached to pins 10, 11, 12, 13
 
 created 15 March 2010
 updated 16 Mar 2012
 by Tom Igoe with input from Usman Haque and Joe Saavedra
 
http://arduino.cc/en/Tutorial/PachubeClient
 This code is in the public domain.
 esta es esparta
 esto ahora es mas que esparta
 esto es mas lineas
 y lineas
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <DHT22.h>
//#include <DHT.h>

#define APIKEY         "Q1NyHPLaMEinr1P3ZPNfZ5KFqcCSAKx0QTQ1UkZveTlFVT0g" // replace your pachube api key here
#define FEEDID         119298 // replace your feed ID
#define USERAGENT      "QUANTUMCAFE PROTOTYPE" // user agent is the project name
#define DHT22_PIN 3

// assign a MAC address for the ethernet controller.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
// fill in your address here:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// fill in an available IP address on your network here,
// for manual configuration:
IPAddress ip(192,168,1,2);
// initialize the library instance:
EthernetClient client;

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(216,52,233,122);      // numeric IP for api.pachube.com
//IPAddress server(216,52,233,122);      // numeric IP for api.pachube.com
//char server[] = "api.pachube.com";   // name address for pachube API

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 60000;   //milliseconds delay between updates to Pachube.com

// Setup a DHT22 instance
DHT22 myDHT22(DHT22_PIN);

void setup() {
  // start serial port:
  Serial.begin(115200);
  Serial.println("Cosm 3-sensor test");
 // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // DHCP failed, so use a fixed IP address:
    Ethernet.begin(mac, ip);
  }
}

void loop()
{
  // This is where HTML is returned from the server and errors are reported.
  
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    //Serial.println();
    //Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && ((millis() - lastConnectionTime) > postingInterval)) {
    // read sensors
    int light = analogRead(A0);   
    int humidity = 0; //myDHT22.getHumidityInt()/10;
    int temperature = 0; //myDHT22.getTemperatureCInt()/10;
    if (getEnviron(temperature, humidity)) sendData(light, humidity, temperature);
    else Serial.println("Digital sensor read failed");
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

boolean getEnviron(int& temp, int& rh)
{
  DHT22_ERROR_t errorCode;
  delay(2000);
  //Serial.print("Requesting data...");
  errorCode = myDHT22.readData();
  switch(errorCode)
  {
    case DHT_ERROR_NONE:
      Serial.println("   ***   getEnviron() got data ...");
      //Serial.print(myDHT22.getTemperatureC());
      //Serial.print("C ");
      //Serial.print(myDHT22.getHumidity());
      //Serial.println("%");
      // Alternately, with integer formatting which is clumsier but more compact to store and
   // can be compared reliably for equality:
   //   
      char buf[128];
      sprintf(buf, "Integer-only reading: Temperature %hi.%01hi C, Humidity %i.%01i %%RH",
                   myDHT22.getTemperatureCInt()/10, abs(myDHT22.getTemperatureCInt()%10),
                   myDHT22.getHumidityInt()/10, myDHT22.getHumidityInt()%10);
      //Serial.println(buf);
      rh = myDHT22.getHumidityInt();
      temp = myDHT22.getTemperatureCInt();
      return true;
    case DHT_ERROR_CHECKSUM:
      Serial.print("check sum error ");
      Serial.print(myDHT22.getTemperatureC());
      Serial.print("C ");
      Serial.print(myDHT22.getHumidity());
      Serial.println("% ");
      break;
    case DHT_BUS_HUNG:
      Serial.println("BUS Hung ");
      break;
    case DHT_ERROR_NOT_PRESENT:
      Serial.println("Not Present ");
      break;
    case DHT_ERROR_ACK_TOO_LONG:
      Serial.println("ACK time out ");
      break;
    case DHT_ERROR_SYNC_TIMEOUT:
      Serial.println("Sync Timeout ");
      break;
    case DHT_ERROR_DATA_TIMEOUT:
      Serial.println("Data Timeout ");
      break;
    case DHT_ERROR_TOOQUICK:
      Serial.println("Polled to quick ");
      break;
  }
  return false;
}

// this method makes a HTTP connection to the server:
void sendData(int Light, int Humidity, int Temperature)
{
//  Serial.println("*** ENVIRON");
//  Serial.println(Light);
//  Serial.println(Temperature);
//  Serial.println(Humidity);
  client.stop();
  delay(500);
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("   ***   sendData() connecting...");
    // send the HTTP PUT request:
    client.print("PUT /v2/feeds/");
    client.print(FEEDID);
    client.println(".csv HTTP/1.1");
    client.println("Host: api.pachube.com");
    client.print("X-PachubeApiKey: ");
    client.println(APIKEY);
    client.print("User-Agent: ");
    client.println(USERAGENT);
    client.print("Content-Length: ");

    char data[128];
    sprintf(data, "Light,%hi\nTemperature,%hi.%01hi\nHumidity,%i.%01i", Light, Temperature/10, Temperature%10, Humidity/10, Humidity%10);
    
    Serial.println("   ***   sendData() data...");
    Serial.println(data);
    
    int thisLength = -1;
    char* length = data;
    while(*(length++)) thisLength++;
    client.println(thisLength+1);
    
    // last pieces of the HTTP PUT request:
    client.println("Content-Type: text/csv");
    client.println("Connection: close");
    client.println();

    // here's the actual content of the PUT request:
    client.print(data);
  } 
  else
  {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
   // note the time that the connection was made or attempted:
  lastConnectionTime = millis();
}


// This method calculates the number of digits in the
// sensor reading.  Since each digit of the ASCII decimal
// representation is a byte, the number of digits equals
// the number of bytes:

int getLength(int someValue)
{
  // there's at least one byte:
  int digits = 1;
  // continually divide the value by ten, 
  // adding one to the digit count for each
  // time you divide, until you're at 0:
  int dividend = someValue /10;
  while (dividend > 0) {
    dividend = dividend /10;
    digits++;
  }
  // return the number of digits:
  return digits;
}
