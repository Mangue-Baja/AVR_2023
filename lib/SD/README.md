    
## Funções para o SD card

### sd_device.h

- Inicializa o módulo SD e a estrutura do csv
```c
bool init_sd(uint8_t pin);
```

- Salva no cartão SD os dados de tempo em 30 e 100 metros e da velocidade final
```c
void save_Data(unsigned long _t30, unsigned long _t100, float _v)
```
