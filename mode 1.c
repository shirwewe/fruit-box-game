// goal: mode 1: catch more fruit as you can 
// Game over when: 1. catch a bomb, 2 loosing 3 fruits

// Fruits would drop at constant speed with random place, the speed of fruit dropping is not changing for now 
// However, the num of the bombs will increase while time goes while num of fruits is not changing.

/* This files provides address values that exist in the system */

#include <stdlib.h>
#include <graphicd.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Cyclone V FPGA devices */
#define LEDR_BASE             ((volatile int *)0xFF200000)
#define HEX3_HEX0_BASE        ((volatile int *)0xFF200020)
#define HEX5_HEX4_BASE        ((volatile int *)0xFF200030)
#define SW_BASE               ((volatile int *)0xFF200040)
#define KEY_BASE              ((volatile int *)0xFF200050)
#define TIMER_BASE            ((volatile int *)0xFF202000)
#define PIXEL_BUF_CTRL_BASE   ((volatile int *)0xFF203020)
#define CHAR_BUF_CTRL_BASE    ((volatile int *)0xFF203030)

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

int prev_fruit_x, prev_fruit_y; 	// x, y coordinates of boxes to draw
int prev_prev_fruit_x, prev_prev_fruit_y; 	// x, y coordinates of boxes to draw

int dx_basket, dy_basket; // amount to move boxes in animation
int color_box[NUM_BOXES];						// color
unsigned int color[] = {WHITE, YELLOW, RED, GREEN, BLUE, CYAN, MAGENTA, GREY, PINK, ORANGE};
int pixel_buffer_start;


short int Buffer1[240][512]; // 240 rows, 320 columns + paddings
short int Buffer2[240][512];

typedef struct {
	char name[12];
	int width;
	int height;
	int *fruit_image_map;
} Fruit;

Fruit fruits[] = {
	{"WATERMELON", 41, 33, watermelon_map},
	{"BANANA", 39, 40, banana_map},
	{"MANDARIN", 32, 38, mandarin_map},
	{"APPLE",27, 25, apple_map},
	{"BLUEBERRY", 16, 20, blueberry_map},
	{"GRAPE", 30, 26, grape_map},
	{"ORANGE", 19, 40, orange_map},
	{"PEACH", 32, 33, peach_map},
	{"STRAWBERRY", 18, 14, strawberry_map}
};
int num_of_fruits = sizeof(fruits) / sizeof(fruits[0]);

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
void draw_bomb (int x, int y);
void erase_bomb(int x, int y);


/******************************************************************************
 * This program draws rectangles and boxes on the VGA screen, and moves them
 * in an animated way.
 *****************************************************************************/
