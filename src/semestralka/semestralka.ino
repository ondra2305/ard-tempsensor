#include <DHT.h>
#include <Ethernet.h>

//vvvvvvvvvvvvvvvvv SET ETHERNET DIGITAL PIN HERE
#define ETHERNETPIN 10

//-------------------------------------------------------------
//vvvvvvvvvvvvvvvvv CHOOSE SDCARD/MQTT DATA COLLECTION HERE
// Uncomment either #define SDCARD or #define MQTT
//#define SDCARD
#define MQTT
//-------------------------------------------------------------

//vvvvvvvvvvvvvvvvv SET DATA LOGGING INTERVAL IN MS HERE
const int dataLogInterval = 10000;

#ifdef SDCARD
//-------------------------------------------------------------
#include <SPI.h>
#include <SD.h>
#include <NTPClient.h>

int lineCount = 0;
//vvvvvvvvvvvvvvvvv SET SDCARD SPI CS PIN HERE
#define SD_PIN 4
EthernetUDP Udp;
NTPClient timeClient(Udp, "pool.ntp.org", 7200);
//-------------------------------------------------------------
#endif

//vvvvvvvvvvvvvvvvv SET SENSOR DIGITAL PIN HERE
#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMillis = 0;

#ifdef MQTT
//-------------------------------------------------------------
#include <MQTT.h>
#define MQTTSERVER IPAddress(192, 168, 1, 100) //hostname can be used in " "
#define MQTT_USERNAME "admin"
#define MQTT_PASSWORD "Arduino"
#define MQTT_TEMP_TOPIC "home/sensors/Arduino/Temp"
#define MQTT_HUM_TOPIC "home/sensors/Arduino/Hum"

MQTTClient mqclient;
EthernetClient ethclient;
//-------------------------------------------------------------
#endif

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SET STATIC IP HERE IF NEEDED
//IPAddress ip(192, 168, 10, 254);

EthernetServer server(80);

#ifdef SDCARD
void renameOldFile()
{
    timeClient.update();

    if (SD.exists("SDATA.CSV")) {
        unsigned long timestamp = timeClient.getEpochTime();
        String newFileName = String("OLD" + String(timestamp % 100000UL) + ".CSV");  // Use only last 5 digits of timestamp
        File oldFile = SD.open("SDATA.CSV", FILE_READ);
        Serial.println(newFileName.c_str());
        File newFile = SD.open(newFileName.c_str(), FILE_WRITE);

        while (oldFile.available()) {
            newFile.write(oldFile.read());
        }
        newFile.close();
        oldFile.close();

        if (!SD.remove("SDATA.CSV")) {
            Serial.println(F("FILE RENAME ERR!"));
        }
    }
    return;
}

bool writeToFile(char tempString[8], char humString[8])
{
    timeClient.update();

    if (lineCount >= 60) {
        Serial.println(F("RENAMING"));
        renameOldFile();
        lineCount = 0;
    }

    File dataFile = SD.open("SDATA.CSV", FILE_WRITE);
    if (dataFile) {
      dataFile.print(timeClient.getEpochTime());
      dataFile.print(",");
      dataFile.print(tempString);
      dataFile.print(",");
      dataFile.println(humString);
      dataFile.close();

      lineCount++;
      return true;
    } else {
      return false;
    }
}
#endif

#ifdef MQTT
//-------------------------------------------------------------
void MQTTconnect() {
  int cnt = 0;
  Serial.print("MQTT conn");
  while (!mqclient.connect("Arduino", MQTT_USERNAME, MQTT_PASSWORD)) {
    if(cnt > 5)
    {
      Serial.println(F("MQTT ERR!"));
      while(true);
    }
    Serial.print(".");
    delay(1000);
  }

  mqclient.subscribe(MQTT_TEMP_TOPIC);
  mqclient.subscribe(MQTT_HUM_TOPIC);
}
//-------------------------------------------------------------
#endif

void readSensorToStrTemp(char tempString[8])
{
  double temp = dht.readTemperature();
  dtostrf(temp, 2, 2, tempString);
}

void readSensorToStrHum(char humString[8])
{
  double hum = dht.readHumidity();
  dtostrf(hum, 2, 2, humString);
}

void setup() {
  Ethernet.init(ETHERNETPIN);
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  dht.begin();

  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("DHCP ERR!"));

    if (Ethernet.hardwareStatus() == EthernetNoHardware)
      Serial.println(F("ETH SHLD ERR!"));

    if (Ethernet.linkStatus() == LinkOFF)
      Serial.println(F("ETH CABLE ERR!"));

    while (true);
  }

  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());

  #ifdef SDCARD
  if (!SD.begin(SD_PIN)) {
    Serial.println(F("SD ERR!"));
    while(true);
  }
  renameOldFile();
  #endif

  #ifdef MQTT
  mqclient.begin(MQTTSERVER, 1883, ethclient);
  MQTTconnect();
  #endif

  server.begin();
}

void loop() {
  char tempString[8];
  char humString[8];
  readSensorToStrTemp(tempString);
  readSensorToStrHum(humString);

  if (millis() - lastMillis > dataLogInterval) {
    lastMillis = millis();
    #ifdef MQTT
    mqclient.publish(MQTT_TEMP_TOPIC, tempString);
    mqclient.publish(MQTT_HUM_TOPIC, humString);
    #endif

    #ifdef SDCARD
    if(!writeToFile(tempString, humString))
    {
      Serial.println(F("DATAFILE ERR!"));
    }
    #endif
  }

  EthernetClient client = server.available();
  if (client) {
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 5");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<h1>Arduino Sensor</h1>");
          client.print("<hr>");
          client.print("<h2>Temperature: ");
          client.print(tempString);
          client.print(" C.</h2>");
          client.print("<h2>Humidity: ");
          client.print(humString);
          client.print(" %.</h2>");
          client.print("<hr>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}
