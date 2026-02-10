#ifndef RA8876_H
#define RA8876_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "hardware/spi.h"

#define RA8876_SDRAM_SIZE   (16 * 1024 * 1024)
#define RA8876_BURST_SIZE   20

typedef enum {
    RA8876_SRR          = 0x00,
    RA8876_CCR          = 0x01,
    RA8876_MACR         = 0x02,
    RA8876_ICR          = 0x03,
    RA8876_MRWDP        = 0x04,
    RA8876_PPLLC1       = 0x05,
    RA8876_PPLLC2       = 0x06,
    RA8876_MPLLC1       = 0x07,
    RA8876_MPLLC2       = 0x08,
    RA8876_SPLLC1       = 0x09,
    RA8876_SPLLC2       = 0x0A,
    RA8876_INTEN        = 0x0B,
    RA8876_INTF         = 0x0C,
    RA8876_MPWCTR       = 0x10,
    RA8876_PIPCDEP      = 0x11,
    RA8876_DPCR         = 0x12,
    RA8876_PCSR         = 0x13,
    RA8876_HDWR         = 0x14,
    RA8876_HDWFTR       = 0x15,
    RA8876_HNDR         = 0x16,
    RA8876_HNDFTR       = 0x17,
    RA8876_HSTR         = 0x18,
    RA8876_HPWR         = 0x19,
    RA8876_VDHR         = 0x1A,
    RA8876_VNDR         = 0x1C,
    RA8876_VSTR         = 0x1E,
    RA8876_VPWR         = 0x1F,
    RA8876_MISA         = 0x20,
    RA8876_MIW          = 0x24,
    RA8876_MWULX        = 0x26,
    RA8876_MWULY        = 0x28,
    RA8876_PWDULX       = 0x2A,
    RA8876_PWDULY       = 0x2C,
    RA8876_PISA         = 0x2E,
    RA8876_PIW          = 0x32,
    RA8876_PWIULX       = 0x34,
    RA8876_PWIULY       = 0x36,
    RA8876_PWW          = 0x38,
    RA8876_PWH          = 0x3A,
    RA8876_GTCCR        = 0x3C,
    RA8876_BTCR         = 0x3D,
    RA8876_CURHS        = 0x3E,
    RA8876_CURVS        = 0x3F,
    RA8876_GCHP         = 0x40,
    RA8876_GCVP         = 0x42,
    RA8876_GCC0         = 0x44,
    RA8876_GCC1         = 0x45,
    RA8876_CVSSA        = 0x50,
    RA8876_CVS_IMWTH    = 0x54,
    RA8876_AWUL_X       = 0x56,
    RA8876_AWUL_Y       = 0x58,
    RA8876_AW_WTH       = 0x5A,
    RA8876_AW_HT        = 0x5C,
    RA8876_AW_COLOR     = 0x5E,
    RA8876_CURH         = 0x5F,
    RA8876_CURV         = 0x61,
    RA8876_F_CURX       = 0x63,
    RA8876_F_CURY       = 0x65,
    RA8876_DCR0         = 0x67,
    RA8876_DLHSR        = 0x68,
    RA8876_DLVSR        = 0x6A,
    RA8876_DLHER        = 0x6C,
    RA8876_DLVER        = 0x6E,
    RA8876_DTPH         = 0x70,
    RA8876_DTPV         = 0x72,
    RA8876_DCR1         = 0x76,
    RA8876_ELL_A        = 0x77,
    RA8876_ELL_B        = 0x79,
    RA8876_DEHR         = 0x7B,
    RA8876_DEVR         = 0x7D,
    RA8876_PSCLR        = 0x84,
    RA8876_PMUXR        = 0x85,
    RA8876_PCFGR        = 0x86,
    RA8876_TCMPB0       = 0x88,
    RA8876_TCNTB0       = 0x8A,
    RA8876_TCMPB1       = 0x8C,
    RA8876_TCNTB1       = 0x8E,
    RA8876_BTE_CTRL0    = 0x90,
    RA8876_BTE_CTRL1    = 0x91,
    RA8876_BTE_COLR     = 0x92,
    RA8876_S0_STR       = 0x93,
    RA8876_S0_WTH       = 0x97,
    RA8876_S0_X         = 0x99,
    RA8876_S0_Y         = 0x9B,
    RA8876_S1_STR       = 0x9D,
    RA8876_S1_WTH       = 0xA1,
    RA8876_S1_X         = 0xA3,
    RA8876_S1_Y         = 0xA5,
    RA8876_DT_STR       = 0xA7,
    RA8876_DT_WTH       = 0xAB,
    RA8876_DT_X         = 0xAD,
    RA8876_DT_Y         = 0xAF,
    RA8876_BTE_WTH      = 0xB1,
    RA8876_BTE_HIG      = 0xB3,
    RA8876_APB_CTRL     = 0xB5,
    RA8876_CCR0         = 0xCC,
    RA8876_CCR1         = 0xCD,
    RA8876_GTFNT_SEL    = 0xCE,
    RA8876_GTFNT_CR     = 0xCF,
    RA8876_FLDR         = 0xD0,
    RA8876_F2FSSR       = 0xD1,
    RA8876_FGCR         = 0xD2,
    RA8876_FGCG         = 0xD3,
    RA8876_FGCB         = 0xD4,
    RA8876_BGCR         = 0xD5,
    RA8876_BGCG         = 0xD6,
    RA8876_BGCB         = 0xD7,
    RA8876_CGRAM_STR    = 0xDB,
    RA8876_PMU          = 0xDF,
    RA8876_SDRAR        = 0xE0,
    RA8876_SDRMD        = 0xE1,
    RA8876_SDR_REF_ITVL = 0xE2,
    RA8876_SDRCR        = 0xE4,
    RA8876_CHIP_ID      = 0xFF,
} ra8876_reg_t;

