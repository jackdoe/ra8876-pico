#include "ra8876.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "pico/stdlib.h"

#define CGRAM_ADDR (RA8876_PAGE_SIZE * 10)

static inline void cs_select(ra8876_t *dev) {
    asm volatile("nop \n nop \n nop");
    gpio_put(dev->pin_cs, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(ra8876_t *dev) {
    asm volatile("nop \n nop \n nop");
    gpio_put(dev->pin_cs, 1);
    asm volatile("nop \n nop \n nop");
}

uint8_t ra8876_read_status(ra8876_t *dev) {
    uint8_t cmd = 0x40;
    uint8_t status = 0;
    cs_select(dev);
    spi_write_blocking(dev->spi, &cmd, 1);
    spi_read_blocking(dev->spi, 0, &status, 1);
    cs_deselect(dev);
    return status;
}

void ra8876_write_cmd(ra8876_t *dev, uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    cs_select(dev);
    spi_write_blocking(dev->spi, buf, 2);
    cs_deselect(dev);
}

void ra8876_write_data(ra8876_t *dev, uint8_t data) {
    uint8_t buf[2] = {0x80, data};
    cs_select(dev);
    spi_write_blocking(dev->spi, buf, 2);
    cs_deselect(dev);
}

void ra8876_write_data_burst(ra8876_t *dev, const uint8_t *data, size_t len) {
    const size_t CHUNK_SIZE = 16;
    size_t offset = 0;
    uint8_t cmd_header = 0x80;
    while (offset < len) {
        while ((ra8876_read_status(dev) & 0x40) == 0)
            tight_loop_contents();

        size_t chunk = len - offset;
        if (chunk > CHUNK_SIZE) chunk = CHUNK_SIZE;

        cs_select(dev);
        spi_write_blocking(dev->spi, &cmd_header, 1);
        spi_write_blocking(dev->spi, &data[offset], chunk);
        cs_deselect(dev);

        offset += chunk;
    }
}

uint8_t ra8876_read_data(ra8876_t *dev) {
    uint8_t tx[2] = {0xC0, 0x00};
    uint8_t rx[2];
    cs_select(dev);
    spi_write_read_blocking(dev->spi, tx, rx, 2);
    cs_deselect(dev);
    return rx[1];
}

void ra8876_write_reg(ra8876_t *dev, uint8_t reg, uint8_t val) {
    ra8876_write_cmd(dev, reg);
    ra8876_write_data(dev, val);
}

uint8_t ra8876_read_reg(ra8876_t *dev, uint8_t reg) {
    ra8876_write_cmd(dev, reg);
    return ra8876_read_data(dev);
}

void ra8876_write_reg16(ra8876_t *dev, uint8_t reg, uint16_t val) {
    ra8876_write_reg(dev, reg, val & 0xFF);
    ra8876_write_reg(dev, reg + 1, val >> 8);
}

static void write_reg32(ra8876_t *dev, uint8_t reg, uint32_t val) {
    ra8876_write_reg(dev, reg, val & 0xFF);
    ra8876_write_reg(dev, reg + 1, (val >> 8) & 0xFF);
    ra8876_write_reg(dev, reg + 2, (val >> 16) & 0xFF);
    ra8876_write_reg(dev, reg + 3, (val >> 24) & 0xFF);
}

void ra8876_wait_ready(ra8876_t *dev) {
    while ((ra8876_read_status(dev) & 0x04) == 0);
}

void ra8876_wait_write_fifo(ra8876_t *dev) {
    while (ra8876_read_status(dev) & 0x80);
}

static void ra8876_wait_write_fifo_empty(ra8876_t *dev) {
    while ((ra8876_read_status(dev) & 0x40) == 0);
}

void ra8876_wait_task_busy(ra8876_t *dev) {
    while (ra8876_read_status(dev) & 0x08);
}

static void set_draw_color(ra8876_t *dev, uint8_t color) {
    // RGB332 to RGB888 expansion for foreground color registers
    ra8876_write_reg(dev, 0xD2, (color & 0xE0));
    ra8876_write_reg(dev, 0xD3, (color & 0x1C) << 3);
    ra8876_write_reg(dev, 0xD4, (color & 0x03) << 6);
}

static void set_two_points(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ra8876_write_reg16(dev, 0x68, x0);
    ra8876_write_reg16(dev, 0x6A, y0);
    ra8876_write_reg16(dev, 0x6C, x1);
    ra8876_write_reg16(dev, 0x6E, y1);
}

static void set_three_points(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    ra8876_write_reg16(dev, 0x68, x0);
    ra8876_write_reg16(dev, 0x6A, y0);
    ra8876_write_reg16(dev, 0x6C, x1);
    ra8876_write_reg16(dev, 0x6E, y1);
    ra8876_write_reg16(dev, 0x70, x2);
    ra8876_write_reg16(dev, 0x72, y2);
}

static void draw_and_wait(ra8876_t *dev, uint8_t reg, uint8_t cmd) {
    ra8876_write_reg(dev, reg, cmd);
    ra8876_wait_task_busy(dev);
}

static void soft_reset(ra8876_t *dev) {
    ra8876_write_reg(dev, 0x00, 0x01);
    sleep_ms(100);
    ra8876_write_reg(dev, 0x00, 0x00);
    sleep_ms(100);
}

uint8_t ra8876_get_chip_id(ra8876_t *dev) {
    return ra8876_read_reg(dev, 0xFF);
}

static bool init_pll(ra8876_t *dev) {
    ra8876_write_reg(dev, 0x05, 0x02);
    ra8876_write_reg(dev, 0x06, 0x09);
    ra8876_write_reg(dev, 0x07, 0x02);
    ra8876_write_reg(dev, 0x08, 0x18);
    ra8876_write_reg(dev, 0x09, 0x02);
    ra8876_write_reg(dev, 0x0A, 0x18);
    ra8876_write_reg(dev, 0x01, 0x00);
    sleep_ms(2);
    ra8876_write_reg(dev, 0x01, 0x80);
    sleep_ms(2);
    return (ra8876_read_reg(dev, 0x01) & 0x80) != 0;
}

static bool init_sdram(ra8876_t *dev) {
    ra8876_write_reg(dev, 0xE0, 0x29);
    ra8876_write_reg(dev, 0xE1, 0x03);
    ra8876_write_reg16(dev, 0xE2, 1875);
    ra8876_write_reg(dev, 0xE4, 0x01);
    for (int i = 0; i < 1000; i++) {
        sleep_ms(1);
        if (ra8876_read_status(dev) & 0x04) return true;
    }
    return false;
}

static void init_display(ra8876_t *dev) {
    uint8_t ccr = ra8876_read_reg(dev, 0x01) & 0xE6;
    ra8876_write_reg(dev, 0x01, ccr);
    ra8876_write_reg(dev, 0x02, 0x00);
    ra8876_write_reg(dev, 0x03, 0x00);
    ra8876_write_reg(dev, 0x12, 0x80);
    ra8876_write_reg(dev, 0x13, 0xC0);

    ra8876_write_reg(dev, 0x14, (RA8876_WIDTH / 8) - 1);
    ra8876_write_reg(dev, 0x15, RA8876_WIDTH % 8);
    ra8876_write_reg(dev, 0x16, 19);
    ra8876_write_reg(dev, 0x17, 0);
    ra8876_write_reg(dev, 0x18, 19);
    ra8876_write_reg(dev, 0x19, 8);

    ra8876_write_reg16(dev, 0x1A, RA8876_HEIGHT - 1);
    ra8876_write_reg16(dev, 0x1C, 22);
    ra8876_write_reg(dev, 0x1E, 11);
    ra8876_write_reg(dev, 0x1F, 9);

    ra8876_write_reg(dev, 0x10, 0x00);  // 8bpp mode for main image
    ra8876_write_reg(dev, 0x11, 0x00);  // 8bpp mode for PIP1 and PIP2
    write_reg32(dev, 0x20, 0);
    ra8876_write_reg16(dev, 0x24, RA8876_WIDTH);
    write_reg32(dev, 0x26, 0);
    write_reg32(dev, 0x50, 0);
    ra8876_write_reg16(dev, 0x54, RA8876_WIDTH);
    write_reg32(dev, 0x56, 0);
    ra8876_write_reg16(dev, 0x5A, RA8876_WIDTH);
    ra8876_write_reg16(dev, 0x5C, RA8876_HEIGHT);
    ra8876_write_reg(dev, 0x5E, 0x00);  // 8bpp block mode

    ra8876_write_reg(dev, 0x12, ra8876_read_reg(dev, 0x12) | 0x40);
}

void ra8876_set_backlight(ra8876_t *dev, uint8_t brightness) {
    ra8876_write_reg(dev, 0x84, 0x03);
    ra8876_write_reg(dev, 0x85, 0x5A);
    ra8876_write_reg16(dev, 0x8A, 0xFF);
    ra8876_write_reg(dev, 0x8C, brightness);
    ra8876_write_reg(dev, 0x8D, 0x00);
    ra8876_write_reg(dev, 0x86, 0x03);
}

bool ra8876_init(ra8876_t *dev) {
    dev->char_width = 8;
    dev->char_height = 16;
    dev->cols = RA8876_WIDTH / 8;
    dev->rows = RA8876_HEIGHT / 16;
    dev->cursor_x = 0;
    dev->cursor_y = 0;
    dev->font_size = 0;
    dev->scale_x = 1;
    dev->scale_y = 1;
    dev->fg_color = RA8876_WHITE;
    dev->bg_color = RA8876_BLACK;
    dev->transparent_bg = false;
    dev->canvas_addr = 0;
    dev->draw_page = 0;
    dev->display_page = 0;
    dev->num_pages = 1;
    dev->reg03 = 0x00;
    dev->reg10 = 0x00;  // 8bpp mode
    dev->reg3C = 0x00;
    dev->regCD = 0x00;

    spi_init(dev->spi, dev->spi_speed);
    spi_set_format(dev->spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(dev->pin_miso, GPIO_FUNC_SPI);
    gpio_set_function(dev->pin_sck, GPIO_FUNC_SPI);
    gpio_set_function(dev->pin_mosi, GPIO_FUNC_SPI);
    gpio_init(dev->pin_cs);
    gpio_set_dir(dev->pin_cs, GPIO_OUT);
    gpio_put(dev->pin_cs, 1);

    sleep_ms(100);

    soft_reset(dev);

    uint8_t id = ra8876_get_chip_id(dev);
    if (id != 0x76 && id != 0x77) {
        printf("RA8876: Invalid chip ID 0x%02X\n", id);
        return false;
    }
    if (!init_pll(dev)) {
        printf("RA8876: PLL init failed\n");
        return false;
    }
    if (!init_sdram(dev)) {
        printf("RA8876: SDRAM init failed\n");
        return false;
    }

    init_display(dev);
    ra8876_vsync_init(dev);
    ra8876_set_backlight(dev, 255);
    ra8876_select_internal_font(dev, RA8876_FONT_16, RA8876_ENC_8859_1);
    ra8876_set_text_colors(dev, RA8876_WHITE, RA8876_BLACK);
    return true;
}

void ra8876_fill_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
    set_two_points(dev, x, y, x + w - 1, y + h - 1);
    set_draw_color(dev, color);
    draw_and_wait(dev, 0x76, 0xE0);
}

void ra8876_draw_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
    set_two_points(dev, x, y, x + w - 1, y + h - 1);
    set_draw_color(dev, color);
    draw_and_wait(dev, 0x76, 0xA0);
}

void ra8876_draw_line(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color) {
    set_two_points(dev, x0, y0, x1, y1);
    set_draw_color(dev, color);
    draw_and_wait(dev, 0x67, 0x80);
}

static void draw_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint8_t color, uint8_t cmd) {
    ra8876_write_reg16(dev, 0x7B, x);
    ra8876_write_reg16(dev, 0x7D, y);
    ra8876_write_reg16(dev, 0x77, rx);
    ra8876_write_reg16(dev, 0x79, ry);
    set_draw_color(dev, color);
    draw_and_wait(dev, 0x76, cmd);
}

