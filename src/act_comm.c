/***************************************************************************
*   Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
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
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "interp.h"

/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MIL];

   if (IS_NPC(ch))
	return;
  
   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
	    if (is_clead(ch))
		update_clanlist(ch, ch->clead, FALSE, TRUE);
	    if (is_clan(ch))
		update_clanlist(ch, ch->clead, FALSE, FALSE);
	    if (ch->shadow)
	    {
		ch->shadowing->shadowed = FALSE;
		ch->shadowing->shadower = NULL;
		ch->shadowing = NULL;
		ch->shadow = FALSE;
	    }
    	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
 	    stop_fighting(ch,TRUE);
	    if (ch->level > HERO)
	    {
		update_wizlist(ch, 1);
	    }
	    do_function(ch, do_quit,"");
	    unlink(strsave);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
}

void do_rerol( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to reroll yourself.\n\r",ch);
}

void do_reroll( CHAR_DATA *ch, char *argument)
{
   char strsave[MIL];

   if (IS_NPC(ch))
	return;

   if ((ch->level < 30) && (ch->pcdata->tier < 1 )) {
	      send_to_char("You must be at least level 30 to reroll.\n\r",ch);
		      return;
			    }
   if (ch->pcdata->confirm_reroll)
   {
	DESCRIPTOR_DATA *d;
	bool found = FALSE;

	if (argument[0] != '\0')
	{
	    send_to_char("Reroll status removed.\n\r",ch);
	    ch->pcdata->confirm_reroll = FALSE;
	    return;
	}
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING &&
	    d->character == ch)
	    {
		found = TRUE;
		break;
	    }
	}
	if (!found)
	{
	    send_to_char( "Can't find your descriptor, aborted\n\r",ch);
	    return;
	}

	if (is_clead(ch))
	    update_clanlist(ch, ch->clead, FALSE, TRUE);
	if (is_clan(ch))
	    update_clanlist(ch, ch->clead, FALSE, FALSE);
	if (ch->shadow)
	{
	    ch->shadowing->shadowed = FALSE;
	    ch->shadowing->shadower = NULL;
	    ch->shadowing = NULL;
	    ch->shadow = FALSE;
	}
	sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	wiznet("$N returns to $S birth.",ch,NULL,0,0,0);
	stop_fighting(ch,TRUE);
	if (ch->level > HERO)
	    update_wizlist(ch, 1);
	if (ch->pcdata->tier == 1)
	    SET_BIT( ch->act, PLR_SECOND );
	if (ch->pcdata->tier == 2)
	    SET_BIT( ch->act, PLR_THIRD );
	send_to_char("Hit return to continue.\n\r", ch);
        ch->was_in_room = ch->in_room;
        char_from_room( ch );
	if ( ch == char_list )
	{
	    char_list = ch->next;
	}
	else
	{
	    CHAR_DATA *prev;
 
	    for ( prev = char_list; prev != NULL; prev = prev->next )
	    {
		if ( prev->next == ch )
		{
		    prev->next = ch->next;
		    break;
		}
	    }
 
	    if ( prev == NULL )
	    {
		bug( "Extract_char: char not found.", 0 );
		return;
	    }
	}

	d->connected = CON_REROLL;
	return;
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type reroll. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type reroll again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible,\n\r",ch);
    send_to_char("and will set you back to level 1, as a newbie!\n\r",ch);
    send_to_char("You will lose {B{zEVERYTHING{x you are carrying!\n\r",ch);
    if (IS_HERO(ch))
    {
	send_to_char("{RIf you are moving to the next tier, see {zADVANCE{x{R instead!{x\n\r",ch);
    }
    send_to_char("Typing reroll with an argument will undo reroll status.\n\r",
	ch);
    ch->pcdata->confirm_reroll = TRUE;
    wiznet("$N is contemplating rerolling.",ch,NULL,0,0,get_trust(ch));
}


void do_route( CHAR_DATA *ch, char *argument)
{
    if ( !str_cmp( argument, "cleric") )
    {
    	send_to_char("Go 17 east, 3 north, west, 2 south then down.\n\r",ch);
    }
else if ( !str_cmp(argument, "astral plane") ) { send_to_char("Astral plane: 1n 3u 1n 6u 2n .\n\r",ch);}
else if ( !str_cmp(argument, "camelot") ) { send_to_char("Camelot: 7w 1n.\n\r",ch);}
else if ( !str_cmp(argument, "dark continent") ) { send_to_char("Dark Continent: 35e 1n 2e 3s.\n\r",ch);}
else if ( !str_cmp(argument, "drow city") ) { send_to_char("Drow City: 6e 4s 2e 2s 2e 1d 1w 1s 2d .\n\r",ch);}
else if ( !str_cmp(argument, "dwarven kingdom") ) { send_to_char("Dwarven Kingdom: 6e 3n 2e .\n\r",ch);}
else if ( !str_cmp(argument, "gnome village") ) { send_to_char("Gnome Village: 8e 1s .\n\r",ch);}
else if ( !str_cmp(argument, "holy grove") ) { send_to_char("Holy Grove: 8e 1n .\n\r",ch);}
else if ( !str_cmp(argument, "olympus") ) { send_to_char("Olympus: 3e 4n 2w 8n 2u .\n\r",ch);}
else if ( !str_cmp(argument, "mob factory") ) { send_to_char("Mob Factory: 3w 3s 1e .\n\r",ch);}
else if ( !str_cmp(argument, "moria") ) { send_to_char("Moria: 6e 6n .\n\r",ch);}
else if  ( !str_cmp( argument, "aerial city") ) { send_to_char("3e 4n 2w 7n 6e 4s 4e 1s.\n\r",ch); }
else if  ( !str_cmp( argument, "cloudy mountain") ) { send_to_char("3e 8s 2w 2s 2e 1s 4e.\n\r",ch); }
else if  ( !str_cmp( argument, "hell") ) { 
send_to_char("3e 7s 1w 6s 1d 1s 2w 2s 1e 1n 1d 1n 2e 1s 4e 2s 1w 2s 2e 1d 1n 1e 1s 1e 2s 1n.\n\r",ch); }
else if  ( !str_cmp( argument, "kerofk") ) { send_to_char("3e 4n 2w 7n 1e 2n 1e 1s 5e.\n\r",ch); }
else if  ( !str_cmp( argument, "stoneBow dale") ) { send_to_char("3e 8s 2w 8s.\n\r",ch); }
else if  ( !str_cmp( argument, "warrior") ) { send_to_char("Go 16 east, 2 south, east.\n\r",ch); }
else if ( !str_cmp(argument, "fanatics tower") ) { send_to_char("21e 8n 2w up the ladder, 3n.\n\r",ch); }
else if ( !str_cmp(argument, "thieves") ) { send_to_char("24e s open west, s d.\n\r",ch); }
else if ( !str_cmp(argument, "drakyri") ) { send_to_char("11w 2s 2w n 2w n w.\n\r",ch); }
else if ( !str_cmp(argument, "elemental") ) { send_to_char("6e 4s 2e s 2e d s 4u n d.\n\r",ch); }
/*

Nirvana: 8e 2n 2e 1u 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Ofcol: 3e 4n 2w 7n 6e 1n 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Pirate Ship: 34e 1n 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Pyramid: 1s 2e 2s 10e 1n 1e 1n 3e 1u 1n 1e 1u 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Reverse Palace: 13w 3n 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Shire: 5w 1n 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Smurfville: 7e 4n 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
The Marsh: 1s 2e 2s 6w 1n 1w 1s 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Troll Den: 1s 2e 2s 6w 1n 1e 1s 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Valley of the Titans: 13w 2n 1e 2n 4w 1n 2w 2n 2w 5s 4w 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
City of Anon: 3e 8s 2w 17s 12e 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Dangerous Neighborhood: 6s, e, s, 2e, s, e 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Divided Souls:  from Tavern in Shadow Grove, d, 2n, e 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Dragon Tower: in Shadow Grove 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Haon Dor:  6w 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Mahn-Tor:  6e, 4s, 2e, s, 2e, d, 3n, e, 3s, 3, 2s, w, 3s, d, 9s, 2u 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Mega-One-City:  s, 2e, s, Boat/Fly, s, 13e, s 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Mirror Realm:  6e, 2n, 3e, 2n 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Mutant Dump: 28e, 2n, e 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
New Thalos: 16e 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Plains of the North:  4e, 3n, 2w, 2n 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Sands of Sorrow:  s, 2e, s, Boat/Fly, s, 10e 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Sewers: 2s d 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Shadow Grove:  13w, s, 2w, 2s, w, s, 3w, n, w, n 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Valley of the Elves:  4e, 3n, 2w, 5n, w, 3n, d 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
The Void:  s, 3e, s, u 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Wyvern Tower:  6e, 4s, 2e, s, 2e, d, Boat/Fly, 7e 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
Abyss: 6e 4s 2e s 2e d s u n 
    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);

    else if ( !str_cmp(argument, "") ) {
    	send_to_char(".\n\r",ch);
    }
*/ 
   else if ( !str_cmp(argument, "all") )
    {
send_to_char("Moria: 6e 6n .\n\r",ch);
send_to_char("Camelot: 7w 1n.\n\r",ch);
send_to_char("Dark Continent: 35e 1n 2e 3s.\n\r",ch);
send_to_char("Drow City: 6e 4s 2e 2s 2e 1d 1w 1s 2d .\n\r",ch);
send_to_char("Dwarven Kingdom: 6e 3n 2e .\n\r",ch);
send_to_char("Gnome Village: 8e 1s .\n\r",ch);
send_to_char("Holy Grove: 8e 1n .\n\r",ch);
send_to_char("Olympus: 3e 4n 2w 8n 2u .\n\r",ch);
send_to_char("Mob Factory: 3w 3s 1e .\n\r",ch);
send_to_char("Astral plane: 1n 3u 1n 6u 2n .\n\r",ch);
    	send_to_char("thieves guild ",ch);
    	send_to_char("Go 24 east, south, open west, south then down.\n\r",ch);
    	send_to_char("warrior guild ",ch);
    	send_to_char("Go 16 east, 2 south, east.\n\r",ch);
    	send_to_char("clerics guild ",ch);
    	send_to_char("Go 17 east, 3 north, west, 2 south then down.\n\r",ch);
    	send_to_char("elemental ",ch);
    	send_to_char("6e 4s 2e s 2e d s 4 n d.\n\r",ch);
    	send_to_char("drakyri ",ch);
    	send_to_char("11w 2s 2w n 2w n w.\n\r",ch);
    	send_to_char("stoneBow dale ",ch);
    	send_to_char("3e 8s 2w 8s.\n\r",ch);
    	send_to_char("kerofk ",ch);
    	send_to_char("3e 4n 2w 7n 1e 2n 1e 1s 5e.\n\r",ch);
    	send_to_char("hell ",ch);
    	send_to_char("3e 7s 1w 6s 1d 1s 2w 2s 1e 1n 1d 1n 2e 1s 4e 2s 1w 2s 2e 1d 1n 1e 1s 1e 2s 1n.\n\r",ch);
    	send_to_char("aerial city ",ch);
    	send_to_char("3e 4n 2w 7n 6e 4s 4e 1s.\n\r",ch);
    	send_to_char("cloudy mountain ",ch);
    	send_to_char("3e 8s 2w 2s 2e 1s 4e.\n\r",ch);
    	send_to_char("fanatics tower ",ch);
	send_to_char("21e 8n 2w up the ladder, 3n.\n\r",ch);
    }
    else 
    {
    	send_to_char("Can't find that location, ",ch);
    	send_to_char("type 'route all' for all known directions.\n\r",ch);
    }
}

