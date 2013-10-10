
#include "squabble.h"

#define MAX_LINE	64
#define MAX_CHAR	8

extern unsigned char *letter_pts;

static void check_n_letter(int, const char *);
static void check_string(int, int, const char *, const int);
static Bool dict_letters(int, const char *);
static Bool dict_match(const char *);
static void play_notify();
static void recurse(const char *, const char *);
static int score(const char *,int,int,int);

static char *strings = NULL;
static int nstrings = 0;
static int len;
static char **dict = NULL;
static int ndict = 0;

void ai_init(const char *fname) {
	char line[MAX_LINE];
	FILE *d;
	if (fname) d = fopen(fname,"r");
	else d = fopen(RESOURCE_DIR "dict","r");
	if (!d) die("unable to open dictionary");
	while (fgets(line,MAX_LINE,d)) {
		line[strlen(line)-1] = '\0'; /* remove newlines */
		dict = (char **) realloc(dict,(++ndict)*sizeof(char *));
		dict[ndict-1] = strdup(line);
	}
	fclose(d);
}

void *ai_threaded(void *arg) {
	const char *letters = (const char *) arg;
	int i, j, n_let;
	len = strlen(letters) + 1;
	nstrings = len - 1;
	for (i = len - 2; i; i--) nstrings *= i;
	strings = (char *) calloc(nstrings * len, sizeof(char));
	if (!strings) die("unable to allocate \"strings\" memory\n");
	nstrings = 0;
	ai_play.score = 0;
	recurse("",letters);
	char *str;
	for (n_let = strlen(letters); n_let; n_let--)
			check_n_letter(n_let,letters);
	free(strings);
	if (!ai_play.score) {
		fprintf(logger,"%d: AI trades tiles\n", time(NULL));
		tiles_trade(TILE_OPPONENT);
	}
	else {
		play_notify();
	}
	if ( stage & STAGE_AI ) {
		stage = (STAGE_AI|STAGE_DONE);
	}
	else if ( stage & STAGE_PLAYER ) {
		// display hint
	}
	return NULL;
}

void ai_find_play(const char *letters) {
	pthread_t ai_thread;
	pthread_create(&ai_thread,NULL,ai_threaded,(void *) letters);
}

void ai_free() {
	int i;
	for (i = 0; i < ndict; i++) free(dict[i]);
	free(dict);
	ndict = 0;
}


void ai_play_done() {
	int i, j, x, y, n = 0;
	for (i = 0; i < 15; i++) for (j = 0; j < 15; j++) {
		if (board[i][j].tile &&
				board[i][j].tile->flags == TILE_BOARD_TEMP)
			goto double_break;
	}
	double_break:
	x = i; y = j;
	ai_play.down = 0;
	for (j = 0; j < 15; j++)
		if (j != y && board[x][j].tile &&
				board[x][j].tile->flags == TILE_BOARD_TEMP)
			ai_play.down = 1;
	j = y;
	if (ai_play.down) {
		while (j && board[i][j-1].tile) j--;
		ai_play.x = i; ai_play.y = j;
		while (board[i][j].tile)
			ai_play.word[n++] = board[i][j++].tile->letter;
	}
	else {
		while (i && board[i-1][j].tile) i--;
		ai_play.x = i; ai_play.y = j;
		while (board[i][j].tile)
			ai_play.word[n++] = board[i++][j].tile->letter;
	}
	ai_play.word[n] = '\0';
	ai_play.score = score(ai_play.word, ai_play.x, ai_play.y, ai_play.down);
	play_notify();
	for (i = 0; i < 7 && rack[i]; i++) {
		if (rack[i]->flags == TILE_BOARD_TEMP) {
			rack[i]->flags = TILE_BOARD_PERM;
			rack[i] = NULL;
		}
	}
	tiles_get(TILE_PLAYER);
}

/******************************************************/

