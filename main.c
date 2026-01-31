#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "ra8876.h"

static ra8876_t display = {
    .spi = spi0,
    .pin_miso = 16,
    .pin_cs = 17,
    .pin_sck = 18,
    .pin_mosi = 19,
    .spi_speed = 20000000
};

void demo1_shapes(void) {
    printf("Demo 1: Shapes\n");

    ra8876_fill_screen(&display, RA8876_BLACK);
    ra8876_print(&display, 400, 10, RA8876_WHITE, "Shape Demo");

    ra8876_print(&display, 10, 50, RA8876_GRAY, "Grid:");
    for (int x = 100; x < 400; x += 50) {
        ra8876_draw_line(&display, x, 80, x, 280, RA8876_DARKGRAY);
    }
    for (int y = 80; y < 300; y += 50) {
        ra8876_draw_line(&display, 100, y, 400, y, RA8876_DARKGRAY);
    }

    ra8876_print(&display, 450, 50, RA8876_GRAY, "Rectangles:");
    ra8876_draw_rect(&display, 450, 80, 101, 71, RA8876_RED);
    ra8876_fill_rect(&display, 560, 80, 101, 71, RA8876_GREEN);
    ra8876_draw_rect(&display, 670, 80, 101, 71, RA8876_BLUE);
    ra8876_fill_rect(&display, 680, 90, 81, 51, RA8876_CYAN);

    ra8876_print(&display, 800, 50, RA8876_GRAY, "Rounded:");
    ra8876_draw_rounded_rect(&display, 800, 80, 121, 71, 15, RA8876_YELLOW);
    ra8876_fill_rounded_rect(&display, 810, 90, 101, 51, 10, RA8876_ORANGE);

    ra8876_print(&display, 450, 170, RA8876_GRAY, "Circles:");
    ra8876_draw_circle(&display, 500, 240, 40, RA8876_YELLOW);
    ra8876_fill_circle(&display, 600, 240, 40, RA8876_MAGENTA);
    ra8876_draw_circle(&display, 700, 240, 50, RA8876_WHITE);
    ra8876_fill_circle(&display, 700, 240, 30, RA8876_ORANGE);

    ra8876_print(&display, 10, 320, RA8876_GRAY, "Ellipses:");
    ra8876_draw_ellipse(&display, 150, 400, 80, 40, RA8876_CYAN);
    ra8876_fill_ellipse(&display, 300, 400, 40, 60, RA8876_RED);
    ra8876_draw_ellipse(&display, 450, 400, 60, 30, RA8876_GREEN);
    ra8876_fill_ellipse(&display, 450, 400, 40, 20, RA8876_YELLOW);

    ra8876_print(&display, 550, 320, RA8876_GRAY, "Triangles:");
    ra8876_draw_triangle(&display, 600, 350, 550, 450, 650, 450, RA8876_WHITE);
    ra8876_fill_triangle(&display, 750, 350, 700, 450, 800, 450, RA8876_BLUE);
    ra8876_fill_triangle(&display, 900, 350, 850, 450, 950, 450, RA8876_RED);
    ra8876_draw_triangle(&display, 900, 350, 850, 450, 950, 450, RA8876_WHITE);

    ra8876_print(&display, 10, 480, RA8876_GRAY, "Lines:");
    for (int i = 0; i < 10; i++) {
        uint16_t color = ra8876_rgb565(255 - i*25, i*25, 128);
        ra8876_draw_line(&display, 100 + i*30, 510, 100 + i*30 + 100, 580, color);
    }

    ra8876_print(&display, 500, 480, RA8876_GRAY, "BTE Expand (1bpp->16bpp):");
    static const uint8_t icon_32x32[128] = {
        0x00,0x00,0x00,0x00, 0x00,0x7F,0xFE,0x00, 0x01,0xFF,0xFF,0x80, 0x03,0xFF,0xFF,0xC0,
        0x07,0xFF,0xFF,0xE0, 0x0F,0xC0,0x03,0xF0, 0x1F,0x00,0x00,0xF8, 0x1E,0x00,0x00,0x78,
        0x3C,0x00,0x00,0x3C, 0x3C,0x00,0x00,0x3C, 0x78,0x00,0x00,0x1E, 0x78,0x00,0x00,0x1E,
        0x70,0x00,0x00,0x0E, 0xF0,0x00,0x00,0x0F, 0xF0,0x00,0x00,0x0F, 0xF0,0x00,0x00,0x0F,
        0xF0,0x00,0x00,0x0F, 0xF0,0x00,0x00,0x0F, 0xF0,0x00,0x00,0x0F, 0x70,0x00,0x00,0x0E,
        0x78,0x00,0x00,0x1E, 0x78,0x00,0x00,0x1E, 0x3C,0x00,0x00,0x3C, 0x3C,0x00,0x00,0x3C,
        0x1E,0x00,0x00,0x78, 0x1F,0x00,0x00,0xF8, 0x0F,0xC0,0x03,0xF0, 0x07,0xFF,0xFF,0xE0,
        0x03,0xFF,0xFF,0xC0, 0x01,0xFF,0xFF,0x80, 0x00,0x7F,0xFE,0x00, 0x00,0x00,0x00,0x00,
    };
    ra8876_bte_expand(&display, 0, 500, 510, 32, 32, icon_32x32, RA8876_CYAN, RA8876_BLACK);
    ra8876_bte_expand(&display, 0, 540, 510, 32, 32, icon_32x32, RA8876_RED, RA8876_BLACK);
    ra8876_bte_expand(&display, 0, 580, 510, 32, 32, icon_32x32, RA8876_GREEN, RA8876_DARKGRAY);
    ra8876_bte_expand(&display, 0, 620, 510, 32, 32, icon_32x32, RA8876_YELLOW, RA8876_BLUE);

    ra8876_printf(&display, 700, 550, RA8876_WHITE, "%dx%d @ %dbpp",
        RA8876_WIDTH, RA8876_HEIGHT, RA8876_BPP);

    sleep_ms(5000);
}

void demo2_bounce(void) {
    printf("Demo 2: Bouncing Rectangle (Double Buffered)\n");

    ra8876_buffer_init(&display, 2);

    int x = 100, y = 100;
    int dx = 5, dy = 3;
    int w = 120, h = 80;

    uint8_t r = 255, g = 0, b = 0;
    int color_state = 0;

    uint32_t frame_count = 0;
    uint32_t last_fps_time = time_us_32();
    uint32_t fps = 0;

    uint16_t bg_color = ra8876_rgb565(0, 0, 64);

    for (int i = 0; i < 500; i++) {
        uint32_t frame_start = time_us_32();
        uint8_t page = ra8876_get_draw_page(&display);

        ra8876_page_fill_screen(&display, page, bg_color);

        x += dx;
        y += dy;

        if (x <= 0 || x + w >= RA8876_WIDTH) {
            dx = -dx;
            x += dx;
        }
        if (y <= 0 || y + h >= RA8876_HEIGHT - 30) {
            dy = -dy;
            y += dy;
        }

        switch (color_state) {
            case 0: g += 5; if (g >= 255) { g = 255; color_state = 1; } break;
            case 1: r -= 5; if (r <= 0) { r = 0; color_state = 2; } break;
            case 2: b += 5; if (b >= 255) { b = 255; color_state = 3; } break;
            case 3: g -= 5; if (g <= 0) { g = 0; color_state = 4; } break;
            case 4: r += 5; if (r >= 255) { r = 255; color_state = 5; } break;
            case 5: b -= 5; if (b <= 0) { b = 0; color_state = 0; } break;
        }
        uint16_t color = ra8876_rgb565(r, g, b);

        ra8876_page_fill_rect(&display, page, x, y, w, h, color);
        ra8876_page_printf(&display, page, 10, RA8876_HEIGHT - 20, RA8876_WHITE, "FPS: %lu", fps);

        ra8876_swap_buffers(&display);

        frame_count++;
        uint32_t now = time_us_32();
        if (now - last_fps_time >= 1000000) {
            fps = frame_count;
            frame_count = 0;
            last_fps_time = now;
            printf("FPS: %lu\n", fps);
        }

        uint32_t frame_time = time_us_32() - frame_start;
        if (frame_time < 16000) {
            sleep_us(16000 - frame_time);
        }
    }

    ra8876_buffer_disable(&display);

    printf("Bounce demo complete\n");
}

