/***************************************************************************
*   Originai Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*   Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                          *
*   Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*   Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                          *
*   In order to use any part of this Merc Diku Mud, you must comply with   *
*   both the original Diku license in 'license.doc' as well the Merc       *
*   license in 'license.txt'.  In particular, you may not remove either of *
*   these copyright notices.                                               *
*                                                                          *
*   Much time and thought has gone into this software and you are          *
*   benefitting.  We hope that you share your changes too.  What goes      *
*   around, comes around.                                                  *
***************************************************************************/
/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file 'rom.license'                             *
***************************************************************************/
/***************************************************************************
*       ROT 2.0 is copyright 1996-1999 by Russ Walsh                       *
*       By using this code, you have agreed to follow the terms of the     *
*       ROT license, in the file 'rot.license'                             *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
	#include <sys/types.h>
	#if defined(WIN32)
	#else
		#include <sys/time.h>
	#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

/* command procedures needed */
DECLARE_DO_FUN(	do_showclass	);

void do_showclass(CHAR_DATA *ch, char *argument)
{
 char arg1[MAX_INPUT_LENGTH];
 char buf[MAX_STRING_LENGTH];

 int class,level,skill;

 int i;

 strcpy(buf,"");
 argument = one_argument(argument, arg1);

 if(arg1[0]=='\0')
 {
  send_to_char("Syntax: showclass [class]\n\r",ch);
  return;
 }
 if((class=class_lookup(arg1))==-1)
 {
  send_to_char("Class not found.\n\r",ch);
  return;
 }

 sprintf(buf,"{xSpells/skills for %s:{x\n\r",class_table[class].name);
 send_to_char(buf,ch);
 i=0;
 for(level=1;level<=LEVEL_HERO;level++)
 {
  for(skill=0;skill<MAX_SKILL;skill++)
  {
   if(skill_table[skill].skill_level[class]!=level)
    continue;
   i++;
   sprintf(buf,"{CLevel %3d: {x: {c%-20s{x  ",
    level,skill_table[skill].name);
   send_to_char(buf,ch);
   if(i==2)
   {
    send_to_char("\n\r",ch);
    i=0;
   }
  }
 }
}

