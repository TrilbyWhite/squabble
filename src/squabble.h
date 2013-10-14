
#ifndef __SQUABBLE_H__
#define __SQUABBLE_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/cursorfont.h>

#define STRING(s)			STRINGIFY(s)
#define STRINGIFY(s)		#s
#define RESOURCE_DIR		STRING(SHARE_DIR)

#define TILE_BOARD_TEMP	0x01
#define TILE_BOARD_PERM	0x02
#define TILE_BAG			0x03
#define TILE_PLAYER		0x04
#define TILE_OPPONENT	0x05
#define TILE_NULL			0x00

#define STAGE_NULL						0x0000
#define STAGE_FIRST						0x1000
#define STAGE_PLAYER						0x0010
#define STAGE_AI							0x0020
#define STAGE_THINKING					0x0001
#define STAGE_DONE						0x0002

#define IN_STAGE(y)		((stage & (y)) == stage)

typedef struct {
	char letter;
	unsigned short int pts;
	int x, y, flags;
} Tile;

typedef struct {
	Tile *tile;
	unsigned short int word_bonus;
	unsigned short int tile_bonus;
} Square;

int stage;
struct { int x, y, score; char word[16], down; } ai_play;
cairo_t *c;
float csx, csy;
FILE *logger;
Square board[15][15];
Tile tile_floating;
Tile *tile_target;
Tile *rack[7];
unsigned short int *difficulty;
int score_player, score_comp;
time_t timer;
Bool running, ai_running;

const char *tiles(int);

#endif /* __SQUABBLE_H__ */


