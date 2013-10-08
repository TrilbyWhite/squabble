
#include "squabble.h"

static void board_init(const char *);
static void board_free();
static void init_all();
static void free_all();

void die(const char *fmt, ...) {
	fprintf(stderr,"%s\n",fmt);
	exit(1);
}

void board_init(const char *fname) {
	FILE *brd;
	brd = fopen(fname,"r");
	if (!brd) brd = fopen(RESOURCE_DIR "bonus","r");
	if (!brd) die("unable to open board layout");
	int i,j;
	for (j = 0; j < 15; j++) {
		for (i = 0; i < 15; i++) {
			if (fscanf(brd," %hu,%hu",
					&board[i][j].tile_bonus,&board[i][j].word_bonus) != 2)
				die("bad board layout file format");
		}
		fscanf(brd,"\n");
	}
	fclose(brd);
}

void board_free() {
}

void init_all() {
	score_player = score_comp = 0;
	board_init(NULL);
	xcairo_init();
	ai_init(NULL);
	tiles_init(NULL);
	draw_init(NULL);
}

void free_all() {
	draw_free();
	tiles_free();
	xcairo_free();
	ai_free();
	board_free();
}

int main(int argc, const char **argv) {
	logger = stderr;
	init_all();
	main_loop();
	free_all();
	return 0;
}



