/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#define DEBOUNCE_TIME 300



/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint32_t left_toggles = 0;
uint32_t right_toggles = 0,hazard_toggles=0;
uint32_t last_debounce_time_left = 0;
uint32_t last_debounce_time_right = 0,last_debounce_time_hazard = 0;
uint32_t counter_right=0, counter_left=0,counter_hazard=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);
  uint32_t current_time = HAL_GetTick();

  //se condicionan las acciones a realizar si se realizan las interrupciones en cada boton

  if (GPIO_Pin == S1_Pin){

	  // se usa un Debounce_time para evitar le ruido por rebote. es decir despues de un puslo se espera el tiempo de debounce time para poder contar otra pulsasion

	  if (current_time - last_debounce_time_left >= DEBOUNCE_TIME) {

		  // Tiempo de reset de 1 segundo, exeptuando el reset despues de dos pulsasiones.
          if (current_time - last_debounce_time_left > 1000 && counter_left < 2) {
              counter_left = 0;
          }

// se inicia el contador de pulsos
		  counter_left++;
		  last_debounce_time_left = current_time;


//se asegura apagar la otra lapara cuando esat se encienda.
		  if(counter_left>0){
		 			  right_toggles=0;
		 		  }
// se condiciona el contador para hacer la accion segun la cantidad de pulsos
		  if(counter_left==1){

        	  HAL_UART_Transmit(&huart2, "S1\r\n", 4, 10);
        	  left_toggles = 6;
          }
          else if(counter_left==2){
        	  HAL_UART_Transmit(&huart2, "S1_toggles\r\n",12, 10);
        	  left_toggles = 0xEEEEEEE;  // Contador muy grande, hace que haya un parpadeo practicamente infito.



          }
          else if (counter_left>=3){  // si hay una tercera pulsasion se resetean los contadores (se apaga el led)
        	  HAL_UART_Transmit(&huart2, "S1_off\r\n",8, 10);
        	  counter_left=0;
        	  left_toggles = 0;



          }


     }

  // De la misma forma se hace para el boton S1
  } else if (GPIO_Pin == S2_Pin){
	  if (current_time - last_debounce_time_right >= DEBOUNCE_TIME) {


          if(current_time - last_debounce_time_right > 1000 && counter_right<2){
        	  counter_right=0;
          }
	       counter_right++;
	       last_debounce_time_right = current_time;

			  if(counter_right>0){
			 			  left_toggles=0;
			 		  }



	      if(counter_right==1){

	          HAL_UART_Transmit(&huart2, "S2\r\n",4, 10);
	          right_toggles = 6;



	          }

	          else if(counter_right==2){

	        	  HAL_UART_Transmit(&huart2, "S2_toggles\r\n", 12, 10);
	              right_toggles = 0xEEEEEEE;



	          }
	          else if(counter_right>=3){

	        	  HAL_UART_Transmit(&huart2, "S2_off\r\n", 8, 10);
	              right_toggles = 0;
	         	  counter_right = 0;


	          }


	          }

	  }else if(GPIO_Pin==S3_Pin){// El Boton S3 es para la señal de parqueo

		  if (current_time - last_debounce_time_right >= DEBOUNCE_TIME) {  // Evitar rebote


		       counter_hazard++;
		       last_debounce_time_right = current_time;

				  if(counter_hazard>0){ // si se activan las estacionarias, se apagan las direccionales
					  left_toggles = 0;
					  right_toggles = 0;
				  }


		      if(counter_hazard==1){ //se pone una bandera Hazadr_toggles si hay una pulsacion.

		          HAL_UART_Transmit(&huart2, "S3_on\r\n",7, 10);

		          hazard_toggles=1;



		          }
//
		          else if(counter_hazard>=2){ // se pone en cero la bandera y el resto de contadores(se apagan las dos luces)

		        	  HAL_UART_Transmit(&huart2, "S3_off\r\n", 8, 10);
		              right_toggles = 0;
		              left_toggles = 0;
		         	  counter_hazard = 0;
		         	  hazard_toggles=0;



		          }


		          }




	  }

  }


  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_GPIO_EXTI_Callback could be implemented in the user file
   */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Funcionde parpadeo para comprobar el funcionamiento.
void heartbeat (void){

	static uint32_t heartbeat_tick = 0;
	if(heartbeat_tick  < HAL_GetTick() ){
		heartbeat_tick = HAL_GetTick() + 500;
		HAL_GPIO_TogglePin(D1_GPIO_Port, D1_Pin);
	}
}
/* USER CODE END 0 */

//Funcionde control para la direccional izquierda

void turn_signal_left (void){
	static uint32_t tunr_togle_tick = 0;
	if(tunr_togle_tick  < HAL_GetTick() ){

		if(left_toggles> 0){// si el contador es mayor que cero se ejecuta la accion
			tunr_togle_tick = HAL_GetTick() + 300;  // tiempo actual tyransucrrido mas 300 ms
			HAL_GPIO_TogglePin(D3_GPIO_Port, D3_Pin);  // se hace un cambio de estado del pin del LED cada 300ms
			left_toggles--; // se resta 1 cada por cada ciclo.(este condtador decide cuatas veces se rquiere el parpadeo)
		} else{

			HAL_GPIO_WritePin(D3_GPIO_Port, D3_Pin, 1);  // si no se activa la direccional, que permanezca apagada
		}

	}
}


//Funcionde control para la direccional derecha. (Funciona igual que la anterior)
void turn_signal_right (void){

	static uint32_t tunr_togle_tick = 0;
	if(tunr_togle_tick  < HAL_GetTick() ){
		if(right_toggles> 0){
			tunr_togle_tick = HAL_GetTick() + 300;
			HAL_GPIO_TogglePin(D4_GPIO_Port, D4_Pin);
			right_toggles--;

		} else{
			HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, 1);

		}

	}
}




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  HAL_Init();

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //Llamado a funciones
	  heartbeat();
	  turn_signal_left();
	  turn_signal_right();

	  //control de señal de parqueo
	  if(counter_hazard==1){
		  HAL_GPIO_WritePin(D3_GPIO_Port, D3_Pin, 1);
		  HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, 1);
		  HAL_Delay(300);
		  HAL_GPIO_WritePin(D3_GPIO_Port, D3_Pin, 0);
		  HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, 0);
		  HAL_Delay(300);

		  //si la bandera counter_hazard esta activa se realiza un parapdeo infinito con el uso del Delay. Esto hace que las luces de parqueo tengan la maxima prioridad.


	  }




	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */


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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, D1_Pin|D3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : S1_Pin S2_Pin */
  GPIO_InitStruct.Pin = S1_Pin|S2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : D1_Pin D3_Pin */
  GPIO_InitStruct.Pin = D1_Pin|D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : S3_Pin */
  GPIO_InitStruct.Pin = S3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(S3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : D4_Pin */
  GPIO_InitStruct.Pin = D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(D4_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
