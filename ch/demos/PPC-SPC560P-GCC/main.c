/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "shell.h"
#include "chprintf.h"

#define SHELL_WA_SIZE   THD_WA_SIZE(1024)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state time\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.sp,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: test\r\n");
    return;
  }
  tp = chThdCreateFromHeap(NULL, TEST_WA_SIZE, chThdGetPriority(),
                           TestThread, chp);
  if (tp == NULL) {
    chprintf(chp, "out of memory\r\n");
    return;
  }
  chThdWait(tp);
}

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"test", cmd_test},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD1,
  commands
};

/*
 * LEDs blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");

  while (TRUE) {
    unsigned i;

    for (i = 0; i < 4; i++) {
      palClearPad(PD, PD_LED1);
      chThdSleepMilliseconds(100);
      palClearPad(PD, PD_LED2);
      chThdSleepMilliseconds(100);
      palClearPad(PD, PD_LED3);
      chThdSleepMilliseconds(100);
      palClearPad(PD, PD_LED4);
      chThdSleepMilliseconds(100);
      palSetPad(PD, PD_LED1);
      chThdSleepMilliseconds(100);
      palSetPad(PD, PD_LED2);
      chThdSleepMilliseconds(100);
      palSetPad(PD, PD_LED3);
      chThdSleepMilliseconds(100);
      palSetPad(PD, PD_LED4);
      chThdSleepMilliseconds(300);
    }

    for (i = 0; i < 4; i++) {
      palTogglePort(PD, PAL_PORT_BIT(PD_LED1) | PAL_PORT_BIT(PD_LED2) |
                        PAL_PORT_BIT(PD_LED3) | PAL_PORT_BIT(PD_LED4));
      chThdSleepMilliseconds(500);
      palTogglePort(PD, PAL_PORT_BIT(PD_LED1) | PAL_PORT_BIT(PD_LED2) |
                        PAL_PORT_BIT(PD_LED3) | PAL_PORT_BIT(PD_LED4));
      chThdSleepMilliseconds(500);
    }

    for (i = 0; i < 4; i++) {
      palTogglePad(PD, PD_LED1);
      chThdSleepMilliseconds(250);
      palTogglePad(PD, PD_LED1);
      palTogglePad(PD, PD_LED2);
      chThdSleepMilliseconds(250);
      palTogglePad(PD, PD_LED2);
      palTogglePad(PD, PD_LED3);
      chThdSleepMilliseconds(250);
      palTogglePad(PD, PD_LED3);
      palTogglePad(PD, PD_LED4);
      chThdSleepMilliseconds(250);
      palTogglePad(PD, PD_LED4);
    }

    for (i = 0; i < 4; i++) {
      palClearPort(PD, PAL_PORT_BIT(PD_LED1) | PAL_PORT_BIT(PD_LED3));
      palSetPort(PD, PAL_PORT_BIT(PD_LED2) | PAL_PORT_BIT(PD_LED4));
      chThdSleepMilliseconds(500);
      palClearPort(PD, PAL_PORT_BIT(PD_LED2) | PAL_PORT_BIT(PD_LED4));
      palSetPort(PD, PAL_PORT_BIT(PD_LED1) | PAL_PORT_BIT(PD_LED3));
      chThdSleepMilliseconds(500);
    }

    palSetPort(PD, PAL_PORT_BIT(PD_LED1) | PAL_PORT_BIT(PD_LED2) |
                   PAL_PORT_BIT(PD_LED3) | PAL_PORT_BIT(PD_LED4));
  }
  return 0;
}

/*
 * Application entry point.
 */
int main(void) {
  Thread *shelltp = NULL;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Shell manager initialization.
   */
  shellInit();

  /*
   * Activates the serial driver 1 using the driver default configuration.
   */
  sdStart(&SD1, NULL);

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity.
   */
  while (TRUE) {

    if (!shelltp)
      shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    else if (chThdTerminated(shelltp)) {
      chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
      shelltp = NULL;           /* Triggers spawning of a new shell.        */
    }
    chThdSleepMilliseconds(1000);
  }
  return 0;
}
