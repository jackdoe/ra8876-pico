#include "ra8876.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "pico/stdlib.h"

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

void ra8876_write_cmd(ra8876_t *dev, ra8876_reg_t reg) {
    uint8_t buf[2] = {0x00, reg};
    cs_select(dev);
    spi_write_blocking(dev->spi, buf, 2);
    cs_deselect(dev);
}

void ra8876_write_data(ra8876_t *dev, uint8_t data) {
    while (ra8876_read_status(dev) & 0x80);
    uint8_t buf[2] = {0x80, data};
    cs_select(dev);
    spi_write_blocking(dev->spi, buf, 2);
    cs_deselect(dev);
}

void ra8876_write_data_burst(ra8876_t *dev, const uint8_t *data, size_t len) {
    dev->burst_buf[0] = 0x80;
    size_t offset = 0;
    while (offset < len) {
        while ((ra8876_read_status(dev) & 0x40) == 0)
            tight_loop_contents();
        size_t chunk = len - offset;
        if (chunk > RA8876_BURST_SIZE) chunk = RA8876_BURST_SIZE;
        memcpy(&dev->burst_buf[1], &data[offset], chunk);
        cs_select(dev);
        spi_write_blocking(dev->spi, dev->burst_buf, chunk + 1);
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

static inline void cmd(ra8876_t *dev, ra8876_reg_t reg) {
    uint8_t buf[2] = {0x00, (uint8_t)reg};
    cs_select(dev);
    spi_write_blocking(dev->spi, buf, 2);
    cs_deselect(dev);
}

static inline void dat(ra8876_t *dev, uint8_t d) {
    uint8_t buf[2] = {0x80, d};
    cs_select(dev);
    spi_write_blocking(dev->spi, buf, 2);
    cs_deselect(dev);
}

static inline void reg_wr(ra8876_t *dev, ra8876_reg_t reg, uint8_t val) {
    cmd(dev, reg);
    dat(dev, val);
}

static inline void reg_wr16(ra8876_t *dev, ra8876_reg_t reg, uint16_t val) {
    reg_wr(dev, reg, val & 0xFF);
    reg_wr(dev, (ra8876_reg_t)(reg + 1), val >> 8);
}

static inline void reg_wr32(ra8876_t *dev, ra8876_reg_t reg, uint32_t val) {
    reg_wr(dev, reg, val & 0xFF);
    reg_wr(dev, (ra8876_reg_t)(reg + 1), (val >> 8) & 0xFF);
    reg_wr(dev, (ra8876_reg_t)(reg + 2), (val >> 16) & 0xFF);
    reg_wr(dev, (ra8876_reg_t)(reg + 3), (val >> 24) & 0xFF);
}

void ra8876_write_reg(ra8876_t *dev, ra8876_reg_t reg, uint8_t val) {
    reg_wr(dev, reg, val);
}

uint8_t ra8876_read_reg(ra8876_t *dev, ra8876_reg_t reg) {
    cmd(dev, reg);
    return ra8876_read_data(dev);
}

void ra8876_write_reg16(ra8876_t *dev, ra8876_reg_t reg, uint16_t val) {
    reg_wr16(dev, reg, val);
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

static void set_draw_color(ra8876_t *dev, uint32_t color) {
    reg_wr(dev, RA8876_FGCR, (color >> 16) & 0xFF);
    reg_wr(dev, RA8876_FGCG, (color >> 8) & 0xFF);
    reg_wr(dev, RA8876_FGCB, color & 0xFF);
}

static void set_bg_draw_color(ra8876_t *dev, uint32_t color) {
    reg_wr(dev, RA8876_BGCR, (color >> 16) & 0xFF);
    reg_wr(dev, RA8876_BGCG, (color >> 8) & 0xFF);
    reg_wr(dev, RA8876_BGCB, color & 0xFF);
}

static void set_two_points(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    reg_wr16(dev, RA8876_DLHSR, x0);
    reg_wr16(dev, RA8876_DLVSR, y0);
    reg_wr16(dev, RA8876_DLHER, x1);
    reg_wr16(dev, RA8876_DLVER, y1);
}

static void set_three_points(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    reg_wr16(dev, RA8876_DLHSR, x0);
    reg_wr16(dev, RA8876_DLVSR, y0);
    reg_wr16(dev, RA8876_DLHER, x1);
    reg_wr16(dev, RA8876_DLVER, y1);
    reg_wr16(dev, RA8876_DTPH, x2);
    reg_wr16(dev, RA8876_DTPV, y2);
}

static void draw_and_wait(ra8876_t *dev, ra8876_reg_t reg, uint8_t val) {
    reg_wr(dev, reg, val);
    ra8876_wait_task_busy(dev);
}

static void soft_reset(ra8876_t *dev) {
    reg_wr(dev, RA8876_SRR, 0x01);
    sleep_ms(100);
    reg_wr(dev, RA8876_SRR, 0x00);
    sleep_ms(100);
}

uint8_t ra8876_get_chip_id(ra8876_t *dev) {
    return ra8876_read_reg(dev, RA8876_CHIP_ID);
}

static bool init_pll(ra8876_t *dev) {
    reg_wr(dev, RA8876_PPLLC1, 0x02);
    reg_wr(dev, RA8876_PPLLC2, 0x09);
    reg_wr(dev, RA8876_MPLLC1, 0x02);
    reg_wr(dev, RA8876_MPLLC2, 0x18);
    reg_wr(dev, RA8876_SPLLC1, 0x02);
    reg_wr(dev, RA8876_SPLLC2, 0x18);
    reg_wr(dev, RA8876_CCR, 0x00);
    sleep_ms(2);
    reg_wr(dev, RA8876_CCR, 0x80);
    sleep_ms(2);
    return (ra8876_read_reg(dev, RA8876_CCR) & 0x80) != 0;
}

static bool init_sdram(ra8876_t *dev) {
    reg_wr(dev, RA8876_SDRAR, 0x29);
    reg_wr(dev, RA8876_SDRMD, 0x03);
    reg_wr16(dev, RA8876_SDR_REF_ITVL, 1875);
    reg_wr(dev, RA8876_SDRCR, 0x01);
    for (int i = 0; i < 1000; i++) {
        sleep_ms(1);
        if (ra8876_read_status(dev) & 0x04) return true;
    }
    return false;
}

static void init_display(ra8876_t *dev) {
    uint8_t ccr = ra8876_read_reg(dev, RA8876_CCR) & 0xE6;
    reg_wr(dev, RA8876_CCR, ccr);

    dev->reg02 = 0x00;
    reg_wr(dev, RA8876_MACR, dev->reg02);
    dev->reg03 = 0x00;
    reg_wr(dev, RA8876_ICR, dev->reg03);

    reg_wr(dev, RA8876_DPCR, 0x80);
    reg_wr(dev, RA8876_PCSR, 0xC0);

    reg_wr(dev, RA8876_HDWR, (dev->width / 8) - 1);
    reg_wr(dev, RA8876_HDWFTR, dev->width % 8);
    reg_wr(dev, RA8876_HNDR, 19);
    reg_wr(dev, RA8876_HNDFTR, 0);
    reg_wr(dev, RA8876_HSTR, 19);
    reg_wr(dev, RA8876_HPWR, 8);

    reg_wr16(dev, RA8876_VDHR, dev->height - 1);
    reg_wr16(dev, RA8876_VNDR, 22);
    reg_wr(dev, RA8876_VSTR, 11);
    reg_wr(dev, RA8876_VPWR, 9);

    dev->reg10 = 0x00;
    reg_wr(dev, RA8876_MPWCTR, dev->reg10);
    reg_wr(dev, RA8876_PIPCDEP, 0x00);

    reg_wr32(dev, RA8876_MISA, 0);
    reg_wr16(dev, RA8876_MIW, dev->width);
    reg_wr16(dev, RA8876_MWULX, 0);
    reg_wr16(dev, RA8876_MWULY, 0);

    reg_wr(dev, RA8876_AW_COLOR, 0x00);

    reg_wr32(dev, RA8876_CVSSA, 0);
    reg_wr16(dev, RA8876_CVS_IMWTH, dev->width);
    reg_wr16(dev, RA8876_AWUL_X, 0);
    reg_wr16(dev, RA8876_AWUL_Y, 0);
    reg_wr16(dev, RA8876_AW_WTH, dev->width);
    reg_wr16(dev, RA8876_AW_HT, dev->height);

}

static void init_pwm(ra8876_t *dev) {
    reg_wr(dev, RA8876_PSCLR, 0x03);
    reg_wr(dev, RA8876_PMUXR, 0x02);
    reg_wr16(dev, RA8876_TCNTB0, 0x00FF);
    reg_wr16(dev, RA8876_TCMPB0, 0x00FF);
    reg_wr(dev, RA8876_PCFGR, 0x03);
}

void ra8876_set_backlight(ra8876_t *dev, uint8_t brightness) {
    reg_wr16(dev, RA8876_TCMPB0, brightness);
}

void ra8876_display_on(ra8876_t *dev) {
    reg_wr(dev, RA8876_DPCR, ra8876_read_reg(dev, RA8876_DPCR) | 0x40);
}

void ra8876_display_off(ra8876_t *dev) {
    reg_wr(dev, RA8876_DPCR, ra8876_read_reg(dev, RA8876_DPCR) & ~0x40);
}

void ra8876_standby(ra8876_t *dev) {
    ra8876_wait_task_busy(dev);
    reg_wr(dev, RA8876_PMU, 0x01);
    reg_wr(dev, RA8876_PMU, 0x81);
}

void ra8876_wake_standby(ra8876_t *dev) {
    reg_wr(dev, RA8876_PMU, 0x01);
    while ((ra8876_read_status(dev) & 0x02) != 0);
    init_pwm(dev);
    ra8876_set_backlight(dev, 255);
}

bool ra8876_init(ra8876_t *dev, uint16_t width, uint16_t height) {
    dev->width = width;
    dev->height = height;
    dev->page_size = (uint32_t)width * height;
    dev->max_pages = RA8876_SDRAM_SIZE / dev->page_size;

    dev->char_width = 8;
    dev->char_height = 16;
    dev->canvas_addr = 0;
    dev->draw_page = 0;
    dev->display_page = 0;
    dev->num_pages = 1;
    dev->reg02 = 0x00;
    dev->reg03 = 0x00;
    dev->reg3C = 0x00;
    dev->regCC = 0x00;
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
    ra8876_fill_screen(dev, RA8876_BLACK);
    ra8876_wait_task_busy(dev);
    ra8876_display_on(dev);
    ra8876_vsync_init(dev);
    init_pwm(dev);
    ra8876_set_backlight(dev, 255);
    ra8876_select_internal_font(dev, RA8876_FONT_16, RA8876_ENC_8859_1);
    ra8876_set_text_colors(dev, RA8876_WHITE, RA8876_BLACK);
    return true;
}

void ra8876_fill_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
    set_two_points(dev, x, y, x + w - 1, y + h - 1);
    set_draw_color(dev, color);
    draw_and_wait(dev, RA8876_DCR1, 0xE0);
}

void ra8876_draw_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
    set_two_points(dev, x, y, x + w - 1, y + h - 1);
    set_draw_color(dev, color);
    draw_and_wait(dev, RA8876_DCR1, 0xA0);
}

void ra8876_draw_line(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color) {
    set_two_points(dev, x0, y0, x1, y1);
    set_draw_color(dev, color);
    draw_and_wait(dev, RA8876_DCR0, 0x80);
}

static void draw_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint32_t color, uint8_t cmd) {
    reg_wr16(dev, RA8876_DEHR, x);
    reg_wr16(dev, RA8876_DEVR, y);
    reg_wr16(dev, RA8876_ELL_A, rx);
    reg_wr16(dev, RA8876_ELL_B, ry);
    set_draw_color(dev, color);
    draw_and_wait(dev, RA8876_DCR1, cmd);
}