void ra8876_fill_circle(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t r, uint8_t color) {
    draw_ellipse(dev, x, y, r, r, color, 0xC0);
}

void ra8876_draw_circle(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t r, uint8_t color) {
    draw_ellipse(dev, x, y, r, r, color, 0x80);
}

void ra8876_fill_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint8_t color) {
    draw_ellipse(dev, x, y, rx, ry, color, 0xC0);
}

void ra8876_draw_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint8_t color) {
    draw_ellipse(dev, x, y, rx, ry, color, 0x80);
}

void ra8876_fill_rounded_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint8_t color) {
    set_two_points(dev, x, y, x + w - 1, y + h - 1);
    ra8876_write_reg16(dev, 0x77, r);
    ra8876_write_reg16(dev, 0x79, r);
    set_draw_color(dev, color);
    draw_and_wait(dev, 0x76, 0xF0);
}

void ra8876_draw_rounded_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint8_t color) {
    set_two_points(dev, x, y, x + w - 1, y + h - 1);
    ra8876_write_reg16(dev, 0x77, r);
    ra8876_write_reg16(dev, 0x79, r);
    set_draw_color(dev, color);
    draw_and_wait(dev, 0x76, 0xB0);
}

void ra8876_fill_triangle(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color) {
    set_three_points(dev, x0, y0, x1, y1, x2, y2);
    set_draw_color(dev, color);
    draw_and_wait(dev, 0x67, 0xE2);
}

