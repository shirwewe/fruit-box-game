#include <graphics.h>
/* This files provides address values that exist in the system */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>



/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
	
// Control Register for Interrupt

/*#ifndef __NIOS2_CTRL_REG_MACROS__
#define __NIOS2_CTRL_REG_MACROS__
#define NIOS2_READ_STATUS(dest) \
do { dest = __builtin_rdctl(0); } while (0)
#define NIOS2_WRITE_STATUS(src) \
do { __builtin_wrctl(0, src); } while (0)
#define NIOS2_READ_ESTATUS(dest) \
do { dest = __builtin_rdctl(1); } while (0)
#define NIOS2_READ_BSTATUS(dest) \
do { dest = __builtin_rdctl(2); } while (0)
#define NIOS2_READ_IENABLE(dest) \
do { dest = __builtin_rdctl(3); } while (0)
#define NIOS2_WRITE_IENABLE(src) \
do { __builtin_wrctl(3, src); } while (0)
#define NIOS2_READ_IPENDING(dest) \
do { dest = __builtin_rdctl(4); } while (0)
#define NIOS2_READ_CPUID(dest) \
do { dest = __builtin_rdctl(5); } while (0)
#endif*/

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0
#define TRUE 1
	
#define COUNTER_VALUE 50000000000
	

int resolution_x, resolution_y; 							// VGA screen size
int sizeof_pixel;						// number of bytes per pixel
int video_m, video_n;				// number of bits in VGA y coord (m), x coord (n)

int basket_y_pos;
int basket_x_pos;

int prev_basket_x, prev_basket_y; 	// x, y coordinates of boxes to draw
int prev_prev_basket_x, prev_prev_basket_y; 	// x, y coordinates of boxes to draw

int fruit_y_pos;
int fruit_x_pos;

int prev_fruit_x, prev_fruit_y; 	// x, y coordinates of boxes to draw
int prev_prev_fruit_x, prev_prev_fruit_y; 	// x, y coordinates of boxes to draw

int rand_fruit;
int prev_rand_fruit;
int prev_prev_rand_fruit;
int drop_speed;
int missed_fruit;

int total_score;

int empty_x_x_pos, empty_x_y_pos;
int redx_x_pos, redx_y_pos;

int* fruit_map[] = {watermelon_map, orange_map, apple_map, grape_map, peach_map, mandarin_map, banana_map, blueberry_map, strawberry_map};
int fruit_width[] = {WATERMELON_WIDTH, ORANGE_WIDTH, APPLE_WIDTH, GRAPE_WIDTH, PEACH_WIDTH, MANDARIN_WIDTH, BANANA_WIDTH, BLUEBERRY_WIDTH, STRAWBERRY_WIDTH};
int fruit_height[] = {WATERMELON_HEIGHT, ORANGE_HEIGHT, APPLE_HEIGHT, GRAPE_HEIGHT, PEACH_HEIGHT, MANDARIN_HEIGHT, BANANA_HEIGHT, BLUEBERRY_HEIGHT, STRAWBERRY_HEIGHT};

int dx_basket, dy_basket; // amount to move basket
int color_box[NUM_BOXES];						// color
unsigned int color[] = {WHITE, YELLOW, RED, GREEN, BLUE, CYAN, MAGENTA, GREY, PINK, ORANGE};
int pixel_buffer_start;


short int Buffer1[240][512]; // 240 rows, 320 columns + paddings
short int Buffer2[240][512];

void get_screen_specs(void);
void clear_screen(void);
void draw_box(int, int, short int);
void draw_empty_x(int,int);
void plot_pixel(int, int, short int);
void draw_line(int, int, int, int, int);
void wait_for_vsync(void);
void erase_line(int, int, int, int);
void initializer();
void erase_box(int, int);
void erase_basket(int,int);
void draw_basket(int, int);
void draw_fruit(int, int, int, int, int*);
void erase_fruit(int, int, int, int);
void config_timer(void);
void draw_game_over(void);
void draw_red_x(int, int);
void display_HEX(int);
//void interrupt_handler(void);
//void TIMER_ISR(void);

/******************************************************************************
 * This program draws rectangles and boxes on the VGA screen, and moves them
 * in an animated way.
 *****************************************************************************/
