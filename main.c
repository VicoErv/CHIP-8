#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint8_t buf[4096];
    uint8_t V[16];
    uint16_t I = 0;
    uint16_t pc = 0x200;

    uint16_t stack[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sp = 0;

    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;

    uint8_t gfx[64 * 32];

    uint8_t keypad[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (int i = 0; i < 64 * 32; i++)
    {
        gfx[i] = 0;
    }

    FILE *f = fopen("IBM Logo.ch8", "rb");

    if (!f)
    {
        perror("fopen");

        return 1;
    }

    size_t bytes_read = fread(buf + 0x200, 1, sizeof(buf) - 0x200, f);
    fclose(f);

    while (1)
    {
        uint16_t opcode =
            buf[pc] << 8 |
            buf[pc + 1];

        pc += 2;

        switch (opcode & 0xF000)
        {
        case 0x1000:
        {
            pc = opcode & 0x0FFF;

            break;
        }
        case 0x2000:
        {
            stack[sp] == pc;
            sp++;
            pc = opcode & 0x0FFF;

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
            }

            break;
        }
        }
    }

    printf("Read %zu bytes \n", bytes_read);

    return 0;
}
