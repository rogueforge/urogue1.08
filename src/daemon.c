/*
 * Last Modified: Jul 21, 1995
 */

/*
 * Contains functions for dealing with things that happen in the
 * future.
 *
 */

#include "rogue.h"

int demoncnt;

struct delayed_action d_list[MAXDAEMONS];

/*
    d_slot()
        Find an empty slot in the daemon/fuse list
*/

struct delayed_action *
d_slot(void)
{
    int i;
    struct delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
        if (dev->d_type == EMPTY)
            return(dev);

    return(NULL);
}

/*
     find_slot()
         Find a particular fuse in the table
*/

struct delayed_action *
find_slot(fuse (*func)(fuse_arg *arg))
{
    int i;
    struct delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
        if ( (dev->d_type == FUSE) && (func == dev->d_fuse) )
            return(dev);

    return(NULL);
}

/*
    daemon()
        Start a daemon, takes a function.
*/

void
start_daemon(daemon_t (*func)(daemon_arg *arg), void *arg, int whendo)
{
    struct delayed_action *dev;

    dev = d_slot();

    if (dev != NULL)
    {
        dev->d_type = DAEMON;
        dev->d_when = whendo;
        dev->d_daemon = func;
        dev->d_arg = arg;
        dev->d_time = 1;
        demoncnt += 1;  /* update count */
    }
}

/*
    kill_daemon()
        Remove a daemon from the list
*/

void
kill_daemon(daemon_t (*func)(daemon_arg *arg))
{
    int i;
    struct delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
        if ( (dev->d_type == DAEMON) && (func == dev->d_daemon) )
        {
            /* Take it out of the list */

            dev->d_type = EMPTY;
            demoncnt -= 1;      /* update count */
            return;
        }
}

/*
    do_daemons()
        Run all the daemons that are active with the current flag,
        passing the argument to the function.
*/

void
do_daemons(int now)
{
    struct delayed_action *dev;

    /* Loop through the devil list */

    for (dev = d_list; dev < &d_list[MAXDAEMONS]; dev++)

        /* Executing each one, giving it the proper arguments */

        if ( (dev->d_when == now) && (dev->d_type == DAEMON))
        {
            daemon_arg arg;

            arg.varg = dev->d_arg;
            (*dev->d_daemon)(&arg);
        }
}

/*
    fuse()
        Start a fuse to go off in a certain number of turns
*/

void
light_fuse(fuse (*func)(fuse_arg *arg), void *arg, int time, int whendo)
{
    struct delayed_action   *wire;

    wire = d_slot();

    if (wire != NULL)
    {
        wire->d_type = FUSE;
        wire->d_when = whendo;
        wire->d_fuse = func;
        wire->d_arg  = arg;
        wire->d_time = time;
        demoncnt += 1;  /* update count */
    }
}

/*
    lengthen()
        Increase the time until a fuse goes off
*/

void
lengthen_fuse(fuse (*func)(fuse_arg *arg), int xtime)
{
    struct delayed_action *wire;

    if ((wire = find_slot(func)) == NULL)
        return;

    wire->d_time += xtime;

    return;
}

/*
    extinguish()
        Put out a fuse
*/

void
extinguish_fuse(fuse (*func)(fuse_arg *arg))
{
    struct delayed_action   *wire;

    if ((wire = find_slot(func)) == NULL)
        return;

    wire->d_type = EMPTY;
    demoncnt -= 1;

    return;
}

/*
    do_fuses()
        Decrement counters and start needed fuses
*/

void
do_fuses(int now)
{
    struct delayed_action   *wire;

    /* Step though the list */

    for (wire = d_list; wire < &d_list[MAXDAEMONS]; wire++)
    {
        /*
         * Decrementing counters and starting things we want.  We
         * also need to remove the fuse from the list once it has
         * gone off.
         */

        if( (wire->d_type == FUSE) && (wire->d_when == now) )
        {
            if (--wire->d_time <= 0)
            {
                fuse_arg arg;

                arg.varg = wire->d_arg;
                wire->d_type = EMPTY;
                (*wire->d_fuse)(&arg);
                demoncnt -= 1;
            }
        }
    }

    return;
}

/*
    activity()
        Show wizard number of demaons and memory blocks used
*/

void
activity(void)
{
    msg("Daemons = %d : Memory Items = %d ", demoncnt, total);
    return;
}

/*
    waste_time()
        Do nothing but let other things happen
*/

void
waste_time(void)
{
    if (inwhgt)     /* if from wghtchk then done */
        return;

    do_daemons(BEFORE);
    do_fuses(BEFORE);
    do_daemons(AFTER);
    do_fuses(AFTER);
}
