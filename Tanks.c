#include <ncurses.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

int map[50][200], heights[200];
int dead, p1x, p1y, ip1y, p2x, p2y, ip2y, angle, velocity, sx, sy, turn, type;
int depth;
WINDOW *win;

int cloud[5][22] = {
        {4,4,4,4,4,4,4,6,6,6,4,4,4,4,4,4,4,4,4,4,4,4},
        {4,4,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,4,4},
        {6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
        {4,6,6,6,6,4,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,4}, 
        {4,4,6,6,4,4,4,4,6,6,6,6,6,4,4,4,6,6,4,4,4,4}
    };

int clouds[7][2] = {{47, 10}, {45, 42}, {48, 66}, {42, 91}, {47, 118}, {49, 144}, {43, 168}};

void reset();
void draw(int pro);

void init() {
    srand(time(NULL));
    win = initscr(); cbreak(); noecho(); nodelay(win, true);
    start_color();
    init_color(10, 514, 396, 224); // Ground 131,101,57
    init_color(11, 271, 294, 106); // Artillery 69, 75, 27
    init_color(12, 392, 127, 112); // Bullet 196, 196, 196 / R: 100, G:32.5, B: 28.6
    init_color(13, 529, 808, 922); // Sky 135, 206, 235
    init_color(14, 337, 490, 275); // Grass 86,125,70
    init_color(15, 925, 941, 945); // Clouds 236, 240, 241
    init_color(16, 839, 420, 51); // Explosion 214, 107, 13
    init_pair(1, 10, 10); // Ground
    init_pair(2, 11, 11); // Artillery
    init_pair(3, 12, 12); // Bullet
    init_pair(4, 10, 13); // Sky
    init_pair(5, 14, 14); // Grass
    init_pair(6, 15, 15); // Clouds
    init_pair(7, 16, 16); // Explosion

    reset();
}

void reset() {
    dead = angle = velocity = 0;
    turn = type = 1; depth = -1;

    p1x = (10 + rand()%5 - 2) * 5 - 2;
    p2x = (30 + rand()%5 - 2) * 5 - 2;

    int height = rand()%30;
    while (height < 20) {
        height += 8;
    } while (height > 30) {
        height -= 8;
    }

    for (int i=0; i<200; heights[i++]=0);

    height = 50 - height;

    for (int i=1; i<200; i++) {
        if ((i-1)%3 == 0) {
            heights[i] = height;
            heights[i-1] = height + rand()%10 - 5;
            heights[i+1] = height;
            while (++ height < 20);
            while (-- height > 30);
            height += rand()%10 - 5;
        }
    }

    int flag = 1;
    for (int i=1; i<200; i++) {
        if (i > 1 && i<198 && flag)
            heights[i] = (heights[i-2]+heights[i-1] + heights[i+1]+heights[i+2])/4;
        else if (!flag) flag = 1;
        if (p1x == i || p2x == i) {
            heights[i-1] = heights[i];
            heights[i+1] = heights[i];
            flag = 0;
        }
    }

    for (int i=0; i<200; i++) {
        for (int j=0; j<50; j++) {
            if (j < heights[i]-1) map[j][i] = 1;
            else if (j == heights[i]-1) map[j][i] = 5;
            else map[j][i] = 4;
            if (j == heights[i]) {
                if (i == p1x) {
                    p1y = j;
                    map[j][i] = 2;
                    map[j-10][i] = 2;
                } else if (i == p2x) {
                    p2y = j;
                    map[j][i] = 2;
                    map[j-10][i] = 2;
                }
            }
        }
    }
    ip1y = p1y;
    ip2y = p2y;
}

double angles[91] = {0.00000, 0.01745, 0.03490, 0.05234, 0.06976, 0.08716, 0.10453, 0.12187, 0.13917, 0.15643, 0.17365, 0.19081, 0.20791, 0.22495, 0.24192, 0.25882, 0.27564, 0.29237, 0.30902, 0.32557, 0.34202, 0.35837, 0.37461, 0.39073, 0.40674, 0.42262, 0.43837, 0.45399, 0.46947, 0.48481, 0.50000, 0.51504, 0.52992, 0.54464, 0.55919, 0.57358, 0.58779, 0.60182, 0.61566, 0.62932, 0.64279, 0.65606, 0.66913, 0.68199, 0.69466, 0.70711, 0.71934, 0.73135, 0.74314, 0.75471, 0.76604, 0.77715, 0.78801, 0.79864, 0.80902, 0.81915, 0.82904, 0.83867, 0.84805, 0.85717, 0.86603, 0.87462, 0.88295, 0.89101, 0.89879, 0.90631, 0.91355, 0.92050, 0.92718, 0.93358, 0.93969, 0.94552, 0.95106, 0.95630, 0.96126, 0.96593, 0.97030, 0.97437, 0.97815, 0.98163, 0.98481, 0.98769, 0.99027, 0.99255, 0.99452, 0.99619, 0.99756, 0.99863, 0.99939, 0.99985, 1.00000};
//Starts with sin do 90-angle to get cos

void progress() {
    for (int k=0; k<7; k++) {
        for (int i=0; i<5; i++)
            for (int j=0; j<22; j++)
                map[clouds[k][0]-i][(clouds[k][1]+j >= 200)? clouds[k][1]+j - 200:clouds[k][1]+j] = 4;
        
        clouds[k][1] ++;
        if (clouds[k][1] >= 200) clouds[k][1] = 0;

        for (int i=0; i<5; i++)
            for (int j=0; j<22; j++)
                map[clouds[k][0]-i][(clouds[k][1]+j >= 200)? clouds[k][1]+j - 200:clouds[k][1]+j] = cloud[i][j];
    }
    draw(1);
}

void draw(int pro) {
    // clear();
    int y, x;
    getyx(win, y, x);
    for (int j=49; j>=0; j--) {
        for (int i=0; i<200; i++) {
            if (j == sy && i == sx && j != 0 && i != 0) {
                attron(COLOR_PAIR(3));
                mvprintw(49-j, i, " ");
                attroff(COLOR_PAIR(3));
            } else {
                attron(COLOR_PAIR(map[j][i]));
                mvprintw(49-j, i, " ");
                attroff(COLOR_PAIR(map[j][i]));
            }
            if (map[j][i] == 7 && !pro) map[j][i] = 4;
        }
    }
    refresh();
    move(y, x);
}

int getin() {
    int y, x;
    getyx(win, y, x);
    int in = getch(); usleep(20000);
    int count = 0;
    while (in == -1) {
        in = getch(); usleep(20000);
        if (++ count >= 10) {
            progress();
            count = 0;
        }
    }
    move(y, x);
    return in;
}

void getinput() {
    sx = sy = 0;
    mvprintw(51, 0, "Player %d's turn\n", turn);
    printw("Type of explosive, 1 (shell), 2 (Bunker buster), 3 (High explosive round), 4 (MOAB) ...\n");
    printw("Type: ");
    refresh();
    int in = 0, res = 0;
    do {
        in = getin();
        in -= (int)'0';
    } while (in < 1 || in > 4);
    printw("%d", in);
    refresh();
    type = in;
    printw("\nFor angle less than 10 make sure to do 01 or 02 ...\n");
    printw("Angle: ");
    refresh();
    in = 0;
    do {
        in = getin();
        in -= (int)'0';
    } while (in < 0 || in > 9);
    printw("%d", in);
    refresh();
    res += in * 10;
    if (in != 9) {
        do {
            in = getin();
            in -= (int)'0';
        } while (in < 0 || in > 9);
        printw("%d", in);
        res += in;
    } else {
        printw("0");
    }
    angle = res;
    refresh();

    printw("\nFor velocity less than 100 make sure to do 001 or 002 ...\n");
    printw("Velocity: ");
    refresh();
    res = 0;
    do {
        in = getin();
        in -= (int)'0';
    } while (in < 0 || in > 9);//1);
    printw("%d", in);
    res += in * 100;
    refresh();
    do {
        in = getin();
        in -= (int)'0';
    } while (in < 0 || in > 9);//|| (res != 100)? in > 9:in > 4);
    printw("%d", in);
    refresh();
    res += in * 10;
    do {
        in = getin();
        in -= (int)'0';
    } while (in < 0 || in > 9);
    printw("%d", in);
    res += in;
    velocity = res;
    refresh();
}

void collide() {
    /*
    2 c depth 5c width of 105 mm
     ***
    *****       - Ground
     ***
    3c depth 7c width of 155 mm
       *
      ***
    *******    - Ground
      ***
    3c depth then 3c depth and 7c width explosion of BLU-109
       *        - Ground
       *
       * 
      ***
    *******
     *****

    5c depth with 11c width of MOAB
      *******
       *****    - Ground
    ***********
     *********
       *****


    */
    if (type == 1) {
        for (int i=sx-1; i<sx+2; i++) {
            map[sy+1][i] = 7;
        } for (int i=sx-2; i<sx+3; i++) {
            map[sy][i] = 7;
        } for (int i=sx-1; i<sx+2; i++) {
            map[sy-1][i] = 7;
        }
        sx = sy = 0;
    } else if (type == 2) {
        map[sy+2][sx] = 7;
        for (int i=sx-1; i<sx+2; i++) {
            map[sy+1][i] = 7;
        } for (int i=sx-3; i<sx+4; i++) {
            map[sy][i] = 7;
        } for (int i=sx-1; i<sx+2; i++) {
            map[sy-1][i] = 7;
        }
        sx = sy = 0;
    } else if (type == 3) {
        if (depth == 3 || depth == 2 || depth == 1) { map[sy][sx] = 7; depth --; }
        else if (depth == 0) {
            for (int i=sx-1; i<sx+2; i++) {
                map[sy-3][i] = 7;
            } for (int i=sx-3; i<sx+4; i++) {
                map[sy-4][i] = 7;
            } for (int i=sx-2; i<sx+3; i++) {
                map[sy-5][i] = 7;
            }
            depth = -1;
            sx = sy = 0;
        } else if (depth == -1) {
            depth = 3;
        } else {
            depth = -1;
            sx = sy = 0;
        }
    } else if (type == 4) {
        for (int i=sx-3; i<sx+4; i++) {
            map[sy+1][i] = 7;
        } for (int i=sx-2; i<sx+3; i++) {
            map[sy][i] = 7;
        } for (int i=sx-5; i<sx+6; i++) {
            map[sy-1][i] = 7;
        } for (int i=sx-4; i<sx+5; i++) {
            map[sy-2][i] = 7;
        } for (int i=sx-3; i<sx+4; i++) {
            map[sy-3][i] = 7;
        }
        sx = sy = 0;
    }
    
}

void calculate(double t) {
    double ux, uy;
    uy = angles[angle] * (double)velocity;
    ux = angles[90-angle] * (double)velocity;

    if (turn == 1) {
        sx = (int)(ux * t)*.1 + p1x;
        sy = (int)((uy - 4.9 * t) * t)*.1 + p1y;
    } else {
        sx = p2x - (int)(ux * t)*.1;
        sy = (int)((uy - 4.9 * t) * t)*.1 + p2y;
    }
    if (sy < 50 && sx < 200 && sx > 0) {
        if (((map[sy][sx] != 4 && map[sy][sx] != 6) || (map[sy][sx+1] != 4 && map[sy][sx+1] != 6) || (map[sy][sx-1] != 4 && map[sy][sx-1] != 6) ) && t > 1) { 
            // if (t < 1) {
            //     if (map[sy][sx] == 2) map[p1y][p1x] = 2;
            // }
            collide();
            // if (t < 1) {
            //     if (turn == 1 && map[p1y][p1x] == 4) map[p1y][p1x] = 2;
            //     else if (turn == 2 && map[p2y][p2x] == 4) map[p2y][p2x] = 2;
            // }
        }
    }
}

int main(void) {
    init();
    for (int i=0; i<3; i++) {
        while (1) {
            clear();
            draw(0);
            getinput();
            double time = .0625;
            int count = 0;
            do {
                calculate(time);
                time += .0625;
                if (sy < 0) sy = 0;
                if (sy < 50) {
                    clear();
                    draw(1);
                }
                mvprintw(51, 0, "\nsx: %d, sy: %d\n", sx, sy);
                refresh();
                if (++ count >= 20) {
                    progress();
                    count = 0;
                }
                usleep(10000);
            } while (sy > 0 && sx > 0 && sx < 200);
            progress();
            usleep(200000);
            progress();
            usleep(200000);
            draw(0);
            while ((map[p1y - 1][p1x] == 4 || map[p2y - 1][p2x] == 4) && (ip1y != p1y - 1 && ip2y != p2y - 1)) { if (map[p1y - 1][p1x] == 4 && ip1y != p1y - 1) { map[p1y --][p1x] = 4; map[p1y][p1x] = 2; } if (map[p2y - 1][p2x] == 4 && ip2y != p2y - 1) { map[p2y --][p2x] = 4; map[p2y][p2x] = 2; } progress(); usleep(200000); }

            if (map[ip1y-10][p1x] == 4) { turn = 2; break; }
            if (map[ip2y-10][p2x] == 4) { turn = 1; break; }
            turn = (turn == 1)? 2:1;
        }
        clear();
        draw(0);
        mvprintw(51, 0, "\nPlayer %d wins!!!\n", turn);
        refresh();
        usleep(1000000);
        reset();
    }

    endwin();
    return 0;
}
