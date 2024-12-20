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
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "ds3231.h"
#include "uart.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void test_Uart();
void ButtonHandle();
void updateTime();
int displayTimeOut();
void displayMode();
void displayTime();
void displayTimeMode1();
void displayTimeMode2();
void checkTimeOut();
void button_Hour();
void update(int, int);
void updateUart();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t command_data[MAX_BUFFER_SIZE];
struct dateTime {
	int x_axis;
	int y_axis;
	uint8_t *showName;
	uint16_t  fontColor;
	uint16_t  bgColor;
	uint8_t writeName;
	int countDown;
	uint8_t titleName[100];
};

struct dateTime dateTimeInfo[7] = {
		{70, 100, &ds3231_hours, GREEN, BLACK, ADDRESS_HOUR, 0, "HOUR"},
		{110, 100, &ds3231_min, GREEN, BLACK, ADDRESS_MIN, 0, "MIN"},
		{150, 100, &ds3231_sec, GREEN, BLACK, ADDRESS_SEC, 0, "SEC"},
		{20, 130, &ds3231_day, YELLOW, BLACK, ADDRESS_DAY, 0, "DAY"},
		{70, 130, &ds3231_date, YELLOW, BLACK, ADDRESS_DATE, 0, "DATE"},
		{110, 130, &ds3231_month, YELLOW, BLACK, ADDRESS_MONTH, 0, "MONTH"},
		{150, 130, &ds3231_year, YELLOW, BLACK, ADDRESS_YEAR, 0, "YEAR"},
};

int mode = 0; // 0: view date and time; 1: edit date and time; 2: countdown

int editIndex = 0;
int editBlink = 0;
int tempValue = 0;
int countdownIndex = 0;
uint8_t uartReadIndex = 0;
int flag_button = 0;
uint8_t tempStr[100];
int hand_mode = 0;
int timeOut = 0;
int isSendUartSignal = 0;
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
  MX_TIM2_Init();
  MX_SPI1_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  system_init();
  updateTime();
  /* USER CODE END 2 */
uint16_t index = 0;
int count = 0;
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  while(!flag_timer[1]);
	  flag_timer[1] = 0;
	  button_Scan();
	  ButtonHandle();
	  button_Hour();
	  switch(mode){
	  case 0:
		  ds3231_ReadTime();
		  if(!timeOut) checkTimeOut();
		  if(timeOut == 1) {
  //			  lcd_Clear(WHITE);
			  lcd_StrCenter(0, 2, "ALARM!!!", RED, BLUE, 16, 1);
			continue;
		  }
		  displayMode();
		  displayTime();
		  break;
	  case 1:
		  displayMode();
		  displayTimeMode1();
		  break;

	  case 2:
		  displayMode();
		  displayTimeMode2();
		  break;
	  default:
		  break;
	  }
	  updateUart();

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void system_init(){
	  HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
	  HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
	  HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);
	  timer_init();
	  led7_init();
	  button_init();
	  lcd_init();
	  uart_init_rs232();
	  setTimer(50, 1);
}

uint16_t count_led_debug = 0;

void test_LedDebug(){
	count_led_debug = (count_led_debug + 1)%20;
	if(count_led_debug == 0){
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	}
}

void test_button(){
	for(int i = 0; i < 16; i++){
		if(button_count[i] == 1){
			led7_SetDigit(i/10, 2, 0);
			led7_SetDigit(i%10, 3, 0);
		}
	}
}
void updateTime(){
	ds3231_Write(ADDRESS_YEAR, 24);
	ds3231_Write(ADDRESS_MONTH, 11);
	ds3231_Write(ADDRESS_DATE, 13);
	ds3231_Write(ADDRESS_DAY, 4);
	ds3231_Write(ADDRESS_HOUR, 9);
	ds3231_Write(ADDRESS_MIN, 27);
	ds3231_Write(ADDRESS_SEC, 23);
}

void test_Uart(){
	if(button_count[12] == 1){
		uart_Rs232SendNum(ds3231_hours);
		uart_Rs232SendString(":");
		uart_Rs232SendNum(ds3231_min);
		uart_Rs232SendString(":");
		uart_Rs232SendNum(ds3231_sec);
		uart_Rs232SendString("\n");
	}
}

void button_Hour(){
	if (button_count[4] == 1){
		hand_mode = 1;
		mode = 5;
		lcd_Clear(BLACK);
		lcd_ShowStr(20, 30, "Mode: ", WHITE, RED, 24, 0);
		lcd_ShowIntNum(150, 30, 3, 1, GREEN, BLACK, 24);
	}
}

void displayMode () {
	 lcd_ShowStr(20, 30, "Mode: ", WHITE, RED, 24, 0);
	 lcd_ShowIntNum(150, 30, mode, 1, GREEN, BLACK, 24);
}

