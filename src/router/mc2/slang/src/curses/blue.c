/*****************************************************************************
 *                                                                           *
 *                         B l u e   M o o n                                 *
 *                         =================                                 *
 *                               V2.2                                        *
 *                   A patience game by T.A.Lister                           *
 *            Integral screen support by Eric S. Raymond                     *
 *                                                                           *
 *****************************************************************************/

/*
 * Compile this with the command `cc -O blue.c -lcurses -o blue'. For best
 * results, use the portable freeware ncurses(3) library.  On non-Intel
 * machines, SVr4 curses is just as good.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#if HAVE_TERMIOS_H
#include <sys/termios.h>
#endif

#include <curses.h>
#ifndef SLANG
#include <term.h>
#endif

#define NOCARD		(-1)

#define ACE		0
#define KING		12
#define SUIT_LENGTH	13

#define HEARTS		0
#define SPADES		1
#define DIAMONDS	2
#define CLUBS		3
#define NSUITS		4

#define GRID_WIDTH	14	/*    13+1  */
#define GRID_LENGTH	56	/* 4*(13+1) */
#define PACK_SIZE	52

#define BASEROW		1
#define PROMPTROW	11

static int deck_size = PACK_SIZE;	/* initial deck */
static int deck[PACK_SIZE];

static int grid[GRID_LENGTH];	/* card layout grid */
static int freeptr[4];		/* free card space pointers */

static int deal_number=0;

static chtype ranks[SUIT_LENGTH][2] =
{
    {' ', 'A'},
    {' ', '2'},
    {' ', '3'},
    {' ', '4'},
    {' ', '5'},
    {' ', '6'},
    {' ', '7'},
    {' ', '8'},
    {' ', '9'},
    {'1', '0'},
    {' ', 'J'},
    {' ', 'Q'},
    {' ', 'K'}
};

static chtype letters[] =
{
    'h' | COLOR_PAIR(COLOR_RED),	/* hearts */
    's' | COLOR_PAIR(COLOR_GREEN),	/* spades */
    'd' | COLOR_PAIR(COLOR_RED),	/* diamonds */
    'c' | COLOR_PAIR(COLOR_GREEN),	/* clubs */
};

#if defined(__i386__)
static chtype glyphs[] =
{
    '\003' | A_ALTCHARSET | COLOR_PAIR(COLOR_RED),	/* hearts */
    '\006' | A_ALTCHARSET | COLOR_PAIR(COLOR_GREEN),	/* spades */
    '\004' | A_ALTCHARSET | COLOR_PAIR(COLOR_RED),	/* diamonds */
    '\005' | A_ALTCHARSET | COLOR_PAIR(COLOR_GREEN),	/* clubs */
};
#endif /* __i386__ */

static chtype *suits = letters;	/* this may change to glyphs below */

static void die(int onsig)
{
    signal(onsig, SIG_IGN);
    endwin();
    exit(0);
}

static void init_vars(void)
{
    int i;

    deck_size = PACK_SIZE;
    for (i=0; i < PACK_SIZE; i++)
	deck[i]=i;
    for (i = 0; i < 4; i++)
	freeptr[i]=i * GRID_WIDTH;
}

static void shuffle(int size)
{
    int i,j,numswaps,swapnum,temp;

    numswaps=size*10;		/* an arbitrary figure */

    for (swapnum=0;swapnum<numswaps;swapnum++)
    {
	i=rand() % size;
	j=rand() % size;
	temp=deck[i];
	deck[i]=deck[j];
	deck[j]=temp;
    }
}

static void deal_cards(void)
{
    int ptr, card=0, value, csuit, crank, suit, aces[4];

    for (suit=HEARTS;suit<=CLUBS;suit++)
    {
	ptr=freeptr[suit];
	grid[ptr++]=NOCARD;	/* 1st card space is blank */
	while ((ptr % GRID_WIDTH) != 0)
	{
	    value=deck[card++];
	    crank=value % SUIT_LENGTH;
	    csuit=value / SUIT_LENGTH;
	    if (crank==ACE)
		aces[csuit]=ptr;
	    grid[ptr++]=value;
	}
    }

    if (deal_number==1)		/* shift the aces down to the 1st column */
	for (suit=HEARTS;suit<=CLUBS;suit++)
	{
	    grid[suit * GRID_WIDTH] = suit * SUIT_LENGTH;
	    grid[aces[suit]]=NOCARD;
	    freeptr[suit]=aces[suit];
	}
}

static void printcard(int value)
{
    (void) addch(' ');
    if (value == NOCARD)
	(void) addstr("   ");
    else
    {
	addch(ranks[value % SUIT_LENGTH][0] | COLOR_PAIR(COLOR_BLUE));
	addch(ranks[value % SUIT_LENGTH][1] | COLOR_PAIR(COLOR_BLUE));
	addch(suits[value / SUIT_LENGTH]);
    }
    (void) addch(' ');
}

static void display_cards(int deal)
{
    int row, card;

    clear();
    (void)printw(
		 "Blue Moon 2.1 - by Tim Lister & Eric Raymond - Deal %d.\n",
		 deal);
    for(row=HEARTS;row<=CLUBS;row++)
    {
	move(BASEROW + row + row + 2, 1);
	for(card=0;card<GRID_WIDTH;card++)
	    printcard(grid[row * GRID_WIDTH + card]);
    }

    move(PROMPTROW + 2, 0); refresh();
#define P(x)	(void)printw("%s\n", x)
P("   This 52-card solitaire starts with  the entire deck shuffled and dealt");
P("out in four rows.  The aces are then moved to the left end of the layout,");
P("making 4 initial free spaces.  You may move to a space only the card that");
P("matches the left neighbor in suit, and is one greater in rank.  Kings are");
P("high, so no cards may be placed to their right (they create dead spaces).");
P("  When no moves can be made,  cards still out of sequence are  reshuffled");
P("and dealt face up after the ends of the partial sequences, leaving a card");
P("space after each sequence, so that each row looks like a partial sequence");
P("followed by a space, followed by enough cards to make a row of 14.       ");
P("  A moment's reflection will show that this game cannot take more than 13");
P("deals. A good score is 1-3 deals, 4-7 is average, 8 or more is poor.     ");
#undef P
    refresh();
}

