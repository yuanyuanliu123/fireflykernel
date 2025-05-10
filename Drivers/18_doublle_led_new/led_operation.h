#ifndef _LED_OPR_H
#define _LED_OPR_H 

struct led_operations {
	int num;
	int (*init) (void);
	int (*ctl) (int which, char status);
	int (*exit) (void);
};

struct led_operations *get_board_led_opr(void);

#endif
