#include "ra8876.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define PIN_MISO    16
#define PIN_CS      17
#define PIN_SCK     18
#define PIN_MOSI    19
#define SPI_PORT    spi0
#define SPI_SPEED   20000000

typedef struct {
    uint16_t char_width;
    uint16_t char_height;
    uint16_t cols;
    uint16_t rows;
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint8_t  font_size;
    uint8_t  scale_x;
    uint8_t  scale_y;
    uint16_t fg_color;
    uint16_t bg_color;
    bool     transparent_bg;
} text_state_t;

static text_state_t g_text = {
    .char_width = 8, .char_height = 16,
    .cols = RA8876_WIDTH / 8, .rows = RA8876_HEIGHT / 16,
    .cursor_x = 0, .cursor_y = 0,
    .font_size = 0, .scale_x = 1, .scale_y = 1,
    .fg_color = RA8876_WHITE, .bg_color = RA8876_BLACK,
    .transparent_bg = false
};

static uint32_t g_canvas_addr = 0;

static uint8_t g_draw_page = 0;
static uint8_t g_display_page = 0;
static uint8_t g_num_pages = 1;

static uint8_t g_reg03 = 0x00;
static uint8_t g_reg10 = 0x04;
static uint8_t g_reg3C = 0x00;
static uint8_t g_regCD = 0x00;

