/****************************************************************************
 *  @project:     ATS Project - ESP32 code
 *  @file_name:   main.ino
 *  @brief:
 *  @note:
 *           
 *  @author:      Ivan Pavao Lozancic ivanl@dump.hr
 *  @date:        11-12-2018
 ****************************************************************************/
#include <WiFi.h>
#include <Int64String.h>
#include <string>

//#define WEBSOCKET

#ifdef WEBSOCKET
#include <WebSocketClient.h>  //Use WebSocket if defined
#else
#include <SocketIOClient.h>   //Use Socket.IO if WebSocket is not defined
#endif //WEBSOCKET

/****************************************************************************
 *                            Public defions
 ***************************************************************************/

//Board and code configuration
#define BAUD_RATE   115200
#define WIFI_ENABLE
#define SERVER_CONNECT
#define TEST_MODE
//#define TEST_MODE_US
//#define TEST_COMMANDS
//#define TEST_MODE_SERVER_READ
#define US_CONNECTED

//HC-SR04 Configuration
#define US_ECHO           0x0C  //GPIO-
#define US_TRIGGER        0x0E  //GPIO-
#define FIRST_LANE_COMP   10
#define SECOND_LANE_COMP  30

//LED PWM Control setup
#define LEDC_CHANNEL_0          0         //Use channel 0 of max 16
#define LEDC_TIMER_13_BIT       13        //Precision
#define LEDC_BASE_FREQ          5000      //PWM base freq
#define LIGHT_PIN               0x1B      //GPIO-27
#define LIGHT_FADE_TIME_COMPARE 50
                                        //DEFAULT MODE
#define DEFAULT_BRIGHTNESS      25      //127 - 50%
#define DEFAULT_MODE            0x00      
                                        //EMERGENCY MODE
#define EMERGENCY_BRIGHTNESS    0xFF    //255 - 100%
#define EMERGENCY_MODE          0x01
                                        //MAINTENCE MODE
#define MAINTENCE_BRIGTHNESS    0x00    //0 - 0%
#define MAINTENCE_MODE          0x02
                                        //ALWAYS_ON
#define DEFAULT_ALWAYS_ON        0x00   // 0 - OFF
uint8_t light_config[4] =  {DEFAULT_MODE, DEFAULT_BRIGHTNESS, DEFAULT_BRIGHTNESS, DEFAULT_ALWAYS_ON};  //[MODE] - [BRIGHTNESS] - [CURRENT_BRIGHTNESS] - [ALWAYS_ON]
uint32_t previousTime_LIGHT = 0;

//WiFi Configuration
#ifdef WIFI_ENABLE
#define HOT_SPOT

#ifdef HOT_SPOT
#define SSID      "ATS - Hotspot"
#define PASSWORD  "ats123A"
#endif //HOME

#endif //WIFI_ENABLE

//Server configuration
#ifdef SERVER_CONNECT
#define TIMER_POOL_COMPARE 5000
#define HOST  "ats-infokup.azurewebsites.net"
#define PATH  "" 
#define PORT  80
uint32_t previousTime_POOL = 0;
SocketIOClient client;
WiFiClient wificlient;
extern String RID;
extern String Rname;
extern String Rcontent;
#endif //SERVER_CONNECT

//COMMANDS
#define COMMAND_VEHICLEPASS 0x01
#define COMMAND_CO2_UPDATE  0x02
#define COMMAND_ERROR       0x03
#define COMMAND_UNIT_INIT   0x04
#define COMMAND_DIM_UPDATE  0x05
#define COMMAND_OTA_UPDATE  0x06
#define COMMAND_LAMP_MODE   0x07

//ERROR defions
#define ERR_CO2SENS_FALIURE 0x01
#define ERR_CARSENS_FALIURE 0x02
#define ERR_LIGHT_FALIURE   0x03

//TEST_DATA
#define TESTDATA_COMMAND_VEHICLEPASS  0x81         
#define TESTDATA_COMMAND_CO2UPDATE    0x201       
#define TESTDATA_COMMAND_ERROR        0x03
#define TESTDATA_COMMAND_UNIT_INIT    0x800000000001

