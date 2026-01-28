/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <lwip.h>
#include "enc28j60.h"   //ENC28j60
#include "ethernetif.h"
#include "lwip/udp.h"
#include <string.h>
#include "mfrc522.h"
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include "ssd1306_conf.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
ENC_HandleTypeDef enc_handle;
static uint8_t enc_flag = 0;

extern struct netif gnetif;  //netif object

static struct udp_pcb *udp_pcb_handle;
static ip_addr_t pc_ip;
static u16_t bbb_port =  5005;

volatile uint8_t udp_rx_flag = 0;

uint8_t angle=0;
/* RFID */
//volatile uint8_t rfid_ready = 0;
//uint8_t rfid_uid[8];
//uint8_t rfid_uid_len = 0;

/* UDP reply */
volatile uint8_t udp_reply_ready = 0;
char udp_reply_buf[32];

MFRC522_t rfid = {
    .hspi   = &hspi1,
    .csPort = GPIOA,
    .csPin  = GPIO_PIN_4,
    .rstPort = GPIOB,
    .rstPin  = GPIO_PIN_1
};

uint8_t rfid_uid[5];
volatile uint8_t rfid_present = 0;

char AText[] = "Access Granted";
char DText[] = "Access Denied";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI1_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM3_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void udp_send_data(void);
void udp_rx_callback(void *arg,
                     struct udp_pcb *pcb,
                     struct pbuf *p,
                     const ip_addr_t *addr,
                     u16_t port);
void rfid_poll(void);
void Servo_SetAngle(uint8_t angle);
//void MFRC522_Recover(void);
//static uint8_t uid_is_valid(uint8_t *uid);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_SPI1_Init();
  MX_IWDG_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  MFRC522_Init(&rfid);

  uint8_t mac[6] = {0x02, 0x12, 0x34, 0x56, 0x78, 0x9A};

  enc_handle.Init.MACAddr = mac;
  enc_handle.Init.DuplexMode = ETH_MODE_HALFDUPLEX;
  enc_handle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
  enc_handle.Init.InterruptEnableBits = EIE_LINKIE | EIE_PKTIE;

  HAL_Delay(100);
  if (!enc_start(&enc_handle))
  {
      // ENC not detected → SPI / CS / wiring issue
      while (1)
      {
      }
  }

  enc_set_MAC(&enc_handle);

  /*Initialize lwIP stack*/
  //lwip_init();
  MX_LWIP_Init();
  
  /* UDP init */
    udp_pcb_handle = udp_new();
    if (udp_pcb_handle == NULL)
      while (1);

    IP4_ADDR(&pc_ip, 192,168,1,20);

    udp_bind(udp_pcb_handle, IP_ADDR_ANY, 5005);
    //udp_connect(udp_pcb_handle, &pc_ip, 5005);
    udp_recv(udp_pcb_handle, udp_rx_callback, NULL);

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

    ssd1306_Init();

   ssd1306_Fill(Black);
   ssd1306_UpdateScreen();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  rfid_poll();
	  //ssd1306_Fill(Black);
	  //ssd1306_UpdateScreen();
	  if(enc_flag)
	  {
		  enc_flag = 0;
		  enc_irq_handler(&enc_handle);

		  //while(enc_get_received_frame(&enc_handle))
		  //{
			  ethernetif_input(&gnetif);
		  //}
			  enc_enable_interrupts(EIE_INTIE | EIE_PKTIE | EIE_LINKIE);
	  }

	  if(enc_handle.transmitLength > 0)
	  {
		  ethernet_transmit();
	  }
	  
	  //rfid_poll();

	  if (rfid_present)
	  {
	      udp_send_data();
	      //card_sent = 1;
	      rfid_present = 0;
	  }


	  if (udp_reply_ready)
	  {
		  udp_reply_ready = 0;

		  if(strcmp(udp_reply_buf, "ACK\n") == 0)
		  {
			  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, SET);
			  ssd1306_SetCursor(0, 10);
			  ssd1306_WriteString("Access", Font_11x18, White);

			  ssd1306_SetCursor(0, 35);
			  ssd1306_WriteString("Granted", Font_11x18, White);
			  ssd1306_UpdateScreen();
			  //HAL_Delay(1000);
			  for (angle = 0; angle <= 180; angle++)
			  {
			      Servo_SetAngle(angle);
			      HAL_Delay(10);
			  }

			  HAL_Delay(1000);
			  ssd1306_SetCursor(0, 0);
			  ssd1306_Fill(Black);
			  ssd1306_UpdateScreen();
			  /* Close */
			  for (angle = 180; angle > 0; angle--)
			  {
			      Servo_SetAngle(angle);
			      HAL_Delay(10);
			  }
			  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, RESET);

			  //ssd1306_Fill(Black);
			  // Open slowly

		  }
		  else if(strcmp(udp_reply_buf, "DENY\n") == 0)
		  {
			  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, SET);
			  ssd1306_SetCursor(0, 10);
			  ssd1306_WriteString("Access", Font_11x18, White);

			  ssd1306_SetCursor(0, 35);
			  ssd1306_WriteString("Denied", Font_11x18, White);
			  ssd1306_UpdateScreen();
			  HAL_Delay(3000);
			  ssd1306_SetCursor(0, 0);
			  ssd1306_Fill(Black);
			  ssd1306_UpdateScreen();
			  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, RESET);
			  //ssd1306_Fill(Black);

		  }
	  }
	      	      
	  sys_check_timeouts();

	  HAL_IWDG_Refresh(&hiwdg);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = 1250;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 15;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 19999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15
                           PD4 PD5 PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_0)
	{
		enc_flag = 1;
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);  //Ethernet frame reached ENC
	}

}

