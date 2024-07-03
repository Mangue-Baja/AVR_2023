#include <Arduino.h>

#define Module_0m   0x00 // id do modulo
#define Module_30m  0x03 // id do modulo
#define Module_100m 0x10 // id do modulo
#define BRIDGE      0xff // id do modulo

/* Selecione por meio desse define para qual modulo esta indo o Firmware
 * Module_0m == 0x00, referente ao modulo fixado no inicio do AV(0 metros) que 
 possui com ele o modulo LCD para mostrar o tempo e a velocidade do prototipo 
 juntamente com o modulo SD para armazenar esses dados;
 
 * Module_30m == 0x03, modulo com sensor fixado nos 30 metros para aquisição da aceleração;
 
 * Module_100m == 0x10, modulo com sensor fixado nos 100 e 101 metros para aquisição da velocidade
 e também do tempo em 100 metros;
 
 * BRIDGE == 0xff, apenas um modulo que servira de ponte para que a comunicação ESP-NOW possa acontecer
 sem que haja erro de sinal ou até mesmo não conseguir se comunicar.
*/
#define MODE Module_0m

void setup() 
{
  Serial.begin(115200);
  
  switch (MODE)
  {
    case Module_0m:
      /* code */
      break;

    case Module_30m:
      /* code */
      break;

    case Module_100m:
      /* code */
      break;

    case BRIDGE:
      /* code */
      break;

    default:
      pinMode(LED_BUILTIN, OUTPUT);
      for (;;)
      {
        Serial.println("MODULO NAO DEFINIDO, BUILD NOVAMENTE");
        delay(250);
        digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
      }
      break;
  }
}

void loop() {}