void do_advance( CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   char buf[MSL];
   char name[MSL];
   int old_tier;
   int sex[2];

   if (IS_NPC(ch))
	return;

   if ( ch->level < 101 ) {
	send_to_char("You must at least be a Hero to advance to next tier.\n\r",ch);
	return;
   }

   if (ch->pcdata->advanced != 0) {
	send_to_char("Sorry, only one advance per hero.\n\r",ch);
	return;
   }

   if (ch->pcdata->tier >= 2) {
	send_to_char("Sorry, no fifth tier is available (yet).\n\r",ch);
	return;
   }

   argument = one_argument( argument, arg );

   if (arg[0] == '\0') {
	send_to_char( "Syntax: advance <new character name>.\n\r", ch );
	return;
   }

   if ( check_char_exist( arg ) ) {
	send_to_char( "That name is already in use, or is illegal.\n\r", ch );
	return;
   }

   sex[0] = ch->sex;
   sex[1] = ch->pcdata->true_sex;
   old_tier = ch->pcdata->tier;
   sprintf(name, "%s", str_dup(ch->name));
   ch->name = str_dup(arg);
   ch->pcdata->tier++;

	if (ch->pcdata->tier == 1) SET_BIT( ch->act, PLR_SECOND );
	if (ch->pcdata->tier == 2) SET_BIT( ch->act, PLR_THIRD );
	save_char_obj( ch );
	REMOVE_BIT( ch->act, PLR_SECOND );
	REMOVE_BIT( ch->act, PLR_THIRD );

   buf[0] = '\0';
   strcat(buf, "A new pfile by the name ");
   strcat(buf, capitalize(ch->name));
   strcat(buf, " has been saved.\n\r");

   ch->name = str_dup(name);
   ch->sex = sex[0];
   ch->pcdata->true_sex = sex[1];
   ch->pcdata->tier = old_tier;
   send_to_char( buf, ch );
   buf[0] = '\0';
   if (ch->level >= LEVEL_HERO)
	ch->pcdata->advanced = 1;
   wiznet("$N has advanced to next tier.",ch,NULL,0,0,get_trust(ch));

   return;
}

/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{
    char buf[MSL];

    /* lists all channels and their status */
    send_to_char("   channel     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("ooc            ",ch);
    if (!IS_SET(ch->comm,COMM_NOOOC))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("clan gossip    ",ch);
    if (!IS_SET(ch->comm,COMM_NOCGOSSIP))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("quest gossip   ",ch);
    if (!IS_SET(ch->comm,COMM_NOCGOSSIP))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("auction        ",ch);
    if (!IS_SET(ch->comm,COMM_NOAUCTION))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("music          ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("Q/A            ",ch);
    if (!IS_SET(ch->comm,COMM_NOASK))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("Quote          ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUOTE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("grats          ",ch);
    if (!IS_SET(ch->comm,COMM_NOGRATS))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_IMMORTAL(ch))
    {
      send_to_char("god channel    ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
        send_to_char("ON\n\r",ch);
      else
        send_to_char("OFF\n\r",ch);
    }

    send_to_char("shouts         ",ch);
    if (!IS_SET(ch->comm,COMM_SHOUTSOFF))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("tells          ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("quiet mode     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_SET(ch->comm,COMM_AFK))
	send_to_char("You are AFK.\n\r",ch);

    if (IS_SET(ch->comm,COMM_STORE))
	send_to_char("You store tells during fights.\n\r",ch);

    if (IS_SET(ch->comm,COMM_TITLE_LOCK))
	send_to_char("Your title does not change when leveling.\n\r",ch);

    if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
	send_to_char("You are immune to snooping.\n\r",ch);
   
    if (ch->lines != PAGELEN)
    {
	if (ch->lines)
	{
	    sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
	    send_to_char(buf,ch);
 	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if (ch->prompt != NULL)
    {
	sprintf(buf,"Your current prompt is: %s\n\r",ch->prompt);
	send_to_char(buf,ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("You cannot shout.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot show emotions.\n\r",ch);

}

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear tells again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear tells.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* afk command */

void do_afk ( CHAR_DATA *ch, char * argument)
{
    char buf[MSL];

    if (IS_SET(ch->comm,COMM_AFK))
    {
      if(ch->tells)
      {
         sprintf( buf, "AFK mode removed.  You have {R%d{x tells waiting.\n\r", ch->tells );
         send_to_char( buf, ch );
         send_to_char("Type 'replay' to see tells.\n\r",ch);
      }
      else
      {
         send_to_char("AFK mode removed.  You have no tells waiting.\n\r",ch);
      }
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   else
   {
     send_to_char("You are now in AFK mode.\n\r",ch);
     SET_BIT(ch->comm,COMM_AFK);
   }
}

void do_autostore ( CHAR_DATA *ch, char * argument)
{
   if (IS_SET(ch->comm,COMM_STORE))
   {
     send_to_char("You will no longer store tells during fights.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_STORE);
   }
   else
   {
     send_to_char("You will now store tells during fights.\n\r",ch);
     SET_BIT(ch->comm,COMM_STORE);
   }
}

void do_replay (CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
	send_to_char("You can't replay.\n\r",ch);
	return;
    }

    if (buf_string(ch->pcdata->buffer)[0] == '\0')
    {
	send_to_char("You have no tells to replay.\n\r",ch);
	return;
    }

    page_to_char(buf_string(ch->pcdata->buffer),ch);
    clear_buf(ch->pcdata->buffer);
    ch->tells = 0;
}

/* 
void do_auction( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int wtime;

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOAUCTION))
      {
	send_to_char("Auction channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOAUCTION);
      }
      else
      {
	send_to_char("Auction channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOAUCTION);
      }
    }
    else 
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;

	REMOVE_BIT(ch->comm,COMM_NOAUCTION);
      }

      if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
      && (!IS_IMMORTAL(ch)))
      {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
      }

      sprintf( buf, "You auction '{A%s{x'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOAUCTION) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
		act_new("$n auctions '{A$t{x'",
		    ch,argument,d->character,TO_VICT,POS_DEAD);
	    }
 	}
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}
*/

void talk_auction (char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

    sprintf (buf,"AUCTION: %s", argument);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING) && !IS_SET(original->comm, COMM_NOAUCTION) )
            act (buf, original, NULL, NULL, TO_CHAR);

    }
}

void do_ooc( CHAR_DATA *ch, char *argument )
{
    //char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int wtime;

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOOOC))
      {
	send_to_char("Ooc channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOOOC);
      }
      else
      {
	send_to_char("Ooc channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOOOC);
      }
    }
    else  /* ooc message sent, turn ooc on if it is off */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;

	REMOVE_BIT(ch->comm,COMM_NOOOC);
      }

      if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
      && (!IS_IMMORTAL(ch)))
      {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
      }

    if (ch->newbie != 0)
    {
        send_to_char("You do not yet have access to this channel.\n\r",ch);
        return;
    }
     act_new("{W[{D$n {CO{co{CC{W]{m:{x '{c$t{x'",ch,argument,NULL,TO_CHAR,POS_DEAD);
     act_new("{W[{D$n {CO{co{CC{W]{m:{x '{c$t{x'",ch,argument,NULL,TO_VICT,POS_DEAD);
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOOOC) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
		act_new("{W[{D$n {CO{co{CC{W]{m:{x '{c$t{x'", ch,argument,d->character,TO_VICT,POS_DEAD);
	    }
 	}
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}

