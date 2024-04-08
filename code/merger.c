#include <graphics.h>
#include <audio.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <definitions.h>
#include <functions.h>

	

int resolution_x, resolution_y; 							// VGA screen size
int sizeof_pixel;						// number of bytes per pixel
int video_m, video_n;				// number of bits in VGA y coord (m), x coord (n)

int basket_y_pos;
int basket_x_pos;

int prev_basket_x, prev_basket_y; 	// x, y coordinates of boxes to draw
int prev_prev_basket_x, prev_prev_basket_y; 	// x, y coordinates of boxes to draw

int fruit_y_pos_s;
int fruit_x_pos_s;

int prev_fruit_x_s, prev_fruit_y_s; 	// x, y coordinates of boxes to draw
int prev_prev_fruit_x_s, prev_prev_fruit_y_s; 	// x, y coordinates of boxes to draw

int rand_fruit_s;
int prev_rand_fruit_s;
int prev_prev_rand_fruit_s;
int drop_speed;
int missed_fruit;
int gameTime;
int num_fruits_onScreen;

int fruit_y_pos_s[Maxfruit_onScreen];
int fruit_x_pos_s[Maxfruit_onScreen];


int prev_fruit_x_s[Maxfruit_onScreen], prev_fruit_y_s[Maxfruit_onScreen]; 	// x, y coordinates of boxes to draw
int prev_prev_fruit_x_s[Maxfruit_onScreen], prev_prev_fruit_y_s[Maxfruit_onScreen]; 	// x, y coordinates of boxes to draw

int rand_fruit_S[Maxfruit_onScreen];
int prev_rand_fruit_s[Maxfruit_onScreen];
int prev_prev_rand_fruit_s[Maxfruit_onScreen];

int survival;
int main_menu;
int leader_board;
int play_again;

int total_score;
char high_score[] = "0";
char second_score[] = "0";
char third_score[] = "0";
char fourth_score[] = "0";
char fifth_score[] = "0";

char* scores[5] = {high_score, second_score, third_score, fourth_score, fifth_score};

int empty_x_x_pos, empty_x_y_pos;
int redx_x_pos, redx_y_pos;

int* fruit_map[] = {watermelon_map, orange_map, apple_map, grape_map, peach_map, mandarin_map, banana_map, blueberry_map, strawberry_map};
int fruit_width[] = {WATERMELON_WIDTH, ORANGE_WIDTH, APPLE_WIDTH, GRAPE_WIDTH, PEACH_WIDTH, MANDARIN_WIDTH, BANANA_WIDTH, BLUEBERRY_WIDTH, STRAWBERRY_WIDTH};
int fruit_height[] = {WATERMELON_HEIGHT, ORANGE_HEIGHT, APPLE_HEIGHT, GRAPE_HEIGHT, PEACH_HEIGHT, MANDARIN_HEIGHT, BANANA_HEIGHT, BLUEBERRY_HEIGHT, STRAWBERRY_HEIGHT};

int dx_basket, dy_basket; // amount to move basket
int color_box[NUM_BOXES];						// color
unsigned int color[] = {WHITE, YELLOW, RED, GREEN, BLUE, CYAN, MAGENTA, GREY, PINK, ORANGE};
int pixel_buffer_start;
int character_buffer_start;

short int Buffer1[240][512]; // 240 rows, 320 columns + paddings
short int Buffer2[240][512];

struct audio_t {
	volatile unsigned int control;
	volatile unsigned char rarc;
	volatile unsigned char ralc;
	volatile unsigned char wsrc;
	volatile unsigned char wslc;
    volatile unsigned int ldata;
	volatile unsigned int rdata;
};

struct audio_t *const audiop = ((struct audio_t *)0xff203040);



/******************************************************************************
 * This program draws rectangles and boxes on the VGA screen, and moves them
 * in an animated way.
 *****************************************************************************/
