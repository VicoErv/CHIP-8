#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 10

SDL_Keycode keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(void)
{
    srand(time(NULL));

    uint8_t buf[4096];
    uint8_t V[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t I = 0;
    uint16_t pc = 0x200;
    uint16_t old_pc = 0x200;

    uint16_t stack[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sp = 0;

    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;

    uint8_t gfx[64 * 32];

    uint8_t keypad[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < 80; i++)
    {
        buf[i] = fontset[i];
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "CHIP-8 Display", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    for (int i = 0; i < 64 * 32; i++)
    {
        gfx[i] = 0;
    }

    FILE *f = fopen("Astro Dodge.ch8", "rb");

    if (!f)
    {
        perror("fopen");

        return 1;
    }

    size_t bytes_read = fread(buf + 0x200, 1, sizeof(buf) - 0x200, f);
    fclose(f);

    SDL_Event e;
    int running = 1;

    int frame_start;
    int frame_time;

    while (running)
    {
        frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }

            if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
            {
                int pressed = (e.type == SDL_KEYDOWN);
                for (int i = 0; i < 16; i++)
                {
                    if (e.key.keysym.sym == keymap[i])
                    {
                        keypad[i] = pressed;
                    }
                }
            }
        }

        for (int cycle = 0; cycle < 10; cycle++)
        {
            uint16_t opcode =
            buf[pc] << 8 |
            buf[pc + 1];

        old_pc = pc;

        int increment_pc = 1;

        switch (opcode & 0xF000)
        {
        case 0x0000:
        {
            switch (opcode & 0x00FF)
            {
            case 0xE0:
            {
                for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
                    gfx[i] = 0;
                break;
            }

            case 0xEE:
            {
                if (sp == 0)
                {
                    printf("Stack underflow!\n");
                    break;
                }
                sp--;
                pc = stack[sp];
                break;
            }

            default:
                printf("Unknown 0x0NNN opcode: 0x%04X\n", opcode);
                break;
            }
            break;
        }
        case 0x1000:
        {
            pc = opcode & 0x0FFF;
            increment_pc = 0;

            break;
        }
        case 0x2000:
        {
            if (sp >= 16)
            {
                printf("Stack overflow!\n");
                break;
            }
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
            increment_pc = 0;

            break;
        }
        case 0x3000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t kk = opcode & 0x00FF;

            if (V[x] == kk)
            {
                pc += 2;
            }

            break;
        }
        case 0x4000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t kk = opcode & 0x00FF;

            if (V[x] != kk)
            {
                pc += 2;
            }

            break;
        }
        case 0x5000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;

            if (V[x] == V[y])
            {
                pc += 2;
            }

            break;
        }
        case 0x6000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t kk = opcode & 0x00FF;
            V[x] = kk;

            break;
        }
        case 0x7000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t kk = opcode & 0x00FF;
            V[x] += kk;

            break;
        }
        case 0x8000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            uint8_t last = opcode & 0x000F;

            switch (last)
            {
            case 0:
            {
                V[x] = V[y];
                break;
            }
            case 1:
            {
                V[x] |= V[y];
                break;
            }
            case 2:
            {
                V[x] &= V[y];
                break;
            }
            case 3:
            {
                V[x] ^= V[y];
                break;
            }
            case 4:
            {
                uint16_t sum = V[x] + V[y];
                V[0xF] = (sum > 0xFF) ? 1 : 0;
                V[x] = sum & 0xFF;
                break;
            }
            case 5:
            {
                uint8_t sub = V[x] - V[y];
                V[0xF] = (V[x] > V[y]) ? 1 : 0;
                V[x] = sub;
                break;
            }
            case 6:
            {
                uint8_t lsb = V[x] & 0x1;
                V[0xF] = lsb;
                V[x] >>= 1;
                break;
            }
            case 7:
            {
                uint8_t subn = V[y] - V[x];
                V[0xF] = (V[y] > V[x]) ? 1 : 0;
                V[x] = subn;
                break;
            }
            case 0xE:
            {
                uint8_t msb = (V[x] & 0x80) >> 7;
                V[0xF] = msb;
                V[x] <<= 1;
                break;
            }

            default:
                printf("Unknown 0x8XYN opcode: 0x%04X\n", opcode);
                break;
            }
            break;
        }
        case 0x9000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;

            if (V[x] != V[y])
            {
                pc += 2;
            }

            break;
        }
        case 0xA000:
        {
            uint16_t nnn = opcode & 0x0FFF;
            I = nnn;
            break;
        }
        case 0xB000:
        {
            uint16_t nnn = opcode & 0x0FFF;
            pc = nnn + V[0];
            increment_pc = 0;
            break;
        }
        case 0xC000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t kk = opcode & 0x0FF;

            uint8_t num = rand() % 0x00FF;
            V[x] = num & kk;
            break;
        }
        case 0xD000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            uint8_t n = opcode & 0x000F;

            V[0xF] = 0;

            for (int row = 0; row < n; row++)
            {
                uint8_t byte = buf[I + row];

                for (int col = 0; col < 8; col++)
                {
                    uint8_t pixel = (byte >> (7 - col)) & 1;

                    int sx = (V[x] + col) % SCREEN_WIDTH;
                    int sy = (V[y] + row) % SCREEN_HEIGHT;

                    uint8_t *screen_pixel = &gfx[sy * SCREEN_WIDTH + sx];

                    if (*screen_pixel & pixel)
                    {
                        V[0xF] = 1;
                    }

                    *screen_pixel ^= pixel;
                }
            }

            break;
        }
        case 0xE000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint16_t nibble = opcode & 0x00FF;

            switch (nibble)
            {
            case 0x9E:
            {
                if (keypad[V[x]])
                {
                    pc += 2;
                    increment_pc = 1;
                }

                break;
            }
            case 0xA1:
            {
                if (!keypad[V[x]])
                {
                    pc += 2;
                    increment_pc = 1;
                }

                break;
            }

            default:
                printf("Unknown 0xEXNN opcode: 0x%04X\n", opcode);
                break;
            }

            break;
        }
        case 0xF000:
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint16_t nibble = opcode & 0x00FF;

            switch (nibble)
            {
            case 0x07:
            {
                V[x] = delay_timer;
                break;
            }
            case 0x0A:
            {
                increment_pc = 0;

                for (int i = 0; i < 16; i++)
                {
                    if (keypad[i])
                    {
                        V[x] = i;
                        pc += 2;
                        increment_pc = 1;
                        break;
                    }
                }

                break;
            }
            case 0x15:
            {
                delay_timer = V[x];
                break;
            }
            case 0x18:
            {
                sound_timer = V[x];
                break;
            }
            case 0x1E:
            {
                I += V[x];
                break;
            }
            case 0x29:
            {
                I = V[x] * 5;
                break;
            }
            case 0x33:
            {
                buf[I] = V[x] / 100;
                buf[I + 1] = (V[x] / 10) % 10;
                buf[I + 2] = V[x] % 10;
                break;
            }
            case 0x55:
            {
                for (int i = 0; i <= x; i++)
                {
                    buf[I + i] = V[i];
                }
                break;
            }
            case 0x65:
            {
                for (int i = 0; i <= x; i++)
                {
                    V[i] = buf[I + i];
                }
                break;
            }

            default:
                printf("Unknown 0xFXNN opcode: 0x%04X\n", opcode);
                break;
            }

            break;
        }

        default:
            printf("Unknown opcode: 0x%04X\n", opcode);
            break;
        }

        if (increment_pc)
        {
            pc += 2;
        }
        }

        if (delay_timer > 0)
            delay_timer--;
        if (sound_timer > 0)
            sound_timer--;

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        for (int py = 0; py < SCREEN_HEIGHT; py++)
        {
            for (int px = 0; px < SCREEN_WIDTH; px++)
            {
                if (gfx[py * SCREEN_WIDTH + px])
                {
                    SDL_Rect r = {px * SCALE, py * SCALE, SCALE, SCALE};
                    SDL_RenderFillRect(ren, &r);
                }
            }
        }

        SDL_RenderPresent(ren);

        frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < 16)
        {
            SDL_Delay(16 - frame_time);
        }
    }

    SDL_Delay(5000);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
