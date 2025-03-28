/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "stdio.h"
#include "string.h"
#include <stdlib.h>

#define RXBUFFERSIZE  256     //最大接收字节数
#define MAX_TOKENS 5          //解析字符串中有几个部分

char RxBuffer[RXBUFFERSIZE];  //接收数据
uint8_t aRxBuffer;			      //接收中断缓冲
uint8_t Uart1_Rx_Cnt = 0;     //接收缓冲计数
/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
int parse_and_store(char *input, char *tokens[], int max_tokens) 
{
  int i = 0;
  char *token = strtok(input, "|");
  while (token != NULL && i < max_tokens) {
    tokens[i++] = token;
    token = strtok(NULL, "|");
  }
  return i; // 返回解析到的标记数量
}

void extract_value(char *input, char *output) {
  char *token = strtok(input, "-");
  if (token != NULL) {
    token = strtok(NULL, "-");
    if (token != NULL) {
      strcpy(output, token);
    }
  }
}

void enable_led_encoder() {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
}

void disable_led_encoder() {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}

void play_dlp_trigger(int cnt, int hd, int ld) {
  disable_led_encoder();
  for (int i = 0; i < cnt; i++) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);   // trig dlp
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);  // trig cam
    HAL_Delay(hd);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
    HAL_Delay(ld);
  }
}

void set_led_state(int id) {
  // PA3 to PA7 对应的 GPIO 引脚
  uint16_t pins[] = {GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};
  int num_pins = sizeof(pins) / sizeof(pins[0]);

  // 将 id 转换为二进制，并设置相应的 GPIO 引脚状态
  for (int i = 0; i < num_pins; i++) {
      if (id & (1 << i)) {
          HAL_GPIO_WritePin(GPIOA, pins[i], GPIO_PIN_SET); // 亮
      } else {
          HAL_GPIO_WritePin(GPIOA, pins[i], GPIO_PIN_RESET); // 暗
      }
  }
}

void play_led_trigger(int cnt, int hd, int id) {
  set_led_state(id);
  enable_led_encoder();
  HAL_Delay(hd);
  disable_led_encoder();
}

void enable_indicator_light(){
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // 亮
}

void disable_indicator_light(){
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // 亮
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_UART_TxCpltCallback could be implemented in the user file
   */
 
	if(Uart1_Rx_Cnt >= 255)  //溢出判断
	{
		Uart1_Rx_Cnt = 0;
		memset(RxBuffer,0x00,sizeof(RxBuffer));
    char *err = "Data Overflow!";
    HAL_UART_Transmit(&huart1, (uint8_t *)err, strlen(err), 0xffff);
	}
	else
	{
		RxBuffer[Uart1_Rx_Cnt++] = aRxBuffer;  
		if((RxBuffer[Uart1_Rx_Cnt-1] == 0x0A)&&(RxBuffer[Uart1_Rx_Cnt-2] == 0x0D))  //判断结束位
		{
			// HAL_UART_Transmit(&huart1, (uint8_t *)&RxBuffer, Uart1_Rx_Cnt, 0xFFFF);   //将收到的信息发送出去
      // while(HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX);              //检测UART发送结束
      
      // 解析字符串 "trig-DLP|cnt-12|hd-100|ld-100|id-0"
      char *tokens[MAX_TOKENS];
      int token_count = parse_and_store(RxBuffer, tokens, MAX_TOKENS);

      // 检查是否有且仅有 4 个竖线分隔符
      if (token_count == MAX_TOKENS) {
        char *values[MAX_TOKENS];
        for (int i = 0; i < MAX_TOKENS; i++) {
          values[i] = (char *)malloc(10);
          extract_value(tokens[i], values[i]);
        }
        // HAL_UART_Transmit(&huart1, (uint8_t *)values[1], strlen(values[1]), 0xffff);
        // HAL_UART_Transmit(&huart1, (uint8_t *)values[2], strlen(values[2]), 0xffff);
        // HAL_UART_Transmit(&huart1, (uint8_t *)values[3], strlen(values[3]), 0xffff);
        // HAL_UART_Transmit(&huart1, (uint8_t *)values[4], strlen(values[4]), 0xffff);
        int cnt = atoi(values[1]);
        int hd = atoi(values[2]);
        int ld = atoi(values[3]);
        int id = atoi(values[4]);
        if (strcmp(values[0], "DLP") == 0) {
          enable_indicator_light();
          play_dlp_trigger(cnt, hd, ld);
          char *err = "Trig DLP Success!\r\n";
          HAL_UART_Transmit(&huart1, (uint8_t *)err, strlen(err), 0xffff);
          disable_indicator_light();
        }
        else if (strcmp(values[0], "LED") == 0)
        {
          if (id < 1 || id > 32) {
            char *err = "Invalid LED ID!\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t *)err, strlen(err), 0xffff);
          } else {
            enable_indicator_light();
            play_led_trigger(cnt, hd, id);
            char *err = "Trig LED Success!\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t *)err, strlen(err), 0xffff);
            disable_indicator_light();
          }
        }else{
          char *err = "Invalid command!\r\n";
          HAL_UART_Transmit(&huart1, (uint8_t *)err, strlen(err), 0xffff);
        }
        
      } else {
        // 处理错误情况
        char *err = "Invalid format!\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)err, strlen(err), 0xffff);
      }
      Uart1_Rx_Cnt = 0;
			memset(RxBuffer,0x00,sizeof(RxBuffer));                                    //清空数组
		}
	}
	
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&aRxBuffer, 1);   //因为接收中断使用了一次即关闭，所以在最后加入这行代码即可实现无限使用
}
/* USER CODE END 1 */
