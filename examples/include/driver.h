#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>

/**
 * driver.h
 * --------
 * acts as a backend for all example code--
 * boots up SDL and creates a nice API (start, update, end) 
 * to abstract all the nasty SDL specifics away
 * 
 */

/***************************************************************
 *                                                             *
 *                 screen dimensions & buffers                 *
 *                                                             *
 ***************************************************************/

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern uint32_t* colors;
extern float* depths;

/***************************************************************
 *                                                             *
 *                        examples' API                        *
 *                                                             *
 ***************************************************************/

extern void
start();

extern void
update(float dt);

extern void
end();

#endif /* DRIVER_H */