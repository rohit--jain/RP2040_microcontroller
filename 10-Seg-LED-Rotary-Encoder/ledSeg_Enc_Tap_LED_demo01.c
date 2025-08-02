#include "hardware/adc.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>

// macros for controlling the internal LED
#define LED_ON cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1)
#define LED_OFF cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0)

// macros for used GPIO pin IDs
#define ENC_A 16
#define ENC_B 17
#define LED_STRIP (uint[]){6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
#define LED_RGB (uint[]){3, 2, 4}
#define MIC_ADC 0
#define MIC_GPIO 26

static int mic_volume = 0;

enum RGB {
   RED = 0,
   GREEN = 1,
   BLUE = 2,
   OFF = 3
};

// called if an interrupt occurs
void encoder_callback(uint gpio, uint32_t events) {
   bool enc_a = gpio_get(ENC_A);
   bool enc_b = gpio_get(ENC_B);

   static bool ccw_fall = 0; // bool used when falling edge is triggered
   static bool cw_fall = 0;

   // select the desired behavior with if-statement over calling GPIO ID
   // processes raw encoder data (-> gray code) and increments / decrements mic_volume
   if (gpio == ENC_A) {
      if ((!cw_fall) && enc_b && !enc_a) // cw_fall is set to TRUE when phase A interrupt is triggered
         cw_fall = 1;

      if ((ccw_fall) && !enc_b && !enc_a) // if ccw_fall is already set to true from a previous B phase trigger, the ccw event will be triggered
      {
         cw_fall = 0;
         ccw_fall = 0;

         // action for counter clockwise movement goes here
         mic_volume = mic_volume > 0 ? mic_volume - 1 : mic_volume; // decrement mic_volume, 0-10
         gpio_put(LED_STRIP[mic_volume], 0);                        // turn on corresponding LEDs on strip
         printf("Mic volume: %d \n", mic_volume);
      }
   }

   if (gpio == ENC_B) {
      if ((!ccw_fall) && !enc_b && enc_a) // ccw leading edge is true
         ccw_fall = 1;

      if ((cw_fall) && !enc_b && !enc_a) // cw trigger
      {
         cw_fall = 0;
         ccw_fall = 0;

         // action for clockwise movement goes here
         mic_volume = mic_volume < 10 ? mic_volume + 1 : mic_volume; // increment mic volume , 0-10
         gpio_put(LED_STRIP[mic_volume - 1], 1);                     // turn on corresponding LEDs on strip
         printf("Mic volume: %d \n", mic_volume);
      }
   }
}

void setup() {
   stdio_init_all();

   // Init Encoder GPIOs and active internal pull-up resistors
   gpio_init(ENC_A);
   gpio_set_dir(ENC_A, GPIO_IN);
   gpio_set_pulls(ENC_A, 1, 0);

   gpio_init(ENC_B);
   gpio_set_dir(ENC_B, GPIO_IN);
   gpio_set_pulls(ENC_B, 1, 0);

   // attach the interrupt callback to any encoder GPIO, all GPIOs with IRQ enabled call the same callback
   gpio_set_irq_enabled_with_callback(ENC_A, GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
   gpio_set_irq_enabled(ENC_B, GPIO_IRQ_EDGE_FALL, true);

   // Init all LED Strip GPIOs and set them as output
   for (int i = 0; i < 10; i++) {
      gpio_init(LED_STRIP[i]);
      gpio_set_dir(LED_STRIP[i], GPIO_OUT);
   }

   // Init all RGB LED GPIOs and set them as output
   for (int i = 0; i < 3; i++) {
      gpio_init(LED_RGB[i]);
      gpio_set_dir(LED_RGB[i], GPIO_OUT);
   }

   // setup analog to digital converter (adc) for mic input
   adc_init();
   adc_gpio_init(MIC_GPIO);
   adc_select_input(MIC_ADC);
}

// set the RGB LED to the specified color
void set_rgb_led(enum RGB color) {
   for (int i = 0; i < OFF; i++) {
      if (i == color) {
         gpio_put(LED_RGB[i], 1);
      } else {
         gpio_put(LED_RGB[i], 0);
      }
   }
}

// control the RGB LED color with the microphone signal (multiplied by mic_volume)
// no signal -> off, low signal -> blue, medium signal-> green, high signal -> red
void control_rgb_led_with_mic() {
   const float conversion_factor = 3.3f / (1 << 12); // convert the raw adc out to voltages using this factor
   float mic_with_gain = adc_read() * conversion_factor * mic_volume;

   if (mic_with_gain < 0) {
      set_rgb_led(OFF);
   } else if (mic_with_gain < 0.1) {
      set_rgb_led(OFF);
   } else if (mic_with_gain < 0.2) {
      set_rgb_led(BLUE);
   } else if (mic_with_gain < 0.7) {
      set_rgb_led(GREEN);
   } else {
      set_rgb_led(RED);
   }
}

/****************************************************************************************************************************/

int main() {
   sleep_ms(500);
   setup();

   while (true) {
      control_rgb_led_with_mic();
      sleep_ms(20);
   }
}