void ra8876_fill_circle(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t r, uint32_t color) {
    draw_ellipse(dev, x, y, r, r, color, 0xC0);
}

void ra8876_draw_circle(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t r, uint32_t color) {
    draw_ellipse(dev, x, y, r, r, color, 0x80);
}

void ra8876_fill_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint32_t color) {
    draw_ellipse(dev, x, y, rx, ry, color, 0xC0);
}

void ra8876_draw_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint32_t color) {
    draw_ellipse(dev, x, y, rx, ry, color, 0x80);
}

void ra8876_fill_rounded_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint32_t color) {
    set_two_points(dev, x, y, x + w - 1, y + h - 1);
    reg_wr16(dev, RA8876_ELL_A, r);
    reg_wr16(dev, RA8876_ELL_B, r);
    set_draw_color(dev, color);
    draw_and_wait(dev, RA8876_DCR1, 0xF0);
}

void ra8876_draw_rounded_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint32_t color) {
    set_two_points(dev, x, y, x + w - 1, y + h - 1);
    reg_wr16(dev, RA8876_ELL_A, r);
    reg_wr16(dev, RA8876_ELL_B, r);
    set_draw_color(dev, color);
    draw_and_wait(dev, RA8876_DCR1, 0xB0);
}

void ra8876_fill_triangle(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color) {
    set_three_points(dev, x0, y0, x1, y1, x2, y2);
    set_draw_color(dev, color);
    draw_and_wait(dev, RA8876_DCR0, 0xE2);
}

