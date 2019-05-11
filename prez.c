#include "neslib.h"

#define FLOOR_Y 25 // Vertical position of the floor
#define LOGO_X 14 // Horizontal position of the logo
#define LOGO_Y 1 // Vertical position of the logo
#define START_X 1 // initial X position of text cursor
#define START_Y 7 // initial Y position of text cursor
#define LINE_BREAK 2 // Number of line breaks
#define WAIT_CHAR 2 // Number of frames to wait between each character
#define WAIT_LINE 2 // Number of frames to wait between each newline
#define WAIT_FAST 1 // Fast scroll

// Text of our pages
#define NUM_PAGES 4
static char* TEXT[NUM_PAGES] = {
    // --------------------->
    "Riviera DEV presente\n\n"
    "Programmation NES\n"
    "en langage C",

    // ----------------------->
    "Venez apprendre a coder\n"
    "sur NES facilement !\n\n"
    "grace a NESLIB !!!",

    // ----------------------->
    "Rendez-vous le\n"
    "vendredi 17 mai\n"
    "a 13h30",

    // ----------------------->
    "\n\n"
    "     THANK YOU MARIO!\n\n"
    "  BUT OUR PRINCESS IS IN\n"
    "  ANOTHER CASTLE!",
  };

// Background and foreground palettes
const unsigned char pal[32] = {
    // Background
    0x31, 0x27, 0x0D, 0x11, // Logo & text
    0x30, 0x0d, 0x27, 0x16, // Bricks
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    // Sprites
    0x31, 0x30, 0x29, 0x09, // Twin dragon
    0x30, 0x17, 0x27, 0x16, // Mario
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

// Variables
static unsigned char page; // current page displayed
static unsigned char index; // current index in text
static unsigned char c; // last character read
static unsigned char x; // offset in buffer 'background' of the beginning of the line
static unsigned char y; // offset in buffer 'background' of next char to write
static unsigned char ps; // current pad state
static char char_buf[4] = {
    0, // MSB
    0, // LSB
    0, // character
    NT_UPD_EOF
}; // we'll use this buffer to update a single background sprite

static char line_buf[28] = {
    0, // MSB
    0, // LSB
    24, // Line length
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    NT_UPD_EOF
};

// Clear all lines of text in the background buffer
void clear_all() {
#define CLEAR_LINE(line) \
    line_buf[0] = MSB(NTADR_A(START_X,START_Y+line*LINE_BREAK))|NT_UPD_HORZ; \
    line_buf[1] = LSB(NTADR_A(START_X,START_Y+line*LINE_BREAK)); \
    delay(1);

    set_vram_update(line_buf);
    CLEAR_LINE(0);
    CLEAR_LINE(1);
    CLEAR_LINE(2);
    CLEAR_LINE(3);
    CLEAR_LINE(4);
    CLEAR_LINE(5);
    CLEAR_LINE(6);
    CLEAR_LINE(7);
    CLEAR_LINE(8);
    set_vram_update(NULL);

#undef CLEAR_LINE
}

void display_next_character() {
    c = TEXT[page][index];
    ps = pad_poll(0);
    switch (c) {
    case 0:
        break;

    case '\n':
        y += LINE_BREAK;
        x = START_X;
        delay((ps & (PAD_A | PAD_B)) ? WAIT_FAST : WAIT_LINE);
        break;

    default:
        char_buf[0] = MSB(NTADR_A(x,y));
        char_buf[1] = LSB(NTADR_A(x,y));
        char_buf[2] = c;
        set_vram_update(char_buf);
        // FALL THROUGH

    case ' ':
        x++;
        delay((ps & (PAD_A | PAD_B)) ? WAIT_FAST : WAIT_CHAR);
    }
    set_vram_update(NULL);
    index++;
}

void display_page() {
    // Init page state
    index = 0;
    x = START_X;
    y = START_Y;

    do {
        display_next_character();
    } while (c);

    do {
        ppu_wait_frame();
        ps = pad_trigger(0); // trigger because an explicit press is needed

        if (ps & PAD_A && (page < (NUM_PAGES - 1))) {
            page++;
            break;
        } else if ((ps & PAD_B) && page) {
            page--;
            break;
        }
    } while (1);
}

void display_logo(void) {
    // 4 lines of background sprites
    static unsigned char logo[37] = {
        0xc0, 0xc1,
        0xd0, 0xd1, 0xd2,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
    };
    vram_adr(NTADR_A(LOGO_X,LOGO_Y));
    vram_write(logo, 2);
    vram_adr(NTADR_A(LOGO_X,LOGO_Y+1));
    vram_write(logo + 2, 3);
    vram_adr(NTADR_A(LOGO_X,LOGO_Y+2));
    vram_write(logo + 5, 16);
    vram_adr(NTADR_A(LOGO_X,LOGO_Y+3));
    vram_write(logo + 21, 16);
}

void display_floor(void) {
    static unsigned char floor[32] = {
        0x02, 0x03, 0x02, 0x03, 0x02, 0x03, 0x02, 0x03, 0x02, 0x03, 0x02, 0x03, 0x02, 0x03, 0x02, 0x03,
        0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13, 0x12, 0x13,
    };

    // Background palettes
    vram_adr(NAMETABLE_B-16);
    vram_fill((1 << 6) | (1 << 4) | (1 << 2) | (1 << 0), 16);

    vram_adr(NTADR_A(0,FLOOR_Y));
    vram_write(floor, 16);
    vram_adr(NTADR_A(16,FLOOR_Y));
    vram_write(floor, 16);
    vram_adr(NTADR_A(0,FLOOR_Y+1));
    vram_write(floor+16, 16);
    vram_adr(NTADR_A(16,FLOOR_Y+1));
    vram_write(floor+16, 16);
    vram_adr(NTADR_A(0,FLOOR_Y+2));
    vram_write(floor, 16);
    vram_adr(NTADR_A(16,FLOOR_Y+2));
    vram_write(floor, 16);
    vram_adr(NTADR_A(0,FLOOR_Y+3));
    vram_write(floor+16, 16);
    vram_adr(NTADR_A(16,FLOOR_Y+3));
    vram_write(floor+16, 16);
}

void main(void) {
    pal_all(pal);
    display_logo();
    display_floor();

    bank_bg(0);
    bank_spr(1);
    ppu_on_all();

    music_play(0);

    page = 0;
    do {
        display_page();
        clear_all();
    } while (1);
}