void demo3_text(void) {
    printf("Demo 3: Text Editor\n");

    ra8876_fill_screen(&display, RA8876_BLUE);

    ra8876_fill_rect(&display, 0, 0, RA8876_WIDTH, 31, RA8876_DARKGRAY);
    ra8876_print(&display, 10, 8, RA8876_WHITE, "Untitled.txt - Text Editor");

    ra8876_fill_rect(&display, 0, 31, RA8876_WIDTH, RA8876_HEIGHT - 61, RA8876_BLACK);

    ra8876_fill_rect(&display, 0, RA8876_HEIGHT - 30, RA8876_WIDTH, 30, RA8876_DARKGRAY);

    ra8876_set_text_colors(&display, RA8876_GREEN, RA8876_BLACK);

    const char *lines[] = {
        "// RA8876 Text Demo",
        "// This demonstrates the text capabilities",
        "",
        "#include <stdio.h>",
        "#include \"ra8876.h\"",
        "",
        "int main() {",
        "    ra8876_init();",
        "    ra8876_print(0, 0, WHITE, \"Hello World!\");",
        "    ",
        "    while (1) {",
        "        // Your code here",
        "    }",
        "    return 0;",
        "}",
        "",
        "// Use streaming API for fast text output:",
        "// ra8876_begin_text_stream();",
        "// ra8876_stream_char('X');",
        "// ra8876_end_text_stream();",
    };

    int num_lines = sizeof(lines) / sizeof(lines[0]);
    int cursor_line = 9;
    int cursor_col = 4;

    for (int i = 0; i < num_lines && i < 30; i++) {
        ra8876_set_text_cursor(&display, 8, 40 + i * 18);
        ra8876_put_string(&display, lines[i]);
    }

    uint16_t cursor_x = 8 + cursor_col * 8;
    uint16_t cursor_y = 40 + cursor_line * 18;
    bool cursor_visible = true;

    uint32_t frame_count = 0;
    uint32_t last_fps_time = time_us_32();
    uint32_t fps = 0;
    uint32_t last_blink = time_us_32();

    for (int i = 0; i < 300; i++) {
        uint32_t now = time_us_32();

        if (now - last_blink >= 500000) {
            cursor_visible = !cursor_visible;
            last_blink = now;

            if (cursor_visible) {
                ra8876_fill_rect(&display, cursor_x, cursor_y, 3, 17, RA8876_WHITE);
            } else {
                ra8876_fill_rect(&display, cursor_x, cursor_y, 3, 17, RA8876_BLACK);
            }
        }

        frame_count++;
        if (now - last_fps_time >= 1000000) {
            fps = frame_count;
            frame_count = 0;
            last_fps_time = now;
            printf("FPS: %lu (cursor blink demo)\n", fps);

            ra8876_fill_rect(&display, 0, RA8876_HEIGHT - 30, RA8876_WIDTH, 30, RA8876_DARKGRAY);
            ra8876_printf(&display, 10, RA8876_HEIGHT - 22, RA8876_WHITE,
                "Line %d, Col %d | FPS: %lu | 128x37 chars",
                cursor_line + 1, cursor_col + 1, fps);
        }

        sleep_ms(16);
    }

    printf("Text demo complete\n");
}

void demo4_bte(void) {
    printf("Demo 4: BTE Engine\n");

    ra8876_buffer_init(&display, 2);

    ra8876_page_fill_screen(&display, 2, RA8876_MAGENTA);
    ra8876_page_fill_rect(&display, 2, 10, 10, 81, 81, RA8876_RED);
    ra8876_page_fill_rect(&display, 2, 30, 30, 41, 41, RA8876_YELLOW);
    ra8876_page_fill_circle(&display, 2, 50, 50, 20, RA8876_BLUE);

    ra8876_page_fill_screen(&display, 3, RA8876_BLACK);
    for (int i = 0; i < 10; i++) {
        ra8876_page_fill_rect(&display, 3, i * 100, 0, 51, 600, ra8876_rgb565(i * 25, 50, 100));
    }

    int sprite_x = 100, sprite_y = 100;
    int dx = 4, dy = 3;
    uint32_t frame_count = 0;
    uint32_t last_fps_time = time_us_32();
    uint32_t fps = 0;

    for (int i = 0; i < 500; i++) {
        uint32_t frame_start = time_us_32();
        uint8_t page = ra8876_get_draw_page(&display);

        ra8876_bte_copy(&display, 3, 0, 0, page, 0, 0, RA8876_WIDTH, RA8876_HEIGHT, RA8876_ROP_S);
        ra8876_bte_copy_chroma(&display, 2, 0, 0, page, sprite_x, sprite_y, 100, 100, RA8876_MAGENTA);
        ra8876_bte_copy_chroma(&display, 2, 0, 0, page, sprite_x + 150, sprite_y, 100, 100, RA8876_MAGENTA);
        ra8876_bte_copy_chroma(&display, 2, 0, 0, page, sprite_x + 300, sprite_y, 100, 100, RA8876_MAGENTA);

        ra8876_page_printf(&display, page, 10, 10, RA8876_WHITE, "BTE Demo - FPS: %lu", fps);
        ra8876_page_printf(&display, page, 10, 30, RA8876_CYAN, "Sprites with chroma key transparency");

        ra8876_swap_buffers(&display);

        sprite_x += dx;
        sprite_y += dy;
        if (sprite_x <= 0 || sprite_x + 400 >= RA8876_WIDTH) dx = -dx;
        if (sprite_y <= 50 || sprite_y + 100 >= RA8876_HEIGHT) dy = -dy;

        frame_count++;
        uint32_t now = time_us_32();
        if (now - last_fps_time >= 1000000) {
            fps = frame_count;
            frame_count = 0;
            last_fps_time = now;
            printf("BTE FPS: %lu\n", fps);
        }

        uint32_t frame_time = time_us_32() - frame_start;
        if (frame_time < 16000) sleep_us(16000 - frame_time);
    }

    ra8876_buffer_disable(&display);
    printf("BTE demo complete\n");
}

