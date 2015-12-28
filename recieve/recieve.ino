#include <VirtualWire.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>  
#include <SPI.h>
#include <Ethernet.h>
#include <ThingSpeak.h>
#include <stdlib.h>

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
//IPAddress ip(192, 168, 137, 177);
char thingSpeakAddress[] = "184.106.153.149";
String APIKey = "65OB4H8GUCSLZVQX";             // enter your channel's Write API Key
const int updateThingSpeakInterval = 20 * 1000; // 20 second interval at which to update ThingSpeak
EthernetServer server(80);
EthernetClient ethClient;
const int led_pin = 13;
const int receive_pin = 2;

long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;

float T[6];
float H[6];

int index = 0;
boolean updated = false;

EthernetClient client;

void setup()
{  
  //lcd.begin(16,2); 
  
 // lcd.backlight(); 
  //lcd.setCursor(0,0); 
  Serial.begin(115200); 

 // lcd.print("DHCP init");
  Serial.println("DHCP init");

  Ethernet.begin(mac);
  ThingSpeak.begin(ethClient);
  
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  //lcd.setCursor(0,0); 
  //lcd.print(Ethernet.localIP());

  pinMode(13,OUTPUT);
  vw_set_rx_pin(receive_pin);
  vw_setup(4000);  

  vw_rx_start();     

  for (int ix=0;ix<6;ix++) {
    T[ix] = 0;
    H[ix] = 0;
  }
}

void loop()
{
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    char* message;

    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
      digitalWrite(led_pin, HIGH); 
      message = (char*)buf; 
      //lcd.print(message);
      Serial.println(message);
            
      String str = String(message);
      String firstChar = String(str.charAt(0));
      index = firstChar.toInt()-1;
      //lcd.setCursor(0, index % 2); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)

      T[index] = str.substring(str.indexOf("T:") + 2, str.indexOf("H:")).toFloat();
      H[index] = str.substring(str.indexOf("H:") + 2, str.indexOf("H:") + 5).toFloat();
      
      digitalWrite(led_pin, LOW);
      updated = true;      
    }
    
    if (updated) {
      String toSend = "";
      for (int i=1; i<6; i=i+2) {
        char temp[7] = "";
        char hum[2] = "";
        dtostrf(T[1+(i/2)],2,2,temp);
        dtostrf(H[1+(i/2)],2,0,hum);
        String tempString(temp);
        tempString.trim();
        String humString(hum);  
        humString.trim();
        String indexer(i);
        String indexer2(i+1);
        toSend += ("field"+indexer+"="+tempString+"&field"+indexer2+"="+humString);
        if (i<5) {
          toSend += "&";     
        }
      }
      updated = false;

      // Print Update Response to Serial Monitor
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
      // Disconnect from ThingSpeak
      if (!client.connected() && lastConnected) {
        Serial.println("...disconnected");
        client.stop();
      }
      // Update ThingSpeak
      if (!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval)) {
        updateThingSpeak(toSend);
    
      }
      lastConnected = client.connected();
  
    }
  
  // listen for incoming clients
  client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");
          client.println();
          client.println("{");
          client.println("  \"readings\": [");
          for (int ix = 1; ix < 6; ix++) {
            client.print("    {");
            client.print("\"id\": \"");
            client.print(ix);
            client.print("\", \"temperature\": \"");
            client.print(T[ix]);
            client.print("\", \"humidity\": \"");
            client.print(H[ix]);
            client.print("\"}");
            if (ix<5) {
              client.print(",");
            }
            client.println();
          }          
          client.println("  ]");
          client.println("}");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    Ethernet.maintain();
  }
}

void updateThingSpeak(String tsData) {
  if (client.connect(thingSpeakAddress, 80)) {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: 184.106.153.149\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + APIKey + "\n");
    //client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    //client.print("api_key=" + APIKey + "\n");
    client.println(tsData);
    lastConnectionTime = millis();

    if (client.connected()) {
      Serial.println("Connecting to ThingSpeak...");
      Serial.print("Sending: ");
      Serial.println(tsData);
    }
  }
}