void do_qgossip( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    DESCRIPTOR_DATA *d;
    int wtime;

    if (!ch->on_gquest && !IS_IMMORTAL(ch))
    {   
        send_to_char("You are not on a quest!\n\r",ch);
        return;
    }
    if (ch->newbie != 0)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }
    if (argument[0] == '\0' )
    {   
      if (IS_SET(ch->comm,COMM_NOQGOSSIP))
      { 
        send_to_char("Quest gossip channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOQGOSSIP);
      } 
      else
      {   
        send_to_char("Quest gossip channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOQGOSSIP);
      } 
    }  
    else  /* qgossip message sent, turn qgossip on if it isn't already */
    {   
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }

        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
        }
 
        if ((!str_infix(" fuck ", argument))
        ||  (!str_prefix("fuck ", argument))
        ||  (!str_suffix(" fuck", argument))
        ||  (!str_suffix(" fuck.", argument))
        ||  (!str_suffix(" fuck!", argument))
        ||  (!str_cmp("fuck", argument))
        ||  (!str_cmp("fuck.", argument))
        ||  (!str_cmp("fuck!", argument))
        ||  (!str_cmp("shit", argument))
        ||  (!str_cmp("shit.", argument))
        ||  (!str_cmp("shit!", argument))
        ||  (!str_infix(" shit ", argument))
        ||  (!str_prefix("shit ", argument))
        ||  (!str_suffix(" shit", argument))
        ||  (!str_suffix(" shit.", argument))
        ||  (!str_suffix(" shit!", argument)))
        {
            if (!IS_IMMORTAL(ch)) {
                send_to_char("Don't let the IMM's catch you talking like that!\n\r",ch);
                return;
            }
        }

      if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
      && (!IS_IMMORTAL(ch)))
      {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
      }

      REMOVE_BIT(ch->comm,COMM_NOQGOSSIP);
 
      sprintf( buf, "You qgossip '{l%s{x'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
        int pos;
        bool found = FALSE;

        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOQGOSSIP) &&
             !IS_SET(victim->comm,COMM_QUIET) &&
             ((victim->on_gquest) || IS_IMMORTAL(victim)))
        {
            for (pos = 0; pos < MAX_FORGET; pos++)
            {
                if (victim->pcdata->forget[pos] == NULL)
                    break;
                if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
                    found = TRUE;
            }
            if (!found)
            {
                act_new( "$n qgossips '{l$t{x'",
                    ch,argument, d->character, TO_VICT,POS_SLEEPING );
            }
        }
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}


