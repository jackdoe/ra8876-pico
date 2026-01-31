#ifndef RA8876_H
#define RA8876_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "hardware/spi.h"

#define RA8876_WIDTH        1024
#define RA8876_HEIGHT       600
#define RA8876_BPP          16

#define RA8876_BLACK        0x0000
#define RA8876_WHITE        0xFFFF
#define RA8876_RED          0xF800
#define RA8876_GREEN        0x07E0
#define RA8876_BLUE         0x001F
#define RA8876_YELLOW       0xFFE0
#define RA8876_CYAN         0x07FF
#define RA8876_MAGENTA      0xF81F
#define RA8876_GRAY         0x8410
#define RA8876_DARKGRAY     0x4208
#define RA8876_ORANGE       0xFC00

#define RA8876_FONT_16      0
#define RA8876_FONT_24      1
#define RA8876_FONT_32      2

#define RA8876_ENC_8859_1   0x00
#define RA8876_ENC_8859_2   0x01
#define RA8876_ENC_8859_4   0x02
#define RA8876_ENC_8859_5   0x03

#define RA8876_PAGE_SIZE    (RA8876_WIDTH * RA8876_HEIGHT * 2)
#define RA8876_SDRAM_SIZE   (16 * 1024 * 1024)
#define RA8876_MAX_PAGES    13

#define RA8876_ROP_BLACK       0x00
#define RA8876_ROP_S_AND_D     0x80
#define RA8876_ROP_S           0xC0
#define RA8876_ROP_NOT_S       0x30
#define RA8876_ROP_D           0xA0
#define RA8876_ROP_NOT_D       0x50
#define RA8876_ROP_S_XOR_D     0x60
#define RA8876_ROP_S_OR_D      0xE0
#define RA8876_ROP_WHITE       0xF0

typedef struct {
    spi_inst_t *spi;
    uint8_t pin_miso;
    uint8_t pin_cs;
    uint8_t pin_sck;
    uint8_t pin_mosi;
    uint32_t spi_speed;

    uint16_t char_width;
    uint16_t char_height;
    uint16_t cols;
    uint16_t rows;
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint8_t font_size;
    uint8_t scale_x;
    uint8_t scale_y;
    uint16_t fg_color;
    uint16_t bg_color;
    bool transparent_bg;

    uint32_t canvas_addr;
    uint8_t draw_page;
    uint8_t display_page;
    uint8_t num_pages;

    uint8_t reg03;
    uint8_t reg10;
    uint8_t reg3C;
    uint8_t regCD;

    uint8_t burst_buf[65];
} ra8876_t;

bool ra8876_init(ra8876_t *dev);
uint8_t ra8876_get_chip_id(ra8876_t *dev);

void ra8876_write_cmd(ra8876_t *dev, uint8_t cmd);
void ra8876_write_data(ra8876_t *dev, uint8_t data);
void ra8876_write_data_burst(ra8876_t *dev, const uint8_t *data, size_t len);
uint8_t ra8876_read_data(ra8876_t *dev);
uint8_t ra8876_read_status(ra8876_t *dev);
void ra8876_write_reg(ra8876_t *dev, uint8_t reg, uint8_t val);
uint8_t ra8876_read_reg(ra8876_t *dev, uint8_t reg);
void ra8876_write_reg16(ra8876_t *dev, uint8_t reg, uint16_t val);

void ra8876_wait_ready(ra8876_t *dev);
void ra8876_wait_write_fifo(ra8876_t *dev);
void ra8876_wait_task_busy(ra8876_t *dev);
void ra8876_vsync_init(ra8876_t *dev);
void ra8876_wait_vsync(ra8876_t *dev);

