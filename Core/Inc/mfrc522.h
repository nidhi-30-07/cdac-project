
#ifndef MFRC522_H_
#define MFRC522_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

// SPI Handle
extern SPI_HandleTypeDef hspi1;

// MFRC522 Pins
#define MFRC522_CS_PORT   GPIOA
#define MFRC522_CS_PIN    GPIO_PIN_4

#define MFRC522_RST_PORT  GPIOB
#define MFRC522_RST_PIN   GPIO_PIN_0

// MFRC522 Commands
#define PCD_IDLE         0x00
#define PCD_AUTHENT      0x0E
#define PCD_TRANSCEIVE   0x0C
#define PCD_SOFTRESET    0x0F

#define PICC_REQIDL      0x26

// Status Codes
#define MI_OK            0
#define MI_ERR           1

// Function Prototypes
void MFRC522_Init(void);
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType);
uint8_t MFRC522_Anticoll(uint8_t *serNum);

#endif