int main(void){
	volatile int *TIMER_ptr = (int *) TIMER_BASE;
    volatile int *pixel_ctrl_ptr = (int *) PIXEL_BUF_CTRL_BASE; // pixel controller
	volatile int *LEDR_ptr = (int *)LEDR_BASE;
    volatile int *KEY_ptr = (int *)KEY_BASE;
	volatile int *SW_ptr = (int *)SW_BASE;
	volatile int *HEX3_0_ptr = (int *)HEX3_HEX0_BASE;
	volatile int *HEX5_4_ptr = (int *)HEX5_HEX4_BASE;
  
	sizeof_pixel = 2; 
	video_m = 8; // y has 8 bits
	video_n = 9; // x has 9 bits

    // initialize location and direction of rectangles(not shown)
    /* initialize the location of the front pixel buffer in the pixel buffer controller */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; // first store the address in the back buffer back buffer has the address of BUFFER1
    /* now, swap the front and back buffers, to initialize front pixel buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    /* Erase the pixel buffer */
    get_screen_specs(); // determine X, Y screen size
    clear_screen();
	erase_all_text();

    /* set a location for the back pixel buffer in the pixel buffer controller
        */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen();
	
	main_menu = 1;
	/* set interrupt mask bits for levels 1 (keys)  */
	*(KEY_ptr + 2) = 0x4;
	NIOS2_WRITE_IENABLE(0x3);
	NIOS2_WRITE_STATUS(1); // enable Nios II interrupts
	
	while(main_menu){
		initializer();
		erase_all_text();
		config_timer();
		srand(time(0));
		*(HEX3_0_ptr) = 0x3F;
		*(HEX5_4_ptr) = 0;
		*(LEDR_ptr) = 0;
		//LAUNCH PAGE
		int KEY0 = 1;
		int i = 0;
		play_again = 1;
		int playback_length = starting_audio_samples/4;
		while(KEY0){
			draw_start_page();
			while(1){
				int switch_value = *(SW_ptr) & 0b11;
				int key_0_data = *(KEY_ptr) & 0b1;
				if(switch_value == 1 && key_0_data == 0){ // if sw0 is on
					replace_fruit_fren(47, 170);
				}
				else if(switch_value == 2 && key_0_data == 0){ // if sw1 is on
					replace_survival(194, 170);
				}
				else if (switch_value == 1 && key_0_data == 1){
					KEY0 = 0;
				}
				else if (switch_value == 2 && key_0_data == 1){
					survival = 1;
					KEY0 = 0;
				}
				break;
			}
			
			wait_for_vsync(); // swap front and back buffers on VGA vertical sync
			pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
			audio_playback_mono(starting_audio, playback_length, i);
			i += playback_length;
			if (i > starting_audio_samples){
				i = 0;
			}
		}
		clear_screen();
		wait_for_vsync();
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
		clear_screen();
		*(KEY_ptr + 3) = 0x1;
		if(survival == 1){
			while(1){
                gameTime++;
                printf("GameTime is now: %d \n", gameTime);

                // erases basket that was drawn in the last iteration
		        erase_basket(prev_prev_basket_x, prev_prev_basket_y);
		        prev_prev_basket_x = prev_basket_x;
		        prev_prev_basket_y = prev_basket_y;
		        prev_basket_x = basket_x_pos;
		        prev_basket_y = basket_y_pos;

                for (int i = 0; i < num_fruits_onScreen; i++){
                /* Erase any fruits and baskets that were drawn in the last iteration */
		        erase_fruit(prev_prev_fruit_x_s[i], prev_prev_fruit_y_s[i], fruit_width[prev_prev_rand_fruit_s[i]], fruit_height[prev_prev_rand_fruit_s[i]]);
                prev_prev_fruit_y_s[i] = prev_fruit_y_s[i];
		        prev_prev_fruit_x_s[i] = prev_fruit_x_s[i];
		        prev_fruit_y_s[i] = fruit_y_pos_s[i];
		        prev_fruit_x_s[i] = fruit_x_pos_s[i];
		
		        prev_prev_rand_fruit_s[i] = prev_rand_fruit_s[i];
		        prev_rand_fruit_s[i] = rand_fruit_s[i];
                printf("Pass test 2,erease \n");
                }

                draw_basket(basket_x_pos, basket_y_pos);

                for (int i = 0; i < num_fruits_onScreen; ++i){
		            draw_fruit(fruit_x_pos_s[i], fruit_y_pos_s[i], fruit_width[rand_fruit_s[i]], fruit_height[rand_fruit_s[i]], fruit_map[rand_fruit_s[i]]);
                    //update next fruit location due to timer
		            fruit_y_pos_s[i] = fruit_y_pos_s[i] + drop_speed;
                    //printf("Pass test 3, draw \n");
                }

                if ((*(TIMER_ptr) & 0x1) == 1){
			        *TIMER_ptr = 0; //reset t0
			        drop_speed++;
		        }

                for (int i = 0; i < num_fruits_onScreen; ++i){
                    if (rand_fruit_s[i] == 9){
                        printf("This is BOMB. \n");
                        if(fruit_x_pos_s[i] > basket_x_pos && fruit_x_pos_s[i] < basket_x_pos + BASKET_WIDTH && 
                            fruit_y_pos_s[i] == RESOLUTION_Y - BASKET_HEIGHT){
			                display_HEX(total_score);
                            printf("The total score is: %d\n ", total_score);
				            draw_game_over();
				            return 0;
		                }else if(fruit_y_pos_s[i] > RESOLUTION_Y - BASKET_HEIGHT/2){
                            printf("Nice! U didnt get the bomb\n");
                            erase_fruit(prev_prev_fruit_x_s[i], prev_prev_fruit_y_s[i], fruit_width[prev_prev_rand_fruit_s[i]], fruit_height[prev_prev_rand_fruit_s[i]]);
			                erase_fruit(prev_fruit_x_s[i], prev_fruit_y_s[i], fruit_width[prev_rand_fruit_s[i]], fruit_height[prev_rand_fruit_s[i]]);
			                erase_fruit(fruit_x_pos_s[i], fruit_y_pos_s[i], fruit_width[rand_fruit_s[i]], fruit_height[rand_fruit_s[i]]);
			                
                            //continue;
                        }
              
                    }else{
		                if(fruit_x_pos_s[i] > basket_x_pos && fruit_x_pos_s[i] < basket_x_pos + BASKET_WIDTH && 
                            fruit_y_pos_s[i] == RESOLUTION_Y - BASKET_HEIGHT){
                            total_score += 100;
			                printf("The total score is: %d\n ", total_score);
			                display_HEX(total_score);
			                erase_fruit(prev_prev_fruit_x_s[i], prev_prev_fruit_y_s[i], fruit_width[prev_prev_rand_fruit_s[i]], fruit_height[prev_prev_rand_fruit_s[i]]);
			                erase_fruit(prev_fruit_x_s[i], prev_fruit_y_s[i], fruit_width[prev_rand_fruit_s[i]], fruit_height[prev_rand_fruit_s[i]]);
			                erase_fruit(fruit_x_pos_s[i], fruit_y_pos_s[i], fruit_width[rand_fruit_s[i]], fruit_height[rand_fruit_s[i]]);
			                //continue;
		                }// drops a different fruit at a new location after the fruit hits the bottom of the screen
                        else if(fruit_y_pos_s[i] > RESOLUTION_Y - BASKET_HEIGHT/2){
                            missed_fruit++;
                            //printf("You missed %d fruits\n", missed_fruit);
			                erase_fruit(prev_prev_fruit_x_s[i], prev_prev_fruit_y_s[i], fruit_width[prev_prev_rand_fruit_s[i]], fruit_height[prev_prev_rand_fruit_s[i]]);
			                erase_fruit(prev_fruit_x_s[i], prev_fruit_y_s[i], fruit_width[prev_rand_fruit_s[i]], fruit_height[prev_rand_fruit_s[i]]);
			                erase_fruit(fruit_x_pos_s[i], fruit_y_pos_s[i], fruit_width[rand_fruit_s[i]], fruit_height[rand_fruit_s[i]]);
                        }
                    }//end of else 
            
		        } // end of for loop checking hit or miss
                
                if (gameTime % 35 == 0 && num_fruits_onScreen < Maxfruit_onScreen) {
                    //int new_fruit = num_fruits_onScreen++;
                    rand_fruit_s[num_fruits_onScreen] = rand() % 10;	
                    fruit_y_pos_s[num_fruits_onScreen] = 0;
	                fruit_x_pos_s[num_fruits_onScreen] = rand() % (RESOLUTION_X - 1);
                    printf("generate %d\n", rand_fruit_s[num_fruits_onScreen]); 
                    num_fruits_onScreen ++;   
                    /*
                    for (int i = 0; i < Maxfruit_onScreen; ++i){                
                        rand_fruit_s[i] = rand() % 10;	
	                    fruit_y_pos_s[i] = 0;
	                    fruit_x_pos_s[i] = rand() % (RESOLUTION_X - fruit_width[rand_fruit_s[i]]);
                        fruit_drop_delay[i] = (rand() % 5) + 1;//

	                    printf("generate %d\n", rand_fruit_s);
                        num_fruits_onScreen++;
                        break; // only add one?
                    }
                    */
            
                }
                printf("num of fruits on screen is %d \n", num_fruits_onScreen);

            }// end of big while loop
		}//end of mode 1
		else{
		// GAME MODE 1: Fruit Drop increases with time, miss four fruit, game over
			while (1){	
				// erases basket that was drawn in the last iteration
				erase_basket(prev_prev_basket_x, prev_prev_basket_y);
				prev_prev_basket_x = prev_basket_x;
				prev_prev_basket_y = prev_basket_y;
				prev_basket_x = basket_x_pos;
				prev_basket_y = basket_y_pos;

				/* Erase any fruits and baskets that were drawn in the last iteration */
				erase_fruit(prev_prev_fruit_x_s, prev_prev_fruit_y_s, fruit_width[prev_prev_rand_fruit_s], fruit_height[prev_prev_rand_fruit_s]);
				prev_prev_fruit_y_s = prev_fruit_y_s;
				prev_prev_fruit_x_s = prev_fruit_x_s;
				prev_fruit_y_s = fruit_y_pos_s;
				prev_fruit_x_s = fruit_x_pos_s;

				prev_prev_rand_fruit_s = prev_rand_fruit_s;
				prev_rand_fruit_s = rand_fruit_s;

				// draws new basket and fruit location
				draw_basket(basket_x_pos, basket_y_pos);
				draw_fruit(fruit_x_pos_s, fruit_y_pos_s, fruit_width[rand_fruit_s], fruit_height[rand_fruit_s], fruit_map[rand_fruit_s]);

				// draws x's
				if (missed_fruit == 0){
					draw_empty_x(empty_x_x_pos, empty_x_y_pos);
					draw_empty_x(empty_x_x_pos, empty_x_y_pos + 1 * EMPTY_X_HEIGHT);
					draw_empty_x(empty_x_x_pos, empty_x_y_pos + 2 * EMPTY_X_HEIGHT);
				}
				else if (missed_fruit == 1){
					draw_red_x(redx_x_pos, redx_y_pos);
					*(LEDR_ptr) = 0b1;
				}
				else if(missed_fruit == 2){
					draw_red_x(redx_x_pos, redx_y_pos);
					draw_red_x(redx_x_pos, redx_y_pos + RED_X_HEIGHT);
					*(LEDR_ptr) = 0b11;
				}
				else if(missed_fruit == 3){
					draw_red_x(redx_x_pos, redx_y_pos);
					draw_red_x(redx_x_pos, redx_y_pos + RED_X_HEIGHT);
					draw_red_x(redx_x_pos, redx_y_pos + 2 * RED_X_HEIGHT);
					*(LEDR_ptr) = 0b111;
				}
				else if(missed_fruit == 4){
					//	draw_game_over();
					break;
				}
				//update next fruit location due to timer
				fruit_y_pos_s = fruit_y_pos_s + drop_speed;

				if ((*(TIMER_ptr) & 0x1) == 1){
					*TIMER_ptr = 0; //reset t0
					drop_speed++;
				}
				// if a catch is registered
				if(fruit_x_pos_s > basket_x_pos && fruit_x_pos_s < basket_x_pos + BASKET_WIDTH && fruit_y_pos_s == RESOLUTION_Y - BASKET_HEIGHT){
					audio_playback_mono(fruit_caught, fruit_caught_samples, 0);
					total_score += 100;
					display_HEX(total_score);
					erase_fruit(prev_prev_fruit_x_s, prev_prev_fruit_y_s, fruit_width[prev_prev_rand_fruit_s], fruit_height[prev_prev_rand_fruit_s]);
					erase_fruit(prev_fruit_x_s, prev_fruit_y_s, fruit_width[prev_rand_fruit_s], fruit_height[prev_rand_fruit_s]);
					erase_fruit(fruit_x_pos_s, fruit_y_pos_s, fruit_width[rand_fruit_s], fruit_height[rand_fruit_s]);

					rand_fruit_s = rand() % 8;	
					fruit_y_pos_s = 0;
					fruit_x_pos_s = rand() % (RESOLUTION_X - 1);
					//continue;

				}

				// drops a different fruit at a new location after the fruit hits the bottom of the screen
				else if(fruit_y_pos_s > RESOLUTION_Y - BASKET_HEIGHT/2){
					audio_playback_mono(fruit_miss, fruit_miss_samples, 0);
					missed_fruit++;
					erase_fruit(prev_prev_fruit_x_s, prev_prev_fruit_y_s, fruit_width[prev_prev_rand_fruit_s], fruit_height[prev_prev_rand_fruit_s]);
					erase_fruit(prev_fruit_x_s, prev_fruit_y_s, fruit_width[prev_rand_fruit_s], fruit_height[prev_rand_fruit_s]);
					erase_fruit(fruit_x_pos_s, fruit_y_pos_s, fruit_width[rand_fruit_s], fruit_height[rand_fruit_s]);

					rand_fruit_s = rand() % 8;	
					fruit_y_pos_s = 0;
					fruit_x_pos_s = rand() % (RESOLUTION_X - 1);

				}

				// code that controls the keys
				int edgecapture_bit = *(KEY_ptr + 3) & 0b11;
				if(edgecapture_bit == 1){ // if key 0 is pressed
					basket_x_pos += 40;
					if(basket_x_pos > RESOLUTION_X - BASKET_WIDTH){
						basket_x_pos -= 40;
					}
					*(KEY_ptr + 3) = 0xFF; // reset edge capture bit
				}
				else if(edgecapture_bit == 2){ // if key 1 is pressed
					basket_x_pos-= 40;
					if(basket_x_pos < 0){
						basket_x_pos+= 40;
					}
					*(KEY_ptr + 3) = 0xFF; // reset edge capture bit
				}


				// controls the speed of the fruit dropping manually
				/*int SW_value = *(SW_ptr);
					switch (SW_value) {
						case 0b1:
							drop_speed = 1;
							break;
						case 0b10:
							drop_speed = 2;
							break;
						case 0b100:
							drop_speed = 3;
							break;
						case 0b1000:
							drop_speed = 4;
							break;
						default:
							break;
					}*/
				wait_for_vsync(); // swap front and back buffers on VGA vertical sync
				pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer	
			}
			main_menu = 0;
			leader_board = 0;
			
			while(1){
				draw_game_over();
				while(1){
					int switch_value = *(SW_ptr) & 0b111;
					int key_0_data = *(KEY_ptr) & 0b1;
					if(switch_value == 1 && key_0_data == 0){ // if back to main menu
						replace_main_menu(135, 115);
					}
					else if(switch_value == 2 && key_0_data == 0){ // if back to main menu
						replace_leader_board(139, 144);
					}
					else if (switch_value == 1 && key_0_data == 1){
						main_menu = 1;
					}
					else if (switch_value == 2 && key_0_data == 1){
						leader_board = 1;
					}
					
					break;
				}
				if(main_menu == 1){
					break;
				}
				if(leader_board == 1){
					break;
				}
				wait_for_vsync(); // swap front and back buffers on VGA vertical sync
				pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer	
			}
			while(leader_board){
				clear_screen();
				char text[] = "LEADERBOARD";
				char message[] = "PRESS KEY 1 TO RETURN TO MAIN MENU";
				char first[] = "FIRST PLACE SCORE: ";
				char second[] = "SECOND PLACE SCORE: ";
				char third[] = "THIRD PLACE SCORE: ";
				char fourth[] = "FOURTH PLACE SCORE: ";
				char fifth[] = "FIFTH PLACE SCORE: ";
					
				draw_text(35,10, text);
				draw_text(55,15, high_score);
				draw_text(55,20, second_score);
				draw_text(55,25, third_score);
				draw_text(55,30, fourth_score);
				draw_text(55,35, fifth_score);
				
				draw_text(30,15, first);
				draw_text(30,20, second);
				draw_text(30,25, third);
				draw_text(30,30, fourth);
				draw_text(30,35, fifth);
				draw_text(30, 40, message);
				
				
				int key_1_pressed = *(KEY_ptr) & 0b11;
				if(key_1_pressed == 2){
					leader_board = 0;
				}
				
				wait_for_vsync(); // swap front and back buffers on VGA vertical sync
				pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
			}
			wait_for_vsync(); // swap front and back buffers on VGA vertical sync
			pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
			main_menu = 1;
		}
		clear_screen();
		wait_for_vsync(); // swap front and back buffers on VGA vertical sync
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer	
	}
}