static inline void cs_select(void) {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(void) {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

uint8_t ra8876_read_status(void) {
    uint8_t cmd = 0x40;
    uint8_t status = 0;
    cs_select();
    spi_write_blocking(SPI_PORT, &cmd, 1);
    spi_read_blocking(SPI_PORT, 0, &status, 1);
    cs_deselect();
    return status;
}

void ra8876_write_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    cs_select();
    spi_write_blocking(SPI_PORT, buf, 2);
    cs_deselect();
}

void ra8876_write_data(uint8_t data) {
    while (ra8876_read_status() & 0x80);
    uint8_t buf[2] = {0x80, data};
    cs_select();
    spi_write_blocking(SPI_PORT, buf, 2);
    cs_deselect();
}

static uint8_t g_burst_buf[65];

void ra8876_write_data_burst(const uint8_t *data, size_t len) {
    g_burst_buf[0] = 0x80;
    size_t offset = 0;
    while (offset < len) {
        while ((ra8876_read_status() & 0x40) == 0)
            tight_loop_contents();
        size_t chunk = len - offset;
        if (chunk > 64) chunk = 64;
        memcpy(&g_burst_buf[1], &data[offset], chunk);
        cs_select();
        spi_write_blocking(SPI_PORT, g_burst_buf, chunk + 1);
        cs_deselect();
        offset += chunk;
    }
}

uint8_t ra8876_read_data(void) {
    uint8_t tx[2] = {0xC0, 0x00};
    uint8_t rx[2];
    cs_select();
    spi_write_read_blocking(SPI_PORT, tx, rx, 2);
    cs_deselect();
    return rx[1];
}

void ra8876_write_reg(uint8_t reg, uint8_t val) {
    ra8876_write_cmd(reg);
    ra8876_write_data(val);
}

uint8_t ra8876_read_reg(uint8_t reg) {
    ra8876_write_cmd(reg);
    return ra8876_read_data();
}

void ra8876_write_reg16(uint8_t reg, uint16_t val) {
    ra8876_write_reg(reg, val & 0xFF);
    ra8876_write_reg(reg + 1, val >> 8);
}

static void write_reg32(uint8_t reg, uint32_t val) {
    ra8876_write_reg(reg, val & 0xFF);
    ra8876_write_reg(reg + 1, (val >> 8) & 0xFF);
    ra8876_write_reg(reg + 2, (val >> 16) & 0xFF);
    ra8876_write_reg(reg + 3, (val >> 24) & 0xFF);
}

void ra8876_wait_ready(void) {
    while ((ra8876_read_status() & 0x04) == 0);
}

void ra8876_wait_write_fifo(void) {
    while (ra8876_read_status() & 0x80);
}

static void ra8876_wait_write_fifo_empty(void) {
    while ((ra8876_read_status() & 0x40) == 0);
}

void ra8876_wait_task_busy(void) {
    while (ra8876_read_status() & 0x08);
}

static void set_draw_color(uint16_t color) {
    ra8876_write_reg(0xD2, (color >> 11) << 3);
    ra8876_write_reg(0xD3, ((color >> 5) & 0x3F) << 2);
    ra8876_write_reg(0xD4, (color & 0x1F) << 3);
}

static void set_two_points(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ra8876_write_reg16(0x68, x0);
    ra8876_write_reg16(0x6A, y0);
    ra8876_write_reg16(0x6C, x1);
    ra8876_write_reg16(0x6E, y1);
}

static void set_three_points(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    ra8876_write_reg16(0x68, x0);
    ra8876_write_reg16(0x6A, y0);
    ra8876_write_reg16(0x6C, x1);
    ra8876_write_reg16(0x6E, y1);
    ra8876_write_reg16(0x70, x2);
    ra8876_write_reg16(0x72, y2);
}

static void draw_and_wait(uint8_t reg, uint8_t cmd) {
    ra8876_write_reg(reg, cmd);
    ra8876_wait_task_busy();
}

static void soft_reset(void) {
    ra8876_write_reg(0x00, 0x01);
    sleep_ms(100);
    ra8876_write_reg(0x00, 0x00);
    sleep_ms(100);
}

uint8_t ra8876_get_chip_id(void) {
    return ra8876_read_reg(0xFF);
}

static bool init_pll(void) {
    ra8876_write_reg(0x05, 0x02);
    ra8876_write_reg(0x06, 0x09);
    ra8876_write_reg(0x07, 0x02);
    ra8876_write_reg(0x08, 0x18);
    ra8876_write_reg(0x09, 0x02);
    ra8876_write_reg(0x0A, 0x18);
    ra8876_write_reg(0x01, 0x00);
    sleep_ms(2);
    ra8876_write_reg(0x01, 0x80);
    sleep_ms(2);
    return (ra8876_read_reg(0x01) & 0x80) != 0;
}

static bool init_sdram(void) {
    ra8876_write_reg(0xE0, 0x29);
    ra8876_write_reg(0xE1, 0x03);
    ra8876_write_reg16(0xE2, 1875);
    ra8876_write_reg(0xE4, 0x01);
    for (int i = 0; i < 1000; i++) {
        sleep_ms(1);
        if (ra8876_read_status() & 0x04) return true;
    }
    return false;
}

static void init_display(void) {
    uint8_t ccr = ra8876_read_reg(0x01) & 0xE6;
    ra8876_write_reg(0x01, ccr);
    ra8876_write_reg(0x02, 0x00);
    ra8876_write_reg(0x03, 0x00);
    ra8876_write_reg(0x12, 0x80);
    ra8876_write_reg(0x13, 0xC0);

    ra8876_write_reg(0x14, (RA8876_WIDTH / 8) - 1);
    ra8876_write_reg(0x15, RA8876_WIDTH % 8);
    ra8876_write_reg(0x16, 19);
    ra8876_write_reg(0x17, 0);
    ra8876_write_reg(0x18, 19);
    ra8876_write_reg(0x19, 8);

    ra8876_write_reg16(0x1A, RA8876_HEIGHT - 1);
    ra8876_write_reg16(0x1C, 22);
    ra8876_write_reg(0x1E, 11);
    ra8876_write_reg(0x1F, 9);

    ra8876_write_reg(0x10, 0x04);
    write_reg32(0x20, 0);
    ra8876_write_reg16(0x24, RA8876_WIDTH);
    write_reg32(0x26, 0);
    write_reg32(0x50, 0);
    ra8876_write_reg16(0x54, RA8876_WIDTH);
    write_reg32(0x56, 0);
    ra8876_write_reg16(0x5A, RA8876_WIDTH);
    ra8876_write_reg16(0x5C, RA8876_HEIGHT);
    ra8876_write_reg(0x5E, 0x01);

    ra8876_write_reg(0x12, ra8876_read_reg(0x12) | 0x40);
}

void ra8876_set_backlight(uint8_t brightness) {
    ra8876_write_reg(0x84, 0x03);
    ra8876_write_reg(0x85, 0x5A);
    ra8876_write_reg16(0x8A, 0xFF);
    ra8876_write_reg(0x8C, brightness);
    ra8876_write_reg(0x8D, 0x00);
    ra8876_write_reg(0x86, 0x03);
}

bool ra8876_init(void) {
    spi_init(SPI_PORT, SPI_SPEED);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    sleep_ms(100);

    soft_reset();

    uint8_t id = ra8876_get_chip_id();
    if (id != 0x76 && id != 0x77) {
        printf("RA8876: Invalid chip ID 0x%02X\n", id);
        return false;
    }
    if (!init_pll()) {
        printf("RA8876: PLL init failed\n");
        return false;
    }
    if (!init_sdram()) {
        printf("RA8876: SDRAM init failed\n");
        return false;
    }

    init_display();
    ra8876_vsync_init();
    ra8876_set_backlight(255);
    ra8876_select_internal_font(RA8876_FONT_16, RA8876_ENC_8859_1);
    ra8876_set_text_colors(RA8876_WHITE, RA8876_BLACK);
    return true;
}

void ra8876_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    set_two_points(x, y, x + w - 1, y + h - 1);
    set_draw_color(color);
    draw_and_wait(0x76, 0xE0);
}

