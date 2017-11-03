#ifndef UTILS_H_
#define UTILS_H_

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdlib.h>

float get_temperature(uint16_t adc_value);	//Funcion que mapea la tension de la termocupla y la temperatura

#define OPAMP_GAIN	234		//Ganancia del circuito amplificador de la termocupla

#endif