void do_grats( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char bufy[MSL];
    char bufz[MSL];
    DESCRIPTOR_DATA *d;
    int wtime;
 
    buf[0] = '\0';
    bufy[0] = '\0';
    bufz[0] = '\0';

    if (ch->newbie != 0)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }

    if (argument[0] == '\0' )
    {
	if (IS_SET(ch->comm,COMM_NOGRATS))
 	{
	    send_to_char("Grats channel is now ON.\n\r",ch);
	    REMOVE_BIT(ch->comm,COMM_NOGRATS);
	}
	else
	{
	    send_to_char("Grats channel is now OFF.\n\r",ch);
	    SET_BIT(ch->comm,COMM_NOGRATS);
	}
    }
    else  /* grats message sent, turn grats on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
        {
	    send_to_char("You must turn off quiet mode first.\n\r",ch);
	    return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
	    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	    return;
        }
 
        if ((!str_infix(" fuck ", argument))
        ||  (!str_prefix("fuck ", argument))
        ||  (!str_suffix(" fuck", argument))
        ||  (!str_suffix(" fuck.", argument))
        ||  (!str_suffix(" fuck!", argument))
        ||  (!str_cmp("fuck", argument))
        ||  (!str_cmp("fuck.", argument))
        ||  (!str_cmp("fuck!", argument))
        ||  (!str_cmp("shit", argument))
        ||  (!str_cmp("shit.", argument))
        ||  (!str_cmp("shit!", argument))
        ||  (!str_infix(" shit ", argument))
        ||  (!str_prefix("shit ", argument))
        ||  (!str_suffix(" shit", argument))
        ||  (!str_suffix(" shit.", argument))
        ||  (!str_suffix(" shit!", argument)))
        { 
            if (!IS_IMMORTAL(ch)) {
                send_to_char("Don't let the IMM's catch you talking like that!\n\r",ch);
                return; 
            } 
        } 

	if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
	&& (!IS_IMMORTAL(ch)))
	{
	    send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	    return;
	}

	REMOVE_BIT(ch->comm,COMM_NOGRATS);

	sprintf( buf, "You grats '{J%s{x'\n\r", argument );
	send_to_char( buf, ch );
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    CHAR_DATA *victim;
	    int pos;
	    bool found = FALSE;

	    victim = d->original ? d->original : d->character;
 
	    if ( d->connected == CON_PLAYING &&
		d->character != ch &&
		!IS_SET(victim->comm,COMM_NOGRATS) &&
		!IS_SET(victim->comm,COMM_QUIET) )
	    {
		for (pos = 0; pos < MAX_FORGET; pos++)
		{
		    if (victim->pcdata->forget[pos] == NULL)
			break;
		    if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
			found = TRUE;
		}
		if (!found)
		{
		    act_new( "$n grats '{J$t{x'",
			ch,argument, d->character, TO_VICT,POS_SLEEPING );
		}
	    }
	}
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}

void do_quote( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    DESCRIPTOR_DATA *d;
    int wtime;
 
    if (ch->newbie != 0)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUOTE))
      {
        send_to_char("Quote channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOQUOTE);
      }
      else
      {
        send_to_char("Quote channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOQUOTE);
      }
    }
    else  /* quote message sent, turn quote on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
        }
 
        if ((!str_infix(" fuck ", argument))
        ||  (!str_prefix("fuck ", argument))
        ||  (!str_suffix(" fuck", argument))
        ||  (!str_suffix(" fuck.", argument))
        ||  (!str_suffix(" fuck!", argument))
        ||  (!str_cmp("fuck", argument))
        ||  (!str_cmp("fuck.", argument))
        ||  (!str_cmp("fuck!", argument))
        ||  (!str_cmp("shit", argument))
        ||  (!str_cmp("shit.", argument))
        ||  (!str_cmp("shit!", argument))
        ||  (!str_infix(" shit ", argument))
        ||  (!str_prefix("shit ", argument))
        ||  (!str_suffix(" shit", argument))
        ||  (!str_suffix(" shit.", argument))
        ||  (!str_suffix(" shit!", argument)))
        { 
            if (!IS_IMMORTAL(ch)) {
                send_to_char("Don't let the IMM's catch you talking like that!\n\r",ch);
                return; 
            } 
        } 

      if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
      && (!IS_IMMORTAL(ch)))
      {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
      }

      REMOVE_BIT(ch->comm,COMM_NOQUOTE);
 
      sprintf( buf, "You quote '{Q%s{x'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOQUOTE) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
		act_new( "$n quotes '{Q$t{x'",
		    ch,argument, d->character, TO_VICT,POS_SLEEPING );
	    }
        }
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}

/* RT ask channel */
void do_ask( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    DESCRIPTOR_DATA *d;
    int wtime;
 
    if (ch->newbie != 0)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOASK))
      {
        send_to_char("Q/A channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOASK);
      }
      else
      {
        send_to_char("Q/A channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOASK);
      }
    }
    else  /* ask sent, turn Q/A on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}
 
        if ((!str_infix(" fuck ", argument))
        ||  (!str_prefix("fuck ", argument))
        ||  (!str_suffix(" fuck", argument))
        ||  (!str_suffix(" fuck.", argument))
        ||  (!str_suffix(" fuck!", argument))
        ||  (!str_cmp("fuck", argument))
        ||  (!str_cmp("fuck.", argument))
        ||  (!str_cmp("fuck!", argument))
        ||  (!str_cmp("shit", argument))
        ||  (!str_cmp("shit.", argument))
        ||  (!str_cmp("shit!", argument))
        ||  (!str_infix(" shit ", argument))
        ||  (!str_prefix("shit ", argument))
        ||  (!str_suffix(" shit", argument))
        ||  (!str_suffix(" shit.", argument))
        ||  (!str_suffix(" shit!", argument)))
        { 
            if (!IS_IMMORTAL(ch)) {
                send_to_char("Don't let the IMM's catch you talking like that!\n\r",ch);
                return; 
            } 
        } 

	if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
	&& (!IS_IMMORTAL(ch)))
	{
	    send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	    return;
	}

        REMOVE_BIT(ch->comm,COMM_NOASK);
 
      sprintf( buf, "You ask '{P%s{x'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOASK) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
		act_new("$n asks '{P$t{x'",
		    ch,argument,d->character,TO_VICT,POS_SLEEPING);
	    }
        }
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}

/* RT answer channel - uses same line as asks */
void do_answer( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    DESCRIPTOR_DATA *d;
    int wtime;
 
    if (ch->newbie != 0)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOASK))
      {
        send_to_char("Q/A channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOASK);
      }
      else
      {
        send_to_char("Q/A channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOASK);
      }
    }
    else  /* answer sent, turn Q/A on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}
 
        if ((!str_infix(" fuck ", argument))
        ||  (!str_prefix("fuck ", argument))
        ||  (!str_suffix(" fuck", argument))
        ||  (!str_suffix(" fuck.", argument))
        ||  (!str_suffix(" fuck!", argument))
        ||  (!str_cmp("fuck", argument))
        ||  (!str_cmp("fuck.", argument))
        ||  (!str_cmp("fuck!", argument))
        ||  (!str_cmp("shit", argument))
        ||  (!str_cmp("shit.", argument))
        ||  (!str_cmp("shit!", argument))
        ||  (!str_infix(" shit ", argument))
        ||  (!str_prefix("shit ", argument))
        ||  (!str_suffix(" shit", argument))
        ||  (!str_suffix(" shit.", argument))
        ||  (!str_suffix(" shit!", argument)))
        { 
            if (!IS_IMMORTAL(ch)) {
                send_to_char("Don't let the IMM's catch you talking like that!\n\r",ch);
                return; 
            } 
        } 

	if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
	&& (!IS_IMMORTAL(ch)))
	{
	    send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	    return;
	}

        REMOVE_BIT(ch->comm,COMM_NOASK);
 
      sprintf( buf, "You answer '{P%s{x'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOASK) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
		act_new("$n answers '{P$t{x'",
		    ch,argument,d->character,TO_VICT,POS_SLEEPING);
	    }
        }
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}

/* RT music channel */
void do_music( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    DESCRIPTOR_DATA *d;
    int wtime;
 
    if (ch->newbie != 0)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOMUSIC))
      {
        send_to_char("Music channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOMUSIC);
      }
      else
      {
        send_to_char("Music channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOMUSIC);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}
 
        if ((!str_infix(" fuck ", argument))
        ||  (!str_prefix("fuck ", argument))
        ||  (!str_suffix(" fuck", argument))
        ||  (!str_suffix(" fuck.", argument))
        ||  (!str_suffix(" fuck!", argument))
        ||  (!str_cmp("fuck", argument))
        ||  (!str_cmp("fuck.", argument))
        ||  (!str_cmp("fuck!", argument))
        ||  (!str_cmp("shit", argument))
        ||  (!str_cmp("shit.", argument))
        ||  (!str_cmp("shit!", argument))
        ||  (!str_infix(" shit ", argument))
        ||  (!str_prefix("shit ", argument))
        ||  (!str_suffix(" shit", argument))
        ||  (!str_suffix(" shit.", argument))
        ||  (!str_suffix(" shit!", argument)))
        { 
            if (!IS_IMMORTAL(ch)) {
                send_to_char("Don't let the IMM's catch you talking like that!\n\r",ch);
                return; 
            } 
        } 

	if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
	&& (!IS_IMMORTAL(ch)))
	{
	    send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	    return;
	}

        REMOVE_BIT(ch->comm,COMM_NOMUSIC);
 
      sprintf( buf, "You MUSIC: '{N%s{x'\n\r", argument );
      send_to_char( buf, ch );
      sprintf( buf, "$n MUSIC: '{N%s{x'", argument );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOMUSIC) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
		act_new("$n MUSIC: '{N$t{x'", ch,argument,d->character,TO_VICT,POS_SLEEPING);
	    }
        }
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
	send_to_char("Immortal channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
	send_to_char("Immortal channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      } 
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOWIZ);

    sprintf( buf, "{W[{D$n {CI{cmm{CT{calks{W]{m:{x {D%s{x", argument );
    act_new("{W[{D$n {CI{cmm{CT{calks{W]{m:{x {D$t{x",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING && 
	     IS_IMMORTAL(d->character) && 
             !IS_SET(d->character->comm,COMM_NOWIZ) )
	{
	    act_new("{W[{D$n {CI{cmm{CT{calks{W]{m:{x {D$t{x",ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }

    return;
}



void do_say( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if (ch->shadow)
    {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }

    act( "{W$n {DSays{x: '{m$T{x'", ch, NULL, argument, TO_ROOM );
    act( "{WYou {DSay{x: '{m$T{x'", ch, NULL, argument, TO_CHAR );

     if ( !IS_NPC(ch) )
     {
       CHAR_DATA *mob, *mob_next;
       for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
       {
           mob_next = mob->next_in_room;
           if ( IS_NPC(mob) && HAS_TRIGGER( mob, TRIG_SPEECH )
           &&   mob->position == mob->pIndexData->default_pos )
               mp_act_trigger( argument, mob, ch, NULL, NULL, TRIG_SPEECH );
       }
     }
    return;
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if (ch->newbie != 0)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }
    if (argument[0] == '\0' )
    {
      	if (IS_SET(ch->comm,COMM_SHOUTSOFF))
      	{
            send_to_char("You can hear shouts again.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	else
      	{
            send_to_char("You will no longer hear shouts.\n\r",ch);
            SET_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	return;
    }

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't shout.\n\r", ch );
        return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
    && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }
 
    REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);

    WAIT_STATE( ch, 12 );

    act( "You shout '{T$T{x'", ch, NULL, argument, TO_CHAR );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_SHOUTSOFF) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
		act("$n shouts '{T$t{x'",ch,argument,d->character,TO_VICT);
	    }
	}
    }

    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    CHAR_DATA *victim;
    int pos;
    bool found = FALSE;
    int wtime;

    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char("You must turn off deaf mode first.\n\r",ch);
	return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
    && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
        sprintf(buf,"%s tells you '{c%s{x'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	victim->tells++;
	return;
    }
    if ((ch->newbie != 0) && (!IS_IMMORTAL(victim))
    && (victim->newbie == 0))
    {
	send_to_char("You can only tell to immortals or other newbies.\n\r",ch);
	return;
    }

    if (!IS_NPC(victim))
    {
	for (pos = 0; pos < MAX_FORGET; pos++)
	{
	    if (victim->pcdata->forget[pos] == NULL)
		break;
	    if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		found = TRUE;
	}
    }
    if (found)
    {
	act("$N doesn't seem to be listening to you.",ch,NULL,victim,TO_CHAR);
	return;
    }

/* Removed..Vengeance */
/*
    if ( !(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
*/
  
    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
	if (IS_NPC(victim))
	{
	    act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
	    return;
	}

	act("$E is AFK, but your tell will go through when $E returns.",
	    ch,NULL,victim,TO_CHAR);
	sprintf(buf,"%s tells you '{c%s{x'\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	victim->tells++;
	return;
    }

    if (IS_SET(victim->comm,COMM_STORE)
	&& (victim->fighting != NULL) )
    {
	if (IS_NPC(victim))
	{
	    act("$E is fighting, and not receiving tells.",ch,NULL,victim,TO_CHAR);
	    return;
	}

	act("$E is fighting, but your tell will go through when $E finishes.",
	    ch,NULL,victim,TO_CHAR);
	sprintf(buf,"%s tells you '{c%s{x'\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	victim->tells++;
	return;
    }

    if (argument[0] == '=')
	argument++;
    act( "You tell $N '{c$t{x'", ch, argument, victim, TO_CHAR );
    act_new("$n tells you '{c$t{x'",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MSL];
    int pos;
    bool found = FALSE;
    int wtime;

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
    && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
        sprintf(buf,"%s tells you '{c%s{x'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	victim->tells++;
        return;
    }

    if (!IS_NPC(victim))
    {
	for (pos = 0; pos < MAX_FORGET; pos++)
	{
	    if (victim->pcdata->forget[pos] == NULL)
		break;
	    if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		found = TRUE;
	}
    }
    if (found)
    {
	act("$N doesn't seem to be listening to you.",ch,NULL,victim,TO_CHAR);
	return;
    }

/* Removed...Vengeance */
/*
    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
*/

    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
    {
        act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD);
        return;
    }

