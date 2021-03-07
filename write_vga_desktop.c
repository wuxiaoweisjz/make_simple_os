#define  COL8_000000  0
#define  COL8_FF0000  1
#define  COL8_00FF00  2
#define  COL8_FFFF00  3
#define  COL8_0000FF  4
#define  COL8_FF00FF  5
#define  COL8_00FFFF  6
#define  COL8_FFFFFF  7
#define  COL8_C6C6C6  8
#define  COL8_840000  9
#define  COL8_008400  10
#define  COL8_848400  11
#define  COL8_000084  12
#define  COL8_840084  13
#define  COL8_008484  14
#define  COL8_848484  15

#define  PORT_KEYDAT  0x0060
#define  PIC_OCW2     0x20
#define PIC1_OCW2 0xA0

void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out(int port, int data);
int  io_load_eflags(void);
void io_store_eflags(int eflags);
void show_char(void);
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram,int xsize,  unsigned char c, int x, int y,
int x0, int y0);

struct  BOOTINFO {
    char* vgaRam;
    short screenX, screenY;
};


void initBootInfo(struct BOOTINFO *pBootInfo);

static char fontA[16] = {0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
    0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
};

extern char systemFont[16];

void showFont8(char *vram, int xsize, int x, int y, char c, char* font);

void showString(char* vram, int xsize, int x, int y, char color, unsigned char *s );

void putblock(char* vram, int vxsize, int pxsize,
int pysize, int px0, int py0, char* buf, int bxsize);

void init_mouse_cursor(char* mouse, char bc);
void intHandlerFromC(char* esp);

static char mcursor[256];
static struct BOOTINFO bootInfo;
static char keyval[5] = {'0', 'X', 0, 0, 0};
struct FIFO8 {
    unsigned char* buf;
    int p, q, size, free, flags;
};
static struct FIFO8 keyinfo;
static struct FIFO8 mouseinfo;

static char keybuf[32];
static char mosuebuf[128];


struct MouseDec {
    unsigned char buf[3], phase;
    int x, y, btn;
};
static struct MouseDec mdec;
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char* buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

char   charToHexVal(char c);
char*  charToHexStr(unsigned char c);

void init_keyboard(void);
void enable_mouse(struct MouseDec *mdec);

void show_mouse_info(); 
int mouse_decode(struct MouseDec *mdec, unsigned char dat);
static int mx = 0, my = 0;
static int xsize =0, ysize = 0;
void CMain(void) {
    initBootInfo(&bootInfo);
    char*vram = bootInfo.vgaRam;
    xsize = bootInfo.screenX, ysize = bootInfo.screenY;

    fifo8_init(&keyinfo, 32, keybuf);
    fifo8_init(&mouseinfo, 128, mosuebuf);
    init_palette();
    init_keyboard();
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize-1, ysize-29);
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize-28, xsize-1, ysize-28);
    boxfill8(vram, xsize, COL8_FFFFFF, 0, ysize-27, xsize-1, ysize-27);
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize-26, xsize-1, ysize-1);
    
    boxfill8(vram, xsize, COL8_FFFFFF, 3, ysize-24, 59, ysize-24);
    boxfill8(vram, xsize, COL8_FFFFFF, 2, ysize-24, 2, ysize-4);
    boxfill8(vram, xsize, COL8_848484, 3, ysize-4,  59, ysize-4);
    boxfill8(vram, xsize, COL8_848484, 59, ysize-23, 59, ysize-5);
    boxfill8(vram, xsize, COL8_000000, 2, ysize-3, 59, ysize-3);
    boxfill8(vram, xsize, COL8_000000, 60, ysize-24, 60, ysize-3);

    boxfill8(vram, xsize, COL8_848484, xsize-47, ysize-24, xsize-4, ysize-24);
    boxfill8(vram, xsize, COL8_848484, xsize-47, ysize-23, xsize-47, ysize-4);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize-47, ysize-3, xsize-4, ysize-3);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize-3,  ysize-24, xsize-3, ysize-3);
 


    mx = (xsize-16)/2;
    my = (ysize-28-16)/2;
    init_mouse_cursor(mcursor, COL8_008484);
    putblock(vram, xsize, 16, 16, mx,my, mcursor, 16);
    io_sti();
    enable_mouse(&mdec);
    int data  = 0;
    for(;;) {
       io_cli();
       if(fifo8_status(&keyinfo) + fifo8_status(&mouseinfo) == 0) {
           io_stihlt();
       } else if (fifo8_status(&keyinfo) != 0){
           io_sti();
           data = fifo8_get(&keyinfo);
           char* pstr = charToHexStr(data);
           static int showPos = 0; 
           showString(vram, xsize, showPos, 0, COL8_FFFFFF, pstr);
           showPos += 32;
       } else if (fifo8_status(&mouseinfo) !=0) {
           show_mouse_info();
       }

    }

}

