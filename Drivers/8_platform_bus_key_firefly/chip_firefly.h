#ifndef __CHIP_H__
#define __CHIP_H__

struct key_operations {
    int (*init) (int which);
    int (*ctl) (int which);
    void (*exit) (int witch);
};

#endif