void ra8876_draw_triangle(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color) {
    ra8876_wait_task_busy(dev);
    set_three_points(dev, x0, y0, x1, y1, x2, y2);
    set_draw_color(dev, color);
    draw_and_wait(dev, RA8876_DCR0, 0xA2);
}

void ra8876_fill_screen(ra8876_t *dev, uint32_t color) {
    ra8876_fill_rect(dev, 0, 0, dev->width, dev->height, color);
}

void ra8876_draw_curve(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint8_t quadrant, uint32_t color) {
    draw_ellipse(dev, x, y, rx, ry, color, 0x90 | (quadrant & 0x03));
}

void ra8876_fill_curve(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint8_t quadrant, uint32_t color) {
    draw_ellipse(dev, x, y, rx, ry, color, 0xD0 | (quadrant & 0x03));
}

void ra8876_set_text_mode(ra8876_t *dev) {
    ra8876_wait_task_busy(dev);
    dev->reg03 |= 0x04;
    reg_wr(dev, RA8876_ICR, dev->reg03);
}

void ra8876_set_graphics_mode(ra8876_t *dev) {
    ra8876_wait_task_busy(dev);
    dev->reg03 &= ~0x04;
    reg_wr(dev, RA8876_ICR, dev->reg03);
}

void ra8876_select_internal_font(ra8876_t *dev, uint8_t size, uint8_t encoding) {
    switch (size) {
        case RA8876_FONT_24: dev->char_width = 12; dev->char_height = 24; break;
        case RA8876_FONT_32: dev->char_width = 16; dev->char_height = 32; break;
        default: dev->char_width = 8; dev->char_height = 16; break;
    }
    dev->regCC = ((size & 0x03) << 4) | (encoding & 0x03);
    reg_wr(dev, RA8876_CCR0, dev->regCC);
    reg_wr(dev, RA8876_CCR1, dev->regCD);
}

