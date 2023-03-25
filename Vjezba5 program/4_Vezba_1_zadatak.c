/* Driver includes */
#include "HWS_Driver/HWS_conf.h"
static const char character[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x06D, 0x7D, 0x07, 0x7F, 0x6F };
#define MAX_SEM_COUNT 10

/* Priorities at which the tasks are created. */
#define task_prioritet		( tskIDLE_PRIORITY + 2 )

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "I_Vezba.h"
static mxDisp7seg_Handle myDisp;
static SemaphoreHandle_t binSem1;
static TimerHandle_t myTimer1, MyTimer2;
static TaskHandle_t tA, tB, tC, tD;
/* A software timer that is started from the tick hook. */

static BaseType_t myTask;

//local function declaration


static void TimerCallback(TimerHandle_t tmH);

typedef struct taskInfo_ {
	SemaphoreHandle_t clk_in;
	SemaphoreHandle_t clk_out;
	unsigned char* disp;
}taskInfo;
static unsigned char dispMem[4];
static SemaphoreHandle_t s1, s2, s3, s4;


static void TimerCallback(TimerHandle_t tmH)
{
	static unsigned char count = 0;
	static unsigned char secount = 0;

	mxDisp7seg_SelectDigit(myDisp, (3 - count));
	mxDisp7seg_SetDigit(myDisp, character[dispMem[count]]);
	count++;
	count &= 0x03;
	secount++;
	if (secount == 25) {
		secount = 0;
		xSemaphoreGive(s1);
	}
	//za zadatak pod b)
	//mxDisp7seg_SelectDigit(myDisp, 4);// selektovan 5 displej
	//mxDisp7seg_SetDigit(myDisp, character[uxTaskGetNumberOfTasks()]);// ispisujemo broj taskova
}



static void only_tsk(void* pvParams)
{
	unsigned char cifra = 0;
	taskInfo* tinf_local;
	while (1) {
		
		tinf_local= (taskInfo*)pvParams;// castujemo iz pointera  void*  u pointer  taskInfo*
		
		xSemaphoreTake(tinf_local->clk_in, portMAX_DELAY);
		cifra++;
		
		if (cifra == 10) { // ako je broj dostigao vrednost 10, treba da se inkrementira vrednost vise cifre na displeju
			cifra = 0;
			xSemaphoreGive(tinf_local->clk_out);// dajemo takstni signal visoj cifri preko semafora 
		}
		
		*(tinf_local->disp) = cifra;// upisujemo cifru u odgovarajuci clan niza dispMem

		//za zadatak pod b)
		/*if (dispMem[1] == 9 && tC != NULL) //ako je dostigao broj 90 i ako vec nije obrisan
		{
			vTaskDelete(tC); tC = NULL;
		}*/
	}
}

void IV_vezba_1(void)
{
	/* Inicijalizacija drajvera za displej*/
	mxDisp7seg_Init();
	myDisp = mxDisp7seg_Open(MX7_DISPLAY_0);
	// initialize display to 0000
	dispMem[0] = 0;
	dispMem[1] = 0;
	dispMem[2] = 0;
	dispMem[3] = 0;
	
	TimerHandle_t tH;

	

	// -------------- CREATE TASKS AND OTHER OBJECTS HERE --------

	tH = xTimerCreate(NULL, pdMS_TO_TICKS(20), pdTRUE, NULL, TimerCallback);
	if (tH == NULL)
		while (1);
	xTimerStart(tH, 0);
	
	s1 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s1 == NULL)
		while (1);

	s2 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s2 == NULL)
		while (1);

	s3 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s3 == NULL)
		while (1);

	s4 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s4 == NULL)
		while (1);

	taskInfo  tinf1 =
	{
	  s1,
	  s2,
	  &dispMem[0]// moglo je i :   dispMem
	};
	taskInfo  tinf2 =	{ s2, s3,&dispMem[1] }; //moglo je i : dispMem+1
	taskInfo  tinf3 ={ s3,s4,&dispMem[2] };  ////moglo je i : dispMem+2
	taskInfo  tinf4 ={ s4,  NULL,  &dispMem[3] }; //moglo je i : dispMem+3
	

	if (xTaskCreate(only_tsk, NULL, configMINIMAL_STACK_SIZE, &tinf1, 2, &tA) != pdPASS)
		while (1);
	if (xTaskCreate(only_tsk, NULL, configMINIMAL_STACK_SIZE, &tinf2, 2, &tB) != pdPASS)
		while (1);
	if (xTaskCreate(only_tsk, NULL, configMINIMAL_STACK_SIZE, &tinf3, 2, &tC) != pdPASS)
		while (1);
	if (xTaskCreate(only_tsk, NULL, configMINIMAL_STACK_SIZE, &tinf4, 2, &tD) != pdPASS)
		while (1);

	
	
	
	// -----------------------------------------------------------

	vTaskStartScheduler();
	while (1);
}