#ifndef _KEYDRV_H
#define _KEYDRV_H
#include "chip_firefly.h"

void key_class_create_device(int minor, const char *name_key);
void key_class_destroy_device(int minor);
void register_key_operations(struct key_operations *opr);

#endif