void demo5_cursor(void) {
    printf("Demo 5: Hardware Cursor\n");

    ra8876_fill_screen(&display, RA8876_BLACK);
    ra8876_fill_rect(&display, 0, 0, RA8876_WIDTH, 31, RA8876_DARKGRAY);
    ra8876_print(&display, 10, 8, RA8876_WHITE, "Hardware Cursor Demo - Zero CPU Blink");

    ra8876_fill_rect(&display, 20, 50, RA8876_WIDTH - 40, RA8876_HEIGHT - 100, ra8876_rgb565(0, 0, 40));

    ra8876_set_text_colors(&display, RA8876_GREEN, ra8876_rgb565(0, 0, 40));

    const char *text[] = {
        "The RA8876 has a hardware text cursor.",
        "It blinks automatically with ZERO CPU overhead.",
        "",
        "No SPI traffic needed for cursor animation!",
        "Perfect for text editors and terminals.",
        "",
        "void main() {",
        "    printf(\"Hello World!\");",
        "    return 0;",
        "}",
    };

    for (int i = 0; i < 10; i++) {
        ra8876_set_text_cursor(&display, 30, 60 + i * 20);
        ra8876_put_string(&display, text[i]);
    }

    ra8876_set_text_cursor(&display, 30, 60);
    ra8876_cursor_size(&display, 8, 16);
    ra8876_cursor_blink_rate(&display, 30);
    ra8876_cursor_show(&display, true);

    ra8876_print(&display, 30, 400, RA8876_YELLOW, "Cursor blinks with no CPU involvement!");
    ra8876_print(&display, 30, 430, RA8876_CYAN, "Try ra8876_invert_area() for text selection:");
    ra8876_set_text_cursor(&display, 30, 60);

    sleep_ms(2000);

    ra8876_invert_area(&display, 30, 60, 8 * 38, 20);
    sleep_ms(1000);
    ra8876_invert_area(&display, 30, 80, 8 * 45, 20);

    sleep_ms(5000);

    ra8876_cursor_hide(&display);
    ra8876_invert_area(&display, 30, 60, 8 * 38, 20);
    ra8876_invert_area(&display, 30, 80, 8 * 45, 20);

    printf("Cursor demo complete\n");
}

void demo6_pip(void) {
    printf("Demo 6: PIP Hardware HUD\n");

    ra8876_page_fill_screen(&display, 0, ra8876_rgb565(0, 0, 80));
    for (int i = 0; i < 20; i++) {
        ra8876_page_draw_line(&display, 0, 0, i * 30, RA8876_WIDTH, i * 30, RA8876_DARKGRAY);
        ra8876_page_draw_line(&display, 0, i * 60, 0, i * 60, RA8876_HEIGHT, RA8876_DARKGRAY);
    }
    ra8876_page_print(&display, 0, 300, 280, RA8876_WHITE, "Main Display - Background Grid");
    ra8876_page_print(&display, 0, 250, 320, RA8876_GRAY, "PIP windows float above without redrawing this");

    ra8876_page_fill_rect(&display, 2, 0, 0, 200, 100, ra8876_rgb565(40, 0, 0));
    ra8876_page_draw_rect(&display, 2, 0, 0, 200, 100, RA8876_RED);
    ra8876_page_print(&display, 2, 10, 10, RA8876_WHITE, "HUD Window 1");
    ra8876_page_print(&display, 2, 10, 35, RA8876_YELLOW, "HP: 100/100");
    ra8876_page_print(&display, 2, 10, 55, RA8876_CYAN, "MP: 50/50");
    ra8876_page_print(&display, 2, 10, 75, RA8876_GREEN, "Gold: 1234");

    ra8876_page_fill_rect(&display, 3, 0, 0, 152, 62, ra8876_rgb565(0, 40, 0));
    ra8876_page_draw_rect(&display, 3, 0, 0, 152, 62, RA8876_GREEN);
    ra8876_page_print(&display, 3, 10, 10, RA8876_WHITE, "Minimap");
    ra8876_page_fill_circle(&display, 3, 75, 35, 5, RA8876_RED);
    ra8876_page_fill_rect(&display, 3, 50, 25, 11, 11, RA8876_BLUE);
    ra8876_page_fill_rect(&display, 3, 100, 40, 21, 11, RA8876_YELLOW);

    ra8876_pip1_enable(&display, 2, 20, 20, 200, 100);
    ra8876_pip2_enable(&display, 3, RA8876_WIDTH - 170, 20, 152, 62);

    ra8876_page_print(&display, 0, 300, 550, RA8876_WHITE, "PIP windows move with ZERO background redraw!");

    int pip1_x = 20, pip1_y = 20;
    int pip1_dx = 3, pip1_dy = 2;
    int pip2_x = RA8876_WIDTH - 170, pip2_y = 20;
    int pip2_dx = -2, pip2_dy = 3;

    for (int i = 0; i < 500; i++) {
        pip1_x += pip1_dx;
        pip1_y += pip1_dy;
        if (pip1_x <= 0 || pip1_x + 200 >= RA8876_WIDTH) pip1_dx = -pip1_dx;
        if (pip1_y <= 0 || pip1_y + 100 >= RA8876_HEIGHT - 50) pip1_dy = -pip1_dy;
        ra8876_pip1_move(&display, pip1_x, pip1_y);

        pip2_x += pip2_dx;
        pip2_y += pip2_dy;
        if (pip2_x <= 0 || pip2_x + 152 >= RA8876_WIDTH) pip2_dx = -pip2_dx;
        if (pip2_y <= 0 || pip2_y + 62 >= RA8876_HEIGHT - 50) pip2_dy = -pip2_dy;
        ra8876_pip2_move(&display, pip2_x, pip2_y);

        sleep_ms(16);
    }

    ra8876_pip1_disable(&display);
    ra8876_pip2_disable(&display);

    printf("PIP demo complete\n");
}

static const uint8_t oldschool_font_8x16[95 * 16] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x18,0x3C,0x3C,0x3C,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00,
    0x00,0x66,0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x6C,0x6C,0xFE,0x6C,0x6C,0x6C,0xFE,0x6C,0x6C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x18,0x7C,0xC6,0xC0,0x78,0x3C,0x06,0xC6,0x7C,0x18,0x18,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0xC6,0xCC,0x18,0x30,0x60,0xCC,0xC6,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x38,0x6C,0x6C,0x38,0x76,0xDC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,0x00,
    0x00,0x18,0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x0C,0x18,0x30,0x30,0x30,0x30,0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x30,0x18,0x0C,0x0C,0x0C,0x0C,0x0C,0x18,0x30,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x30,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x02,0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7C,0xC6,0xCE,0xDE,0xF6,0xE6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7C,0xC6,0x06,0x0C,0x18,0x30,0x60,0xC0,0xFE,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7C,0xC6,0x06,0x06,0x3C,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xFE,0xC0,0xC0,0xFC,0x06,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x38,0x60,0xC0,0xC0,0xFC,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xFE,0xC6,0x06,0x0C,0x18,0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7E,0x06,0x06,0x0C,0x78,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x06,0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x06,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x60,0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x60,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7C,0xC6,0xC6,0x0C,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7C,0xC6,0xC6,0xDE,0xDE,0xDE,0xDC,0xC0,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x66,0x66,0x66,0xFC,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xC0,0xC2,0x66,0x3C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xF8,0x6C,0x66,0x66,0x66,0x66,0x66,0x6C,0xF8,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x62,0x66,0xFE,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xDE,0xC6,0x66,0x3A,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xE6,0x66,0x6C,0x6C,0x78,0x6C,0x6C,0x66,0xE6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x38,0x6C,0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x0C,0x0E,0x00,0x00,0x00,0x00,
    0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x6C,0x66,0x66,0xE6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7C,0xC6,0xC6,0x60,0x38,0x0C,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7E,0x7E,0x5A,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xD6,0xD6,0xFE,0x6C,0x6C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xC6,0xC6,0x6C,0x38,0x38,0x38,0x6C,0xC6,0xC6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x66,0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xFE,0xC6,0x8C,0x18,0x30,0x60,0xC2,0xC6,0xFE,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x80,0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,0x00,0x00,0x00,0x00,
    0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
    0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x78,0x0C,0x7C,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0xE0,0x60,0x60,0x78,0x6C,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x1C,0x0C,0x0C,0x3C,0x6C,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xFE,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x38,0x6C,0x64,0x60,0xF0,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,0x00,0x00,0x00,
    0x00,0x00,0xE0,0x60,0x60,0x6C,0x76,0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x18,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x06,0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00,0x00,0x00,
    0x00,0x00,0xE0,0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xEC,0xFE,0xD6,0xD6,0xD6,0xC6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x1E,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xDC,0x76,0x66,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0x70,0x1C,0xC6,0x7C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x10,0x30,0x30,0xFC,0x30,0x30,0x30,0x34,0x18,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xD6,0xD6,0xFE,0x6C,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xC6,0x6C,0x38,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0x7E,0x06,0x0C,0xF8,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xFE,0xCC,0x18,0x30,0x66,0xFE,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x0E,0x18,0x18,0x18,0x70,0x18,0x18,0x18,0x0E,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x18,0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x70,0x18,0x18,0x18,0x0E,0x18,0x18,0x18,0x70,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