void check_n_letter(int n, const char *letters) {
	if (!dict_letters(n,letters)) return;
	int i = 0, j;
	int jump = (len - 1 - n ? len - 1 - n : 1);
	for (i = jump - 1; i > 0; i--) jump *= i;
	if (n > 3) jump *= difficulty[7-n];
	char *p,str[MAX_CHAR];
	if ( stage & STAGE_FIRST ) {
		for (p = strings; *p != '\0'; p += len*jump) {
			strcpy(str,p); str[n] = '\0';
			ai_play.y = 7;
			if (dict_match(str)) {
				if ( (i=score(str,7-strlen(str)/2,7,0)) > ai_play.score ) {
					ai_play.x = 7 - strlen(str)/2;
					ai_play.score = i;
					strcpy(ai_play.word,str);
				}
			}
		}
	}
	else for (p = strings; *p != '\0'; p += len*jump) {
		strcpy(str,p); str[n] = '\0';
		for (i = 0; i < 16 - n; i++) for (j = 0; j < 15; j++)
			check_string(i,j,str,0);
		for (i = 0; i < 15; i++) for (j = 0; j < 16 - n; j++)
			check_string(i,j,str,1);
	}
}

char word[16];
void check_string(int x, int y, const char *str, const int down) {
	if (board[x][y].tile) return;
	Tile *t;
	int i, c, opt_score;
	char connect = 0;
	/* find real start of word */
	if (down) while (y && board[x][y-1].tile) y--;
	else while (x && board[x-1][y].tile) x--;
	/* build "word" */
	for (i = 0, c = 0; i < 15 - (down ? y : x) && c < strlen(str); i++) {
		if ( (t = board[(down?x:x+i)][(down?y+i:y)].tile) ){
			connect = word[i] = t->letter;
		}
		else
			word[i] = str[c++];
	}
	word[i] = '\0';
	/* check for real word, then get score */
	if ( i && connect && c == strlen(str) && dict_match(word) ) {
		// TODO check "cross" words
		opt_score = score(word,x,y,down);
		if (opt_score > ai_play.score) {
			ai_play.score = opt_score;
			ai_play.x = x; ai_play.y = y;
			ai_play.down = down;
			strcpy(ai_play.word,word);
		}
	}
}

Bool dict_letters(int n, const char *letters) {
	char match[MAX_CHAR];
	int i, j, k, missed;
	for (i = 0; i < ndict; i++) {
		memset(match,0,MAX_CHAR);
		missed = 0;
		for (j = 0; j < strlen(dict[i]); j++) {
			for (k = 0; k < strlen(letters); k++) {
				if ( match[k] == 1 ) continue;
				if ( letters[k] == dict[i][j] ) {
					match[k] = 1;
					break;
				}
			}
		}
		for (k = 0; k < strlen(letters); k++) {
			if (match[k] == 0) missed++;
		}
		if (missed <= strlen(letters) - n) {
			return True;
		}
	}
	return False;
}

Bool dict_match(const char *word) {
	int i, n, ds = 0, de = ndict;
	for (i = 0; i < strlen(word); i++) {
		n = ds;
		while ( n<de && ( dict[n][i] == '\0' || word[i] > dict[n][i] ) &&
				( !i || dict[n][i-1] == dict[ds][i-1] ) ) n++;
		if (n >= de || word[i] != dict[n][i]) return False;
		ds = n;
		while ( (word[i] == dict[n][i]) ) {
			if ( word[i+1] == '\0' && dict[n][i+1] == '\0')
				return True;
			n++;
		}
		de = n;
	}
	return False;
}