// code for subroutines (not shown)

void draw_text(int x, int y, char * text_ptr) {
	int offset;
	volatile char *character_buffer_start = (char *)0x09000000; // char controller
	/* assume that the text string fits on one line */
	offset = (y << 7) + x;
	while (*(text_ptr)) {
		*(character_buffer_start + offset) = *(text_ptr); // write to the character buffer
		++text_ptr;
		++offset;
	}
}

void erase_all_text(){
	for (int x = 0; x < CHAR_RESOLUTION_X; x++){
		for(int y = 0; y < CHAR_RESOLUTION_Y; y++){
			volatile char *character_buffer_start = (char *)0x09000000; // char controller
			*(character_buffer_start + (y << 7) + x ) = 0x20;
		}
	}
}


void initializer(){
	rand_fruit_s = rand() % 8;
	// initialzies basket start position
	basket_y_pos = RESOLUTION_Y - BASKET_HEIGHT;
	basket_x_pos = (RESOLUTION_X/2) - (BASKET_WIDTH/2);
	fruit_y_pos_s = 0;
	fruit_x_pos_s = rand() % (RESOLUTION_X - 1);
	total_score = 0;
	drop_speed = 1;
	missed_fruit = 0;
	empty_x_x_pos = 300;
	empty_x_y_pos = 5;
	redx_x_pos = 300;
	redx_y_pos = 5;
	
}

