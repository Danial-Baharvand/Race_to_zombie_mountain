#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <graphics.h>
#include <sprite.h>
#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <lcd.h>
#include <macros.h>
#include "lcd_model.h"
#include "usb_serial.h"

#define MAX_FUEL 500
Sprite my_car;
Sprite houses[20];
Sprite trees[20];
Sprite barrels[20];
Sprite depots[10];
int house_count=20;
int tree_count=20;
int barrel_count=20;
int depot_count=10;
int distance=0;
float speed=1;
uint8_t max_speed=10;
uint8_t health=5;
int fuel=500;
int top_up=0;
bool left=false;
bool right=false;
bool up=false;
bool down=false;
bool left_button=false;
bool right_button=false;
bool paused=false;
volatile uint8_t state_left = 0;
volatile uint8_t state_right = 0;
volatile uint8_t state_up = 0;
volatile uint8_t state_down = 0;
volatile uint8_t state_left_button = 0;
volatile uint8_t state_right_button = 0;
volatile uint32_t overflow_count = 0;

uint8_t my_car_image[] = {
	0b00011000,
	0b00111100,
	0b11011011,
	0b11011011,
	0b00011000,
	0b00111100,
	0b00111100,
	0b11011011,
	0b11011011,
	0b00111100,
};
uint8_t house_image[] = {
	0b11111111,
	0b10111101,
	0b11011011,
	0b11011011,
	0b10011001,
	0b10111101,
	0b10111101,
	0b11011011,
	0b11011011,
	0b11111111,
};
uint8_t tree_image[] = {
	0b00111000,
	0b01111100,
	0b11111010,
	0b11111111,
	0b11111111,
	0b00111111,
	0b00111000,
	0b00111000,
	0b00111000,
	0b00111000,
};
uint8_t barrel_image[] = {
	0b00000000,
	0b00110000,
	0b01111010,
	0b11111100,
	0b11111110,
	0b01111110,
	0b01111100,
	0b01110000,
	0b00100000,
	0b00000000,
};
uint8_t depot_image[] = {
	0b11111111,
	0b11111111,
	0b11100000,
	0b11111111,
	0b11111111,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
};
void setup_hardware(void){
	TCCR0A = 0;
	TCCR0B = 4; 
	TIMSK0 = 1;
	TCCR3A = 0;
	TCCR3B = 1; 
	TIMSK3 = 1;
	sei();
	CLEAR_BIT(DDRB, 1);
	CLEAR_BIT(DDRD, 0);
	CLEAR_BIT(DDRD, 1);
	CLEAR_BIT(DDRB, 7);
	CLEAR_BIT(DDRF, 5);
	CLEAR_BIT(DDRF, 6);
}
ISR(TIMER3_OVF_vect) {
	if (!paused){
		overflow_count ++;
	}
}
ISR(TIMER0_OVF_vect) {
	state_left = ((state_left << 1) & 0b00111111) | BIT_IS_SET(PINB, 1);
	if (state_left == 0b00111111){
		left = true;
	}
	if (state_left == 0){
		left = false;
	}
	state_right = ((state_right << 1) & 0b00111111) | BIT_IS_SET(PIND, 0);
	if (state_right == 0b00111111){
		right = true;
	}
	if (state_right == 0){
		right = false;
	}
	state_up = ((state_up << 1) & 0b00111111) | BIT_IS_SET(PIND, 1);
	if (state_up == 0b00111111){
		up = true;
	}
	if (state_up == 0){
		up = false;
	}
	state_down = ((state_down << 1) & 0b00111111) | BIT_IS_SET(PINB, 7);
	if (state_down == 0b00111111){
		down = true;
	}
	if (state_down == 0){
		down = false;
	}
	state_left_button = ((state_left_button << 1) & 0b00111111) | BIT_IS_SET(PINF, 6);
	if (state_left_button == 0b00111111){
		left_button = true;
	}
	if (state_left_button == 0){
		left_button = false;
	}
	state_right_button = ((state_right_button << 1) & 0b00111111) | BIT_IS_SET(PINF, 5);
	if (state_right_button == 0b00111111){
		right_button = true;
	}
	if (state_right_button == 0){
		right_button = false;
	}
}
void draw_formatted(int x, int y, const char * format, ...) {
	va_list args;
	va_start(args, format);
	char buffer[100];
	vsprintf(buffer, format, args);
	draw_string(x, y, buffer,1);
}
double elapsed_time(){
	double my_time = ( overflow_count * 65536.0 + TCNT3 ) * 1.0  / 8000000.0;
	return my_time;
}
void pause_menu(void){
	double paused_time=elapsed_time();
	uint32_t overflow_count_back_up=overflow_count;
	while(!down){
		clear_screen();
		
		draw_formatted( 2, 2, "Time:%.2f",paused_time);
		draw_formatted( 2, 15, "Distance:%d",distance);
		show_screen();
		if (left_button){

		}
		if (right_button){
	
		}
	}
	overflow_count=overflow_count_back_up;
}
void splash(void){
	
	
	draw_string( 0, 0, "Race To Zombie ",FG_COLOUR); 
	draw_string( 0, 10, "Mountain",FG_COLOUR); 
	draw_string( 0, 20, "Danial Baharvand",FG_COLOUR); 
	draw_string( 0, 30, "n10084983",FG_COLOUR);
/* 	draw_string( (w/2)-15, (h/2)-9, "By Danial Baharvand",FG_COLOUR);
	draw_string( (w/2)-4, (h/2)-8, "n10084983",FG_COLOUR);
	draw_string( (w/2)-12, (h/2)-7, "Accelerate:W Decelerate:S",FG_COLOUR);
	draw_string( (w/2)-7, (h/2)-6, "Left:A Right:D",FG_COLOUR);
	draw_string( (w/2)-15, (h/2)-5, "Stop buy fuel depots to refuel",FG_COLOUR);
	draw_string( (w/2)-19, (h/2)-4, "Compelete 5 laps and reach 10KM to WIN",FG_COLOUR);
	draw_string( (w/2)-11, (h/2)-3, "Press ANY key to START",FG_COLOUR); */
	show_screen();
}
void wait_for_key(void){
	while(!BIT_IS_SET(PINF, 5) && !BIT_IS_SET(PINF, 6)){
		//do nothing
	}
}
float sprite_x(Sprite sprite_name){
	return sprite_name.x;
}
float sprite_y(Sprite sprite_name){
	return sprite_name.y;
}
/* void sprite_move(Sprite sprite_name,int dx, int dy){
	sprite_name.x += dx;
	sprite_name.y += dy;
} */
void draw_road(void){
	int w = LCD_X;
	int h = LCD_Y;
	draw_line( w/4, 1, w/4, h-2, 1 );
	draw_line( (w/4)*3, 1, (w/4)*3, h-2, 1 );
	draw_line( w/4, -10000, (w/4)*3, -10000, 1 );
}
int road_x(int abs_y){
	int y= 5*(sin(0.1*(/* LCD_Y- */LCD_Y-abs_y-distance)))+21;
	return y;
}
void draw_sin_road(int distance){
	uint8_t x;
	int16_t y;
	for (int i=LCD_Y-distance;i>=-distance;i=i-1){
		x=5*(sin(0.1*i))+21;
		y=i+distance;
		draw_pixel(x, y, 1);
		draw_pixel(x+42, y, 1);
	}
}
void speed_check(void){
	if (my_car.x > road_x(12) && my_car.x<road_x(12)+42-8){
		max_speed=10;
	}
	else if(speed>3){
		max_speed=3;
		speed=max_speed;
	} else{
			max_speed=3;
	}
}
void consume_fuel(void){
	fuel=fuel-speed;
}
/* void reset_fuel_timer(void){
	if (speed>0){
		timer_reset(fuel_timer);
	}
} */
/* void update_top_up(void){
	if (speed>0){
		top_up=(MAX_FUEL-fuel)/30;
	}
} */
void refuel(void){
	for ( int i = 0; i < depot_count; i++ ) {
		if ((abs(depots[i].y-(my_car.y-5)))<10 && abs(depots[i].x-my_car.x)<20 && speed==0 && fuel<MAX_FUEL ){
			/* fuel=fuel+top_up; */		
			fuel=fuel+16;
		}
	}
}
void draw_fuel(void){
	uint8_t fuel_meter=0b00000000;
	for (int i=fuel/100;i>0;i--){
		SET_BIT(fuel_meter, abs(i-5)+1);
	}
		SET_BIT(fuel_meter, 0);
		SET_BIT(fuel_meter, 7);
		lcd_position(2, 0);
		lcd_write(1, fuel_meter);
}
void draw_speed(void){
	uint8_t speed_meter=0b00000000;
	for (int i=speed/2;i>0;i--){
		SET_BIT(speed_meter, abs(i-5)+1);
	}
		SET_BIT(speed_meter, 0);
		SET_BIT(speed_meter, 7);
		lcd_position(4, 0);
		lcd_write(1, speed_meter);
}
void draw_health(void){
	uint8_t health_meter=0b00000000;
	for (int i=health;i>0;i--){
		SET_BIT(health_meter, abs(i-5)+1);
	}
		SET_BIT(health_meter, 0);
		SET_BIT(health_meter, 7);
		lcd_position(6, 0);
		lcd_write(1, health_meter);
}
	