int main(void){

    volatile int * pixel_ctrl_ptr = (int *) PIXEL_BUF_CTRL_BASE; // pixel controller
	//volatile int *LEDR_ptr = LEDR_BASE;
    volatile int *KEY_ptr = KEY_BASE;
	//volatile int *SW_ptr = SW_BASE;
    // declare other variables(not shown)
	sizeof_pixel = 2; 
	video_m = 8; // y has 8 bits
	video_n = 9; // x has 9 bits
	Fruit drop_this_fruit;
	printf("pass 1");

    // initialize location and direction of rectangles(not shown)
    /* initialize the location of the front pixel buffer in the pixel buffer controller */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; // first store the address in the back buffer back buffer has the address of BUFFER1
    /* now, swap the front and back buffers, to initialize front pixel buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    /* Erase the pixel buffer */
    get_screen_specs(); // determine X, Y screen size
    //clear_screen();

    /* set a location for the back pixel buffer in the pixel buffer controller
        */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	printf("pass 2");
    clear_screen();
	printf("pass 3");
	initializer();
	printf("pass 4");

    while (1)
    {
        // create an array of fruits 
        // create an array of bombs

        // create an array bomb_y[] for each bomb's y value
	    // create an array bomb_x[] for each bomb's x value
	    int bomb_x[300] = {0};
	    int bomb_y[300] = {0};
		int fruit_x_pos[num_of_fruits]; 
    	int fruit_y_pos[num_of_fruits];
		
		int prev_fruit_x[num_of_fruits];
		int prev_fruit_y[num_of_fruits];

        srand(time(NULL)); //check this!!!!!!

        // the size of the array is: num_of_bombs
	    int num_of_bombs = 0;
		int total_fruit_dropped = 0;
	    bool game_over = false;
	    int total_score = 0;
		int gameTime = 0;
		printf("pass 5");

        // check if game is over or not
        while (!game_over){

			gameTime++;
			
			for (int i = 0; i < total_fruit_dropped; i++) {
        		erase_fruit(prev_fruit_x[i], prev_fruit_y[i], fruits[i].width, fruits[i].height);
    		}
			erase_basket(prev_prev_basket_x, prev_prev_basket_y);
			erase_bomb(prev_prev_bomb_x, prev_prev_bomb_y); // has to change
		    draw_basket(basket_x_pos, basket_y_pos);
			draw_bomb(bomb_x, bomb_y); // Has to change
			
			prev_prev_basket_x = prev_basket_x;
    		prev_prev_basket_y = prev_basket_y;
    		prev_basket_x = basket_x_pos;
    		prev_basket_y = basket_y_pos;
			
			for (int i = 0; i < num_of_fruits; i++) {
    			prev_fruit_x[i] = fruit_x_pos[i];
    			prev_fruit_y[i] = fruit_y_pos[i];
			}
			printf("pass 6");

            /* Erase any boxes and lines that were drawn in the last iteration 
		    erase_fruit(prev_prev_fruit_x, prev_prev_fruit_y, fruit.width, WATERMELON_HEIGHT);
		    erase_basket(prev_prev_basket_x, prev_prev_basket_y);
		    draw_basket(basket_x_pos, basket_y_pos);
		    // draw_fruit(fruit_x_pos, fruit_y_pos, WATERMELON_WIDTH, WATERMELON_HEIGHT, watermelon_map);
		
		    //draw_fruit(100, 0, BANANA_WIDTH, BANANA_HEIGHT, banana_map);
		    //draw_fruit(200, 0, MANDARIN_WIDTH, MANDARIN_HEIGHT, mandarin_map);
		
		    prev_prev_basket_x = prev_basket_x;
		    prev_prev_basket_y = prev_basket_y;
		    prev_basket_x = basket_x_pos;
		    prev_basket_y = basket_y_pos; 
		
		
		    prev_prev_fruit_y = prev_fruit_y;
		    prev_prev_fruit_x = prev_fruit_x;
		
		    //prev_fruit_y = fruit_y_pos;
		    //prev_fruit_x = fruit_x_pos;
		*/
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
		
		
		    //if(fruit_x_pos - WATERMELON_WIDTH == basket_x_pos && fruit_y_pos - ){
		//	continue;
		//}
		/*
		    int SW_value = *(SW_ptr);
		    switch (SW_value) {
  			case 0b1:
				fruit_y_pos += 2;
				if(fruit_y_pos > 240){
					fruit_y_pos -= 2;
				}
    		break;
  			case 0b10:
				fruit_y_pos += 4;
				if(fruit_y_pos > 240){
					fruit_y_pos -= 4;
				}
    		break;
			case 0b100:
				fruit_y_pos += 8;
				if(fruit_y_pos > 240){
					fruit_y_pos -= 8;
				}
    		break;
			case 0b1000:
				fruit_y_pos += 16;
				if(fruit_y_pos > 240){
					fruit_y_pos -= 16;
				}
    		break;
  			default:
				fruit_y_pos++;
				//if(fruit_y_pos > 240){
					//fruit_y_pos -= 1;
				//}
   	
			}
		*/
			printf("gameTime now is %d \n", gameTime);
            // fruit drops
			if (gameTime % 5 == 1){
				printf("can a fruit drop \n");
				// generate fruit
				int fruit_x = rand() % RESOLUTION_X;
				int fruit_y = 0;
				int random_fruit = rand() % num_of_fruits;
				drop_this_fruit = fruits[random_fruit];
				printf("pass 7");
				draw_fruit(fruit_x, fruit_y, drop_this_fruit.width, drop_this_fruit.height, drop_this_fruit.fruit_image_map);
				fruit_x_pos[random_fruit] = fruit_x;
				fruit_y_pos[random_fruit] = fruit_y;
				total_fruit_dropped += 1;
				printf("pass 8");

			}
			printf("gameTime now is %d \n", gameTime);
			//As time goes, the num of bombs is increasing by generateing boomb faster
			if ((gameTime % (100 - gameTime / 4)) - 10  == 0){
				
				printf("bomb can drop \n");
				
				//generate a bomb
				int new_bomb_x = rand() % RESOLUTION_X;
				draw_bomb(new_bomb_x, 0); /// HAS to Change!!!!
				printf("bomb draw");

				// update the bomb position to the num_of bomb array
				bomb_x[num_of_bombs] = new_bomb_x; 
				bomb_y[num_of_bombs] = 0;
				num_of_bombs ++;
				printf("num of bomb is now %d \n", num_of_bombs);
				//generate a bomb
			}
			printf("num of bomb is now %d \n", num_of_bombs);

            //Determine if it catched the fruit
            // 1. check if the fruit reaches the basket level, --> 99!!! has to change
            //         -> if yes, initialize back to 0 which means the fruits wether be catched or lost
			for (int i = 0; i < num_of_bombs; i++){

            	if(bomb_y[i] == BASKET_HEIGHT){ 
					bomb_y[i] = 0;
				
					//2. check if it catched the bomb
                	// if the position of the bomb >= the postion of the left basket && <= the postion of right point of the basket, game over
					// if over 1/2 of the bomb fall in the basket, game over
					if(basket_x_pos < (bomb_x[i] + 0.5*(BOMB_WIDTH)) && 
						(basket_x_pos + BASKET_WIDTH) > (bomb_x[i] + 0.5*(BOMB_WIDTH))){ //check the postion!!!!!!
					//the bomb is catched
					//call function to draw game over
						//draw_game_over();
						game_over = true;
						break;
					}	
                	// else if not reached to the basket
                	else if(bomb_y[i] < (RESOLUTION_Y - BASKET_HEIGHT)){
						draw_bomb(bomb_x[i], bomb_y[i]);// CHange this!!!
						bomb_y[i] += 1;
			    	}		
				
				}
			}
			
			printf ("total_fruit_dropped is %d \n", total_fruit_dropped);
			for (int i = 0; i <= total_fruit_dropped; i++){
				if (fruit_y_pos[i] == BASKET_HEIGHT){// check when fruits gets to the basket
					if (fruit_x_pos[i] >= basket_x_pos && 
						fruit_x_pos[i] <= (basket_x_pos + BASKET_WIDTH)){
						total_score +=1;
						printf ("Score: %d \n", total_score);
						}
					}
					else if(fruit_y_pos[i] < (RESOLUTION_Y - BASKET_HEIGHT)){
						draw_fruit( fruit_x_pos[i], fruit_y_pos[i], drop_this_fruit.width, drop_this_fruit.height, drop_this_fruit.fruit_image_map);
						fruit_y_pos[i] += 1;
			    	}	
						 
				}
			
				wait_for_vsync(); // swap front and back buffers on VGA vertical sync
				pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer	
						 				 
			}
			              
            // need a count score function to test if this actual works
            // need an array that store the fruits, and radomly drops from randomly location 
        	// all in a for loop, need to keep track on the time, need to decide how the bomb can be change with the time goes
			

			clear_screen();
			wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer	
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
	printf("draw fruit exist \n");
}


void draw_bomb(int x, int y){
	for (int i = y ; i < RESOLUTION_Y; i++){ // i is basket height
		for (int j = x; j < x + BOMB_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, bomb_map[(i-y)*BOMB_WIDTH+(j-x)]);
		} 
	}
}