void display_HEX(int num){
	volatile int *HEX3_0_ptr = (int *)HEX3_HEX0_BASE;
	volatile int *HEX5_4_ptr = (int *)HEX5_HEX4_BASE;
	int score = num;
	
	int result_3_0 = 0;
	int result_5_4 = 0;
	int remainder = 0;
	int count = 0;
	
	int seven_seg_decode_table[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67}; // 0-9
	
	while (score > 0){
		score = score / 10;
		count++;
	}
	for (int i = 0; i < count; i++){
		remainder = num % 10;
		//printf("%d\n The remainder is: ", remainder);
		num = num / 10;
		
		if(i < 4){
			result_3_0 = ((seven_seg_decode_table[remainder] << (8 * i)) | result_3_0);
		}
		else if(i >= 4){
			result_5_4 = ((seven_seg_decode_table[remainder] << (8 * (i - 4))) | result_5_4);
		}
		else if(i > 5){
			continue; // NEED TO SHOW THAT THEY BEAT THE GAME HERE
		}
	}
	*(HEX3_0_ptr) = result_3_0;
	*(HEX5_4_ptr) = result_5_4;
}

void audio_playback_mono(int* samples, int duration, int start) {
			int length = start;
			audiop->control = 0b1100; // clear the output and input FIFOs
            audiop->control = 0x0; // resume input conversion
            while(start < length + duration) {
              // output data if there is space in the output FIFOs
              if ((audiop->wsrc != 0) && (audiop->wslc != 0)) {
                  audiop->ldata = samples[start];
                  audiop->rdata = samples[start];
				  start++;
              }
            }
}	

