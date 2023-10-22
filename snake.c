#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

int board[50][50];
int direction;
int fx, fy, hx, hy, score, dead;
int prevx[50], prevy[50];

void init() {
    srand(time(NULL));
    WINDOW *win = initscr(); cbreak(); noecho(); nodelay(win, true);
    start_color();
    init_color(10, 224, 1000, 78); // Body
    init_color(11, 67, 804, 184); // Head
    init_color(12, 1000, 455, 482); // Apple
    init_color(13, 498, 502, 463); // Wall
    init_pair(1, 10, 10); // Body
    init_pair(2, 11, 11); // Head
    init_pair(3, 12, 12); // Apple
    init_pair(4, 10, 13); // Wall

    
    score = 0;
    dead = 0;
    direction = 0;

    for (int i=0; i<50; i++) {
        prevx[i] = prevy[i] = 0;
        board[i][0] = 4;
        for (int j=1; j<49; j++) {
            if (i == 0 || i == 49) board[i][j] = 4;
            else board[i][j] = 0;
        }
        board[i][49] = 4;
    }

    fx = rand()%48 + 1;
    fy = rand()%48 + 1;
    board[fy][fx] = 3;

    board[25][25] = 2;
    hx = hy = 25;
}

void draw() {
    clear();
    for (int i=0; i<50; i++) {
        attron(COLOR_PAIR(4));
        mvprintw(i, 0,  "  ");
        mvprintw(0, i*2,  "  ");
        mvprintw(49, i*2,  "  ");
        mvprintw(i, 49*2, "  ");
        attroff(COLOR_PAIR(4));
    }
    attron(COLOR_PAIR(2));
    mvprintw(hy, hx*2, "  ");
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(3));
    mvprintw(fy, fx*2, "  ");
    attroff(COLOR_PAIR(3));

    attron(COLOR_PAIR(1));
    for (int i=0; i<score; i++) {
        mvprintw(prevy[i], prevx[i]*2, "  ");
    }
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(4));
    mvprintw(0, 0, "%d", score);
    attroff(COLOR_PAIR(4));
    refresh();
}

void getinput() {
    int in = getch();
    if (in == 'w') {
        direction = 0;
    } else if (in == 'd') {
        direction = 1;
    } else if (in == 's') {
        direction = 2;
    } else if (in == 'a') {
        direction = 3;
    }

    board[hy][hx] = 0;

    for (int i=49; i > 0; i--) {
        prevx[i] = prevx[i-1];
        prevy[i] = prevy[i-1];
    }

    prevx[0] = hx;
    prevy[0] = hy;

    switch (direction) {
    case 0:
        hy --;
        if (hy < 1) dead = 1;
        break;
    case 1:
        hx ++;
        if (hx > 48) dead = 1;
        break;
    case 2:
        hy ++;
        if (hy > 48) dead = 1;
        break;
    case 3:
        hx --;
        if (hx < 1) dead = 1;
        break;
    }

    if (hy == fy && hx == fx) {
        score ++;
        fx = rand()%48 + 1;
        fy = rand()%48 + 1;
        board[fy][fx] = 3;
    }

    board[hy][hx] = 2;
}

int main(void) {
    init();

    while (!dead) {
        draw();
        getinput();
        usleep(100000);
    }

    draw();
    usleep(500000);

    while (score -- > 0) {
        draw();
        usleep(200000);
    }

    usleep(100000);

    endwin();
}