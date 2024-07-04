
## Funções para a comunicação ESP-NOW

### AV_espnow.h

- Ponteiro que referencia a funções do tipo/retorno **void** com os parâmetros **const uint8_t*** , **const uint8_t*** , **int**
```c
typedef void (*esp_now_receiver_callback_t)(const uint8_t *, const uint8_t *, int);
```

- Ponteiro que referencia a funções do tipo/retorno **void** com os parâmetros **const uint8_t*** , **esp_now_send_status_t**
```c
typedef void (*esp_now_transmitter_callback_t)(const uint8_t *, esp_now_send_status_t);
```