/* UDP SEND */
void udp_send_data(void)
{
    char msg[32];

    //if (rfid_present)
    //{
        snprintf(msg, sizeof(msg),
                 "UID:%02X%02X%02X%02X",
                 rfid_uid[0],
                 rfid_uid[1],
                 rfid_uid[2],
                 rfid_uid[3]);

    /*    rfid_present = 0;
    }
    else
    {
        return;
    }*/

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg), PBUF_RAM);
    if (!p) return;

    memcpy(p->payload, msg, strlen(msg));
    udp_sendto(udp_pcb_handle, p, &pc_ip, bbb_port);
    pbuf_free(p);
}

void rfid_poll(void)
{
    static uint8_t last_uid[4] = {0};
    //static uint8_t card_present = 0;
    uint8_t atqa[2];

    MFRC522_AntennaOn(&rfid);
    HAL_Delay(10);  // Let RF stabilize

    if (MFRC522_RequestA(&rfid, atqa) == STATUS_OK)
    {
			if (MFRC522_ReadUid(&rfid, rfid_uid) == STATUS_OK)
			{
				/* If UID is same as last one, ignore */
				if (memcmp(last_uid, rfid_uid, 4) == 0)
					return;

				/* New card detected */
				memcpy(last_uid, rfid_uid, 4);
				rfid_present = 1;
				//card_present = 1;   // lock until removal

			}
    	}

}

/* UDP RX CALLBACK (NON-BLOCKING) */
void udp_rx_callback(void *arg,
                     struct udp_pcb *pcb,
                     struct pbuf *p,
                     const ip_addr_t *addr,
                     u16_t port)
{
    if (!p) return;

    memset(udp_reply_buf, 0, sizeof(udp_reply_buf));
    memcpy(udp_reply_buf, p->payload, LWIP_MIN(p->len, sizeof(udp_reply_buf)-1));

    ip_addr_set(&pc_ip, addr);
    bbb_port = port;

    udp_reply_ready = 1;

    pbuf_free(p);
}

void Servo_SetAngle(uint8_t angle)
{
    uint16_t pulse;

    pulse = 1000 + ((angle * 1000) / 180); // 1000–2000 us

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
