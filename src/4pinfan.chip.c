/*
 * Copyright (C) 2026 B1200 Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include "wokwi-api.h"

typedef struct {
  pin_t pin_fan_tach;
  pin_t pin_fan_pwm;
  uint8_t pin_state_tach;
  uint32_t max_rpm_attr;
  timer_t timer_id_tach;
  uint32_t riseTime;
  uint32_t fallTime;
  uint32_t highTime;
  uint32_t lowTime;
  uint32_t period;
} chip_state_t;

/**
 * @brief Timer callback for fan tachometer emulation
 *
 * This timer callback is used to emulate a fan tachometer. It reads the
 * maximum RPM attribute and calculates the interval between two pulses
 * based on the fan speed. The interval is then used to start a new
 * timer, which will trigger this callback again. The callback also
 * toggles the state of the pin_fan_tach pin.
 *
 * It emulates a 4-pin fan tachometer output, which typically has two pulses
 * per rotation. The frequency of the tachometer signal is calculated based on
 * the maximum RPM attribute. The pulse width is set to 1/4 of the period. This
 * means that there are two HIGH and two LOW pulses per rotation. This is
 * achieved by shifting the frequency by 2 bits. This is a simplification and
 * may not be accurate for all fans, however, it provides a reasonable
 * approximation. The rounding errors help to emulate variation in speed of a
 * real fan.
 *
 * @param user_data Pointer to a chip_state_t struct.
 */
void chip_timer_tach_callback(void* user_data) {
  chip_state_t* data = (chip_state_t*)user_data;

  // rounding errors help to emulate a real fan
  uint16_t max_rpm = (uint16_t)attr_read_float(data->max_rpm_attr);

  uint16_t rpm =
      (uint32_t)max_rpm * (uint32_t)data->highTime / (uint32_t)data->period;

  uint16_t frequency = rpm / 60;  // RPS
  frequency <<= 2;  // * (2*2) Two pulses per rotation * Both a HIGH and LOW
                    // toggle per pulse
  uint32_t interval = 1000000.0f / (float)frequency;  // In microseconds

  pin_write(data->pin_fan_tach, data->pin_state_tach);
  data->pin_state_tach = !data->pin_state_tach;

  timer_start(data->timer_id_tach, interval, false);
}

/**
 * @brief Pin state change callback
 *
 * This callback is used to detect changes in the fan PWM pin state.
 * When the fan PWM pin state changes, it calculates the PWM duty cycle
 * and stores it in the chip_state_t struct.
 *
 * @param user_data Pointer to a chip_state_t struct.
 * @param pin The pin that triggered the callback.
 * @param pin_value The new state of the pin (HIGH or LOW).
 */
void pin_state_change_callback(void* user_data, pin_t pin, uint32_t pin_value) {
  chip_state_t* data = (chip_state_t*)user_data;

  if (!pin_read(data->pin_fan_tach) && (pin == data->pin_fan_pwm)) {
    // PWM changed Let's calculate PWM duty cycle
    uint32_t currentTime = (uint32_t)get_sim_nanos() >> 11;  // In microseconds

    switch (pin_value) {
      case HIGH:
        // Rising edge detected
        data->period = currentTime - data->riseTime;
        data->lowTime = currentTime - data->fallTime;
        data->riseTime = currentTime;
        break;
      case LOW:
        // Falling edge detected
        data->highTime = currentTime - data->riseTime;
        data->fallTime = currentTime;
        break;
      default:
        /* ERROR: It should never get here */
        return;
        break;
    }
  }
}
/**
 * @brief Initializes the chip state.
 *
 * Initializes the chip state by setting up the pin
 * states, attributes, and timers. It also sets up a pin
 * state change callback to detect changes in the fan PWM
 * pin state, and a timer callback to toggle the fan tach
 * pin state based on the fan speed.
 */
void chip_init() {
  // setvbuf(stdout, NULL, _IOLBF, 1024);
  // printf("\nchip_init() begin\n");

  chip_state_t* chip = malloc(sizeof(chip_state_t));

  chip->highTime = 0;
  chip->lowTime = 0;
  chip->riseTime = 0;
  chip->fallTime = 0;
  chip->period = 1;

  chip->max_rpm_attr = (uint32_t)attr_init_float("max_rpm", 5000);

  chip->pin_fan_tach = pin_init("TACH", OUTPUT_LOW);
  chip->pin_fan_pwm = pin_init("PWM", INPUT_PULLUP);

  const pin_watch_config_t pin_change_config = {
      .edge = BOTH,
      .pin_change = pin_state_change_callback,
      .user_data = chip,
  };
  pin_watch(chip->pin_fan_pwm, &pin_change_config);

  const timer_config_t config_tach = {
      .callback = chip_timer_tach_callback,
      .user_data = chip,
  };
  chip->timer_id_tach = timer_init(&config_tach);
  timer_start(chip->timer_id_tach, 500000, false);  // update every 0.5 seconds
}