void ra8876_fill_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ra8876_draw_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ra8876_draw_line(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ra8876_fill_circle(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t radius, uint16_t color);
void ra8876_draw_circle(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t radius, uint16_t color);
void ra8876_fill_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint16_t color);
void ra8876_draw_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint16_t color);
void ra8876_fill_rounded_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color);
void ra8876_draw_rounded_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color);
void ra8876_fill_triangle(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ra8876_draw_triangle(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ra8876_fill_screen(ra8876_t *dev, uint16_t color);

void ra8876_select_internal_font(ra8876_t *dev, uint8_t size, uint8_t encoding);
void ra8876_set_text_colors(ra8876_t *dev, uint16_t fg, uint16_t bg);
void ra8876_set_text_transparent(ra8876_t *dev, bool transparent);
void ra8876_set_text_cursor(ra8876_t *dev, uint16_t x, uint16_t y);
void ra8876_put_string(ra8876_t *dev, const char *str);
void ra8876_print(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t color, const char *str);
void ra8876_printf(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t color, const char *fmt, ...);

void ra8876_set_canvas_addr(ra8876_t *dev, uint32_t addr);
void ra8876_set_canvas_page(ra8876_t *dev, uint8_t page);
void ra8876_set_display_addr(ra8876_t *dev, uint32_t addr);

void ra8876_buffer_init(ra8876_t *dev, uint8_t num_pages);
void ra8876_swap_buffers(ra8876_t *dev);
void ra8876_buffer_disable(ra8876_t *dev);
uint8_t ra8876_get_draw_page(ra8876_t *dev);

void ra8876_bte_copy(ra8876_t *dev, uint8_t src_page, uint16_t src_x, uint16_t src_y,
                     uint8_t dst_page, uint16_t dst_x, uint16_t dst_y,
                     uint16_t width, uint16_t height, uint8_t rop);

void ra8876_bte_copy_chroma(ra8876_t *dev, uint8_t src_page, uint16_t src_x, uint16_t src_y,
                            uint8_t dst_page, uint16_t dst_x, uint16_t dst_y,
                            uint16_t width, uint16_t height, uint16_t chroma);

void ra8876_bte_blend(ra8876_t *dev, uint8_t s0_page, uint16_t s0_x, uint16_t s0_y,
                      uint8_t s1_page, uint16_t s1_x, uint16_t s1_y,
                      uint8_t dst_page, uint16_t dst_x, uint16_t dst_y,
                      uint16_t width, uint16_t height, uint8_t alpha);

void ra8876_bte_solid_fill(ra8876_t *dev, uint8_t page, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height, uint16_t color);

void ra8876_bte_batch_start(ra8876_t *dev, uint8_t page, uint16_t width, uint16_t height);
void ra8876_bte_batch_fill(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t color);

void ra8876_bte_write(ra8876_t *dev, uint8_t page, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height,
                      const uint16_t *data);

void ra8876_bte_expand(ra8876_t *dev, uint8_t page, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height,
                       const uint8_t *bitmap, uint16_t fg, uint16_t bg);

void ra8876_set_backlight(ra8876_t *dev, uint8_t brightness);

void ra8876_cursor_show(ra8876_t *dev, bool blink);
void ra8876_cursor_hide(ra8876_t *dev);
void ra8876_cursor_size(ra8876_t *dev, uint8_t width, uint8_t height);
void ra8876_cursor_blink_rate(ra8876_t *dev, uint8_t frames);

void ra8876_invert_area(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void ra8876_pip1_enable(ra8876_t *dev, uint8_t src_page, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void ra8876_pip1_move(ra8876_t *dev, uint16_t x, uint16_t y);
void ra8876_pip1_disable(ra8876_t *dev);
void ra8876_pip2_enable(ra8876_t *dev, uint8_t src_page, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void ra8876_pip2_move(ra8876_t *dev, uint16_t x, uint16_t y);
void ra8876_pip2_disable(ra8876_t *dev);

void ra8876_cgram_init(ra8876_t *dev);
void ra8876_cgram_upload_font(ra8876_t *dev, const uint8_t *data, uint8_t first_char, uint8_t num_chars, uint8_t font_height);
void ra8876_select_cgram_font(ra8876_t *dev, uint8_t size);
void ra8876_put_cgram_string(ra8876_t *dev, const char *str);

void ra8876_set_fg_color(ra8876_t *dev, uint16_t color);
void ra8876_set_bg_color(ra8876_t *dev, uint16_t color);
void ra8876_set_text_mode(ra8876_t *dev);
void ra8876_set_graphics_mode(ra8876_t *dev);

void ra8876_cgram_upload_inv_font(ra8876_t *dev, const uint8_t *data,
                                  uint16_t offset_index, uint8_t first_char,
                                  uint8_t num_chars, uint8_t font_height);

static inline uint16_t ra8876_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#endif
