#ifndef _LED_OPR_H
#define _LED_OPR_H 

struct led_operations {
	int num;
	int (*init) (int which);
	int (*ctl) (int which, char status);
};

struct led_operations *get_board_led_opr(void);

#endif