void draw_dashboard_frame(void){
	uint8_t space =0b10000001;
	uint8_t line = 0b11111111;
	lcd_position(0, 0);
	lcd_write(1, line);
	lcd_position(1, 0);
	lcd_write(1, space);
	lcd_position(3, 0);
	lcd_write(1, space);
	lcd_position(5, 0);
	lcd_write(1, space);
	lcd_position(7, 0);
	lcd_write(1, space);
	lcd_position(8, 0);
	lcd_write(1, line);
	
}
void draw_dashboard(void){
	draw_dashboard_frame();
	draw_fuel();
	draw_speed();
	draw_health();
	
}
void create_my_car(void){
	int w = LCD_X;
	int h = LCD_Y;
	int my_car_x = (w/2)-4 ;
	int my_car_y = (h*75)/100 ;
	sprite_init(&my_car,my_car_x, my_car_y, 8, 10, my_car_image);
}	

void create_houses(void){
	for ( int i = 0; i < house_count; i++ ) {
		int hy = (LCD_Y - rand() %1000) + 1 ;
		int w=5*(sin(0.1*LCD_Y-hy))+3;
		int hx = rand() % (w) +1 ;
		sprite_init(&houses[i], hx, hy, 8, 10, house_image);
		
	}
}
void create_trees(void){
	for ( int i = 0; i < tree_count; i++ ) {
		int hy = (LCD_Y - rand() %1000) + 1 ;
		int w=5*(sin(0.1*LCD_Y-hy))+3;
		int hx = rand() % (w) +68 ;
		sprite_init(&trees[i], hx, hy, 8, 10, tree_image);
		
	}
}
/* void create_barrels(void){
	for ( int i = 0; i < barrel_count; i++ ) {
		int hy = (LCD_Y - rand() %1000) + 1 ;
		int hx = (rand() % 25)+(5*(sin(0.1*hy))+26);
		sprite_init(&barrels[i], hx, hy, 8, 10, barrel_image);
		
	}
} */
void create_barrels(void){
	for ( int i = 0; i < barrel_count; i++ ) {
		int hy = (LCD_Y - rand() %1000) + 1 ;
		int hx = (rand() % 20)+road_x(hy)+8;
		sprite_init(&barrels[i], hx, hy, 8, 10, barrel_image);
		
	}
}
void create_depots(void){
	for ( int i = 0; i < depot_count; i++ ) {
		int hy = (LCD_Y - rand() %1000) + 1 ;
		int hx;
		if (rand()%2==0){
		hx = 8;
		}else{
		hx = 68;
	}
		sprite_init(&depots[i], hx, hy, 8, 10, depot_image);
	}
}
void step_my_car(void) {
	if ( left && speed>=1 ) {
		if ( my_car.x > 1 ) {
			my_car.x-=speed;
		}
	}
	else if ( right && speed>=1 ) {
		if ( my_car.x < LCD_X - 8 ) {
			my_car.x+=speed;
		}
	}
	else if ( right_button ) {
		if ( speed>0) {
		speed=speed-0.5;
		if(speed<0){
			speed=0;
		}
		}
	}
	else if ( left_button ) {
		if ( speed<max_speed) {
			if (max_speed==10){
				speed=speed+0.18;
			}else if(max_speed==3){
				speed=speed+0.04;
			}
		}
	}
	else if ( up ) {
		pause_menu();
	}
	else if (speed >=2){
		if(max_speed==10){
			speed=speed-0.3;
		}
		if(max_speed==3){
			speed=speed-0.06;
		}
	}
	else if (speed <1){
		if(max_speed==10){
			speed=speed+0.05;
		}
		if(max_speed==3){
			speed=speed+0.03;
		}
	}
	speed_check();
}
void step(Sprite sprite_array[],int sprite_count,int x_step,int y_step){
	for ( int i = 0; i < sprite_count; i++ ) {
	/* 	sprite_move( sprite_array[i], x_step, y_step ); */
		sprite_array[i].x+=x_step;
		sprite_array[i].y+=y_step;
	}
}
void step_enviroment(int speed){
	step(houses,house_count,0,speed);
	step(trees,tree_count,0,speed);
	step(barrels,barrel_count,0,speed);
	step(depots,depot_count,0,speed);
}
void draw_sprites (Sprite sprite[], int sprite_count){
	for ( int i = 0; i < sprite_count; i++ ) {
		sprite_draw(&sprite[i]);
	}
}
void draw_enviroment(void){
	draw_sprites (houses, house_count);
	draw_sprites (barrels, barrel_count);
	draw_sprites (depots, depot_count);
	draw_sprites (trees, tree_count);
}


