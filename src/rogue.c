/*
 * Last Modified: Jul 21, 1995
 */
#include "rogue.h"
/*
 * Now all the global variables
 */
struct trap traps[2*MAXTRAPS];		/* twice as much for special effects */
struct room rooms[MAXROOMS];		/* One for each room -- A level */
struct room *oldrp;			/* Roomin(&player.t_oldpos) */
struct thing player;			/* The rogue */
struct thing *beast;			/* The last beast that attacked us */
struct object *cur_armor;		/* What a well dresssed rogue wears */
struct object *cur_weapon = NULL;	/* ... and wields */
struct object *cur_ring[10];		/* His rings */
struct object *cur_shield = NULL;	/* future expansion */
struct object *cur_cloak = NULL;
struct object *cur_shoes = NULL;
struct object *cur_hat = NULL;
struct object *cur_charm = NULL;	/* Currently charmed monsters */
struct linked_list *fam_ptr = NULL;	/* A ptr to the familiar */
struct linked_list *lvl_obj = NULL; 	/* Treasure on this level */
struct linked_list *mlist = NULL;	/* Monsters on this level */
struct linked_list *curr_mons = NULL;	/* The mons. currently moving */
struct linked_list *next_mons = NULL;	/* The mons. after curr_mons */

/*
 * Command line options
 */
int prnews;				/* Print news file */

int char_type = C_NOTSET;		/* what type of character is player */
int foodlev = 1;			/* how fast he eats food */
int ntraps;				/* Number of traps on this level */
int seed;				/* Random number seed */
int dnum;				/* Dungeon number */
int max_level;				/* Deepest player has gone */
int lost_dext;				/* amount of lost dexterity */
int no_command = 0;
int level = 0;
long purse = 0;
int inpack = 0;
unsigned long total = 0;
int see_dist = 3;
int no_food = 0;
int count = 0;
int food_left = HUNGERTIME;
int group = 1;
int hungry_state = F_OK;
int infest_dam=0;
int lost_str=0;
int lastscore = -1;
int hold_count = 0;
int trap_tries = 0;
int has_artifact = 0;
int picked_artifact = 0;
int msg_index = 0;
int luck = 0;
int resurrect = 0;
int inv_type = INV_CLEAR;		/* Overwrite style of inventory */
int fam_type = 0;			/* The type of familiar */
int mons_summoned = 0;			/* Number of summoned monsters */
char *lastfmt;
char *lastarg;
char curpurch[15];			/* name of item ready to buy */
char PLAYER = VPLAYER;			/* what the player looks like */
char take;				/* Thing the rogue is taking */
char outbuf[LINELEN];			/* Output buffer for stdout */
char runch;				/* Direction player is running */
char whoami[2*LINELEN];			/* Name of player */
char fruit[2*LINELEN];			/* Favorite fruit */
char msgbuf[10][2*LINELEN];		/* message buffer */
char file_name[2*LINELEN];		/* Save file name */
char score_file[2*LINELEN];		/* Score file name */
char home[2*LINELEN];			/* User's home directory */
WINDOW *cw;				/* Window that the player sees */
WINDOW *hw;				/* Used for the help command */
WINDOW *mw;				/* Used to store mosnters */
int pool_teleport = FALSE;		/* just teleported from a pool */
int inwhgt = FALSE;			/* true if from wghtchk() */
int after;				/* True if we want after daemons */
int waswizard;				/* Was a wizard sometime */
int canwizard;				/* Will be permitted to do this */
int playing = TRUE; 
int running = FALSE; 
int fighting = FALSE; 
int wizard = FALSE;
int wiz_verbose = TRUE;

int terse = FALSE; 
int door_stop = FALSE;
int jump = TRUE; 
int doorstop = TRUE;
int firstmove = FALSE; 
int askme = FALSE;
int moving = FALSE;
int in_shell = FALSE; 
coord delta;				/* Change indicated to get_dir() */
LEVTYPE levtype;			/* type of level i'm on */

