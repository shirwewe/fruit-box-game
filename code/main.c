/* This files provides address values that exist in the system */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <graphics.h>


/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030

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

int ran_num;

int* fruit_map[] = {watermelon_map, orange_map, apple_map, grape_map, peach_map, mandarin_map, banana_map, blueberry_map, strawberry_map};
int* fruit_width[] = {WATERMELON_WIDTH, ORANGE_WIDTH, APPLE_WIDTH, GRAPE_WIDTH, PEACH_WIDTH, MANDARIN_WIDTH, BANANA_WIDTH, BLUEBERRY_WIDTH, STRAWBERRY_WIDTH};
int* fruit_height[] = {WATERMELON_HEIGHT, ORANGE_HEIGHT, APPLE_HEIGHT, GRAPE_HEIGHT, PEACH_HEIGHT, MANDARIN_HEIGHT, BANANA_HEIGHT, BLUEBERRY_HEIGHT, STRAWBERRY_HEIGHT};

int dx_basket, dy_basket; // amount to move boxes in animation
int color_box[NUM_BOXES];						// color
unsigned int color[] = {WHITE, YELLOW, RED, GREEN, BLUE, CYAN, MAGENTA, GREY, PINK, ORANGE};
int pixel_buffer_start;


short int Buffer1[240][512]; // 240 rows, 320 columns + paddings
short int Buffer2[240][512];

void get_screen_specs(void);
void clear_screen(void);
void draw_box(int, int, short int);
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

/******************************************************************************
 * This program draws rectangles and boxes on the VGA screen, and moves them
 * in an animated way.
 *****************************************************************************/
int main(void){

    volatile int * pixel_ctrl_ptr = (int *) PIXEL_BUF_CTRL_BASE; // pixel controller
	volatile int *LEDR_ptr = LEDR_BASE;
    volatile int *KEY_ptr = KEY_BASE;
	volatile int *SW_ptr = SW_BASE;
    // declare other variables(not shown)
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

    while (1)
    {
		erase_basket(prev_prev_basket_x, prev_prev_basket_y);
		prev_prev_basket_x = prev_basket_x;
		prev_prev_basket_y = prev_basket_y;
		prev_basket_x = basket_x_pos;
		prev_basket_y = basket_y_pos; 
		
		erase_fruit(prev_prev_fruit_x, prev_prev_fruit_y, fruit_width[ran_num], fruit_height[ran_num]);
		prev_prev_fruit_y = prev_fruit_y;
		prev_prev_fruit_x = prev_fruit_x;

		prev_fruit_y = fruit_y_pos;
		prev_fruit_x = fruit_x_pos;

		
		while(fruit_y_pos < 240){
	
			erase_basket(prev_prev_basket_x, prev_prev_basket_y);
			draw_basket(basket_x_pos, basket_y_pos);
			prev_prev_basket_x = prev_basket_x;
			prev_prev_basket_y = prev_basket_y;
			prev_basket_x = basket_x_pos;
			prev_basket_y = basket_y_pos; 
		
			int edgecapture_bit = *(KEY_ptr + 3) & 0b11;
			if(edgecapture_bit == 1){ // if key 0 is pressed
				basket_x_pos += 20;
				if(basket_x_pos > RESOLUTION_X - BASKET_WIDTH){
					basket_x_pos -= 20;
				}
				*(KEY_ptr + 3) = 0xFF; // reset edge capture bit
			}
			else if(edgecapture_bit == 2){ // if key 1 is pressed
				basket_x_pos-= 20;
				if(basket_x_pos < 0){
					basket_x_pos+= 20;
				}
				*(KEY_ptr + 3) = 0xFF; // reset edge capture bit
			}
			/* Erase any fruits and baskets that were drawn in the last iteration */
			erase_fruit(prev_prev_fruit_x, prev_prev_fruit_y, fruit_width[ran_num], fruit_height[ran_num]);
			draw_fruit(fruit_x_pos, fruit_y_pos, fruit_width[ran_num], fruit_height[ran_num], fruit_map[ran_num]);

		
			prev_prev_fruit_y = prev_fruit_y;
			prev_prev_fruit_x = prev_fruit_x;

			prev_fruit_y = fruit_y_pos;
			prev_fruit_x = fruit_x_pos;

			//if(fruit_x_pos - WATERMELON_WIDTH == basket_x_pos && fruit_y_pos - ){
			//	continue; this is code for setting fruit in basket
			
			int SW_value = *(SW_ptr);
			switch (SW_value) {
				case 0b1:
					fruit_y_pos += 2;
				break;
				case 0b10:
					fruit_y_pos += 4;
					
				break;
				case 0b100:
					fruit_y_pos += 8;
					
				break;
				case 0b1000:
					fruit_y_pos += 16;
				break;
				default:
					fruit_y_pos++;
				break;
			}
			wait_for_vsync(); // swap front and back buffers on VGA vertical sync
			pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
		}
		
		ran_num++;
		if(ran_num > 8){
			ran_num = 0;	
		}
		fruit_y_pos = 0;
		fruit_x_pos = rand() % 8;
		
	}
	
}

// code for subroutines (not shown)

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

void initializer(){
	
	int ran_num = rand() % 8;
	// initialzies basket start position
	basket_y_pos = RESOLUTION_Y - BASKET_HEIGHT;
	basket_x_pos = (RESOLUTION_X/2) - (BASKET_WIDTH/2);
	fruit_y_pos = 0;
	fruit_x_pos = rand() % (RESOLUTION_X - 1);
	
	
	
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
	*pixel_ctrl_ptr = 1; // this will start the sync process, write 1 into the front buf
	status = *(pixel_ctrl_ptr + 3);
	
	while((status & 0x01) != 0){ // polling the status bit, the status bit will turn to 0 when front buffer is done rendering
		status = *(pixel_ctrl_ptr + 3);
	}
}


