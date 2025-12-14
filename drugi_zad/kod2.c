#include "stm32c0xx_hal.h"
#include <stdint.h>

void SystemClock_Config(void);
void MX_GPIO_Init(void);

typedef struct {
    uint8_t  device_id;
    uint16_t adc_value;
    uint8_t  led_status;     
} CAN_Message;

volatile CAN_Message can_buffer;

ADC_HandleTypeDef hadc1;

uint32_t last_tick = 0;
uint32_t last_led_tick = 0;
uint32_t last_ack_tick = 0;
uint8_t id = 0;
uint16_t adc = 0;
uint8_t led = 0;
uint8_t new_message = 0;

uint16_t adc_value = 0;
uint8_t led_state = 0;

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC_Init();

  for (;;) {
    Transmitter();
    Receiver();
  }
}

void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void MX_ADC_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1; 
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;          
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;          
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;          
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;     
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;        
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;  
  hadc1.Init.DMAContinuousRequests = DISABLE;         
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;

  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
      return;
  }

  sConfig.Channel = ADC_CHANNEL_0;          
  sConfig.Rank = ADC_REGULAR_RANK_1;        
  sConfig.SamplingTime = ADC_SAMPLETIME_12CYCLES_5;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
      return;
  }
}

void Transmitter(void) {
  if (HAL_GetTick() - last_tick >= 500) {
        HAL_ADC_Start(&hadc1);                     
        HAL_ADC_PollForConversion(&hadc1, 10);     
        adc_value = HAL_ADC_GetValue(&hadc1); 
        HAL_ADC_Stop(&hadc1);                       

        led_state = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET) ? 1 : 0;
        new_message = 1;

        can_buffer.device_id = 1;
        can_buffer.adc_value = adc_value;
        can_buffer.led_status = led_state;

        last_tick = HAL_GetTick();
    }
}

void Receiver(void) {

  if (new_message == 1) {
      id = can_buffer.device_id;
      adc = can_buffer.adc_value;
      led = can_buffer.led_status;

      if (adc > 2000) {
          printf("High ADC value from Node 1!\n");
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
      }
      else {
          printf("ID %u: ADC=%u, LED=%u\n", id, adc, led);
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
      }

      new_message = 0;
  }

    if (HAL_GetTick() - last_ack_tick >= 1000) {
        printf("ACK sent\n");
        last_ack_tick = HAL_GetTick();
    }
}
