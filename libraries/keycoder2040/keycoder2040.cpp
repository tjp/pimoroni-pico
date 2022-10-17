#include <math.h>
#include <cstdlib>
#include <string.h>
#include <time.h>
#include <cstdio>

#include "pico/stdlib.h"
#include "libraries/keycoder2040/keycoder2040.hpp"
#include "common/pimoroni_common.hpp"
#include "drivers/is31fl3731/is31fl3731.hpp"

namespace pimoroni{


    namespace keycoder2040{

        

            keycoder2040::Interface(PIO pio, uint sm, Direction direction, float counts_per_rev, bool count_microsteps, uint16_t freq_divider)
                : pio(pio)
                , sm(sm)
                , enc_direction(direction)
                , enc_counts_per_rev(MAX(counts_per_rev, FLT_EPSILON))
                , count_microsteps(count_microsteps)
                , freq_divider(freq_divider) {}

            bool Interface::init(){

                pio_sm_claim(pio, sm);
                uint pio_idx = pio_get_index(pio);

                // If this is the first time using an encoder on this PIO, add the program to the PIO memory
                if(claimed_sms[pio_idx] == 0) {
                    pio_program_offset[pio_idx] = pio_add_program(pio, &keycoder_program);
                }

                // Initilise all required 
                gpio_init_mask(ENCODERS_MASK);
                gpio_set_dir_masked(ENCODER_MASK , false); //set the encoder rows IO to input 
                for (int index=0 ; index < ENCODERS_WIDTH ; index++ ){
                    gpio_pull_down(ENCODERS_START + index);
                    gpio_set_fuction(ENCODERS_START + index, GPIO_FUNC_PIO0);
                }

                gpio_init_mask(COLUMN_MASK);
                gpio_set_dir_masked(COLUMN_MASK , true); //set the column drivers to output
                for (int index=0 ; index < COLUMN_WIDTH ; index++ ){
                    gpio_pull_down(COLUMN_START + index);
                    gpio_set_fuction(COLUMN_START + index, GPIO_FUNC_PIO0);
                }

                // Create PIO configuration handler
                pio_sm_config c = encoder_program_get_default_config(pio_program_offset[pio_idx]);
                
                // Set Input pins base 
                sm_config_set_in_pins(&c, ENCODERS_START);

                // Setup FIFO Config autopush off 
                sm_config_set_in_shift(&c, false, false, 1);
                sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);

                // Setup Column pins in sideset configuration 
                sm_config_set_sideset_pins(&c, COLUMN_START);
                sm_config_set_sideset(&c , COLUMN_WIDTH, false, false);

                // Set the PIO clock speed as a division of clk_sys
                sm_config_set_clkdiv_int_frac(&c, freq_divider, 0);

                // Write settings into PIO             
                pio_sm_init(pio, sm, pio_program_offset[pio_idx], &c);
                
                //put any inital PIO sm instructions here 
                //pio_sm_exec(pio, sm, pio_encode_set(pio_x, (uint)enc_state_a << 1 | (uint)enc_state_b));

                // Set sm running
                pio_sm_set_enabled(pio, sm, true);

                return true;

            }

            void Interface::pio_interrupt_handler(uint pio_idx) {
                // Go through each SM on the PIO to see which triggered this interrupt,
                // and if there's an associated encoder, have it update its state
                process_steps();
                }
                

            void Interface::pio0_interrupt_handler() {
            pio_interrupt_handler(0);
            }

            void Interface::pio1_interrupt_handler() {
            pio_interrupt_handler(1);
            }

            void Interface::process_steps(){

                new_state = pio_sm_get_blocking(pio, sm);

                // TODO check which encoder moved and update steps

                last_state = new_state;


            }

            uint32_t Interface::get_last_state(){
                return last_state;
            }

        
        }

                    



    }
}