void ra8876_draw_triangle(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color) {
    set_three_points(dev, x0, y0, x1, y1, x2, y2);
    set_draw_color(dev, color);
    draw_and_wait(dev, 0x67, 0xA2);
}

void ra8876_fill_screen(ra8876_t *dev, uint8_t color) {
    ra8876_fill_rect(dev, 0, 0, RA8876_WIDTH, RA8876_HEIGHT, color);
}

void ra8876_set_text_mode(ra8876_t *dev) {
    ra8876_wait_task_busy(dev);
    dev->reg03 |= 0x04;
    ra8876_write_reg(dev, 0x03, dev->reg03);
}

void ra8876_set_graphics_mode(ra8876_t *dev) {
    ra8876_wait_task_busy(dev);
    dev->reg03 &= ~0x04;
    ra8876_write_reg(dev, 0x03, dev->reg03);
}

void ra8876_select_internal_font(ra8876_t *dev, uint8_t size, uint8_t encoding) {
    dev->font_size = size;
    switch (size) {
        case RA8876_FONT_24: dev->char_width = 12; dev->char_height = 24; break;
        case RA8876_FONT_32: dev->char_width = 16; dev->char_height = 32; break;
        default: dev->char_width = 8; dev->char_height = 16; break;
    }
    dev->cols = RA8876_WIDTH / (dev->char_width * dev->scale_x);
    dev->rows = RA8876_HEIGHT / (dev->char_height * dev->scale_y);

    ra8876_write_reg(dev, 0xCC, ((size & 0x03) << 4) | (encoding & 0x03));
    if (dev->transparent_bg) {
        dev->regCD |= 0x40;
    } else {
        dev->regCD &= ~0x40;
    }
    ra8876_write_reg(dev, 0xCD, dev->regCD);
}