/* Removed....Vengeance */
/*
    if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
    {
	send_to_char( "In your dreams, or what?\n\r", ch );
	return;
    }
*/

    if (IS_SET(victim->comm,COMM_AFK))
    {
        if (IS_NPC(victim))
        {
            act_new("$E is AFK, and not receiving tells.",
		ch,NULL,victim,TO_CHAR,POS_DEAD);
            return;
        }
 
        act_new("$E is AFK, but your tell will go through when $E returns.",
            ch,NULL,victim,TO_CHAR,POS_DEAD);
        sprintf(buf,"%s tells you '{c%s{x'\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	victim->tells++;
        return;
    }

    act_new("You tell $N '{c$t{x'",ch,argument,victim,TO_CHAR,POS_DEAD);
    act_new("$n tells you '{c$t{x'",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );

    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't yell.\n\r", ch );
        return;
    }
    if (ch->newbie != 0)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }
    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
    && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    act("You yell '{T$t{x'",ch,argument,NULL,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	int pos;
	bool found = FALSE;

	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
        &&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
		if (d->character->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,d->character->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
		act("$n yells '{T$t{x'",ch,argument,d->character,TO_VICT);
	    }
	}
    }

    return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }
 
    if (ch->shadow)
    {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    MOBtrigger = FALSE;
    act( "{B$n $T{x", ch, NULL, argument, TO_ROOM );
    act( "{B$n $T{x", ch, NULL, argument, TO_CHAR );
    MOBtrigger = TRUE;
    return;
}


void do_pmote( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MIL], temp[MSL];
    unsigned int matches = 0;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }
 
    if (ch->shadow)
    {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    act( "{c$n $t{x", ch, argument, NULL, TO_CHAR );

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument,vch->name)) == NULL)
	{
	    MOBtrigger = FALSE;	
	    act("{c$N $t{x",vch,argument,ch,TO_CHAR);
	    MOBtrigger = TRUE;
	    continue;
	}

	strcpy(temp,argument);
	temp[strlen(argument) - strlen(letter)] = '\0';
   	last[0] = '\0';
 	name = vch->name;
	
	for (; *letter != '\0'; letter++)
	{ 
	    if (*letter == '\'' && matches == strlen(vch->name))
	    {
		strcat(temp,"r");
		continue;
	    }

	    if (*letter == 's' && matches == strlen(vch->name))
	    {
		matches = 0;
		continue;
	    }
	    
 	    if (matches == strlen(vch->name))
	    {
		matches = 0;
	    }

	    if (*letter == *name)
	    {
		matches++;
		name++;
		if (matches == strlen(vch->name))
		{
		    strcat(temp,"you");
		    last[0] = '\0';
		    name = vch->name;
		    continue;
		}
		strncat(last,letter,1);
		continue;
	    }

	    matches = 0;
	    strcat(temp,last);
	    strncat(temp,letter,1);
	    last[0] = '\0';
	    name = vch->name;
	}

	MOBtrigger = FALSE;
	act("{c$N $t{x",vch,temp,ch,TO_CHAR);
	MOBtrigger = TRUE;
    }
	
    return;
}


/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char *	message[2*MAX_CLASS];
};

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
	    "You sizzle with {Be{bn{Be{br{Bg{by{x.",
	    "$n sizzles with {Be{bn{Be{br{Bg{by{x.",
	    "You feel very {Wholy{x.",
	    "$n looks very {Wholy{x.",
	    "You perform a small card trick.",
	    "$n performs a small card trick.",
	    "You show your bulging muscles.",
	    "$n shows $s pitiful muscles.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "... 98, 99, 100 ... you do pushups.",
	    "..1....mmph!.. 2 .. $n does pushups.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon."
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers.",
	    "A {Whalo{x appears over your head.",
	    "A {Whalo{x appears over $n's head.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!"
	}
    },

    {
	{
	    "{BBlue sparks{x fly from your fingers.",
	    "{BBlue sparks{x fly from $n's fingers.",
	    "A {Whalo{x appears over your head.",
	    "A {Whalo{x appears over $n's head.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean.",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "Little {rred{x lights dance in your {z{Re{xy{z{Re{xs.",
	    "Little {rred{x lights dance in $n's {z{Re{xy{z{Re{xs."
	}
    },

    {
	{
	    "Little {rred{x lights dance in your {z{Re{xy{z{Re{xs.",
	    "Little {rred{x lights dance in $n's {z{Re{xy{z{Re{xs.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it."
	}
    },

    {
	{
	    "A slimy {ggreen monster{x appears before you and bows.",
	    "A slimy {ggreen monster{x appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle.",
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed."
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An {Wangel{x consults you.",
	    "An {Wangel{x consults $n.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n craps out.",
	    "... 98, 99, 100 ... you do pushups.",
	    "..1....mmph!.. 2 .. $n does pushups.",
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body {Yglows with an unearthly light{x.",
	    "$n's body {Yglows with an unearthly light{x.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "PeeWee Herman laughs at $n's physique.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder."
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders.",
	    "A {Ccool breeze{x refreshes you.",
	    "A {Ccool breeze{x refreshes $n.",
	    "You step behind your {Dshadow{x.",
	    "$n steps behind $s {Dshadow{x.",
	    "Arnold Schwarzenegger admires your physique.",
	    "PeeWee Herman laughs at $n's physique."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder.",
	    "The sky changes {Rc{Co{Gl{Bo{Yr{x to match your eyes.",
	    "The sky changes {Rc{Co{Gl{Bo{Yr{x to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "Your head disappears.",
	    "$n's head disappears. {D(I was tired of looking at it anyway){x"
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears. {D(I was tired of looking at it anyway){x",
	    "A {Ccool breeze{x refreshes you.",
	    "A {Ccool breeze{x refreshes $n.",
	    "You step behind your {Dshadow{x.",
	    "$n steps behind $s {Dshadow{x.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear.",
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug.",
	    "Your head disappears.",
	    "$n's head disappears. {D(I was tired of looking at it anyway){x",
	    "A {Ccool breeze{x refreshes you.",
	    "A {Ccool breeze{x refreshes $n.",
	    "You step behind your {Dshadow{x.",
	    "$n steps behind $s {Dshadow{x."
	}
    },

    {
	{
	    "The sky changes {Rc{Co{Gl{Bo{Yr{x to match your eyes.",
	    "The sky changes {Rc{Co{Gl{Bo{Yr{x to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop breaks $s hand.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s rolls of fat."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s rolls of fat.",
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid w{zi{xnks at you.",
	    "An eye in a pyramid w{zi{xnks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye."
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid w{zi{xnks at you.",
	    "An eye in a pyramid w{zi{xnks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?"
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great god $g gives you a staff.",
	    "The great god $g gives $n a staff.",
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great god $g gives you a staff.",
	    "The great god $g gives $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him.",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown."
	}
    }
};


void do_enlist( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0] == '\0' ) {
	send_to_char( "Syntax: enlist <character name>.\n\r", ch );
	return;
    }
  
    if ( ( victim = get_char_world( ch, argument ) ) == NULL ) {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch != victim && ch->level < 102 ) {
	send_to_char( "Only an immortal can MAKE someone else pkable.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) ) {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->exbit1_flags, PK_VETERAN) ) {
	send_to_char( "They are a Veteran.\n\r", ch );
	return;
    } 

    if ( victim->level <= 12 ) {
	if ( victim->pcdata->pdeath >= 5 || victim->pcdata->pkills >= 5 ) {
	send_to_char( "You are now a Veteran.\n\r", victim );
	SET_BIT( victim->exbit1_flags, PK_VETERAN );
	return;
    } else {	
	send_to_char( "You are now considered a {R{zPKER{x!\n\r", victim );
	SET_BIT( victim->exbit1_flags, RECRUIT );
	return;
	}
    }

    if ( IS_SET(victim->exbit1_flags, RECRUIT) || IS_SET(victim->exbit1_flags, PK_VETERAN) ) {
	send_to_char( "Someone beat you to it.\n\r", ch );
	return;
    }
	else if ( victim->pcdata->pdeath >= 5 || victim->pcdata->pkills >= 5 ) {
	send_to_char( "You are a Veteran.\n\r", victim );
	SET_BIT( victim->exbit1_flags, PK_VETERAN );
	return;
    } else {
	SET_BIT( victim->exbit1_flags, RECRUIT );
	send_to_char( "You are now considered a {R{zPKER{x!\n\r", victim );
	return;
    }
}


void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    if (ch->shadow)
    {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    if (ch->class < MCLT_1)
    {
	act( pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR );
	act( pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM );
    } else {
	act( pose_table[pose].message[2*(ch->class-(MCLT_1))+0], ch, NULL, NULL, TO_CHAR );
	act( pose_table[pose].message[2*(ch->class-(MCLT_1))+1], ch, NULL, NULL, TO_ROOM );
    }
    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}


void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d,*d_next;
    int id;

    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller)) )
    {
        send_to_char ("Wait till you have sold/bought the item on auction.\n\r",ch);
        return;
    }

    if (ch->shadow)
    {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }

    if ( IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGER) || IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGED) )
    {

	REMOVE_BIT(ch->exbit1_flags, EXBIT1_CHALLENGER);
	REMOVE_BIT(ch->comm,COMM_NOCHANNELS);
	REMOVE_BIT(ch->act,PLR_NORESTORE);
	char_from_room( ch );
	char_to_room( ch, get_room_index(ROOM_VNUM_LIMBO) );
	REMOVE_BIT(ch->exbit1_flags, EXBIT1_CHALLENGED);
	REMOVE_BIT(ch->comm,COMM_NOCHANNELS);
	REMOVE_BIT(ch->act,PLR_NORESTORE);
	char_from_room( ch );
	char_to_room( ch, get_room_index(ROOM_VNUM_LIMBO) );
	arena = FIGHT_OPEN;
    }
/*
    send_to_char( "{rReality is attacking you!{x\n\r",ch);
    send_to_char( "{cYou hit Reality hard!{x\n\r",ch);
    send_to_char( "{BReality's truth does UNSPEAKABLE things to you!{x\n\r\n\r",ch);
    WAIT_STATE(ch,25 * PULSE_VIOLENCE);
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
    wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));
 */
        switch (number_range(1,3)) {
                        case 1:
send_to_char( "{r			You think you must be dreaming.{x\n\r",ch);
send_to_char( "{c		     You finally manage to wake yourself.{x\n\r",ch);
send_to_char( "{B	  What a strange dream, you close your eyes and go back to sleep.{x\n\r",ch); break;
                        case 2:
send_to_char( "\n\r                         Speak not: whisper not:\n\r",ch);
send_to_char( "                      I know all that ye would tell,\n\r",ch);
send_to_char( "                    But to speak might break the spell\n\r",ch);
send_to_char( "                      Which must bend the invincible,\n\r",ch);
send_to_char( "                          The stern of thought;\n\r",ch);
send_to_char( "                   Yet defies the deepest power of Hell.\n\r\n\r",ch ); break;
                        case 3: send_to_char( "{hReality arrives from somewhere.{x\n\r",ch);
   switch( number_range(1,3) ) {
case 1:
    send_to_char( "{hYou really hit Reality hard!{x\n\r",ch);
    send_to_char( "{hReality turns your attack against you!{x\n\r",ch); break;
case 2:
    send_to_char( "{hYour attack misses Reality!!{x\n\r",ch);
        send_to_char( "{hReality Rips into your fantasy!!{x\n\r",ch); break;
case 3:
    send_to_char( "{hReality's backstab {z{W@@@@@@@{x {z{Y*{xM{z{Y*{xe{z{Y*{xG{z{Y*{xa{z{Y*{xN{z{Y*{xu{z{Y*{xK{z{Y*{xe{z{Y*{x{rs{z{R*{x{W @@@@@@@ You!{x\n\r",ch);break;
        }
   switch( number_range(1,5) ) {
case 1: send_to_char( "{iReality's truth *** DEVASTATES *** you!{x\n\r\n\r",ch); break;
case 2: send_to_char( "{iReality's truth === OBLITERATES === you!{x\n\r\n\r",ch); break;
case 3: send_to_char( "{iReality's truth >>> ANNIHILATES <<< you!{x\n\r\n\r",ch); break;
case 4: send_to_char( "{iReality's truth <<< ERADICATES >>> you!{x\n\r\n\r",ch); break;
case 5: send_to_char( "{iReality's truth does UNSPEAKABLE things to you!{x\n\r\n\r",ch); break;
        }
        send_to_char( "{cYou have been {RKILLED!! (just kidding){x\n\r\n\r", ch );
        }
    WAIT_STATE(ch,15 * PULSE_VIOLENCE);
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
     wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));

    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_obj( ch );
    id = ch->id;
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if (tch && tch->id == id)
	{
	    extract_char(tch,TRUE);
	    close_socket(d);
	} 
    }

    return;
}