void demo7_cgram(void) {
    printf("Demo 7: Custom CGRAM Font\n");

    ra8876_cgram_init(&display);
    ra8876_cgram_upload_font(&display, oldschool_font_8x16, ' ', 95, 16);

    ra8876_select_cgram_font(&display, RA8876_FONT_16);

    ra8876_fill_screen(&display, ra8876_rgb565(0, 0, 32));
    ra8876_set_text_colors(&display, RA8876_GREEN, ra8876_rgb565(0, 0, 32));
    ra8876_set_text_cursor(&display, 0, 0);
    ra8876_put_cgram_string(&display, "OLDSCHOOL FONT DEMO - ASCII 32-126");

    ra8876_set_text_cursor(&display, 0, 20);
    ra8876_put_cgram_string(&display, "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");

    ra8876_set_text_cursor(&display, 0, 40);
    ra8876_put_cgram_string(&display, "the quick brown fox jumps over the lazy dog");

    ra8876_set_text_cursor(&display, 0, 60);
    ra8876_put_cgram_string(&display, "0123456789 !@#$%^&*() []{}|;':\",./<>?");

    ra8876_set_text_colors(&display, RA8876_CYAN, ra8876_rgb565(0, 0, 32));
    ra8876_set_text_cursor(&display, 0, 100);
    ra8876_put_cgram_string(&display, "C:\\>DIR /W");
    ra8876_set_text_cursor(&display, 0, 120);
    ra8876_put_cgram_string(&display, " Volume in drive C is RETRO_DOS");
    ra8876_set_text_cursor(&display, 0, 140);
    ra8876_put_cgram_string(&display, " Directory of C:\\GAMES");
    ra8876_set_text_cursor(&display, 0, 160);
    ra8876_put_cgram_string(&display, "DOOM     QUAKE    DUKE3D   HEXEN    HERETIC");

    sleep_ms(3000);

    ra8876_buffer_init(&display, 4);

    uint16_t cols = RA8876_WIDTH / 8;
    uint16_t rows = RA8876_HEIGHT / 16;
    uint16_t total_chars = cols * rows;

    char line[129];

    printf("Benchmarking CGRAM: %d cols x %d rows = %d chars\n", cols, rows, total_chars);

    uint32_t frames = 0;
    uint32_t start_time = time_us_32();
    uint32_t fps = 0;
    uint32_t last_fps_time = start_time;

    for (int f = 0; f < 300; f++) {
        uint8_t page = ra8876_get_draw_page(&display);
        ra8876_page_fill_screen(&display, page, RA8876_BLACK);
        ra8876_set_canvas_page(&display, page);
        ra8876_set_text_colors(&display, RA8876_GREEN, RA8876_BLACK);

        for (int row = 0; row < rows; row++) {
            for (int i = 0; i < cols; i++) {
                line[i] = ' ' + ((i + row + f) % 95);
            }
            line[cols] = '\0';

            ra8876_set_text_cursor(&display, 0, row * 16);
            ra8876_put_cgram_string(&display, line);
        }

        ra8876_set_text_colors(&display, RA8876_WHITE, RA8876_BLACK);
        ra8876_select_internal_font(&display, RA8876_FONT_16, RA8876_ENC_8859_1);
        ra8876_page_printf(&display, page, 10, RA8876_HEIGHT - 20, RA8876_WHITE, "FPS: %lu", fps);
        ra8876_select_cgram_font(&display, RA8876_FONT_16);

        ra8876_swap_buffers(&display);

        frames++;
        uint32_t now = time_us_32();
        if (now - last_fps_time >= 1000000) {
            fps = frames;
            frames = 0;
            last_fps_time = now;
            printf("CGRAM FPS: %lu\n", fps);
        }
    }

    uint32_t elapsed = time_us_32() - start_time;
    float avg_fps = 300.0f * 1000000.0f / elapsed;
    float chars_per_sec = (float)total_chars * 300.0f * 1000000.0f / elapsed;

    printf("CGRAM Benchmark: 300 frames in %lu us\n", elapsed);
    printf("Average FPS: %.2f\n", avg_fps);
    printf("Chars/sec: %.0f\n", chars_per_sec);

    ra8876_buffer_disable(&display);

    ra8876_select_internal_font(&display, RA8876_FONT_16, RA8876_ENC_8859_1);
    ra8876_fill_screen(&display, RA8876_BLACK);
    ra8876_printf(&display, 10, 10, RA8876_WHITE, "CGRAM Benchmark Complete");
    ra8876_printf(&display, 10, 40, RA8876_GREEN, "300 frames: %lu us", elapsed);
    ra8876_printf(&display, 10, 70, RA8876_CYAN, "Average FPS: %.2f", avg_fps);
    ra8876_printf(&display, 10, 100, RA8876_YELLOW, "Chars/sec: %.0f", chars_per_sec);
    ra8876_printf(&display, 10, 130, RA8876_MAGENTA, "Screen: %dx%d = %d chars", cols, rows, total_chars);

    sleep_ms(5000);
}

void demo8_internal_font_bench(void) {
    printf("Demo 8: Internal Font Benchmark\n");

    ra8876_buffer_init(&display, 2);

    uint16_t cols = RA8876_WIDTH / 8;
    uint16_t rows = RA8876_HEIGHT / 16;
    uint16_t total_chars = cols * rows;

    char line[129];

    printf("Benchmarking Internal Font: %d cols x %d rows = %d chars\n", cols, rows, total_chars);

    uint32_t frames = 0;
    uint32_t start_time = time_us_32();
    uint32_t fps = 0;
    uint32_t last_fps_time = start_time;

    ra8876_select_internal_font(&display, RA8876_FONT_16, RA8876_ENC_8859_1);

    for (int f = 0; f < 300; f++) {
        uint8_t page = ra8876_get_draw_page(&display);
        ra8876_set_canvas_page(&display, page);
        ra8876_set_text_colors(&display, RA8876_GREEN, RA8876_BLACK);

        for (int row = 0; row < rows; row++) {
            for (int i = 0; i < cols; i++) {
                line[i] = ' ' + ((i + row + f) % 95);
            }
            line[cols] = '\0';

            ra8876_set_text_cursor(&display, 0, row * 16);
            ra8876_put_string(&display, line);
        }

        ra8876_page_printf(&display, page, 10, RA8876_HEIGHT - 20, RA8876_WHITE, "FPS: %lu", fps);

        ra8876_swap_buffers(&display);
        frames++;
        uint32_t now = time_us_32();
        if (now - last_fps_time >= 1000000) {
            fps = frames;
            frames = 0;
            last_fps_time = now;
            printf("Internal Font FPS: %lu\n", fps);
        }
    }

    uint32_t elapsed = time_us_32() - start_time;
    float avg_fps = 300.0f * 1000000.0f / elapsed;
    float chars_per_sec = (float)total_chars * 300.0f * 1000000.0f / elapsed;

    printf("Internal Font Benchmark: 300 frames in %lu us\n", elapsed);
    printf("Average FPS: %.2f\n", avg_fps);
    printf("Chars/sec: %.0f\n", chars_per_sec);

    ra8876_buffer_disable(&display);

    ra8876_fill_screen(&display, RA8876_BLACK);
    ra8876_printf(&display, 10, 10, RA8876_WHITE, "Internal Font Benchmark Complete");
    ra8876_printf(&display, 10, 40, RA8876_GREEN, "300 frames: %lu us", elapsed);
    ra8876_printf(&display, 10, 70, RA8876_CYAN, "Average FPS: %.2f", avg_fps);
    ra8876_printf(&display, 10, 100, RA8876_YELLOW, "Chars/sec: %.0f", chars_per_sec);
    ra8876_printf(&display, 10, 130, RA8876_MAGENTA, "Screen: %dx%d = %d chars", cols, rows, total_chars);

    sleep_ms(5000);
}

