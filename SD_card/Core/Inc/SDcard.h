/*
 * SDcard.h
 *
 *  Created on: Nov 22, 2024
 *      Author: burakguzeller
 */

#ifndef INC_SDCARD_H_
#define INC_SDCARD_H_

#include "main.h"
#include "fatfs.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/******** EXTERN TYPEDEF STRUCT *****************/
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;
extern void debugMessage(char *format,...);

/******** DEFINE MACRO DEFINATION ***************/
#define SD_CARD_MOUNT_ERROR "ERROR IN MOUNTING SD CARD.\r\n"
#define SD_CARD_MOUNT_SUCCESS "SD CARD MOUNTED SUCCESSFULLY\r\n"


/********* FUNCTION PROTOIPES ******************/
void SDCardIsCapacity(void);
bool mountSDCard(void);
bool createSDFile(const char *fileName);
void createAndWriteSDFile(const char *fileName, const char *data);
void LoggerSDFile(const char *fileName, const char *data);
void readSDFileData(const char *fileName);
void readLargeSDFile(const char *fileName);
void removingSDFile(const char* fileName);

#endif /* INC_SDCARD_H_ */
