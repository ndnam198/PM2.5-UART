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
#include <string.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKE02Z4.h"


#define I2C_MASTER_SLAVE_ADDR_7BIT 0x70
#define I2C_DATA_LENGTH 6
#define CRC_POLYNOMIAL 0x131 //P(x) = x^8 + x^5 + x^4 + 1 = 100110001



uint8_t read_Buffer[I2C_DATA_LENGTH];	//A buffer stores I2C data received

enum SHTC3_COMMAND{
	SOFTWARE_RESET = 0x805D,
	READ_ID_REGISTER = 0xEFC8,
	WAKEUP = 0x3517,
	SLEEP = 0xB098,
	//stretching enabled
	normalMode_stretchingEnabled_T_FIRST = 0x7CA2,
	normalMode_stretchingEnabled_RH_FIRST = 0x5C24,
	lowPowMode_stretchingEnabled_T_FIRST = 0x6458,
	lowPowMode_stretchingEnabled_RH_FIRST = 0x44DE,
	//stretching disabled
	normalMode_stretchingDisabled_T_FIRST = 0x7866,
	normalMode_stretchingDisabled_RH_FIRST = 0x58E0,
	lowPowMode_stretchingDisabled_T_FIRST = 0x609C,
	lowPowMode_stretchingDisabled_RH_FIRST = 0x401A
};

struct SHTC3{
	float humidity;
	float temperature;
};

static struct SHTC3 SHTC3_Data;

const uint8_t SM_UART_04L_Header[] = {0x42, 0x4D}; //Start each data frame with this 2 byte

//A 32 byte buffer to store Sensor data, ch to store header for comparision
static uint8_t SM_UART_04L_Data[32], ch;

struct SM_UART_04L_Data{
	uint16_t framelen;
	uint16_t pm10_standard, pm25_standard, pm100_standard;
	uint16_t pm10_env, pm25_env, pm100_env;
	uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
	uint16_t unused;
	uint16_t checksum;
};

static struct SM_UART_04L_Data PM_Data;



/*
 * @brief Configure I2C Master to send command to slave
 *
 *
 * */
void SHTC3_sendCommand(enum SHTC3_COMMAND command){
	i2c_master_transfer_t Transfer_config;
	memset(&Transfer_config, 0, sizeof(Transfer_config));
	Transfer_config.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
	Transfer_config.direction      = kI2C_Write;
	Transfer_config.subaddress     = (uint32_t)command;
	Transfer_config.subaddressSize = 1;
	Transfer_config.data           = NULL;
	Transfer_config.dataSize       = 0;
	Transfer_config.flags          = kI2C_TransferDefaultFlag;
	I2C_MasterTransferBlocking(I2C0, &Transfer_config);
}

/*
 * @brief Do nothing command
 *
 * */
void I2C_Wait(uint32_t i){
	while(i--){
		__NOP();
	}
}


/*
 *	@brief Configure I2C Master to receive data from slave, return value to read_Buffer[I2C_DATA_LENGTH]
 *
 *
 * */
void SHTC3_readData(){
	i2c_master_transfer_t Receiver_config;
	memset(&Receiver_config, 0, sizeof(Receiver_config));
	Receiver_config.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
	Receiver_config.direction      = kI2C_Read;
	Receiver_config.subaddress     = (uint32_t)NULL;
	Receiver_config.subaddressSize = 0;
	Receiver_config.data           = read_Buffer;
	Receiver_config.dataSize       = I2C_DATA_LENGTH;
	Receiver_config.flags          = kI2C_TransferDefaultFlag;
	I2C_MasterTransferBlocking(I2C0, &Receiver_config);
}

/*
 * @brief 4 Steps for a single read of SHTC3 Sensor
 * 	1 Wakeup Command
 * 	2 Measurement command
 * 	3 Read out command
 * 	4 Sleep command
 *
 * */
void SHTC3_singleRead(){
	SHTC3_sendCommand(WAKEUP);
	I2C_Wait(1000);
	SHTC3_sendCommand(normalMode_stretchingEnabled_RH_FIRST);
	I2C_Wait(1000);
	SHTC3_readData();
	I2C_Wait(2000);
	SHTC3_sendCommand(SLEEP);
}

/*
 * @brief Test checksum byte returned in data frame
 *
 *
 * */
uint8_t SHTC3_testChecksum(uint8_t *data_frame){
	uint8_t bit;
	uint8_t crc = 0xFF; // calculated checksum
	uint8_t byteCtr;    // byte counter

	// calculates 8-Bit checksum with given polynomial
	for(byteCtr = 0; byteCtr < 2; byteCtr++) {
		crc ^= (data_frame[byteCtr]);
		for(bit = 8; bit > 0; --bit) {
		  if(crc & 0x80) {
			crc = (crc << 1) ^ CRC_POLYNOMIAL;
		  } else {
			crc = (crc << 1);
		  }
		}
	}

	if(crc != data_frame[2]) return 0;
	else return 1;
}

/*
 * @brief Calculate the Humidity value
 *
 *
 * */
void SHTC3_humidCal(uint8_t *data_frame, struct SHTC3 *SHTC3_Data){
	// RH = rawValue / 2^16 * 100
	SHTC3_Data->humidity = 100 * (float)(data_frame[0] * 256 + data_frame[1]) / 65536.0f;
}

/*
 * @brief Calculate the temperature value
 *
 *
 * */
void SHTC3_tempCal(uint8_t *data_frame, struct SHTC3 *SHTC3_Data){
	// T = -45 + 175 * rawValue / 2^16
	SHTC3_Data->temperature = 175 * (float)(data_frame[3] * 256 + data_frame[4]) / 65536.0f - 45.0f;
}

/*
 * @brief get Sensor data
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
 * @brief test checksum byte
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
 * @brief calculate value from data frame
 *
 *
 * */
void SM_UART_04L_dataCal(uint8_t *SM_UART_04L_Data, struct SM_UART_04L_Data *PM_Data){
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

    	//SM_UART_04L
    	SM_UART_04L_getData(SM_UART_04L_Data, ch);
    	if(SM_UART_04L_testCheckSum(SM_UART_04L_Data, &PM_Data) == 1){
    		SM_UART_04L_dataCal(SM_UART_04L_Data, &PM_Data);
    	}

    	//SHTC3
    	SHTC3_singleRead();
    	if(SHTC3_testChecksum(read_Buffer) == 1){
    		SHTC3_humidCal(read_Buffer, &SHTC3_Data);
    	}
    	if(SHTC3_testChecksum(read_Buffer) == 1){
    		SHTC3_tempCal(read_Buffer, &SHTC3_Data);
    	}

    }
    return 0 ;
}