void demo9_vsync_test(void) {
    printf("Demo 9: VSYNC Counter Test\n");

    ra8876_fill_screen(&display, RA8876_BLACK);
    ra8876_print(&display, 10, 10, RA8876_WHITE, "VSYNC Counter Test");
    ra8876_print(&display, 10, 40, RA8876_GRAY, "Counting vsync events for 10 seconds...");

    uint32_t vsync_count = 0;
    uint32_t start_time = time_us_32();
    uint32_t last_update = start_time;

    while (time_us_32() - start_time < 10000000) {
        ra8876_wait_vsync(&display);
        vsync_count++;

        uint32_t now = time_us_32();
        if (now - last_update >= 1000000) {
            uint32_t elapsed_sec = (now - start_time) / 1000000;
            float hz = (float)vsync_count / elapsed_sec;
            ra8876_fill_rect(&display, 10, 80, 391, 121, RA8876_BLACK);
            ra8876_printf(&display, 10, 80, RA8876_GREEN, "VSYNC count: %lu", vsync_count);
            ra8876_printf(&display, 10, 110, RA8876_CYAN, "Elapsed: %lu sec", elapsed_sec);
            ra8876_printf(&display, 10, 140, RA8876_YELLOW, "Refresh rate: %.1f Hz", hz);
            printf("VSYNC: %lu total, %.1f Hz\n", vsync_count, hz);
            last_update = now;
        }
    }

    float final_hz = (float)vsync_count / 10.0f;
    ra8876_printf(&display, 10, 200, RA8876_WHITE, "Final: %lu vsyncs = %.2f Hz", vsync_count, final_hz);
    printf("Final: %lu vsyncs in 10 sec = %.2f Hz\n", vsync_count, final_hz);

    sleep_ms(5000);
}

void demo10_cgram_cursor(void) {
    printf("Demo 10: CGRAM + Hardware Cursor + Selection\n");

    ra8876_cgram_init(&display);
    ra8876_cgram_upload_font(&display, oldschool_font_8x16, ' ', 95, 16);

    ra8876_fill_screen(&display, ra8876_rgb565(0, 0, 40));

    ra8876_select_cgram_font(&display, RA8876_FONT_16);
    ra8876_set_text_colors(&display, RA8876_GREEN, ra8876_rgb565(0, 0, 40));

    ra8876_set_text_cursor(&display, 0, 0);
    ra8876_put_cgram_string(&display, "RETRO TERMINAL v1.0");

    ra8876_set_text_cursor(&display, 0, 20);
    ra8876_put_cgram_string(&display, "=====================================");

    const char *lines[] = {
        "C:\\>DIR",
        " Volume in drive C is RETRO_DOS",
        " Directory of C:\\",
        "",
        "COMMAND  COM     47,845  01-01-90  12:00a",
        "CONFIG   SYS        512  01-01-90  12:00a",
        "AUTOEXEC BAT        128  01-01-90  12:00a",
        "GAMES    <DIR>           01-01-90  12:00a",
        "TOOLS    <DIR>           01-01-90  12:00a",
        "",
        "        3 file(s)     48,485 bytes",
        "        2 dir(s)  123,456,789 bytes free",
        "",
        "C:\\>_",
    };

    for (int i = 0; i < 14; i++) {
        ra8876_set_text_cursor(&display, 0, 50 + i * 18);
        ra8876_put_cgram_string(&display, lines[i]);
    }

    ra8876_set_text_cursor(&display, 32, 50 + 13 * 18);
    ra8876_cursor_size(&display, 8, 16);
    ra8876_cursor_show(&display, true);

    ra8876_set_text_colors(&display, RA8876_YELLOW, ra8876_rgb565(0, 0, 40));
    ra8876_set_text_cursor(&display, 0, 350);
    ra8876_put_cgram_string(&display, "Hardware cursor blinking - zero CPU!");

    sleep_ms(3000);

    ra8876_set_text_colors(&display, RA8876_CYAN, ra8876_rgb565(0, 0, 40));
    ra8876_set_text_cursor(&display, 0, 380);
    ra8876_put_cgram_string(&display, "Selecting text with invert_area()...");

    sleep_ms(1000);
    ra8876_invert_area(&display, 0, 50 + 4 * 18, 8 * 45, 18);
    sleep_ms(500);
    ra8876_invert_area(&display, 0, 50 + 5 * 18, 8 * 45, 18);
    sleep_ms(500);
    ra8876_invert_area(&display, 0, 50 + 6 * 18, 8 * 45, 18);

    ra8876_set_text_colors(&display, RA8876_WHITE, ra8876_rgb565(0, 0, 40));
    ra8876_set_text_cursor(&display, 0, 420);
    ra8876_put_cgram_string(&display, "3 files selected!");

    sleep_ms(3000);

    ra8876_invert_area(&display, 0, 50 + 4 * 18, 8 * 45, 18);
    ra8876_invert_area(&display, 0, 50 + 5 * 18, 8 * 45, 18);
    ra8876_invert_area(&display, 0, 50 + 6 * 18, 8 * 45, 18);

    ra8876_cursor_hide(&display);

    ra8876_set_text_cursor(&display, 0, 460);
    ra8876_put_cgram_string(&display, "Moving cursor across the screen:");

    ra8876_set_text_cursor(&display, 0, 500);
    ra8876_put_cgram_string(&display, "..................................................");

    for (int i = 0; i < 50; i++) {
        ra8876_set_text_cursor(&display, i * 8, 500);
        ra8876_cursor_show(&display, true);
        sleep_ms(50);
    }

    ra8876_cursor_hide(&display);

    ra8876_set_text_cursor(&display, 0, 540);
    ra8876_put_cgram_string(&display, "Cursor on text: HELLO_WORLD");

    ra8876_set_text_cursor(&display, 8 * 21, 540);
    ra8876_cursor_show(&display, true);

    sleep_ms(10000);

    ra8876_cursor_hide(&display);
    ra8876_select_internal_font(&display, RA8876_FONT_16, RA8876_ENC_8859_1);
}

#define LIFE_CELL_SIZE 8
#define LIFE_COLS (RA8876_WIDTH / LIFE_CELL_SIZE)
#define LIFE_ROWS ((RA8876_HEIGHT - 40) / LIFE_CELL_SIZE)

