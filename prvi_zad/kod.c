#include "stm32c0xx_hal.h"

void SystemClock_Config(void);
void MX_GPIO_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    for (;;)
    {
        for (uint8_t value = 0; value < 16; ++value)
        {
            // A0 (LSB) = bit0
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, (value & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            // A1 = bit1
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, (value & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            // A2 = bit2 (PA4)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, (value & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            // A3 (MSB) = bit3 (PB1)
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (value & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

            HAL_Delay(500); // pola sekunde
        }
    }
}

// ------------------------- GPIO Inicijalizacija -------------------------
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Omogućujemo satove za portove A i B
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Konfiguracija PA0, PA1, PA4 kao izlazi
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Konfiguracija PB1 (A3) kao izlaz
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}