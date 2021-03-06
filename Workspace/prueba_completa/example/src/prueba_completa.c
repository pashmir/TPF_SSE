#include "main.h"
/* Definidicion de los perfiles de temperatura*/
struct temperature_profile temperature_profile1, temperature_profile2;

/*****************************************************************************
 * Interrupciones
 ****************************************************************************/
/* Handler de la USART */
void USART_IRQN_HANDLER()
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	control_char=Chip_UART_ReadByte(USART);
	Chip_UART_IntDisable(USART, UART_IER_RBRINT);
	Chip_UART_ReadLineStatus(USART);

	xSemaphoreGiveFromISR(xUSARTSemaphore, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

/* Handler del ADC que mide la temperatura */
void ADC_IRQN_HANDLER(void){
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	Chip_ADC_ReadValue(ADC_TEMPERATURE, ADC_CHANNEL,&adc_data);	//Se hace una nueva lectura de la temperatura

	xSemaphoreGiveFromISR(xADCSemaphore, &xHigherPriorityTaskWoken);	//Se otorga el semaforo
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);		//Se fuerza un cambio de contexto si es necesario
}

/* Handler de las interrupciones del GPIO que detectan los cruces por cero */
void PHASE_IRQN_HANDLER(void){
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PHASE_PININT_INDEX));	//Se limpian las interrupciones

	xSemaphoreGiveFromISR(xPhaseSemaphore, &xHigherPriorityTaskWoken);	//Se otorga el semaforo
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);		//Se fuerza un cambio de contexto si es necesario
}

/*****************************************************************************
 * Tareas
 ****************************************************************************/
static void vHandlerUSART(void *pvParameters){
	ConfigureUSART();

	/* Se toma el semaforo para vaciarlo antes de entrar al loop infinito */
	xSemaphoreTake(xUSARTSemaphore,(portTickType) 0);

	while(1)
	{
		/* La tarea permanece bloqueada hasta que el semaforo se libera */
		xSemaphoreTake(xUSARTSemaphore,portMAX_DELAY);
		if (control_char=='B')
			xSemaphoreGive(xStopSemaphore);
		if (control_char=='A' || control_char=='C')
			xSemaphoreGive(xStartSemaphore);
		Chip_UART_IntEnable(USART, UART_IER_RBRINT);
	}
}

/* Esta tarea se encarga de encender el horno cuando se presiona el boton
 * de encendido. */
