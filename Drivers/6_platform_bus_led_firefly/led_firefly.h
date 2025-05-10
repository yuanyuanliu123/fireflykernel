#ifndef _LEDDRV_H
#define _LEDDRV_H
#include "chip_firefly.h"

void led_class_create_device(int minor, const char *name_led);
void led_class_destroy_device(int minor);
void register_led_operations(struct led_operations *opr);

#endif