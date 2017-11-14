#ifndef _MAIN_H_
#define _MAIN_H_

#define BOARD_EDU_CIAA_NXP

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>

#include "Configure.h"
#include "utils.h"
xSemaphoreHandle xUSARTSemaphore;	// Semaforo para la recepcion por usart
TaskHandle_t USARTTaskHandle;		// Handler de la tarea de recepcion por usart
volatile char control_char=0;

xSemaphoreHandle xStartSemaphore;	//Semaforo para el encendido del horno
TaskHandle_t StartTaskHandle;		//Handler de la tarea de encendido

xSemaphoreHandle xStopSemaphore;	//Semaforo para el apagado del horno
TaskHandle_t StopTaskHandle;		//Handler de la tarea de apagado

xSemaphoreHandle xADCSemaphore;	//Semaforo para el ADC
TaskHandle_t ADCTaskHandle;		//Handler de la tarea del ADC
volatile uint16_t adc_data=0;	//Variable global donde se almacenan las mediciones del ADC

xSemaphoreHandle xControllerSemaphore;	//Semaforo para el controlador
TaskHandle_t ControllerTaskHandle;		//Handler de la tarea del controlador

xSemaphoreHandle xPhaseSemaphore;		//Semaforo para la deteccion de cruces por cero
TaskHandle_t PhaseTaskHandle;			//Handler de la tarea del detector de fase

TaskHandle_t UpdateReferenceTaskHandle; //Handler de la tarea que actualiza la referencia de temperatura
TaskHandle_t USARTTransmitTaskHandle;	//Handler de la tarea que envia la info medida

float thermocouple_temp=0;		//Temperatura de la termocupla
bool trigger_state=false;		//Estado del trigger

#define TEMPERATURE_INERTIA 5		//Inercia de temperatura del horno
#define REFERENCE_INTERVAL (5000/(portTICK_RATE_MS))	//Intervalo de tiempo para la actualizacion de la referencia 5s es el viejo
float reference=0;				//Temperatura de referencia

/* Curva de temperatura que se debe seguir */
const float temperature_profile1_val[]={25,
		   29.082,
		   29.195,
		   29.942,
		   31.186,
		   32.804,
		   34.691,
		   36.751,
		   38.902,
		   41.076,
		   43.214,
		   45.267,
		   47.198,
		   48.977,
		   50.583,
		   52.004,
		   53.234,
		   54.273,
		   55.128,
		   55.809,
		   56.332,
		   56.715,
		   56.982,
		   57.155,
		   57.260,
		   57.324,
		   57.373,
		   57.433,
		   57.529,
		   57.683,
		   57.915,
		   58.242,
		   58.678,
		   59.228,
		   59.897,
		   60.681,
		   61.569,
		   62.543,
		   63.577,
		   64.637,
		   65.677,
		   66.644,
		   67.470,
		   68.079,
		   68.381,
		   68.274,
		   67.640,
		   66.348,
		   64.253,
		   61.193
};
const float temperature_profile2_val[]={
		80,80,80,80,80,80,80,80,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175,
		175
};

struct temperature_profile {
	float * values;
	int size;
};

static void vHandlerUSART(void *pvParameters);	//Tarea que recibe de la usart y da los semaforos de encendido y apagado
static void vHandlerStart(void *pvParameters);	//Tarea donde se espera que se presione el boton de encendido
static void vHandlerStop(void *pvParameters);	//Tarea donde se espera que se presione el boton de apagado
static void vHandlerGetTemperature(void *pvParameters);	//Tarea que mide la temperatura
static void vHandlerController(void *pvParameters);		//Tarea que controla la resistencia
static void vHandlerZeroCrossing(void *pvParameters);	//Tarea que se ejecuta con los cruces por cero
static void vHandlerUpdateTemperatureReference(void *pvParameters);	//Tarea que actualiza la referencia de temperatura
static void vHandlerUSARTTransmit(void *pvParameters);	//Tarea que envia la data al teléfono

#endif
