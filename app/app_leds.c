#include "app_leds.h"
#include "freertos.h"
#include "cmsis_os.h"
#include "common.h"

#define NUM_TIMERS 5

TimerHandle_t xLedsTimerHandle;

void ledsTimerCallback(TimerHandle_t xTimer) {
	static unsigned int counter = 0;

	counter++;

	//Check configuration state
	switch (decodedConfig.state)
	{
		case CONF_CORRECT:
		{
			Led_TurnOn(2);
			break;
		}
		case CONF_RETRIEVING | CONF_BUILDING:
		{
			if (counter%4)
			{
				Led_Toggle(2);
			}
			break;
		}
		default:
		{
			Led_Toggle(2);
			break;
		}
	}

	if (counter >= 100)
	{
		counter = 1;
	}

	/* Optionally do something if the pxTimer parameter is NULL. */
	configASSERT(xTimer);


}

void leds_timer_init(void) {
	xLedsTimerHandle = xTimerCreate( /* Just a text name, not used by the RTOS
		 kernel. */
		"Timer",
		/* The timer period in ticks, must be
		 greater than 0. */
		100,
		/* The timers will auto-reload themselves
		 when they expire. */
		pdTRUE,
		/* The ID is used to store a count of the
		 number of times the timer has expired, which
		 is initialised to 0. */
		(void *) 0,
		/* Each timer calls the same callback when
		 it expires. */
		ledsTimerCallback);

		if (xLedsTimerHandle == NULL) {
			/* The timer was not created. */
		} else {
			/* Start the timer.  No block time is specified, and
			 even if one was it would be ignored because the RTOS
			 scheduler has not yet been started. */
			if (xTimerStart(xLedsTimerHandle, 0) != pdPASS) {
				/* The timer could not be set into the Active
				 state. */
			}
		}
}