void ra8876_set_fg_color(ra8876_t *dev, uint8_t color) {
    dev->fg_color = color;
    set_draw_color(dev, color);
}

void ra8876_set_bg_color(ra8876_t *dev, uint8_t color) {
    dev->bg_color = color;
    // RGB332 to RGB888 expansion for background color registers
    ra8876_write_reg(dev, 0xD5, (color & 0xE0));
    ra8876_write_reg(dev, 0xD6, (color & 0x1C) << 3);
    ra8876_write_reg(dev, 0xD7, (color & 0x03) << 6);
}

void ra8876_set_text_colors(ra8876_t *dev, uint8_t fg, uint8_t bg) {
    ra8876_set_fg_color(dev, fg);
    ra8876_set_bg_color(dev, bg);
}

void ra8876_set_text_transparent(ra8876_t *dev, bool transparent) {
    dev->transparent_bg = transparent;
    if (transparent) {
        dev->regCD |= 0x40;
    } else {
        dev->regCD &= ~0x40;
    }
    ra8876_write_reg(dev, 0xCD, dev->regCD);
}

void ra8876_set_text_cursor(ra8876_t *dev, uint16_t x, uint16_t y) {
    dev->cursor_x = x;
    dev->cursor_y = y;
    ra8876_write_reg16(dev, 0x63, x);
    ra8876_write_reg16(dev, 0x65, y);
}

void ra8876_put_string(ra8876_t *dev, const char *s) {
    ra8876_set_text_mode(dev);
    ra8876_write_cmd(dev, 0x04);
    size_t len = strlen(s);
    if (len > 0) {
        ra8876_write_data_burst(dev, (const uint8_t *)s, len);
    }
    ra8876_set_graphics_mode(dev);
}

void ra8876_print(ra8876_t *dev, uint16_t x, uint16_t y, uint8_t color, const char *s) {
    ra8876_set_fg_color(dev, color);
    ra8876_set_text_cursor(dev, x, y);
    ra8876_put_string(dev, s);
}

void ra8876_printf(ra8876_t *dev, uint16_t x, uint16_t y, uint8_t color, const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ra8876_print(dev, x, y, color, buf);
}

void ra8876_set_canvas_addr(ra8876_t *dev, uint32_t addr) {
    dev->canvas_addr = addr;
    write_reg32(dev, 0x50, addr);
    ra8876_wait_ready(dev);
}

void ra8876_set_canvas_page(ra8876_t *dev, uint8_t page) {
    if (page >= RA8876_MAX_PAGES) page = RA8876_MAX_PAGES - 1;
    ra8876_set_canvas_addr(dev, page * RA8876_PAGE_SIZE);
}

