#include "Configure.h"

/*****************************************************************************
 * Funciones de configuracion
*****************************************************************************/
/* Sets up system hardware */
void SetupHardware(void){
	SystemCoreClockUpdate();
	Board_Init();
}

/* Configuracion del ADC */
void ConfigureADC( void ){
	Chip_ADC_Init(ADC_TEMPERATURE, &ADCSetup);						//Se inicializa el ADC
	Chip_ADC_EnableChannel(ADC_TEMPERATURE, ADC_CHANNEL, ENABLE);		//Se habilita el canal 0
	NVIC_SetPriority(ADC_IRQN, ADC_INTERRUPT_PRIORITY);	//Se setea la prioridad de la interrupcion
	Chip_ADC_Int_SetChannelCmd(ADC_TEMPERATURE, ADC_CHANNEL, ENABLE);	//Se habilita las interrupciones del canal 0
	NVIC_EnableIRQ(ADC_IRQN);								//Se habilita las interrupciones
}

/* Configuro las interrupciones externas generadas por el detector de fase.*/
void ConfigurePhaseDetector( void ){

	/* Se configura el pin de deteccion para el modo GPIO */
	Chip_SCU_PinMuxSet(PHASE_SCU_INT_PORT, PHASE_SCU_INT_PIN,(SCU_MODE_INBUFF_EN|SCU_MODE_INACT|SCU_MODE_FUNC0) );

	/* Se configura el pin como entrada */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, PHASE_GPIO_INT_PORT, PHASE_GPIO_INT_PIN);

	/* Se configura el canal PININT_INDEX para las interrupciones */
	Chip_SCU_GPIOIntPinSel(PHASE_PININT_INDEX, PHASE_GPIO_INT_PORT, PHASE_GPIO_INT_PIN);

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PHASE_PININT_INDEX));	//Se limpian las interrupciones
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(PHASE_PININT_INDEX));	//Se setea el trigger por flancos
	LPC_GPIO_PIN_INT->SIENR |= PININTCH(PHASE_PININT_INDEX);					//Se setean los flancos ascendentes
	LPC_GPIO_PIN_INT->SIENF |= PININTCH(PHASE_PININT_INDEX);					//Se setean los flancos descendentes

	/* Habilitacion de las interrupciones */
	NVIC_SetPriority(PHASE_IRQN, PHASE_INTERRUPT_PRIORITY);	//Se setea la prioridad de la interrupcion
	NVIC_ClearPendingIRQ(PHASE_IRQN);						//Se limpian las interrupciones pendientes
	NVIC_EnableIRQ(PHASE_IRQN);								//Se habilitan las interrupciones para el pin
}

void ConfigureTriggerResistor( void ){
	/* Configuracion del pin de trigger del triac */
	Chip_SCU_PinMuxSet(TRIGGER_SCU_INT_PORT, TRIGGER_SCU_INT_PIN,(SCU_MODE_INBUFF_EN|SCU_MODE_INACT|SCU_MODE_FUNC0));
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, TRIGGER_GPIO_INT_PORT, TRIGGER_GPIO_INT_PIN);	//Pin de salida
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, TRIGGER_GPIO_INT_PORT, TRIGGER_GPIO_INT_PIN, (bool) false);
}

void ConfigureStartButton( void ){
	/* Se configura el pin de deteccion para el modo GPIO */
	Chip_SCU_PinMuxSet(START_SCU_INT_PORT, START_SCU_INT_PIN,(SCU_MODE_INBUFF_EN|SCU_MODE_INACT|SCU_MODE_FUNC0) );

	/* Se configura el pin como entrada */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, START_GPIO_INT_PORT, START_GPIO_INT_PIN);

	/* Se configura el canal PININT_INDEX para las interrupciones */
	Chip_SCU_GPIOIntPinSel(START_PININT_INDEX, START_GPIO_INT_PORT, START_GPIO_INT_PIN);

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(START_PININT_INDEX));	//Se limpian las interrupciones
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(START_PININT_INDEX));	//Se setea el trigger por flancos
	LPC_GPIO_PIN_INT->SIENR |= PININTCH(START_PININT_INDEX);					//Se setean los flancos ascendentes

	/* Habilitacion de las interrupciones */
	NVIC_SetPriority(START_IRQN, START_INTERRUPT_PRIORITY);	//Se setea la prioridad de la interrupcion
	NVIC_ClearPendingIRQ(START_IRQN);						//Se limpian las interrupciones pendientes
	NVIC_EnableIRQ(START_IRQN);								//Se habilitan las interrupciones para el pin
}

void ConfigureStopButton( void ){
	/* Se configura el pin de deteccion para el modo GPIO */
	Chip_SCU_PinMuxSet(STOP_SCU_INT_PORT, STOP_SCU_INT_PIN,(SCU_MODE_INBUFF_EN|SCU_MODE_INACT|SCU_MODE_FUNC0) );

	/* Se configura el pin como entrada */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, STOP_GPIO_INT_PORT, STOP_GPIO_INT_PIN);

	/* Se configura el canal PININT_INDEX para las interrupciones */
	Chip_SCU_GPIOIntPinSel(STOP_PININT_INDEX, STOP_GPIO_INT_PORT, STOP_GPIO_INT_PIN);

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(STOP_PININT_INDEX));	//Se limpian las interrupciones
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(STOP_PININT_INDEX));	//Se setea el trigger por flancos
	LPC_GPIO_PIN_INT->SIENR |= PININTCH(STOP_PININT_INDEX);					//Se setean los flancos ascendentes

	/* Habilitacion de las interrupciones */
	NVIC_SetPriority(STOP_IRQN, STOP_INTERRUPT_PRIORITY);	//Se setea la prioridad de la interrupcion
	NVIC_ClearPendingIRQ(STOP_IRQN);						//Se limpian las interrupciones pendientes
	NVIC_EnableIRQ(STOP_IRQN);								//Se habilitan las interrupciones para el pin
}
/*****************************************************************************/
