/*
 * syscall.c

 *
 *  Created on: Nov 7, 2018
 *      Author: aclem
 */


#include  <errno.h>
#include  <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
#include "project_config.h"

#if RTT_DBG_MSG_ENABLE
	#include "RTT/SEGGER_RTT.h"
#endif

int _write(int file, char *data, int len)
{
   if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
   {
      errno = EBADF;
      return -1;
   }

   //SEGGER_RTT_Write(0, data, len);

   // return # of bytes written - as best we can tell
#if RTT_DBG_MSG_ENABLE
   return SEGGER_RTT_Write(0, data, len);
#else
   return len;
#endif
}