void ra8876_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    set_two_points(x, y, x + w - 1, y + h - 1);
    set_draw_color(color);
    draw_and_wait(0x76, 0xA0);
}

void ra8876_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    set_two_points(x0, y0, x1, y1);
    set_draw_color(color);
    draw_and_wait(0x67, 0x80);
}

static void draw_ellipse(uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint16_t color, uint8_t cmd) {
    ra8876_write_reg16(0x7B, x);
    ra8876_write_reg16(0x7D, y);
    ra8876_write_reg16(0x77, rx);
    ra8876_write_reg16(0x79, ry);
    set_draw_color(color);
    draw_and_wait(0x76, cmd);
}

void ra8876_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color) {
    draw_ellipse(x, y, r, r, color, 0xC0);
}

void ra8876_draw_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color) {
    draw_ellipse(x, y, r, r, color, 0x80);
}

void ra8876_fill_ellipse(uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint16_t color) {
    draw_ellipse(x, y, rx, ry, color, 0xC0);
}

void ra8876_draw_ellipse(uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint16_t color) {
    draw_ellipse(x, y, rx, ry, color, 0x80);
}

void ra8876_fill_rounded_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color) {
    set_two_points(x, y, x + w - 1, y + h - 1);
    ra8876_write_reg16(0x77, r);
    ra8876_write_reg16(0x79, r);
    set_draw_color(color);
    draw_and_wait(0x76, 0xF0);
}

void ra8876_draw_rounded_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color) {
    set_two_points(x, y, x + w - 1, y + h - 1);
    ra8876_write_reg16(0x77, r);
    ra8876_write_reg16(0x79, r);
    set_draw_color(color);
    draw_and_wait(0x76, 0xB0);
}

void ra8876_fill_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    set_three_points(x0, y0, x1, y1, x2, y2);
    set_draw_color(color);
    draw_and_wait(0x67, 0xE2);
}

void ra8876_draw_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    set_three_points(x0, y0, x1, y1, x2, y2);
    set_draw_color(color);
    draw_and_wait(0x67, 0xA2);
}

void ra8876_fill_screen(uint16_t color) {
    ra8876_fill_rect(0, 0, RA8876_WIDTH, RA8876_HEIGHT, color);
}

void ra8876_set_text_mode(void) {
    ra8876_wait_task_busy();
    g_reg03 |= 0x04;
    ra8876_write_reg(0x03, g_reg03);
}

void ra8876_set_graphics_mode(void) {
    ra8876_wait_task_busy();
    g_reg03 &= ~0x04;
    ra8876_write_reg(0x03, g_reg03);
}