#define RA8876_BLACK        0x000000
#define RA8876_WHITE        0xFFFFFF
#define RA8876_RED          0xFF0000
#define RA8876_GREEN        0x00FF00
#define RA8876_BLUE         0x0000FF
#define RA8876_YELLOW       0xFFFF00
#define RA8876_CYAN         0x00FFFF
#define RA8876_MAGENTA      0xFF00FF
#define RA8876_GRAY         0x808080
#define RA8876_DARKGRAY     0x404040
#define RA8876_ORANGE       0xFF8000

#define RA8876_FONT_16      0
#define RA8876_FONT_24      1
#define RA8876_FONT_32      2

#define RA8876_ENC_8859_1   0x00
#define RA8876_ENC_8859_2   0x01
#define RA8876_ENC_8859_4   0x02
#define RA8876_ENC_8859_5   0x03

#define RA8876_ROP_BLACK       0x00
#define RA8876_ROP_S_AND_D     0x80
#define RA8876_ROP_S           0xC0
#define RA8876_ROP_NOT_S       0x30
#define RA8876_ROP_D           0xA0
#define RA8876_ROP_NOT_D       0x50
#define RA8876_ROP_S_XOR_D     0x60
#define RA8876_ROP_S_OR_D      0xE0
#define RA8876_ROP_WHITE       0xF0

#define RA8876_DIR_LR_TB       0
#define RA8876_DIR_RL_TB       1
#define RA8876_DIR_TB_LR       2
#define RA8876_DIR_BT_LR       3

#define RA8876_CURVE_BL        0x00
#define RA8876_CURVE_UL        0x01
#define RA8876_CURVE_UR        0x02
#define RA8876_CURVE_BR        0x03

typedef struct {
    spi_inst_t *spi;
    uint8_t pin_miso;
    uint8_t pin_cs;
    uint8_t pin_sck;
    uint8_t pin_mosi;
    uint32_t spi_speed;

    uint16_t width;
    uint16_t height;
    uint32_t page_size;
    uint8_t max_pages;

    uint16_t char_width;
    uint16_t char_height;

    uint32_t canvas_addr;
    uint8_t draw_page;
    uint8_t display_page;
    uint8_t num_pages;

    uint8_t reg02;
    uint8_t reg03;
    uint8_t reg10;
    uint8_t reg3C;
    uint8_t regCC;
    uint8_t regCD;

    uint8_t burst_buf[RA8876_BURST_SIZE + 1];
} ra8876_t;

bool ra8876_init(ra8876_t *dev, uint16_t width, uint16_t height);
uint8_t ra8876_get_chip_id(ra8876_t *dev);

