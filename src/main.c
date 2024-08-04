#include "SDL_render.h"
#include "cartesian.h"
#include <SDL.h>
#include <math.h>
#include <stdio.h>

#define SCREEN_HEIGHT 400
#define SCREEN_WIDTH 640

uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
// clang-format off
uint8_t mapdata[24*24] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1,
    1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1,
    1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};
// clang-format on

int running = 1;

float angular_velocity = 0.01f;
float move_speed = 0.5f;

int scale = 15;

v2 pos = {22, 28};
v2 dir = {1, 0};
v2 camera_plane = {0, 0.66f};
void handle_input();

int main() {
    SDL_Window *window =
        SDL_CreateWindow("raycasting", 20, 20, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Event e;
    while (running) {
        for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
            pixels[i] = 0x0;
        }
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                running = 0;
            }
        }

        for (int x = 0; x < SCREEN_WIDTH; x++) {
            double cameraX = 2 * x / (double)SCREEN_WIDTH - 1;
            double rayDirX = dir.x + camera_plane.x * cameraX;
            double rayDirY = dir.y + camera_plane.y * cameraX;

            int mapX = (int)pos.x, mapY = (int)pos.y;
            double sideDistX, sideDistY;

            double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);
            double perpWallDist;

            int stepX, stepY;

            int hit = 0, side;
            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = (pos.x - mapX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (mapX + 1.0 - pos.x) * deltaDistX;
            }

            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = (pos.y - mapY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (mapY + 1.0 - pos.y) * deltaDistY;
            }

            while (hit == 0) {
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                } else {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    if (mapY < 0) {
                        mapY = 0;
                    }
                    side = 1;
                }

                if (mapdata[(SCREEN_WIDTH)*mapY + mapX] > 0) {
                    printf("%d\t%d\n", mapY, mapX);
                    hit = 1;
                }
            }

            if (side == 0)
                perpWallDist = (sideDistX - deltaDistX);
            else
                perpWallDist = (sideDistY - deltaDistY);

            int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist) * scale;
            int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawStart < 0)
                drawStart = 0;
            int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawEnd >= SCREEN_HEIGHT)
                drawEnd = SCREEN_HEIGHT - 1;
            for (int i = drawStart; i <= drawEnd; i++) {
                pixels[(SCREEN_WIDTH)*i + x] = 0xFFFFFFFF;
            }
        }
        handle_input();

        SDL_UpdateTexture(screen, NULL, pixels, SCREEN_WIDTH * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopyEx(renderer, screen, NULL, NULL, 0.0, NULL,
                         SDL_FLIP_NONE);
        SDL_RenderPresent(renderer);

        // printf("x:%f\ty:%f\n", dir.x, dir.y);
    }

    return 0;
}

void handle_input() {
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_W]) {
        pos.x += dir.x * move_speed;
        pos.y += dir.y * move_speed;
    }

    if (state[SDL_SCANCODE_A]) {
        rotate(&dir, -angular_velocity);
        rotate(&camera_plane, -angular_velocity);
    }
    if (state[SDL_SCANCODE_D]) {
        rotate(&dir, angular_velocity);
        rotate(&camera_plane, angular_velocity);
    }
}
