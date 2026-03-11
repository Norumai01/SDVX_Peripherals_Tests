//
// Created by norumai on 3/11/2026.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define ENC_A     10
#define ENC_B     11
#define ENC_C     12  // encoder push button

volatile int position = 0;

void encoder_isr (uint gpio, uint32_t events) {
  bool a = gpio_get(ENC_A);
  bool b = gpio_get(ENC_B);
  position += (a != b) ? 1 : -1;
}

int main() {
  stdio_init_all();

  // Initialize service
  sleep_ms(10000);
  printf("Encoder Test ready!\n");

  // Encoder channels
  gpio_init(ENC_A);
  gpio_init(ENC_B);
  gpio_set_dir(ENC_A, GPIO_IN);
  gpio_set_dir(ENC_B, GPIO_IN);
  gpio_pull_up(ENC_A);
  gpio_pull_up(ENC_B);

  // Encoder button
  gpio_init(ENC_C);
  gpio_set_dir(ENC_C, GPIO_IN);
  gpio_pull_up(ENC_C);

  gpio_set_irq_enabled_with_callback(ENC_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
    true, &encoder_isr);

  bool last_button = true;
  int last_pos = 0;

  while (true) {
    // Encoder switch (push click)
    bool button = gpio_get(ENC_C);
    if (button != last_button) {
      printf(button ? "Encoder click RELEASED\n" : "Encoder CLICKED\n");
      last_button = button;
      sleep_ms(10);
    }

    // Encoder rotation
    if (position != last_pos) {
      printf("Encoder position: %d\n", position);
      last_pos = position;
    }

    sleep_ms(5);
  }
}