/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
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
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2; // Aggiunto per Peltier
TIM_HandleTypeDef htim3; // Dichiarato una sola volta

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */
// Rimosse le doppie dichiarazioni qui
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void); // Aggiunto prototipo Peltier

/* USER CODE BEGIN PFP */
void Buzzer_Beep(uint32_t frequency_hz, uint32_t duration_ms);
void Control_Peltier_Voltage(float percentage);
void Control_Fans(float fan1_pct, float fan2_pct);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief Esegue un beep singolo con frequenza variabile
  */
void Buzzer_Beep(uint32_t frequency_hz, uint32_t duration_ms)
{
    uint32_t timer_clock = 16000000;
    uint32_t arr = (timer_clock / frequency_hz) - 1;

    __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, arr / 2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_Delay(duration_ms);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM2_Init(); // Inizializzazione Peltier
  MX_TIM3_Init(); // Inizializzazione Ventole
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */

  // Segnale acustico di avvio (1 secondo a 1000Hz)
  Buzzer_Beep(1000, 1000);

  // Avvia PWM Peltier
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  // Avvia PWM ventole
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  // Test iniziale: ventole al 75% e Peltier al 20%
  Control_Fans(75.0f, 75.0f);
  Control_Peltier_Voltage(20.0f);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief TIM2 Initialization Function (Peltier)
  */
static void MX_TIM2_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 799; // Per avere 20kHz
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim2);
}

// ... (MX_TIM1_Init, MX_TIM3_Init, etc. restano come generati da CubeMX) ...

/* USER CODE BEGIN 4 */

/**
  * @brief Regola la tensione media sulla cella di Peltier (0-100%)
  */
void Control_Peltier_Voltage(float percentage)
{
    if (percentage > 100.0f) percentage = 100.0f;
    if (percentage < 0.0f)   percentage = 0.0f;

    // ARR = 799, quindi risoluzione 800 passi
    uint32_t compare_value = (uint32_t)((percentage * 800.0f) / 100.0f);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, compare_value);
}

/**
  * @brief Regola la velocità delle ventole (0-100%)
  */
void Control_Fans(float fan1_pct, float fan2_pct)
{
    if (fan1_pct > 100.0f) fan1_pct = 100.0f;
    if (fan1_pct < 0.0f)   fan1_pct = 0.0f;
    if (fan2_pct > 100.0f) fan2_pct = 100.0f;
    if (fan2_pct < 0.0f)   fan2_pct = 0.0f;

    // ARR = 999, risoluzione 1000 passi
    uint32_t ccr1 = (uint32_t)((fan1_pct * 1000.0f) / 100.0f);
    uint32_t ccr2 = (uint32_t)((fan2_pct * 1000.0f) / 100.0f);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr1);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ccr2);
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
#ifdef USE_FULL_ASSERT
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