void play_notify() {
	if ( stage & STAGE_PLAYER )
		fprintf(logger,"%d: Player ",time(NULL));
	else if ( stage & STAGE_AI )
		fprintf(logger,"%d: AI ",time(NULL));
	else return;
	if (dict_match(ai_play.word)) {
		fprintf(logger,"plays \"%s\" at (%d,%d) for %d points\n",
			ai_play.word, ai_play.x,ai_play.y,ai_play.score);
		if ( stage & STAGE_PLAYER ) score_player += ai_play.score;
		else score_comp += ai_play.score;
	}
	else {
		fprintf(logger,"cheats! \"%s\" at (%d,%d) for %d points\n",
			ai_play.word, ai_play.x,ai_play.y,ai_play.score);
	}
}

void recurse(const char *front, const char *back) {
	if (strlen(back) <= 1) {
		strcpy(strings+nstrings*len,front);
		strcat(strings+nstrings*len,back);
		nstrings++;
		return;
	}
	int i, j;
	char nfront[MAX_CHAR],nback[MAX_CHAR];
	for (i = 0; i < strlen(back); i++) {
		strcpy(nfront,front);
		nfront[strlen(nfront) + 1] = '\0';
		nfront[strlen(nfront)] = back[i];
		for (j = 0; j < strlen(back); j++) {
			if (j < i) nback[j] = back[j];
			else nback[j] = back[j+1];
		}
		recurse(nfront,nback);
	}
}

int score(const char *word, int x, int y, int down) {
	int i, pts = 0, word_multi = 1, multi_score = 0;
	for (i = 0; i < strlen(word); i++) {
		if (board[x][y].tile && board[x][y].tile->flags == TILE_BOARD_PERM) {
			pts += letter_pts[word[i]-97];
		}
		else {
			pts += letter_pts[word[i]-97] * board[x][y].tile_bonus;
			word_multi *= board[x][y].word_bonus;
if (down) {
if ( (x > 0 && board[x-1][y].tile) || (x < 14 && board[x+1][y].tile) ) {
	int xx, mw=1, sub=0, crossn = 0;
	char cross[15];
	for (xx = x; xx && board[xx-1][y].tile ; xx--);
	for (xx = xx; xx < 15 && (board[xx][y].tile || xx==x); xx++) {
		if (board[xx][y].tile->flags == TILE_BOARD_PERM) {
			sub += letter_pts[board[xx][y].tile->letter - 97];
			cross[crossn++] = board[xx][y].tile->letter;
		}
		else {
			sub += letter_pts[word[y] - 97] * board[xx][y].tile_bonus;
			mw *= board[xx][y].word_bonus;
			cross[crossn++] = word[y];
		}
	}
	cross[crossn] = '\0';
	if (dict_match(cross)) {
		multi_score += sub*mw;
		fprintf(logger,"%s at (%d,%d) -> Crossword \"%s\"\n",
				word,x,y,cross,sub*mw);
	}
	else {
		return 0;
	}
}
}
else {
if ( (y > 0 && board[x][y-1].tile) || (y < 14 && board[x][y+1].tile) ) {
	int yy, mw=1, sub=0, crossn = 0;
	char cross[15];
	for (yy = y; yy && board[x][yy-1].tile; yy--);
	for (yy = yy; yy < 15 && (board[x][yy].tile || yy==y); yy++) {
		if (board[x][yy].tile->flags == TILE_BOARD_PERM) {
			sub += letter_pts[board[x][yy].tile->letter - 97];
			cross[crossn++] = board[x][yy].tile->letter;
		}
		else {
			sub += letter_pts[word[x] - 97] * board[x][yy].tile_bonus;
			mw = board[x][yy].word_bonus;
			cross[crossn++] = word[x];
		}
	}
	cross[crossn] = '\0';
	if (dict_match(cross)) {
		multi_score += sub*mw;
		fprintf(logger,"%s at (%d,%d) -> Crossword \"%s\"\n",
				word,x,y,cross,sub*mw);
	}
	else {
		return 0;
	}
}
}
		}
		if (down) y++; else x++;
	}
	pts *= word_multi;
	pts += multi_score;
	if (strlen(word) == 7) pts += 50;
	return pts;
}