static int find(int card)
{
    int i;

    if ((card<0) || (card>=PACK_SIZE))
	return(NOCARD);
    for(i = 0; i < GRID_LENGTH; i++)
	if (grid[i] == card)
	    return i;
    return(NOCARD);
}

static void movecard(int src, int dst)
{
    grid[dst]=grid[src];
    grid[src]=NOCARD;

    move( BASEROW + (dst / GRID_WIDTH)*2+2, (dst % GRID_WIDTH)*5 + 1);
    printcard(grid[dst]);

    move( BASEROW + (src / GRID_WIDTH)*2+2, (src % GRID_WIDTH)*5 + 1);
    printcard(grid[src]);

    refresh();
}

static void play_game(void)
{
    int dead=0, i, j;
    char c;
    int select[4], card;

    while (dead<4)
    {
	dead=0;
	for (i=0;i<4;i++)
	{
	    card=grid[freeptr[i]-1];

	    if (	((card % SUIT_LENGTH)==KING)
		||
		(card==NOCARD)	)
		select[i]=NOCARD;
	    else
		select[i]=find(card+1);

	    if (select[i]==NOCARD)
		dead++;
	};

	if (dead < 4)
	{
	    char	live[NSUITS+1], *lp = live;

	    for (i=0;i<4;i++)
	    {
		if (select[i] != NOCARD)
		{
		    move(BASEROW + (select[i] / GRID_WIDTH)*2+3,
			 (select[i] % GRID_WIDTH)*5);
		    (void)printw("   %c ", *lp++ = 'a' + i);
		}
	    };
	    *lp = '\0';

	    if (strlen(live) == 1)
	    {
		move(PROMPTROW,0);
		(void)printw(
		    "Making forced moves...                                 ");
		refresh();
		(void) sleep(1);
		c = live[0];
	    }
	    else
	    {
		char	buf[BUFSIZ];

		(void)sprintf(buf,
			"Type [%s] to move, r to redraw, q or INTR to quit: ",
			live);

		do {
		    move(PROMPTROW,0);
		    (void) addstr(buf);
		    move(PROMPTROW, (int)strlen(buf));
		    clrtoeol();
		    (void) addch(' ');
		} while
		    (((c = getch())<'a' || c>'d') && (c!='r') && (c!='q'));
	    }

	    for (j = 0; j < 4; j++)
		if (select[j]!=NOCARD)
		{
		    move(BASEROW + (select[j] / GRID_WIDTH)*2+3,
			 (select[j] % GRID_WIDTH)*5);
		    (void)printw("     ");
		}

	    if (c == 'r')
		display_cards(deal_number);
	    else if (c == 'q')
		die(SIGINT);
	    else
	    {
		i = c-'a';
		if (select[i] == NOCARD)
		    beep();
		else
		{
		    movecard(select[i], freeptr[i]);
		    freeptr[i]=select[i];
		}
	    }
	}
    }

    move(PROMPTROW, 0);
    standout();
    (void)printw("Finished deal %d - type any character to continue...", deal_number);
    standend();
    (void) getch();
}

static int collect_discards(void)
{
    int row, col, cardno=0, finish, gridno;

    for (row=HEARTS;row<=CLUBS;row++)
    {
	finish=0;
	for (col=1;col<GRID_WIDTH;col++)
	{
	    gridno=row * GRID_WIDTH + col;

	    if ((grid[gridno]!=(grid[gridno-1]+1))&&(finish==0))
	    {
		finish=1;
		freeptr[row]=gridno;
	    };

	    if ((finish!=0)&&(grid[gridno]!=NOCARD))
		deck[cardno++]=grid[gridno];
	}
    }
    return cardno;
}

static void game_finished(int deal)
{
    clear();
    (void)printw("You finished the game in %d deals. This is ",deal);
    standout();
    if (deal<2)
	(void)addstr("excellent");
    else if (deal<4)
	(void)addstr("good");
    else if (deal<8)
	(void)addstr("average");
    else
	(void)addstr("poor");
    standend();
    (void) addstr(".         ");
    refresh();
}

int main(int argc, char *argv[])
{
    (void) signal(SIGINT, die);
    initscr();

    /*
     * We use COLOR_GREEN because COLOR_BLACK is wired to the wrong thing.
     */
    start_color();
    init_pair(COLOR_RED,     COLOR_RED,   COLOR_WHITE);
    init_pair(COLOR_BLUE,    COLOR_BLUE,  COLOR_WHITE);
    init_pair(COLOR_GREEN,   COLOR_BLACK, COLOR_WHITE);

#if defined(__i386__) && defined(A_ALTCHARSET)
    if (tigetstr("smpch"))
	suits = glyphs;
#endif /* __i386__ && A_ALTCHARSET */

    cbreak();

    if (argc == 2)
	srand((unsigned)atoi(argv[1]));
    else
	srand((unsigned)time((time_t *)0));

    init_vars();

    do{
	deal_number++;
	shuffle(deck_size);
	deal_cards();
	display_cards(deal_number);
	play_game();
    }
    while
	((deck_size=collect_discards()) != 0);

    game_finished(deal_number);

    die(SIGINT);
    /*NOTREACHED*/
   return 1;
}

/* blue.c ends here */