static void vHandlerStart(void *pvParameters){

	//ConfigureStartButton();		//Se configura el boton de encendido
    Board_LED_Set(4, false);		//Se apaga el led de encendido
    Board_LED_Set(5, true);			//Se enciende el led de apagado

	/* Se toma el semaforo para vaciarlo antes de entrar al loop infinito */
    xSemaphoreTake(xStartSemaphore, (portTickType) 0);

	/* La tarea permanece bloqueada hasta que el semaforo se libera */
	xSemaphoreTake(xStartSemaphore, portMAX_DELAY);

	/* Cuando se presiona el boton de encendido, se le da el semaforo a la tarea. */

	Board_LED_Set(4, true);			//Se enciende un led para indicar que esta encendido
	Board_LED_Set(5, false);		//Se apaga el led de apagado

	//NVIC_DisableIRQ(START_IRQN);	//Deshabilito las interrupciones del boton de encendido

	//ConfigureStopButton();		//Se configura el boton de apagado
	ConfigureADC();				//Configuracion del ADC
	ConfigurePhaseDetector();	//Configuracion del detector de cruce por cero
	ConfigureTriggerResistor();	//Configuracion del trigger que controla la resistencia

	vSemaphoreCreateBinary(xStopSemaphore);		//Se crea el semaforo para el boton de apagado
	vSemaphoreCreateBinary(xADCSemaphore);		//Se crea el semaforo de la tarea que mide la temperatura
	vSemaphoreCreateBinary(xControllerSemaphore);	//Se crea el semaforo del controlador
	vSemaphoreCreateBinary(xPhaseSemaphore);	//Se crea el semaforo para detectar la fase

	/* Se verifica que el semaforo haya sido creado correctamente */
	if( (xStopSemaphore!=(xSemaphoreHandle)NULL)&&(xADCSemaphore!=(xSemaphoreHandle)NULL)
			&&(xControllerSemaphore!=(xSemaphoreHandle)NULL)&&(xPhaseSemaphore!=(xSemaphoreHandle)NULL) ){

		/* Se crea la tarea de encendido */
		xTaskCreate(vHandlerStop, (char *) "Stop", configMINIMAL_STACK_SIZE,
							(void *) 0, (tskIDLE_PRIORITY + 3UL), &StopTaskHandle );
		/* Se crea la tarea que mide la temperatura */
		xTaskCreate(vHandlerGetTemperature, (char *) "GetTemperature", configMINIMAL_STACK_SIZE,
							(void *) 0, (tskIDLE_PRIORITY + 1UL), &ADCTaskHandle );
		/* Se crea la tarea que controla la resistencia */
		xTaskCreate(vHandlerController, (char *) "Controller", configMINIMAL_STACK_SIZE,
							(void *) 0, (tskIDLE_PRIORITY + 1UL), &ControllerTaskHandle );
		/* Se crea la tarea que detecta los cruces por cero */
		xTaskCreate(vHandlerZeroCrossing, (char *) "ZeroCrossing", configMINIMAL_STACK_SIZE,
							(void *) 0, (tskIDLE_PRIORITY + 2UL), &PhaseTaskHandle );
		/* Se crea la tarea que actualiza la temperatura de referencia con el programa que corresponda*/
		if (control_char=='A')
		xTaskCreate(vHandlerUpdateTemperatureReference, (char *) "UpdateReference", configMINIMAL_STACK_SIZE,
							(void *) &temperature_profile1, (tskIDLE_PRIORITY + 4UL), &UpdateReferenceTaskHandle );
		if (control_char=='C')
			xTaskCreate(vHandlerUpdateTemperatureReference, (char *) "UpdateReference", configMINIMAL_STACK_SIZE,
								(void *) &temperature_profile2, (tskIDLE_PRIORITY + 4UL), &UpdateReferenceTaskHandle );
		oven_state=ON;
		xTaskCreate(vHandlerUSARTTransmit, (char *) "USARTTransmit",configMINIMAL_STACK_SIZE,
							(void *) 0, (tskIDLE_PRIORITY + 1U), &USARTTransmitTaskHandle);
	}

	vTaskSuspend( StartTaskHandle );	//Se suspende la tarea de encendido

	while(1){	//Las tareas y semaforos no deben ser creadas nuevamente
			/* La tarea permanece bloqueada hasta que el semaforo se libera */
			xSemaphoreTake(xStartSemaphore, portMAX_DELAY);

			/* Cuando se presiona el boton de encendido, se le da el semaforo a la tarea. */

			Board_LED_Set(4, true);			//Se enciende un led para indicar que esta encendido
			Board_LED_Set(5, false);		//Se apaga el led de apagado

			NVIC_DisableIRQ(START_IRQN);	//Deshabilito las interrupciones del boton de encendido
	        NVIC_EnableIRQ(STOP_IRQN);	//Habilito las interrupciones del boton de apagado

	        NVIC_EnableIRQ(ADC_IRQN);		//Habilito las interrupciones del ADC
	        NVIC_EnableIRQ(PHASE_IRQN);		//Habilito las interrupciones del detector de fase

			vTaskResume(StopTaskHandle);	//Se desbloquean las tareas
			vTaskResume(ADCTaskHandle);
			vTaskResume(ControllerTaskHandle);
			vTaskResume(PhaseTaskHandle);
			if (control_char=='A')
				xTaskCreate(vHandlerUpdateTemperatureReference, (char *) "UpdateReference", configMINIMAL_STACK_SIZE,
										(void *) &temperature_profile1, (tskIDLE_PRIORITY + 4UL), &UpdateReferenceTaskHandle );
			if (control_char=='C')
				xTaskCreate(vHandlerUpdateTemperatureReference, (char *) "UpdateReference", configMINIMAL_STACK_SIZE,
										(void *) &temperature_profile2, (tskIDLE_PRIORITY + 4UL), &UpdateReferenceTaskHandle );
			oven_state=ON;
			vTaskResume(USARTTransmitTaskHandle);

			vTaskSuspend( StartTaskHandle );	//Se suspende la tarea de encendido
	}

}

