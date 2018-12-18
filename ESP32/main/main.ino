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
#include <Int64String.h>
#include <string>

//#define WEBSOCKET

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

#define HOME
//#define DUMP
//#define BLUE_CAFFE
//#define RETRO
//#define MOBITEL
//#define DOMACIN
//#define EXELIA

#ifdef HOME
#define SSID      "Jonelo2"
#define PASSWORD  "172030ZN"
#endif //HOME

#ifdef BLUE_CAFFE
#define SSID      "Blue Eyes"
#define PASSWORD  "blueeyes"
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

#ifdef DOMACIN
#define SSID      "Domacin"
#define PASSWORD  "domacin123"
#endif //DOMACIN

#ifdef EXELIA
#define SSID      "Exelia"
#define PASSWORD  "nismofirma"
#endif //EXELIA

#endif //WIFI_ENABLE

//Server configuration
#ifdef SERVER_CONNECT
#define HOST  "192.168.1.9"
#define PATH  "/" 
#define PORT  2000
#define TEST_DATA "DrazenDebil"
#endif //SERVER_CONNECT

#ifdef WEBSOCKET
WebSocketClient webSocketClient;
WiFiClient client;
#else
SocketIOClient client;
WiFiClient wificlient;
extern String RID;
extern String Rname;
extern String Rcontent;
#endif //WEBSOCKET

//COMMANDS
#define COMMAND_VEHICLEPASS 0x01
#define COMMAND_CO2_UPDATE  0x02
#define COMMAND_ERROR       0x03
#define COMMAND_UNIT_INIT   0x04

//ERROR definitions
#define ERR_CO2SENS_FALIURE 0x01
#define ERR_CARSENS_FALIURE 0x02
#define ERR_LIGHT_FALIURE   0x03

//TEST_DATA
#define TESTDATA_COMMAND_VEHICLEPASS  0x81         
//10000001 + command = 100000010001 = 2065
//129

#define TESTDATA_COMMAND_CO2UPDATE    0x201       
//1000000001 + command = 10000000010010 = 8210
//513

#define TESTDATA_COMMAND_ERROR        0x03      
//11 + command = 110011 = 51
//3

#define TESTDATA_COMMAND_UNITINIT     0x800000000001
//100000000000000000000000000000000000000000000001 + command = 2251799813685268
//140737488355329 

//TEMP DATA 
#define SECTOR 0x01
#define ID     0x02¸

//VEHICLE_PASS configuration
#define TIMER_VP_COMPARE  180000
uint32_t previousTime = 0;
uint8_t car_num = 0;

//CO2_UPDATE configuration
#define TIMER_CU_COMPARE 300000

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
  uint64_t dataBuffer;

  if(command == COMMAND_VEHICLEPASS){

    dataBuffer = (uint64_t)TESTDATA_COMMAND_VEHICLEPASS << 4 | (uint64_t)COMMAND_VEHICLEPASS;

    data = String(int64String(dataBuffer));

    client.sendJSON("", data);

  } else if(command == COMMAND_CO2_UPDATE){
    
    dataBuffer = (uint64_t)TESTDATA_COMMAND_CO2UPDATE << 4 | (uint64_t)COMMAND_CO2_UPDATE;

    data = String(int64String(dataBuffer));


    client.sendJSON("", data);

  } else if(command == COMMAND_ERROR){

    dataBuffer = (uint64_t)TESTDATA_COMMAND_ERROR << 4 | (uint64_t)COMMAND_ERROR;

    data = String(int64String(dataBuffer));


    client.sendJSON("", data);
  
  } else if(command == COMMAND_UNIT_INIT){

    dataBuffer = (uint64_t)TESTDATA_COMMAND_UNITINIT;

    data = String(int64String(dataBuffer));

    client.sendJSON("init", data);

  } else {

    #ifdef TEST_MODE
    Serial.print("Command "); Serial.print(command); Serial.println(" is incorrect! Couldn't send data to server!");
    #endif //TEST_MODE

    return false;
  }

  #ifdef TEST_MODE
  Serial.print("Command "); Serial.print(command); Serial.println(" was sent to server.");
  Serial.print("Buffer: "); Serial.println(int64String(dataBuffer)); Serial.println();
  #endif //TEST_MODE

  return true;
}

/****************************************************************************
 *  @name:        sendCommand_UnitInit
 *  *************************************************************************
 *  @brief:       Sends test UNIT_INIT commnad with data to server
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:     
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        12-08-2018
 ***************************************************************************/
