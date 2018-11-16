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
//#define DUMP
//#define BLUE_CAFFE
//#define RETRO
#define MOBITEL

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
#endif  //DUMP

#ifdef RETRO
#define SSID      "Retro_Caffe"
#define PASSWORD  "Lozinka12345"
#endif //RETRO

#ifdef MOBITEL
#define SSID      "loza"
#define PASSWORD  "la123456"
#endif

#endif //WIFI_ENABLE

//Server configuration
#ifdef SERVER_CONNECT
#define HOST  "192.168.43.208"     //Replace with server IP
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

//COMMANDS
#define COMMAND_VEHICLEPASS   0x01
#define COMMAND_CO2_UPDATE    0x02
#define COMMAND_ERROR         0x03
#define COMMAND_UNIT_INIT     0x04

//TEST_DATA
#define TESTDATA_COMMAND_VEHICLEPASS  "000000010000000001"
#define TESTDATA_COMMAND_CO2UPDATE    "0000000100000000010000000001"
#define TESTDATA_COMMAND_ERROR        "00000001000000000101"
#define TESTDATA_COMMAND_UNITINIT     "000000000000000000000000000000000000000000000011"

//TEMP DATA
#define SECTOR 0x01
#define ID     0x02

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

  Serial.print("Connecting to: "); Serial.println(ssid);

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

  webSocketclient.sendData(TEST_DATA);
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
 *  @name:        sendTestData_SOCKETIO
 *  *************************************************************************
 *  @brief:       Sends test data to server
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      [true]  - command is correct
 *                [false] - command is incorrect
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-16-2018
 ***************************************************************************/
bool sendTestData_SOCKETIO(uint8_t command){

  String data;

  if(command == COMMAND_VEHICLEPASS){

    data = String(COMMAND_VEHICLEPASS) + TESTDATA_COMMAND_VEHICLEPASS;

    client.sendJSON("update", data);

    return true;

  } else if(command = COMMAND_CO2_UPDATE){
    
    data = String(COMMAND_CO2_UPDATE) + TESTDATA_COMMAND_CO2UPDATE;

    client.sendJSON("update", data);

    return true;

  } else if(command = COMMAND_ERROR){

    data = String(COMMAND_ERROR) + TESTDATA_COMMAND_ERROR;

    client.sendJSON("update", data);

    return true;
  
  } else if(command = COMMAND_UNIT_INIT){

    data = String(COMMAND_UNIT_INIT) + TESTDATA_COMMAND_UNITINIT;

    client.sendJSON("update", data);

    return true;

  }

  return false;
}

/****************************************************************************
 *  @name:        updateLights
 *  *************************************************************************
 *  @brief:       Updates current traffic light state for defined secotr.
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:     
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-16-2018
 ***************************************************************************/
void updateLights(){
}

/****************************************************************************
 *  @name:        vehicleStateUpdate
 *  *************************************************************************
 *  @brief:       Sends vehiclePass comand to server
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   string data - data buffer
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-16-2018
 ***************************************************************************/
void vehicleStateUpdate(){

  String data = String(COMMAND_VEHICLEPASS) + SECTOR + ID;

  client.sendJSON("update", data);

  delay(100);
}

/****************************************************************************
 *                            Setup function
 ***************************************************************************/
void setup() {

  Serial.begin(BAUD_RATE);
 
  WiFiConnect(SSID, PASSWORD);

  clientConnect(HOST, PORT);

  delay(1000);

  sendTestData_SOCKETIO(COMMAND_VEHICLEPASS);
  sendTestData_SOCKETIO(COMMAND_CO2_UPDATE);
  sendTestData_SOCKETIO(COMMAND_ERROR);
  sendTestData_SOCKETIO(COMMAND_UNIT_INIT);
 
}

/****************************************************************************
 *                            Main function
 ***************************************************************************/
void loop() {
 
}