void ra8876_set_fg_color(ra8876_t *dev, uint32_t color) {
    set_draw_color(dev, color);
}

void ra8876_set_bg_color(ra8876_t *dev, uint32_t color) {
    set_bg_draw_color(dev, color);
}

void ra8876_set_text_colors(ra8876_t *dev, uint32_t fg, uint32_t bg) {
    ra8876_set_fg_color(dev, fg);
    ra8876_set_bg_color(dev, bg);
}

void ra8876_set_text_transparent(ra8876_t *dev, bool transparent) {
    if (transparent)
        dev->regCD |= 0x40;
    else
        dev->regCD &= ~0x40;
    reg_wr(dev, RA8876_CCR1, dev->regCD);
}

void ra8876_set_text_scale(ra8876_t *dev, uint8_t sx, uint8_t sy) {
    if (sx < 1) sx = 1; if (sx > 4) sx = 4;
    if (sy < 1) sy = 1; if (sy > 4) sy = 4;
    dev->regCD = (dev->regCD & 0xF0) | ((sx - 1) << 2) | (sy - 1);
    reg_wr(dev, RA8876_CCR1, dev->regCD);
}

void ra8876_set_text_rotation(ra8876_t *dev, bool rotate90) {
    if (rotate90)
        dev->regCD |= 0x10;
    else
        dev->regCD &= ~0x10;
    reg_wr(dev, RA8876_CCR1, dev->regCD);
}

void ra8876_set_text_spacing(ra8876_t *dev, uint8_t line_gap, uint8_t char_gap) {
    reg_wr(dev, RA8876_FLDR, line_gap & 0x1F);
    reg_wr(dev, RA8876_F2FSSR, char_gap & 0x3F);
}

void ra8876_set_text_cursor(ra8876_t *dev, uint16_t x, uint16_t y) {
    reg_wr16(dev, RA8876_F_CURX, x);
    reg_wr16(dev, RA8876_F_CURY, y);
}

void ra8876_put_string(ra8876_t *dev, const char *s) {
    size_t len = strlen(s);
    if (len == 0) return;
    ra8876_set_text_mode(dev);
    cmd(dev, RA8876_MRWDP);
    ra8876_write_data_burst(dev, (const uint8_t *)s, len);
    ra8876_set_graphics_mode(dev);
}

void ra8876_print(ra8876_t *dev, uint16_t x, uint16_t y, uint32_t color, const char *s) {
    ra8876_set_fg_color(dev, color);
    ra8876_set_text_cursor(dev, x, y);
    ra8876_put_string(dev, s);
}

void ra8876_printf(ra8876_t *dev, uint16_t x, uint16_t y, uint32_t color, const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ra8876_print(dev, x, y, color, buf);
}

void ra8876_set_canvas_addr(ra8876_t *dev, uint32_t addr) {
    dev->canvas_addr = addr;
    reg_wr32(dev, RA8876_CVSSA, addr);
    ra8876_wait_ready(dev);
}

void ra8876_set_canvas_page(ra8876_t *dev, uint8_t page) {
    if (page >= dev->max_pages) page = dev->max_pages - 1;
    ra8876_set_canvas_addr(dev, ra8876_page_addr(dev, page));
}

void ra8876_set_canvas(ra8876_t *dev, uint32_t addr, uint16_t width) {
    dev->canvas_addr = addr;
    reg_wr32(dev, RA8876_CVSSA, addr);
    reg_wr16(dev, RA8876_CVS_IMWTH, width);
    ra8876_wait_ready(dev);
}

void ra8876_set_display_addr(ra8876_t *dev, uint32_t addr) {
    reg_wr32(dev, RA8876_MISA, addr);
    ra8876_wait_ready(dev);
}

void ra8876_set_active_window(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    reg_wr16(dev, RA8876_AWUL_X, x);
    reg_wr16(dev, RA8876_AWUL_Y, y);
    reg_wr16(dev, RA8876_AW_WTH, w);
    reg_wr16(dev, RA8876_AW_HT, h);
}

void ra8876_scroll(ra8876_t *dev, uint16_t x, uint16_t y) {
    reg_wr16(dev, RA8876_MWULX, x);
    reg_wr16(dev, RA8876_MWULY, y);
}

void ra8876_set_write_direction(ra8876_t *dev, uint8_t dir) {
    dev->reg02 = (dev->reg02 & ~0x06) | ((dir & 0x03) << 1);
    reg_wr(dev, RA8876_MACR, dev->reg02);
}