//VEHICLE_PASS configuration
#define   TIMER_VP_COMPARE    60000
#define   TIMER_VP_CHECK_COMP 700
uint32_t  previousTime_VP_UP = 0;
uint32_t  previousTime_VP = 0;
uint8_t   car_num = 0;
uint8_t   ultrasonicFaliureOverflow = 0;

//CO2_UPDATE configuration
#define TIMER_CU_COMPARE 300000
uint32_t previousTime_CU = 0;

//ERROR configuration
#define TIMER_ERR_COMPARE 5000
uint32_t previousTime_ERR = 0;

/****************************************************************************
 *                            Public functions
 ***************************************************************************/

/****************************************************************************
 *  @name:        getDeviceInfo
 *  *************************************************************************
 *  @brief:       Get's all important informations about device
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-20-2018
 ***************************************************************************/
#ifdef TEST_MODE
void getDeviceInfo(){

  uint8_t MAC[6];
  uint64_t MACBuffer = 0;
  uint8_t COUNTER;

  Serial.println("========================");
  Serial.println("Device info:");

  //Get device MAC adress
  WiFi.macAddress(MAC);

  Serial.println("MAC Address: ");

  for(COUNTER = 0; COUNTER < 6; COUNTER++){
    MACBuffer = (uint64_t)MAC[COUNTER] << (40 - (8 * COUNTER)) | MACBuffer;
    
    #ifdef TEST_MODE
    Serial.print("MAC["); Serial.print(COUNTER); Serial.print("] = "); Serial.println(MAC[COUNTER]);
    #endif //TEST_MODE

  }
  
  Serial.println("========================");

  //Get device IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("========================");

}
#endif //TEST_MODE

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
    Serial.println();
    Serial.print("Falied to connect to: "); Serial.println(ssid);
    Serial.println("Restarting device...");
    #endif //TEST_MODE

    ESP.restart();

    return false;
  }

  #ifdef TEST_MODE
  Serial.print("Succesfully connected to: "); Serial.println(ssid);
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
#ifdef SERVER_CONNECT
bool clientConnect(char host[], uint16_t port){

  if(client.connect(host, port)){

    #ifdef TEST_MODE
    Serial.print("Connected to "); Serial.print(host); Serial.print(" to port:"); Serial.println(port);
    #endif //TEST_MODE

    return true;
  } else {

    #ifdef TEST_MODE
    Serial.println("Connection failed.");
    Serial.println("Restarting device...");
    #endif //TEST_MODE

    //ESP.restart();

    return false;
  }
}
#endif //SERVER_CONNECT

/****************************************************************************
 *  @name:        poolServer
 *  *************************************************************************
 *  @brief:       Sends empty data to not lose connection to the server
 *  @note:
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:     
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        01-16-2019
 ***************************************************************************/
#ifdef SERVER_CONNECT
void poolServer(){

  uint32_t currentTime = millis();

  if(currentTime > previousTime_POOL + TIMER_POOL_COMPARE){

    client.sendJSON("", "");

    previousTime_POOL = currentTime;
  }
}
#endif //SERVER_CONNECT

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

    dataBuffer = (uint64_t)TESTDATA_COMMAND_UNIT_INIT;

    data = String(int64String(dataBuffer));

    client.sendJSON("", data);

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
 *  @name:        sendCommand_Unit
 *  *************************************************************************
 *  @brief:       Sends test UNIT_ commnad with data to server
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
  }

  //Data encryption for UNIT_ command
  dataBuffer = MACBuffer << 4 | (uint64_t)COMMAND_UNIT_INIT;

  //Packing data buffer into string
  data = String(int64String(dataBuffer));

  #ifdef TEST_MODE
  Serial.print("MACBuffer = "); Serial.println(int64String(MACBuffer));
  Serial.print("Data buffer = "); Serial.println(data);
  Serial.print("Reversed: ");
  Serial.println(int64String(dataBuffer >> 4 & 0xFFFFFFFFFFFFF));
  #endif //TEST_MODE

  //Sending command with data to server
  client.sendJSON("", data);
}
/****************************************************************************
 *  @name:        setLED
 *  *************************************************************************
 *  @brief:       Set LED with PWM to desired brightness
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:     
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        01-05-2019
 ***************************************************************************/