void eraseMosue(char* vram) {
    boxfill8(vram, xsize, COL8_008484, mx, my, mx+15, my+15);
}
void drawMouse(char* vram) {
    putblock(vram, xsize, 16, 16, mx, my, mcursor, 16);
}

void computeMousePosition(struct MouseDec* mdec) {
    mx += mdec->x;
    my += mdec->y;
    if (mx < 0) {
        mx = 0;
    }
    if (my < 0) {
        my = 0;
    }
    if (mx > xsize -16) {
        mx = xsize - 16;
    }
    if (my > ysize - 16) {
        my = ysize -16;
    }
}
void show_mouse_info() {
    char *vram = bootInfo.vgaRam;
    unsigned char data = 0;

    io_sti();
    data = fifo8_get(&mouseinfo);
    if (mouse_decode(&mdec, data) != 0) {
        eraseMosue(vram);
        computeMousePosition(&mdec);
        drawMouse(vram);
    }
}

void initBootInfo(struct BOOTINFO *pBootInfo) {
    pBootInfo->vgaRam = (char*)0xa0000;
    pBootInfo->screenX = 320;
    pBootInfo->screenY = 200;
}

void showString(char* vram, int xsize, int x, int y, char color, unsigned char *s ) {
    for (; *s != 0x00; s++) {
       showFont8(vram, xsize, x, y,color, systemFont+ *s * 16);
       x += 8;
    }
}

void init_palette(void) {
    static  unsigned char table_rgb[16 *3] = {
        0x00,  0x00,  0x00,
        0xff,  0x00,  0x00,
        0x00,  0xff,  0x00,
        0xff,  0xff,  0x00,
        0x00,  0x00,  0xff,
        0xff,  0x00,  0xff,
        0x00,  0xff,  0xff,
        0xff,  0xff,  0xff,
        0xc6,  0xc6,  0xc6,
        0x84,  0x00,  0x00,
        0x00,  0x84,  0x00,
        0x84,  0x84,  0x00,
        0x00,  0x00,  0x84,
        0x84,  0x00,  0x84,
        0x00,  0x84,  0x84,
        0x84,  0x84,  0x84,
    };
 
    set_palette(0, 15, table_rgb);
    return;
}

void set_palette(int start,  int end,  unsigned char* rgb) {
    int i, eflags;
    eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8,  start);  //set  palette number
    for (i = start; i <=end; i++ ) {
        io_out8(0x03c9,  rgb[0] / 4);
        io_out8(0x03c9,  rgb[1] / 4);
        io_out8(0x03c9,  rgb[2] / 4);
 
        rgb += 3;
    }

    io_store_eflags(eflags);
    return;
}

void boxfill8(unsigned char* vram, int xsize, unsigned char c, 
int x0, int y0, int x1, int y1) {
    int  x, y;
    for (y = y0; y <= y1; y++)
     for (x = x0; x <= x1; x++) {
         vram[y * xsize + x] = c;
     }

}

void showFont8(char *vram, int xsize, int x, int y, char c, char* font) {
    int i;
    char d;

    for (i = 0; i < 16; i++) {
        d = font[i]; 
        if ((d & 0x80) != 0) {vram[(y+i)*xsize + x + 0] = c;}
        if ((d & 0x40) != 0) {vram[(y+i)*xsize + x + 1] = c;}
        if ((d & 0x20) != 0) {vram[(y+i)*xsize + x + 2] = c;} 
        if ((d & 0x10) != 0) {vram[(y+i)*xsize + x + 3] = c;}
        if ((d & 0x08) != 0) {vram[(y+i)*xsize + x + 4] = c;}
        if ((d & 0x04) != 0) {vram[(y+i)*xsize + x + 5] = c;}
        if ((d & 0x02) != 0) {vram[(y+i)*xsize + x + 6] = c;}
        if ((d & 0x01) != 0) {vram[(y+i)*xsize + x + 7] = c;}
    }

}