void displayTime(){
	for (int i = 0; i < 7; i ++) {
		lcd_ShowIntNum (dateTimeInfo[i].x_axis, dateTimeInfo[i].y_axis,
					*(dateTimeInfo[i].showName), 2, dateTimeInfo[i].fontColor,
					dateTimeInfo[i].bgColor, 24);
	}
}

int countMode1 = 0;
int temp_value[7] = {0};
int save_time[7] = {0};
void displayTimeMode1(){
//	lcd_Clear(BLACK);
	countMode1 = (countMode1 + 1) % 5;
	if(countMode1 == 0){
		lcd_ShowIntNum (dateTimeInfo[editIndex].x_axis, dateTimeInfo[editIndex].y_axis,
								temp_value[editIndex], 2, dateTimeInfo[editIndex].fontColor,
								dateTimeInfo[editIndex].bgColor, 24);
	}
	else {
		lcd_ShowIntNum (dateTimeInfo[editIndex].x_axis, dateTimeInfo[editIndex].y_axis,
								*(dateTimeInfo[editIndex].showName), 2, BLACK,
								BLACK, 24);
	}
}

void displayTimeMode2(){
//	lcd_Clear(BLACK);
	countMode1 = (countMode1 + 1) % 5;
	if(countMode1 == 0){
		lcd_ShowIntNum (dateTimeInfo[editIndex].x_axis, dateTimeInfo[editIndex].y_axis,
										temp_value[editIndex], 2, dateTimeInfo[editIndex].fontColor,
										dateTimeInfo[editIndex].bgColor, 24);
	}
	else {
		lcd_ShowIntNum (dateTimeInfo[editIndex].x_axis, dateTimeInfo[editIndex].y_axis,
										*(dateTimeInfo[editIndex].showName), 2, BLACK,
										BLACK, 24);
	}
}

int logic_handle(int index, int value){
	switch(index){
	case 0:
		if(value > 23) return 0;
		break;
	case 1:
		if(value > 59) return 0;
		break;
	case 2:
		if(value > 59) return 0;
		break;
	case 3:
		if(value > 8) return 2;
		break;
	case 4:
		if(value > 31) return 1;
		break;
	case 5:
		if(value > 12) return 1;
		break;
	case 6:
		if(value > 99) return 0;
		break;
	}
	return value;
}

int logic_handle1(int index, int value){
	switch(index){
	case 0:
		if(value > 23) return 1;
		break;
	case 1:
		if(value > 59) return 1;
		break;
	case 2:
		if(value > 59) return 1;
		break;
	case 3:
		if(value > 8) return 1;
		break;
	case 4:
		if(value > 31) return 1;
		break;
	case 5:
		if(value > 12) return 1;
		break;
	case 6:
		if(value > 99) return 1;
		break;
	}
	return 0;
}

void update(int index, int value){
	switch(index){
	case 0:
		ds3231_Write(ADDRESS_HOUR, value);
		break;
	case 1:
		ds3231_Write(ADDRESS_MIN, value);
		break;
	case 2:
		ds3231_Write(ADDRESS_SEC, value);
		break;
	case 3:
		ds3231_Write(ADDRESS_DAY, value);
		break;
	case 4:
		ds3231_Write(ADDRESS_DATE, value);
		break;
	case 5:
		ds3231_Write(ADDRESS_MONTH, value);
		break;
	case 6:
		ds3231_Write(ADDRESS_YEAR, value);
		break;
	}
}

void ButtonHandle() {
    if (button_count[0] == 1) {
    	mode = (mode + 1) % 3;
    	lcd_Clear(BLACK);
    	if(mode == 1){
    	        editIndex = 0;
    	        temp_value[editIndex] = *(dateTimeInfo[editIndex].showName);
    	        lcd_Clear(BLACK);
    	     }
	 if(mode == 2){
		 lcd_Clear(BLACK);
		editIndex = 0;
		temp_value[editIndex] = *(dateTimeInfo[editIndex].showName);
		for (int i = 0; i < 7; i++){
			save_time[i] = *(dateTimeInfo[i].showName);
		}
	 }
    }
    if(button_count[1] == 1){
    	mode = 0;
    	timeOut = 0;
    	lcd_Clear(BLACK);
    }


    switch(mode){
    case 0:
    	break;
    case 1:
    	if(button_count[11] == 1){//change index;
    		lcd_Clear(BLACK);
    		editIndex = (editIndex + 1) % 7;
    		temp_value[editIndex] = *(dateTimeInfo[editIndex].showName);
    	}
    	if(button_count[3] % 4 == 1){ //increase value
    		temp_value[editIndex] = (temp_value[editIndex] + 1);
    		    		temp_value[editIndex] = logic_handle(editIndex, temp_value[editIndex]);
    	}
    	if(button_count[12] == 1){ //save
    		lcd_Clear(BLACK);
    		update(editIndex, temp_value[editIndex]);
    		editIndex = (editIndex + 1) % 7;
    		temp_value[editIndex] = *(dateTimeInfo[editIndex].showName);
    	}
    	break;
    case 2:
    	if(button_count[11] == 1){//change index;
    		lcd_Clear(BLACK);
    	    editIndex = (editIndex + 1) % 7;
    	    temp_value[editIndex] = *(dateTimeInfo[editIndex].showName);
    	   }
    	if(button_count[3] % 4 == 1){ //increase value
    		temp_value[editIndex] = (temp_value[editIndex] + 1);
    		    		temp_value[editIndex] = logic_handle(editIndex, temp_value[editIndex]);
		}
		if(button_count[12] == 1){ //save
			lcd_Clear(BLACK);
			save_time[editIndex] = temp_value[editIndex];
			editIndex = (editIndex + 1) % 7;
			temp_value[editIndex] = *(dateTimeInfo[editIndex].showName);
		}
    	break;
    }
}