void force_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d,*d_next;
    int id;
    int lose;

    if ( IS_NPC(ch) )
	return;

    if ( ch->level >= LEVEL_IMMORTAL )
	return;


    if ( ch->position == POS_FIGHTING )
    {
	lose = (ch->desc != NULL) ? 25 : 50;
	if (!IS_SET(ch->act, PLR_LQUEST))
	    gain_exp( ch, 0 - lose );
	stop_fighting( ch, TRUE );
    }

    if ( ch->position  < POS_STUNNED  )
    {
	lose = (ch->desc != NULL) ? 50 : 100;
	if (!IS_SET(ch->act, PLR_LQUEST))
	    gain_exp( ch, 0 - lose );
    }
    if (ch->shadow)
    {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    send_to_char( "{RReality is attacking you!{x\n\r",ch);
    send_to_char( "{hYou hit Reality hard!{x\n\r",ch);
    send_to_char( "Reality's truth does UNSPEAKABLE things to you!{x\n\r\n\r",ch);
    WAIT_STATE(ch,25 * PULSE_VIOLENCE);
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
     wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));

    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_obj( ch );
    id = ch->id;
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if (tch && tch->id == id)
	{
	    extract_char(tch,TRUE);
	    close_socket(d);
	} 
    }

    return;
}


void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    save_char_obj( ch );
    printf_to_char(ch, "Saving %s.\n\r", ch->name);
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if (IS_SET(victim->in_room->room_flags,ROOM_CLAN_ENT))
    {
	send_to_char( "Not in this room!\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR);
        return;
    }

    if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
    && ( !IS_IMMORTAL( ch ) )
    && ( !IS_IMMORTAL( victim ) )
    && ( ch != victim )
    && ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
    {
	act("You can't seem to follow $N.\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (ch->shadow)
    {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}

void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }
    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
    {
	REMOVE_BIT(ch->act, ACT_PET);
	ch->timer = 0;
	ch->hastimer = TRUE;
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL],arg2[MIL];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete") || !str_cmp(arg2,"mob"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if (!str_cmp(arg2,"reroll"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
	||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	send_to_char( "Ok.\n\r", ch );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}

void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"[%2d %s%s%s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5ld xp\n\r",
		    gch->level,
		    IS_NPC(gch) ? "Mob": 
		    (gch->pcdata->tier != 3) ? class_table[gch->class].who_name :
			class_table[gch->class].swho_name,
		    IS_NPC(gch) ? "": 
		    (gch->pcdata->tier != 3) ? "" :
			"/",
		    IS_NPC(gch) ? "": 
		    (gch->pcdata->tier != 3) ? "" :
			class_table[gch->clasb].swho_name,
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act_new( "$N isn't following you.", ch, NULL, victim, TO_CHAR, POS_SLEEPING );
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
        send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
        return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act("You like your master too much to leave $m!",ch,NULL,victim,TO_VICT);
    	return;
    }

    if (victim->level - ch->level > 14)
    {
	send_to_char("They are to high of a level for your group.\n\r",ch);
	return;
    }

    if (victim->level - ch->level < -14)
    {
	send_to_char("They are to low of a level for your group.\n\r",ch);
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	return;
    }

    if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
    && ( !IS_IMMORTAL( ch ) )
    && ( !IS_IMMORTAL( victim ) )
    && ( ch != victim )
    && ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
    {
	send_to_char("They are unable to join your group.\n\r",ch);
	return;
    }

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    CHAR_DATA *gch;
    int members;
    int amount_platinum = 0, amount_gold = 0, amount_silver = 0;
    int share_platinum, share_gold, share_silver;
    int extra_platinum, extra_gold, extra_silver;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
               one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount_silver = atoi( arg1 );

    if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

    if (arg3[0] != '\0')
	amount_platinum = atoi(arg3);

    if ( amount_platinum < 0 || amount_gold < 0 || amount_silver < 0)
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount_platinum == 0 && amount_gold == 0 && amount_silver == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( (ch->silver + (ch->gold * 100) + (ch->platinum * 10000) )
       < (amount_silver + (amount_gold * 100) + (amount_platinum * 10000) ) )
    {
	send_to_char( "You don't have that much to split.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share_platinum   = amount_platinum / members;
    extra_platinum   = amount_platinum % members;

    amount_gold += (extra_platinum * 100);
    share_gold   = amount_gold / members;
    extra_gold   = amount_gold % members;

    amount_silver += (extra_gold * 100);
    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    if ( share_platinum == 0 && share_gold == 0 && share_silver == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    deduct_cost(ch,amount_platinum-extra_platinum,VALUE_PLATINUM);
    add_cost(ch,share_platinum,VALUE_PLATINUM);
    deduct_cost(ch,amount_gold-extra_gold,VALUE_GOLD);
    add_cost(ch,share_gold,VALUE_GOLD);
    deduct_cost(ch,amount_silver,VALUE_SILVER);
    add_cost(ch,share_silver+extra_silver,VALUE_SILVER);

    if (share_platinum > 0)
    {
	sprintf(buf,
	    "You split %d platinum coins. Your share is %d platinum.\n\r",
	     amount_platinum-extra_platinum,share_platinum);
	send_to_char(buf,ch);
    }
    if (share_gold > 0)
    {
	sprintf(buf,
	    "You split %d gold coins. Your share is %d gold.\n\r",
	     amount_gold-extra_gold,share_gold);
	send_to_char(buf,ch);
    }
    if (share_silver > 0)
    {
	sprintf(buf,
	    "You split %d silver coins. Your share is %d silver.\n\r",
 	    amount_silver,share_silver + extra_silver);
	send_to_char(buf,ch);
    }

    if (share_gold == 0 && share_silver == 0)
    {
	sprintf(buf,"$n splits %d platinum coins. Your share is %d platinum.",
		amount_platinum-extra_platinum,share_platinum);
    }
    else if (share_platinum == 0 && share_silver == 0)
    {
	sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
		amount_gold-extra_gold,share_gold);
    }
    else if (share_platinum == 0 && share_gold == 0)
    {
	sprintf(buf,"$n splits %d silver coins. Your share is %d silver.",
		amount_silver,share_silver);
    }
    else if (share_silver == 0)
    {
	sprintf(buf, "$n splits %d platinum and %d gold coins. giving you %d platinum and %d gold.\n\r",
	 amount_platinum-extra_platinum, amount_gold-extra_gold,
	 share_platinum, share_gold);
    }
    else if (share_gold == 0)
    {
	sprintf(buf, "$n splits %d platinum and %d silver coins. giving you %d platinum and %d silver.\n\r",
	 amount_platinum-extra_platinum, amount_silver,
	 share_platinum, share_silver);
    }
    else if (share_platinum == 0)
    {
	sprintf(buf, "$n splits %d gold and %d silver coins. giving you %d gold and %d silver.\n\r",
	 amount_gold-extra_gold, amount_silver,
	 share_gold, share_silver);
    }
    else
    {
	sprintf(buf, "$n splits %d platinum, %d gold and %d silver coins. giving you %d platinum, %d gold and %d silver.\n\r",
	 amount_platinum-extra_platinum, amount_gold-extra_gold, amount_silver,
	 share_platinum, share_gold, share_silver);
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    add_cost(gch,share_platinum,VALUE_PLATINUM);
	    add_cost(gch,share_gold,VALUE_GOLD);
	    add_cost(gch,share_silver,VALUE_SILVER);
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    CHAR_DATA *gch;

    if (ch->newbie != 0)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }
    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
    && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "%s tells the group '{K%s{x'\n\r", ch->name, argument );
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    send_to_char( buf, gch );
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach == NULL || bch == NULL)
	return FALSE;

    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