static uint8_t life_grid[LIFE_ROWS][LIFE_COLS];
static uint8_t life_next[LIFE_ROWS][LIFE_COLS];

static void life_init_random(void) {
    for (int y = 0; y < LIFE_ROWS; y++) {
        for (int x = 0; x < LIFE_COLS; x++) {
            life_grid[y][x] = (rand() % 100) < 25 ? 1 : 0;
        }
    }
}

static void life_init_glider_gun(void) {
    for (int y = 0; y < LIFE_ROWS; y++)
        for (int x = 0; x < LIFE_COLS; x++)
            life_grid[y][x] = 0;

    int ox = 10, oy = 10;
    int gun[][2] = {
        {0,4},{0,5},{1,4},{1,5},
        {10,4},{10,5},{10,6},{11,3},{11,7},{12,2},{12,8},{13,2},{13,8},
        {14,5},{15,3},{15,7},{16,4},{16,5},{16,6},{17,5},
        {20,2},{20,3},{20,4},{21,2},{21,3},{21,4},{22,1},{22,5},
        {24,0},{24,1},{24,5},{24,6},
        {34,2},{34,3},{35,2},{35,3}
    };
    int n = sizeof(gun) / sizeof(gun[0]);
    for (int i = 0; i < n; i++) {
        int x = ox + gun[i][0];
        int y = oy + gun[i][1];
        if (x < LIFE_COLS && y < LIFE_ROWS)
            life_grid[y][x] = 1;
    }
}

static int life_count_neighbors(int x, int y) {
    int xm1 = (x == 0) ? LIFE_COLS - 1 : x - 1;
    int xp1 = (x == LIFE_COLS - 1) ? 0 : x + 1;
    int ym1 = (y == 0) ? LIFE_ROWS - 1 : y - 1;
    int yp1 = (y == LIFE_ROWS - 1) ? 0 : y + 1;

    return life_grid[ym1][xm1] + life_grid[ym1][x] + life_grid[ym1][xp1] +
           life_grid[y][xm1] + life_grid[y][xp1] +
           life_grid[yp1][xm1] + life_grid[yp1][x] + life_grid[yp1][xp1];
}

static void life_step(void) {
    for (int y = 0; y < LIFE_ROWS; y++) {
        for (int x = 0; x < LIFE_COLS; x++) {
            int n = life_count_neighbors(x, y);
            if (life_grid[y][x]) {
                life_next[y][x] = (n == 2 || n == 3) ? 1 : 0;
            } else {
                life_next[y][x] = (n == 3) ? 1 : 0;
            }
        }
    }
    for (int y = 0; y < LIFE_ROWS; y++)
        for (int x = 0; x < LIFE_COLS; x++)
            life_grid[y][x] = life_next[y][x];
}

static void life_draw(uint8_t page) {
    ra8876_bte_solid_fill(&display, page, 0, 40, RA8876_WIDTH, RA8876_HEIGHT - 40, RA8876_BLACK);

    ra8876_bte_batch_start(&display, page, LIFE_CELL_SIZE - 1, LIFE_CELL_SIZE - 1);
    for (int y = 0; y < LIFE_ROWS; y++) {
        for (int x = 0; x < LIFE_COLS; x++) {
            if (life_grid[y][x]) {
                uint16_t px = x * LIFE_CELL_SIZE;
                uint16_t py = 40 + y * LIFE_CELL_SIZE;
                ra8876_bte_batch_fill(&display, px, py, RA8876_GREEN);
            }
        }
    }
    ra8876_wait_task_busy(&display);
}

void demo11_game_of_life(void) {
    printf("Demo 11: Conway's Game of Life\n");

    srand(time_us_32());

    life_init_glider_gun();

    ra8876_buffer_init(&display, 2);

    uint32_t gen = 0;
    uint32_t frames = 0;
    uint32_t fps = 0;
    uint32_t last_fps_time = time_us_32();

    for (int i = 0; i < 1000; i++) {
        uint8_t page = ra8876_get_draw_page(&display);

        ra8876_bte_solid_fill(&display, page, 0, 0, RA8876_WIDTH, 40, ra8876_rgb565(0, 0, 60));

        life_draw(page);

        ra8876_select_internal_font(&display, RA8876_FONT_16, RA8876_ENC_8859_1);
        ra8876_page_printf(&display, page, 10, 10, RA8876_WHITE, "Game of Life  Gen: %lu  FPS: %lu  Grid: %dx%d", gen, fps, LIFE_COLS, LIFE_ROWS);

        ra8876_swap_buffers(&display);

        life_step();
        gen++;

        frames++;
        uint32_t now = time_us_32();
        if (now - last_fps_time >= 1000000) {
            fps = frames;
            frames = 0;
            last_fps_time = now;
            printf("Life FPS: %lu, Gen: %lu\n", fps, gen);
        }

        if (gen == 200) {
            life_init_random();
            gen = 0;
        }
    }

    ra8876_buffer_disable(&display);
    printf("Game of Life demo complete\n");
}

#define PLAT_GROUND_Y       500
#define PLAT_PLAYER_W       24
#define PLAT_PLAYER_H       32
#define PLAT_GRAVITY        1
#define PLAT_JUMP_VEL       -16
#define PLAT_SCROLL_SPEED   4
#define PLAT_MAX_PLATFORMS  12

typedef struct {
    int16_t x, y;
    int16_t w, h;
    uint16_t color;
} platform_t;

static struct {
    int16_t x, y;
    int16_t vy;
    bool on_ground;
    uint32_t score;
} player;

static platform_t platforms[PLAT_MAX_PLATFORMS];
static int16_t scroll_x;
static int16_t bg_scroll;

static void plat_init(void) {
    player.x = 100;
    player.y = PLAT_GROUND_Y - PLAT_PLAYER_H;
    player.vy = 0;
    player.on_ground = true;
    player.score = 0;
    scroll_x = 0;
    bg_scroll = 0;

    for (int i = 0; i < PLAT_MAX_PLATFORMS; i++) {
        platforms[i].x = 300 + i * 150 + (rand() % 80);
        platforms[i].y = 350 + (rand() % 120);
        platforms[i].w = 80 + (rand() % 60);
        platforms[i].h = 16;
        platforms[i].color = ra8876_rgb565(50 + (rand() % 50), 150 + (rand() % 100), 50);
    }
}

static void plat_spawn_platform(int idx) {
    int max_x = 0;
    for (int i = 0; i < PLAT_MAX_PLATFORMS; i++) {
        if (platforms[i].x > max_x) max_x = platforms[i].x;
    }
    platforms[idx].x = max_x + 120 + (rand() % 100);
    platforms[idx].y = 300 + (rand() % 180);
    platforms[idx].w = 60 + (rand() % 80);
    platforms[idx].h = 16;
    platforms[idx].color = ra8876_rgb565(50 + (rand() % 50), 150 + (rand() % 100), 50);
}

static bool plat_on_platform(void) {
    int16_t foot_y = player.y + PLAT_PLAYER_H;

    if (foot_y >= PLAT_GROUND_Y) return true;

    for (int i = 0; i < PLAT_MAX_PLATFORMS; i++) {
        int16_t plat_screen_x = platforms[i].x - scroll_x;
        if (player.x + PLAT_PLAYER_W > plat_screen_x &&
            player.x < plat_screen_x + platforms[i].w &&
            foot_y >= platforms[i].y && foot_y <= platforms[i].y + 8 &&
            player.vy >= 0) {
            return true;
        }
    }
    return false;
}

