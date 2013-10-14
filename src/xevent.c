
#include "squabble.h"

extern Display *dpy;
extern Window root, win;
extern Cursor invisible_cursor;

static void buttonpress(XEvent *);
static void buttonrelease(XEvent *);
static void expose(XEvent *);
static void keypress(XEvent *);
static void motionnotify(XEvent *);


static Bool player_turn;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress]		= buttonpress,
	[ButtonRelease]	= buttonrelease,
	[Expose]				= expose,
	[KeyPress]			= keypress,
	[MotionNotify]		= motionnotify,
};


void buttonpress(XEvent *ev) {
	XButtonEvent *e = &ev->xbutton;
	int x = e->x/csx, y = e->y/csy, xx, yy;
	if (e->button == 1) {
		if (y < 100) { // TOP
			// top click
		}
		else if (y < 850) { // MID
			if ( (250 < x) && (x < 1000) ) { // board click
				xx = (x - 250)/50; yy = (y - 100)/50;
				if ( (tile_target=board[xx][yy].tile) ) {
					tile_floating.x = x - 25;
					tile_floating.y = y - 25;
					XDefineCursor(dpy,win,invisible_cursor);
					XGrabPointer(dpy,root,True,PointerMotionMask |
							ButtonReleaseMask,GrabModeAsync, GrabModeAsync,
							None, None, CurrentTime);
				}
			}
			else { // margin click
			}
		}
		else { // BOTTOM
			if ( (250 < x) && (x < 1000) ) { // player's rack
				int i;
				for (i = 0; i < 7 && rack[i]; i++) {
					if (rack[i]->flags == TILE_BOARD_TEMP) continue;
					if (x > rack[i]->x && x < rack[i]->x + 50) {
						tile_target = rack[i];
						tile_floating.x = x - 25;
						tile_floating.y = y - 25;
						XDefineCursor(dpy,win,invisible_cursor);
						XGrabPointer(dpy,root,True,PointerMotionMask |
								ButtonReleaseMask,GrabModeAsync, GrabModeAsync,
								None, None, CurrentTime);
					}
				}
			}
			else { // bottom corner
			}
		}
	}
	draw();
}

void buttonrelease(XEvent *ev) {
	XUngrabPointer(dpy,CurrentTime);
	XUndefineCursor(dpy,win);
	XButtonEvent *e = &ev->xbutton;
	int x = e->x/csx, y = e->y/csy, xx, yy;
	if (tile_target->flags != TILE_BOARD_PERM) {
		if ( (250 < x) && (x < 1000) && (100 < y) && (y < 850) ) {
			/* place tile on board */
			xx = (x - 250)/50; yy = (y - 100)/50;
			if ( !board[xx][yy].tile ) {
				if (tile_target->flags == TILE_BOARD_TEMP) {
					int tx = (tile_target->x-250)/50;
					int ty = (tile_target->y-100)/50;
					board[tx][ty].tile = NULL;
				}
				board[xx][yy].tile = tile_target;
				tile_target->x = xx*50+250;
				tile_target->y = yy*50+100;
				tile_target->flags = TILE_BOARD_TEMP;
			}
		}
		else {
			if (tile_target->flags == TILE_BOARD_TEMP) {
				/* remove tile from board */
				int tx = (tile_target->x-250)/50;
				int ty = (tile_target->y-100)/50;
				board[tx][ty].tile = NULL;
			}
			tile_target->x = x;
			tile_target->flags = TILE_PLAYER;
			tile_target->y = 880;
			if ( (300 < x) && (x < 1000) && (875 < y) && (y < 975) ) {
				/* placement within rack */
			}
			// else if (bag area) {}
			else {
				/* place at end of rack */
				tile_target->x = 900;
			}
			tiles_rack_sort();
		}
	}
	tile_floating.letter = 0;
	tile_target = NULL;
	draw();
}

void expose(XEvent *ev) {
	draw();
}

void keypress(XEvent *ev) {
	XKeyEvent *e = &ev->xkey;
	KeySym keysym = XkbKeycodeToKeysym(dpy,(KeyCode)e->keycode,0,0);
	if (keysym == XK_q) running = False;
	else if (keysym == XK_Return) {
		if ( IN_STAGE(STAGE_PLAYER|STAGE_THINKING) ) {
			stage = (STAGE_PLAYER|STAGE_DONE);
			ai_play_done();
			draw();
		}
	}
}

void motionnotify(XEvent *ev) {
	while (XCheckTypedEvent(dpy,MotionNotify,ev));
	XButtonEvent *e = &ev->xbutton;
	tile_floating.x = e->x/csx - 25; tile_floating.y = e->y/csy - 25;
	draw();
}

void main_loop() {
	draw();
	srand(time(NULL));
	running = True;
	if (rand() % 2) {
		stage = (STAGE_PLAYER|STAGE_THINKING);
	}
	else {
 		stage = (STAGE_AI|STAGE_FIRST|STAGE_THINKING);
 		ai_find_play(tiles(TILE_OPPONENT));
	}
	XEvent ev;
	int xfd = ConnectionNumber(dpy);
	struct timeval timeout;
	fd_set fds;
	timer = time(NULL);
	while (running) {
		FD_ZERO(&fds); FD_SET(xfd,&fds);
		memset(&timeout,0,sizeof(struct timeval));
		timeout.tv_sec = 1;
		select(xfd+1,&fds,0,0,&timeout);
		if (FD_ISSET(xfd,&fds)) while (XPending(dpy)) {
			XNextEvent(dpy,&ev);
			if (ev.type < 33 && handler[ev.type])
				handler[ev.type](&ev);
		}
		else draw();
		if ( IN_STAGE(STAGE_PLAYER|STAGE_DONE) ) {
			timer = time(NULL);
			ai_find_play(tiles(TILE_OPPONENT));
			stage = (STAGE_AI|STAGE_THINKING);
		}
		else if ( IN_STAGE(STAGE_AI|STAGE_DONE) ) {
			timer = time(NULL);
			tiles_play(TILE_OPPONENT);
			stage = (STAGE_PLAYER|STAGE_THINKING);
			draw();
		}
	}
	while (ai_running) usleep(5000);
}