void ra8876_select_internal_font(uint8_t size, uint8_t encoding) {
    g_text.font_size = size;
    switch (size) {
        case RA8876_FONT_24: g_text.char_width = 12; g_text.char_height = 24; break;
        case RA8876_FONT_32: g_text.char_width = 16; g_text.char_height = 32; break;
        default: g_text.char_width = 8; g_text.char_height = 16; break;
    }
    g_text.cols = RA8876_WIDTH / (g_text.char_width * g_text.scale_x);
    g_text.rows = RA8876_HEIGHT / (g_text.char_height * g_text.scale_y);

    ra8876_write_reg(0xCC, ((size & 0x03) << 4) | (encoding & 0x03));
    if (g_text.transparent_bg) {
        g_regCD |= 0x40;
    } else {
        g_regCD &= ~0x40;
    }
    ra8876_write_reg(0xCD, g_regCD);
}

void ra8876_set_fg_color(uint16_t color) {
    g_text.fg_color = color;
    set_draw_color(color);
}

void ra8876_set_bg_color(uint16_t color) {
    g_text.bg_color = color;
    ra8876_write_reg(0xD5, (color >> 11) << 3);
    ra8876_write_reg(0xD6, ((color >> 5) & 0x3F) << 2);
    ra8876_write_reg(0xD7, (color & 0x1F) << 3);
}

void ra8876_set_text_colors(uint16_t fg, uint16_t bg) {
    ra8876_set_fg_color(fg);
    ra8876_set_bg_color(bg);
}

void ra8876_set_text_transparent(bool transparent) {
    g_text.transparent_bg = transparent;
    if (transparent) {
        g_regCD |= 0x40;
    } else {
        g_regCD &= ~0x40;
    }
    ra8876_write_reg(0xCD, g_regCD);
}

void ra8876_set_text_cursor(uint16_t x, uint16_t y) {
    g_text.cursor_x = x;
    g_text.cursor_y = y;
    ra8876_write_reg16(0x63, x);
    ra8876_write_reg16(0x65, y);
}

void ra8876_put_string(const char *s) {
    ra8876_set_text_mode();
    ra8876_write_cmd(0x04);
    while (*s) {
        ra8876_wait_write_fifo();
        ra8876_write_data(*s++);
    }
    ra8876_set_graphics_mode();
}

void ra8876_print(uint16_t x, uint16_t y, uint16_t color, const char *s) {
    ra8876_set_fg_color(color);
    ra8876_set_text_cursor(x, y);
    ra8876_put_string(s);
}

void ra8876_printf(uint16_t x, uint16_t y, uint16_t color, const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ra8876_print(x, y, color, buf);
}

void ra8876_set_canvas_addr(uint32_t addr) {
    g_canvas_addr = addr;
    write_reg32(0x50, addr);
    ra8876_wait_ready();
}

void ra8876_set_canvas_page(uint8_t page) {
    if (page >= RA8876_MAX_PAGES) page = RA8876_MAX_PAGES - 1;
    ra8876_set_canvas_addr(page * RA8876_PAGE_SIZE);
}

void ra8876_set_display_addr(uint32_t addr) {
    write_reg32(0x20, addr);
    ra8876_wait_ready();
}

void ra8876_buffer_init(uint8_t num_pages) {
    if (num_pages < 1) num_pages = 1;
    if (num_pages > RA8876_MAX_PAGES) num_pages = RA8876_MAX_PAGES;

    ra8876_wait_task_busy();
    ra8876_wait_vsync();

    g_num_pages = num_pages;
    g_display_page = 0;
    g_draw_page = (num_pages > 1) ? 1 : 0;

    ra8876_set_display_addr(0);

    for (uint8_t i = 0; i < num_pages; i++) {
        ra8876_set_canvas_page(i);
        ra8876_fill_screen(RA8876_BLACK);
    }

    ra8876_set_canvas_page(g_draw_page);
}

void ra8876_vsync_init(void) {
    ra8876_write_reg(0x0B, ra8876_read_reg(0x0B) | 0x10);
    ra8876_write_reg(0x0C, 0x10);
}

void ra8876_wait_vsync(void) {
    while ((ra8876_read_reg(0x0C) & 0x10) == 0);
    ra8876_write_reg(0x0C, 0x10);
}

void ra8876_swap_buffers(void) {
    if (g_num_pages < 2) return;

    ra8876_wait_task_busy();
    ra8876_wait_vsync();

    g_display_page = g_draw_page;
    g_draw_page = (g_draw_page + 1) % g_num_pages;

    ra8876_set_display_addr(g_display_page * RA8876_PAGE_SIZE);
    ra8876_set_canvas_addr(g_draw_page * RA8876_PAGE_SIZE);
}

