#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "stm32f10x.h"
#include <stdio.h>

#define PROT_FIRST_BYTE		0x0D
#define PROT_SECOND_BYTE	0x0A
#define PROT_PADD_BYTE		0x50
#define MAX_PACKAGE_LEN		20
#define MAX_BUFFER_LEN		41
#define MAX_FRAME_LEN		45

uint8_t recvByteHandler(uint8_t byte, uint8_t *recv_package_buff, uint8_t *package_length);
uint8_t packageToFrame(uint8_t *package_buff, uint8_t package_length, uint8_t *frame_buff, uint8_t *frame_length);


#endif
