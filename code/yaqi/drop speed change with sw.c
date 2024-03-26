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
#define BACKGROUND 0x06BF

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0
#define TRUE 1


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


int resolution_x, resolution_y; 							// VGA screen size
int sizeof_pixel;						// number of bytes per pixel
int video_m, video_n;				// number of bits in VGA y coord (m), x coord (n)
int x_box[NUM_BOXES], y_box[NUM_BOXES]; 	// x, y coordinates of boxes to draw
//int very_old_box_x[NUM_BOXES], very_old_box_y[NUM_BOXES];
//int old_box_x[NUM_BOXES], old_box_y[NUM_BOXES];

//int dx_box[NUM_BOXES], dy_box[NUM_BOXES]; // amount to move boxes in animation
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
void fruit_drop(int speed);




void wait_for_vsync(){
	volatile int *pixel_ctrl_ptr = (int*)PIXEL_BUF_CTRL_BASE;
	int status;
	*pixel_ctrl_ptr = 1; // this will start the sync process, write 1 into the front buf
	status = *(pixel_ctrl_ptr + 3);
	
	while((status & 0x01) != 0){ // polling the status bit, the status bit will turn to 0 when front buffer is done rendering
		status = *(pixel_ctrl_ptr + 3);
	}
}


void clear_screen()
{
	int x;
    for (x = 0; x <= 319; x++)
    {
	int y;
        for (y = 0; y <= 239; y++)
        {
			if(y<190)
            	plot_pixel(x, y, BACKGROUND);
			else
				plot_pixel(x, y, WHITE);
				
        }
    }
	
	return;
	
}

//drop the pixel
void plot_pixel(int x, int y, short int line_color)
{ // write pixel values in the frame buffer
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

/*
// using switches to contorl the speed
char seg7[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 
			   0x7f, 0x67, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

void show (int value){
	volatile int *HEX3_0_ptr = (int*)0xFF200020;
	*HEX3_0_ptr = seg7[value & 0xF] |
		seg7[(value >> 4) & 0xF] << 8 |
		seg7[(value >> 8) & 0xF] << 16 |
		seg7[(value >> 12)] << 24;
}
*/

/*
int read_sw(){
	volatile int *sw_ptr = (int *)0xFF200040;
	int sw_value=0;

	while(sw_value == 0){
		sw_value = *sw_ptr;
		switch(sw_value) {
	  		//2 ^ 1: sw1
      		case 2 :
        		sw_value = 1;
         		break;
	  		//2 ^ 2: sw2
      		case 4 :
		 		sw_value = 2;
         		break;
	  		//2 ^ 3: sw3
      		case 8 :
		 		sw_value = 3;  
         		break;
			//2 ^ 4: sw4
      		case 16 :
         		sw_value = 4;  
         		break;
			//2 ^ 5: sw5
      		case 32 :
         		sw_value = 5;  
         		break;
			//2 ^ 6: sw6
	  		case 64 :
         		sw_value = 6;  
         		break;
			
      		default :
				sw_value = 0;  
         		break;
   		}
		
	}

	
	printf("SW:%d \n",sw_value);
	return sw_value;
}
*/

int read_sw(){
	volatile int *sw_ptr = (int *)SW_BASE;
	return *sw_ptr;
}

int main(void){
    while(1){
        volatile int * pixel_ctrl_ptr = (int *) PIXEL_BUF_CTRL_BASE; //(int*) 0xFF203020
        int sw = read_sw();
        printf("%d",sw);
        
        /* initialize the location of the front pixel buffer in the pixel buffer controller */
        *(pixel_ctrl_ptr + 1) = (int) &Buffer1; // first store the address in the back buffer back buffer has the address of BUFFER1
        /* now, swap the front and back buffers, to initialize front pixel buffer location */
        wait_for_vsync();
        /* initialize a pointer to the pixel buffer, used by drawing functions */
        pixel_buffer_start = *pixel_ctrl_ptr;
        /* Erase the pixel buffer */
        //get_screen_specs(); // determine X, Y screen size
        clear_screen();
        /* set a location for the back pixel buffer in the pixel buffer controller
           */
        *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
        clear_screen();

        //creat an array fruit_x[] for the fruits' x value
        int fruits_x[300] = {0};
        int fruits_y[300] = {0};

        // the size of the array is number of fruits would drop
        int num_of_fruits = 0;
        bool game_over = false;
        int total_score = 0;

        while(!game_over){
            while(1){ //1

            
            //clear_screen();
            
            //check if cathed the fruits
            int speed = 1;

            // Adjust speed based on switch value
            if(sw & 0x01) speed = 1; // Fast
            if(sw & 0x02) speed = 2; // Faster
            if(sw & 0x04) speed = 3; // Even faster

            //fruit_drop(speed);
            int fruit_x = 160;
            int fruit_y = 0;
            for (int fruit_y =0 ; fruit_y < 240; fruit_y += speed){
            plot_pixel( fruit_x, fruit_y, 0xF800);
			wait_for_vsync();
            plot_pixel( fruit_x, fruit_y, 0);
            
		    }
		    for (int fruit_y = 239; fruit_y >= 0; fruit_y -= speed){
            plot_pixel( fruit_x, fruit_y, 0xF800);
			wait_for_vsync();
            plot_pixel( fruit_x, fruit_y, 0);
		    }
            
            } //1
        }

    return 0;

    }
}

void fruit_drop(int speed){
    int fruit_x = 160;
    int fruit_y = 0;
    clear_screen();

    while (fruit_y < 240){
        
        plot_pixel( fruit_x, fruit_y, 0xF800);
        wait_for_vsync();
			//erase_line(0, i, 319, i);
        plot_pixel( fruit_x, fruit_y, 0);
        fruit_y += speed;
    }

    while(1){
		for (int fruit_y =0 ; fruit_y < 240; fruit_y += speed){
            plot_pixel( fruit_x, fruit_y, 0xF800);
			wait_for_vsync();
            plot_pixel( fruit_x, fruit_y, 0);
            
		}
		for (int fruit_y = 239; fruit_y >= 0; fruit_y -= speed){
            plot_pixel( fruit_x, fruit_y, 0xF800);
			wait_for_vsync();
            plot_pixel( fruit_x, fruit_y, 0);
		}
	}

    /*
    while(1){
		for (int i =0 ; i < 240; i++){
            plot_pixel( 66, i, 0xF800);
			//draw_line(0, i, 319, i, 0xF800);
			wait_for_vsync();
			//erase_line(0, i, 319, i);
            plot_pixel( 66, i, 0);
		}
		for (int i = 239; i >= 0; i--){
            plot_pixel( 66, i, 0xF800);
			//draw_line(0, i, 319, i, 0xF800);
			wait_for_vsync();
			//erase_line(0, i, 319, i);
            plot_pixel( 66, i, 0);
		}
	}
    */
}
    