/*
 * Last Modified: Jul 14, 1997
 */

/*
 * File for the fun ends
 * Death or a total win
 *
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include "rogue.h"

static struct sc_ent
{
    int sc_lvl;
    long    sc_score;
    char    sc_name[76];
    long    sc_gold;
    int sc_flags;
    int sc_level;
    int   sc_artifacts;
    int   sc_monster;
} top_ten[10];
static void wait_return(void);
static void hold_sc(int fd);


static const char *rip[] =
{
    "                       __________",
    "                      /          \\",
    "                     /    REST    \\",
    "                    /      IN      \\",
    "                   /     PEACE      \\",
    "                  /                  \\",
    "                  |                  |",
    "                  |                  |",
    "                  |    killed by     |",
    "                  |                  |",
    "                  |       1993       |",
    "                 *|     *  *  *      | *",
    "       ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
    0
};

/*
    death()
        Do something really fun when he dies
*/

void
death(int monst)
{
    char **dp = (char **) rip, *killer;
    struct tm   *lt;
    time_t  date;
    char    buf[80];

    if (is_wearing(R_RESURRECT) || rnd(wizard ? 3 : 67) == 0)
    {
        int die = TRUE;

        if (resurrect-- == 0)
            msg("You've run out of lives.");
        else if (!save_resurrect(ring_value(R_RESURRECT)))
            msg("Your attempt to return from the grave fails.");
        else
        {
            struct linked_list  *item;
            struct linked_list  *next_item;
            struct object   *obj;
            int rm, flags, cnt;
            coord   pos;

            die = FALSE;
            msg("You feel a sudden warmth and then nothingness.");
            teleport();

            if (ring_value(R_RESURRECT) > 1 && rnd(10))
            {
                pstats.s_hpt = 2 * pstats.s_const;
                pstats.s_const = max(pstats.s_const - 1, 3);
            }
            else
            {
                for (item = pack; item != NULL; item = next_item)
                {
                    obj = OBJPTR(item);

                    if (obj->o_flags & ISOWNED || obj->o_flags & ISPROT)
                    {
                        next_item = next(item);
                        continue;
                    }

                    flags = obj->o_flags;
                    obj->o_flags &= ~ISCURSED;

                    if (obj == cur_armor)
                        cur_armor = NULL;

                    dropcheck(obj);
                    obj->o_flags = flags;
                    next_item = next(item);
                    rem_pack(obj);
                    item = make_item(obj);

                    if (obj->o_type == ARTIFACT)
                        has_artifact &= ~(1 << obj->o_which);
                    cnt = 0;

                    do
                    {
                        rm = rnd_room();
                        rnd_pos(&rooms[rm], &pos);
                    }
                    while(winat(pos.y, pos.x) != FLOOR && ++cnt <= 500);

                    if (cnt > 500)
                        pos = hero;

                    obj->o_pos = pos;
                    add_obj(item, obj->o_pos.y, obj->o_pos.x);
                }

                pstats.s_hpt = pstats.s_const;
                pstats.s_const = max(pstats.s_const - roll(2, 2), 3);
            }

            chg_str(roll(1, 4), TRUE, FALSE);
            pstats.s_lvl = max(pstats.s_lvl, 1);
            no_command += 2 + rnd(4);

            if (on(player, ISHUH))
                lengthen_fuse(unconfuse, rnd(8) + HUHDURATION);
            else
                light_fuse(unconfuse, 0, rnd(8) + HUHDURATION, AFTER);

            turn_on(player, ISHUH);
            light(&hero);
        }

        if (die)
        {
            wmove(cw, mpos, 0);
            waddstr(cw, morestr);
            touchwin(cw);
            wrefresh(cw);
            wait_for(' ');
        }
        else
            return;
    }

    signal(SIGHUP, SIG_IGN);    /* the score is still to be kept */

    time(&date);
    lt = localtime(&date);
    clear();
    wclear(cw);
    move(8, 0);

    while (*dp)
        printw("%s\n", *dp++);

    mvaddstr(14, 28 - ((int)(strlen(whoami) + 1) / 2), whoami);
    sprintf(buf, "%d+%ld Points", pstats.s_lvl, pstats.s_exp);
    mvaddstr(15, 28 - ((int)(strlen(buf) + 1) / 2), buf);
    killer = killname(monst,buf);
    mvaddstr(17, 28 - ((int)(strlen(killer) + 1) / 2), killer);
    mvaddstr(18, 26, (sprintf(prbuf, "%4d", 1900 + lt->tm_year), prbuf));
    move(LINES - 1, 0);

    mvaddstr(LINES - 1, 0, retstr);
    wait_return();
    idenpack();
    wrefresh(cw);
    refresh();

    score(pstats.s_exp, pstats.s_lvl, KILLED, monst);
    byebye();
}

