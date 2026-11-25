/*
 * Last Modified: Jul 21, 1995
 */

/*
 * save and restore routines
 *
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "rogue.h"

static int  encwrite(char *start, long size, int outfd);
static int  encread(char *start, long size, int infd);
static int  putword(long word, int file);
static long getword(int fd);
static long save_size(void);

/*
 * A saved game is the game's variables from version to rg_edata and
 * from rg_bss to rg_ebss: vers.c is linked first and alloc.c last.
 */

extern char __executable_start[], _end[];
extern char rg_bss[], *const rg_edata, *const rg_ebss;
extern void rg_rebase(char *text, char *end, char *data, char *edata,
    char *bss, char *ebss);
extern int  rg_restored(void);

long save_stamp[2];                     /* tell one build from another */
char save_win[3][MAXLINES][MAXCOLS];    /* stdscr, mw and cw */
int  save_lines, save_cols;             /* and how big they were */

#ifdef __SANITIZE_ADDRESS__
#define NOCHECK __attribute__((no_sanitize_address))
extern int rg_read(int fd, void *buf, size_t n);
#define read    rg_read
#else
#define NOCHECK
#endif

/*
    save_ok()
        A saved game is the game's variables from version to rg_edata
        and from rg_bss to rg_ebss.  See that they are where that takes
        them to be, by one variable of each kind, and that save_win[]
        has room for the screen.
*/

static int
save_ok(void)
{
#ifdef MEMCHECK
    return(FALSE);      /* the storage is memory.c's, not the game's */
#endif
    return((char *) &foodlev >= version && (char *) &foodlev < rg_edata &&
        (char *) &save_lines >= rg_bss && (char *) &save_lines < rg_ebss &&
        LINES <= MAXLINES && COLS <= MAXCOLS);
}

/*
    save_wins()
        Copy what is in the windows that hold the level to save_win[],
        or back from it into the windows
*/

static void
save_wins(int back)
{
    int i, y, x;
    WINDOW *win;

    for (i = 0; i < 3; i++)
    {
        win = (i == 0 ? stdscr : i == 1 ? mw : cw);

        for (y = 0; y < LINES; y++)
            for (x = 0; x < COLS; x++)
                if (back)
                    mvwaddch(win, y, x, (unsigned char) save_win[i][y][x]);
                else
                    save_win[i][y][x] = mvwinch(win, y, x) & A_CHARTEXT;
    }
}

int
save_game(void)
{
    FILE *savefd;
    char    buf[2 * LINELEN];
    char    oldfile[2*LINELEN];
    if (!save_ok())
    {
        msg("This rogue cannot save a game");
        return(FALSE);
    }

    /* get file name */

    strcpy(oldfile,file_name);

    do
    {
        mpos = 0;

        if (oldfile[0] != '\0')
            msg("Save file [%s]: ", file_name);
        else
            msg("Save file as: ");

        mpos = 0;
        buf[0] = '\0';

        if (get_string(buf, cw) == QUIT)
        {
            msg("");
            return(FALSE);
        }

        if ( (buf[0] == 0) && (oldfile[0] != 0) )
            strcpy(file_name, oldfile);
        else if (buf[0] != 0)
            strcpy(file_name, buf);
        else
        {
            msg("");
            return(FALSE);
        }

        wclear(hw);
        wmove(hw, LINES - 1, 0);
        wrefresh(hw);

        if ((savefd = fopen(file_name, "w")) == NULL)
            msg(strerror(errno));    /* fake perror() */
    }
    while (savefd == NULL);

    /* write out [compressed?] file */

    save_file(savefd);
    return(TRUE);
}

/*
    save_file()
        Write out the game, encrypted: its variables in two pieces, with
        what the windows hold and stamps that tell this build from others
*/