void ra8876_set_display_addr(ra8876_t *dev, uint32_t addr) {
    write_reg32(dev, 0x20, addr);
    ra8876_wait_ready(dev);
}

void ra8876_buffer_init(ra8876_t *dev, uint8_t num_pages) {
    if (num_pages < 1) num_pages = 1;
    if (num_pages > RA8876_MAX_PAGES) num_pages = RA8876_MAX_PAGES;

    ra8876_wait_task_busy(dev);
    ra8876_wait_vsync(dev);

    dev->num_pages = num_pages;
    dev->display_page = 0;
    dev->draw_page = (num_pages > 1) ? 1 : 0;

    ra8876_set_display_addr(dev, 0);

    for (uint8_t i = 0; i < num_pages; i++) {
        ra8876_set_canvas_page(dev, i);
        ra8876_fill_screen(dev, RA8876_BLACK);
    }

    ra8876_set_canvas_page(dev, dev->draw_page);
}

void ra8876_vsync_init(ra8876_t *dev) {
    ra8876_write_reg(dev, 0x0B, ra8876_read_reg(dev, 0x0B) | 0x10);
    ra8876_write_reg(dev, 0x0C, 0x10);
}

void ra8876_wait_vsync(ra8876_t *dev) {
    while ((ra8876_read_reg(dev, 0x0C) & 0x10) == 0);
    ra8876_write_reg(dev, 0x0C, 0x10);
}

void ra8876_swap_buffers(ra8876_t *dev) {
    if (dev->num_pages < 2) return;

    ra8876_wait_task_busy(dev);
    ra8876_wait_vsync(dev);

    dev->display_page = dev->draw_page;
    dev->draw_page = (dev->draw_page + 1) % dev->num_pages;

    ra8876_set_display_addr(dev, dev->display_page * RA8876_PAGE_SIZE);
    ra8876_set_canvas_addr(dev, dev->draw_page * RA8876_PAGE_SIZE);
}

void ra8876_buffer_disable(ra8876_t *dev) {
    ra8876_wait_task_busy(dev);
    dev->num_pages = 1;
    dev->draw_page = 0;
    dev->display_page = 0;
    ra8876_set_display_addr(dev, 0);
    ra8876_set_canvas_addr(dev, 0);
}

uint8_t ra8876_get_draw_page(ra8876_t *dev) { return dev->draw_page; }

static void bte_set_source0(ra8876_t *dev, uint32_t addr, uint16_t width, uint16_t x, uint16_t y) {
    write_reg32(dev, 0x93, addr);
    ra8876_write_reg16(dev, 0x97, width);
    ra8876_write_reg16(dev, 0x99, x);
    ra8876_write_reg16(dev, 0x9B, y);
}

static void bte_set_source1(ra8876_t *dev, uint32_t addr, uint16_t width, uint16_t x, uint16_t y) {
    write_reg32(dev, 0x9D, addr);
    ra8876_write_reg16(dev, 0xA1, width);
    ra8876_write_reg16(dev, 0xA3, x);
    ra8876_write_reg16(dev, 0xA5, y);
}

static void bte_set_dest(ra8876_t *dev, uint32_t addr, uint16_t width, uint16_t x, uint16_t y) {
    write_reg32(dev, 0xA7, addr);
    ra8876_write_reg16(dev, 0xAB, width);
    ra8876_write_reg16(dev, 0xAD, x);
    ra8876_write_reg16(dev, 0xAF, y);
}

static void bte_set_size(ra8876_t *dev, uint16_t width, uint16_t height) {
    ra8876_write_reg16(dev, 0xB1, width);
    ra8876_write_reg16(dev, 0xB3, height);
}

static void bte_start(ra8876_t *dev, uint8_t rop, uint8_t op) {
    ra8876_write_reg(dev, 0x92, 0x00);  // 8bpp for S0, S1, and Dest
    ra8876_write_reg(dev, 0x91, (rop & 0xF0) | (op & 0x0F));
    ra8876_write_reg(dev, 0x90, 0x10);
    while (ra8876_read_reg(dev, 0x90) & 0x10);
}

void ra8876_bte_copy(ra8876_t *dev, uint8_t src_page, uint16_t src_x, uint16_t src_y,
                     uint8_t dst_page, uint16_t dst_x, uint16_t dst_y,
                     uint16_t width, uint16_t height, uint8_t rop) {
    ra8876_wait_task_busy(dev);
    bte_set_source0(dev, src_page * RA8876_PAGE_SIZE, RA8876_WIDTH, src_x, src_y);
    bte_set_dest(dev, dst_page * RA8876_PAGE_SIZE, RA8876_WIDTH, dst_x, dst_y);
    bte_set_size(dev, width, height);
    bte_start(dev, rop, 0x02);
}