void init_mouse_cursor(char* mouse, char bc) {
    static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};

      int x, y;
      for (y = 0; y < 16; y++) {
          for (x = 0; x < 16; x++) {
             if (cursor[y][x] == '*') {
                 mouse[y*16 + x] = COL8_000000;
             }
             if (cursor[y][x] == 'O') {
                mouse[y*16 + x] = COL8_FFFFFF;
             }
             if (cursor[y][x] == '.') {
                 mouse[y*16 + x] = bc;
             }
          }
      }
}

void putblock(char* vram, int vxsize, int pxsize,
int pysize, int px0, int py0, char* buf, int bxsize) {
    int x, y;
    for (y = 0; y < pysize; y++)
      for (x = 0; x < pxsize; x++) {
          vram[(py0+y) * vxsize + (px0+x)] = buf[y * bxsize + x];
      }
}


void intHandlerFromC(char* esp) {
    char*vram = bootInfo.vgaRam;
    int xsize = bootInfo.screenX, ysize = bootInfo.screenY;
    io_out8(PIC_OCW2, 0x21);
    unsigned char data = 0;
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&keyinfo, data);
    return;
}

char   charToHexVal(char c) {
    if (c >= 10) {
        return 'A' + c - 10;
    } 

    return '0' + c;
}

char*  charToHexStr(unsigned char c) {
    int i = 0;
    char mod = c % 16;
    keyval[3] = charToHexVal(mod);
    c = c / 16;
    keyval[2] = charToHexVal(c);
  
    return keyval;
}

#define port_keydat 0x0060
#define port_keysta 0x0064
#define port_keycmd 0x0064
#define keysta_send_notready 0x02
#define keycmd_write_mode 0x60 
#define kbc_mode 0x47
void wait_kbc_sendready() {
    for(;;) {
        if ((io_in8(port_keysta) & keysta_send_notready) == 0) {
            break;
        }
    }
}
void init_keyboard(void) {
    wait_kbc_sendready();
    io_out8(port_keycmd, keycmd_write_mode);
    wait_kbc_sendready();
    io_out8(port_keydat, kbc_mode);
    return;
}
#define keycmd_sendto_mosue 0xd4
#define mousecmd_enable  0xf4

void enable_mouse(struct MouseDec * mdec) {
    wait_kbc_sendready();
    io_out8(port_keycmd, keycmd_sendto_mosue);
    wait_kbc_sendready();
    io_out8(port_keydat, mousecmd_enable);

    mdec->phase = 0;
    return;
}
void intHandlerForMouse(char* esp) {
    unsigned char data;
    io_out8(PIC1_OCW2,0x20);
    io_out8(PIC_OCW2,0x20);

    data = io_in8(port_keydat);
    fifo8_put(&mouseinfo, data);
}

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf){
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->p = 0;
    fifo->q = 0;
    return;
}
#define flags_overturn 0x0001
int fifo8_put(struct FIFO8 *fifo, unsigned char data) {
    if (fifo->free ==0) {
        fifo->flags |= flags_overturn;
        return -1;
    }
    fifo->buf[fifo->p] = data;
    fifo->p++;
    if (fifo->p == fifo->size) {
        fifo->p = 0;
    }
    fifo->free--;
    return 0;
}
int fifo8_get(struct FIFO8 *fifo) {
    int data;
    if (fifo->free == fifo->size) {
        return -1;
    }
    data = fifo->buf[fifo->q];
    fifo->q++;
    if (fifo->q == fifo->size) {
        fifo->q = 0;
    }
    fifo->free++;
    return data;
}
int fifo8_status(struct FIFO8 *fifo) {
    return fifo->size - fifo->free;
}
int mouse_decode(struct MouseDec *mde, unsigned char dat) {
    if (mde->phase == 0) {
        if (dat == 0xfa) {
            mde->phase = 1;
        }
        return 0;
    }
    
    if (mde->phase == 1) {
        if ((dat & 0xc8) == 0x08) {
            mde->buf[0] = dat;
            mde->phase = 2;
        }
        return 0;
    }

    if (mde->phase == 2) {
        mde->buf[1] = dat;
        mde->phase = 3;
        return 0;
    }

    if (mde->phase == 3) {
        mde->buf[2] = dat;
        mde->phase = 1;
        mde->btn=mde->buf[0] & 0x07;
        mde->x = mde->buf[1];
        mde->y = mde->buf[2];
        if ((mde->buf[0] & 0x10) != 0) {
            mde->x |= 0xffffff00;
        }
        if ((mde->buf[0] & 0x20) != 0) {
            mde->y |= 0xffffff00;
        }

        mde->y = -mde->y;
    }
    return -1;
}