/*
 * ColoUr setting and unsetting, way cool, Ant Oct 94
 *        revised to include config colour, Ant Feb 95
 */
void do_colour( CHAR_DATA *ch, char *argument )
{
    char 	arg[ MAX_STRING_LENGTH ];

    if( IS_NPC( ch ) )
    {
	send_to_char_bw( "ColoUr is not ON, Way Moron!\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if( !*arg )
    {
	if( !IS_SET( ch->act, PLR_COLOUR ) )
	{
	    SET_BIT( ch->act, PLR_COLOUR );
	    send_to_char( "ColoUr is now ON, Way Cool!\n\r"
		"Further syntax:\n\r   colour {c<{xfield{c> <{xcolour{c>{x\n\r"
		"   colour {c<{xfield{c>{x {cbeep{x|{cnobeep{x\n\r"
		"Type help {ccolour{x and {ccolour2{x for details.\n\r"
		"ColoUr is brought to you by Lope, ant@solace.mh.se.\n\r", ch );
	}
	else
	{
	    send_to_char_bw( "ColoUr is now OFF, <sigh>\n\r", ch );
	    REMOVE_BIT( ch->act, PLR_COLOUR );
	}
	return;
    }

    if( !str_cmp( arg, "default" ) )
    {
	default_colour( ch );
	send_to_char_bw( "ColoUr setting set to default values.\n\r", ch );
	return;
    }

    if( !str_cmp( arg, "all" ) )
    {
	all_colour( ch, argument );
	return;
    }

    /*
     * Yes, I know this is ugly and unnessessary repetition, but its old
     * and I can't justify the time to make it pretty. -Lope
     */
    if( !str_cmp( arg, "text" ) )
    {
	ALTER_COLOUR( text )
    }
    else if( !str_cmp( arg, "auction" ) )
    {
	ALTER_COLOUR( auction )
    }
    else if( !str_cmp( arg, "auction_text" ) )
    {
	ALTER_COLOUR( auction_text )
    }
    else if( !str_cmp( arg, "gossip" ) )
    {
	ALTER_COLOUR( gossip )
    }
    else if( !str_cmp( arg, "gossip_text" ) )
    {
	ALTER_COLOUR( gossip_text )
    }
    else if( !str_cmp( arg, "music" ) )
    {
	ALTER_COLOUR( music )
    }
    else if( !str_cmp( arg, "music_text" ) )
    {
	ALTER_COLOUR( music_text )
    }
    else if( !str_cmp( arg, "question" ) )
    {
	ALTER_COLOUR( question )
    }
    else if( !str_cmp( arg, "question_text" ) )
    {
	ALTER_COLOUR( question_text )
    }
    else if( !str_cmp( arg, "answer" ) )
    {
	ALTER_COLOUR( answer )
    }
    else if( !str_cmp( arg, "answer_text" ) )
    {
	ALTER_COLOUR( answer_text )
    }
    else if( !str_cmp( arg, "quote" ) )
    {
	ALTER_COLOUR( quote )
    }
    else if( !str_cmp( arg, "quote_text" ) )
    {
	ALTER_COLOUR( quote_text )
    }
    else if( !str_cmp( arg, "immtalk_text" ) )
    {
	ALTER_COLOUR( immtalk_text )
    }
    else if( !str_cmp( arg, "immtalk_type" ) )
    {
	ALTER_COLOUR( immtalk_type )
    }
    else if( !str_cmp( arg, "info" ) )
    {
	ALTER_COLOUR( info )
    }
    else if( !str_cmp( arg, "say" ) )
    {
	ALTER_COLOUR( say )
    }
    else if( !str_cmp( arg, "say_text" ) )
    {
	ALTER_COLOUR( say_text )
    }
    else if( !str_cmp( arg, "tell" ) )
    {
	ALTER_COLOUR( tell )
    }
    else if( !str_cmp( arg, "tell_text" ) )
    {
	ALTER_COLOUR( tell_text )
    }
    else if( !str_cmp( arg, "reply" ) )
    {
	ALTER_COLOUR( reply )
    }
    else if( !str_cmp( arg, "reply_text" ) )
    {
	ALTER_COLOUR( reply_text )
    }
    else if( !str_cmp( arg, "gtell_text" ) )
    {
	ALTER_COLOUR( gtell_text )
    }
    else if( !str_cmp( arg, "gtell_type" ) )
    {
	ALTER_COLOUR( gtell_type )
    }
    else if( !str_cmp( arg, "wiznet" ) )
    {
	ALTER_COLOUR( wiznet )
    }
    else if( !str_cmp( arg, "room_title" ) )
    {
	ALTER_COLOUR( room_title )
    }
    else if( !str_cmp( arg, "room_text" ) )
    {
	ALTER_COLOUR( room_text )
    }
    else if( !str_cmp( arg, "room_exits" ) )
    {
	ALTER_COLOUR( room_exits )
    }
    else if( !str_cmp( arg, "room_things" ) )
    {
	ALTER_COLOUR( room_things )
    }
    else if( !str_cmp( arg, "prompt" ) )
    {
	ALTER_COLOUR( prompt )
    }
    else if( !str_cmp( arg, "fight_death" ) )
    {
	ALTER_COLOUR( fight_death )
    }
    else if( !str_cmp( arg, "fight_yhit" ) )
    {
	ALTER_COLOUR( fight_yhit )
    }
    else if( !str_cmp( arg, "fight_ohit" ) )
    {
	ALTER_COLOUR( fight_ohit )
    }
    else if( !str_cmp( arg, "fight_thit" ) )
    {
	ALTER_COLOUR( fight_thit )
    }
    else if( !str_cmp( arg, "fight_skill" ) )
    {
	ALTER_COLOUR( fight_skill )
    }
    else
    {
	send_to_char_bw( "Unrecognised Colour Parameter Not Set.\n\r", ch );
	return;
    }

    send_to_char_bw( "New Colour Parameter Set.\n\r", ch );
    return;
}

/*
void do_arena( CHAR_DATA *ch, char *argument )
{
     if (IS_SET(ch->comm,COMM_NOARENA))
     {
        send_to_char("Arena channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOARENA);
     }
     else
     {
        send_to_char("Arena channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOARENA);
     }
}

void do_flame( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char bufy[MSL];
    char bufz[MSL];
    DESCRIPTOR_DATA *d;
    int wtime;
 
    buf[0] = '\0';
    bufy[0] = '\0';
    bufz[0] = '\0';
    if (argument[0] == '\0' )
    {
	if (IS_SET(ch->comm,COMM_NOFLAME))
 	{
	    send_to_char("Flame channel is now ON.\n\r",ch);
	    REMOVE_BIT(ch->comm,COMM_NOFLAME);
	}
	else
	{
	    send_to_char("Flame channel is now OFF.\n\r",ch);
	    SET_BIT(ch->comm,COMM_NOFLAME);
	
	}
			 
    }
    else  
    {
	if (IS_SET(ch->comm,COMM_QUIET))
        {
	    send_to_char("You must turn off quiet mode first.\n\r",ch);
	    return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
	    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	    return;
        }
 

            if ((ch->level < 40 && (ch->pcdata->tier < 1))) 
            {
		send_to_char("Someone says, 'This channel not available until level 40 \n\r",ch);
	        return;
            }

	if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
	&& (!IS_IMMORTAL(ch)))
	{
	    send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	    return;
	}


	sprintf( buf, "{MYou{x {RF{x*{Ml{x*{RA{x*{Mm{x*{RE{x--{r>>{x{R>>> {z{M%s{x {r>>{x{R>>{z>{x\r", argument );
	send_to_char( buf, ch );
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    CHAR_DATA *victim;
	    int pos;
	    bool found = FALSE;

	    victim = d->original ? d->original : d->character;
 
	    if ( d->connected == CON_PLAYING &&
		d->character != ch &&
		!IS_SET(victim->comm,COMM_NOFLAME) &&
		!IS_SET(victim->comm,COMM_QUIET) )
	    {
		for (pos = 0; pos < MAX_FORGET; pos++)
		{
		    if (victim->pcdata->forget[pos] == NULL)
			break;
		    if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
			found = TRUE;
		}
	if (!found) 
		{
		    act_new( "{M$n{x {RF{x*{Ml{x*{RA{x*{Mm{x*{RE{x{W'{x{ys{x--{r>>{x{R>>{z> {M$t{x {r>>{x{R>>>{x\r",
			ch,argument, d->character, TO_VICT,POS_STANDING );
		}
	    }
	}
    }
    wtime = UMAX(4, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}
*/

void do_gmote( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if (ch->level < 2)
    {
	send_to_char("You do not yet have access to this channel.\n\r",ch);
	return;
    }
    if (argument[0] == '\0' )
    {
      	if (IS_SET(ch->comm,COMM_NOOOC))
      	{
            send_to_char("You can hear gmotes again.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOOOC);
      	}
      	else
      	{
            send_to_char("You will no longer hear gmotes.\n\r",ch);
            SET_BIT(ch->comm,COMM_NOOOC);
      	}
      	return;
    }

    if ( IS_SET(ch->comm, COMM_NOOOC) )
    {
        send_to_char( "You can't gmote.\n\r", ch );
        return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
    && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    if (ch->newbie != 0)
    {
        send_to_char("You do not yet have access to this channel.\n\r",ch);
        return;
    }
 
    REMOVE_BIT(ch->comm,COMM_NOOOC);

//    WAIT_STATE( ch, 8 );

    act( "{W[{CG{cmot{Ce{W]{m:{x $n $T.", ch, NULL, argument,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_NOOOC) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
		act("{W[{CG{cmot{Ce{W]{m:{x $n $t.",ch,argument,d->character,TO_VICT);
	    }
	}
    }

    return;
}
/*

void do_socials (CHAR_DATA * ch, char *argument) {
    char *buf;
    int count = 0, match = 0, col = 0, size = 0, social[MAX_SOCIALS];
    for (count = 0; social_table[count].name[0] != '\0'; ++count)
        social[count] = count;
    qsort(social, count, sizeof(int), sort_socials);
                                                                                                                                               
    for (match = 0; match < count; ++match) {
        size = strlen(social_table[social[match]].name) + 1;
        if (size < 0 )
            size = 13; // Needs to be at least this size.. :)
        buf = malloc(count * sizeof(char));
        sprintf(buf, "%-12s", social_table[social[match]].name);
        send_to_char(buf, ch);
        if (++col % 6 == 0)
            send_to_char("\r\n", ch);
        free(buf);
    }
    if (col % 6 != 0)
        send_to_char ("\n\r", ch);
    return;
}

 */
void do_gocial(CHAR_DATA *ch, char *argument)
{
    char command[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    int counter;
    int count;
    char buf2[MAX_STRING_LENGTH];

    argument = one_argument(argument,command);

    if (command[0] == '\0')
    {
        send_to_char("YO YO u must USE A SOCIAL in order to gocial not just type gocial, type socials for a list.\n\r",ch);
        return;
    }

    found = FALSE;
    for (cmd = 0; social_table[cmd].name[0] != '\0'; cmd++)
    {
        if (command[0] == social_table[cmd].name[0]
        && !str_prefix( command,social_table[cmd].name ) )
        {
            found = TRUE;
            break;
        }
    }

    if (!found)
    {
        send_to_char("{*Thats not a valid SOCIAL, type socials for a list.{x\n\r",ch);
        return;
    }

    if (!IS_NPC(ch) && IS_SET(ch->comm,   COMM_QUIET))
    {
        send_to_char("You must turn off quiet mode first.\n\r",ch);
        return;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOOOC))
    {
        send_to_char("But you have the ooc channel turned off!\n\r",ch);
        return;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS))
    {
        send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
        return;
    }

    if (ch->newbie != 0)
    {
        send_to_char("You do not yet have access to this channel.\n\r",ch);
        return;
    }

    switch (ch->position)
    {
    case POS_DEAD:
        send_to_char("Lie still; you are DEAD!\n\r",ch);
        return;
    case POS_INCAP:
    case POS_MORTAL:
        send_to_char("You are {Rhurt {wfar too bad for that.\n\r",ch);
        return;
    case POS_STUNNED:
        send_to_char("You are too stunned for that.\n\r",ch);
        return;
    }

    one_argument(argument,arg);
    victim = NULL;
    if (arg[0] == '\0')
    {
        sprintf(buf, "{W[{CG{cocia{Cl{W]{m:{x %s", social_table[cmd].char_no_arg );
        act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
        sprintf(buf, "{W[{CG{cocia{Cl{W]{m:{x %s", social_table[cmd].others_no_arg );
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *vch;
            vch = d->original ? d->original : d->character;
            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET(vch->comm,COMM_NOOOC) &&
                !IS_SET(vch->comm,COMM_QUIET))
            {
                act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
            }
        }
    }
    else if ((victim = get_char_world(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }
    else if (victim == ch)
    {
        sprintf(buf,"{W[{CG{cocia{Cl{W]{m:{x %s", social_table[cmd].char_auto);
        act_new(buf,ch,NULL,NULL,TO_CHAR,POS_DEAD);
        sprintf(buf,"{W[{CG{cocia{Cl{W]{m:{x %s", social_table[cmd].others_auto);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *vch;
            vch = d->original ? d->original : d->character;
            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                !IS_SET(vch->comm,COMM_NOOOC) &&
                !IS_SET(vch->comm,COMM_QUIET))
            {
                act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
            }
        }
    }
    else
    {
        sprintf(buf,"{W[{CG{cocia{Cl{W]{m:{x %s", social_table[cmd].char_found);
        act_new(buf,ch,NULL,victim,TO_CHAR,POS_DEAD);
        sprintf(buf,"{W[{CG{cocia{Cl{W]{m:{x %s", social_table[cmd].vict_found);
        act_new(buf,ch,NULL,victim,TO_VICT,POS_DEAD);
        
        sprintf(buf,"{W[{CG{cocia{Cl{W]{m:{x %s", social_table[cmd].others_found);
        for (counter = 0; buf[counter+1] != '\0'; counter++)
        {
            if (buf[counter] == '$' && buf[counter + 1] == 'N')
            {
                strcpy(buf2,buf);
                buf2[counter] = '\0';
                strcat(buf2,victim->name);
                for (count = 0; buf[count] != '\0'; count++)
                {
                    buf[count] = buf[count+counter+2];
                }
                strcat(buf2,buf);
                strcpy(buf,buf2);

            }
            else if (buf[counter] == '$' && buf[counter + 1] == 'E')
            {
                switch (victim->sex)
                {
                default:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"it");
                    for (count = 0; buf[count] != '\0'; count ++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 1:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"it");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 2:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"it");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                }
            }
            else if (buf[counter] == '$' && buf[counter + 1] == 'M')
            {
                buf[counter] = '%';
                buf[counter + 1] = 's';
                switch (victim->sex)
                {
                default:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"it");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 1:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"him");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 2:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"her");
                    for (count = 0; buf[count] != '\0'; count++);
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                }
            }
            else if (buf[counter] == '$' && buf[counter + 1] == 'S')
            {
                switch (victim->sex)
                {
                default:
                strcpy(buf2,buf);
                buf2[counter] = '\0';
                strcat(buf2,"its");
                for (count = 0;buf[count] != '\0'; count++)
                {
                    buf[count] = buf[count+counter+2];
                }
                strcat(buf2,buf);
                strcpy(buf,buf2);
                break;
                case 1:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"his");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                case 2:
                    strcpy(buf2,buf);
                    buf2[counter] = '\0';
                    strcat(buf2,"hers");
                    for (count = 0; buf[count] != '\0'; count++)
                    {
                        buf[count] = buf[count+counter+2];
                    }
                    strcat(buf2,buf);
                    strcpy(buf,buf2);
                    break;
                }
            }

        }
        for (d=descriptor_list; d != NULL; d = d->next)
        {
            CHAR_DATA *vch;
            vch = d->original ? d->original : d->character;
            if (d->connected == CON_PLAYING &&
                d->character != ch &&
                d->character != victim &&
                !IS_SET(vch->comm, COMM_NOOOC) &&
                !IS_SET(vch->comm,COMM_QUIET))
            {
                act_new(buf,ch,NULL,vch,TO_VICT,POS_DEAD);
            }
        }
    }
    return;
}