void
save_file(FILE *savefd)
{
    int fd = fileno(savefd);

    wmove(cw, LINES - 1, 0);
    wrefresh(cw);
    fflush(savefd);
    save_lines = LINES;
    save_cols = COLS;
    save_wins(FALSE);
    save_stamp[0] = (long) main - (long) version;
    save_stamp[1] = (long) rg_restored - (long) __executable_start;
    encwrite(version, rg_edata - version, fd);
    encwrite(rg_bss, rg_ebss - rg_bss, fd);
    fclose(savefd);
}

int
restore(char *file)
{
    int     infd;
    char    *sp;
    char    buf[2 * LINELEN], name[2 * LINELEN];
    struct stat sbuf;
    int     y, x;
    FILE    *score = fd_score;  /* main()'s, not the saved game's */

    if (strcmp(file, "-r") == 0)
        file = strcpy(name, file_name);

    if ((infd = open(file, O_RDONLY)) < 0)
    {
        endwin();
        perror(file);
        return(FALSE);
    }

    fflush(stdout);
    encread(buf, (long) strlen(version) + 1, infd);

    if (strcmp(buf, version) != 0)
    {
        endwin();
        printf("Save Game Version: %s\nReal Game Version: %s\n", buf, version);
        printf("Sorry, saved game is out of date.\n");
        return(FALSE);
    }

    fstat(infd, &sbuf);

    if (sbuf.st_size != save_size())
    {
        endwin();
        printf("Sorry, saved game is out of date.\n");
        return(FALSE);
    }

    fflush(stdout);
    lseek(infd, 0L, SEEK_SET);

    if (encread(version, rg_edata - version, infd) != rg_edata - version ||
        strcmp(buf, version) != 0 ||
        encread(rg_bss, rg_ebss - rg_bss, infd) != rg_ebss - rg_bss)
    {
        endwin();
        printf("Sorry, saved game is damaged.\n");
        return(FALSE);
    }

    if (save_stamp[0] != (long) main - (long) version ||
        save_stamp[1] != (long) rg_restored - (long) __executable_start)
    {
        endwin();
        printf("Sorry, saved game is out of date.\n");
        return(FALSE);
    }

    rg_rebase(__executable_start, _end, version, rg_edata, rg_bss, rg_ebss);

    if (!rg_restored())
    {
        endwin();
        printf("Sorry, saved game is damaged.\n");
        return(FALSE);
    }

    fd_score = score;

    /*
     * The windows main() made are not the ones in the saved game,
     * which are gone.  Make them again and put back what they held.
     */

    getmaxyx(stdscr, y, x);

    if (y < save_lines || x < save_cols)
    {
        endwin();
        printf("Sorry, this saved game needs a terminal with %d lines and "
            "%d columns.\n", save_lines, save_cols);
        return(FALSE);
    }

    LINES = save_lines;
    COLS = save_cols;
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);
    save_wins(TRUE);

    /*
     * we do not close the file so that we will have a hold of the inode
     * for as long as possible
     */

    mpos = 0;
    mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf.st_mtime));

    if (remove(file) < 0)
    {
        endwin();
        printf("Cannot unlink file\n");
        return(FALSE);
    }

    if ((sp = getenv("OPTIONS")) != NULL)
        parse_opts(sp);

    strcpy(file_name, file);

    clearok(cw, TRUE);
    touchwin(cw);
    noecho();
    nonl();

    while(playing)
    {
        do_daemons(BEFORE);
        do_fuses(BEFORE);

        command();  /* Command execution */

        if (after)
            do_after_effects();
    }

    fatal("");

    return(FALSE);
}

