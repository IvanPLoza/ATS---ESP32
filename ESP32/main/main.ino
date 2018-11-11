#include <WiFi.h>
#include <WebSocketClient.h>

 
#define SSID      "Jonelo2"
#define PASSWORD  "172030ZN"

#define HOST  "192.168.1.4"
#define PATH  "/" 
#define PORT  8999

#define BAUD_RATE 115200

#define TEST_DATA "DrazenDebil"

String dataReceived;
 
WebSocketClient webSocketClient;
WiFiClient client;
 
void setup() {
  Serial.begin(BAUD_RATE);
 
  WiFi.begin(SSID, PASSWORD);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
  delay(5000);
 
  if (client.connect(HOST, PORT)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
  }
 
  webSocketClient.path = PATH;
  webSocketClient.host = HOST;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
  }
 
}
 
void loop() {
 
  if (client.connected()) {
 
    webSocketClient.sendData(TEST_DATA);
 
    webSocketClient.getData(dataReceived);
    if (dataReceived.length() > 0) {
      Serial.print("Received data: ");
      Serial.println(dataReceived);
    }
 
  } else {
    Serial.println("Client disconnected.");
  }
 
}