/*
    wait_return()
        Wait for return before going on with the scores, but give up
        when input has ended, so that the score is still kept
*/

static void
wait_return(void)
{
    int c, cnt = 0;

    while ((c = getch()) != '\n' && c != '\r')
        if (c == ERR && cnt++ > 100)
            return;
}

/*
    hold_sc()
        Lock the score file while the table is read and written back.
        The lock goes when the file is closed.  Anyone who can read the
        file can hold a lock on it too, so try for a few seconds and
        then go on without one.
*/

static void
hold_sc(int fd)
{
    struct flock fl;
    int tries;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    for (tries = 0; tries < 6; tries++)
    {
        if (fcntl(fd, F_SETLK, &fl) == 0)
            return;

        sleep(1);
    }
}

/*
    score()
        figure score and post it.
*/

void
score(long amount, int lvl, int flags, int monst) /*ARGSUSED*/
{
    struct sc_ent   *scp=NULL, *sc2=NULL;
    int i;
    char    *killer;
    char buf[1024];
    struct rlimit lim;
    sigset_t set, oset;

    static const char *reason[] =
    {
        "killed",
        "quit",
        "a winner",
        "a total winner"
    };

    char    *packend;
    file_name[0] = '\0';   /* the game is over: nothing to save */
    signal(SIGHUP, SIG_IGN);    /* and the score is still to be kept */

    if (flags != WINNER && flags != TOTAL && flags != SCOREIT)
    {
        if (flags == CHICKEN)
            packend = "when you quit";
        else
            packend = "at your untimely demise";

        noecho();
        nl();
        refresh();
        showpack(packend);
    }

    /* Open file and read list */

    if (fd_score == NULL)
    {
        printf("No score file opened\n");
        return;
    }

    for (scp = top_ten; scp < &top_ten[10]; scp++)
    {
        scp->sc_lvl = 0L;
        scp->sc_score = 0L;

        for (i = 0; i < 76; i++)
            scp->sc_name[i] = ucrnd(255);

        scp->sc_gold = 0L;
        scp->sc_flags = rnd(10);
        scp->sc_level = rnd(10);
        scp->sc_monster = srnd(10);
        scp->sc_artifacts = 0;
    }

    if (flags != SCOREIT)
    {
        mvaddstr(LINES - 1, 0, retstr);
        refresh();
        fflush(stdout);
        wait_return();
    }
    if (!waswizard && flags != SCOREIT)
        hold_sc(fileno(fd_score));

    fseek(fd_score, 0L, SEEK_SET);
    i = fread(top_ten, 1, sizeof(top_ten), fd_score);

    /*
     * Keep the entries read whole that hold a score, one of the four
     * flags and a killer the game knows, and take the rest for empty
     */

    for (scp = sc2 = top_ten; scp < &top_ten[10]; scp++)
        if ((char *) (scp + 1) - (char *) top_ten <= i &&
            scp->sc_score != 0 && scp->sc_flags >= 0 && scp->sc_flags <= 3 &&
            scp->sc_monster >= D_CLUMSY && scp->sc_monster <= nummonst + 1)
            {
            scp->sc_name[sizeof(scp->sc_name) - 1] = '\0';
            *sc2++ = *scp;
        }
    for (; sc2 < &top_ten[10]; sc2++)
    {
        sc2->sc_lvl = 0;
        sc2->sc_score = 0;
        sc2->sc_artifacts = 0;
    }

    sc2 = NULL;

    /* Insert player in list if need be */

    if (!waswizard)
    {
        for (scp = top_ten; scp < &top_ten[10]; scp++)
        {
            if (lvl > scp->sc_lvl)
                break;

            if (lvl == scp->sc_lvl && amount > scp->sc_score)
                break;
        }

        if (scp < &top_ten[10])
        {
            sc2 = &top_ten[9];	/* the last entry makes room */

            while (sc2 > scp)
            {
                *sc2 = sc2[-1];
                sc2--;
            }
            
			scp->sc_lvl = lvl;
			scp->sc_gold = purse;
            scp->sc_score = amount;
            sprintf(buf, "%.*s, %s", (int) sizeof(scp->sc_name) - 20, whoami,
                    which_class(player.t_ctype));
            strncpy(scp->sc_name, buf, sizeof(scp->sc_name) - 1);
            scp->sc_name[sizeof(scp->sc_name) - 1] = '\0';
            scp->sc_flags = flags;
			
            if (flags == WINNER)
                scp->sc_level = max_level;
            else
                scp->sc_level = level;
				
            scp->sc_monster = monst;
	    scp->sc_artifacts = has_artifact;

            sc2 = scp;
        }
    }
    /* Leave the table alone if a limit on the size of a file cuts it short */

    /*
     * Leave the table alone if a limit on the size of a file cuts it
     * short, and hold back the signals that would end the game while
     * it is written
     */

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGQUIT);
    sigprocmask(SIG_BLOCK, &set, &oset);if (sc2 != NULL &&
        !(getrlimit(RLIMIT_FSIZE, &lim) == 0 && lim.rlim_cur < sizeof(top_ten)))
        {
        fseek(fd_score, 0L, SEEK_SET);
        /* Update the list file */
        fwrite(top_ten, sizeof(top_ten), 1, fd_score);
    }

    fclose(fd_score);  /* which lets go of the lock */
    fd_score = NULL;
    sigprocmask(SIG_SETMASK, &oset, NULL);

    if (flags != SCOREIT)
    {
        clear();
        refresh();
        endwin();
    }

    /* Print the list */

    printf("\nTop Ten Adventurers:\n%4s %15s %10s %s\n",
	    "Rank", "Score", "Gold", "Name");

    for (scp = top_ten; scp < &top_ten[10]; scp++)
    {
        if (scp->sc_score)
        {
		    char lev[20];
                        scp->sc_name[sizeof(scp->sc_name) - 1] = '\0';

                        for (i = 0; scp->sc_name[i] != '\0'; i++)
                            if (scp->sc_name[i] < ' ' || scp->sc_name[i] > '~')
                                scp->sc_name[i] = '?';
			
			sprintf(lev, "%d+%ld", scp->sc_lvl, scp->sc_score);
            printf("%4d %15s %10ld %s:", (int) (scp - top_ten + 1), 
			       lev,
                   scp->sc_gold,
                   scp->sc_name);

            if (scp->sc_artifacts)
            {
                char  thangs[80];
                int   n;
                int   first = TRUE;

                thangs[0] = '\0';

                for (n = 0; n <= maxartifact; n++)
                {
                    if (scp->sc_artifacts & (1 << n))
                    {
                        if (strlen(thangs))
                            strcat(thangs, ", ");

                        if (first)
                        {
                            strcat(thangs, "retrieved ");
                            first = FALSE;
                        }

                        if (45 - strlen(thangs) < strlen(arts[n].ar_name))
                        {
                            printf("%s\n%32s", thangs," ");
                            thangs[0] = '\0';
                        }
                        strcat(thangs, arts[n].ar_name);
                    }
                }

                if (strlen(thangs))
                    printf("%s,", thangs);

                printf("\n%32s"," ");
            }

            printf("%s on level %d",reason[scp->sc_flags],scp->sc_level);

            if (scp->sc_flags == 0)
            {
                printf(" by \n%32s"," ");
                killer = killname(scp->sc_monster, buf);
                printf(" %s", killer);
            }

            putchar('\n');
        }
    }

    /* Leave the table alone if a limit on the size of a file cuts it short */

}