/*
void draw_game_over(){

}
*/

void erase_basket(int x, int y){
	for (int i = y ; i < RESOLUTION_Y; i++){ // i is basket height
		for (int j = x; j < x + BASKET_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, 0);
		} 
	}
	
}

void erase_bomb(int x, int y){
	for (int i = y ; i < RESOLUTION_Y; i++){ // i is basket height
		for (int j = x; j < x + BOMB_WIDTH; j++){ // j is basket width
			plot_pixel(j, i, 0);
		} 
	}
	
}

void initializer(){

	// initialzies basket start position
	basket_y_pos = RESOLUTION_Y - BASKET_HEIGHT;
	basket_x_pos = (RESOLUTION_X/2) - (BASKET_WIDTH/2);
	
	//int fruit_y_pos[num_of_fruits] = {0};
	//int fruit_x_pos[num_of_fruits] = {0};
	
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

void plot_pixel(int x, int y, short int color) {
    int shift_x, shift_y;
    shift_x = sizeof_pixel - 1; // shift x address bits by sizeof(pixel)
    shift_y = video_n + (sizeof_pixel - 1); // shift y address by |x address| + sizeof(pixel)
    // 计算内存地址时确保不越界
    if (x >= 0 && x < resolution_x && y >= 0 && y < resolution_y) {
        *(short int *)(pixel_buffer_start + (y << shift_y) + (x << shift_x)) = color;
    }
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
