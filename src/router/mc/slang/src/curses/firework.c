#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <curses.h>
#include <ctype.h>
#include <time.h>

static int get_colour(void);
static void explode(int row, int col);
static void showit(void);

int main(int argc, char *argv[])
{
int start,end,row,diff,flag = 0,direction;
unsigned seed;

       initscr();
       if (has_colors())
          start_color();
       seed = time((time_t *)0);
       srand(seed);
       cbreak();
       for (;;) {
            do {
                start = rand() % (COLS -3);
                end = rand() % (COLS - 3);
                start = (start < 2) ? 2 : start;
                end = (end < 2) ? 2 : end;
                direction = (start > end) ? -1 : 1;
                diff = abs(start-end);
            } while (diff<2 || diff>=LINES-2);
            attrset(A_NORMAL);
            for (row=1;row<diff;row++) {
                mvprintw(LINES - row,start + (row * direction),
                    (direction < 0) ? "\\" : "/");
                if (flag++) {
                    showit();
                    erase();
                    flag = 0;
                }
            }
            if (flag++) {
                showit();
                flag = 0;
            }
            seed = time((time_t *)0);
            srand(seed);
            explode(LINES-row,start+(diff*direction));
            erase();
            showit();
       }
}

static
void explode(int row, int col)
{
       erase();
       mvprintw(row,col,"-");
       showit();

       init_pair(1,get_colour(),COLOR_BLACK);
       attrset(COLOR_PAIR(1));
       mvprintw(row-1,col-1," - ");
       mvprintw(row,col-1,"-+-");
       mvprintw(row+1,col-1," - ");
       showit();

       init_pair(1,get_colour(),COLOR_BLACK);
       attrset(COLOR_PAIR(1));
       mvprintw(row-2,col-2," --- ");
       mvprintw(row-1,col-2,"-+++-");
       mvprintw(row,  col-2,"-+#+-");
       mvprintw(row+1,col-2,"-+++-");
       mvprintw(row+2,col-2," --- ");
       showit();

       init_pair(1,get_colour(),COLOR_BLACK);
       attrset(COLOR_PAIR(1));
       mvprintw(row-2,col-2," +++ ");
       mvprintw(row-1,col-2,"++#++");
       mvprintw(row,  col-2,"+# #+");
       mvprintw(row+1,col-2,"++#++");
       mvprintw(row+2,col-2," +++ ");
       showit();

       init_pair(1,get_colour(),COLOR_BLACK);
       attrset(COLOR_PAIR(1));
       mvprintw(row-2,col-2,"  #  ");
       mvprintw(row-1,col-2,"## ##");
       mvprintw(row,  col-2,"#   #");
       mvprintw(row+1,col-2,"## ##");
       mvprintw(row+2,col-2,"  #  ");
       showit();

       init_pair(1,get_colour(),COLOR_BLACK);
       attrset(COLOR_PAIR(1));
       mvprintw(row-2,col-2," # # ");
       mvprintw(row-1,col-2,"#   #");
       mvprintw(row,  col-2,"     ");
       mvprintw(row+1,col-2,"#   #");
       mvprintw(row+2,col-2," # # ");
       showit();
}

static
int get_colour(void)
{
 int attr;
       attr = (rand() % 16)+1;
       if (attr == 1 || attr == 9)
          attr = COLOR_RED;
       if (attr > 8)
          attr |= A_BOLD;
       return(attr);
}

static void
showit(void)
{
	refresh();
	napms(120);
}
