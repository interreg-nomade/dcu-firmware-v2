#include "FreeRTOS.h"

#include "app_usbad.h"
#include "data/structures.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "project_config.h"
#include "common.h"

#include "queues/notification/notification_service_queue.h"

#include "data_op/op.h"

#define 	PRINT_RTOS_TASK_USBAD	0

static instrument_config_t * pUsbAdInstrument;

static void usbadTask(const void * params);
static osThreadId usbadTaskHandle;

void usbad_task_init(void)
{
	osThreadDef(usbad, usbadTask, osPriorityBelowNormal, 0, 256);
	usbadTaskHandle = osThreadCreate(osThread(usbad), NULL);
}

void USBAD_Config_Init(void)
{
	/* Find if an Android Device (AD) is present in the configuration file */
	CONFIG_WAITING_FOR_DECODE(); /* Block on this line untill a configuration is decoded and valid */

	/* New configuration retrieved. */
	int32_t n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_USBAD);

	/* One USB AD instrument detected in the config */
	if (n==1)
	{
		/* Retrieved one usb ad instrument */
		n = getInstrumentFromConfig(&decodedConfig.conf, &pUsbAdInstrument, SETUP_PRM_COMM_METHOD_USBAD);
		/* Initialize the notification queue */
		notification_service_queue_init(10, sizeof(usbad_data_t));

		if (n == 1)
		{
			/* Correctly retrieved instrument pointer */
		}
	}
	else
	{
		/* No AD instrument found delete the task related to it */
		/* Or too many AD device instrument found which is not possible */
		osThreadTerminate(usbadTaskHandle);
	}

}

static void usbadTask(const void * params)
{
	USBAD_Config_Init();

	static usbad_data_t previousUsbadfeedback = {
			.cycleCounter = 0x00,
			.OAScalculatedValue = 0x00,
			.feedbackStatue = 0x00,
			.sensorActivate = 0x00
	};

	usbad_data_t * usbadfeedback = NULL;

	if(pUsbAdInstrument)
	{
		usbadfeedback = (usbad_data_t *) pUsbAdInstrument->data;
	}

	for(;;)
	{
		/* The task block until it receives data from the AD */
		notification_service_receive(usbadfeedback, sizeof(usbad_data_t), portMAX_DELAY);
		usbadfeedback->cycleCounter = swap_uint32(usbadfeedback->cycleCounter);

		if(usbadfeedback->feedbackStatue != previousUsbadfeedback.feedbackStatue)
		{
#if PRINT_RTOS_TASK_USBAD
			printf("Android device as instrument:\nBuzzer status : %d, Haptic staus : %d; Visual status : %d, OAS calculated value : %d\n",
					usbadfeedback.buzzer, usbadfeedback.haptic, usbadfeedback.visual, usbadfeedback.OAScalculatedValue);
#endif
			previousUsbadfeedback = *usbadfeedback;
		}
		else
		{
#if PRINT_RTOS_TASK_USBAD
			printf("OAS calculated value : %x\n", usbadfeedback.OAScalculatedValue);
#endif
		}
		//TODO : ADD the part relating to the haptic feedback
		if(usbadfeedback->haptic)
		{
			if(usbadfeedback->OAScalculatedValue)
			{
				//Haptic_TurnOn();
			}
			else
			{
				//Haptic_TurnOff();
			}
		}
		else
		{
			//Haptic_TurnOff();
		}
	}
}
