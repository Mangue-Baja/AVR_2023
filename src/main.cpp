#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include "soft_defs.h"
#include "hard_defs.h"

/* Defines type of ESP32 function */
#define M_0
//#define M_30
//#define M_100_101

/* Libraries Variables */
LiquidCrystal_I2C lcd(0x27, 16, 2); 
esp_now_peer_info_t peerInfo;

/* Global Variables */
bool esp_now_ok = false;
bool conf_30 = false, conf_100 = false;
uint8_t AddressFor_0[] = { 0x40, 0x91, 0x51, 0xFB, 0xEA, 0x18 };

/* Global Functions */
void Pin_Config();
/* ESP-NOW Functions */
void receiveCallBack(const uint8_t* macAddr, const uint8_t* data, int len);
void sentCallBack(const uint8_t* macAddr, esp_now_send_status_t status);
void formatMacAddress(const uint8_t* MACAddr, char* info, int maxLength);
bool sent_to_all(const uint8_t msg);
bool sent_to_single(const uint8_t msg);

void setup() 
{
  Serial.begin(115200);

  // Setup Pins
  Pin_Config();

  // Put ESP32 into Station Mode
  WiFi.mode(WIFI_MODE_STA);
  Serial.printf("Mac address: ");
  Serial.println(WiFi.macAddress());
  /*
    * E1_0 = 40:91:51:FB:EA:18
    * E2_30 = 40:91:51:FC:EA:CC
    * E3_100 = EC:62:60:9C:BD:DC
  */

  // Disconnect from Wifi
  WiFi.disconnect();

  #ifdef M_0
    lcd.init();
    lcd.backlight();
    lcd.clear();
  #endif
}

void loop() 
{
  switch(st)
  {
    case INIT:
    {
      if(esp_now_init()==ESP_OK)
      {
        //Serial.println("Init ESP_NOW Protocol");

        /* Broadcast Functions */ 
        esp_now_register_recv_cb(receiveCallBack);
        esp_now_register_send_cb(sentCallBack);

        #if defined(M_30) || defined(M_100_101)
          // Register peer
          memcpy(peerInfo.peer_addr, AddressFor_0, 6);
          peerInfo.channel = 0;
          peerInfo.encrypt = false;

          // Add peer
          if(esp_now_add_peer(&peerInfo)!=ESP_OK)
          {
            Serial.println("Failed to add peer");
            return;
          }
        #endif

      } else {

        #if defined(M_30) || defined(M_100_101)
          digitalWrite(LED_BUILTIN, HIGH);
          delay(1500);
          esp_restart();

          break;
        #endif  

        //Serial.println("ESP-NOW init Failed!!");
        digitalWrite(LED_BUILTIN, HIGH);

        lcd.print("ESP-NOW ERROR");
        delay(2000);
        lcd.clear();
        lcd.print("Reiniciando");
        delay(1000); lcd.print('.');
        delay(1000); lcd.print('.');
        delay(1000); lcd.print('.');

        delay(500);
        esp_restart(); 
      }

      #if defined(M_30) || defined(M_100_101)
        st=WAIT;
      #else
        while(!esp_now_ok || !conf_30 || !conf_100)
        {
          //Serial.printf("%d %d %d", esp_now_ok, conf_30, conf_100);
          sent_to_all(1);
          delay(500);
        }

        st = MENU;
        //(sent_to_all(1) ? st=WAIT : 0);
      #endif
      
      break;
    }
      
    case WAIT:
    {
      break;
    }

    case MENU:
    {
      break;
    }
  }
}

/* Setup Functions */
void Pin_Config()
{
  pinMode(LED_BUILTIN, OUTPUT);

  return;
}

/* ESP-NOW Functions */
void receiveCallBack(const uint8_t* macAddr, const uint8_t* data, int len)
{
  // Called when data is received
  uint8_t recv=0;
  
  memcpy(&recv, data, sizeof(data));
  Serial.print("Data received: ");
  Serial.println(len);
  Serial.print("Message: ");
  Serial.println(recv);
  Serial.println();

  if(recv==1)
  {
    #ifdef M_30
      //confirm_30 |= recv;
      //(sent_to_single(1) ? confirm_30 << recv : 0);
      sent_to_single(0);
    #elif defined(M_100_101)
      //confirm_100 |= recv;
      //(sent_to_single(2) ? confirm_100 << recv : 0);
      sent_to_single(2);
    #endif
  }

  if(recv==0)
    conf_30 = true;

  if(recv==2)
    conf_100 = true;
}

void sentCallBack(const uint8_t* macAddr, esp_now_send_status_t status) 
{
  // Called when data is sent
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);

  Serial.printf("Last packet sent to: %s\n", macStr);
  Serial.print("Last packet send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Sucess" : "Delivery Failed");

  if(status==ESP_NOW_SEND_SUCCESS)
    esp_now_ok = true;
}

void formatMacAddress(const uint8_t* MACAddr, char* info, int maxLength)
{
  // Formats MAC Address
  snprintf(info, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", MACAddr[0], MACAddr[1], MACAddr[2], MACAddr[3], MACAddr[4], MACAddr[5]);
}

bool sent_to_all(const uint8_t msg)
{
  // Broadcast a message to every device in range
  uint8_t BroadcastAdress[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  esp_now_peer_info_t BroadInfo = {};

  memcpy(&BroadInfo.peer_addr, BroadcastAdress, 6);

  if(!esp_now_is_peer_exist(BroadcastAdress))
  {
    esp_now_add_peer(&BroadInfo);
  }

  // Send message
  esp_err_t result = esp_now_send(BroadcastAdress, (uint8_t *)&msg, sizeof(msg));

  return result==ESP_OK ? true : false;
}

bool sent_to_single(const uint8_t msg)
{
  esp_err_t result = esp_now_send(AddressFor_0, (uint8_t *)&msg, sizeof(msg));

  return result==ESP_OK ? true : false;
}