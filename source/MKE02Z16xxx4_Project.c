/*
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @file    MKE02Z16xxx4_Project.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKE02Z4.h"

const uint8_t SM_UART_04L_Header[] = {0x42, 0x4D};
//A 32 byte buffer to store Sensor data, ch to store header for comparision
static volatile uint8_t SM_UART_04L_Data[32], ch;
struct SM_UART_04L_Data{
	uint16_t framelen;
	uint16_t pm10_standard, pm25_standard, pm100_standard;
	uint16_t pm10_env, pm25_env, pm100_env;
	uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
	uint16_t unused;
	uint16_t checksum;
};

static volatile struct SM_UART_04L_Data PM_Data;

/*
 *
 *
 *
 * */
void SM_UART_04L_getData(uint8_t *SM_UART_04L_Data, uint8_t ch){
	UART_ReadBlocking(UART0, &ch, 1);
	if (ch == SM_UART_04L_Header[0]){
		UART_ReadBlocking(UART0, &ch, 1);
		if(ch == SM_UART_04L_Header[1]){
			SM_UART_04L_Data[0] = 0x42;
			SM_UART_04L_Data[1] = 0x4D;
			UART_ReadBlocking(UART0, SM_UART_04L_Data+2, 32);
		}
	}
}

/*
 *
 *
 *
 * */
uint8_t SM_UART_04L_testCheckSum(uint8_t *SM_UART_04L_Data, struct SM_UART_04L_Data *PM_Data){
	PM_Data->framelen = SM_UART_04L_Data[2]*256 + SM_UART_04L_Data[3];
	uint16_t csFromData = SM_UART_04L_Data[30]*256 + SM_UART_04L_Data[31];
	uint16_t csCal = 0;
	uint8_t i = 0;
	for(i = 0; i < 32; i++){
		csCal += SM_UART_04L_Data[i];
	}
	if(csCal == csFromData) return 1;
	else return 0;
}

/*
 *
 *
 *
 * */
void SM_UART_04L_dataParse(uint8_t *SM_UART_04L_Data, struct SM_UART_04L_Data *PM_Data){
	PM_Data->pm10_standard = SM_UART_04L_Data[4]*256 + SM_UART_04L_Data[5];
	PM_Data->pm25_standard = SM_UART_04L_Data[6]*256 + SM_UART_04L_Data[7];
	PM_Data->pm100_standard = SM_UART_04L_Data[8]*256 + SM_UART_04L_Data[9];
	PM_Data->pm10_env = SM_UART_04L_Data[10]*256 + SM_UART_04L_Data[11];
	PM_Data->pm25_env = SM_UART_04L_Data[12]*256 + SM_UART_04L_Data[13];
	PM_Data->pm100_env = SM_UART_04L_Data[14]*256 + SM_UART_04L_Data[15];
}

/*
 * @brief   Application entry point.
 */
int main(void) {

	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();


    while(1) {
    	SM_UART_04L_getData(SM_UART_04L_Data, ch);
    	if(SM_UART_04L_testCheckSum(SM_UART_04L_Data, &PM_Data) == 1){
    		SM_UART_04L_dataParse(SM_UART_04L_Data, &PM_Data);
    	}
    }
    return 0 ;
}
