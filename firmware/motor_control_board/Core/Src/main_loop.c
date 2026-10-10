
#include "main.h"
#include "motor_control.h"



void main_loop(){

	Motor_Init();

	while(1){

		/* USER CODE BEGIN 3 */
		HAL_Delay(5000);


		Motor_Set_PWM(MOTOR_ID_M1, 2000);
		HAL_Delay(3000);
		Motor_Stop(0);

		Motor_Set_PWM(MOTOR_ID_M2, 2000);
		HAL_Delay(3000);
		Motor_Stop(0);

		Motor_Set_PWM(MOTOR_ID_M3, 2000);
		HAL_Delay(3000);
		Motor_Stop(0);

		Motor_Set_PWM(MOTOR_ID_M4, 2000);
		HAL_Delay(3000);
		Motor_Stop(0);


	}
}
