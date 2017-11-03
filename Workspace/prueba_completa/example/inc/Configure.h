#ifndef _CONFIGURE_H_
#define _CONFIGURE_H_

#include "board.h"
/* Configuracion de la USART */
#define USART_IRQN_HANDLER 		 UART3_IRQHandler
#define USART					 LPC_USART3
#define USART_IRQN				 USART3_IRQn
#define USART_INTERRUPT_PRIORITY 1

/* Configuracion del ADC */
#define ADC_TEMPERATURE			LPC_ADC0
#define ADC_CHANNEL 			ADC_CH0
#define ADC_IRQN				ADC0_IRQn
#define ADC_IRQN_HANDLER		ADC0_IRQHandler
#define ADC_INTERRUPT_PRIORITY	5
ADC_CLOCK_SETUP_T ADCSetup;			//Estructura de condiguracion del ADC
#define MAX_ADC_SAMPLES			10	//Cantidad de muestras del ADC que se promedian
#define ADC_SAMPLES_DELAY		(10/(portTICK_RATE_MS))	//Delay entre actualizaciones de la temperatura

/* Pin de deteccion de fase */
#define PHASE_SCU_INT_PORT	6			//Puerto del SCU usado para la deteccion de fase
#define PHASE_SCU_INT_PIN	1			//Pin del SCU usado para la deteccion de fase
#define PHASE_GPIO_INT_PORT	3			//Puerto del GPIO usado para la deteccion de fase
#define PHASE_GPIO_INT_PIN	0			//Pin del GPIO usado para la deteccion de fase
#define PHASE_PININT_INDEX	0			//Canal de las interrupciones para el cruce por cero
#define PHASE_INTERRUPT_PRIORITY	5	//Prioridad de la interrupcion del detector de cruce por cero
#define PHASE_IRQN_HANDLER	GPIO0_IRQHandler	//Handler de la interrupcion
#define PHASE_IRQN			PIN_INT0_IRQn		//Interrupcion

/* Pin de trigger del triac */
#define TRIGGER_SCU_INT_PORT	6			//Puerto del SCU usado para el trigger del triac
#define TRIGGER_SCU_INT_PIN		5			//Pin del SCU usado para el trigger del triac
#define TRIGGER_GPIO_INT_PORT	3			//Puerto del GPIO usado para el trigger del triac
#define TRIGGER_GPIO_INT_PIN	4			//Pin del GPIO usado para el trigger del triac

/* Pin de encendido */
#define START_SCU_INT_PORT	1			//Puerto del SCU usado para el boton de encendido
#define START_SCU_INT_PIN	0			//Pin del SCU usado para el boton de encendido
#define START_GPIO_INT_PORT	0			//Puerto del GPIO usado para el boton de encendido
#define START_GPIO_INT_PIN	4			//Pin del GPIO usado para el boton de encendido
#define START_PININT_INDEX	1			//Canal de las interrupciones para el boton de encendido
#define START_INTERRUPT_PRIORITY	6	//Prioridad de la interrupcion para el boton de encendido
#define START_IRQN_HANDLER	GPIO1_IRQHandler	//Handler de la interrupcion
#define START_IRQN			PIN_INT1_IRQn		//Interrupcion

/* Pin de apagado */
#define STOP_SCU_INT_PORT	1			//Puerto del SCU usado para el boton de apagado
#define STOP_SCU_INT_PIN	1			//Pin del SCU usado para el boton de apagado
#define STOP_GPIO_INT_PORT	0			//Puerto del GPIO usado para el boton de apagado
#define STOP_GPIO_INT_PIN	8			//Pin del GPIO usado para el boton de apagado
#define STOP_PININT_INDEX	2			//Canal de las interrupciones para el boton de apagado
#define STOP_INTERRUPT_PRIORITY	6		//Prioridad de la interrupcion para el boton de apagado
#define STOP_IRQN_HANDLER	GPIO2_IRQHandler	//Handler de la interrupcion
#define STOP_IRQN			PIN_INT2_IRQn		//Interrupcion

void SetupHardware(void);				//Configuracion del hardware
void ConfigureUSART(void);				//Configuracion de la USART
void ConfigureADC(void);				//Configuracion del ADC
void ConfigurePhaseDetector(void);		//Configuracion del detector de cruce por cero
void ConfigureTriggerResistor(void);	//Configuracion del trigger que controla la resistencia
void ConfigureStartButton(void);		//Configuracion del boton de encendido
void ConfigureStopButton(void);			//Configuracion del boton de apagado

#endif
