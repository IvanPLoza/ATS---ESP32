/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */
#define ESP32
#include <SocketIOClient.h>

SocketIOClient client;
const char* ssid     = "Retro_Caffe";
const char* password = "Lozinka12345";

char host[] = "192.168.1.16";
int port = 1337;
extern String RID;
extern String Rname;
extern String Rcontent;

unsigned long previousMillis = 0;
long interval = 5000;
unsigned long lastreply = 0;
unsigned long lastsend = 0;

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
if (client.connected())
  {
    client.send("connection", "update", "12");

    Serial.println("SUCCESS");
  }
}

void loop() {
unsigned long currentMillis = millis();
  if (client.monitor())
  {
    lastreply = millis(); 
    Serial.println(RID);
    if (Rname == "update")
    {
      Serial.print("Il est ");
      Serial.println(Rcontent);
    }
  }
  

}