static int16_t plat_get_ground_y(void) {
    int16_t ground = PLAT_GROUND_Y;

    for (int i = 0; i < PLAT_MAX_PLATFORMS; i++) {
        int16_t plat_screen_x = platforms[i].x - scroll_x;
        if (player.x + PLAT_PLAYER_W > plat_screen_x &&
            player.x < plat_screen_x + platforms[i].w &&
            platforms[i].y < ground &&
            player.y + PLAT_PLAYER_H <= platforms[i].y + 8) {
            ground = platforms[i].y;
        }
    }
    return ground;
}

static bool plat_should_jump(void) {
    if (!player.on_ground) return false;

    for (int i = 0; i < PLAT_MAX_PLATFORMS; i++) {
        int16_t plat_screen_x = platforms[i].x - scroll_x;
        int16_t dist = plat_screen_x - player.x;

        if (dist > 0 && dist < 120 && platforms[i].y < player.y) {
            return true;
        }
    }

    bool gap_ahead = true;
    for (int i = 0; i < PLAT_MAX_PLATFORMS; i++) {
        int16_t plat_screen_x = platforms[i].x - scroll_x;
        if (plat_screen_x > player.x + PLAT_PLAYER_W &&
            plat_screen_x < player.x + 100 &&
            platforms[i].y > player.y) {
            gap_ahead = false;
            break;
        }
    }

    if (player.y + PLAT_PLAYER_H >= PLAT_GROUND_Y - 10) {
        gap_ahead = false;
    }

    return gap_ahead && (rand() % 60 == 0);
}

static void plat_update(void) {
    scroll_x += PLAT_SCROLL_SPEED;
    bg_scroll += PLAT_SCROLL_SPEED / 2;
    player.score = scroll_x / 10;

    for (int i = 0; i < PLAT_MAX_PLATFORMS; i++) {
        if (platforms[i].x - scroll_x < -platforms[i].w) {
            plat_spawn_platform(i);
        }
    }

    if (plat_should_jump()) {
        player.vy = PLAT_JUMP_VEL;
        player.on_ground = false;
    }

    player.vy += PLAT_GRAVITY;
    player.y += player.vy;

    int16_t ground = plat_get_ground_y();
    if (player.y + PLAT_PLAYER_H >= ground) {
        player.y = ground - PLAT_PLAYER_H;
        player.vy = 0;
        player.on_ground = true;
    } else {
        player.on_ground = plat_on_platform();
    }

    if (player.y > RA8876_HEIGHT) {
        plat_init();
    }
}

static void plat_draw(uint8_t page) {
    ra8876_bte_solid_fill(&display, page, 0, 0, RA8876_WIDTH, RA8876_HEIGHT, ra8876_rgb565(40, 44, 52));

    for (int i = 0; i < 6; i++) {
        int16_t mx = ((i * 200) - (bg_scroll / 3) % 200 + 1200) % 1200 - 100;
        if (mx >= 0 && mx + 60 < RA8876_WIDTH)
            ra8876_bte_solid_fill(&display, page, mx, 80 + i * 15, 60, 40, ra8876_rgb565(60, 64, 72));
    }

    ra8876_bte_solid_fill(&display, page, 0, PLAT_GROUND_Y, RA8876_WIDTH, RA8876_HEIGHT - PLAT_GROUND_Y, ra8876_rgb565(60, 40, 30));
    ra8876_bte_solid_fill(&display, page, 0, PLAT_GROUND_Y, RA8876_WIDTH, 5, ra8876_rgb565(80, 60, 40));

    for (int i = 0; i < PLAT_MAX_PLATFORMS; i++) {
        int16_t px = platforms[i].x - scroll_x;
        if (px > -platforms[i].w && px < RA8876_WIDTH) {
            int16_t draw_x = (px < 0) ? 0 : px;
            int16_t draw_w = platforms[i].w - ((px < 0) ? -px : 0);
            if (draw_x + draw_w > RA8876_WIDTH) draw_w = RA8876_WIDTH - draw_x;
            if (draw_w > 0) {
                ra8876_bte_solid_fill(&display, page, draw_x, platforms[i].y, draw_w, platforms[i].h, platforms[i].color);
                ra8876_bte_solid_fill(&display, page, draw_x, platforms[i].y, draw_w, 4, ra8876_rgb565(100, 200, 100));
            }
        }
    }

    uint16_t body_color = ra8876_rgb565(220, 120, 100);
    uint16_t head_color = ra8876_rgb565(255, 200, 180);

    ra8876_bte_solid_fill(&display, page, player.x, player.y + 10, PLAT_PLAYER_W, PLAT_PLAYER_H - 10, body_color);
    ra8876_bte_solid_fill(&display, page, player.x + 4, player.y, 16, 14, head_color);

    int leg_offset = (scroll_x / 4) % 8;
    if (!player.on_ground) leg_offset = 4;
    ra8876_bte_solid_fill(&display, page, player.x + 4, player.y + PLAT_PLAYER_H, 4, leg_offset, body_color);
    ra8876_bte_solid_fill(&display, page, player.x + PLAT_PLAYER_W - 8, player.y + PLAT_PLAYER_H, 4, 8 - leg_offset, body_color);
}

void demo12_platformer(void) {
    printf("Demo 12: Auto-Playing Platformer\n");

    srand(time_us_32());
    plat_init();

    ra8876_buffer_init(&display, 5);

    uint32_t frames = 0;
    uint32_t fps = 0;
    uint32_t last_fps_time = time_us_32();

    for (int i = 0; i < 3000; i++) {
        uint8_t page = ra8876_get_draw_page(&display);

        plat_update();
        plat_draw(page);

        ra8876_select_internal_font(&display, RA8876_FONT_16, RA8876_ENC_8859_1);
        ra8876_page_printf(&display, page, 10, 10, RA8876_WHITE, "SCORE: %lu", player.score);
        ra8876_page_printf(&display, page, 900, 10, RA8876_WHITE, "FPS: %lu", fps);

        ra8876_wait_task_busy(&display);
        ra8876_swap_buffers(&display);

        frames++;
        uint32_t now = time_us_32();
        if (now - last_fps_time >= 1000000) {
            fps = frames;
            frames = 0;
            last_fps_time = now;
        }
    }

    ra8876_buffer_disable(&display);
    printf("Platformer demo complete\n");
}