void config_timer(){
	volatile int *TIMER_ptr = (int *) TIMER_BASE;
	*(TIMER_ptr) = 0; //Resetting TO
	*(TIMER_ptr + 2) = COUNTER_VALUE & 0xFFFF;
	*(TIMER_ptr + 3) = (COUNTER_VALUE >> 16) & 0xFFFF;
	*(TIMER_ptr + 1) = 0b0110; // turns on start, cont, and interrupt
}

void draw_red_x(int x, int y){
	for (int i = y ; i < y + RED_X_HEIGHT; i++){ // i is basket height
		for (int j = x; j < x + RED_X_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, red_x_map[(i-y)*RED_X_WIDTH+(j-x)]);
		} 
	}
	
}

void draw_empty_x(int x, int y){
	for (int i = y ; i < y + EMPTY_X_HEIGHT; i++){ // i is basket height
		for (int j = x; j < x + EMPTY_X_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, empty_x_map[(i-y)*EMPTY_X_WIDTH+(j-x)]);
		} 
	}
}


void erase_fruit(int x, int y, int fruit_width, int fruit_height){
	for (int i = y ; i < y + fruit_height; i++){ // i is basket height
		for (int j = x; j < x + fruit_width; j++){ // j is basket width
			plot_pixel(j, i, 0);
		} 
	}
}

