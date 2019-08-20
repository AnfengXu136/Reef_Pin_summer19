#include "sdfile.h"

int make_integer(float num) {
    int thous;
    int hund;
    int ten;
    int	one;
    int integer;
    int counter = 0;

    thous = num;
    while(thous >= 1000) {
        thous -= 1000;
        counter++;
    }
    thous = counter;
    hund = num - thous*1000;
    counter = 0;
    while(hund >= 100) {
        hund -= 100;
        counter++;
    }
    hund = counter;
    ten = num - thous*1000 - hund*100;
    counter = 0;
    while (ten >= 10) {
        ten -= 10;
        counter++;
    }
    ten = counter;
    one = num - thous*1000 - hund*100 - ten*10;
    counter = 0;
    while (one >= 1) {
        one -= 1;
        counter++;
    }
    one = counter;
    integer = thous*1000 + hund*100 + ten*10 + one;
    return integer;
}

float make_frac(float num, int integer) {
	float frac;
	frac = num - integer;
	return frac;
}

void print_rtc_time_to_string(void) {
	char timestamp[32];

	uint32_t seconds, minutes, hours, days, year, month;
//	uint32_t dayOfWeek;
	seconds = RTC_GetCounter();
	/* calculate minutes */
	minutes = seconds / 60;
	seconds -= minutes * 60;
	/* calculate hours */
	hours = minutes / 60;
	minutes -= hours * 60;
	/* calculate days */
	days = hours / 24;
	hours -= days * 24;
	/* Unix time starts in 1970 on a Thursday */
	year = 2019;
//	dayOfWeek = 4;
	while (1) {
		bool leapYear = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
		uint16_t daysInYear = leapYear ? 366 : 365;
		if (days >= daysInYear) {
//			dayOfWeek += leapYear ? 2 : 1;
			days -= daysInYear;
//			if (dayOfWeek >= 7)
//				dayOfWeek -= 7;
			++year;
		} else {
//			dayOfWeek += days;
//			dayOfWeek %= 7;
			/* calculate the month and day */
			static const uint8_t daysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31,
					31, 30, 31, 30, 31 };
			for (month = 0; month < 12; ++month) {
				uint8_t dim = daysInMonth[month];
				/* add a day to february if this is a leap year */
				if (month == 1 && leapYear)
					++dim;
				if (days >= dim)
					days -= dim;
				else
					break;
			}
			break;
		}
	}
	sprintf(timestamp, "%d-%d-%d_%d-%d-%d", (int) year, month, days + 1,
				hours, minutes, seconds);
}

/* Purpose:
 * creates log name from RTC
 * names the file to be created
 * mounts SD card
 * opens file
 * returns log number
*/
int sd_mkfile(void) {
	static FATFS FatFs;
	FILINFO fno;
	FRESULT fr;
	int logNumber;
    char logName[100], logTitle[100];

	// mount SD card
	if (f_mount(&FatFs, "", 0) == FR_OK) printf("Mount success\n");
	else printf("Mount failed\n");
	// create log name and open file
	logNumber = 0;
	do {
		logNumber++;
		sprintf(logName, "log_%d.csv", logNumber);
		fr = f_stat(logName, &fno);
	} while (fr == FR_OK);
    sprintf(logName, "log_%d.csv", logNumber);
    if (f_open(&file, logName, FA_OPEN_ALWAYS | FA_WRITE) == FR_OK) {
    	printf("File opened\n");
    	sprintf(logTitle, "Depth(m):,Pressure(mbar):\n");
    	f_puts(logTitle, &file);
    }
    else printf("File failed\n");
    return logNumber;
}

/* Purpose:
 * loads data from pressure sensor into opened file
 * returns write error
*/
int sd_data(float depth, float pressure) {
    char buffer[100];

	//go to the end of the file
    f_lseek(&file, file.fsize);
    //get data and generate string
	sprintf(buffer, "%0.3f,%0.3f\n", depth, pressure);
    //write data to the file
    if (f_puts(buffer, &file) != -1) {
    	printf("Write success\n");
    	return 0;
    }
    else {
    	printf("Write failed\n");
    	return 1;
    }
}

/* Purpose:
 * closes file
 * unmounts SD card
*/
void sd_closefile(void) {
	if (f_close (&file) == FR_OK) {
	    printf("Close success\n");
	}
	else printf("Close failed\n");
	if (f_mount(0, "", 0) == FR_OK) {
	    printf("Unmounted\n");
	}
	else printf("Mount failed\n");
}