void demo13_blend_write_pip(void) {
    printf("Demo 13: BTE Write, Blend & Dual PIP\n");

    uint16_t *pixels = malloc(128 * 128 * sizeof(uint16_t));
    if (!pixels) {
        printf("Failed to allocate pixel buffer\n");
        return;
    }

    for (int y = 0; y < 128; y++) {
        for (int x = 0; x < 128; x++) {
            int cx = x - 64, cy = y - 64;
            int dist = (cx * cx + cy * cy) / 32;
            uint8_t r = (x * 2) & 0xFF;
            uint8_t g = (y * 2) & 0xFF;
            uint8_t b = (255 - dist) & 0xFF;
            pixels[y * 128 + x] = ra8876_rgb565(r, g, b);
        }
    }

    ra8876_page_fill_screen(&display, 2, ra8876_rgb565(20, 20, 60));
    for (int i = 0; i < 12; i++) {
        uint16_t color = ra8876_rgb565(100 + i * 10, 50, 150 - i * 10);
        ra8876_page_fill_circle(&display, 2, 100 + i * 80, 300, 60 - i * 3, color);
    }
    ra8876_page_print(&display, 2, 350, 50, RA8876_WHITE, "Layer A: Circles");

    ra8876_page_fill_screen(&display, 3, ra8876_rgb565(60, 20, 20));
    for (int i = 0; i < 8; i++) {
        uint16_t color = ra8876_rgb565(200, 100 + i * 15, 50);
        ra8876_page_fill_rect(&display, 3, 50 + i * 120, 100, 51, 401, color);
    }
    ra8876_page_print(&display, 3, 350, 50, RA8876_WHITE, "Layer B: Bars");

    ra8876_page_fill_screen(&display, 4, RA8876_BLACK);
    ra8876_bte_write(&display, 4, 0, 0, 128, 128, pixels);
    ra8876_page_draw_rect(&display, 4, 0, 0, 128, 128, RA8876_WHITE);

    ra8876_page_fill_rect(&display, 5, 0, 0, 140, 60, ra8876_rgb565(0, 60, 0));
    ra8876_page_draw_rect(&display, 5, 0, 0, 140, 60, RA8876_GREEN);
    ra8876_page_print(&display, 5, 10, 10, RA8876_WHITE, "PIP2 Overlay");
    ra8876_page_print(&display, 5, 10, 35, RA8876_CYAN, "Alpha Blend");

    free(pixels);

    ra8876_page_fill_screen(&display, 0, RA8876_BLACK);

    ra8876_pip1_enable(&display, 4, 100, 200, 128, 128);
    ra8876_pip2_enable(&display, 5, 800, 50, 140, 60);

    int pip1_x = 100, pip1_y = 200;
    int pip1_dx = 4, pip1_dy = 3;
    int pip2_x = 800, pip2_y = 50;
    int pip2_dx = -3, pip2_dy = 2;
    uint8_t alpha = 0;
    int alpha_dir = 4;

    uint32_t frames = 0;
    uint32_t fps = 0;
    uint32_t last_fps_time = time_us_32();

    for (int i = 0; i < 600; i++) {
        ra8876_bte_blend(&display, 2, 0, 0, 3, 0, 0, 0, 0, 0, RA8876_WIDTH, RA8876_HEIGHT, alpha);

        ra8876_page_printf(&display, 0, 10, 10, RA8876_WHITE, "Alpha: %3d/255  FPS: %lu", alpha, fps);
        ra8876_page_print(&display, 0, 10, 570, RA8876_GRAY, "bte_write: texture in PIP1 | bte_blend: layer mixing | pip1+pip2: dual floating windows");

        pip1_x += pip1_dx;
        pip1_y += pip1_dy;
        if (pip1_x <= 0 || pip1_x + 128 >= RA8876_WIDTH) pip1_dx = -pip1_dx;
        if (pip1_y <= 40 || pip1_y + 128 >= RA8876_HEIGHT - 40) pip1_dy = -pip1_dy;
        ra8876_pip1_move(&display, pip1_x, pip1_y);

        pip2_x += pip2_dx;
        pip2_y += pip2_dy;
        if (pip2_x <= 0 || pip2_x + 140 >= RA8876_WIDTH) pip2_dx = -pip2_dx;
        if (pip2_y <= 40 || pip2_y + 60 >= RA8876_HEIGHT - 40) pip2_dy = -pip2_dy;
        ra8876_pip2_move(&display, pip2_x, pip2_y);

        alpha += alpha_dir;
        if (alpha >= 252 || alpha <= 4) alpha_dir = -alpha_dir;

        ra8876_wait_vsync(&display);

        frames++;
        uint32_t now = time_us_32();
        if (now - last_fps_time >= 1000000) {
            fps = frames;
            frames = 0;
            last_fps_time = now;
        }
    }

    ra8876_pip1_disable(&display);
    ra8876_pip2_disable(&display);
    printf("Blend/Write/PIP demo complete\n");
}

void demo14_text_transparency(void) {
    printf("Demo 14: Text Transparency\n");

    ra8876_fill_screen(&display, RA8876_BLACK);

    for (int i = 0; i < 20; i++) {
        uint16_t color = ra8876_rgb565(i * 12, 50, 255 - i * 12);
        ra8876_fill_rect(&display, i * 52, 0, 52, RA8876_HEIGHT, color);
    }

    for (int y = 0; y < RA8876_HEIGHT; y += 40) {
        ra8876_draw_line(&display, 0, y, RA8876_WIDTH, y, RA8876_DARKGRAY);
    }

    ra8876_print(&display, 300, 30, RA8876_WHITE, "Text Transparency Demo");

    ra8876_set_text_transparent(&display, false);
    ra8876_set_text_colors(&display, RA8876_WHITE, RA8876_RED);
    ra8876_set_text_cursor(&display, 50, 100);
    ra8876_put_string(&display, "OPAQUE TEXT - Background color fills character cells");

    ra8876_set_text_cursor(&display, 50, 140);
    ra8876_set_text_colors(&display, RA8876_BLACK, RA8876_YELLOW);
    ra8876_put_string(&display, "This text has a solid yellow background");

    ra8876_set_text_transparent(&display, true);
    ra8876_set_text_colors(&display, RA8876_WHITE, RA8876_BLACK);
    ra8876_set_text_cursor(&display, 50, 220);
    ra8876_put_string(&display, "TRANSPARENT TEXT - Pattern shows through!");

    ra8876_set_text_cursor(&display, 50, 260);
    ra8876_set_text_colors(&display, RA8876_YELLOW, RA8876_BLACK);
    ra8876_put_string(&display, "Great for HUDs, overlays, and floating labels");

    ra8876_set_text_cursor(&display, 50, 300);
    ra8876_set_text_colors(&display, RA8876_CYAN, RA8876_BLACK);
    ra8876_put_string(&display, "The background color is ignored when transparent");

    for (int y = 380; y < 500; y += 30) {
        for (int x = 50; x < 900; x += 150) {
            uint16_t color = ra8876_rgb565(rand() % 256, rand() % 256, rand() % 256);
            ra8876_fill_circle(&display, x + rand() % 50, y + rand() % 20, 10 + rand() % 20, color);
        }
    }

    ra8876_set_text_cursor(&display, 50, 400);
    ra8876_set_text_colors(&display, RA8876_WHITE, RA8876_BLACK);
    ra8876_put_string(&display, "Transparent text over random circles");

    ra8876_set_text_cursor(&display, 50, 440);
    ra8876_set_text_colors(&display, RA8876_GREEN, RA8876_BLACK);
    ra8876_put_string(&display, "Perfect for game UIs and status displays");

    ra8876_set_text_transparent(&display, false);
    ra8876_set_text_colors(&display, RA8876_WHITE, RA8876_DARKGRAY);
    ra8876_set_text_cursor(&display, 50, 520);
    ra8876_put_string(&display, "Back to opaque mode - use ra8876_set_text_transparent(bool)");

    ra8876_select_internal_font(&display, RA8876_FONT_16, RA8876_ENC_8859_1);

    sleep_ms(8000);
}

int main() {
    stdio_init_all();
    sleep_ms(1000);

    printf("\n=== RA8876 Demo Suite ===\n");

    if (!ra8876_init(&display)) {
        printf("RA8876 init failed!\n");
        while (1) sleep_ms(1000);
    }

    printf("Chip ID: 0x%02X\n", ra8876_get_chip_id(&display));

    while (1) {
        demo1_shapes();
        demo2_bounce();
        demo3_text();
        demo4_bte();
        demo5_cursor();
        demo6_pip();
        demo7_cgram();
        demo8_internal_font_bench();
        demo9_vsync_test();
        demo10_cgram_cursor();
        demo11_game_of_life();
        demo12_platformer();
        demo13_blend_write_pip();
        demo14_text_transparency();
    }
}
