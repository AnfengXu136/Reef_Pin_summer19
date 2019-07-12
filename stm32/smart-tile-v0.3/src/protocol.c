#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

// Buffer for store received chars
static uint8_t recv_buffer[MAX_BUFFER_LEN];


// private functions
void removePadding(uint8_t recv_length, uint8_t *recv_package_buff, uint8_t *package_length);

uint8_t recvByteHandler(uint8_t byte, uint8_t *recv_package_buff, uint8_t *package_length) {
	static uint8_t recv_state = 0;
	static uint8_t recv_last_byte = 0x00;
	static uint8_t recv_length = 0;
	static uint8_t recv_idx = 0;
	static uint8_t recv_ignore_flag = 0;
	static uint8_t recv_sum = 0;
	uint8_t rtrn_value = 0;

	switch(recv_state) {
	case 0:
		printf("We are in case 0\n");
		if (recv_last_byte == PROT_FIRST_BYTE && byte == PROT_SECOND_BYTE) {
			recv_state = 1;
			recv_idx = 0;
		}
		else {
			recv_state = 0;
			recv_idx = 0;
		}
		break;
	case 1:
		printf("We are in case 1\n");
		recv_state = 2;
		recv_idx = 0;
		recv_length = byte;
		if(byte > 29 && byte < 40) recv_length = byte - 30;
		recv_sum = byte;
		if(byte == PROT_FIRST_BYTE) {
			recv_ignore_flag = 1;
		}else{
			recv_ignore_flag = 0;
		}
		break;
	case 2:
		printf("We are in case 2\n");
		if (recv_last_byte == PROT_FIRST_BYTE && byte == PROT_SECOND_BYTE) {
			recv_state = 1;
			recv_idx = 0;
		}
		else {
			if (recv_idx < recv_length) {
				if (recv_idx < MAX_BUFFER_LEN) {
					recv_sum += byte;
					if(recv_ignore_flag) {
						recv_ignore_flag = 0;
					} else {
						recv_buffer[recv_idx] = byte;
						recv_idx++;
					}
				}
				else {
					recv_state = 0;
					recv_idx = 0;
					recv_sum = 0;
				}
			}
			else {
				if (recv_idx == recv_length && recv_sum == byte) {
					// remove padding and process
					removePadding(recv_length, recv_package_buff, package_length);
					rtrn_value = 1;
				}
				recv_state = 0;
				recv_idx = 0;
				recv_sum = 0;
			}
		}
		break;
	default:
		// idle state
		recv_state = 0;
		recv_idx = 0;
		recv_sum = 0;
	}
	recv_last_byte = byte;
	return rtrn_value;
}

uint8_t packageToFrame(uint8_t *package_buff, uint8_t package_length, uint8_t *frame_buff, uint8_t *frame_length) {
	uint8_t p_idx;
	uint8_t f_idx;
	uint8_t padd_cnt = 0;
	uint8_t sum = 0;
	uint8_t padd_first_flag = 0;
	if(package_length > MAX_PACKAGE_LEN) return 0;
	for(p_idx=0; p_idx<package_length; p_idx++) {
		if(package_buff[p_idx] == PROT_FIRST_BYTE) padd_cnt++;
	}
	if(padd_cnt+package_length == PROT_FIRST_BYTE) padd_first_flag = 1;
	// header
	frame_buff[0] = PROT_FIRST_BYTE;
	frame_buff[1] = PROT_SECOND_BYTE;
	frame_buff[2] = padd_cnt + package_length;
	sum += frame_buff[2];
	f_idx = 3;
	if(padd_first_flag) {
		frame_buff[f_idx] = PROT_PADD_BYTE;
		sum += PROT_PADD_BYTE;
		f_idx++;
	}
	for (p_idx=0; p_idx<package_length; p_idx++) {
		frame_buff[f_idx] = package_buff[p_idx];
		sum += package_buff[p_idx];
		f_idx++;
		if(package_buff[p_idx] == PROT_FIRST_BYTE) {
			frame_buff[f_idx] = PROT_PADD_BYTE;
			sum += PROT_PADD_BYTE;
			f_idx++;
		}
	}
	frame_buff[f_idx] = sum;
	*frame_length = padd_cnt + package_length + padd_first_flag + 4;
	return 1;
}

void removePadding(uint8_t recv_length, uint8_t *recv_package_buff, uint8_t *package_length) {
	uint8_t p_idx;
	uint8_t f_idx;
	f_idx = 0;
	p_idx = 0;
	while(f_idx<recv_length) {
		recv_package_buff[p_idx] = recv_buffer[f_idx];
		if(recv_buffer[f_idx] ==  PROT_FIRST_BYTE) f_idx++;
		f_idx++;
		p_idx++;
	}
	*package_length = p_idx;
}
