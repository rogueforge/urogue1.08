/*
 * Last Modified: Oct 17, 1995
 */

 /*
 * Rogue
 * Exploring the dungeons of doom
 * Copyright (C) 1980 by Michael Toy and Glenn Wichman
 * All rights reserved
 *
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "rogue.h"

/* Where the scores are kept, unless the build says otherwise */

#ifndef SCOREFILE
#define SCOREFILE "urogue.scr"
#endif

FILE *fd_score = NULL;

/* Command line options */

int prscore;        /* Print scores */
int prversion;      /* Print version info */

int
main(int argc, char *argv[])
{
    int x;
    struct stat sbuf, sbuf2;
    char    *env;
    char    *seed;
    time_t lowtime;
    time_t    now;
    int rflag = 0;
    char *nm;
	float scale;

    for (x = 1; x < argc; x++)
    {
        if (argv[x][0] != '-')
            break;

        switch (argv[x][1])
        {
            case 's':
                prscore = TRUE;
                break;

            case 'v':
                prversion = TRUE;
                break;

            case 'r':
                rflag = TRUE;
                break;

            default:
                fprintf(stderr,"%s: Unknown option '%c'.\n",argv[0],argv[x][1]);
                exit(1);
        }
    }

    if (!rflag)
    {
        argc -= (x - 1);
        argv += (x - 1);
    }
    /*
     * A file grown past its limit should make the write fail, not end
     * the game in the middle of a save
     */

#ifdef SIGXFSZ
    signal(SIGXFSZ, SIG_IGN);
#endif

    /* Get default score file */

    strncpy(score_file, SCOREFILE, 2 * LINELEN - 1);
    /* Keep the score file off stdin, stdout and stderr */

    while ((x = open("/dev/null", O_RDWR)) >= 0 && x < 3)
        continue;

    if (x >= 3)
        close(x);

    /*
     * Keep scores only in a plain file with one link, and see that the
     * file opened is the one looked at.  One that is not there yet is made
     * new.
     */

    if (lstat(score_file, &sbuf) < 0)
        x = (errno == ENOENT ? open(score_file, O_RDWR | O_CREAT | O_EXCL, 0644) : -1);
    else if (!S_ISREG(sbuf.st_mode) || sbuf.st_nlink != 1)
        x = -1;
    else if ((x = open(score_file, O_RDWR)) >= 0 &&
             (fstat(x, &sbuf2) < 0 || sbuf2.st_dev != sbuf.st_dev || sbuf2.st_ino != sbuf.st_ino))
             {
        close(x);
        x = -1;
    }
    if (x < 0 || (fd_score = fdopen(x, "r+")) == NULL)
    {
        if (x >= 0)
            close(x);

        fd_score = NULL;
    }

    if ((env = getenv("OPTIONS")) != NULL)
        parse_opts(env);

    nm = getenv("USER");

    if (whoami[0] != '\0')    /* the name option has named the hero */
        ;
    else if (nm != NULL)
        strncpy(whoami, nm, 2 * LINELEN - 1);
    else
        strcpy(whoami,"anonymous");
		
	lowtime = time(&now);
	
	seed = getenv("SEED");

	if (seed != NULL && atoi(seed) > 0)
	    dnum = atoi(seed);
	else
	    dnum = (int)lowtime;
	
	ur_srandom(dnum);

    if (env == NULL || fruit[0] == '\0')
    {
        static const char *funfruit[] =
        {
            "candleberry", "caprifig", "dewberry", "elderberry",
            "gooseberry", "guanabana", "hagberry", "ilama", "imbu",
            "jaboticaba", "jujube", "litchi", "mombin", "pitanga",
            "prickly pear", "rambutan", "sapodilla", "soursop",
            "sweetsop", "whortleberry"
        };

        strcpy(fruit, funfruit[rnd(sizeof(funfruit) / sizeof(funfruit[0]))]);
    }

    /* put a copy of fruit in the right place */

    fd_data[1].mi_name = strdup(fruit); 

    /* print scores */

    if (prscore)
    {
        waswizard = TRUE;
        score(0L, 0, SCOREIT, 0);
        exit(0);
    }

    /* check for version option */

    if (prversion)
    {
        printf("UltraRogue Version %s.\n", release);
        exit(0);
    }

    if (wizard || seed != NULL)
        printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
        printf("Hello %s, just a moment while I dig the dungeon...", whoami);
#ifdef MEMCHECK

    mem_debug(2);
	mem_tracking(1);
#endif

    fflush(stdout);

    init_things();      /* Set up probabilities of things */
    init_fd();          /* Set up food probabilities */
    init_colors();      /* Set up colors of potions */
    init_stones();      /* Set up stone settings of rings */
    init_materials();   /* Set up materials of wands */
    initscr();          /* Start up cursor package */

    if (LINES < 15 || COLS < 40)
    {
        endwin();
        printf("\nSorry, urogue needs a terminal with at least 15 lines and "
            "40 columns.\n");
        exit(1);
    }

    refresh();

    /*
     * The level is laid out by LINES and COLS in windows that size. Have
     * curses keep the size the screen has now, even if the terminal is
     * resized.
     */

    sprintf(prbuf, "%d", LINES);
    setenv("LINES", prbuf, 1);
    sprintf(prbuf, "%d", COLS);
    setenv("COLUMNS", prbuf, 1);

    init_names();       /* Set up names of scrolls */
    cbreak();
    crmode();           /* Cbreak mode */
    noecho();           /* Echo off */
    nonl();
	
    scale = (float) (LINES * COLS) / (80.0F * 25.0F); /* get food right for     */
	                                              /* different screen sizes */
												  
    food_left = (int) (food_left * scale);

    /* Set up windows */

    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);

    if (argc == 2 && argv[1][0] != '\0' && !restore(argv[1]))
        /* Note: restore returns on error only */
        exit(1);

    waswizard = wizard; /* set wizard flags */

    init_player();      /* look up things and outfit pack */

    resurrect = pstats.s_const;
    init_exp();         /* set first experience level change */
    init_flags();       /* set initial flags */
    wclear(hw);
    wrefresh(hw);
    new_level(POSTLEV); /* Draw current level */

    /* Start up daemons and fuses */

    start_daemon(doctor, &player, AFTER);

    light_fuse(swander, 0, WANDERTIME, AFTER);

    start_daemon(stomach, 0, AFTER);
    start_daemon(runners, 0, AFTER);

    char_type = player.t_ctype;
    player.t_oldpos = hero;
    oldrp = roomin(hero);
    after = TRUE;

    signal(SIGINT, quit_handler);

    while(playing)
    {
        do_daemons(BEFORE);
        do_fuses(BEFORE);

        command();  /* Command execution */

        if (after)
            do_after_effects();
    }

    fatal("");

    return(0);
}

/*
    fatal()
        Exit the program, printing a message.
*/

void
fatal(char *s)
{
    clear();
    move(LINES - 2, 0);
    printw("%s", s);
    wrefresh(stdscr);
    endwin();
    printf("\n");       /* So the cursor doesn't stop at the end of the line */
    exit(100);
}

/*
    rnd()
        Pick a very random number.
*/

unsigned char
ucrnd(unsigned char range)
{
    return (unsigned char)(range <= 0 ? 0 : (ur_random() & 0x7fffffffL) % range);
}

short
srnd(short range)
{
    return (short)(range <= 0 ? 0 : (ur_random() & 0x7fffffffL) % range);
}

int
rnd(int range)
{
    return (range <= 0 ? 0 : (ur_random() & 0x7fffffffL) % range);
}

unsigned long
ulrnd(unsigned long range)
{
    return(range <= 0 ? 0 : (ur_random() & 0x7fffffffL) % range);
}

/*
    roll()
        roll a number of dice
*/

int
roll(int number, int sides)
{
    int dtotal = 0;

    while (number--)
        dtotal += rnd(sides) + 1;

    return(dtotal);
}