void setLED(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {

  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * _min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

/****************************************************************************
 *  @name:        updateLights
 *  *************************************************************************
 *  @brief:       Updates current traffic light state.
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:     
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-16-2018
 ***************************************************************************/
void updateLights(bool pass){

  //If car passed by light pole
  uint32_t currentTime = millis();

  if(currentTime > previousTime_LIGHT + LIGHT_FADE_TIME_COMPARE){
    if(light_config[0] == DEFAULT_MODE && light_config[3] == 0){
      if(pass == true){
        setLED(LEDC_CHANNEL_0, 255);
        light_config[2] = 255;
      }
      else{
        if(light_config[2] > light_config[1]){

          light_config[2]-= 5;

          if(light_config[2] < light_config[1]){
            light_config[2] = light_config[1];
          }
        }
        else{
          light_config[2] = light_config[1];
        }
        setLED(LEDC_CHANNEL_0, light_config[2]);
      }
    }
    else if(light_config[0] == DEFAULT_MODE && light_config[3] == 1){
      setLED(LEDC_CHANNEL_0, light_config[2]);
    }
    else if(light_config[0] == EMERGENCY_MODE){
    setLED(LEDC_CHANNEL_0, EMERGENCY_BRIGHTNESS);
    }
    else if(light_config[0] == MAINTENCE_MODE){
    setLED(LEDC_CHANNEL_0, MAINTENCE_BRIGTHNESS);
    }

    previousTime_LIGHT = currentTime;
  }
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
void vehicleStateUpdate(uint8_t CAR_NUM){

  uint32_t data = (CAR_NUM << 4) | COMMAND_VEHICLEPASS;
  
  String dataBuffer = String(data);

  client.sendJSON("update", dataBuffer);

  delay(100);
}

/****************************************************************************
 *  @name:        command_UnitInit
 *  *************************************************************************
 *  @brief:       Command UNIT_ function.
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

  #ifdef TEST_MODE
  Serial.println("==========COMMAND================================");
  Serial.println("");
  Serial.println("=================================================");
  #endif //TEST_MODE
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
 *  @date:        15-01-2019
 ***************************************************************************/
void command_DimUpdate(uint64_t DATA){

  uint8_t brightness = (DATA >> 4) & 0xFF;
  uint8_t alwaysOn_mode = (DATA >> 5) & 0x01;
  
  light_config[1] = brightness;     //Set new defualt brightness of the light
  light_config[3] = alwaysOn_mode;  //Set new AlwaysOn mode

  #ifdef TEST_MODE
  Serial.println("==========COMMAND================================");
  Serial.print("Brightness of the light was changed to: "); Serial.println(brightness);
  Serial.print("Lamp always_on mode was changed to: "); Serial.println(alwaysOn_mode);
  Serial.println("=================================================");
  #endif //TEST_MODE
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

  #ifdef TEST_MODE
  Serial.println("==========COMMAND================================");
  Serial.println("");
  Serial.println("=================================================");
  #endif //TEST_MODE
}

/****************************************************************************
 *  @name:        command_LampMode
 *  *************************************************************************
 *  @brief:       Command LAMP_MODE function.
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        01-12-2019
 ***************************************************************************/
void command_LampMode(uint64_t DATA){

  uint8_t LAMP_MODE = DATA >> 4 & 0xFF;

  light_config[0] = LAMP_MODE;

  #ifdef TEST_MODE
  Serial.println("==========COMMAND================================");
  Serial.print("Lamp mode changed to: "); 
  Serial.println(LAMP_MODE);
  Serial.println("=================================================");
  #endif //TEST_MODE
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
    Serial.println("==========SERVER=================================");
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

    #ifdef TEST_MODE_SERVER_READ 
    Serial.print("Recieved data decrypted value: "); Serial.println(data);
    Serial.print("Data value: "); Serial.println(int64String(dataValue));
    #endif //TEST_MODE

    command = dataValue & 0xF; //First four bits

    switch(command){

      case COMMAND_UNIT_INIT: //UNIT_INIT

        command_UnitInit(dataValue);
      break;

      case COMMAND_DIM_UPDATE: //DIM_UPDATE

        command_DimUpdate(dataValue); 
      break;

      case COMMAND_OTA_UPDATE: //OTA

        command_OTA(dataValue);
      break;

      case COMMAND_LAMP_MODE: //LAMP_MODE

        command_LampMode(dataValue);
      break;

      default:

        //Command not found

        #ifdef TEST_MODE
        Serial.println("Received invalid command/data from server.");
        Serial.println("=================================================");
        #endif //TEST_MODE

        return false;
      break;
    }

    #ifdef TEST_MODE
    Serial.print("Received command: "); Serial.println(command);
<<<<<<< HEAD
    Serial.println("================================================="); Serial.println("");
=======
    Serial.println("=================================================");
>>>>>>> 12fa0f54de05b424c160da84eb96bec0641e9e42
    #endif //TEST_MODE

    return true;
  }
  return false;
}
/****************************************************************************
 *  @name:        ultrasonicCarRead
 *  *************************************************************************
 *  @brief:       Reads sensor data to check for car pass
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      [0] = there is no car in lanes
 *                [1] = there is car in first lane and maybe in second?
 *                [2] = there is car in second lane
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        01-05-2019
 ***************************************************************************/
#ifdef US_CONNECTED
uint8_t ultrasonicCarRead(){

  uint64_t duration; 
  uint32_t distance;

  digitalWrite(US_TRIGGER, LOW);
  delayMicroseconds(2);

// Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(US_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIGGER, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(US_ECHO, HIGH);

  // Calculating the distance
  distance = duration*0.034/2;

  #ifdef TEST_MODE_US
  Serial.print("Distance: ");
  Serial.println(distance);
  #endif //TEST_MODE_US

  if(distance != 0){

    if(ultrasonicFaliureOverflow != 0){
      
      if(ultrasonicFaliureOverflow == 10){
        sendCommand_UnitInit();
      }

      ultrasonicFaliureOverflow = 0;
    }

    if(distance > FIRST_LANE_COMP){

      //THERE IS NO CAR
      #ifdef TEST_MODE_US
      Serial.println("US_CarCheck: No car detected.");
      #endif //TEST_MODE_US

      return 0;
    }

    /*else if(distance > FIRST_LANE_COMP && distance <= SECOND_LANE_COMP){

      //CAR ON SECOND LANE

      #ifdef TEST_MODE_US
      Serial.println("US_CarCheck: Car is on second lane.");
      #endif //TEST_MODE_US

      return 2;
    }*/

    else if(distance <= FIRST_LANE_COMP){

      //CAR ON FIRST LANE

      #ifdef TEST_MODE_US
      Serial.println("US_CarCheck: Car is on first lane.");
      #endif //TEST_MODE_US

      return 1;
    }
  }
  else if(ultrasonicFaliureOverflow < 10){

    ultrasonicFaliureOverflow++;
  }
  else if(ultrasonicFaliureOverflow == 10){

    sendError(ERR_CARSENS_FALIURE);
  }
  
  return 0;
}
#endif //US_CONNECTED

/****************************************************************************
 *  @name:        sendError
 *  *************************************************************************
 *  @brief:       sends ERROR to the server
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      [true]  - read data includes command
 *                [false] - read data is false
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        01-14-2019
 ***************************************************************************/
bool sendError(uint8_t error){

  String data;
  uint8_t dataBuffer;
  uint32_t currentTime = millis();

  switch(error){

    #ifdef US_CONNECTED
    case ERR_CARSENS_FALIURE:

      dataBuffer = ERR_CARSENS_FALIURE << 4 | COMMAND_ERROR;

    break;
    #endif //US_CONNECTED

    case ERR_CO2SENS_FALIURE:

      dataBuffer = ERR_CO2SENS_FALIURE << 4 | COMMAND_ERROR;

    break;

    case ERR_LIGHT_FALIURE:

      dataBuffer = ERR_LIGHT_FALIURE << 4 | COMMAND_ERROR;

    break;

    default:

    return false; //NO COMMAND FOUND!
  }

  data = String(dataBuffer);

  if(currentTime >= previousTime_ERR + TIMER_ERR_COMPARE){

    client.sendJSON("", data);

    previousTime_ERR = currentTime;

    #ifdef TEST_MODE
    Serial.print("DataBuffer sent on ERROR command: "); Serial.println(String(dataBuffer)); 
    Serial.print("ERROR: "); Serial.print(String(dataBuffer >> 4 & 0xF)); Serial.println(" was sent to server.");
    #endif //TEST_MODE
  }

  return true;
}
/****************************************************************************
 *  @name:        checkCarPass
 *  *************************************************************************
 *  @brief:       Counts cars that passed
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

  #ifdef US_CONNECTED
  if(ultrasonicCarRead() > 0){
    
    if(currentTime > previousTime_VP + TIMER_VP_CHECK_COMP){

      previousTime_VP = currentTime;

      car_num++;

      #ifdef TEST_MODE
      Serial.println("CAR PASSED!");
      #endif //TEST_MODE
    }

    updateLights(true);
  }
  else{

    updateLights(false);
  }
  #else
  updateLights(false);
  #endif //US_CONNECTED

  if(currentTime == previousTime_VP_UP + TIMER_VP_COMPARE){

    previousTime_VP_UP = currentTime;

    #ifdef TEST_MODE
    Serial.print("Passed Cars: ");
    Serial.println(car_num);
    #endif //TEST_MODE

    vehicleStateUpdate(car_num);

    car_num = 0;
  }
}

/****************************************************************************
 *  @name:        readCO2Sensor
 *  *************************************************************************
 *  @brief:       Reads data from sensor
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      [data] - sensor value
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        12-01-2018
 ***************************************************************************/
uint16_t readCO2Sensor(){

  uint32_t sensorRead = millis();
  uint16_t data;

  data = (uint16_t)sensorRead >> 22 & 0x3FF;
  
  return  data;
}

/****************************************************************************
 *  @name:        checkCO2Emissions
 *  *************************************************************************
 *  @brief:       sends data from CO2 sensor to server
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        01-14-2019
 ***************************************************************************/
void checkCO2Emissions(){

  uint32_t currentTime = millis();
  uint16_t dataBuffer;
  String data;

  if(currentTime >= TIMER_CU_COMPARE + previousTime_CU){

    previousTime_CU = currentTime;

    dataBuffer = (uint16_t)readCO2Sensor() << 4 | COMMAND_CO2_UPDATE;

    data = String(dataBuffer);

    client.sendJSON("", data);
  }
}

/****************************************************************************
 *  @name:        Hardwer
 *  *************************************************************************
 *  @brief:       Set pins and other stuff for hardwer to work properly
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        01-05-2019
 ***************************************************************************/
void Hardware(){

  //Light setup
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LIGHT_PIN, LEDC_CHANNEL_0);
  pinMode(LED_BUILTIN, OUTPUT);

  //HC-SR04 setup
  #ifdef US_CONNECTED
  pinMode(US_ECHO, INPUT);
  pinMode(US_TRIGGER, OUTPUT);
  #endif //US_CONNECTED
}

/****************************************************************************
 *                            Setup function
 ***************************************************************************/
void setup() {

  #ifdef TEST_MODE
  Serial.begin(BAUD_RATE);
  #endif //TEST_MODE
  
  Hardware();

  //Flash the builtin LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
 
  #ifdef WIFI_ENABLE
  WiFiConnect(SSID, PASSWORD);

  #ifdef SERVER_CONNECT
  clientConnect(HOST, PORT);

  delay(1000);

  sendCommand_UnitInit();
  
  #ifdef TEST_COMMANDS
  sendTestData_SOCKETIO(COMMAND_VEHICLEPASS);
  sendTestData_SOCKETIO(COMMAND_CO2_UPDATE);
  sendTestData_SOCKETIO(COMMAND_ERROR);
  #endif //TEST_COMMANDS
  #endif //SERVER_CONNECT
  #endif //WIFI_ENABLE

  #ifdef TEST_MODE
  getDeviceInfo();
  #endif //TEST_MODE
}

/****************************************************************************
 *                            Main function
 ***************************************************************************/
void loop() {

  while(client.connected() && WiFi.status() == WL_CONNECTED){
    checkCarPass();

    commandHandler();

    checkCO2Emissions();

    #ifdef SERVER_CONNECT
    poolServer();
    #endif //TEST_MODE
  }

  Serial.println("Disconnected! Internet lost or server down. Restarting device in 2 seconds!");

  setLED(LEDC_CHANNEL_0, 255);
  delay(2000);
  setLED(LEDC_CHANNEL_0, 0);

  ESP.restart();
}