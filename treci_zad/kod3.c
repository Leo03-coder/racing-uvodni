#include "stm32c0xx_hal.h"
#include <stdio.h>

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_ADC_Init(void);
uint16_t Read_ADC_Channel(uint8_t channel);
void Error_Handler(void);

ADC_HandleTypeDef hadc1;
uint16_t apps1_adc = 0;
uint16_t apps2_adc = 0;
uint16_t apps1_prc = 0;
uint16_t apps2_prc = 0;
uint16_t diff = 0;
uint32_t implaus_start = 0; 
uint8_t implaus_active = 0;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC_Init();
    
    printf("Potentiometer Reader Started\r\n");
    printf("A0    | A1     | diff\r\n");
    printf("------------\r\n");

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    for (;;)
    {
      apps1_adc = Read_ADC_Channel(0);  
      apps2_adc = Read_ADC_Channel(1);  
      apps1_prc = apps1_adc * 100 / 4095;
      apps2_prc = apps2_adc * 100 / 4095;
      
      diff = abs(apps1_prc - apps2_prc);
      
      printf("%-5u | %-5u |%-5u\r\n", apps1_prc, apps2_prc, diff);

      if(diff > 10) {
        if(!implaus_active) {
            implaus_start = HAL_GetTick();
            implaus_active = 1;
        }
        else if(HAL_GetTick() - implaus_start >= 3000) {
            Error_Handler();
        }
      }
      else {
          implaus_active = 0;
      }
        
      HAL_Delay(100);
    }
}

void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void MX_ADC_Init(void)
{
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;      
    hadc1.Init.ContinuousConvMode = DISABLE;         
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;                  
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;   

    if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();
}

uint16_t Read_ADC_Channel(uint8_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    sConfig.Channel = channel;           
    sConfig.Rank = ADC_REGULAR_RANK_1;   
    sConfig.SamplingTime = ADC_SAMPLETIME_12CYCLES_5;
    
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) 
    {
        printf("ADC Config failed for channel %d\r\n", channel);
        return 0;
    }
    
    HAL_ADC_Start(&hadc1);
    
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        uint16_t value = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        return value;
    }
    
    HAL_ADC_Stop(&hadc1);
    return 0;
}

void Error_Handler(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    while(1);
}