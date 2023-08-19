/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rtthread.h"
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

/* USER CODE BEGIN PV */

/*定义线程控制*/
static rt_thread_t receive_thread = RT_NULL;
static rt_thread_t send_thread = RT_NULL;
/*定义消息队列控制*/
static rt_mq_t test_mq = RT_NULL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void receive_thread_entry(void *parametr);
static void send_thread_entry(void *parameter);
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  rt_show_version();
  test_mq = rt_mq_create("test_mq",
                         40,
                         20,
                         RT_IPC_FLAG_FIFO);
  if (test_mq!=RT_NULL)
  {
    /* code */
    rt_kprintf("消息队列创建成功！\n\n");
  }
  
  receive_thread =
      rt_thread_create(
          "receive",
          receive_thread_entry,
          RT_NULL,
          512,
          3,
          20);
  if (receive_thread != RT_NULL)
  {
    rt_thread_startup(receive_thread);
  }
  else
  {
    return -1;
  }
  send_thread =
      rt_thread_create(
          "send",
          send_thread_entry,
          RT_NULL,
          512,
          4,
          20);
  if (send_thread != RT_NULL)
  {
    /* code */
    rt_thread_startup(send_thread);
  }
  else
    return -1;

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/*线程定义*/
static void receive_thread_entry(void *parametr)
{
  rt_err_t uwRet = RT_EOK;
  uint32_t r_queue;
  while (1)
  {
    uwRet = rt_mq_recv(test_mq,
                       &r_queue,
                       sizeof(r_queue),
                       RT_WAITING_FOREVER);
    if (RT_EOK == uwRet)
    {
      rt_kprintf("本次接收到的数据是，%d\n", r_queue);
    }
    else
    {
      rt_kprintf("接收数据出错，错误代码：0x%lx\n", uwRet);
    }
    rt_thread_delay(200);
  }
}

static void send_thread_entry(void *parameter)
{
  rt_err_t uwRet = RT_EOK;
  uint32_t send_data1 = 1;
  uint32_t send_data2 = 2;
  while (1)
  {
    if (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == SET) // KEY1
    {
      uwRet = rt_mq_send(test_mq,
                         &send_data1,
                         sizeof(send_data1));
      if (RT_EOK != uwRet)
      {
        rt_kprintf("数据不能发送到消息队列！错误代码：%lx\n", uwRet);
      }
    }
    if (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == SET)
    {
      uwRet = rt_mq_send(test_mq,
                         &send_data2,
                         sizeof(send_data2));
      if (RT_EOK != uwRet)
      {
        rt_kprintf("数据不能发送到消息队列！错误代码：%lx\n", uwRet);
      }
    }
    rt_thread_delay(20);
  }
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
