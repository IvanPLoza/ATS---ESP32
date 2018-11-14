/****************************************************************************
 *  @project:     ATS Project - ESP32 code
 *  @file_name:   main.ino
 *  @brief:
 *  @note:
 *           
 *  @author:      Ivan Pavao Lozancic ivanplozancic@gmail.com
 *  @date:        11-12-2018
 ****************************************************************************/
#include <WiFi.h>

//#define WEBSOCKET
#ifdef WEBSOCKET
#include <WebSocketClient.h>  //Use WebSocket if defined
#else
#include <SocketIOClient.h>   //Use Socket.IO if WebSocket is not defined
#endif //WEBSOCKET

/****************************************************************************
 *                            Public definitions
 ***************************************************************************/

//Board and code configuration
#define BAUD_RATE 115200
#define TEST_MODE
#define WIFI_ENABLE
#define SERVER_CONNECT
//#define DEBUG

//WiFi Configuration
#ifdef WIFI_ENABLE

//#define HOME
#define DUMP
//#define BLUE_CAFFE

#ifdef HOME
#define SSID      "Jonelo2"
#define PASSWORD  "172030ZN"
#endif //HOME

#ifdef BLUE_CAFFE
#define SSID      "BlueEyes2"
#define PASSWORD  "blueeyes2"
#endif //BLUE_EYES

#ifdef DUMP
#define SSID      "dump"
#define PASSWORD  "Dump.12345"
#endif

#endif //WIFI_ENABLE

//Server configuration
#ifdef SERVER_CONNECT
#define HOST  "192.168.1.4"     //Replace with server IP
#define PATH  "/" 
#define PORT  1337
#define TEST_DATA "DrazenDebil"
#endif //SERVER_CONNECT

#ifdef WEBSOCKET
WebSocketClient webSocketClient;
WiFiClient client;
#else 
SocketIOClient client;
#endif //WEBSOCKET



/****************************************************************************
 *                            Public functions
 ***************************************************************************/

/****************************************************************************
 *  @name:        WiFiConnect
 *  *************************************************************************
 *  @brief:       Connect to WiFi using ESP32
 *  @note:
 *  *************************************************************************
 *  @param[in]:   char ssid []
 *                char password []
 *  @param[out]:   
 *  @return:      [true]  = wifi connection is established
 *                [false] = wifi connection failed
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-12-2018
 ***************************************************************************/
#ifdef WIFI_ENABLE
bool WiFiConnect(char ssid[], char password[]){

  uint8_t connectTimeOverflow = 0;

  WiFi.begin(ssid, password); //Connect to wifi

  //Check if device is connected
  while (WiFi.status() != WL_CONNECTED  && connectTimeOverflow < 50){

    delay(500);

    #ifdef TEST_MODE
    Serial.print ( "." );
    #endif

    connectTimeOverflow++;
  }
  
  if(WiFi.status() != WL_CONNECTED){

    #ifdef TEST_MODE
    Serial.print("Falied to connect to: "); Serial.println(ssid);
    #endif //TEST_MODE

    return false;
  }

  #ifdef TEST_MODE
  Serial.print("Succesfully connected to: "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  #endif //TEST_MODE

  return true;

}
#endif //WIFI_ENABLE

/****************************************************************************
 *  @name:        clientConnect
 *  *************************************************************************
 *  @brief:       Check if client connection to server is established
 *  @note:
 *  *************************************************************************
 *  @param[in]:   char host []
 *                char port []
 *  @param[out]:   
 *  @return:      [true]  = client connection is established
 *                [false] = client connection failed
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-12-2018
 ***************************************************************************/
bool clientConnect(char host[], uint16_t port){

  if(client.connect(host, port)){

    #ifdef TEST_MODE
    Serial.print("Connected to "); Serial.print(host); Serial.print(" to port:"); Serial.println(port);
    #endif //TEST_MODE

    return true;
  } else {

    #ifdef TEST_MODE
    Serial.println("Connection failed.");
    #endif //TEST_MODE

    return false;
  }
}

/****************************************************************************
 *  @name:        clientHandshake
 *  *************************************************************************
 *  @brief:       WebSocket client handshake (connect) function 
 *  @note:
 *  *************************************************************************
 *  @param[in]:   char path []
 *                char host []
 *  @param[out]:   
 *  @return:      [true]  = client connection is established
 *                [false] = client connection failed
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-12-2018
 ***************************************************************************/
#ifdef WEBSOCKET
bool clientHandshake(char path[], char host[]){

  webSocketClient.path = path;
  webSocketClient.host = host;

  if(webSocketClient.handshake(client)) {
    #ifdef TEST_MODE
    Serial.println("Handshake with WebSocket server was successful!");
    #endif //TEST_MODE

    return true;

  }

  #ifdef TEST_MODE
  Serial.println("Handshake with WebSocket server failed.");
  #endif //TEST_MODE
    
  return false;

}
#endif //WEBSOCKET

/****************************************************************************
 *  @name:        testCommunication
 *  *************************************************************************
 *  @brief:       Sends and receives data if WebSocket communication is 
 *                working.
 *  @note:        IF THERE IS NO DATA FROM SERVER, COMMUNICATION IS
 *                CONSIDERED AS FAILED!
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      [true]  = communication is working
 *                [false] = communication is not working
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-12-2018
 ***************************************************************************/
#ifdef WEBSOCKET
bool testCommunication_WEBSOCKET(){

  String dataReceived; //Received data

  webSocketClient.sendData(TEST_DATA);
  webSocketClient.getData(dataReceived);

  if(dataReceived.length() > 0){

    #ifdef TEST_MODE
    Serial.println("Communication is working.");
    Serial.print("Received data: ");
    Serial.println(dataReceived);
    #endif //TEST_MODE

    return true;
  }

  #ifdef TEST_MODE
  Serial.println("Communication falied!");
  #endif //TEST_MODE

  return false;


}
#endif //WEBSOCKET

/****************************************************************************
 *                            Setup function
 ***************************************************************************/
void setup() {

  Serial.begin(BAUD_RATE);
 
  WiFiConnect(SSID, PASSWORD);

  clientConnect(HOST, PORT);

  //clientHandshake(PATH, HOST);
 
}

/****************************************************************************
 *                            Main function
 ***************************************************************************/
void loop() {
 
  if(client.connected()){
 
    //testCommunication();

    client.monitor();
 
  } else{

    #ifdef TEST_MODE 
    Serial.println("Client disconnected.");
    #endif //TEST_MODE

  }
 
}