void draw_basket(int x, int y){
	for (int i = y ; i < RESOLUTION_Y; i++){ // i is basket height
		for (int j = x; j < x + BASKET_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, basket_map[(i-y)*BASKET_WIDTH+(j-x)]);
		} 
	}
}

void replace_survival(int x, int y){
	for (int i = y ; i < y + SURVIVAL_HEIGHT; i++){ // i is basket height
		for (int j = x; j < x + SURVIVAL_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, survival_map[(i-y)*SURVIVAL_WIDTH+(j-x)]);
		} 
	}
}

void replace_fruit_fren(int x, int y){
	for (int i = y ; i < y + FRUIT_FRENZY_HEIGHT; i++){ // i is basket height
		for (int j = x; j < x + FRUIT_FRENZY_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, fruit_frenzy_map[(i-y)*FRUIT_FRENZY_WIDTH+(j-x)]);
		} 
	}
}

void replace_main_menu(int x, int y){
	for (int i = y ; i < y + MAIN_MENU_HEIGHT; i++){ // i is basket height
		for (int j = x; j < x + MAIN_MENU_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, main_menu_map[(i-y)*MAIN_MENU_WIDTH+(j-x)]);
		} 
	}
}

void replace_leader_board(int x, int y){
	for (int i = y ; i < y + LEADER_BOARD_HEIGHT; i++){ // i is basket height
		for (int j = x; j < x + LEADER_BOARD_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, leader_board_map[(i-y)*LEADER_BOARD_WIDTH+(j-x)]);
		} 
	}
}

void draw_start_page(){
	for (int i = 0 ; i < RESOLUTION_Y; i++){ // i is basket height
		for (int j = 0; j < RESOLUTION_X; j++){ // j is basket width
			plot_pixel(j, i, start_page_map[(i)*START_PAGE_WIDTH+(j)]);
		} 
	}
}

