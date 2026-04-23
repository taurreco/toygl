
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#include "driver.h"

/**
 * driver.c
 * --------
 * acts as a backend for all example code--
 * boots up SDL and creates a nice API (start, update, end) 
 * to abstract all the nasty SDL specifics away
 * 
 */

/***************************************************************
 *                                                             *
 *                       initialize data                       *
 *                                                             *
 ***************************************************************/

int SCREEN_WIDTH = 1024;
int SCREEN_HEIGHT = 768;

uint32_t* colors;
float* depths;

/***************************************************************
 *                                                             *
 *                        render loop                          *
 *                                                             *
 ***************************************************************/

/********
 * main *
 ********/

/* entry point of the SDL program */
int 
main(int argc, char* args[])
{
    /* allocate frame buffers */

    colors = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    depths = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(float));

    start();     /* call example's start code */

    /* SDL setup magic */

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "SDL", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, 
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1, 
        SDL_RENDERER_ACCELERATED
    );

    SDL_Texture* texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    /* the loop */
    
    int quit = 0;
    int cur_time = 0;
    int prev_sec = 0;
    int prev_time = 0;
    int frame = 0;

    while (!quit) {

        cur_time = SDL_GetTicks();
        
        /* check if quit */

        SDL_Event event;
        while ((SDL_PollEvent(&event)) != 0) {
            /* request quit */
            if (event.type == SDL_QUIT) { 
                quit = 1;
            }
        }

        /* blit to screen */

        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            colors[i] = 0;
            depths[i] = 1000;
        }
        float dt = (cur_time - prev_time) / (float) 1000;

     
        update(dt);   /* call example's update code */

        SDL_UpdateTexture(texture, NULL, colors, SCREEN_WIDTH * 4);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        /* display fps */

        if (cur_time > prev_sec + 1000) {
            printf("FPS: %d\n", frame);
            frame = 0;
            prev_sec = cur_time;
        }

        prev_time = cur_time;
        frame++;
    }

    /* free program memory */

    end();  /* call example's end code */
    free(colors);
    free(depths);
    
    /* free SDL memory */

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    /* quit SDL subsystems */
    SDL_Quit();

    return 0;
}