void ra8876_write_cmd(ra8876_t *dev, ra8876_reg_t reg);
void ra8876_write_data(ra8876_t *dev, uint8_t data);
void ra8876_write_data_burst(ra8876_t *dev, const uint8_t *data, size_t len);
uint8_t ra8876_read_data(ra8876_t *dev);
uint8_t ra8876_read_status(ra8876_t *dev);
void ra8876_write_reg(ra8876_t *dev, ra8876_reg_t reg, uint8_t val);
uint8_t ra8876_read_reg(ra8876_t *dev, ra8876_reg_t reg);
void ra8876_write_reg16(ra8876_t *dev, ra8876_reg_t reg, uint16_t val);

void ra8876_wait_ready(ra8876_t *dev);
void ra8876_wait_write_fifo(ra8876_t *dev);
void ra8876_wait_task_busy(ra8876_t *dev);
void ra8876_vsync_init(ra8876_t *dev);
void ra8876_wait_vsync(ra8876_t *dev);

void ra8876_fill_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void ra8876_draw_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void ra8876_draw_line(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);
void ra8876_fill_circle(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t radius, uint32_t color);
void ra8876_draw_circle(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t radius, uint32_t color);
void ra8876_fill_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint32_t color);
void ra8876_draw_ellipse(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint32_t color);
void ra8876_fill_rounded_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint32_t color);
void ra8876_draw_rounded_rect(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint32_t color);
void ra8876_fill_triangle(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
void ra8876_draw_triangle(ra8876_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
void ra8876_fill_screen(ra8876_t *dev, uint32_t color);
void ra8876_draw_curve(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint8_t quadrant, uint32_t color);
void ra8876_fill_curve(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t rx, uint16_t ry, uint8_t quadrant, uint32_t color);

void ra8876_select_internal_font(ra8876_t *dev, uint8_t size, uint8_t encoding);
void ra8876_set_text_colors(ra8876_t *dev, uint32_t fg, uint32_t bg);
void ra8876_set_text_transparent(ra8876_t *dev, bool transparent);
void ra8876_set_text_cursor(ra8876_t *dev, uint16_t x, uint16_t y);
void ra8876_set_text_scale(ra8876_t *dev, uint8_t sx, uint8_t sy);
void ra8876_set_text_rotation(ra8876_t *dev, bool rotate90);
void ra8876_set_text_spacing(ra8876_t *dev, uint8_t line_gap, uint8_t char_gap);
void ra8876_put_string(ra8876_t *dev, const char *s);
void ra8876_print(ra8876_t *dev, uint16_t x, uint16_t y, uint32_t color, const char *s);
void ra8876_printf(ra8876_t *dev, uint16_t x, uint16_t y, uint32_t color, const char *fmt, ...);

void ra8876_set_canvas_addr(ra8876_t *dev, uint32_t addr);
void ra8876_set_canvas_page(ra8876_t *dev, uint8_t page);
void ra8876_set_canvas(ra8876_t *dev, uint32_t addr, uint16_t width);
void ra8876_set_display_addr(ra8876_t *dev, uint32_t addr);
void ra8876_set_active_window(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void ra8876_scroll(ra8876_t *dev, uint16_t x, uint16_t y);
void ra8876_set_write_direction(ra8876_t *dev, uint8_t dir);
void ra8876_set_display_flip(ra8876_t *dev, bool flip);
void ra8876_set_gamma(ra8876_t *dev, bool enable);

void ra8876_buffer_init(ra8876_t *dev, uint8_t num_pages);
void ra8876_swap_buffers(ra8876_t *dev);
void ra8876_buffer_disable(ra8876_t *dev);
uint8_t ra8876_get_draw_page(ra8876_t *dev);

void ra8876_bte_copy(ra8876_t *dev, uint32_t src_addr, uint16_t src_x, uint16_t src_y,
                     uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                     uint16_t width, uint16_t height, uint8_t rop);

void ra8876_bte_copy_chroma(ra8876_t *dev, uint32_t src_addr, uint16_t src_x, uint16_t src_y,
                            uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                            uint16_t width, uint16_t height, uint32_t chroma);

void ra8876_bte_blend(ra8876_t *dev, uint32_t s0_addr, uint16_t s0_x, uint16_t s0_y,
                      uint32_t s1_addr, uint16_t s1_x, uint16_t s1_y,
                      uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                      uint16_t width, uint16_t height, uint8_t alpha);

void ra8876_bte_solid_fill(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height, uint32_t color);

void ra8876_bte_batch_start(ra8876_t *dev, uint32_t addr, uint16_t width, uint16_t height);
void ra8876_bte_batch_fill(ra8876_t *dev, uint16_t x, uint16_t y, uint32_t color);

void ra8876_bte_write(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height, const uint8_t *data);

void ra8876_bte_write_chroma(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                             uint16_t width, uint16_t height,
                             const uint8_t *data, uint32_t chroma);

void ra8876_bte_expand(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height,
                       const uint8_t *bitmap, uint32_t fg, uint32_t bg);

void ra8876_bte_expand_chroma(ra8876_t *dev, uint32_t addr, uint16_t x, uint16_t y,
                              uint16_t width, uint16_t height,
                              const uint8_t *bitmap, uint32_t fg);

void ra8876_bte_mem_expand(ra8876_t *dev, uint32_t src_addr, uint16_t src_x, uint16_t src_y,
                           uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                           uint16_t width, uint16_t height, uint32_t fg, uint32_t bg);

void ra8876_bte_write_opacity(ra8876_t *dev, uint32_t s1_addr, uint16_t s1_x, uint16_t s1_y,
                              uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                              uint16_t width, uint16_t height,
                              const uint8_t *data, uint8_t alpha);

void ra8876_bte_pattern_fill(ra8876_t *dev, uint32_t pattern_addr,
                             uint32_t dst_addr, uint16_t dst_x, uint16_t dst_y,
                             uint16_t width, uint16_t height,
                             bool pattern_16x16, uint8_t rop);

void ra8876_set_backlight(ra8876_t *dev, uint8_t brightness);
void ra8876_display_on(ra8876_t *dev);
void ra8876_display_off(ra8876_t *dev);
void ra8876_standby(ra8876_t *dev);
void ra8876_wake_standby(ra8876_t *dev);

void ra8876_cursor_show(ra8876_t *dev, bool blink);
void ra8876_cursor_hide(ra8876_t *dev);
void ra8876_cursor_size(ra8876_t *dev, uint8_t width, uint8_t height);
void ra8876_cursor_blink_rate(ra8876_t *dev, uint8_t frames);

void ra8876_gcursor_enable(ra8876_t *dev, uint8_t set);
void ra8876_gcursor_disable(ra8876_t *dev);
void ra8876_gcursor_move(ra8876_t *dev, uint16_t x, uint16_t y);
void ra8876_gcursor_colors(ra8876_t *dev, uint8_t color0, uint8_t color1);
void ra8876_gcursor_load(ra8876_t *dev, uint8_t set, const uint8_t *data);

void ra8876_invert_area(ra8876_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void ra8876_pip1_enable(ra8876_t *dev, uint32_t src_addr, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void ra8876_pip1_move(ra8876_t *dev, uint16_t x, uint16_t y);
void ra8876_pip1_disable(ra8876_t *dev);
void ra8876_pip2_enable(ra8876_t *dev, uint32_t src_addr, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void ra8876_pip2_move(ra8876_t *dev, uint16_t x, uint16_t y);
void ra8876_pip2_disable(ra8876_t *dev);

void ra8876_cgram_init(ra8876_t *dev);
void ra8876_cgram_upload_font(ra8876_t *dev, const uint8_t *data, uint8_t first_char, uint8_t num_chars, uint8_t font_height);
void ra8876_cgram_upload_inv_font(ra8876_t *dev, const uint8_t *data, uint8_t first_char, uint8_t num_chars, uint8_t font_height);
void ra8876_select_cgram_font(ra8876_t *dev, uint8_t size);
void ra8876_put_cgram_string_off(ra8876_t *dev, const char *str, uint8_t offset);
void ra8876_put_cgram_string(ra8876_t *dev, const char *str);

void ra8876_set_fg_color(ra8876_t *dev, uint32_t color);
void ra8876_set_bg_color(ra8876_t *dev, uint32_t color);
void ra8876_set_text_mode(ra8876_t *dev);
void ra8876_set_graphics_mode(ra8876_t *dev);

static inline uint32_t ra8876_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static inline uint32_t ra8876_page_addr(ra8876_t *dev, uint8_t page) {
    return (uint32_t)page * dev->page_size;
}

static inline uint8_t ra8876_pack_rgb332(uint8_t r, uint8_t g, uint8_t b) {
    return (r & 0xE0) | ((g >> 3) & 0x1C) | (b >> 6);
}

static inline uint8_t ra8876_scale_x(ra8876_t *dev) {
    return ((dev->regCD >> 2) & 3) + 1;
}

static inline uint8_t ra8876_scale_y(ra8876_t *dev) {
    return (dev->regCD & 3) + 1;
}

#endif