void ra8876_bte_copy_chroma(ra8876_t *dev, uint8_t src_page, uint16_t src_x, uint16_t src_y,
                            uint8_t dst_page, uint16_t dst_x, uint16_t dst_y,
                            uint16_t width, uint16_t height, uint8_t chroma) {
    ra8876_wait_task_busy(dev);
    // RGB332 chroma key in background color registers
    ra8876_write_reg(dev, 0xD5, (chroma & 0xE0));
    ra8876_write_reg(dev, 0xD6, (chroma & 0x1C) << 3);
    ra8876_write_reg(dev, 0xD7, (chroma & 0x03) << 6);
    bte_set_source0(dev, src_page * RA8876_PAGE_SIZE, RA8876_WIDTH, src_x, src_y);
    bte_set_dest(dev, dst_page * RA8876_PAGE_SIZE, RA8876_WIDTH, dst_x, dst_y);
    bte_set_size(dev, width, height);
    bte_start(dev, RA8876_ROP_S, 0x05);
}

void ra8876_bte_blend(ra8876_t *dev, uint8_t s0_page, uint16_t s0_x, uint16_t s0_y,
                      uint8_t s1_page, uint16_t s1_x, uint16_t s1_y,
                      uint8_t dst_page, uint16_t dst_x, uint16_t dst_y,
                      uint16_t width, uint16_t height, uint8_t alpha) {
    ra8876_wait_task_busy(dev);
    bte_set_source0(dev, s0_page * RA8876_PAGE_SIZE, RA8876_WIDTH, s0_x, s0_y);
    bte_set_source1(dev, s1_page * RA8876_PAGE_SIZE, RA8876_WIDTH, s1_x, s1_y);
    bte_set_dest(dev, dst_page * RA8876_PAGE_SIZE, RA8876_WIDTH, dst_x, dst_y);
    bte_set_size(dev, width, height);
    ra8876_write_reg(dev, 0xB5, alpha >> 3);
    bte_start(dev, RA8876_ROP_S, 0x0A);
}

void ra8876_bte_solid_fill(ra8876_t *dev, uint8_t page, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height, uint8_t color) {
    ra8876_wait_task_busy(dev);
    set_draw_color(dev, color);
    bte_set_dest(dev, page * RA8876_PAGE_SIZE, RA8876_WIDTH, x, y);
    bte_set_size(dev, width, height);
    bte_start(dev, RA8876_ROP_S, 0x0C);
}

void ra8876_bte_batch_start(ra8876_t *dev, uint8_t page, uint16_t width, uint16_t height) {
    write_reg32(dev, 0xA7, page * RA8876_PAGE_SIZE);
    ra8876_write_reg16(dev, 0xAB, RA8876_WIDTH);
    bte_set_size(dev, width, height);
    ra8876_write_reg(dev, 0x92, 0x00);  // 8bpp for S0, S1, and Dest
    ra8876_write_reg(dev, 0x91, (RA8876_ROP_S & 0xF0) | 0x0C);
}

void ra8876_bte_batch_fill(ra8876_t *dev, uint16_t x, uint16_t y, uint8_t color) {
    ra8876_write_reg16(dev, 0xAD, x);
    ra8876_write_reg16(dev, 0xAF, y);
    set_draw_color(dev, color);
    ra8876_write_reg(dev, 0x90, 0x10);
    while (ra8876_read_reg(dev, 0x90) & 0x10);
}

void ra8876_bte_write(ra8876_t *dev, uint8_t page, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height,
                      const uint8_t *data) {
    ra8876_wait_task_busy(dev);
    bte_set_dest(dev, page * RA8876_PAGE_SIZE, RA8876_WIDTH, x, y);
    bte_set_size(dev, width, height);
    ra8876_write_reg(dev, 0x92, 0x00);  // 8bpp for S0, S1, and Dest
    ra8876_write_reg(dev, 0x91, (RA8876_ROP_S & 0xF0) | 0x00);
    ra8876_write_reg(dev, 0x90, 0x10);
    ra8876_write_cmd(dev, 0x04);

    while (ra8876_read_status(dev) & 0x80);
    ra8876_write_data_burst(dev, data, width * height);  // 1 byte per pixel

    while (ra8876_read_reg(dev, 0x90) & 0x10);
}

