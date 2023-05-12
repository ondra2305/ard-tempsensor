#include <DHT.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <StreamString.h>
#include <NTPClient.h>
#include <ctime>

#ifndef STASSID
#define STASSID "TEST"
#define STAPSK "Arduino"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

// HTTP AUTH USERNAME & PASSWORD
const char* www_username = "admin";
const char* www_password = "esp8266";

//-------------------------------------------------------------
//vvvvvvvvvvvvvvvvv CHOOSE SDCARD/MQTT DATA COLLECTION HERE
// Uncomment either #define SDCARD or #define MQTT
//#define SDCARD 
#define MQTT

//vvvvvvvvvvvvvvvvv SET DATA LOGGING INTERVAL IN MS HERE
const int dataLogInterval = 1000;

#define DISCORDWH
//-------------------------------------------------------------

#ifdef DISCORDWH
#include <Discord_WebHook.h>
Discord_Webhook discord;
String DISCORD_WEBHOOK = "https://discord.com/api/webhooks/XXX";
#endif

#ifdef SDCARD
//-------------------------------------------------------------
#include <SPI.h>
#include <SD.h>

int lineCount = 0; 
//vvvvvvvvvvvvvvvvv SET SDCARD SPI CS PIN HERE
#define SD_PIN 8
WiFiUDP Udp;
NTPClient timeClient(Udp, "pool.ntp.org", 7200);
//-------------------------------------------------------------
#endif

//vvvvvvvvvvvvvvvvv SET SENSOR DIGITAL PIN HERE
#define DHTPIN 0
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
WiFiClient espwclient;
//-------------------------------------------------------------
#endif

ESP8266WebServer server(80);
const int led = 13;

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

String readSDSensorData() {
  String dataString = "";

  File dataFile = SD.open("SDATA.CSV");
  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      unsigned long timestamp = line.substring(0, line.indexOf(',')).toInt();

      // Convert timestamp to human readable format
      time_t rawtime = (time_t)timestamp;
      struct tm * dt;
      char buffer [30];
      dt = localtime(&rawtime);
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", dt);
      String formattedTime = String(buffer);

      line.replace(String(timestamp), formattedTime);

      dataString += line;
      dataString += ';';
    }
    dataFile.close();
  }
  return dataString;
}
#endif

StreamString handleRoot() {
  String tempString;
  String humString;
  readSensorToStrTemp(tempString, true);
  readSensorToStrHum(humString, true);

  #ifdef SDCARD
  String dataString = readSDSensorData();
  #endif
  #ifndef SDCARD  
  String dataString = "1683835583,23.5,52.5;1683835584,23.6,52.2;1683835585,23.7,52.1;1683835586,23.8,52.0;1683835587,23.9,51.9;1683835588,24.0,52.2;";
  #endif

  StreamString html;
  html.reserve(1000);  // Preallocate a large chunk to avoid memory fragmentation
  html.println("<!DOCTYPE HTML>"
          "<html>"
          "<head>"
          "<meta name=\"viewport\" content=\"width=device-width\">"
          "<meta http-equiv='refresh' content='5'>"
          "<style>"
          "body { background-color: #f2f2f2; font-family: Arial, sans-serif; margin: 0; }"
          "h1 { color: #4CAF50; }"
          "h2 { color: #5D6D7E; }"
          ".container { max-width: 600px; margin: 0 auto; }"
          "</style>"
          "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"  // Include Chart.js library
          "</head>"
          "<body>"
          "<div class='container' id='chartContainer' data-raw='");
  html.print(dataString);
  html.println("'>"
          "<h1>Arduino/ESP8266 Sensor</h1>"
          "<hr>"
          "<h2>Temperature: ");
  html.println(tempString);
  html.println(" C.</h2>"
          "<h2>Humidity:");
  html.println(humString);
  html.println(" %.</h2>"
          "<hr>"
          "<canvas id='tempChart'></canvas>"
          "<canvas id='humChart'></canvas>");
  #ifndef SDCARD
  html.println("<p style=\"color:red;\">Warning: SD NOT PRESENT! This is only testing data.</p>");
  #endif
  html.println( "<script>"
            "document.addEventListener('DOMContentLoaded', function() {"
            "  var container = document.getElementById('chartContainer');"
            "  var rawData = container.getAttribute('data-raw').trim().split(';');"
            "  var labels = [], tempData = [], humData = [];"
            "  for (var i = 0; i < rawData.length; i++) {"
            "    var splitData = rawData[i].split(',');"
            "    labels.push(splitData[0]);"
            "    tempData.push(splitData[1]);"
            "    humData.push(splitData[2]);"
            "  }"
            "  var ctx1 = document.getElementById('tempChart');"
            "  var tempChart = new Chart(ctx1, {"
            "    type: 'line',"
            "    data: {"
            "      labels: labels,"
            "      datasets: [{"
            "        label: 'Temperature',"
            "        data: tempData,"
            "        backgroundColor: 'rgba(75, 192, 192, 0.2)',"
            "        borderColor: 'rgba(75, 192, 192, 1)',"
            "        borderWidth: 1"
            "      }]"
            "    },"
            "    options: {"
            "      scales: {"
            "        y: {"
            "          beginAtZero: false"
            "        }"
            "      }"
            "    }"
            "  });"
            "  var ctx2 = document.getElementById('humChart');"
            "  var humChart = new Chart(ctx2, {"
            "    type: 'line',"
            "    data: {"
            "      labels: labels,"
            "      datasets: [{"
            "        label: 'Humidity',"
            "        data: humData,"
            "        backgroundColor: 'rgba(153, 102, 255, 0.2)',"
            "        borderColor: 'rgba(153, 102, 255, 1)',"
            "        borderWidth: 1"
            "      }]"
            "    },"
            "    options: {"
            "      scales: {"
            "        y: {"
            "          beginAtZero: false"
            "        }"
            "      }"
            "    }"
            "  });"
            "});"
            "</script>"
            "</div>"
            "</body>"
            "</html>");

    return html;
}

