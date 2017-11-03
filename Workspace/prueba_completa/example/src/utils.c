#include "utils.h"

/* Funcion que mapea la temperatura a partir de la tension de la termocupla */
float get_temperature( uint16_t adc_value ){
	float temp=0;

	temp = ((float)adc_value)*3.3/1024;		//Lo mapeo de 0-3.3v
	temp /= OPAMP_GAIN;						//Lo divido por la ganancia del operacional
	temp = 23701.6*temp-8.75; //23376.525*temp-1.574;				//Mapeo la temperatura
return temp;
}