int collided_with (Sprite obs ) {
	int tc = my_car.y;
	int lc = my_car.x;
	int rc = lc + 8 - 1;
	int bc = tc + 10 - 1;
	
	int to = obs.y;
	int lo = obs.x;
	int ro = lo + 8 - 1;
	int bo = to + 10 - 1;
	
	if ( to > bc ) return 0;
	if ( tc > bo ) return 0;
	if ( lc > ro ) return 0;
	if ( lo > rc ) return 0;
	
	
	return 1;
}
int calc_damage(void){
	int damage=0;
	for ( int i = 0; i < house_count; i++ ) {
		damage=damage + collided_with (houses[i] );
	}
	for ( int i = 0; i < tree_count; i++ ) {
		damage=damage + collided_with (trees[i] );
	}
	for ( int i = 0; i < barrel_count; i++ ) {
		damage=damage + collided_with (barrels[i] );
	}
	for ( int i = 0; i < depot_count; i++ ) {
		damage=damage + collided_with (depots[i] )*5;
	}
	return damage;
		
}
void collision_handler(){
	if (calc_damage()>0){
		health=health-calc_damage();
		fuel=MAX_FUEL;
		speed=0;
		while (calc_damage()>0 || !max_speed==10){
			/* sprite_move_to( my_car, (rand()%(w/2-11))+w/4+1, (h*75)/100 ); */
			my_car.x=(rand() % 30)+road_x(12)+4;
		}
	}
}
int main(void){
	set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
	setup_hardware();
	splash();
	wait_for_key();
	srand(overflow_count);
	overflow_count = 0;
	clear_screen();
	create_my_car();
	create_houses();
	create_trees();
	create_barrels();
	create_depots();
	show_screen();
	while(true){
		clear_screen();
		step_my_car();
		draw_sin_road(distance);
		sprite_draw( &my_car );
		step_enviroment(speed);
		draw_enviroment();
		distance= distance+speed;
		consume_fuel();
		/* update_top_up(); */
		refuel();
		collision_handler();
		show_screen();
		draw_dashboard();
		
		_delay_ms(30);
	}
}