void draw_game_over(){
	for (int i = 0 ; i < RESOLUTION_Y; i++){ // i is basket height
		for (int j = 0; j < RESOLUTION_X; j++){ // j is basket width
			plot_pixel(j, i, game_over_map[(i)*GAME_OVER_WIDTH+(j)]);
		} 
	}
}

void draw_fruit(int x, int y, int fruit_width, int fruit_height, int fruit_map[]){
	for (int i = y ; i < y + fruit_height; i++){ // i is basket height
		for (int j = x; j < x + fruit_width; j++){ // j is basket width
			plot_pixel(j, i, fruit_map[(i-y)*fruit_width+(j-x)]);
		} 
	}
}

void erase_basket(int x, int y){
	for (int i = y ; i < RESOLUTION_Y; i++){ // i is basket height
		for (int j = x; j < x + BASKET_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, 0);
		} 
	}
	
}

void clear_screen(){
	for (int x = 0; x < resolution_x; x++){
		for(int y = 0; y < resolution_y; y++){
			plot_pixel(x, y, 0);
		}
	}
}

void get_screen_specs(){
	resolution_x = RESOLUTION_X;
	resolution_y = RESOLUTION_Y;
}


void plot_pixel(int x, int y, short int color)
{
	int shift_x, shift_y;
	shift_x = sizeof_pixel - 1;					// shift x address bits by sizeof(pixel)
	shift_y = video_n + (sizeof_pixel - 1);	// shift y address by |x address| + sizeof(pixel)
	*(short int *)(pixel_buffer_start + (y << shift_y) + (x << shift_x)) = color;
}

void draw_line(int x0, int y0, int x1, int y1, int colour){
	int temp, y_step;
	int is_steep = ABS(y1 - y0) > ABS(x1 - x0);
	if(is_steep){
		temp = x0;
		x0 = y0;
		y0 = temp;
	
		temp = x1;
		x1 = y1;
		y1 = temp;
	}
	
	if(x0 > x1){
		temp = x0;
		x0 = x1;
		x1 = temp;
		
		temp = y0;
		y0 = y1;
		y1 = temp;
	}
	
	int delta_x = x1 - x0;
	int delta_y = ABS(y1 - y0);
	int error = -(delta_x/2);
	int y = y0;
	
	if (y < y1){
		y_step = 1;
	}
	
	else{
		y_step = -1;
	}
	
	for (int x = x0; x < x1; x++){
		if (is_steep){
			plot_pixel(y, x, colour);
		}
		else{
			plot_pixel(x, y, colour);	
		}
		error = error + delta_y;
		if(error > 0){
			y = y + y_step;
			error = error - delta_x;
		}
	}	
}

void erase_line(int x0, int y0, int x1, int y1){
	int temp, y_step;
	int is_steep = ABS(y1 - y0) > ABS(x1 - x0);
	if(is_steep){
		temp = x0;
		x0 = y0;
		y0 = temp;
	
		temp = x1;
		x1 = y1;
		y1 = temp;
	}
	
	if(x0 > x1){
		temp = x0;
		x0 = x1;
		x1 = temp;
		
		temp = y0;
		y0 = y1;
		y1 = temp;
	}
	
	int delta_x = x1 - x0;
	int delta_y = ABS(y1 - y0);
	int error = -(delta_x/2);
	int y = y0;
	
	if (y < y1){
		y_step = 1;
	}
	
	else{
		y_step = -1;
	}
	
	for (int x = x0; x < x1; x++){
		if (is_steep){
			plot_pixel(y, x, 0);
		}
		else{
			plot_pixel(x, y, 0);	
		}
		error = error + delta_y;
		if(error > 0){
			y = y + y_step;
			error = error - delta_x;
		}
	}	
}

void draw_box(int x0, int y0, short int colour){
	plot_pixel(x0, y0, colour);
	plot_pixel(x0 + 1, y0, colour);
	plot_pixel(x0, y0 + 1, colour);
	plot_pixel(x0 + 1, y0 + 1, colour);

}

void erase_box(int x0, int y0){
	plot_pixel(x0, y0, 0);
	plot_pixel(x0 + 1, y0, 0);
	plot_pixel(x0, y0 + 1, 0);
	plot_pixel(x0 + 1, y0 + 1, 0);

}

void wait_for_vsync(){
	volatile int *pixel_ctrl_ptr = (int*)PIXEL_BUF_CTRL_BASE;
	int status;
	*(pixel_ctrl_ptr) = 1; // this will start the sync process, write 1 into the front buf
	status = *(pixel_ctrl_ptr + 3);
	
	while((status & 0x01) != 0){ // polling the status bit, the status bit will turn to 0 when front buffer is done rendering
		status = *(pixel_ctrl_ptr + 3);
	}
}

