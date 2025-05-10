#ifndef __CHIP_H__
#define __CHIP_H__

struct led_operations {
    int (*init) (int which);
    int (*ctl) (int which, char status);
    void (*exit) (int witch);
};

#endif