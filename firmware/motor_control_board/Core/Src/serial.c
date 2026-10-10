/*
 * serial.c
 *
 *  Created on: May 10, 2026
 *      Author: acantu
 */
#include "serial.h"

#define RX_BUFFER_LEN 512

UART_HandleTypeDef *huart;


uint8_t rx_buffer[RX_BUFFER_LEN];
uint16_t rx_idx = 0;
int count = 0;

int serial_init(UART_HandleTypeDef *_huart){

	huart = _huart;


	HAL_UARTEx_ReceiveToIdle_DMA(huart, rx_buffer, RX_BUFFER_LEN);

	return 0;
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    rx_idx = Size;   // Cumulative position in the circular DMA buffer
    count++;       // How many IDLE events fired (equals number of chunks from sender)
}