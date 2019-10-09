/***************************************************
  Adafruit MQTT Library Ethernet Example

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Alec Moore
  Derived from the code written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>

/************************* Ethernet Client Setup *****************************/
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

//Uncomment the following, and set to a valid ip if you don't have dhcp available.
//IPAddress iotIP (192, 168, 0, 42);
//Uncomment the following, and set to your preference if you don't have automatic dns.
//IPAddress dnsIP (8, 8, 8, 8);
//If you uncommented either of the above lines, make sure to change "Ethernet.begin(mac)" to "Ethernet.begin(mac, iotIP)" or "Ethernet.begin(mac, iotIP, dnsIP)"


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "cbates123"
#define AIO_KEY         "a406a995ee014b60b335592f8b3565f0"


/************ Global State (you don't need to change this!) ******************/

//Set up the ethernet client
EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }


/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish pump_status = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/pump_status");
Adafruit_MQTT_Publish fan_low_status = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/fan_low_status");
Adafruit_MQTT_Publish fan_high_status = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/fan_high_status");

// Setup a feed called 'onoff' for subscribing to changes.
//Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");
Adafruit_MQTT_Subscribe pump = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Pump");
Adafruit_MQTT_Subscribe fan_low = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Fan Low");
Adafruit_MQTT_Subscribe fan_high = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Fan High");

/**pins**/
#define Pump_pin 4  
#define Fan_low_pin 5
#define Fan_high_pin 6
#define Fan_high2_pin 7

/*************************** Sketch Code ************************************/

void setup() {
  Serial.begin(115200);

  Serial.println(F("Adafruit MQTT demo"));

  // Initialise the Client
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(mac);
  delay(3000); //give the ethernet a second to initialize
  
  mqtt.subscribe(&pump);
  mqtt.subscribe(&fan_low);
  mqtt.subscribe(&fan_high);
  
  pinMode(Pump_pin, OUTPUT);
  digitalWrite(Pump_pin, HIGH);
  pinMode(Fan_low_pin, OUTPUT);
  digitalWrite(Fan_low_pin, HIGH);
  pinMode(Fan_high_pin, OUTPUT);
  digitalWrite(Fan_high_pin, HIGH);
  pinMode(Fan_high2_pin, OUTPUT);
  digitalWrite(Fan_high2_pin, HIGH);

}

uint32_t pump_var, fan_low_var, fan_high_var;
uint32_t pump_var_hold, fan_low_var_hold, fan_high_var_hold;

void loop() {

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(3000))) {
    if (subscription == &pump) {
      Serial.print(F("Got: "));
      Serial.println((char *)pump.lastread);
      if (strcmp((char *)pump.lastread, "ON") == 0) {
        digitalWrite(Pump_pin, LOW);
        pump_var=1;
        Serial.println("Pump ON");
      }
      if (strcmp((char *)pump.lastread, "OFF") == 0) {
        digitalWrite(Pump_pin, HIGH);
        pump_var=0;
        Serial.println("Pump OFF");
      }
    }
    if (subscription == &fan_low) {
      Serial.print(F("Got: "));
      Serial.println((char *)fan_low.lastread);
      if (strcmp((char *)fan_low.lastread, "ON") == 0) {
        digitalWrite(Fan_low_pin, LOW);
        fan_low_var=1;
        Serial.println("Fan Low ON");
      }
      if (strcmp((char *)fan_low.lastread, "OFF") == 0) {
        digitalWrite(Fan_low_pin, HIGH);
        fan_low_var=0;
        Serial.println("Fan Low OFF");
      }
    }    
    if (subscription == &fan_high) {
      Serial.print(F("Got: "));
      Serial.println((char *)fan_high.lastread);
      if (strcmp((char *)fan_high.lastread, "ON") == 0) {
        digitalWrite(Fan_high_pin, LOW);
        digitalWrite(Fan_high2_pin, LOW);
        fan_high_var=1;
        Serial.println("Fan High ON");
      }
      if (strcmp((char *)fan_high.lastread, "OFF") == 0) {
        digitalWrite(Fan_high_pin, HIGH);
        digitalWrite(Fan_high2_pin, HIGH);
        fan_high_var=0;
        Serial.println("Fan High OFF");
      }
    }
  }

  // Now we can publish stuff!

  if (pump_var!=pump_var_hold) {
    pump_status.publish(pump_var);
    pump_var_hold=pump_var;
  }
  if (fan_low_var!=fan_low_var_hold) {
    fan_low_status.publish(fan_low_var);
    fan_low_var_hold=fan_low_var;
  }
  if (fan_high_var!=fan_high_var_hold) {
    fan_high_status.publish(fan_high_var);
    fan_high_var_hold=fan_high_var;
  }  
  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

// this function is called whenever an 'digital' feed message
// is received from Adafruit IO. it was attached to
// the 'digital' feed in the setup() function above.

//void fan_low_ctrl(AdafruitIO_Data *data) {
// 
//  Serial.print("received <- ");
// 
//  if(data->toPinLevel() == HIGH)
//    Serial.println("Fan Low on");
//  else
//    Serial.println("Fan Low OFF");
// 
//  // write the current state to the led
//  digitalWrite(fan_low, data->toPinLevel());
// 
//}
//void fan_high_ctrl(AdafruitIO_Data *data) {
// 
//  Serial.print("received <- ");
// 
//  if(data->toPinLevel() == HIGH)
//    Serial.println("Fan High on");
//  else
//    Serial.println("Fan High OFF");
// 
//  // write the current state to the led
//  digitalWrite(fan_high, data->toPinLevel());
//  digitalWrite(fan_high2, data->toPinLevel());
//}
