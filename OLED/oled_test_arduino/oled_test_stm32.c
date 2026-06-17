/**
 * @file    oled_test_stm32.c
 * @brief   OLED SSD1306 Test Suite for STM32 (HAL I2C)
 *
 * Target:  STM32F4xx / STM32G0xx (adaptable to any HAL platform)
 * Display: SSD1306 128x64 I2C OLED
 *
 * Usage:
 *   1. Configure I2C1 in STM32CubeMX (Fast Mode 400 kHz)
 *   2. Add this file to your project
 *   3. Call OLED_RunTestSuite() from main()
 *
 * Dependencies:
 *   ssd1306.h / ssd1306.c  — Olivier Van den Eede's portable driver
 *   (https://github.com/afiskon/stm32-ssd1306)
 */

#include "main.h"
#include "ssd1306.h"   /* Portable SSD1306 HAL driver */
#include <string.h>
#include <math.h>
#include <stdlib.h>

/* Adjust to match your CubeMX I2C handle */
extern I2C_HandleTypeDef hi2c1;

#define OLED_W  128
#define OLED_H   64
#define TEST_DELAY_MS  2500

/* ─── Forward Declarations ─────────────────────────────────────────────────── */
static void Test_WhiteFill(void);
static void Test_BlackFill(void);
static void Test_Checkerboard(void);
static void Test_Text(void);
static void Test_Lines(void);
static void Test_Rectangles(void);
static void Test_Circles(void);
static void Test_ProgressBar(void);
static void Test_SineWave(void);
static void Test_PixelNoise(void);
static void Test_Grid(void);
static void Test_Invert(void);

/* ─── Test Table ───────────────────────────────────────────────────────────── */
typedef void (*TestFn)(void);

typedef struct {
    const char *name;
    TestFn      fn;
} TestEntry;

static const TestEntry g_tests[] = {
    { "White Fill",    Test_WhiteFill    },
    { "Black Fill",    Test_BlackFill    },
    { "Checkerboard",  Test_Checkerboard },
    { "Text",          Test_Text         },
    { "Lines",         Test_Lines        },
    { "Rectangles",    Test_Rectangles   },
    { "Circles",       Test_Circles      },
    { "Progress Bar",  Test_ProgressBar  },
    { "Sine Wave",     Test_SineWave     },
    { "Pixel Noise",   Test_PixelNoise   },
    { "Grid",          Test_Grid         },
    { "Invert",        Test_Invert       },
};
#define TEST_COUNT  (sizeof(g_tests) / sizeof(g_tests[0]))

/* ─── Suite Entry Point ────────────────────────────────────────────────────── */
void OLED_RunTestSuite(void) {
    ssd1306_Init();
    HAL_Delay(100);

    for (uint8_t i = 0; i < TEST_COUNT; i++) {
        /* Brief label screen */
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 28);
        ssd1306_WriteString(g_tests[i].name, Font_7x10, White);
        ssd1306_UpdateScreen();
        HAL_Delay(500);

        /* Run the test */
        g_tests[i].fn();
        HAL_Delay(TEST_DELAY_MS);
    }

    ssd1306_Fill(Black);
    ssd1306_SetCursor(10, 28);
    ssd1306_WriteString("All Tests Done!", Font_7x10, White);
    ssd1306_UpdateScreen();
}

/* ─── Individual Tests ─────────────────────────────────────────────────────── */

/* TEST 0 — All pixels ON */
static void Test_WhiteFill(void) {
    ssd1306_Fill(White);
    ssd1306_UpdateScreen();
}

/* TEST 1 — All pixels OFF */
static void Test_BlackFill(void) {
    ssd1306_Fill(Black);
    ssd1306_SetCursor(10, 28);
    ssd1306_WriteString("Black (OFF)", Font_7x10, White);
    ssd1306_UpdateScreen();
}

/* TEST 2 — Checkerboard */
static void Test_Checkerboard(void) {
    ssd1306_Fill(Black);
    for (uint8_t y = 0; y < OLED_H; y++) {
        for (uint8_t x = 0; x < OLED_W; x++) {
            ssd1306_DrawPixel(x, y, (SSD1306_COLOR)((x + y) % 2));
        }
    }
    ssd1306_UpdateScreen();
}

/* TEST 3 — Text rendering */
static void Test_Text(void) {
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("OLED Text Test", Font_7x10, White);
    ssd1306_SetCursor(0, 12);
    ssd1306_WriteString("0123456789", Font_7x10, White);
    ssd1306_SetCursor(0, 24);
    ssd1306_WriteString("ABCDEFGHIJK", Font_7x10, White);
    ssd1306_SetCursor(0, 36);
    ssd1306_WriteString("abcdefghijk", Font_7x10, White);
    ssd1306_SetCursor(0, 48);
    ssd1306_WriteString("!@#$%^&*()_+", Font_7x10, White);
    ssd1306_UpdateScreen();
}