void ra8876_set_display_flip(ra8876_t *dev, bool flip) {
    uint8_t val = ra8876_read_reg(dev, RA8876_DPCR);
    if (flip)
        val |= 0x08;
    else
        val &= ~0x08;
    reg_wr(dev, RA8876_DPCR, val);
}

void ra8876_set_gamma(ra8876_t *dev, bool enable) {
    if (enable)
        dev->reg3C |= 0x80;
    else
        dev->reg3C &= ~0x80;
    reg_wr(dev, RA8876_GTCCR, dev->reg3C);
}

void ra8876_buffer_init(ra8876_t *dev, uint8_t num_pages) {
    if (num_pages < 1) num_pages = 1;
    if (num_pages > dev->max_pages) num_pages = dev->max_pages;

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
    reg_wr(dev, RA8876_INTEN, ra8876_read_reg(dev, RA8876_INTEN) | 0x10);
    reg_wr(dev, RA8876_INTF, 0x10);
}

void ra8876_wait_vsync(ra8876_t *dev) {
    while ((ra8876_read_reg(dev, RA8876_INTF) & 0x10) == 0);
    reg_wr(dev, RA8876_INTF, 0x10);
}

void ra8876_swap_buffers(ra8876_t *dev) {
    if (dev->num_pages < 2) return;

    ra8876_wait_task_busy(dev);
    ra8876_wait_vsync(dev);

    dev->display_page = dev->draw_page;
    dev->draw_page = (dev->draw_page + 1) % dev->num_pages;

    ra8876_set_display_addr(dev, ra8876_page_addr(dev, dev->display_page));
    ra8876_set_canvas_addr(dev, ra8876_page_addr(dev, dev->draw_page));
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
    reg_wr32(dev, RA8876_S0_STR, addr);
    reg_wr16(dev, RA8876_S0_WTH, width);
    reg_wr16(dev, RA8876_S0_X, x);
    reg_wr16(dev, RA8876_S0_Y, y);
}

static void bte_set_source1(ra8876_t *dev, uint32_t addr, uint16_t width, uint16_t x, uint16_t y) {
    reg_wr32(dev, RA8876_S1_STR, addr);
    reg_wr16(dev, RA8876_S1_WTH, width);
    reg_wr16(dev, RA8876_S1_X, x);
    reg_wr16(dev, RA8876_S1_Y, y);
}

static void bte_set_dest(ra8876_t *dev, uint32_t addr, uint16_t width, uint16_t x, uint16_t y) {
    reg_wr32(dev, RA8876_DT_STR, addr);
    reg_wr16(dev, RA8876_DT_WTH, width);
    reg_wr16(dev, RA8876_DT_X, x);
    reg_wr16(dev, RA8876_DT_Y, y);
}

static void bte_set_size(ra8876_t *dev, uint16_t width, uint16_t height) {
    reg_wr16(dev, RA8876_BTE_WTH, width);
    reg_wr16(dev, RA8876_BTE_HIG, height);
}

static void bte_start(ra8876_t *dev, uint8_t rop, uint8_t op) {
    reg_wr(dev, RA8876_BTE_COLR, 0x00);
    reg_wr(dev, RA8876_BTE_CTRL1, (rop & 0xF0) | (op & 0x0F));
    reg_wr(dev, RA8876_BTE_CTRL0, 0x10);
    while (ra8876_read_reg(dev, RA8876_BTE_CTRL0) & 0x10);
}

static void bte_start_pattern(ra8876_t *dev, uint8_t rop, uint8_t op, bool p16) {
    reg_wr(dev, RA8876_BTE_COLR, 0x00);
    reg_wr(dev, RA8876_BTE_CTRL1, (rop & 0xF0) | (op & 0x0F));
    reg_wr(dev, RA8876_BTE_CTRL0, p16 ? 0x11 : 0x10);
    while (ra8876_read_reg(dev, RA8876_BTE_CTRL0) & 0x10);
}

static void bte_start_mpu(ra8876_t *dev, uint8_t rop, uint8_t op) {
    reg_wr(dev, RA8876_BTE_COLR, 0x00);
    reg_wr(dev, RA8876_BTE_CTRL1, (rop & 0xF0) | (op & 0x0F));
    reg_wr(dev, RA8876_BTE_CTRL0, 0x10);
    cmd(dev, RA8876_MRWDP);
}

static void bte_wait_mpu(ra8876_t *dev) {
    while (ra8876_read_reg(dev, RA8876_BTE_CTRL0) & 0x10);
}

void ra8876_bte_copy(ra8876_t *dev, uint32_t src_addr, uint16_t src_x, uint16_t src_y,
                     uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                     uint16_t width, uint16_t height, uint8_t rop) {
    ra8876_wait_task_busy(dev);
    bte_set_source0(dev, src_addr, dev->width, src_x, src_y);
    bte_set_dest(dev, dst_addr, dev->width, dst_x, dst_y);
    bte_set_size(dev, width, height);
    bte_start(dev, rop, 0x02);
}

