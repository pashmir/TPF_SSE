#include "main.h"

/*****************************************************************************
 * Interrupciones
 ****************************************************************************/
/* Interrupcion del boton de encendido */
void START_IRQN_HANDLER(void){
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(START_PININT_INDEX));	//Se limpian las interrupciones

	xSemaphoreGiveFromISR(xStartSemaphore, &xHigherPriorityTaskWoken);	//Se otorga el semaforo
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);		//Se fuerza un cambio de contexto si es necesario
}

/* Interrupcion del boton de apagado */
void STOP_IRQN_HANDLER(void){
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(STOP_PININT_INDEX));	//Se limpian las interrupciones

	xSemaphoreGiveFromISR(xStopSemaphore, &xHigherPriorityTaskWoken);	//Se otorga el semaforo
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);		//Se fuerza un cambio de contexto si es necesario
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

/* Esta tarea se encarga de encender el horno cuando se presiona el boton
 * de encendido. */
static void vHandlerStart(void *pvParameters){

	ConfigureStartButton();		//Se configura el boton de encendido
    Board_LED_Set(4, false);		//Se apaga el led de encendido
    Board_LED_Set(5, true);			//Se enciende el led de apagado

	/* Se toma el semaforo para vaciarlo antes de entrar al loop infinito */
    xSemaphoreTake(xStartSemaphore, (portTickType) 0);

	/* La tarea permanece bloqueada hasta que el semaforo se libera */
	xSemaphoreTake(xStartSemaphore, portMAX_DELAY);

	/* Cuando se presiona el boton de encendido, se le da el semaforo a la tarea. */

	Board_LED_Set(4, true);			//Se enciende un led para indicar que esta encendido
	Board_LED_Set(5, false);		//Se apaga el led de apagado

	NVIC_DisableIRQ(START_IRQN);	//Deshabilito las interrupciones del boton de encendido

	ConfigureStopButton();		//Se configura el boton de apagado
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
		/* Se crea la tarea que actualiza la temperatura de referencia */
		xTaskCreate(vHandlerUpdateTemperatureReference, (char *) "UpdateReference", configMINIMAL_STACK_SIZE,
							(void *) 0, (tskIDLE_PRIORITY + 4UL), &UpdateReferenceTaskHandle );
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
			vTaskResume(UpdateReferenceTaskHandle);

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

        NVIC_EnableIRQ(START_IRQN);	//Habilito las interrupciones del boton de encendido
        NVIC_DisableIRQ(STOP_IRQN);	//Deshabilito las interrupciones del boton de apagado

        NVIC_DisableIRQ(ADC_IRQN);		//Deshabilito las interrupciones del ADC
        NVIC_DisableIRQ(PHASE_IRQN);	//Deshabilito las interrupciones del detector de fase

        /* Se suspenden las tareas de control de temperatura */
        vTaskSuspend( ADCTaskHandle );
        vTaskSuspend( ControllerTaskHandle );
        vTaskSuspend( PhaseTaskHandle );
        vTaskSuspend( UpdateReferenceTaskHandle );

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

	while (1) {

		if( i < sizeof(temperature_profile)/sizeof(float) ){
			reference=temperature_profile[i];
			i++;
		}
		else{
			xSemaphoreGive(xStopSemaphore);	//Se otorga el semaforo para apagar el horno
			i=0;
		}
		vTaskDelayUntil(&xLastExecutionTime, REFERENCE_INTERVAL);

    }
}


/*****************************************************************************
 * Funcion main
 ****************************************************************************/
int main(void){
	SetupHardware();	//Se inicializa el hardware

	vSemaphoreCreateBinary(xStartSemaphore);		//Se crea el semaforo para el encendido

	/* Se verifica que el semaforo haya sido creado correctamente */
	if( (xStartSemaphore!=(xSemaphoreHandle)NULL)){

		/* Se crea la tarea de encendido */
		xTaskCreate(vHandlerStart, (char *) "Start", configMINIMAL_STACK_SIZE,
							(void *) 0, (tskIDLE_PRIORITY + 4UL), &StartTaskHandle );


		vTaskStartScheduler(); /* Se comienzan a ejecutar las tareas. */
	}
	while (1);	//No se deberia llegar nunca a este punto.
return ((int) NULL);
}