void ra8876_bte_expand(ra8876_t *dev, uint8_t page, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height,
                       const uint8_t *bitmap, uint8_t fg, uint8_t bg) {
    ra8876_wait_task_busy(dev);
    set_draw_color(dev, fg);
    ra8876_set_bg_color(dev, bg);
    bte_set_dest(dev, page * RA8876_PAGE_SIZE, RA8876_WIDTH, x, y);
    bte_set_size(dev, width, height);
    ra8876_write_reg(dev, 0x92, 0x00);  // 8bpp for dest
    ra8876_write_reg(dev, 0x91, (RA8876_ROP_S & 0xF0) | 0x08);
    ra8876_write_reg(dev, 0x90, 0x10);
    ra8876_write_cmd(dev, 0x04);

    size_t row_bytes = (width + 7) / 8;
    while (ra8876_read_status(dev) & 0x80);
    ra8876_write_data_burst(dev, bitmap, row_bytes * height);

    while (ra8876_read_reg(dev, 0x90) & 0x10);
}

void ra8876_cursor_show(ra8876_t *dev, bool blink) {
    dev->reg3C = 0x02 | (blink ? 0x01 : 0x00);
    ra8876_write_reg(dev, 0x3C, dev->reg3C);
}

void ra8876_cursor_hide(ra8876_t *dev) {
    dev->reg3C &= ~0x03;
    ra8876_write_reg(dev, 0x3C, dev->reg3C);
}

void ra8876_cursor_size(ra8876_t *dev, uint8_t width, uint8_t height) {
    ra8876_write_reg(dev, 0x3E, width - 1);
    ra8876_write_reg(dev, 0x3F, height - 1);
}

void ra8876_cursor_blink_rate(ra8876_t *dev, uint8_t frames) {
    ra8876_write_reg(dev, 0x3D, frames);
}

void ra8876_invert_area(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    ra8876_wait_task_busy(dev);
    uint32_t addr = dev->canvas_addr;
    bte_set_source0(dev, addr, RA8876_WIDTH, x, y);
    bte_set_dest(dev, addr, RA8876_WIDTH, x, y);
    bte_set_size(dev, w, h);
    bte_start(dev, RA8876_ROP_NOT_S, 0x02);
}

void ra8876_pip1_enable(ra8876_t *dev, uint8_t src_page, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    dev->reg10 &= ~0x10;
    ra8876_write_reg(dev, 0x10, dev->reg10);
    write_reg32(dev, 0x2E, src_page * RA8876_PAGE_SIZE);
    ra8876_write_reg16(dev, 0x32, RA8876_WIDTH);
    ra8876_write_reg16(dev, 0x2A, x);
    ra8876_write_reg16(dev, 0x2C, y);
    ra8876_write_reg16(dev, 0x34, 0);
    ra8876_write_reg16(dev, 0x36, 0);
    ra8876_write_reg16(dev, 0x38, w);
    ra8876_write_reg16(dev, 0x3A, h);
    dev->reg10 |= 0x80;
    ra8876_write_reg(dev, 0x10, dev->reg10);
}

void ra8876_pip1_move(ra8876_t *dev, uint16_t x, uint16_t y) {
    dev->reg10 &= ~0x10;
    ra8876_write_reg(dev, 0x10, dev->reg10);
    ra8876_write_reg16(dev, 0x2A, x);
    ra8876_write_reg16(dev, 0x2C, y);
}

void ra8876_pip1_disable(ra8876_t *dev) {
    dev->reg10 &= ~0x80;
    ra8876_write_reg(dev, 0x10, dev->reg10);
}

void ra8876_pip2_enable(ra8876_t *dev, uint8_t src_page, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    dev->reg10 |= 0x10;
    ra8876_write_reg(dev, 0x10, dev->reg10);
    write_reg32(dev, 0x2E, src_page * RA8876_PAGE_SIZE);
    ra8876_write_reg16(dev, 0x32, RA8876_WIDTH);
    ra8876_write_reg16(dev, 0x2A, x);
    ra8876_write_reg16(dev, 0x2C, y);
    ra8876_write_reg16(dev, 0x34, 0);
    ra8876_write_reg16(dev, 0x36, 0);
    ra8876_write_reg16(dev, 0x38, w);
    ra8876_write_reg16(dev, 0x3A, h);
    dev->reg10 |= 0x40;
    ra8876_write_reg(dev, 0x10, dev->reg10);
}

void ra8876_pip2_move(ra8876_t *dev, uint16_t x, uint16_t y) {
    dev->reg10 |= 0x10;
    ra8876_write_reg(dev, 0x10, dev->reg10);
    ra8876_write_reg16(dev, 0x2A, x);
    ra8876_write_reg16(dev, 0x2C, y);
}

