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
#include <sys/types.h>
#include <sys/stat.h>
#include "rogue.h"

extern char *sbrk();

static int  encwrite(char *start, long size, int outfd);
static int  encread(char *start, long size, int infd);
static int  putword(long word, int file);
static long getword(int fd);

int
save_game(void)
{
    FILE *savefd;
    int c;
    char    buf[2 * LINELEN];
    char    oldfile[2*LINELEN];

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
        Write out the game: everything from version[] up to the break,
        encrypted
*/

void
save_file(FILE *savefd)
{
    int fd = fileno(savefd);

    wmove(cw, LINES - 1, 0);
    wrefresh(cw);
    fflush(savefd);
    encwrite(version, (char *) sbrk(0) - version, fd);
    fclose(savefd);
}

int
restore(char *file)
{
    int     infd;
    char    *sp;
    char    buf[2 * LINELEN];
    struct stat sbuf;

    if (strcmp(file, "-r") == 0)
        file = file_name;

    if ((infd = open(file, O_RDONLY)) < 0)
    {
        perror(file);
        return(FALSE);
    }

    fflush(stdout);
    encread(buf, (long) strlen(version) + 1, infd);

    if (strcmp(buf, version) != 0)
    {
        printf("Save Game Version: %s\nReal Game Version: %s\n", buf, version);
        printf("Sorry, saved game is out of date.\n");
        return(FALSE);
    }

    fstat(infd, &sbuf);
    fflush(stdout);
    brk(version + sbuf.st_size);
    lseek(infd, 0L, SEEK_SET);
    encread(version, (long) sbuf.st_size, infd);

    /*
     * we do not close the file so that we will have a hold of the inode
     * for as long as possible
     */

    mpos = 0;
    mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf.st_mtime));

    if (remove(file) < 0)
    {
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

static int
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

static int
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
