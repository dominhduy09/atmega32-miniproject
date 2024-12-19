
#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "I2C_LCD.h"


uint8_t ss = 57, mm = 59, hh = 23, dd = 28, MM = 2, leap = 0;
uint16_t yy = 2001, ms = 0;

uint8_t dom[2][12] = {
						{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
						{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}							
					};
					
char I2C_LCD_first_line[] =  "Time:   xx:xx:xx";
char I2C_LCD_second_line[] = "Date: dd:mm:yyyy";
uint8_t Update_LCD = 0;
uint8_t pos_1 = 0,  pos_2 = 0;
uint8_t temp, i, j, lp = 0;
uint8_t BS[6], L_BS[6];


uint8_t Check_button(uint8_t i_th){
	if ((L_BS[i_th] - BS[i_th]) == 1) return 1;				//detect when button is pressed
	else return 0;
} 

uint8_t Check_leap_year(uint16_t year){
	uint16_t leap_year;
	for(leap_year = 1904; leap_year <2100; leap_year += 4){
		if(year == leap_year) return 1;
	}
	return 0;
}

void I2C_LCD_Update(){
	sprintf(I2C_LCD_first_line, "Time:   %02d:%02d:%02d",hh,mm,ss);
	sprintf(I2C_LCD_second_line,"Date: %02d:%02d:%04d",dd,MM,yy);
	if(pos_1 != 0){
		if(ms > 550){
			I2C_LCD_first_line[5+3*pos_1] = '_';
			I2C_LCD_first_line[6+3*pos_1] = '_';
		}
	}
	if(pos_2 != 0){
		if(ms > 550){
			if((pos_2 == 1) || (pos_2 == 2)){
				I2C_LCD_second_line[3+3*pos_2] = '_';
				I2C_LCD_second_line[4+3*pos_2] = '_';
			}
			else if(pos_2 == 3){
				for(j = 0; j < 4; j++) I2C_LCD_second_line[12+j] = '_';
			}
		}
	}
	I2C_LCD_Clear();
	I2C_LCD_GotoXY(0,0);
	I2C_LCD_WriteString(I2C_LCD_first_line);
	I2C_LCD_GotoXY(0,1);
	I2C_LCD_WriteString(I2C_LCD_second_line);
}

void Calendar_Update(){
	ms++;
	if(ms == 1000){
		ms = 0;
		ss++;
		if(ss == 60){
			ss = 0;
			mm++;
			if((mm == 60)){
				mm = 0;
				hh++;
				if(hh == 24){
					hh = 0; 
					if(dd == dom[leap][MM-1]){
						dd = 1;
						MM++;
						if(MM == 13){
							MM = 1;
							yy++;
							leap = Check_leap_year(yy);
						}
					}	
					else dd++;
				}					
			}	
		}
	}
}

void Timer0_CTC_Init(){
	TCCR0 |= (1<<WGM01)|(1<<CS01)|(1<<CS00);
	TIMSK |= (1<<OCIE0);
	OCR0 = 249;
	TCNT0 = 0;
}

int main(void){
	
	I2C_LCD_Init(I2C_FREQ, LS_NONE);
	I2C_LCD_BackLight(ON);
	Timer0_CTC_Init();
	sei();
	leap = Check_leap_year(yy);
	I2C_LCD_Update();
		
	while(1){
		for(i = 0; i < 6; i++) BS[i] = (PIND>>i)&1;
		if(Check_button(0)) pos_1 = (pos_1 + 1)%4;
		if(Check_button(3)) pos_2 = (pos_2 + 1)%4;
		
		if(pos_1 == 1){
			if(Check_button(1)){
				if(hh == 23) hh = 0;
				else hh++;
			}
			if(Check_button(2)){
				if(hh == 0) hh = 23;
				else hh--;
			}
		}	
		if(pos_1 == 2){
			if(Check_button(1)){
				if(mm == 59) mm = 0;
				else mm++;
			}
			if(Check_button(2)){
				if(mm == 0) mm = 59;
				else mm--;
			}
		}
		if(pos_1 == 3){
			if(Check_button(1)){
				if(ss == 59) ss = 0;
				else ss++;
			}
			if(Check_button(2)){
				if(ss == 0) ss = 59;
				else ss--;
			}
		}
		
		if(pos_2 == 1){ 
			if(Check_button(4)){
				if(dd == dom[leap][MM-1]) dd = 1;
				else dd++;
			}
			if(Check_button(5)){
				if(dd == 1) dd = dom[leap][MM-1];
				else dd--;
			}
		}	
		if(pos_2 == 2){
			if(Check_button(4)){
				MM++;
				if(MM == 13) MM = 1;
			}
			if(Check_button(5)){
				if(MM == 1) MM = 12;
				else MM--;
			}
		}	
		if(pos_2 == 3){
			if(Check_button(4)){
				yy++;
				leap = Check_leap_year(yy);
			}
			if(Check_button(5)){
				yy--;
				leap = Check_leap_year(yy);
			}
		}
		for(i = 0; i < 6; i++) L_BS[i] = BS[i];		
		if((ms%50) == 0) I2C_LCD_Update();
    }
	return 0;
}

ISR(TIMER0_COMP_vect){
	Calendar_Update();
	
}