void ra8876_pip2_disable(ra8876_t *dev) {
    dev->reg10 &= ~0x40;
    ra8876_write_reg(dev, 0x10, dev->reg10);
}

void ra8876_cgram_init(ra8876_t *dev) {
    write_reg32(dev, 0xDB, CGRAM_ADDR);
}

void ra8876_cgram_upload_font(ra8876_t *dev, const uint8_t *data, uint8_t first_char, uint8_t num_chars, uint8_t font_height) {
    uint32_t bytes_per_char;
    switch (font_height) {
        case 24: bytes_per_char = 48; break;
        case 32: bytes_per_char = 64; break;
        default: bytes_per_char = 16; break;
    }

    uint32_t addr = CGRAM_ADDR + first_char * bytes_per_char;
    uint32_t total = num_chars * bytes_per_char;

    ra8876_wait_task_busy(dev);
    ra8876_write_reg(dev, 0x5E, 0x04);
    write_reg32(dev, 0x5F, addr);
    ra8876_write_cmd(dev, 0x04);

    while (ra8876_read_status(dev) & 0x80);
    ra8876_write_data_burst(dev, data, total);

    ra8876_wait_write_fifo_empty(dev);
    ra8876_wait_task_busy(dev);
    ra8876_write_reg(dev, 0x5E, 0x00);  // restore 8bpp block mode
    ra8876_write_cmd(dev, 0xFF);
}

void ra8876_select_cgram_font(ra8876_t *dev, uint8_t size) {
    dev->font_size = size;
    switch (size) {
        case RA8876_FONT_24: dev->char_width = 12; dev->char_height = 24; break;
        case RA8876_FONT_32: dev->char_width = 16; dev->char_height = 32; break;
        default: dev->char_width = 8; dev->char_height = 16; break;
    }
    dev->cols = RA8876_WIDTH / (dev->char_width * dev->scale_x);
    dev->rows = RA8876_HEIGHT / (dev->char_height * dev->scale_y);
    write_reg32(dev, 0xDB, CGRAM_ADDR);
    ra8876_write_reg(dev, 0xCC, 0x80 | ((size & 0x03) << 4));
}

void ra8876_put_cgram_string(ra8876_t *dev, const char *str) {
    ra8876_set_text_mode(dev);
    ra8876_write_cmd(dev, 0x04);

    // CGRAM needs 2 bytes per char (0x00 + ASCII), pack into buffer
    uint8_t buf[32];  // 16 chars at a time
    while (*str) {
        size_t count = 0;
        while (*str && count < 32) {
            buf[count++] = 0x00;
            buf[count++] = *str++;
        }
        ra8876_write_data_burst(dev, buf, count);
    }
    ra8876_set_graphics_mode(dev);
}

void ra8876_cgram_upload_inv_font(ra8876_t *dev, const uint8_t *data,
                                  uint16_t offset_index, uint8_t first_char,
                                  uint8_t num_chars, uint8_t font_height) {
    uint32_t bytes_per_char;
    switch (font_height) {
        case 24: bytes_per_char = 48; break;
        case 32: bytes_per_char = 64; break;
        default: bytes_per_char = 16; break;
    }

    uint32_t start_char_idx = offset_index * 256 + first_char;
    uint32_t addr = CGRAM_ADDR + start_char_idx * bytes_per_char;
    uint32_t total_bytes = num_chars * bytes_per_char;

    ra8876_wait_task_busy(dev);
    
    ra8876_write_reg(dev, 0x5E, 0x04);
    write_reg32(dev, 0x5F, addr);
    ra8876_write_cmd(dev, 0x04);

    while (ra8876_read_status(dev) & 0x80);

    const size_t CHUNK_SIZE = 16;
    size_t offset = 0;
    
    while (offset < total_bytes) {
        while ((ra8876_read_status(dev) & 0x40) == 0);

        size_t chunk = total_bytes - offset;
        if (chunk > CHUNK_SIZE) chunk = CHUNK_SIZE;

        cs_select(dev);
        uint8_t cmd = 0x80;
        spi_write_blocking(dev->spi, &cmd, 1);
        
        for(size_t i=0; i<chunk; i++) {
            uint8_t val = ~data[offset + i];
            spi_write_blocking(dev->spi, &val, 1);
        }
        
        cs_deselect(dev);
        offset += chunk;
    }

    ra8876_wait_write_fifo_empty(dev);
    ra8876_wait_task_busy(dev);
    ra8876_write_reg(dev, 0x5E, 0x00);  // restore 8bpp block mode
    ra8876_write_cmd(dev, 0xFF);
}