// interrupt code that i cant figure out
void interrupt_handler(){
	int ipending;
	NIOS2_READ_IPENDING(ipending);
	volatile int *KEY_ptr = (int *) KEY_BASE;
	if ((ipending & 0x2) && ((*(KEY_ptr + 3) & 0b100) == 0b100)){ // interval KEY interrupts
		KEY_ISR();
	}
	return;
}

void KEY_ISR(){ // we can honestly also consider playing music when it speeds up but it need to speed up slower
	volatile int *KEY_ptr = (int *) KEY_BASE;
	int edge_capture_bit = *(KEY_ptr + 3);
	*(KEY_ptr + 2) = 0; // clears the interrupt
	*(KEY_ptr + 3) = 1; // clears the edge capture
	while(edge_capture_bit == 0){
		int edge_capture_bit = *(KEY_ptr + 3);
	}
	return;
}


//void the_exception(void) __attribute__((section(".exceptions")));
void the_exception(void)
{
	asm("subi sp, sp, 128");
	asm("stw et, 96(sp)");
	asm("rdctl et, ctl4");
	asm("beq et, r0, SKIP_EA_DEC"); // Interrupt is not external
	asm("subi ea, ea, 4"); 
	asm("SKIP_EA_DEC:");
	asm("stw r1, 4(sp)"); // Save all registers
	asm("stw r2, 8(sp)");
	asm("stw r3, 12(sp)");
	asm("stw r4, 16(sp)");
	asm("stw r5, 20(sp)");
	asm("stw r6, 24(sp)");
	asm("stw r7, 28(sp)");
	asm("stw r8, 32(sp)");
	asm("stw r9, 36(sp)");
	asm("stw r10, 40(sp)");
	asm("stw r11, 44(sp)");
	asm("stw r12, 48(sp)");
	asm("stw r13, 52(sp)");
	asm("stw r14, 56(sp)");
	asm("stw r15, 60(sp)");
	asm("stw r16, 64(sp)");
	asm("stw r17, 68(sp)");
	asm("stw r18, 72(sp)");
	asm("stw r19, 76(sp)");
	asm("stw r20, 80(sp)");
	asm("stw r21, 84(sp)");
	asm("stw r22, 88(sp)");
	asm("stw r23, 92(sp)");
	asm("stw r25, 100(sp)"); // r25 = bt (skip r24 = et, because it is saved
	// above)
	asm("stw r26, 104(sp)"); // r26 = gp
	// skip r27 because it is sp, and there is no point in saving this
	asm("stw r28, 112(sp)"); // r28 = fp
	asm("stw r29, 116(sp)"); // r29 = ea
	asm("stw r30, 120(sp)"); // r30 = ba
	asm("stw r31, 124(sp)"); // r31 = ra
	asm("addi fp, sp, 128");
	asm("call interrupt_handler"); // Call the C language interrupt handler
	asm("ldw r1, 4(sp)"); // Restore all registers
	asm("ldw r2, 8(sp)");
	asm("ldw r3, 12(sp)");
	asm("ldw r4, 16(sp)");
	asm("ldw r5, 20(sp)");
	asm("ldw r6, 24(sp)");
	asm("ldw r7, 28(sp)");
	asm("ldw r8, 32(sp)");
	asm("ldw r9, 36(sp)");
	asm("ldw r10, 40(sp)");
	asm("ldw r11, 44(sp)");
	asm("ldw r12, 48(sp)");
	asm("ldw r13, 52(sp)");
	asm("ldw r14, 56(sp)");
	asm("ldw r15, 60(sp)");
	asm("ldw r16, 64(sp)");
	asm("ldw r17, 68(sp)");
	asm("ldw r18, 72(sp)");

	asm("ldw r19, 76(sp)");
	asm("ldw r20, 80(sp)");
	asm("ldw r21, 84(sp)");
	asm("ldw r22, 88(sp)");
	asm("ldw r23, 92(sp)");
	asm("ldw r24, 96(sp)");
	asm("ldw r25, 100(sp)"); // r25 = bt
	asm("ldw r26, 104(sp)"); // r26 = gp
	// skip r27 because it is sp, and we did not save this on the stack
	asm("ldw r28, 112(sp)"); // r28 = fp
	asm("ldw r29, 116(sp)"); // r29 = ea
	asm("ldw r30, 120(sp)"); // r30 = ba
	asm("ldw r31, 124(sp)"); // r31 = ra
	asm("addi sp, sp, 128");
	asm("eret");
	}

//void the_reset(void) __attribute__((section(".reset")));
void the_reset(void)
{
	asm(".set noat"); 
	asm(".set nobreak"); 
	asm("movia r2, main"); // Call the C language main program
	asm("jmp r2");
}