void checkTimeOut(){
	int count_temp = 0;
	for (int i = 0; i < 7; i++){
		if(save_time[i] == *(dateTimeInfo[i].showName)) count_temp++;
	}
	if(count_temp == 7) {
		lcd_Clear(BLACK);
		timeOut = 1;
	}
	else timeOut = 0;
}

int sent = 0;
int hand_state = 0;
int count_10s = 0;
int count3 = 0;
void updateUart(){
	if(hand_mode == 1){
		ds3231_ReadTime();
		displayTime();
		count_10s = (count_10s + 1) % 200;
		if(count_10s == 0){
			count3++;
			sent = 0;
			if(count3 >= 3){
				lcd_Clear(BLACK);
				lcd_ShowStr(100, 200, "ERRORS", RED, BLACK, 30, 1);
				hand_mode = 0;
				hand_state = 0;
				sent = 0;
				count3 = 0;
				mode = 0;
				return;
			}
		}
			if (!sent) {
				 sent = 1;
				 switch(hand_state){
						case 0:
							uart_Rs232SendString("Hours\n");
							lcd_ShowStr(20, 60, "Updating Hours", WHITE, RED, 24, 0);
							break;
						case 1:
							uart_Rs232SendString("Minutes\n");
							lcd_ShowStr(20, 60, "Updating Minutes", WHITE, RED, 24, 0);
							break;
						case 2:
							uart_Rs232SendString("Seconds\n");
							lcd_ShowStr(20, 60, "Updating Seconds", WHITE, RED, 24, 0);
							break;
						case 3:
							uart_Rs232SendString("Date\n");
							lcd_ShowStr(20, 60, "Updating Date", WHITE, RED, 24, 0);
							break;
						case 4:
							uart_Rs232SendString("Day\n");
							lcd_ShowStr(20, 60, "Updating Day", WHITE, RED, 24, 0);
							break;
						case 5:
							uart_Rs232SendString("Month\n");
							lcd_ShowStr(20, 60, "Updating Month", WHITE, RED, 24, 0);
							break;
						case 6:
							uart_Rs232SendString("Year\n");
							lcd_ShowStr(20, 60, "Updating Year", WHITE, RED, 24, 0);
							break;
						default:
							break;
						}
			 }
		}

		if(buffer_flag == 1){
			memcpy(command_data, buffer, sizeof(command_data));
			if(index_buffer == 1){
				if(command_data[0] >='0' && command_data[0] <= '9'){
					int temp1 = command_data[0] - 48;
					if(logic_handle1(hand_state, temp1)){
						uart_Rs232SendString("ERROR Input\n");
						buffer_flag = 0;
									index_buffer = 0;
									out_buffer = 0;
									sent = 0;
						return;
					}
					else{
						temp1 = logic_handle(hand_state, temp1);
						update(hand_state, temp1);
					}

				}
				else {
					uart_Rs232SendString("ERROR Input\n");
					buffer_flag = 0;
								index_buffer = 0;
								out_buffer = 0;
								sent = 0;
					return;
				}

			}
			else {
				if(command_data[0] >= '0' && command_data[0] <= '9' && command_data[1] >= '0' && command_data[1] <= '9'){
					int temp1 = (command_data[0]-48)*10 + command_data[1] -48;
					if(logic_handle1(hand_state, temp1)){
						uart_Rs232SendString("ERROR Input\n");
						buffer_flag = 0;
									index_buffer = 0;
									out_buffer = 0;
									sent = 0;
						return;
					}
					else{
						temp1 = logic_handle(hand_state, temp1);
						update(hand_state, temp1);
					}
				}
				else {
					uart_Rs232SendString("ERROR Input\n");
					buffer_flag = 0;
								index_buffer = 0;
								out_buffer = 0;
								sent = 0;
					return;
				}
			}
			count3 = 0;
			buffer_flag = 0;
			index_buffer = 0;
			out_buffer = 0;
			sent = 0;
			if(hand_state == 6) {
				hand_mode = 0;
				mode = 0;
			}
			hand_state = (hand_state + 1) % 7;
			lcd_ShowStr(20, 60, "Updating AAAAAAAAAAA", BLACK, BLACK, 24, 0);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
