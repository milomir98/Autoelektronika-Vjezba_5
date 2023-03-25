// Driver includes
#include "HWS_Driver/HWS_conf.h"
static const char character[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x06D, 0x7D, 0x07, 0x7F, 0x6F };

// Priorities at which the tasks are created
#define task_prioritet		( tskIDLE_PRIORITY + 2 )

// Kernel includes
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "I_Vezba.h"

static mxDisp7seg_Handle myDisp;
static TaskHandle_t task1, task2, task3, task4;
static TimerHandle_t timer1;
static SemaphoreHandle_t binSem1, binSem2, binSem3, binSem4;

static uint8_t dispMem[4] = {0, 0, 0, 0};

// Local function declaration
static void OnlyTask(void* pvParams);
static void TimerCallback(TimerHandle_t tmr);

// Local function implementation
static void OnlyTask(void* pvParams) {
	uint8_t TASK_ID;
	while (1) {
		TASK_ID = (uint8_t)pvParams;
		if (TASK_ID == 1) {			// PRVI TASK
			xSemaphoreTake(binSem1, portMAX_DELAY);
			dispMem[0]++;
			if (dispMem[0] == 10) {
				dispMem[0] = 0;
				xSemaphoreGive(binSem2);
			}
		}
		if (TASK_ID == 2) {			// DRUGI TASK
			xSemaphoreTake(binSem2, portMAX_DELAY);
			dispMem[1]++;
			if (dispMem[1] == 10) {
				dispMem[1] = 0;
				xSemaphoreGive(binSem3);
			}
		}
		if (TASK_ID == 3) {			// TRECI TASK
			xSemaphoreTake(binSem3, portMAX_DELAY);
			dispMem[2]++;
			if (dispMem[2] == 10) {
				dispMem[2] = 0;
				xSemaphoreGive(binSem4);
			}
		}
		if (TASK_ID == 4) {			// CETVRTI TASK
			xSemaphoreTake(binSem3, portMAX_DELAY);
			dispMem[3]++;
			if (dispMem[3] == 10) {
				dispMem[3] = 0;
			}
		}
	}
}
static void TimerCallback(TimerHandle_t tmr) {
	static uint8_t ctrl = 4;
	static uint8_t count = 0;

	mxDisp7seg_SelectDigit(myDisp, ctrl);
	mxDisp7seg_SetDigit(myDisp, character[dispMem[4 - ctrl]]);
	ctrl--;
	if (ctrl == 0)
		ctrl = 4;

	count++;
	if (count == 10) { // PERIOD = 20ms * count = 200ms
		count = 0;
		xSemaphoreGive(binSem1);
	}
}


void IV_vezba_1(void)
{
	// Inicijalizacija drajvera za displej
	mxDisp7seg_Init();
	myDisp = mxDisp7seg_Open(MX7_DISPLAY_0);

	// Kreiranje taskova 
	if (xTaskCreate(OnlyTask, NULL, configMINIMAL_STACK_SIZE, (void*)1, task_prioritet, &task1) != pdPASS)
		while (1);
	if (xTaskCreate(OnlyTask, NULL, configMINIMAL_STACK_SIZE, (void*)2, task_prioritet, &task2) != pdPASS)
		while (1);
	if (xTaskCreate(OnlyTask, NULL, configMINIMAL_STACK_SIZE, (void*)3, task_prioritet, &task3) != pdPASS)
		while (1);
	if (xTaskCreate(OnlyTask, NULL, configMINIMAL_STACK_SIZE, (void*)4, task_prioritet, &task4) != pdPASS)
		while (1);

	// Kreiranje softverskih tajmera 
	timer1 = xTimerCreate(NULL, pdMS_TO_TICKS(20), pdTRUE, NULL, TimerCallback);
	if(timer1 == NULL)
		while (1);
	xTimerStart(timer1, portMAX_DELAY);
	
	// Kreiranje binarnih semafora 
	binSem1 = xSemaphoreCreateBinary();
	if (binSem1 == NULL)
		while (1);
	binSem2 = xSemaphoreCreateBinary();
	if (binSem1 == NULL)
		while (1);
	binSem3 = xSemaphoreCreateBinary();
	if (binSem1 == NULL)
		while (1);
	binSem4 = xSemaphoreCreateBinary();
	if (binSem1 == NULL)
		while (1);

	// Pokretanje rasporedjivaca
	vTaskStartScheduler();
	while (1);
}