void ra8876_buffer_disable(void) {
    ra8876_wait_task_busy();
    g_num_pages = 1;
    g_draw_page = 0;
    g_display_page = 0;
    ra8876_set_display_addr(0);
    ra8876_set_canvas_addr(0);
}

uint8_t ra8876_get_draw_page(void) { return g_draw_page; }

static void bte_set_source0(uint32_t addr, uint16_t width, uint16_t x, uint16_t y) {
    write_reg32(0x93, addr);
    ra8876_write_reg16(0x97, width);
    ra8876_write_reg16(0x99, x);
    ra8876_write_reg16(0x9B, y);
}

static void bte_set_source1(uint32_t addr, uint16_t width, uint16_t x, uint16_t y) {
    write_reg32(0x9D, addr);
    ra8876_write_reg16(0xA1, width);
    ra8876_write_reg16(0xA3, x);
    ra8876_write_reg16(0xA5, y);
}

static void bte_set_dest(uint32_t addr, uint16_t width, uint16_t x, uint16_t y) {
    write_reg32(0xA7, addr);
    ra8876_write_reg16(0xAB, width);
    ra8876_write_reg16(0xAD, x);
    ra8876_write_reg16(0xAF, y);
}

static void bte_set_size(uint16_t width, uint16_t height) {
    ra8876_write_reg16(0xB1, width);
    ra8876_write_reg16(0xB3, height);
}

static void bte_start(uint8_t rop, uint8_t op) {
    ra8876_write_reg(0x92, 0x25);
    ra8876_write_reg(0x91, (rop & 0xF0) | (op & 0x0F));
    ra8876_write_reg(0x90, 0x10);
    while (ra8876_read_reg(0x90) & 0x10);
}

void ra8876_bte_copy(uint8_t src_page, uint16_t src_x, uint16_t src_y,
                     uint8_t dst_page, uint16_t dst_x, uint16_t dst_y,
                     uint16_t width, uint16_t height, uint8_t rop) {
    ra8876_wait_task_busy();
    bte_set_source0(src_page * RA8876_PAGE_SIZE, RA8876_WIDTH, src_x, src_y);
    bte_set_dest(dst_page * RA8876_PAGE_SIZE, RA8876_WIDTH, dst_x, dst_y);
    bte_set_size(width, height);
    bte_start(rop, 0x02);
}

void ra8876_bte_copy_chroma(uint8_t src_page, uint16_t src_x, uint16_t src_y,
                            uint8_t dst_page, uint16_t dst_x, uint16_t dst_y,
                            uint16_t width, uint16_t height, uint16_t chroma) {
    ra8876_wait_task_busy();
    ra8876_write_reg(0xD5, (chroma >> 11) << 3);
    ra8876_write_reg(0xD6, ((chroma >> 5) & 0x3F) << 2);
    ra8876_write_reg(0xD7, (chroma & 0x1F) << 3);
    bte_set_source0(src_page * RA8876_PAGE_SIZE, RA8876_WIDTH, src_x, src_y);
    bte_set_dest(dst_page * RA8876_PAGE_SIZE, RA8876_WIDTH, dst_x, dst_y);
    bte_set_size(width, height);
    bte_start(RA8876_ROP_S, 0x05);
}

void ra8876_bte_blend(uint8_t s0_page, uint16_t s0_x, uint16_t s0_y,
                      uint8_t s1_page, uint16_t s1_x, uint16_t s1_y,
                      uint8_t dst_page, uint16_t dst_x, uint16_t dst_y,
                      uint16_t width, uint16_t height, uint8_t alpha) {
    ra8876_wait_task_busy();
    bte_set_source0(s0_page * RA8876_PAGE_SIZE, RA8876_WIDTH, s0_x, s0_y);
    bte_set_source1(s1_page * RA8876_PAGE_SIZE, RA8876_WIDTH, s1_x, s1_y);
    bte_set_dest(dst_page * RA8876_PAGE_SIZE, RA8876_WIDTH, dst_x, dst_y);
    bte_set_size(width, height);
    ra8876_write_reg(0xB5, alpha >> 3);
    bte_start(RA8876_ROP_S, 0x0A);
}