/* Esta tarea se encarga de apagar el horno cuando se presiona el boton
 * de apagado. */
static void vHandlerStop(void *pvParameters){

	/* Se toma el semaforo para vaciarlo antes de entrar al loop infinito */
    xSemaphoreTake(xStopSemaphore, (portTickType) 0);

	while (1) {
		/* La tarea permanece bloqueada hasta que el semaforo se libera */
        xSemaphoreTake(xStopSemaphore, portMAX_DELAY);

        /* Cuando se presiona el boton de encendido, se le da el semaforo a la tarea. */

        Board_LED_Set(4, false);	//Se apaga el led de encendido
        Board_LED_Set(5, true);		//Se enciende el led de apagado

        //NVIC_EnableIRQ(START_IRQN);	//Habilito las interrupciones del boton de encendido
        //NVIC_DisableIRQ(STOP_IRQN);	//Deshabilito las interrupciones del boton de apagado

        NVIC_DisableIRQ(ADC_IRQN);		//Deshabilito las interrupciones del ADC
        NVIC_DisableIRQ(PHASE_IRQN);	//Deshabilito las interrupciones del detector de fase

        /* Se suspenden las tareas de control de temperatura */
        vTaskSuspend( ADCTaskHandle );
        vTaskSuspend( ControllerTaskHandle );
        vTaskSuspend( PhaseTaskHandle );
        vTaskDelete( UpdateReferenceTaskHandle );
        //vTaskSuspend( USARTTransmitTaskHandle );
        oven_state=OFF;

        /* Se apaga la resistencia calentadora */
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, TRIGGER_GPIO_INT_PORT, TRIGGER_GPIO_INT_PIN, false);

        vTaskResume( StartTaskHandle );	//Se vuelve a iniciar la tarea de encendido. Se espera que se presione el boton
        vTaskSuspend( StopTaskHandle );	//Se suspende la tarea de encendido
    }
}

/* Esta tarea toma muestras de la temperatura y las promedia para reducir el ruido */
static void vHandlerGetTemperature(void *pvParameters){
	uint8_t i=0;	//Cantidad de mediciones realizadas
	uint16_t aux=0;	//Variable auxiliar para almacenar valores del ADC

	while(1){
		Chip_ADC_SetStartMode(ADC_TEMPERATURE, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		/* La tarea permanece bloqueada hasta que el semaforo se libera */
		xSemaphoreTake(xADCSemaphore, portMAX_DELAY);

		i++;	//Se realizo una nueva medicion
		aux+=adc_data;	//Se suma el nuevo valor a los anteriores

		if( i > MAX_ADC_SAMPLES ){
			i=0;
			thermocouple_temp=aux/MAX_ADC_SAMPLES;	//Se guarda el promedio de las temperaturas
			aux=0;
			xSemaphoreGive( xControllerSemaphore );	//Se le da el semaforo a la tarea del control de encendido
			vTaskDelay( ADC_SAMPLES_DELAY );		//Se bloquea la tarea por un cierto tiempo
		}
	}
}

/* Esta tarea aplica el controlador a las lecturas del ADC .*/
static void vHandlerController(void *pvParameters){

	float temperature;		//Temperatura medida con la termocupla

	/* Se toma el semaforo para vaciarlo antes de entrar al loop infinito */
    xSemaphoreTake(xControllerSemaphore, (portTickType) 0);

	while (1) {

		/* La tarea permanece bloqueada hasta que el semaforo se libera */
        xSemaphoreTake(xControllerSemaphore, portMAX_DELAY);

		temperature = get_temperature( thermocouple_temp );	//Mapeo la temperatura

		if( temperature >= reference-TEMPERATURE_INERTIA )
			trigger_state = FALSE;	//Si la temperatura es mayor, se apaga la potencia
		else
			trigger_state = TRUE;	//Si la temperatura es menor, se mantiene encendida la potencia

    }
}

/* Esta tarea se ejecuta siempre que se produzca un cruce por cero.*/
static void vHandlerZeroCrossing(void *pvParameters){

	/* Se toma el semaforo para vaciarlo antes de entrar al loop infinito */
    xSemaphoreTake(xPhaseSemaphore, (portTickType) 0);

	while (1) {

		/* La tarea permanece bloqueada hasta que el semaforo se libera */
        xSemaphoreTake(xPhaseSemaphore, portMAX_DELAY);

        /* Se dispara el trigger */
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, TRIGGER_GPIO_INT_PORT, TRIGGER_GPIO_INT_PIN, trigger_state);
    }
}

