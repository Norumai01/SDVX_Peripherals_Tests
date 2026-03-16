//
// main.c
// SDVX HID test — 1 button + 1 rotary encoder as a USB gamepad
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "hardware/gpio.h"

//--------------------------------------------------------------------+
// Pin Definitions
//--------------------------------------------------------------------+

#define BUTTON_START  0
#define BUTTON_BT_A   3
#define BUTTON_BT_B   4
#define BUTTON_BT_C   28
#define BUTTON_BT_D   27
#define BUTTON_FX_L   5
#define BUTTON_FX_R   26

#define ENC_L_A       10 // VOL-L button
#define ENC_L_B       11
#define ENC_L_SW      12  // encoder push button
#define ENC_R_A       21  // VOL-R button
#define ENC_R_B       20
#define ENC_R_SW      19  // encoder push button


//--------------------------------------------------------------------+
// Encoder State
//--------------------------------------------------------------------+

volatile int enc_l_position = 0;
volatile int enc_r_position = 0;

void encoder_isr(uint gpio, uint32_t events)
{
  switch (gpio) {
    case ENC_L_A: {
      bool a = gpio_get(ENC_L_A);
      bool b = gpio_get(ENC_L_B);
      enc_l_position += (a != b) ? 1 : -1;
      break;
    }
    case ENC_R_A: {
      bool a = gpio_get(ENC_R_A);
      bool b = gpio_get(ENC_R_B);
      enc_r_position += (a != b) ? 1 : -1;
      break;
    }
    default:
      return;
  }
}

//--------------------------------------------------------------------+
// GPIO Setup
//--------------------------------------------------------------------+

static void gpio_setup(void)
{
  // Buttons
  const uint button_pins[] = {
    BUTTON_START, BUTTON_BT_A, BUTTON_BT_B, BUTTON_BT_C,
    BUTTON_BT_D, BUTTON_FX_L, BUTTON_FX_R
  };

  for (int i = 0; i < 7; i++) {
    gpio_init(button_pins[i]);
    gpio_set_dir(button_pins[i], GPIO_IN);
    gpio_pull_up(button_pins[i]);
  }

  // Encoder L
  gpio_init(ENC_L_A);
  gpio_init(ENC_L_B);
  gpio_set_dir(ENC_L_A, GPIO_IN);
  gpio_set_dir(ENC_L_B, GPIO_IN);
  gpio_pull_up(ENC_L_A);
  gpio_pull_up(ENC_L_B);

  gpio_init(ENC_L_SW);
  gpio_set_dir(ENC_L_SW, GPIO_IN);
  gpio_pull_up(ENC_L_SW);

  // Encoder R
  gpio_init(ENC_R_A);
  gpio_init(ENC_R_B);
  gpio_set_dir(ENC_R_A, GPIO_IN);
  gpio_set_dir(ENC_R_B, GPIO_IN);
  gpio_pull_up(ENC_R_A);
  gpio_pull_up(ENC_R_B);

  gpio_init(ENC_R_SW);
  gpio_set_dir(ENC_R_SW, GPIO_IN);
  gpio_pull_up(ENC_R_SW);

  // Interrupts — register callback once, then enable second pin separately
  gpio_set_irq_enabled_with_callback(ENC_L_A,
    GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_isr);

  gpio_set_irq_enabled(ENC_R_A,
    GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

//--------------------------------------------------------------------+
// Blink
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = !led_state; // toggle
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) { blink_interval_ms = BLINK_MOUNTED; }

// Invoked when device is unmounted
void tud_umount_cb(void) { blink_interval_ms = BLINK_NOT_MOUNTED; }

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// HID Task — runs every 10ms
//--------------------------------------------------------------------+

void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  if (!tud_hid_ready()) return;

  // --- Read encoder delta since last report ---
  static int last_l_pos = 0, last_r_pos = 0;
  int delta_l = enc_l_position - last_l_pos;
  int delta_r = enc_r_position - last_r_pos;
  last_l_pos = enc_l_position;
  last_r_pos = enc_r_position;

  // Clamp delta to int8_t range (-127 to 127) just in case of fast spinning
  if (delta_l >  127) delta_l =  127;
  if (delta_l < -127) delta_l = -127;
  if (delta_r >  127) delta_r =  127;
  if (delta_r < -127) delta_r = -127;

  // --- Build gamepad report ---
  hid_gamepad_report_t report = { 0 };

  // Button 0 = your MX switch
  report.buttons =
  (!gpio_get(BUTTON_BT_A)  ? (1 << 0) : 0) |
  (!gpio_get(BUTTON_BT_B)  ? (1 << 1) : 0) |
  (!gpio_get(BUTTON_BT_C)  ? (1 << 2) : 0) |
  (!gpio_get(BUTTON_BT_D)  ? (1 << 3) : 0) |
  (!gpio_get(BUTTON_FX_L)  ? (1 << 4) : 0) |
  (!gpio_get(BUTTON_FX_R)  ? (1 << 5) : 0) |
  (!gpio_get(BUTTON_START) ? (1 << 6) : 0);

  // encoder delta (-127 to 127)
  report.x = (int8_t) delta_l;  // VOL-L
  report.rx = (int8_t) delta_r; // VOL-R

  tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
}

//--------------------------------------------------------------------+
// HID Callbacks (required by TinyUSB)
//--------------------------------------------------------------------+

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t *buffer, uint16_t reqlen)
{
  (void) instance; (void) report_id; (void) report_type;
  (void) buffer;   (void) reqlen;
  return 0;
}

// Empty because PC doesn't need to send anything back. Just recieve inputs from controller.
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const *buffer, uint16_t bufsize)
{
  (void) instance; (void) report_id; (void) report_type;
  (void) buffer;   (void) bufsize;
}

//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+

int main(void)
{
  board_init();
  gpio_setup();

  // TinyUSB init
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  while (1)
  {
    tud_task();
    led_blinking_task();
    hid_task();
  }
}