/* TEST 4 — Lines */
static void Test_Lines(void) {
    ssd1306_Fill(Black);
    ssd1306_Line(0, 0, OLED_W-1, OLED_H-1, White);
    ssd1306_Line(OLED_W-1, 0, 0, OLED_H-1, White);
    ssd1306_Line(0, OLED_H/2, OLED_W-1, OLED_H/2, White);
    ssd1306_Line(OLED_W/2, 0, OLED_W/2, OLED_H-1, White);
    ssd1306_UpdateScreen();
}

/* TEST 5 — Rectangles */
static void Test_Rectangles(void) {
    ssd1306_Fill(Black);
    ssd1306_DrawRectangle(0, 0, OLED_W-1, OLED_H-1, White);
    ssd1306_DrawRectangle(8, 8, OLED_W-9, OLED_H-9, White);
    ssd1306_DrawFilledRectangle(24, 20, OLED_W-25, OLED_H-21, White);
    ssd1306_UpdateScreen();
}

/* TEST 6 — Circles */
static void Test_Circles(void) {
    ssd1306_Fill(Black);
    ssd1306_DrawCircle(32, 32, 28, White);
    ssd1306_DrawCircle(96, 32, 28, White);
    ssd1306_DrawFilledCircle(64, 32, 12, White);
    ssd1306_UpdateScreen();
}

/* TEST 7 — Progress bar */
static void Test_ProgressBar(void) {
    for (uint8_t pct = 0; pct <= 100; pct += 25) {
        ssd1306_Fill(Black);
        ssd1306_SetCursor(30, 5);
        ssd1306_WriteString("Loading...", Font_7x10, White);

        /* Outer frame */
        ssd1306_DrawRectangle(4, 25, 123, 38, White);
        /* Fill */
        uint8_t fillW = (uint8_t)((pct * 117UL) / 100U);
        if (fillW > 0)
            ssd1306_DrawFilledRectangle(5, 26, 5 + fillW, 37, White);

        char buf[8];
        snprintf(buf, sizeof(buf), "%3u%%", pct);
        ssd1306_SetCursor(52, 45);
        ssd1306_WriteString(buf, Font_7x10, White);
        ssd1306_UpdateScreen();
        HAL_Delay(600);
    }
}

/* TEST 8 — Sine wave */
static void Test_SineWave(void) {
    ssd1306_Fill(Black);
    int16_t prevY = OLED_H / 2;
    for (uint8_t x = 0; x < OLED_W; x++) {
        float angle = (float)x / OLED_W * 4.0f * (float)M_PI;
        int16_t y = (int16_t)(OLED_H / 2 + (OLED_H / 2 - 4) * sinf(angle));
        if (x > 0) ssd1306_Line(x-1, prevY, x, y, White);
        prevY = y;
    }
    ssd1306_UpdateScreen();
}

/* TEST 9 — Pixel noise (LCG PRNG) */
static void Test_PixelNoise(void) {
    ssd1306_Fill(Black);
    uint32_t seed = 42;
    for (uint16_t i = 0; i < 400; i++) {
        seed = seed * 1103515245UL + 12345UL;
        uint8_t x = (uint8_t)(seed % OLED_W);
        seed = seed * 1103515245UL + 12345UL;
        uint8_t y = (uint8_t)(seed % (OLED_H - 10));
        ssd1306_DrawPixel(x, y, White);
    }
    ssd1306_SetCursor(20, 54);
    ssd1306_WriteString("Pixel Noise", Font_7x10, White);
    ssd1306_UpdateScreen();
}

/* TEST 10 — Grid */
static void Test_Grid(void) {
    ssd1306_Fill(Black);
    for (uint8_t x = 0; x < OLED_W; x += 8)
        ssd1306_Line(x, 0, x, OLED_H-1, White);
    for (uint8_t y = 0; y < OLED_H; y += 8)
        ssd1306_Line(0, y, OLED_W-1, y, White);
    ssd1306_UpdateScreen();
}

/* TEST 11 — Hardware invert */
static void Test_Invert(void) {
    ssd1306_Fill(Black);
    ssd1306_SetCursor(16, 28);
    ssd1306_WriteString("Invert Test", Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(800);

    /* SSD1306 invert command: 0xA7 = invert, 0xA6 = normal */
    uint8_t cmd = 0xA7;
    HAL_I2C_Mem_Write(&hi2c1, 0x3C << 1, 0x00, 1, &cmd, 1, HAL_MAX_DELAY);
    HAL_Delay(1000);
    cmd = 0xA6; /* restore */
    HAL_I2C_Mem_Write(&hi2c1, 0x3C << 1, 0x00, 1, &cmd, 1, HAL_MAX_DELAY);
}