void readSensorToStrTemp(String& tempString, bool format)
{
  char buffer[8];
  double temp = dht.readTemperature();
  dtostrf(temp, 2, 2, buffer);
  tempString = String(buffer);
  if(tempString == "nan" && format)
  {
    tempString = "<p style='color:red;'>Sensor is not connected properly!</p>";
  }
}

void readSensorToStrHum(String& humString, bool format)
{
  char buffer[8];
  double hum = dht.readHumidity();
  dtostrf(hum, 2, 2, buffer);
  humString = String(buffer);
  if(humString == "nan" && format)
  {
    humString = "<p style='color:red;'>Sensor is not connected properly!</p>";
  }
}

void setup() {
  digitalWrite(led, 0);
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Wifi connecting");

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(led, 1);

  dht.begin();

  #ifdef DISCORDWH
  discord.begin(DISCORD_WEBHOOK);
  #endif

  #ifdef SDCARD
  if (!SD.begin(SD_PIN)) {
    Serial.println(F("SD ERR!"));
    while(true);
  }
  renameOldFile();
  #endif

  #ifdef MQTT
  mqclient.begin(MQTTSERVER, 1883, espwclient);
  MQTTconnect();
  #endif

  server.on("/", []() {
    if (!server.authenticate(www_username, www_password)) {
      return server.requestAuthentication();
    }
    StreamString html = handleRoot();
    server.send(200, "text/html", html.c_str());
  });

  // Optional "API" access
  // Can be changed to JSON formatting
  server.on("/temperature", []() {
    String tempString;
    readSensorToStrTemp(tempString, false);
    server.send(200, "text/plain", tempString.c_str());
  });
  server.on("/humidity", []() {
    String humString;
    readSensorToStrHum(humString, false);
    server.send(200, "text/plain", humString.c_str());
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  String tempString;
  String humString;
  readSensorToStrTemp(tempString, false);
  readSensorToStrHum(humString, false);

  if (millis() - lastMillis > dataLogInterval*10) 
  {
    lastMillis = millis();

    #ifdef DISCORDWH
    //CODE FOR DISCORD NOTIFICATIONS
    double temp = dht.readTemperature();
    double hum = dht.readHumidity();
    if(temp > 24 || hum > 70)
    {
      discord.send("TEMP SENSOR WARNING! Temperature now: " + tempString + "C, humidity: " + humString + "% !");
    }
    #endif
  }

  if (millis() - lastMillis > dataLogInterval) {
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

  server.handleClient();
}