void ra8876_bte_solid_fill(uint8_t page, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height, uint16_t color) {
    ra8876_wait_task_busy();
    set_draw_color(color);
    bte_set_dest(page * RA8876_PAGE_SIZE, RA8876_WIDTH, x, y);
    bte_set_size(width, height);
    bte_start(RA8876_ROP_S, 0x0C);
}

void ra8876_bte_batch_start(uint8_t page, uint16_t width, uint16_t height) {
    write_reg32(0xA7, page * RA8876_PAGE_SIZE);
    ra8876_write_reg16(0xAB, RA8876_WIDTH);
    bte_set_size(width, height);
    ra8876_write_reg(0x92, 0x25);
    ra8876_write_reg(0x91, (RA8876_ROP_S & 0xF0) | 0x0C);
}

void ra8876_bte_batch_fill(uint16_t x, uint16_t y, uint16_t color) {
    ra8876_write_reg16(0xAD, x);
    ra8876_write_reg16(0xAF, y);
    set_draw_color(color);
    ra8876_write_reg(0x90, 0x10);
    while (ra8876_read_reg(0x90) & 0x10);
}

void ra8876_bte_write(uint8_t page, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height,
                      const uint16_t *data) {
    ra8876_wait_task_busy();
    bte_set_dest(page * RA8876_PAGE_SIZE, RA8876_WIDTH, x, y);
    bte_set_size(width, height);
    ra8876_write_reg(0x92, 0x25);
    ra8876_write_reg(0x91, (RA8876_ROP_S & 0xF0) | 0x00);
    ra8876_write_reg(0x90, 0x10);
    ra8876_write_cmd(0x04);

    while (ra8876_read_status() & 0x80);
    ra8876_write_data_burst((const uint8_t *)data, width * height * 2);

    while (ra8876_read_reg(0x90) & 0x10);
}

void ra8876_bte_expand(uint8_t page, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height,
                       const uint8_t *bitmap, uint16_t fg, uint16_t bg) {
    ra8876_wait_task_busy();
    set_draw_color(fg);
    ra8876_set_bg_color(bg);
    bte_set_dest(page * RA8876_PAGE_SIZE, RA8876_WIDTH, x, y);
    bte_set_size(width, height);
    ra8876_write_reg(0x92, 0x25);
    ra8876_write_reg(0x91, (RA8876_ROP_S & 0xF0) | 0x08);
    ra8876_write_reg(0x90, 0x10);
    ra8876_write_cmd(0x04);

    size_t row_bytes = (width + 7) / 8;
    while (ra8876_read_status() & 0x80);
    ra8876_write_data_burst(bitmap, row_bytes * height);

    while (ra8876_read_reg(0x90) & 0x10);
}

void ra8876_cursor_show(bool blink) {
    g_reg3C = 0x02 | (blink ? 0x01 : 0x00);
    ra8876_write_reg(0x3C, g_reg3C);
}

void ra8876_cursor_hide(void) {
    g_reg3C &= ~0x03;
    ra8876_write_reg(0x3C, g_reg3C);
}

void ra8876_cursor_size(uint8_t width, uint8_t height) {
    ra8876_write_reg(0x3E, width - 1);
    ra8876_write_reg(0x3F, height - 1);
}

void ra8876_cursor_blink_rate(uint8_t frames) {
    ra8876_write_reg(0x3D, frames);
}

void ra8876_invert_area(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    ra8876_wait_task_busy();
    uint32_t addr = g_canvas_addr;
    bte_set_source0(addr, RA8876_WIDTH, x, y);
    bte_set_dest(addr, RA8876_WIDTH, x, y);
    bte_set_size(w, h);
    bte_start(RA8876_ROP_NOT_S, 0x02);
}