void
total_winner(void)
{
    struct linked_list  *item;
    struct object   *obj;
    int worth, oldpurse;
    char    c;
    struct linked_list  *bag = NULL;
    file_name[0] = '\0';   /* the game is over: nothing to save */

    clear();
    standout();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr("     Congratulations, you have made it to the light of day!    \n");
    standend();
    addstr("\nYou have joined the elite ranks of those who have \n");
    addstr("escaped the Dungeons of Doom alive.  You journey home \n");
    addstr("and sell all your loot at a great profit.\n");
    addstr("The White Council approves the recommendation of\n");

    if (player.t_ctype == C_FIGHTER)
        addstr("the fighters guild and appoints you Lord Protector\n");
    else if (player.t_ctype == C_ASSASIN)
        addstr("the assassins guild and appoints you Master Murderer\n");
    else if (player.t_ctype == C_NINJA)
        addstr("the ninja guild and appoints you Master of the Wind\n");
    else if (player.t_ctype == C_ILLUSION)
        addstr("the illusionists guild and appoints you Master Wizard\n");
    else if (player.t_ctype == C_MAGICIAN)
        addstr("the magicians guild and appoints you Master Wizard\n");
    else if (player.t_ctype == C_CLERIC)
        addstr("the temple priests and appoints you Master of the Flowers\n");
    else if (player.t_ctype == C_DRUID)
        addstr("the temple priests and appoints you Master of the Flowers\n");
    else if (player.t_ctype == C_RANGER)
        addstr("the rangers guild and appoints you Master Ranger\n");
    else if (player.t_ctype == C_PALADIN)
        addstr("the paladins guild and appoints you Master Paladin\n");
    else if (player.t_ctype == C_THIEF)
    {
        addstr("the thieves guild under protest and appoints you\n");
        addstr("Master of the Highways\n");
    }

    addstr("of the Land Between the Mountains.\n");
    mvaddstr(LINES - 1, 0, spacemsg);
    refresh();
    wait_for(' ');
    clear();
    idenpack();
    oldpurse = purse;
    mvaddstr(0, 0, "   Worth  Item");

    for (c = 'a', item = pack; item != NULL; c++, item = next(item))
    {
        obj = OBJPTR(item);
        worth = get_worth(obj);
        purse += worth;

        if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE)
            bag = obj->o_bag;

        mvprintw(c - 'a' + 1, 0, "%c) %8d  %s", c,
             worth, inv_name(obj, UPPERCASE));
    }

    if (bag != NULL)
    {
        mvaddstr(LINES - 1, 0, morestr);
        refresh();
        wait_for(' ');
        clear();
        mvprintw(0, 0, "Contents of the Magic Purse of Yendor:\n");

        for (c = 'a', item = bag; item != NULL; c++, item = next(item))
        {
            obj = OBJPTR(item);
            worth = get_worth(obj);
            whatis(item);
            purse += worth;
            mvprintw(c - 'a' + 1, 0, "%c) %8d %s\n", c,
                 worth, inv_name(obj, UPPERCASE));
        }
    }

    mvprintw(c - 'a' + 1, 0, "   %6d  Gold Pieces          ", oldpurse);
    refresh();

    if (has_artifact == 255)
        score(pstats.s_exp, pstats.s_lvl, TOTAL, 0);
    else
        score(pstats.s_exp, pstats.s_lvl, WINNER, 0);

    byebye();
}

