/**
 * Copyright (c) 2017 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "app_util_platform.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrfx_gpiote.h"
#include "nrfx_timer.h"

#include "id130c_lcd.h"
#include "id130c_pins.h"

#define BACKLIGHT_TIMER_DURATION_MS 5000

#define BACKLIGHT_TIMER_INSTANCE 0
static const nrfx_timer_t backlight_timer_instance = NRFX_TIMER_INSTANCE(BACKLIGHT_TIMER_INSTANCE);


void pin_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  switch(pin) {
  case ID130C_PIN_TOUCH_INT_N :
    nrfx_gpiote_out_clear(ID130C_PIN_LCD_BL_EN);
    nrfx_timer_enable(&backlight_timer_instance);
    break;
  default:
    break;
  }
}

void timer_handler(nrf_timer_event_t event_type, void* p_context) {
  uint32_t timer = (uint32_t) p_context;
  if (timer == BACKLIGHT_TIMER_INSTANCE && event_type == NRF_TIMER_EVENT_COMPARE0) {
    nrfx_gpiote_out_set(ID130C_PIN_LCD_BL_EN);
  }
}

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
  while(1);
}

int main(void)
{
  APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  
  NRF_LOG_INFO("Demo application started.");
  NRF_LOG_FLUSH();

  // misc init
  APP_ERROR_CHECK(nrfx_gpiote_init());

  // backlight timer
  nrfx_timer_config_t backlight_timer_cfg = {.frequency=NRF_TIMER_FREQ_31250Hz,
					     .mode=NRF_TIMER_MODE_TIMER,
					     .bit_width=NRF_TIMER_BIT_WIDTH_32,
					     .interrupt_priority=NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
					     .p_context=BACKLIGHT_TIMER_INSTANCE};
  APP_ERROR_CHECK(nrfx_timer_init(&backlight_timer_instance, &backlight_timer_cfg, timer_handler));
  uint32_t time_ticks = nrfx_timer_ms_to_ticks(&backlight_timer_instance, BACKLIGHT_TIMER_DURATION_MS);
  nrfx_timer_extended_compare(&backlight_timer_instance, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK | NRF_TIMER_SHORT_COMPARE0_STOP_MASK, true);
 
  // touch init
  nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(false);
  out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
  nrfx_gpiote_out_init(ID130C_PIN_TOUCH_EN_N, &out_config);
  nrfx_gpiote_in_config_t in_config_hitolo = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
  in_config_hitolo.pull = NRF_GPIO_PIN_PULLUP;
  APP_ERROR_CHECK(nrfx_gpiote_in_init(ID130C_PIN_TOUCH_INT_N, &in_config_hitolo, pin_handler));
  nrfx_gpiote_in_event_enable(ID130C_PIN_TOUCH_INT_N, true);

  id130c_lcd_init();

  while (1) {
    id130c_lcd_demo();
  }
}