void ra8876_pip1_enable(uint8_t src_page, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    g_reg10 &= ~0x10;
    ra8876_write_reg(0x10, g_reg10);
    write_reg32(0x2E, src_page * RA8876_PAGE_SIZE);
    ra8876_write_reg16(0x32, RA8876_WIDTH);
    ra8876_write_reg16(0x2A, x);
    ra8876_write_reg16(0x2C, y);
    ra8876_write_reg16(0x34, 0);
    ra8876_write_reg16(0x36, 0);
    ra8876_write_reg16(0x38, w);
    ra8876_write_reg16(0x3A, h);
    g_reg10 |= 0x80;
    ra8876_write_reg(0x10, g_reg10);
}

void ra8876_pip1_move(uint16_t x, uint16_t y) {
    g_reg10 &= ~0x10;
    ra8876_write_reg(0x10, g_reg10);
    ra8876_write_reg16(0x2A, x);
    ra8876_write_reg16(0x2C, y);
}

void ra8876_pip1_disable(void) {
    g_reg10 &= ~0x80;
    ra8876_write_reg(0x10, g_reg10);
}

void ra8876_pip2_enable(uint8_t src_page, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    g_reg10 |= 0x10;
    ra8876_write_reg(0x10, g_reg10);
    write_reg32(0x2E, src_page * RA8876_PAGE_SIZE);
    ra8876_write_reg16(0x32, RA8876_WIDTH);
    ra8876_write_reg16(0x2A, x);
    ra8876_write_reg16(0x2C, y);
    ra8876_write_reg16(0x34, 0);
    ra8876_write_reg16(0x36, 0);
    ra8876_write_reg16(0x38, w);
    ra8876_write_reg16(0x3A, h);
    g_reg10 |= 0x40;
    ra8876_write_reg(0x10, g_reg10);
}

void ra8876_pip2_move(uint16_t x, uint16_t y) {
    g_reg10 |= 0x10;
    ra8876_write_reg(0x10, g_reg10);
    ra8876_write_reg16(0x2A, x);
    ra8876_write_reg16(0x2C, y);
}

void ra8876_pip2_disable(void) {
    g_reg10 &= ~0x40;
    ra8876_write_reg(0x10, g_reg10);
}

#define CGRAM_ADDR (RA8876_PAGE_SIZE * 10)

void ra8876_cgram_init(void) {
    write_reg32(0xDB, CGRAM_ADDR);
}

void ra8876_cgram_upload_font(const uint8_t *data, uint8_t first_char, uint8_t num_chars, uint8_t font_height) {
    uint32_t bytes_per_char;
    switch (font_height) {
        case 24: bytes_per_char = 48; break;
        case 32: bytes_per_char = 64; break;
        default: bytes_per_char = 16; break;
    }

    uint32_t addr = CGRAM_ADDR + first_char * bytes_per_char;
    uint32_t total = num_chars * bytes_per_char;

    ra8876_wait_task_busy();
    ra8876_write_reg(0x5E, 0x04);
    write_reg32(0x5F, addr);
    ra8876_write_cmd(0x04);

    while (ra8876_read_status() & 0x80);
    ra8876_write_data_burst(data, total);

    ra8876_wait_write_fifo_empty();
    ra8876_wait_task_busy();
    ra8876_write_reg(0x5E, 0x01);
    ra8876_write_cmd(0xFF);
}

void ra8876_select_cgram_font(uint8_t size) {
    g_text.font_size = size;
    switch (size) {
        case RA8876_FONT_24: g_text.char_width = 12; g_text.char_height = 24; break;
        case RA8876_FONT_32: g_text.char_width = 16; g_text.char_height = 32; break;
        default: g_text.char_width = 8; g_text.char_height = 16; break;
    }
    g_text.cols = RA8876_WIDTH / (g_text.char_width * g_text.scale_x);
    g_text.rows = RA8876_HEIGHT / (g_text.char_height * g_text.scale_y);
    write_reg32(0xDB, CGRAM_ADDR);
    ra8876_write_reg(0xCC, 0x80 | ((size & 0x03) << 4));
}

void ra8876_put_cgram_string(const char *str) {
    ra8876_set_text_mode();
    ra8876_write_cmd(0x04);
    while (*str) {
        ra8876_wait_write_fifo();
        ra8876_write_data(0x00);
        ra8876_wait_write_fifo();
        ra8876_write_data(*str++);
    }
    ra8876_set_graphics_mode();
}