char *
killname(int monst, char *buf)
{
    if (buf == NULL)
        return("A bug in UltraRogue #102");

    if (monst >= 0)
    {
        switch (monsters[monst].m_name[0])
        {
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
                sprintf(buf, "an %s", monsters[monst].m_name);
                break;
            default:
                sprintf(buf, "a %s", monsters[monst].m_name);
        }

        return(buf);
    }
    else
        switch(monst)
        {
            case D_ARROW:
                strcpy(buf, "an arrow"); break;
            case D_DART:
                strcpy(buf, "a dart"); break;
            case D_BOLT:
                strcpy(buf, "a bolt"); break;
            case D_POISON:
                strcpy(buf, "poison"); break;
            case D_POTION:
                strcpy(buf, "a cursed potion"); break;
            case D_PETRIFY:
                strcpy(buf, "petrification"); break;
            case D_SUFFOCATION:
                strcpy(buf, "suffocation"); break;
            case D_INFESTATION:
                strcpy(buf, "a parasite"); break;
            case D_DROWN:
                strcpy(buf, "drowning"); break;
            case D_FALL:
                strcpy(buf, "falling"); break;
            case D_FIRE:
                strcpy(buf, "slow boiling in oil"); break;
            case D_SPELLFUMBLE:
                strcpy(buf, "a botched spell"); break;
            case D_DRAINLIFE:
                strcpy(buf, "a drain life spell"); break;
            case D_ARTIFACT:
                strcpy(buf, "an artifact of the gods"); break;
            case D_GODWRATH:
                strcpy(buf, "divine retribution"); break;
            case D_CLUMSY:
                strcpy(buf, "excessive clumsyness"); break;
            default:
                strcpy(buf, "stupidity"); break;
        }

    return(buf);
}

