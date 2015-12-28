#include <VirtualWire.h>
#include "DHT.h"

#define DHTPIN2 2     
#define DHTPIN3 3 
#define DHTPIN4 4 
#define DHTPIN5 5 
#define DHTPIN6 6     

#define DHTTYPE DHT22 

const int led_pin = 13;
const int transmit_pin = 8;

DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);
DHT dht4(DHTPIN4, DHTTYPE);
DHT dht5(DHTPIN5, DHTTYPE);
DHT dht6(DHTPIN6, DHTTYPE);

void setup()
{
  vw_set_tx_pin(transmit_pin);;
  vw_set_ptt_inverted(true); 
  vw_setup(2000); 
  dht2.begin();
  dht3.begin();
  dht4.begin();
  dht5.begin();  
  dht6.begin();
}

void loop()
{
  float t2 = dht2.readTemperature();
  float h2 = dht2.readHumidity();
  
  float t3 = dht3.readTemperature();
  float h3 = dht3.readHumidity();

  float t4 = dht4.readTemperature();
  float h4 = dht4.readHumidity();

  float t5 = dht5.readTemperature();
  float h5 = dht5.readHumidity();

  float t6 = dht6.readTemperature();
  float h6 = dht6.readHumidity();    

  log("D2", t2, h2);
  log("D3", t3, h3);
  log("D4", t4, h4);
  log("D5", t5, h5);  
  log("D6", t6, h6);  
  
  String msg2 = prepareMessage(2, t2, h2);
  String msg3 = prepareMessage(3, t3, h3);
  String msg4 = prepareMessage(4, t4, h4);
  String msg5 = prepareMessage(5, t5, h5);
  String msg6 = prepareMessage(6, t6, h6);
  
  sendMessage(msg2);
  delay(100);
  sendMessage(msg3);
  delay(100);
  sendMessage(msg4);
  delay(100);
  sendMessage(msg5);
  delay(100);
  sendMessage(msg6);

  delay(1600);
}

String prepareMessage(int sensorNo, float t, float h) {
  String msg = String(sensorNo) + " T:" + String(t,2) + " H:" + String(h,0);
  return msg;
}

void sendMessage(String msg) {
  digitalWrite(led_pin, HIGH); 
  char valueAsChar[16];
  msg.toCharArray(valueAsChar, msg.length()+1);
  vw_send((uint8_t *)valueAsChar, msg.length()+1);
  vw_wait_tx(); 
  digitalWrite(led_pin, LOW);  
}

void log(String sensor, float t, float h) {
  Serial.print(sensor + " Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print(sensor + " Temperature: "); 
  Serial.print(t);
  Serial.println(" *C ");
}


