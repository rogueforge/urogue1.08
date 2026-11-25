/*
 * Last Modified: Jul 21, 1995
 */

/*
 * random and associated routines lifted from 4.2BSD
 */

#include <stdlib.h>
#include "rogue.h"

void
ur_srandom(unsigned x)
{
    srandom(x);
}

long
ur_random(void)
{
    return( random() );
}
