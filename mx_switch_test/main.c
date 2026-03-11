//
// Created by norumai on 3/11/2026.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BUTTON_PIN 0
#define LED_PIN 25

int main() {
  stdio_init_all();

  sleep_ms(2000);  // delay for testing serial monitor
  printf("Ready!\n");

  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  bool last_state = true;

  while (true) {
    bool current_state = gpio_get(BUTTON_PIN);

    if (current_state != last_state) {
      if (!current_state) {
        printf("Button PRESSED\n");
        gpio_put(LED_PIN, 1);
      } else {
        printf("Button RELEASED\n");
        gpio_put(LED_PIN, 0);
      }
      last_state = current_state;
      sleep_ms(10);
    }
  }
}
