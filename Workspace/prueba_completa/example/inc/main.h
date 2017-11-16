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
#define OFF FALSE
#define ON TRUE
bool oven_state=OFF;

#define TEMPERATURE_INERTIA 5		//Inercia de temperatura del horno
#define REFERENCE_INTERVAL (15000/(portTICK_RATE_MS))	//Intervalo de tiempo para la actualizacion de la referencia 5s es el viejo
float reference=0;				//Temperatura de referencia

/* Curva de temperatura que se debe seguir */
const float temperature_profile1_val[]={
		80,80,80,
		150,150,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190,
		190
};
const float temperature_profile2_val[]={
		80,80,80,
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
