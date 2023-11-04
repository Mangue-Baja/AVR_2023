#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include "soft_defs.h"
#include "hard_defs.h"

#define DEBOUCE_TIME 150
/* Defines type of ESP32 function */
#define M_0
//#define M_30
//#define M_100_101

/* Libraries Variables */
LiquidCrystal_I2C lcd(0x27, 16, 2); 

/* Global Variables */
bool sd_exist = false;
bool esp_now_ok = false;

/* ESP-NOW Callbacks */
void receiveCallBack(const uint8_t* macAddr, const uint8_t* data, int len);
void formatMacAddress(const uint8_t* macAddr, char* info, int maxLength); 
void sentCallBack(const uint8_t* macAddr, esp_now_send_status_t status);
/* Global functions */
void Pin_Config();
bool sent_to_all(const String &msg);
bool read_config();
void Mount_SD();
char potSelect(uint8_t pin, uint8_t num_options);

void setup() 
{
  Serial.begin(115200);

  // Setup Pins
  Pin_Config();

  // Put ESP32 into Station Mode
  WiFi.mode(WIFI_MODE_STA);
  Serial.printf("Mac address: %s\n", WiFi.macAddress());

  // Disconnect from Wifi
  WiFi.disconnect();

  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void loop() 
{
  switch(st)
  {
    case INIT:
    {
      // Initialize ESP-NOW
      if(esp_now_init()==ESP_OK)
      {
        //esp_now_ok = true;
        Serial.println("Init ESP-NOW Protocol");

        /* Broadcast Functions */
        esp_now_register_recv_cb(receiveCallBack);
        esp_now_register_send_cb(sentCallBack);

      } else {
        Serial.println("ESP-NOW init Failed!!");
        digitalWrite(LED_BUILTIN, HIGH);

        lcd.print("ESP-NOW ERROR");
        delay(500);
        lcd.clear();
        lcd.print("Reiniciando");
        delay(500);
        lcd.setCursor(0,13); lcd.write('.');
        delay(1000);
        lcd.setCursor(0,14); lcd.write('.');
        delay(1000);
        lcd.setCursor(0,15); lcd.write('.');

        delay(500);
        esp_restart(); 
      }

      bool ss = sent_to_all("ok");

      ss ? st=WAIT : 0;
    }
      break;

    case WAIT:

      if(listen_30 && listen_100_101)
      {
        lcd.clear();
        lcd.print(F("Tudo Certo!"));
        delay(500);
        lcd.clear();

        listen_30 = false;
        listen_100_101 = false;

        st = BEGIN;

      } else {
        lcd.clear();
        lcd.print(F("Testando..."));
      }
      

      break;

    case BEGIN:

      #ifdef M_30
        /**/
        st = ST_30;
        break;
      #elif defined(M_100_101)
        /**/
        st = ST_100_101;
        break;
      #endif

      lcd.print(F("Bem vindo!!"));
      delay(250);
      lcd.clear();
      ((read_config()) ? (lcd.printf("SD OK!!: %d", sd_exist |= 0x01)) : (lcd.printf("Não há SD: %d", sd_exist &= ~0x01)));
      delay(1000);
      lcd.clear();  

      st = MENU;

      break;
  
    case MENU:
      pos[0] = 17; pos[1] = 23;

      pot_sel = potSelect(POT, 2);

      if(old_pot != pot_sel)
      {
        lcd.setCursor(0,0);
        lcd.print(F("MANGUE AVR - 4x4"));
        lcd.setCursor(1,0);
        lcd.print(F(" RUN  CONFIG"));
        lcd.setCursor(pos[pot_sel] % 16, pos[pot_sel] / 16);
        lcd.write('>');
        old_pot = pot_sel;
      }

      if(!digitalRead(B_SEL))
      {
        delay(DEBOUCE_TIME);
        while(!digitalRead(B_SEL));
        
        st = pot_sel + RUN;
        old_pot = -1;
      }
      
      break;

    case RUN:
      // Send the message here
      /* Not yet */
      break;

    case CONFIG:
      /* Not yet */
      break;

    case ST_30:
      break;

    case ST_100_101:
      break;
  }
}

/* Setup Functions */
void Pin_Config()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(B_SEL, INPUT_PULLUP);
  pinMode(B_CANCEL, INPUT_PULLUP);

  return;
}

/* ESP-NOW Functions */
void receiveCallBack(const uint8_t* macAddr, const uint8_t* data, int len)
{
  // Called when data is received
  char buffer[ESP_NOW_MAX_DATA_LEN + sizeof(char)];  
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, len);

  strncpy(buffer, (const char *)data, msgLen);
  buffer[msgLen] = 0; //Make sure we are null terminated
  
  // Format the MAC address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);

  Serial.printf("Receive message from: %s - %s\n", macStr, buffer);

  if(!strcmp("30_ok", buffer))
  {
    listen_30 = true;
    //lcd.print(F("Ouvindo 30"));
    //delay(500);
    //lcd.clear();
  }

  else if(!strcmp("100_101_ok", buffer))
  {
    listen_100_101 = true;
    //lcd.print(F("Ouvindo 100 &"));
    //lcd.setCursor(1,8); lcd.print(F("101"));
    //delay(500);
    //lcd.clear();
  }
  
}

void formatMacAddress(const uint8_t* MACAddr, char* info, int maxLength)
{
  // Formats MAC Address
  snprintf(info, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", MACAddr[0], MACAddr[1], MACAddr[2], MACAddr[3], MACAddr[4], MACAddr[5]);
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

bool sent_to_all(const String &msg)
{
  // Broadcast a message to every device in range
  uint8_t broadcastAdress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};

  memcpy(&peerInfo.peer_addr, broadcastAdress, 6);

  if(!esp_now_is_peer_exist(broadcastAdress))
  {
    esp_now_add_peer(&peerInfo);
  }

  // Send message
  esp_err_t result = esp_now_send(broadcastAdress, (const uint8_t *)msg.c_str(), msg.length());

  if(result==ESP_OK)
  {
    return true;
  }

  return false;
}

/* Global Functions */
bool read_config()
{
  if(!SD.begin()) 
    { delay(1000); return false; }
  else 
    { Mount_SD(); return true; }
}

void Mount_SD()
{
  int countfiles_onSD = 0;
  char file_name[20];
  File root = SD.open("/");

  while(true)
  {
    File entry = root.openNextFile();
    if(!entry)
      break; // no more files
    
    // for each file count it
    countfiles_onSD++;
    entry.close();
  }
    
  sprintf(file_name, "/%s%d.csv", "AV_data", countfiles_onSD);

  File data = SD.open(file_name, FILE_APPEND);

  if(data)
  {
    //data.printf("VEI=%s,MOLA MOTORA=%s,PRE CARGA MOTORA=%s,PESO=%s,MOLA MOVIDA=%s,PRE CARGA MOVIDA=%s", 
    //new_config.vei, new_config.mola_mot, new_config.pre_mot, new_config.peso_mot, new_config.mola_mov, new_config.pre_mov);
    data.println("Diga qual é o");
    data.println("Setup da CVT: ");
    data.close();

  } else {
    Serial.println(F("FAIL TO OPEN THE FILE"));
    sd_exist &= ~0x01;
  }
}

char potSelect(uint8_t pin, uint8_t num_options)
{
  /* Mapeia o curso do potenciometro para percorrer 2 vezes as opções de seleção,
  retorna o valor da opção escolhida */
  int read_val = analogRead(pin);
  uint8_t option = map(read_val, 50, 1000, 0, 2 * num_options - 1);
  //    if (option >= num_options)
  //        option -= num_options;
  return option % num_options;
}