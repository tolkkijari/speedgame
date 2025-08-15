#include <avr/io.h>
//TODO: kehityksenaikainen
#include <util/delay.h>
//TODO: tsekkaa, voisko tämän hoitaa fiksummin ilman että syö liikaa muistia
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include "linked_list.h"

//Tens of milliseconds
#define INITIAL_DELAY 100
//Tens of milliseconds
#define SPEED_UP_INTERVAL 100
#define TIME_REDUCTION_FACTOR 0.985
#define MAX_AMOUNT_LEDS_IN_MEMORY 10

volatile uint8_t last_clicked_btn = 5;
volatile uint8_t is_clicked = 0;
//Max ~650 seconds until the counter flips over.
volatile unsigned int ten_ms_counter = 0;
volatile uint8_t ms_counter = 0;

ISR(TIMER0_COMPA_vect) {
  ms_counter++;

	if(ms_counter != 0 && ms_counter % 10 == 0) {
		ten_ms_counter++;
		ms_counter = 0;
	}
}

ISR(PCINT2_vect) {
  if(!is_clicked) {
    if(bit_is_clear(PIND, PD0) && last_clicked_btn != 0) {
		is_clicked = 1;
      last_clicked_btn = 0;
    }
    if(bit_is_clear(PIND, PD1) && last_clicked_btn != 1) {
		is_clicked = 1;
      last_clicked_btn = 1;
    }
    if(bit_is_clear(PIND, PD2) && last_clicked_btn != 2) {
		is_clicked = 1;
      last_clicked_btn = 2;
    }
    if(bit_is_clear(PIND, PD3) && last_clicked_btn != 3) {
		is_clicked = 1;
      last_clicked_btn = 3;
    }
  }
}

void init_button_interrupt(void) {
  DDRD = 0xff;
  PORTD = 0xff;
  PCICR |= (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3);
  PCMSK2 |= (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3);
}

static inline void init_timer(void) {
	TCCR0A = (1 << WGM01); // CTC mode - tick the TIMSK0 register when OCR0A is full
  TCCR0B = (1 << CS01); // Prescaler = 8, 8us, 1MHz clock
  OCR0A = 124; // OCR0A is full  after 125 ticks - which  after every one milliseconds
  TIMSK0 |= (1 << OCIE0A); // Enable Timer0 Compare Match A interrupt
}

//TODO: onko tarpeen lisätä interruptien keskeytys käsittelyn ajaksi?
int main(void) {

  DDRC |= 0x0f;
	init_timer();
  init_button_interrupt();
  sei();

  while(1) {
		unsigned int led_start_time = 0;
		unsigned int last_speed_up_time = 0;
		float current_delay = INITIAL_DELAY;
		int is_led_on = 0;
		unsigned int score = 0;
		uint8_t last_selected_led = 5;
  	uint8_t game_on = 0;

		last_clicked_btn = 5;
		ten_ms_counter = 0;
		is_clicked = 0;

		// "The splash screen"
		PORTC = 0x0f;

		//Seed the random generator, the game starts after clicking a button.
		while(!game_on) {
			//TODO: joskus nappi pysyy liian pitkää pohjassa ja aiheuttaa pulmia
			if(last_clicked_btn != 5) {
				srand(ten_ms_counter + ms_counter);
				last_clicked_btn = 5;
				is_clicked = 0;
				game_on = 1;
			}
		}
		
		PORTC = 0x00;
		_delay_ms(1000);
		ten_ms_counter = 0;

		while (game_on) {
			if(!is_led_on) {
				if(get_size() >= MAX_AMOUNT_LEDS_IN_MEMORY) {
					game_on = 0;
					break;
				}

				uint8_t selected_led = rand() % 4;

				//Let's not read the linked list because it might be empty and we won't want the same led to be blinked twice.
				while(selected_led == last_selected_led) {
					selected_led = rand() % 4;
				}

				add_new(selected_led);
				last_selected_led = selected_led;
				
				led_start_time = ten_ms_counter;
				is_led_on = 1;
				//selected_led is between 0-3, and the led pins are between 1, 2, 4, and 8 in binary.
				//A small binary transfer fixes the difference.
				PORTC = 1 << selected_led;
			}

			//Speed up the led switching after every elapsed second.
			if(ten_ms_counter - last_speed_up_time >= SPEED_UP_INTERVAL) {
				current_delay *= TIME_REDUCTION_FACTOR;
				last_speed_up_time = ten_ms_counter;
			}

			if((ten_ms_counter - led_start_time) >= round(current_delay)) {
				is_led_on = 0;
			}

			//TODO: huomioi ns. thread-safety interrupteihin nähden, lisää jokin lukkomuuttuja
			if(is_clicked) {
				is_clicked = 0;

				if(get_start() == last_clicked_btn) {
					delete_start();
					score++;
				}
				else {
					game_on = 0;
					//A temporary way to show the scores as binary.
					// PORTC = 0x00;
					// _delay_ms(2000);
					// PORTC = 0xff;
					// _delay_ms(500);
					// PORTC = score >> 4;
					// _delay_ms(4000);
					// PORTC = 0x00;
					// _delay_ms(500);
					// PORTC = score;
					// _delay_ms(4000);
					// PORTC = 0;
				}
			}
		}

		//Release the allocated memory
		delete_all();
		last_clicked_btn = 5;
		is_clicked = 0;

		PORTC = 0;
	}

  return 0;
}
