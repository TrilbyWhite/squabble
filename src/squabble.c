
#include "squabble.h"

static void board_init(const char *);
static void board_free();
static void init_all();
static void free_all();

unsigned short int _difficulty[][4] = {
	/*        7    6   5   4 */
	 [0] = {103, 101, 43, 13 }, /* moronic */
	 [1] = {101,  97, 43, 13 },
	 [2] = { 97,  89, 41, 13 },
	 [3] = { 89,  83, 41, 13 },
	 [4] = { 83,  79, 37, 11 },
	 [5] = { 79,  73, 37, 11 },
	 [6] = { 73,  71, 31, 11 },
	 [7] = { 71,  67, 31, 11 },
	 [8] = { 67,  61, 29,  7 },
	 [9] = { 61,  59, 29,  7 },
	[10] = { 59,  53, 23,  7 },
	[11] = { 53,  47, 23,  7 },
	[12] = { 47,  43, 17,  5 },
	[13] = { 43,  41, 17,  5 },
	[14] = { 41,  37, 13,  5 }, /* balanced */
	[15] = { 37,  31, 13,  5 },
	[16] = { 31,  29, 11,  3 },
	[17] = { 29,  23, 11,  3 },
	[18] = { 23,  19,  7,  3 },
	[19] = { 19,  17,  7,  3 },
	[20] = { 17,  13,  5,  2 },
	[21] = { 13,  11,  5,  2 },
	[22] = { 11,  7,   3,  2 },
	[23] = {  7,  5,   3,  2 },
	[24] = {  5,  3,   2,  1 },
	[25] = {  3,  2,   2,  1 },
	[26] = {  2,  1,   1,  1 },
	[27] = {  1,  1,   1,  1 }, /* perfect */
};

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
	difficulty = _difficulty[0];
	init_all();
	main_loop();
	free_all();
	return 0;
}



