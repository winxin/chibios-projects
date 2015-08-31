/*
 * (c) 2015 flabbergast <s3+flabbergast@sdfeu.org>
 * Based on ChibiOS 3.0.1 demo code, license below.
 * Licensed under the Apache License, Version 2.0.
 */

/*
 *  ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"

/*
 * Amber LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
  (void)arg;
  chRegSetThreadName("blinker");
  while(true) {
    palClearPad(GPIOA, GPIOA_LED_AMBER);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOA, GPIOA_LED_AMBER);
    chThdSleepMilliseconds(500);
  }
}

static THD_WORKING_AREA(waSerEcho, 128);
static THD_FUNCTION(thSerEcho, arg) {
  (void)arg;
  chRegSetThreadName("SerEcho");
  event_listener_t serial_listener;
  eventflags_t flags;
  chEvtRegisterMask((event_source_t *)chnGetEventSource(&SD1), &serial_listener, EVENT_MASK(1));

  while(true) {
    chEvtWaitOneTimeout(EVENT_MASK(1), MS2ST(10));
    flags = chEvtGetAndClearFlags(&serial_listener);
    if(flags & CHN_INPUT_AVAILABLE) {
      msg_t charbuf;
      do {
        charbuf = chnGetTimeout(&SD1, TIME_IMMEDIATE);
        if(charbuf != Q_TIMEOUT) {
          chSequentialStreamPut(&SD1, charbuf);
        }
      } while(charbuf != Q_TIMEOUT);
    }
  }
}

/*
 * Application entry point.
 */
int main(void) {
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
   * Activates the serial driver 1 using the driver default configuration.
   */
  sdStart(&SD1, NULL);

  /*
   * Creates the serial echo thread.
   */
  chThdCreateStatic(waSerEcho, sizeof(waSerEcho), NORMALPRIO, thSerEcho, NULL);

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while(true) {
    if(palReadPad(GPIOB, GPIOB_BUTTON) == PAL_HIGH) {
      sdWrite(&SD1, (uint8_t *)"hello world\r\n", 13);
      /* chprintf((BaseSequentialStream *)&SD1, "Hello world\r\n"); */
      /* chnWrite((BaseChannel *)&SD1, (uint8_t *)"Hello, world\r\n", 14); */
    }
    chThdSleepMilliseconds(200);
  }
}