void sendCommand_UnitInit(){
  uint8_t MAC[6];
  uint64_t MACBuffer = 0;
  uint64_t dataBuffer;
  uint8_t COUNTER;
  String data;

  //Get MAC Address
  WiFi.macAddress(MAC);

  //Mac address encryption
  for(COUNTER = 0; COUNTER < 6; COUNTER++){
    MACBuffer = (uint64_t)MAC[COUNTER] << (40 - (8 * COUNTER)) | MACBuffer;
    
    #ifdef TEST_MODE
    Serial.print("MAC["); Serial.print(COUNTER); Serial.print("] = "); Serial.println(MAC[COUNTER]);
    #endif //TEST_MODE

  }

  //Data encryption for UNIT_INIT command
  dataBuffer = MACBuffer << 4 | (uint64_t)COMMAND_UNIT_INIT;

  //Packing data buffer into string
  data = String(int64String(dataBuffer));

  #ifdef TEST_MODE
  Serial.print("MACBuffer = "); Serial.println(int64String(MACBuffer));
  Serial.print("Data buffer = "); Serial.println(data);
  #endif //TEST_MODE

  //Sending command with data to server
  //client.sendJSON("init", data);
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
/*void vehicleStateUpdate(uint8_t CAR_NUM){

  uint32_t data = (CAR_NUM << 4) | COMMAND_VEHICLEPASS;
  
  String dataBuffer = String(data);

  client.sendJSON("update", dataBuffer);

  delay(100);
}*/

/****************************************************************************
 *  @name:        command_UnitInit
 *  *************************************************************************
 *  @brief:       Command UNIT_INIT function.
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-20-2018
 ***************************************************************************/
void command_UnitInit(uint64_t DATA){


}

/****************************************************************************
 *  @name:        command_DimUpdate
 *  *************************************************************************
 *  @brief:       Command DIM_UPDATE function.
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-20-2018
 ***************************************************************************/
void command_DimUpdate(uint64_t DATA){
  
}

/****************************************************************************
 *  @name:        command_OTA
 *  *************************************************************************
 *  @brief:       Command OTA function.
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-20-2018
 ***************************************************************************/
void command_OTA(uint64_t DATA){
  
}

/****************************************************************************
 *  @name:        commandHandler
 *  *************************************************************************
 *  @brief:       Reads data from server and executes related command
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      [true]  - read data includes command
 *                [false] - read data is false
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-20-2018
 ***************************************************************************/
bool commandHandler(){

  uint8_t command;
  uint64_t dataValue;
  uint8_t COUNTER;
  uint8_t COUNTER_2;
  uint8_t charsSize;
  char data[] = "";

  if(client.monitor()){

    #ifdef TEST_MODE
    Serial.print("Received data from server! Timestamp: "); Serial.println(millis());
    #endif //TEST_MODEž

    for(COUNTER = 0; COUNTER < Rcontent.length(); COUNTER++){
      if(Rcontent.charAt(COUNTER) == ','){

        charsSize = Rcontent.length() - 2 - COUNTER;

        for(COUNTER_2 = COUNTER + 1; COUNTER_2 < Rcontent.length() - 1; COUNTER_2++){
          data[COUNTER_2 - COUNTER - 1] = Rcontent.charAt(COUNTER_2);
        }
        
        COUNTER = Rcontent.length();
      }
    }

    dataValue = (uint64_t)atoi(data);

    #ifdef TEST_MODE 
    Serial.print("Recieved data decrypted value: "); Serial.println(data);
    Serial.print("Data value: "); Serial.println(int64String(dataValue));
    #endif //TEST_MODE

    command = dataValue & 0xF; //First four bits

    switch(command){

      case 0x04: //UNIT_INIT

        command_UnitInit(dataValue);
      break;

      case 0x05: //DIM_UPDATE

        command_DimUpdate(dataValue); 
      break;

      case 0x06: //OTA

        command_OTA(dataValue);
      break;

      default:

        //Command not found

        #ifdef TEST_MODE
        Serial.println("Received invalid command/data from server.");
        #endif //TEST_MODE

        return false;
      break;
    }

    #ifdef TEST_MODE
    Serial.print("Received command: "); Serial.println(command); Serial.println();
    #endif //TEST_MODE

    return true;
  }
  return false;
}
/****************************************************************************
 *  @name:        checkCarPass
 *  *************************************************************************
 *  @brief:       Reads data from sensor and counts cars that passed
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        12-01-2018
 ***************************************************************************/
void checkCarPass(){

  uint32_t currentTime = millis();

  if(currentTime == previousTime + TIMER_VP_COMPARE){

    previousTime = currentTime;

    //vehicleStateUpdate(car_num);

    car_num = 0;
  }
}

/****************************************************************************
 *  @name:        checkCO2Emissions
 *  *************************************************************************
 *  @brief:       Reads data from sensor and saves emissions
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        12-01-2018
 ***************************************************************************/
void checkCO2Emissions(){

}

/****************************************************************************
 *                            Setup function
 ***************************************************************************/
void setup() {

  Serial.begin(BAUD_RATE);
 
  WiFiConnect(SSID, PASSWORD);

  //clientConnect(HOST, PORT);

  wificlient.connect(HOST, PORT);
  wificlient.println("GET /?mac=test HTTP/1.1"); 
  wificlient.println();

  //client.sendJSON("mac", "2222");

  delay(1000);

  /*endTestData_SOCKETIO(COMMAND_UNIT_INIT);
  sendTestData_SOCKETIO(COMMAND_VEHICLEPASS);
  sendTestData_SOCKETIO(COMMAND_CO2_UPDATE);
  sendTestData_SOCKETIO(COMMAND_ERROR);

  sendCommand_UnitInit();*/
 
}

/****************************************************************************
 *                            Main function
 ***************************************************************************/
void loop() {

  //commandHandler();
  // HTTP Basic Authorization

  checkCarPass();

}