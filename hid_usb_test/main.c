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

#define BUTTON_PIN  0   // MX switch
#define ENC_A       10
#define ENC_B       11
#define ENC_SW      12  // encoder push button (optional for now)

//--------------------------------------------------------------------+
// Encoder State
//--------------------------------------------------------------------+

volatile int encoder_position = 0;

void encoder_isr(uint gpio, uint32_t events)
{
  bool a = gpio_get(ENC_A);
  bool b = gpio_get(ENC_B);
  encoder_position += (a != b) ? 1 : -1;
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
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
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

  // --- Read button (active LOW because of pull-up) ---
  bool button_pressed = !gpio_get(BUTTON_PIN);

  // --- Read encoder delta since last report ---
  static int last_position = 0;
  int delta = encoder_position - last_position;
  last_position = encoder_position;

  // Clamp delta to int8_t range (-127 to 127) just in case of fast spinning
  if (delta >  127) delta =  127;
  if (delta < -127) delta = -127;

  // --- Build gamepad report ---
  hid_gamepad_report_t report = { 0 };

  // Button 0 = your MX switch
  report.buttons = button_pressed ? 1 : 0;

  // X axis = encoder delta (-127 to 127)
  // For SDVX, VOL-L will eventually go here, VOL-R on a second axis (rx)
  report.x = (int8_t) delta;

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

  // GPIO setup
  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);

  gpio_init(ENC_A);
  gpio_init(ENC_B);
  gpio_set_dir(ENC_A, GPIO_IN);
  gpio_set_dir(ENC_B, GPIO_IN);
  gpio_pull_up(ENC_A);
  gpio_pull_up(ENC_B);

  gpio_init(ENC_SW);
  gpio_set_dir(ENC_SW, GPIO_IN);
  gpio_pull_up(ENC_SW);

  gpio_set_irq_enabled_with_callback(ENC_A,
    GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_isr);

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