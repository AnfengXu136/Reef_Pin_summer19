#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "usart.h"

int RECV_CNT = 0;
int dbg_line_flag = 0;
char dbg_line[90];


void packageToString(uint8_t *package_buff, uint8_t package_length, char *line) {
	uint8_t idx = 0;
	sprintf(line, " ");
	for(idx=0; idx<package_length; idx++) {
		sprintf(line + strlen(line),"%02x ", 0xFF	& package_buff[idx]);
	}
}


void parsePackage(uint8_t *package_buff, uint8_t package_length) {
	// TODO test recv package
	// TODO test transmission
	RECV_CNT++;
	// uint8_t testpack[4] = {0x0A,0x0B,0x0C,0x0D};
	sendPackage(package_buff, package_length);
	// packageToString(package_buff, package_length, dbg_line);
	// dbg_line_flag = 1;

}