void ra8876_bte_copy_chroma(ra8876_t *dev, uint32_t src_addr, uint16_t src_x, uint16_t src_y,
                            uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                            uint16_t width, uint16_t height, uint32_t chroma) {
    ra8876_wait_task_busy(dev);
    set_bg_draw_color(dev, chroma);
    bte_set_source0(dev, src_addr, dev->width, src_x, src_y);
    bte_set_dest(dev, dst_addr, dev->width, dst_x, dst_y);
    bte_set_size(dev, width, height);
    bte_start(dev, RA8876_ROP_S, 0x05);
}

void ra8876_bte_blend(ra8876_t *dev, uint32_t s0_addr, uint16_t s0_x, uint16_t s0_y,
                      uint32_t s1_addr, uint16_t s1_x, uint16_t s1_y,
                      uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                      uint16_t width, uint16_t height, uint8_t alpha) {
    ra8876_wait_task_busy(dev);
    bte_set_source0(dev, s0_addr, dev->width, s0_x, s0_y);
    bte_set_source1(dev, s1_addr, dev->width, s1_x, s1_y);
    bte_set_dest(dev, dst_addr, dev->width, dst_x, dst_y);
    bte_set_size(dev, width, height);
    reg_wr(dev, RA8876_APB_CTRL, alpha >> 3);
    bte_start(dev, RA8876_ROP_S, 0x0A);
}

void ra8876_bte_solid_fill(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height, uint32_t color) {
    ra8876_wait_task_busy(dev);
    set_draw_color(dev, color);
    bte_set_dest(dev, addr, dev->width, x, y);
    bte_set_size(dev, width, height);
    bte_start(dev, RA8876_ROP_S, 0x0C);
}

void ra8876_bte_batch_start(ra8876_t *dev, uint32_t addr, uint16_t width, uint16_t height) {
    reg_wr32(dev, RA8876_DT_STR, addr);
    reg_wr16(dev, RA8876_DT_WTH, dev->width);
    bte_set_size(dev, width, height);
    reg_wr(dev, RA8876_BTE_COLR, 0x00);
    reg_wr(dev, RA8876_BTE_CTRL1, (RA8876_ROP_S & 0xF0) | 0x0C);
}

void ra8876_bte_batch_fill(ra8876_t *dev, uint16_t x, uint16_t y, uint32_t color) {
    reg_wr16(dev, RA8876_DT_X, x);
    reg_wr16(dev, RA8876_DT_Y, y);
    set_draw_color(dev, color);
    reg_wr(dev, RA8876_BTE_CTRL0, 0x10);
    while (ra8876_read_reg(dev, RA8876_BTE_CTRL0) & 0x10);
}

void ra8876_bte_write(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height, const uint8_t *data) {
    ra8876_wait_task_busy(dev);
    bte_set_dest(dev, addr, dev->width, x, y);
    bte_set_size(dev, width, height);
    bte_start_mpu(dev, RA8876_ROP_S, 0x00);
    while (ra8876_read_status(dev) & 0x80);
    ra8876_write_data_burst(dev, data, (size_t)width * height );
    bte_wait_mpu(dev);
}

void ra8876_bte_write_chroma(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                             uint16_t width, uint16_t height,
                             const uint8_t *data, uint32_t chroma) {
    ra8876_wait_task_busy(dev);
    set_bg_draw_color(dev, chroma);
    bte_set_dest(dev, addr, dev->width, x, y);
    bte_set_size(dev, width, height);
    bte_start_mpu(dev, RA8876_ROP_S, 0x04);
    while (ra8876_read_status(dev) & 0x80);
    ra8876_write_data_burst(dev, data, (size_t)width * height );
    bte_wait_mpu(dev);
}

void ra8876_bte_expand(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height,
                       const uint8_t *bitmap, uint32_t fg, uint32_t bg) {
    ra8876_wait_task_busy(dev);
    set_draw_color(dev, fg);
    set_bg_draw_color(dev, bg);
    bte_set_dest(dev, addr, dev->width, x, y);
    bte_set_size(dev, width, height);
    bte_start_mpu(dev, RA8876_ROP_S, 0x08);
    size_t row_bytes = (width + 7) / 8;
    while (ra8876_read_status(dev) & 0x80);
    ra8876_write_data_burst(dev, bitmap, row_bytes * height);
    bte_wait_mpu(dev);
}

void ra8876_bte_expand_chroma(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                              uint16_t width, uint16_t height,
                              const uint8_t *bitmap, uint32_t fg) {
    ra8876_wait_task_busy(dev);
    set_draw_color(dev, fg);
    bte_set_dest(dev, addr, dev->width, x, y);
    bte_set_size(dev, width, height);
    bte_start_mpu(dev, RA8876_ROP_S, 0x09);
    size_t row_bytes = (width + 7) / 8;
    while (ra8876_read_status(dev) & 0x80);
    ra8876_write_data_burst(dev, bitmap, row_bytes * height);
    bte_wait_mpu(dev);
}