/* Esta tarea actualiza el valor de referencia */
static void vHandlerUpdateTemperatureReference(void *pvParameters){
	uint8_t i=0;
	portTickType xLastExecutionTime;

	xLastExecutionTime = xTaskGetTickCount();
	struct temperature_profile * temp_prof=(struct temperature_profile *)pvParameters;

	while (1) {

		if( i < temp_prof->size ){
			reference=temp_prof->values[i];
			i++;
		}
		else{
			xSemaphoreGive(xStopSemaphore);	//Se otorga el semaforo para apagar el horno
			i=0;
		}
		vTaskDelayUntil(&xLastExecutionTime, REFERENCE_INTERVAL);

    }
}

static void vHandlerUSARTTransmit(void *pvParameters)
{
	uint8_t temperature;
	portTickType xLastExecutionTime;
	xLastExecutionTime = xTaskGetTickCount();
	while(1)
	{
		if (oven_state==OFF)
		{
			Chip_UART_SendByte(USART,temperature);
			Chip_UART_SendByte(USART,'F');
			vTaskSuspend( USARTTransmitTaskHandle );
		}
		temperature = (uint8_t) get_temperature( thermocouple_temp );
		Chip_UART_SendByte(USART,temperature);
		Chip_UART_SendByte(USART,'\n');
		vTaskDelayUntil(&xLastExecutionTime, 7000 / portTICK_RATE_MS); //envia la data cada 6 seg
	}
}

/*****************************************************************************
 * Funcion main
 ****************************************************************************/
int main(void){
	SetupHardware();	//Se inicializa el hardware
	temperature_profile1.values = &temperature_profile1_val;
	temperature_profile1.size = sizeof(temperature_profile1_val)/sizeof(float);
	temperature_profile2.values = &temperature_profile2_val;
	temperature_profile2.size = sizeof(temperature_profile2_val)/sizeof(float);

	vSemaphoreCreateBinary(xUSARTSemaphore);		//Se crea el semaforo que recibe de la usart
	vSemaphoreCreateBinary(xStartSemaphore);

	/* Se verifica que el semaforo haya sido creado correctamente */
	if( (xUSARTSemaphore!=(xSemaphoreHandle)NULL) && (xStartSemaphore!=(xSemaphoreHandle)NULL)){

		/* Se crea la tarea de USART */
				xTaskCreate(vHandlerUSART, (char *) "USART", configMINIMAL_STACK_SIZE,
									(void *) 0, (tskIDLE_PRIORITY + 4UL), &USARTTaskHandle );
		/* Se crea la tarea de USART */
				xTaskCreate(vHandlerStart, (char *) "START", configMINIMAL_STACK_SIZE,
									(void *) 0, (tskIDLE_PRIORITY + 4UL), &StartTaskHandle );


		vTaskStartScheduler(); /* Se comienzan a ejecutar las tareas. */
	}
	while (1);	//No se deberia llegar nunca a este punto.
return ((int) NULL);
}
