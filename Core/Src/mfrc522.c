
#include "mfrc522.h"

// MFRC522 Registers
#define CommandReg       0x01
#define ComIEnReg        0x02
#define DivIEnReg        0x03
#define ComIrqReg        0x04
#define ErrorReg         0x06
#define Status2Reg       0x08
#define FIFODataReg      0x09
#define FIFOLevelReg     0x0A
#define ControlReg       0x0C
#define BitFramingReg    0x0D
#define ModeReg          0x11
#define TxControlReg     0x14
#define TxASKReg         0x15
#define TModeReg         0x2A
#define TPrescalerReg   0x2B
#define TReloadRegH     0x2C
#define TReloadRegL     0x2D

// Low-Level SPI
static void MFRC522_Select(void)
{
    HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_RESET);
}

static void MFRC522_Deselect(void)
{
    HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_SET);
}

static uint8_t MFRC522_SPI_Transfer(uint8_t data)
{
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, 100);
    return rx;
}

// Register Access
static void MFRC522_WriteReg(uint8_t reg, uint8_t value)
{
    MFRC522_Select();
    MFRC522_SPI_Transfer((reg << 1) & 0x7E);
    MFRC522_SPI_Transfer(value);
    MFRC522_Deselect();
}

static uint8_t MFRC522_ReadReg(uint8_t reg)
{
    uint8_t value;
    MFRC522_Select();
    MFRC522_SPI_Transfer(((reg << 1) & 0x7E) | 0x80);
    value = MFRC522_SPI_Transfer(0x00);
    MFRC522_Deselect();
    return value;
}

static void MFRC522_SetBitMask(uint8_t reg, uint8_t mask)
{
    MFRC522_WriteReg(reg, MFRC522_ReadReg(reg) | mask);
}

static void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    MFRC522_WriteReg(reg, MFRC522_ReadReg(reg) & (~mask));
}

// Core Functions
void MFRC522_Init(void)
{
    // Hardware reset
    HAL_GPIO_WritePin(MFRC522_RST_PORT, MFRC522_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(MFRC522_RST_PORT, MFRC522_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(2);

    MFRC522_WriteReg(CommandReg, PCD_SOFTRESET);
    HAL_Delay(50);

    MFRC522_WriteReg(TModeReg, 0x8D);
    MFRC522_WriteReg(TPrescalerReg, 0x3E);
    MFRC522_WriteReg(TReloadRegL, 30);
    MFRC522_WriteReg(TReloadRegH, 0);

    MFRC522_WriteReg(TxASKReg, 0x40);
    MFRC522_WriteReg(ModeReg, 0x3D);

    // Enable antenna
    if (!(MFRC522_ReadReg(TxControlReg) & 0x03))
        MFRC522_SetBitMask(TxControlReg, 0x03);
}

// Card Detection
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType)
{
    uint8_t status = MI_ERR;
    uint8_t irqWait = 0x30;
    uint8_t lastBits;
    uint8_t n;

    MFRC522_WriteReg(BitFramingReg, 0x07);
    TagType[0] = reqMode;

    MFRC522_WriteReg(CommandReg, PCD_IDLE);
    MFRC522_WriteReg(FIFOLevelReg, 0x80);
    MFRC522_WriteReg(FIFODataReg, TagType[0]);
    MFRC522_WriteReg(CommandReg, PCD_TRANSCEIVE);
    MFRC522_SetBitMask(BitFramingReg, 0x80);

    for (uint16_t i = 2000; i > 0; i--)
    {
        n = MFRC522_ReadReg(ComIrqReg);
        if (n & irqWait) break;
    }

    MFRC522_ClearBitMask(BitFramingReg, 0x80);

    if (!(MFRC522_ReadReg(ErrorReg) & 0x1B))
    {
        status = MI_OK;
        lastBits = MFRC522_ReadReg(ControlReg) & 0x07;
        if (lastBits != 0) TagType[0] = lastBits;
    }

    return status;
}

// Anti-Collision
uint8_t MFRC522_Anticoll(uint8_t *serNum)
{
    uint8_t status = MI_ERR;
    uint8_t serNumCheck = 0;
    uint8_t n;

    MFRC522_WriteReg(BitFramingReg, 0x00);
    MFRC522_WriteReg(CommandReg, PCD_IDLE);
    MFRC522_WriteReg(FIFOLevelReg, 0x80);

    MFRC522_WriteReg(FIFODataReg, 0x93);
    MFRC522_WriteReg(FIFODataReg, 0x20);
    MFRC522_WriteReg(CommandReg, PCD_TRANSCEIVE);
    MFRC522_SetBitMask(BitFramingReg, 0x80);

    for (uint16_t i = 2000; i > 0; i--)
    {
        n = MFRC522_ReadReg(ComIrqReg);
        if (n & 0x30) break;
    }

    MFRC522_ClearBitMask(BitFramingReg, 0x80);

    if (!(MFRC522_ReadReg(ErrorReg) & 0x1B))
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            serNum[i] = MFRC522_ReadReg(FIFODataReg);
            serNumCheck ^= serNum[i];
        }

        if (serNumCheck == MFRC522_ReadReg(FIFODataReg))
            status = MI_OK;
    }

    return status;
}
