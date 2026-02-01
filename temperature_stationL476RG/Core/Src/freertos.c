/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "spi.h"

#include "temperature_sensor.h"
#include "heater_control.h"
#include "St7735s.h"
#include "fan_control.h"
#include "pid_lookup.h"
#include "PID_controller.h"
#include "uart_protocol.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
TempSensorHandle_t tempSensors;
HeaterHandle_t heater;
FanHandle_t fan;
ST7735_Handle_t lcd;
UartProtocol_t protocol;

uint16_t adc_buf_temp[MAX_TEMP_SENSORS];

volatile float g_temperature = 0;
volatile float g_setpoint = 35.0f;
volatile uint16_t g_heater_pwm = 0;
volatile uint16_t g_fan_pwm = 0;
volatile uint32_t g_timestamp = 0;
volatile uint8_t g_data_ready = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for tcp_com */
osThreadId_t tcp_comHandle;
const osThreadAttr_t tcp_com_attributes = {
  .name = "tcp_com",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTask03 */
osThreadId_t myTask03Handle;
const osThreadAttr_t myTask03_attributes = {
  .name = "myTask03",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for temp_meas */
osTimerId_t temp_measHandle;
const osTimerAttr_t temp_meas_attributes = {
  .name = "temp_meas"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void start_usart_com(void *argument);
void LcdTask(void *argument);
void temp_callback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  TempSensor_Init(&tempSensors, &hadc1, MAX_TEMP_SENSORS, adc_buf_temp, 3.3f);
  Heater_Init(&heater, &htim1, TIM_CHANNEL_1);
  Fan_Init(&fan, &htim1, TIM_CHANNEL_2);
  ST7735_Init(&lcd, &hspi2);
  UartProtocol_Init(&protocol, &huart2);

  Heater_Start(&heater);
  TempSensor_Start(&tempSensors);
  Fan_Start(&fan);
  Temperature_Control_Init();
  Temperature_Control_SetTarget(35.0f);
  Temperature_Control_SetFanPWM(299);

  Heater_SetPower(&heater, 0);
  Fan_SetSpeed(&fan, 299);
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of temp_meas */
  temp_measHandle = osTimerNew(temp_callback, osTimerPeriodic, NULL, &temp_meas_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  osTimerStart(temp_measHandle, 1000);
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of tcp_com */
  tcp_comHandle = osThreadNew(start_usart_com, NULL, &tcp_com_attributes);

  /* creation of myTask03 */
  myTask03Handle = osThreadNew(LcdTask, NULL, &myTask03_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_start_usart_com */
/**
* @brief Function implementing the tcp_com thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_start_usart_com */
void start_usart_com(void *argument)
{
  /* USER CODE BEGIN start_usart_com */
  uint8_t rx_byte;
  /* Infinite loop */
  for(;;)
  {
	  if (HAL_UART_Receive(&huart2, &rx_byte, 1, 10) == HAL_OK) {
	      UartProtocol_ReceiveByte(&protocol, rx_byte);
	  }

	          UartCommand_t cmd = UartProtocol_Parse(&protocol);
	  if (cmd == CMD_SET_TEMP) {
	      float new_setpoint = UartProtocol_GetArg(&protocol);
	      Temperature_Control_SetTarget(new_setpoint);
	  }

	   if (g_data_ready) {
	   UartProtocol_SendData(&protocol, g_temperature, g_setpoint,
	                         g_heater_pwm, g_fan_pwm, g_timestamp);
	   g_data_ready = 0;
	  }

	  osDelay(50);
  }
  /* USER CODE END start_usart_com */
}

/* USER CODE BEGIN Header_LcdTask */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LcdTask */
void LcdTask(void *argument)
{
  /* USER CODE BEGIN LcdTask */
  /* Infinite loop */
  for(;;)
  {
   ST7735_ShowTemperature(&lcd,
	                      g_temperature,
	                      g_setpoint,
	                      g_heater_pwm,
	                      g_fan_pwm);
   osDelay(500);
  }
  /* USER CODE END LcdTask */
}

/* temp_callback function */
void temp_callback(void *argument)
{
  /* USER CODE BEGIN temp_callback */
  float t_avg = TempSensor_GetAverageTemperature(&tempSensors);
  Temperature_Control_Update(t_avg);


  uint16_t h_pwm = Temperature_Control_GetHeaterPWM();
  uint16_t f_pwm = Temperature_Control_GetFanPWM();
  Heater_SetPower(&heater, h_pwm);
  Fan_SetSpeed(&fan, f_pwm);

  g_temperature = t_avg;
  g_setpoint = Temperature_Control_GetSetpoint();
  g_heater_pwm = h_pwm;
  g_fan_pwm = f_pwm;
  g_timestamp += 500;
  g_data_ready = 1;



  /* USER CODE END temp_callback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