void ra8876_bte_mem_expand(ra8876_t *dev, uint32_t src_addr, uint16_t src_x, uint16_t src_y,
                           uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                           uint16_t width, uint16_t height, uint32_t fg, uint32_t bg) {
    ra8876_wait_task_busy(dev);
    set_draw_color(dev, fg);
    set_bg_draw_color(dev, bg);
    bte_set_source0(dev, src_addr, dev->width, src_x, src_y);
    bte_set_dest(dev, dst_addr, dev->width, dst_x, dst_y);
    bte_set_size(dev, width, height);
    bte_start(dev, RA8876_ROP_S, 0x0E);
}

void ra8876_bte_write_opacity(ra8876_t *dev, uint32_t s1_addr, uint16_t s1_x, uint16_t s1_y,
                              uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                              uint16_t width, uint16_t height,
                              const uint8_t *data, uint8_t alpha) {
    ra8876_wait_task_busy(dev);
    bte_set_source1(dev, s1_addr, dev->width, s1_x, s1_y);
    bte_set_dest(dev, dst_addr, dev->width, dst_x, dst_y);
    bte_set_size(dev, width, height);
    reg_wr(dev, RA8876_APB_CTRL, alpha >> 3);
    bte_start_mpu(dev, RA8876_ROP_S, 0x0B);
    while (ra8876_read_status(dev) & 0x80);
    ra8876_write_data_burst(dev, data, (size_t)width * height );
    bte_wait_mpu(dev);
}

void ra8876_bte_pattern_fill(ra8876_t *dev, uint32_t pattern_addr,
                             uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                             uint16_t width, uint16_t height,
                             bool pattern_16x16, uint8_t rop) {
    ra8876_wait_task_busy(dev);
    bte_set_source0(dev, pattern_addr, dev->width, 0, 0);
    bte_set_dest(dev, dst_addr, dev->width, dst_x, dst_y);
    bte_set_size(dev, width, height);
    bte_start_pattern(dev, rop, 0x06, pattern_16x16);
}

void ra8876_cursor_show(ra8876_t *dev, bool blink) {
    dev->reg3C = (dev->reg3C & ~0x03) | 0x02 | (blink ? 0x01 : 0x00);
    reg_wr(dev, RA8876_GTCCR, dev->reg3C);
}

void ra8876_cursor_hide(ra8876_t *dev) {
    dev->reg3C &= ~0x03;
    reg_wr(dev, RA8876_GTCCR, dev->reg3C);
}

void ra8876_cursor_size(ra8876_t *dev, uint8_t width, uint8_t height) {
    reg_wr(dev, RA8876_CURHS, width - 1);
    reg_wr(dev, RA8876_CURVS, height - 1);
}

void ra8876_cursor_blink_rate(ra8876_t *dev, uint8_t frames) {
    reg_wr(dev, RA8876_BTCR, frames);
}

void ra8876_gcursor_enable(ra8876_t *dev, uint8_t set) {
    dev->reg3C = (dev->reg3C & ~0x1C) | 0x10 | ((set & 0x03) << 2);
    reg_wr(dev, RA8876_GTCCR, dev->reg3C);
}

void ra8876_gcursor_disable(ra8876_t *dev) {
    dev->reg3C &= ~0x10;
    reg_wr(dev, RA8876_GTCCR, dev->reg3C);
}

void ra8876_gcursor_move(ra8876_t *dev, uint16_t x, uint16_t y) {
    reg_wr16(dev, RA8876_GCHP, x);
    reg_wr16(dev, RA8876_GCVP, y);
}

void ra8876_gcursor_colors(ra8876_t *dev, uint8_t color0, uint8_t color1) {
    reg_wr(dev, RA8876_GCC0, color0);
    reg_wr(dev, RA8876_GCC1, color1);
}

void ra8876_gcursor_load(ra8876_t *dev, uint8_t set, const uint8_t *data) {
    dev->reg3C = (dev->reg3C & ~0x0C) | ((set & 0x03) << 2);
    reg_wr(dev, RA8876_GTCCR, dev->reg3C);
    dev->reg03 = (dev->reg03 & ~0x03) | 0x02;
    reg_wr(dev, RA8876_ICR, dev->reg03);
    cmd(dev, RA8876_MRWDP);
    ra8876_write_data_burst(dev, data, 256);
    dev->reg03 &= ~0x03;
    reg_wr(dev, RA8876_ICR, dev->reg03);
}

void ra8876_invert_area(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    ra8876_wait_task_busy(dev);
    uint32_t addr = dev->canvas_addr;
    bte_set_source0(dev, addr, dev->width, x, y);
    bte_set_dest(dev, addr, dev->width, x, y);
    bte_set_size(dev, w, h);
    bte_start(dev, RA8876_ROP_NOT_S, 0x02);
}

