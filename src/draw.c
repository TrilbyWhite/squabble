
#include "squabble.h"

static cairo_surface_t *imgBrd;
static cairo_surface_t **imgTile = NULL;
static int n_img = 0;

static void create_tile(const char *,int,int);

void draw_init(const char *fname) {
	chdir(RESOURCE_DIR);
	FILE *ts;
	ts = fopen(fname,"r");
	if (!ts) ts = fopen(RESOURCE_DIR "tiles","r");
	if (!ts) die("unable to open tiles file for images");
	char line[64];
	int n = 0, ret = 0, pts;
	cairo_t *c;
	/* CREATE + DRAW TILE IMAGES */
	while (fgets(line,64,ts)) {
		if ( (strlen(line) < 5) || (line[0] == '#') ) continue;
		if (sscanf(line,"%*c, %*d, %d",&pts) != 1) pts = 0;
		line[0] -= 32; line[1] = '\0';
		create_tile(line,n,pts);
		n++;
	}
	create_tile(" ",n,0); n++;
	n_img = n;
	/* CREATE + DRAW BOARD IMAGE */
	imgBrd = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,1250, 950);
	c = cairo_create(imgBrd);
	/* green table */
	cairo_set_source_rgba(c,0.1,0.6,0.3,1);
	cairo_rectangle(c,0,0,1250,950);
	cairo_fill(c);
	/* off white backing */
	cairo_set_source_rgba(c,0.92,0.92,0.85,1);
	cairo_rectangle(c,250,100,750,750);
	cairo_fill(c);
	/* bonus shades */
	int i, j;
	for (i = 0; i < 15; i++) for (j = 0; j < 15; j++) {
		if (board[i][j].word_bonus == 3)
			cairo_set_source_rgba(c,1,0,0,0.5);
		else if (board[i][j].word_bonus == 2)
			cairo_set_source_rgba(c,1,0,0,0.15);
		else if (board[i][j].tile_bonus == 3)
			cairo_set_source_rgba(c,0,0,1,0.5);
		else if (board[i][j].tile_bonus == 2)
			cairo_set_source_rgba(c,0,0,1,0.15);
		else
			cairo_set_source_rgba(c,1,1,1,0);
		cairo_rectangle(c,250+i*50,100+j*50,50,50);
		cairo_fill_preserve(c);
		cairo_set_source_rgba(c,0,0,0,1);
		cairo_stroke(c);
	}
	/* border */
	cairo_set_source_rgba(c,0,0,0,1);
	cairo_set_line_width(c,1);
	cairo_rectangle(c,250,100,750,750);
	cairo_stroke(c);
	cairo_destroy(c);
	fclose(ts);
}

void draw_free() {
	cairo_surface_destroy(imgBrd);
	int i;
	for (i = 0; i < n_img; i++)
		cairo_surface_destroy(imgTile[i]);
	free(imgTile);
	n_img = 0;
}


void draw() {
	xcairo_start();
	cairo_set_source_surface(c,imgBrd,0,0);
	cairo_paint(c);
	int i, j;
	Tile *t;
	for (j = 0; j < 15; j++) for (i = 0; i < 15; i++) { // BOARD
		if ( (t = board[i][j].tile) && (t != tile_target) ) {
			cairo_set_source_surface(c,imgTile[t->letter-'a'],
					t->x,t->y);
			cairo_paint(c);
		}
	}
	/* SCORES */
	char str[28];
	/* player */
	if (stage & STAGE_PLAYER)
		cairo_set_source_rgba(c,0.6,0.9,1,1);
	else
		cairo_set_source_rgba(c,1,1,1,1);
	cairo_rectangle(c,1025,25,200,30);
	cairo_fill_preserve(c);
	cairo_set_source_rgba(c,0,0,0,1);
	cairo_stroke(c);
	cairo_move_to(c,1030,48);
	snprintf(str,sizeof(str),"Player: %d",score_player);
	cairo_show_text(c,str);
	/* computer */
	if (stage & STAGE_AI)
		cairo_set_source_rgba(c,0.6,0.9,1,1);
	else
		cairo_set_source_rgba(c,1,1,1,1);
	cairo_rectangle(c,1025,65,200,30);
	cairo_fill_preserve(c);
	cairo_set_source_rgba(c,0,0,0,1);
	cairo_stroke(c);
	cairo_move_to(c,1030,88);
	snprintf(str,sizeof(str),"Computer: %d",score_comp);
	cairo_show_text(c,str);
	/* TIMER */
	i = (int) difftime(time(NULL),timer);
	j = i % 60;
	snprintf(str,sizeof(str),"TIME: %d:%02d",i/60,j);
	cairo_move_to(c,1030,125);
	cairo_show_text(c,str);
	/* PLAYERS RACK */
	cairo_set_source_rgba(c,0.85,0.75,0.4,1);
	cairo_rectangle(c,300,875,650,65);
	cairo_fill_preserve(c);
	cairo_set_source_rgba(c,0,0,0,1);
	cairo_stroke(c);
	for (i = 0; i < 7 && rack[i]; i++) {
		if (rack[i] == tile_target) continue;
		cairo_set_source_surface(c,imgTile[rack[i]->letter-'a'],
				rack[i]->x,rack[i]->y);
		cairo_paint(c);
	}
	/* TILE PILE */
	tiles_draw_pile(c,imgTile,n_img-1);
	/* FLOATING TILE */
	if (tile_target) {
		cairo_set_source_surface(c,imgTile[tile_target->letter-'a'],
				tile_floating.x,tile_floating.y);
		cairo_paint(c);
	}
	xcairo_end();
}

/*******************************************/

void create_tile(const char *line, int n, int pts) {
	char cpt[2]; cpt[1] = '\0';
	cairo_t *ctile;
	imgTile = (cairo_surface_t **) realloc(imgTile,
			(n+1)*sizeof(cairo_surface_t *));
	imgTile[n] = cairo_image_surface_create(
			CAIRO_FORMAT_ARGB32,50, 50);
	ctile = cairo_create(imgTile[n]);
	cairo_set_line_join(ctile,CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(ctile,3);
	cairo_select_font_face(ctile,"serif",
			CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);
	/* tile background */	
	cairo_set_source_rgba(ctile,1,1,1,1);
	cairo_set_font_size(ctile,32);
	cairo_rectangle(ctile,5,5,40,40);
	cairo_fill_preserve(ctile);
	cairo_set_source_rgba(ctile,0,0,0,1);
	cairo_stroke(ctile);
	/* letter and score */
	cairo_move_to(ctile,8,34);
	cairo_show_text(ctile,line);
	if (pts) {
		cairo_move_to(ctile,33,42);
		cairo_set_font_size(ctile,12);
		cpt[0] = pts + 48;
		cairo_show_text(ctile,cpt);
	}
	cairo_destroy(ctile);
}