/*
    showpack()
        Display the contents of the hero's pack
*/

void
showpack(char *howso)
{
    char    *iname;
    unsigned int worth;
    int cnt, ch, oldpurse;
    struct linked_list  *item;
    struct object   *obj;
    struct linked_list  *bag = NULL;

    cnt = 1;
    clear();
    mvprintw(0, 0, "Contents of your pack %s:\n", howso);
    ch = 0;
    oldpurse = purse;
    purse = 0;

    for (item = pack; item != NULL; item = next(item))
    {
        obj = OBJPTR(item);
        worth = get_worth(obj);
        whatis(item);
        purse += worth;

        if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE)
            bag = obj->o_bag;

        iname = inv_name(obj, UPPERCASE);
        mvprintw(cnt, 0, "%d) %s\n", ch, iname);
        ch += 1;

        if (++cnt > LINES - 5 && next(item) != NULL)
        {
            cnt = 1;
            mvaddstr(LINES - 1, 0, morestr);
            refresh();
            wait_for(' ');
            clear();
        }
    }

    if (bag != NULL)
    {
        mvaddstr(LINES - 1, 0, morestr);
        refresh();
        wait_for(' ');
        clear();
        cnt = 1;
        ch = 0;

        mvprintw(0, 0, "Contents of the Magic Purse of Yendor %s:\n", howso);

        for (item = bag; item != NULL; item = next(item))
        {
            obj = OBJPTR(item);
            worth = get_worth(obj);
            whatis(item);
            purse += worth;
            mvprintw(cnt, 0, "%d) %s\n", ch, inv_name(obj, UPPERCASE));
            ch += 1;

            if (++cnt > LINES - 5 && next(item) != NULL)
            {
                cnt = 1;
                mvaddstr(LINES - 1, 0, morestr);
                refresh();
                wait_for(' ');
                clear();
            }
        }
    }

    mvprintw(cnt + 1, 0, "Carrying %d gold pieces", oldpurse);
    mvprintw(cnt + 2, 0, "Carrying objects worth %ld gold pieces", purse);
    purse += oldpurse;
    refresh();
}

void
byebye(void)
{
    endwin();
    printf("\n");
    exit(0);
}

/*
    save_resurrect()
        chance of resurrection according to modifed D&D probabilities
*/

int
save_resurrect(int bonus)
{
    int need, adjust;

    adjust = pstats.s_const + bonus - luck;

    if (adjust > 17)
        return(TRUE);
    else if (adjust < 14)
        need = 5 * (adjust + 5);
    else
        need = 90 + 2 * (adjust - 13);

    return(roll(1, 100) < need);
}
