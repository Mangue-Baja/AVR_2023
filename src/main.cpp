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
File data;

/* Debug Variables */
bool db = false;
bool sel = false;
/* Global Variables */
char file_name[20];
bool sd_exist = false;
uint8_t AddressFor_0[/*M_0 adress*/] = {0x40, 0x91, 0x51, 0xFB, 0xEA, 0x18}; // Each ESP32 have your Mac Adress

/* Interrupts Routine */
void SelectISR();
/* Global Functions */
void Pin_Config();
bool Mount_SD();
void printRun();
String format_time(unsigned long int t1);
/* ESP-NOW Functions */
void receiveCallBack(const uint8_t* macAddr, const uint8_t* data, int len);
void sentCallBack(const uint8_t* macAddr, esp_now_send_status_t status);
void formatMacAddress(const uint8_t* macAddr, char* info, int maxLength);
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
    * E2_30 = A8:42:E3:C8:47:48
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
        st = WAIT;
      #else
        do
        {
          sent_to_all(1);
          delay(DEBOUCE_TIME);
          //Serial.printf("%d %d %d\n", esp_now_ok, conf_30, conf_100);
        } while(!esp_now_ok || !conf_30 || !conf_100);
        

        lcd.print(F("ESP-NOW ok!"));
        delay(500);
        lcd.clear();

        st = SD_BEGIN;
        //(sent_to_all(1) ? st=WAIT : 0);
      #endif
      
      break;
    }
      
    case WAIT:
    {
      delay(50);
      break;
    }

    case SD_BEGIN:
    {
      (Mount_SD() ? sd_exist |= 0x01 : sd_exist &= ~0x01);

      lcd.print(F((sd_exist ? "SD Instalado" : "Nao ha SD")));
      delay(DEBOUCE_TIME*5);
      lcd.clear();

      st = MENU;
      digitalWrite(LED_BUILTIN, LOW);

      break;
    }

    case MENU:
    {
      if(pot_sel!=old_pot)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(F("MANGUE AV - 4x4"));
        lcd.setCursor(0,1);
        lcd.print(F("   RUN"));
        lcd.setCursor(0,1);
        lcd.print(" ");
        lcd.write('>');
        old_pot = pot_sel;
      }

      if(sel)
      {
        delay(DEBOUCE_TIME);
        lcd.clear();

        t_30 = 0;
        t_100 = 0;
        t_101 = 0;
        vel = 0;
        t_curr = millis();
        
        old_pot = -1;
        sel = false;
        st = RUN;
      }

      /*
      if(!digitalRead(B_SEL))
      {
        delay(DEBOUCE_TIME);
        while(!digitalRead(B_SEL));

        lcd.clear();

        t_30 = 0;
        t_100 = 0;
        t_101 = 0;
        vel = 0;
        t_curr = millis();
        
        old_pot = -1;
        st = RUN;
      }
      */

      break;
    }

    case RUN:
    {
      if(!digitalRead(B_CAN))
      {
        delay(DEBOUCE_TIME);
        while(!digitalRead(B_CAN));

        st = MENU;
        old_pot = -1;
        sent_to_all(3);
      }

      switch(ss_r)
      {
        case START_:
        {
          t_30 = 0;
          t_100 = 0;
          t_101 = 0;
          vel = 0;
          str_vel = "00.00 km/h";

          printRun();

          break;
        }
      }
      
      break;
    }
  }
}

/* Setup Functions */
void Pin_Config()
{
  pinMode(LED_BUILTIN, OUTPUT);
  #ifdef M_0
    //pinMode(B_SEL, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(B_SEL), SelectISR, FALLING);
    pinMode(B_CAN, INPUT_PULLUP);
  #endif

  return;
}

/* ESP-NOW Functions */
void receiveCallBack(const uint8_t* macAddr, const uint8_t* data, int len)
{
  // Called when data is received
  uint8_t recv=0;
  
  memcpy(&recv, data, sizeof(data));
  //Serial.print("Data received: ");
  //Serial.println(len);
  //Serial.print("Message: ");
  //Serial.println(recv);
  //Serial.println();

  if(recv==0)
  {
    conf_30 = true;
  }

  else if(recv==1)
  {
    #ifdef M_30
      sent_to_single(0);
    #endif

    #ifdef M_100_101
      sent_to_single(2);
    #endif
  }

  else if(recv==2)
  {
    conf_100 = true;
  }

  else if(recv==3)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void sentCallBack(const uint8_t* macAddr, esp_now_send_status_t status) 
{
  // Called when data is sent
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);

  //Serial.printf("Last packet sent to: %s\n", macStr);
  //Serial.print("Last packet send status: ");
  //Serial.println(status==ESP_NOW_SEND_SUCCESS ? esp_now_ok |= 0x01 : esp_now_ok &= ~0x01);

  (status==ESP_NOW_SEND_SUCCESS ? esp_now_ok |= 0x01 : esp_now_ok &= ~0x01);
}

void formatMacAddress(const uint8_t* macAddr, char* info, int maxLength)
{
  // Formats MAC Address
  snprintf(info, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

bool sent_to_all(const uint8_t msg)
{
  // Broadcast a message to every device in range
  uint8_t BroadcastAdress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  esp_now_peer_info_t BroadInfo = {};

  memcpy(&BroadInfo.peer_addr, BroadcastAdress, 6);

  if(!esp_now_is_peer_exist(BroadcastAdress)) { esp_now_add_peer(&BroadInfo); }

  // Send message
  esp_err_t result = esp_now_send(BroadcastAdress, (uint8_t *)&msg, sizeof(msg));

  return result==ESP_OK ? true : false;
}

bool sent_to_single(const uint8_t msg)
{
  esp_err_t result = esp_now_send(AddressFor_0, (uint8_t *)&msg, sizeof(msg));

  return result==ESP_OK ? true : false;
}

/* Global Functions */
bool Mount_SD()
{
  if(!SD.begin(SD_CS)) { return false; }

  File root = SD.open("/");
  int CountFilesOnSD = 0;

  while (true)
  {
    File entry = root.openNextFile();
    // no more files
    if(!entry) { break; }

    // for each file count it
    CountFilesOnSD++;
    entry.close();
  }

  sprintf(file_name, "/%s%d.csv", "AV_data", CountFilesOnSD);

  data = SD.open(file_name, FILE_APPEND);

  return data ? true : false;
}

void printRun()
{
  str_30 = format_time(t_30);
  str_100 = format_time(t_100);
  str_101 = format_time(t_101);

  lcd.setCursor(0,0);
  lcd.print(' ' + str_30 + "  " + str_100 + "    ");
  lcd.setCursor(0, 1);
  lcd.print("   " + str_vel + "        ");
}

String format_time(unsigned long int t1)
{
  if(t1 < 10000)
    return '0' + String(t1 / 1000) + ':' + String(t1 % 1000);
  else
    return String(t1 / 1000) + ':' + String(t1 % 1000);
}

/* Interrupts Routine */
void SelectISR()
{
  sel = true;
}