void ra8876_pip1_enable(ra8876_t *dev, uint32_t src_addr, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    dev->reg10 &= ~0x10;
    reg_wr(dev, RA8876_MPWCTR, dev->reg10);
    reg_wr32(dev, RA8876_PISA, src_addr);
    reg_wr16(dev, RA8876_PIW, dev->width);
    reg_wr16(dev, RA8876_PWDULX, x);
    reg_wr16(dev, RA8876_PWDULY, y);
    reg_wr16(dev, RA8876_PWIULX, 0);
    reg_wr16(dev, RA8876_PWIULY, 0);
    reg_wr16(dev, RA8876_PWW, w);
    reg_wr16(dev, RA8876_PWH, h);
    dev->reg10 |= 0x80;
    reg_wr(dev, RA8876_MPWCTR, dev->reg10);
}

void ra8876_pip1_move(ra8876_t *dev, uint16_t x, uint16_t y) {
    dev->reg10 &= ~0x10;
    reg_wr(dev, RA8876_MPWCTR, dev->reg10);
    reg_wr16(dev, RA8876_PWDULX, x);
    reg_wr16(dev, RA8876_PWDULY, y);
}

void ra8876_pip1_disable(ra8876_t *dev) {
    dev->reg10 &= ~0x80;
    reg_wr(dev, RA8876_MPWCTR, dev->reg10);
}

void ra8876_pip2_enable(ra8876_t *dev, uint32_t src_addr, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    dev->reg10 |= 0x10;
    reg_wr(dev, RA8876_MPWCTR, dev->reg10);
    reg_wr32(dev, RA8876_PISA, src_addr);
    reg_wr16(dev, RA8876_PIW, dev->width);
    reg_wr16(dev, RA8876_PWDULX, x);
    reg_wr16(dev, RA8876_PWDULY, y);
    reg_wr16(dev, RA8876_PWIULX, 0);
    reg_wr16(dev, RA8876_PWIULY, 0);
    reg_wr16(dev, RA8876_PWW, w);
    reg_wr16(dev, RA8876_PWH, h);
    dev->reg10 |= 0x40;
    reg_wr(dev, RA8876_MPWCTR, dev->reg10);
}

void ra8876_pip2_move(ra8876_t *dev, uint16_t x, uint16_t y) {
    dev->reg10 |= 0x10;
    reg_wr(dev, RA8876_MPWCTR, dev->reg10);
    reg_wr16(dev, RA8876_PWDULX, x);
    reg_wr16(dev, RA8876_PWDULY, y);
}

void ra8876_pip2_disable(ra8876_t *dev) {
    dev->reg10 &= ~0x40;
    reg_wr(dev, RA8876_MPWCTR, dev->reg10);
}

static uint32_t cgram_addr(ra8876_t *dev) {
    (void)dev;
    return RA8876_SDRAM_SIZE - 65536;
}

void ra8876_cgram_init(ra8876_t *dev) {
    reg_wr32(dev, RA8876_CGRAM_STR, cgram_addr(dev));
}

void ra8876_cgram_upload_font(ra8876_t *dev, const uint8_t *data, uint8_t first_char, uint8_t num_chars, uint8_t font_height) {
    uint32_t bytes_per_char;
    switch (font_height) {
        case 24: bytes_per_char = 48; break;
        case 32: bytes_per_char = 64; break;
        default: bytes_per_char = 16; break;
    }

    uint32_t addr = cgram_addr(dev) + first_char * bytes_per_char;
    uint32_t total = num_chars * bytes_per_char;

    ra8876_wait_task_busy(dev);
    reg_wr(dev, RA8876_AW_COLOR, 0x04);
    reg_wr32(dev, RA8876_CURH, addr);
    cmd(dev, RA8876_MRWDP);

    while (ra8876_read_status(dev) & 0x80);
    ra8876_write_data_burst(dev, data, total);

    ra8876_wait_write_fifo_empty(dev);
    ra8876_wait_task_busy(dev);
    reg_wr(dev, RA8876_AW_COLOR, 0x00);
    cmd(dev, RA8876_CHIP_ID);
}

void ra8876_select_cgram_font(ra8876_t *dev, uint8_t size) {
    switch (size) {
        case RA8876_FONT_24: dev->char_width = 12; dev->char_height = 24; break;
        case RA8876_FONT_32: dev->char_width = 16; dev->char_height = 32; break;
        default: dev->char_width = 8; dev->char_height = 16; break;
    }
    reg_wr32(dev, RA8876_CGRAM_STR, cgram_addr(dev));
    dev->regCC = 0x80 | ((size & 0x03) << 4);
    reg_wr(dev, RA8876_CCR0, dev->regCC);
}

void ra8876_put_cgram_string(ra8876_t *dev, const char *str) {
    ra8876_set_text_mode(dev);
    cmd(dev, RA8876_MRWDP);
    uint8_t buf[64];
    while (*str) {
        size_t n = 0;
        while (*str && n < 64) {
            buf[n++] = 0x00;
            buf[n++] = *str++;
        }
        ra8876_write_data_burst(dev, buf, n);
    }
    ra8876_set_graphics_mode(dev);
}