/*****************************************************************
 *
 *           encwrite, encread: encoded read/write routines
 *
 * The encwrite/encread routines implement a synchronous stream Vernam
 * cipher using a linear congruential random number generator to
 * simulate a one-time pad.  The random seed is encoded and stored in
 * the file using data diffusion (putword,getword).
 *
 * Usage:
 *      encwrite (start, size, outfd);
 *      char *start; long size; int outfd;
 *
 *      encread (start, size, infd);
 *      char *start; long size; int infd;
 *
 * HISTORY
 *
 * 07-May-86  Michael Laman (laman) at NCR Rancho Bernardo
 *      Modified for UltraRogue.  Removed all uses of standard I/O from
 *      data encryption routines.  This is most portable solution around
 *      a bug of limiting how many times a game may be saved.
 *
 * 03-Mar-85  Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Modified for UltraRogue.  Allowed multiple encwrites to a file by
 *      removing lseek calls and computing checksum while reading.
 *
 * 20-Dec-82  Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Created as a replacement for the encwrite/encread in
 *      Rogue 5.2, which are very easily broken.
 *
 *****************************************************************/

/* Constants for key generation */

#define OFFSET      667818L
#define MODULUS     894871L
#define MULT        2399L
#define ENCHAR      ((unsigned char) ((seed= ((seed*MULT+OFFSET)%MODULUS)) >> 10))

/* Constants for checksumming */

#define INITCK      1232531L
#define CKMOD       6506347L

struct wb
{
    int w1, w2, w3, w4;
};

/*****************************************************************
 * Encwrite: write a buffer to a file using data encryption
 *****************************************************************/

NOCHECK static int
encwrite(char *start, long size, int outfd)
{
    unsigned char *buf_p, *end_p;
    int ch;
    long cksum = INITCK, cnt, seed;
    char buffer[512 * 20];

    ur_srandom((unsigned) time(NULL)); /* Build a random seed */
    seed = (ur_random() & 0x7fff);

    putword(seed, outfd);  /* Write the random seed */
    putword(size, outfd);  /* Write the file length */

    /* Now checksum, encrypt, and write out the buffer. */

    while (size)
    {
        buf_p = (unsigned char *) buffer;
        cnt = min(size, (long) sizeof(buffer));
        end_p = (unsigned char *) &buffer[cnt];

        while (buf_p < end_p)
        {
            cksum = ((cksum << 8) + (ch = (unsigned char) *start++)) % CKMOD;
            *buf_p++ = ch ^ ENCHAR;
        }

        if (write(outfd, buffer, cnt) != cnt)
            return(0);

        size -= cnt;
    }

    putword(cksum, outfd); /* Write out the checksum */

    return(1);
}

/*****************************************************************
 * Encread: read a block of encrypted text from a file descriptor
 *****************************************************************/

NOCHECK static int
encread(char *start, long size, int infd)
{
    int length, ch;
    long stored;
    long seed, cnt, cksum = INITCK;

    seed = getword(infd);
    stored = getword(infd);

    if ((length = read(infd, start, (int) min(size, stored))) > 0)
    {
        for (cnt = length; cnt--; )
        {
            ch = (unsigned char) (*start++ ^= ENCHAR);
            cksum = ((cksum << 8) + ch) % CKMOD;
        }

        if ((length == stored) && (getword(infd) != cksum))
        {
            fprintf(stderr, "Sorry, file has been touched.\n");

            while (length--)
                *--start = '\0';    /* Zero the buffer */
        }
    }

    return(length);
}

/*****************************************************************
 * putword: Write out an encoded int word
 *****************************************************************/

static int
putword(long word, int file)
{
    struct wb w;

    w.w1 = rand();
    w.w2 = rand();
    w.w3 = rand();
    w.w4 = w.w1 ^ w.w2 ^ w.w3 ^ word;

    return(write(file, (char *) &w, sizeof(w)));
}

/*****************************************************************
 * getword: Read in an encoded int word
 *****************************************************************/

static long
getword(int fd)
{
    struct wb w;

    if (read(fd, (char *) &w, sizeof(w)) == sizeof(w))
        return(w.w1 ^ w.w2 ^ w.w3 ^ w.w4);
    else
        return(0);
}

/*
    save_size()
        How long a game saved by this build is: the two pieces of the
        game's data, each with the seed, length and checksum encwrite()
        puts around it
*/

static long
save_size(void)
{
    return((rg_edata - version) + (rg_ebss - rg_bss) + 6 * sizeof(struct wb));
}
