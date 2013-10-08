
#include "squabble.h"

unsigned char *letter_pts = NULL;

static Tile *tile_stack = NULL;
static int n_tiles = 0;
char ret[8];

const char *tiles(int loc) {
	if ( loc != TILE_PLAYER && loc != TILE_OPPONENT ) return;
	Tile *t;
	memset(ret,0,8);
	int i,n = 0;
	for (i = 0; i < n_tiles; i++)
		if (tile_stack[i].flags == loc) ret[n++] = tile_stack[i].letter;
	return ret;
}

void tiles_free() {
	free(tile_stack);
	tile_stack = NULL;
	free(letter_pts);
	letter_pts = NULL;
}

void tiles_get(int loc) {
	if ( loc != TILE_PLAYER && loc != TILE_OPPONENT ) return;
	Tile *t;
	int i, j, needed = 7;
	//if (loc == TILE_PLAYER) for (i = 0; i < 7 && rack[i]; i++)
	//	rack[i]->x = 250+(i*75);
	for (i = 0; i < n_tiles; i++)
		if (tile_stack[i].flags == loc) needed--;
	for (i = 0; i < n_tiles && needed; i++) {
		if (tile_stack[i].flags != TILE_BAG) continue;
		tile_stack[i].flags = loc;
		if (loc == TILE_PLAYER) {
			for (j = 0; j < 7; j++) {
				if (!rack[j]) {
					rack[j] = &tile_stack[i];
					tile_stack[i].y = 880;
					tile_stack[i].x = 350+j*75;
					break;
				}
			}
		}
		needed --;
	}
	if (loc == TILE_PLAYER) for (i = 7-needed; i < 7; i++) rack[i] = NULL;
}

void tiles_init(const char *fname) {
	FILE *ts;
	ts = fopen(fname,"r");
	if (!ts) ts = fopen(RESOURCE_DIR "tiles","r");
	if (!ts) die("unable to open tiles file");
	char ch; unsigned short int pts; int count, n = 0, n_let = 0;
	char line[64], cha[2];
	cha[1] = '\0';
	while (fgets(line,64,ts)) {
		if ((strlen(line) < 6) || line[0] == '#') continue;
		if (sscanf(line,"%c, %d, %hu",&ch,&count,&pts) != 3)
			die("bad tiles file format");
		tile_stack = (Tile *) realloc(tile_stack, (n+=count) * sizeof(Tile));
		letter_pts = (unsigned char *) realloc(letter_pts, n_let+1);
		letter_pts[n_let++] = pts;
		for (count; count; count--) {
			tile_stack[n-count].letter = ch;
			tile_stack[n-count].pts = pts;
			tile_stack[n-count].flags = TILE_BAG;
			//strcpy(tile_stack[n-count].img,"_.png");
			//tile_stack[n-count].img[0] = ch;
		}
	}
	n_tiles = n;
	/* shuffle */
	srand(time(NULL));
	int i, j, tpts;
	char tlet;
	for (i = n_tiles - 1; i; i--) {
		j = (int) (((double) i) * rand() / ((double) RAND_MAX));
		tlet = tile_stack[j].letter; tpts = tile_stack[j].pts;
		tile_stack[j].letter = tile_stack[i].letter;
		tile_stack[j].pts = tile_stack[i].pts;
		tile_stack[i].letter = tlet; tile_stack[i].pts = tpts;
	}
	memset(rack,0,7*sizeof(Tile *));
	tiles_get(TILE_PLAYER);
	tiles_get(TILE_OPPONENT);
}

Tile *tiles_loc(int loc, char c) {
	int i;
	for (i = 0; i < n_tiles; i++)
		if (tile_stack[i].flags == loc && tile_stack[i].letter == c)
			return &tile_stack[i];
	return NULL;
}

void tiles_play(int loc) {
	int i; Tile *t;
	for (i = 0; i < strlen(ai_play.word); i++) {
		if ( !(t = tiles_loc(loc,ai_play.word[i])) ) continue;
		if (ai_play.down) {
			board[ai_play.x][ai_play.y+i].tile = t;
			t->x = 250+50*ai_play.x; t->y = 100+50*(ai_play.y+i);
		}
		else {
			board[ai_play.x+i][ai_play.y].tile = t;
			t->x = 250+50*(ai_play.x+i); t->y = 100+50*ai_play.y;
		}
		t->flags = TILE_BOARD_PERM;
	}
	tiles_get(loc);
}

void tiles_rack_sort() {
	int i, j, t, x;
	for (j = 0; j < 7; j++) {
		x = 1000;
		t = -1;
		for (i = 0; i < 7 && rack[i]; i++)
			if (rack[i]->x < x && rack[i]->flags == TILE_PLAYER) {
				t = i;
				x = rack[i]->x;
			}
		if (t > -1) rack[t]->x = 1200+j;
	}
	for (i = 0; i < 7 && rack[i]; i++)
		if (rack[i]->flags == TILE_PLAYER)
			rack[i]->x = 350 + (rack[i]->x - 1200) * 75;
}

void tiles_trade(int loc) {
	Tile *t;
	int i;
	for (i = 0; i < n_tiles; i++)
		if (tile_stack[i].flags == loc) tile_stack[i].flags = TILE_BAG;
	tiles_get(loc);
}