int main(void){
	volatile int *TIMER_ptr = (int *) TIMER_BASE;
    volatile int * pixel_ctrl_ptr = (int *) PIXEL_BUF_CTRL_BASE; // pixel controller
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

    /* set a location for the back pixel buffer in the pixel buffer controller
        */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen();
	initializer();
	config_timer();
	srand(time(0));
	*(HEX3_0_ptr) = 0x3F;
	*(HEX5_4_ptr) = 0;
	
	/* set interrupt mask bits for levels 0 (interval timer)  */
	//NIOS2_WRITE_IENABLE(0x0);
	//NIOS2_WRITE_STATUS(0); // enable Nios II interrupts

    while (1)
    {	
		// erases basket that was drawn in the last iteration
		erase_basket(prev_prev_basket_x, prev_prev_basket_y);
		prev_prev_basket_x = prev_basket_x;
		prev_prev_basket_y = prev_basket_y;
		prev_basket_x = basket_x_pos;
		prev_basket_y = basket_y_pos;

		/* Erase any fruits and baskets that were drawn in the last iteration */
		erase_fruit(prev_prev_fruit_x, prev_prev_fruit_y, fruit_width[prev_prev_rand_fruit], fruit_height[prev_prev_rand_fruit]);
		prev_prev_fruit_y = prev_fruit_y;
		prev_prev_fruit_x = prev_fruit_x;
		prev_fruit_y = fruit_y_pos;
		prev_fruit_x = fruit_x_pos;
		
		prev_prev_rand_fruit = prev_rand_fruit;
		prev_rand_fruit = rand_fruit;

		// draws new basket and fruit location
		draw_basket(basket_x_pos, basket_y_pos);
		draw_fruit(fruit_x_pos, fruit_y_pos, fruit_width[rand_fruit], fruit_height[rand_fruit], fruit_map[rand_fruit]);
		
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
		fruit_y_pos = fruit_y_pos + drop_speed;
		
		if ((*(TIMER_ptr) & 0x1) == 1){
			*TIMER_ptr = 0; //reset t0
			drop_speed++;
		}
		// if a catch is registered
		
		if(fruit_x_pos > basket_x_pos && fruit_x_pos < basket_x_pos + BASKET_WIDTH && fruit_y_pos == RESOLUTION_Y - BASKET_HEIGHT){
			total_score += 100;
			printf("The total score is: %d\n ", total_score);
			display_HEX(total_score);
			erase_fruit(prev_prev_fruit_x, prev_prev_fruit_y, fruit_width[prev_prev_rand_fruit], fruit_height[prev_prev_rand_fruit]);
			erase_fruit(prev_fruit_x, prev_fruit_y, fruit_width[prev_rand_fruit], fruit_height[prev_rand_fruit]);
			erase_fruit(fruit_x_pos, fruit_y_pos, fruit_width[rand_fruit], fruit_height[rand_fruit]);
			
			rand_fruit = rand() % 8;	
			fruit_y_pos = 0;
			fruit_x_pos = rand() % (RESOLUTION_X - 1);
			//continue;

		}
		
		// drops a different fruit at a new location after the fruit hits the bottom of the screen
		else if(fruit_y_pos > RESOLUTION_Y - BASKET_HEIGHT/2){
			missed_fruit++;
			erase_fruit(prev_prev_fruit_x, prev_prev_fruit_y, fruit_width[prev_prev_rand_fruit], fruit_height[prev_prev_rand_fruit]);
			erase_fruit(prev_fruit_x, prev_fruit_y, fruit_width[prev_rand_fruit], fruit_height[prev_rand_fruit]);
			erase_fruit(fruit_x_pos, fruit_y_pos, fruit_width[rand_fruit], fruit_height[rand_fruit]);
			
			rand_fruit = rand() % 8;	
			fruit_y_pos = 0;
			fruit_x_pos = rand() % (RESOLUTION_X - 1);
			
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
		

		// controls the speed of the fruit dropping
		/*int SW_value = *(SW_ptr);
		switch (SW_value) {
			case 0b1:
				if(fruit_y_pos > 240){
					break;
				}
				else{
					fruit_y_pos += 2;
				}
				break;
			case 0b10:
				if(fruit_y_pos > 240){
					break;
				}
				else{
					fruit_y_pos += 4;
				}
				break;
			case 0b100:
				if(fruit_y_pos > 240){
					break;
				}
				else{
					fruit_y_pos += 8;
				}
				break;
			case 0b1000:
				if(fruit_y_pos > 240){
					break;
				}
				else{
					fruit_y_pos += 16;
				}
				break;
			default:
				if(fruit_y_pos > 240){
					break;
				}
				else{
					fruit_y_pos++;
				}
				break;
		}*/
		
		// updates score
		
		//printf("%d", total_score);
		
		

		wait_for_vsync(); // swap front and back buffers on VGA vertical sync
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer	
	}
	
	clear_screen();
	wait_for_vsync(); // swap front and back buffers on VGA vertical sync
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer	
}

// code for subroutines (not shown)


void initializer(){
	rand_fruit = rand() % 8;
	// initialzies basket start position
	basket_y_pos = RESOLUTION_Y - BASKET_HEIGHT;
	basket_x_pos = (RESOLUTION_X/2) - (BASKET_WIDTH/2);
	fruit_y_pos = 0;
	fruit_x_pos = rand() % (RESOLUTION_X - 1);
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
	printf("The result 3 0 is: %d\n", result_3_0);
	printf("The result 4 5 is: %d\n", result_5_4);
	*(HEX3_0_ptr) = result_3_0;
	*(HEX5_4_ptr) = result_5_4;
}
	
		
	


void config_timer(){
	volatile int *TIMER_ptr = (int *) TIMER_BASE;
	*(TIMER_ptr) = 0; //Resetting TO
	*(TIMER_ptr + 2) = COUNTER_VALUE & 0xFFFF;
	*(TIMER_ptr + 3) = (COUNTER_VALUE >> 16) & 0xFFFF;
	*(TIMER_ptr + 1) = 0b0110; // turns on start, cont, and interrupt
}

//void draw_game_over(){

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

/*
void interrupt_handler(){
	int ipending;
	NIOS2_READ_IPENDING(ipending);
	if (ipending & 0x1){ // interval timer interrupts
		TIMER_ISR();
	}
	return;
}

void TIMER_ISR(){ // we can honestly also consider playing music when it speeds up but it need to speed up slower
	volatile int *TIMER_ptr = (int *) TIMER_BASE;
	*(TIMER_ptr) = 0; // clears the interrupt
	//drop_speed++;
	printf("%d", 999);
	return;
}


void the_exception(void) __attribute__((section(".exceptions")));
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

void the_reset(void) __attribute__((section(".reset")));
void the_reset(void)
{
	asm(".set noat"); 
	asm(".set nobreak"); 
	asm("movia r2, main"); // Call the C language main program
	asm("jmp r2");
}

*/