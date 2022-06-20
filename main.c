/**********************************************************
EE337: ADC IC MCP3008 interfacing with pt-51
(1) Complete spi_init() function from spi.h 
(2) Uncomment LM35 interfacing code from main function
		Choose appropriate scaling factor for converting temperature 
		sensor reading sampled using ADC such that it will be in 
		degree Celsius and display it on the LCD)
***********************************************************/

#include <at89c5131.h>
#include "serial.c"													//Driver for interfacing lcd 
#include "mcp3008.h"												//Driver for interfacing ADC ic MCP3008

/***************************************************************
FUNCTION DECLARATIONS
*************************************************************/
float avg_calc(float temp_data[]);
void alarm(unsigned int f, unsigned int t);
void timer_init(void);
void temp_ar_updater(void);
void int_to_string(unsigned int val,unsigned char *temp_str_data);
void msdelay(unsigned int time);

/***************************************************************
GLOBAL VARIABLES USED
*************************************************************/

sbit audio_op = P0^0;          	// alarm output port
unsigned int t_int_count = 0;	// temp interrupt counter
float temp_arr[10] = {0};		// array to store temperatures
float temperature;				// temperature reading
unsigned int adc_val;			// value received
int counter = 0;				// temperature array index

/**********************************************************
  int_to_string(<integer_value>,<string_ptr>): 
	Converts integer to string of length 5
***********************************************************/	
void int_to_string(unsigned int val,unsigned char *temp_str_data){	
	temp_str_data[0]=48+(val/100);
	temp_str_data[1]=48+((val%100)/10);
	temp_str_data[3]=48+(val%10);
}


/**********************************************************
   msdelay(<time_val>): 
	Delay function for delay value <time_val>ms
***********************************************************/	
void msdelay(unsigned int time)
{
	int i,j;
	for(i=0;i<time;i++)
	{
		for(j=0;j<382;j++);
	}
}


/***************************************************************
AVERAGE CALCULATOR FUNCTION
*************************************************************/

float avg_calc(float temp_data[]){

	unsigned int num_temp = 10;
	float sum = 0;
	unsigned int zeros = 0;
	unsigned int i = 0;
	float avg = 0.0;

	for(i=0; i<num_temp; i++){
		sum += temp_data[i];
		if(temp_data[i]==0){
			zeros+=1;
		}
	}
	avg = (float) (sum/(num_temp)); // Remove the "-zeros" part to observe out of range alarm during the start of code
	return avg;
}

/***************************************************************
ALARM FUNCTION
*************************************************************/

void alarm(unsigned int f, unsigned int t){
  	unsigned int h_delay = (unsigned int) (500/f);
	unsigned int count = (unsigned int) (t*f);
	
	audio_op = 1;

	while(count){
		msdelay(h_delay);
		audio_op = ~(audio_op);
		count -= 1;
	}
}


/***************************************************************
TIMER INIT FUNCTION
*************************************************************/

// void timer_init(void){
// 	TMOD &= 0x0F1;
// 	TMOD |= 0x01;
// 	TR0 = 0;
// 	TF0 = 0;
// 	TH0 = 0;
// 	TL0 = 0;
// 	ET0 = 1;
// 	EA = 1;
// 	TR0 = 1;	 
// }

/***************************************************************
LCD VARIABLES
*************************************************************/

char adc_ip_data_ascii[5]={0,0,'.',0, '\0'};			
code unsigned char display_msg2[]="Avg. Temp: ";		
code unsigned char display_alert[]="TEMP RANGE ALERT";	

/***************************************************************
MAIN FUNCTION
*************************************************************/

void main(void)
{	
	unsigned int x;
	float avg_temp;
	unsigned int var = 0;	// state of machine i.e. alarm mode or normal mode
	unsigned int ctr = 0;	// for 4 sec alarm 
	float temp_prev1 = 0; 	// last temp reading
	float temp_prev2 = 0; 	// last-to-last temp reading

	spi_init();																					
	adc_init();
  	uart_init();
	// timer_init();
	
	
	while(1)
	{
		
		temp_prev2 = temp_prev1;
		temp_prev1 = temperature;

		x = adc(7);								
		temperature = (x*0.1*3.2258); 

		temp_arr[counter] = temperature;


		if(ctr==0){

			avg_temp = avg_calc(temp_arr);

			if((temperature>(avg_temp+2)) | (temperature<(avg_temp-2))){
					var=1;
				}
			else{
				var = 0;
			}
		}

		switch (var) {
			case 0: 

				// transmit_string("Counter: ")
				// int_to_string(counter*10, adc_ip_data_ascii);
				// transmit_string(adc_ip_data_ascii);
				// transmit_string("\r\n");

				transmit_string("Temp: ");
				
				int_to_string(temperature*10, adc_ip_data_ascii);
				transmit_string(adc_ip_data_ascii);
				transmit_string(" ");
				
				int_to_string(temp_prev1*10, adc_ip_data_ascii);
				transmit_string(adc_ip_data_ascii);
				transmit_string(" ");

				int_to_string(temp_prev2*10, adc_ip_data_ascii);
				transmit_string(adc_ip_data_ascii);
				transmit_string("\r\n");

				// avg_temp = avg_calc(temp_arr);
				int_to_string(avg_temp*10, adc_ip_data_ascii);
				transmit_string("Avg. Temp: ");
				transmit_string(adc_ip_data_ascii);
				transmit_string("\r\n");
				transmit_string("\r\n");

				// if((temperature>(avg_temp+2)) | (temperature<(avg_temp-2))){
				// 	var=1;
				// }	

				msdelay(1000);

				break;

			case 1:
				if(ctr==0){
				transmit_string("TEMP RANGE ALERT");
				transmit_string("\r\n*");
				int_to_string(avg_temp*10, adc_ip_data_ascii);
				transmit_string("Avg. Temp: ");
				transmit_string(adc_ip_data_ascii);
				transmit_string("\r\n*");
				transmit_string("\r\n*");
				}

				alarm(250, 1);
				ctr += 1;
				if(ctr==4){
					// var = 0;
					ctr = 0;
				}
				break;
		}

		counter = (counter+1)%10;	
	}
	

}

/***************************************************************
TIMER 0 INTERRUPT FUNCTION
*************************************************************/

// void timer0_interrupt(void) interrupt 1
// {
// 	TR0 = 0;
// 	t_int_count += 1;
	
// 	if(t_int_count==30){
// 		t_int_count=0;
// 		temp_ar_updater();
// 	}

// 	TR0 = 1;
// 	ET0 = 1;
// 	TF0 = 0;
// }

/***************************************************************
TEMPERATURE ARRAY UPDATE FUNCTION
*************************************************************/

// void temp_ar_updater(void){
// 	adc_val = adc(7);									//Read analog value from 0th channel of ADC Ic MCP3008
// 	temperature = (adc_val*0.1*3.2258); 	//Converting received 10 bits value to milli volt (3.3*1000*i/p /1023)
	
// 	temp_arr[counter] = temperature;
// 	counter = (counter+1)%10;
// }

