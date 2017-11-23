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
#include <time.h>
#else
	#include <sys/types.h>
	#if defined(WIN32)
		#include <time.h>
	#else
		#include <sys/time.h>
	#endif
#endif
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"

/*
 * Local functions.
 */
int	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
void	enter_exit	args( ( CHAR_DATA *ch, char *arg ) );



void move_char( CHAR_DATA *ch, int door, bool follow, bool quiet )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    int track;
    bool wasquiet;
	char buf [MSL];
	int revdoor;
    wasquiet = quiet;
    if ( door < 0 || door > 11 )
    {
	bug( "Do_move: bad door %d.", door );
	ch->running = FALSE;
	return;
    }

    /*
     * Exit trigger, if activated, bail out. Only PCs are triggered.
     */
    if ( !IS_NPC(ch) && mp_exit_trigger( ch, door ) )
	return;

    in_room = ch->in_room;
    if ( ( ch->alignment < 0)
    &&   door < 6
    &&   ( pexit   = in_room->exit[door+6] ) != NULL )
    {
	door += 6;
    } else if ( ( pexit = in_room->exit[door] ) == NULL)
    {
	if (!quiet)
	{
	    OBJ_DATA *portal;

	    portal = get_obj_list( ch, dir_name[door],  ch->in_room->contents );
	    if (portal != NULL)
	    {
		ch->running = FALSE;
		enter_exit( ch, dir_name[door] );
		return;
	    }
	}
    }

    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(ch,pexit->u1.to_room)
    ||   !can_see_door(ch,pexit->exit_info))
    {
	if (!quiet)
	    send_to_char( "Alas, you cannot go that way.\n\r", ch );
	ch->running = FALSE;
	return;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
    &&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS))
    &&   !IS_TRUSTED(ch,KNIGHT))
    {
	if (!quiet)
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	ch->running = FALSE;
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	if (!quiet)
	    send_to_char( "What?  And leave your beloved master?\n\r", ch );
	ch->running = FALSE;
	return;
    }

    if ( !is_room_owner(ch,to_room) && room_is_private( ch,to_room ))
    {
	if (!quiet)
	    send_to_char( "That room is private right now.\n\r", ch );
	ch->running = FALSE;
	return;
    }

    if ( !IS_NPC(ch) )
    {
	int move;
	bool ts;
	GUILD_DATA *guild;
	RACE_DATA *race;

	ts = TRUE;

	for (guild = to_room->guild; guild != NULL; guild = guild->next)
	{
	    ts = FALSE;
	    if (guild->guild == ch->class)
		ts = TRUE;
	    if ((ch->pcdata->tier >= 2) && (guild->guild == ch->clasb))
		ts = TRUE;
	    if (ts)
		break;
	}
	if (ts)
	{
	    for (race = to_room->race; race != NULL; race = race->next)
	    {
		ts = FALSE;
		if (race->race == ch->race)
		{
		    ts = TRUE;
		    break;
		}
	    }
	}
	if (IS_IMMORTAL(ch))
	    ts = TRUE;

	if (!ts)
	{
	    if (!quiet)
		send_to_char( "You aren't allowed in there.\n\r", ch );
	    ch->running = FALSE;
	    return;
	}

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch)
	    && !(ch->spirit))
	    {
		if (to_room->fall == NULL)
		{
		    if (!quiet)
			send_to_char( "You can't fly.\n\r", ch );
		    ch->running = FALSE;
		    return;
		} else if ( ( pexit   = to_room->fall ) == NULL
		    ||   ( to_room = pexit->u1.to_room   ) == NULL 
		    ||	 !can_see_room(ch,pexit->u1.to_room))
		{
		    if (!quiet)
			send_to_char( "You can't fly.\n\r", ch );
		    ch->running = FALSE;
		    return;
		}
	    }
	}

	if (!IS_NPC(ch) && ch->stance[0] != -1);

	if (( in_room->sector_type == SECT_WATER_NOSWIM
	||    to_room->sector_type == SECT_WATER_NOSWIM )
  	&&    !IS_AFFECTED(ch,AFF_FLYING) && !(ch->spirit))
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		if (to_room->fall == NULL)
		{
		    if (!quiet)
			send_to_char( "You need a boat to go there.\n\r", ch );
		    ch->running = FALSE;
		    return;
		} else if ( ( pexit   = to_room->fall ) == NULL
		    ||   ( to_room = pexit->u1.to_room   ) == NULL 
		    ||	 !can_see_room(ch,pexit->u1.to_room))
		{
		    if (!quiet)
			send_to_char( "You need a boat to go there.\n\r", ch );
		    ch->running = FALSE;
		    return;
		}
	    }
	}

	if (to_room->transfer != NULL)
	    quiet = TRUE;

	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

        move /= 2;  /* i.e. the average */


	/* conditional effects */
	if (IS_AFFECTED(ch,AFF_FLYING) || IS_AFFECTED(ch,AFF_HASTE))
	    move /= 2;

	if (IS_AFFECTED(ch,AFF_SLOW))
	    move *= 2;

	if (ch->running)
	{
	    move *= 3;
	    move /= 2;
	}

	if ( ch->move < move && !(ch->spirit))
	{
	    if (!quiet)
		send_to_char( "You are too exhausted.\n\r", ch );
	    ch->running = FALSE;
	    return;
	}

	WAIT_STATE( ch, 1 );
	if (!(ch->spirit))
	    ch->move -= move;
    }

    if (IS_SET(in_room->room_flags, ROOM_CLAN_ENT))
    {
	CHAR_DATA *nch;
	CHAR_DATA *bch;
	bool found = FALSE;
	bool foundfree = FALSE;

	ch->running = FALSE;
	bch = in_room->people;
	for ( nch = in_room->people; nch != NULL; nch = nch->next_in_room )
	{
	    if (IS_IMMORTAL(ch))
		break;
	    if (!is_clan(ch))
		break;
	    if (!IS_NPC(nch))
		continue;
	    if (!IS_SET(nch->off_flags, OFF_CLAN_GUARD))
		continue;
	    if (ch->clan == nch->pIndexData->clan)
		continue;
	    else
	    {
		found = TRUE;
		bch = nch;
		if (bch->fighting == NULL)
		{
		    foundfree = TRUE;
		    break;
		}
	    }
	}
	if (found)
	{
	    DESCRIPTOR_DATA *d;
	    CHAR_DATA *gch;
	    CHAR_DATA *leader;
	    MOB_INDEX_DATA *pMobIndex;

	    found = FALSE;
	    leader = (ch->leader != NULL) ? ch->leader : ch;
 
	    for ( d = descriptor_list; d != NULL; d = d->next )
	    {
		if ( d->connected != CON_PLAYING )
		    continue;
		gch = d->character;
		if (IS_NPC(gch))
		    continue;
		if (gch == ch)
		    continue;
		if ( is_same_group( gch, ch ) )
		{
		    found = TRUE;
		    gch->leader = NULL;
		    gch->master = NULL;
		    if (gch->desc != NULL)
			send_to_char("{RYour group has been dissolved!{x\n\r",gch);
		}
	    }
	    ch->leader = NULL;
	    ch->master = NULL;
	    if (found)
		send_to_char("{RYour group has been dissolved!{x\n\r",ch);
	    act("You attempt to move $T.", ch, NULL, dir_name[door], TO_CHAR );
	    act("$n attempts to move $T.", ch, NULL, dir_name[door], TO_ROOM );
	    ch->running = FALSE;
	    if (foundfree)
	    {
		act("$N blocks your way!",ch,NULL,bch,TO_CHAR);
		act("$N blocks $s way!",ch,NULL,bch,TO_ROOM);
		multi_hit( bch, ch, TYPE_UNDEFINED );
		return;
	    } else {
		if ((pMobIndex = get_mob_index(bch->pIndexData->vnum))==NULL)
		{
		    return;
		}
		nch = create_mobile( pMobIndex );
		char_to_room( nch, ch->in_room );
		act("$N suddenly materializes in front of you!",ch,NULL,nch,TO_CHAR);
		act("$N suddenly materializes in front of $m!",ch,NULL,nch,TO_ROOM);
		multi_hit( nch, ch, TYPE_UNDEFINED );
		return;
	    }
	    return;
	}
    }

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level <= LEVEL_HERO
    &&   ch->ghost_level <= LEVEL_HERO)
    {
// 	for ( fch = ch->in_room->people; fch != NULL; fch = fch_next )

    if ( IS_AFFECTED ( ch, AFF_FLYING ) )
	{
		if ( IS_AFFECTED ( ch, AFF_FLYING ) && (IS_SET(ch->parts,PART_WINGS)))
		{
act( "$n flaps $s wings and flies $T.", ch, NULL, dir_name[door], TO_ROOM );
		} else {
act( "$n flies $T.", ch, NULL, dir_name[door], TO_ROOM );
		}
	}
	
	else if ( ( !quiet ) && ( ch->hit < ch->max_hit/2 ) )
	{ act( "$n crawls $T.", ch, NULL, dir_name[door], TO_ROOM ); }
	else if ( ( ch->hit < ch->max_hit/2 ) && ( ch->running ) )
	{ act( "$n limps $T.", ch, NULL, dir_name[door], TO_ROOM ); }
	else if ( ( ch->hit >= ch->max_hit/2 ) && ( ch->running ) )
	{ act( "$n runs $T.", ch, NULL, dir_name[door], TO_ROOM ); }
	else if ( ( !quiet ) && ( ch->hit >= ch->max_hit/2 ) )
	{ act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM ); }
	
	}
	/*
	else 
	{ 
			act( "$n sneaks away $T.", ch, NULL, dir_name[door], TO_ROOM ); 
	}
	 */
    char_from_room( ch );
    char_to_room( ch, to_room );
if      ( door == 0 ) {revdoor = 2;sprintf(buf,"the south");}
else if ( door == 1 ) {revdoor = 3;sprintf(buf,"the west");}
else if ( door == 2 ) {revdoor = 0;sprintf(buf,"the north");}
else if ( door == 3 ) {revdoor = 1;sprintf(buf,"the east");}
else if ( door == 4 ) {revdoor = 5;sprintf(buf,"below");}
else                  {revdoor = 4;sprintf(buf,"above");}
    if (IS_NPC(ch) || !IS_IMMORTAL(ch))
    {
	for (track = MAX_TRACK-1; track > 0; track--)
	{
	    ch->track_to[track] = ch->track_to[track-1];
	    ch->track_from[track] = ch->track_from[track-1];
	}
	if (IS_AFFECTED(ch,AFF_FLYING) || ch->spirit)
	{
	    ch->track_from[0] = 0;
	    ch->track_to[0] = 0;
	} else {
	    ch->track_from[0] = in_room->vnum;
	    ch->track_to[0] = to_room->vnum;
	}
    }
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level <= LEVEL_HERO
    &&   ch->ghost_level <= LEVEL_HERO)
    {

    if ( IS_AFFECTED ( ch, AFF_FLYING ) )
	{
		if ( IS_AFFECTED ( ch, AFF_FLYING ) && (IS_SET(ch->parts,PART_WINGS)))
			{
				act( "$n glides in from $T.", ch, NULL, buf, TO_ROOM );
			}else{ 
				act( "$n flies in from $T.", ch, NULL, buf, TO_ROOM );
			}
	}
	else if ( ( !quiet ) && ( ch->hit >= ch->max_hit/2 ) )
		act( "$n has arrived from $T.", ch, NULL, buf, TO_ROOM );
	else if ( ( !quiet ) && ( ch->hit < ch->max_hit/2 ) )
		act( "$n arrives crawling from $T.", ch, NULL, buf, TO_ROOM );
	else if ( ( ch->hit >= ch->max_hit/2 ) && ( ch->running ) )
		act( "$n speeds in from $T.", ch, NULL, buf, TO_ROOM );
	else if ( ( ch->hit < ch->max_hit/2 ) && ( ch->running ) )
		act( "$n limps in from $T.", ch, NULL, buf, TO_ROOM );
	if (IS_NPC(ch))
	{
	    if( ch->say_descr[0] != '\0')
	    {
//		if (!quiet)
		act( "$n {a$T{x", ch, NULL, ch->say_descr, TO_ROOM );
	    }
	}
    }
	/*
	else 
	{
		act( "$n sneaks in from $T.", ch, NULL, buf, TO_ROOM );
	}
	 */

    if (!quiet)
	do_look( ch, "auto" );

    if (to_room->transfer != NULL)
    {
	move_char_transfer(ch, wasquiet);
	ch->running = FALSE;
	to_room = ch->in_room;
    }

    quiet = wasquiet;

    if (ch->running && !IS_IMMORTAL(ch) && !IS_NPC(ch))
    {
	for ( fch = ch->in_room->people; fch != NULL; fch = fch_next )
	{
	    fch_next = fch->next_in_room;

	    if (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)
	    && can_see( fch, ch ) && !IS_SET(fch->in_room->room_flags,ROOM_SAFE)
	    && !IS_AFFECTED(fch,AFF_CALM) && !IS_AFFECTED(fch, AFF_CHARM)
	    && (fch->level >= ch->level - 5) )
	    {
		ch->running = FALSE;
		break;
	    }
	}
    }

    if (in_room == to_room) /* no circular follows */
    {
	ch->running = FALSE;
	return;
    }

    if (ch->shadowed)
    {
	for ( fch = ch->shadower->in_room->people; fch != NULL; fch = fch->next_in_room )
	{
	    if (IS_NPC(fch))
		continue;
	    if (fch == ch->shadower)
		continue;
	    if (!IS_IMMORTAL(fch))
		continue;
	    if (fch->level < get_trust(ch->shadower))
		continue;
	    act( "$n slips out of the room.", ch->shadower, NULL, fch, TO_VICT );
	}
	char_from_room( ch->shadower );
	char_to_room( ch->shadower, to_room );
	for ( fch = ch->shadower->in_room->people; fch != NULL; fch = fch->next_in_room )
	{
	    if (IS_NPC(fch))
		continue;
	    if (fch == ch->shadower)
		continue;
	    if (!IS_IMMORTAL(fch))
		continue;
	    if (fch->level < get_trust(ch->shadower))
		continue;
	    act( "$n silently slips into the room.", ch->shadower, NULL, fch, TO_VICT );
	}
	if (!quiet)
	{
	    act( "You follow $N.", ch->shadower, NULL, ch, TO_CHAR );
	    do_look( ch->shadower, "auto" );
	}
    }
    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM) 
	&&   fch->position < POS_STANDING)
	    do_stand(fch,"");

	if ( fch->master == ch && fch->position == POS_STANDING 
	&&   can_see_room(fch,to_room))
	{

	    if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
	    &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
	    {
		act("You can't bring $N into the city.",
		    ch,NULL,fch,TO_CHAR);
		act("You aren't allowed in the city.",
		    fch,NULL,NULL,TO_CHAR);
		continue;
	    }

	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door, TRUE, FALSE );
	}
    }

    /* 
     * If someone is following the char, these triggers get activated
     * for the followers before the char, but it's safer this way...
     */
    if ( IS_NPC( ch ) && HAS_TRIGGER( ch, TRIG_ENTRY ) )
	mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_ENTRY );
    if ( !IS_NPC( ch ) )
    	mp_greet_trigger( ch );

    return;
}

void move_char_transfer( CHAR_DATA *ch, bool quiet)
{
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    int track;

    in_room = ch->in_room;

    if ( !IS_NPC(ch) )
    {
	send_to_char( "{x\n\r", ch);
	send_to_char( ch->in_room->description, ch );
	send_to_char("\n\r",ch);
    }

    if ( ( pexit   = in_room->transfer ) == NULL
    ||   ( to_room = pexit->u1.to_room ) == NULL 
    ||   ( to_room->transfer ) != NULL 
    ||   ( to_room->fall ) != NULL 
    ||	 !can_see_room(ch,pexit->u1.to_room))
    {
	to_room = get_room_index(ROOM_VNUM_LIMBO);
    }

    if ( !is_room_owner(ch,to_room) && room_is_private( ch,to_room ))
    {
	to_room = get_room_index(ROOM_VNUM_LIMBO);
    }

    if ( !IS_NPC(ch) )
    {
	bool ts;
	GUILD_DATA *guild;
	RACE_DATA *race;

	ts = TRUE;

	for (guild = to_room->guild; guild != NULL; guild = guild->next)
	{
	    ts = FALSE;
	    if (guild->guild == ch->class)
		ts = TRUE;
	    if ((ch->pcdata->tier >= 2) && (guild->guild == ch->clasb))
		ts = TRUE;
	    if (ts)
		break;
	}
	if (ts)
	{
	    for (race = to_room->race; race != NULL; race = race->next)
	    {
		ts = FALSE;
		if (race->race == ch->race)
		{
		    ts = TRUE;
		    break;
		}
	    }
	}
	if (IS_IMMORTAL(ch))
	    ts = TRUE;

	if (!ts)
	{
	    to_room = get_room_index(ROOM_VNUM_LIMBO);
	}

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch)
	    && !(ch->spirit))
	    {
		if (to_room->fall == NULL)
		{
		    to_room = get_room_index(ROOM_VNUM_LIMBO);
		} else if ( ( pexit   = to_room->fall ) == NULL
		    ||   ( to_room = pexit->u1.to_room   ) == NULL 
		    ||	 !can_see_room(ch,pexit->u1.to_room))
		{
		    to_room = get_room_index(ROOM_VNUM_LIMBO);
		}
	    }
	}

	if (( in_room->sector_type == SECT_WATER_NOSWIM
	||    to_room->sector_type == SECT_WATER_NOSWIM )
  	&&    !IS_AFFECTED(ch,AFF_FLYING) && !(ch->spirit))
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		if (to_room->fall == NULL)
		{
		    to_room = get_room_index(ROOM_VNUM_LIMBO);
		} else if ( ( pexit   = to_room->fall ) == NULL
		    ||   ( to_room = pexit->u1.to_room   ) == NULL 
		    ||	 !can_see_room(ch,pexit->u1.to_room))
		{
		    to_room = get_room_index(ROOM_VNUM_LIMBO);
		}
	    }
	}
    }

    char_from_room( ch );
    char_to_room( ch, to_room );
    if (IS_NPC(ch) || !IS_IMMORTAL(ch))
    {
	for (track = MAX_TRACK-1; track > 0; track--)
	{
	    ch->track_to[track] = ch->track_to[track-1];
	    ch->track_from[track] = ch->track_from[track-1];
	}
	if (IS_AFFECTED(ch,AFF_FLYING) || ch->spirit)
	{
	    ch->track_from[0] = 0;
	    ch->track_to[0] = 0;
	} else {
	    ch->track_from[0] = in_room->vnum;
	    ch->track_to[0] = to_room->vnum;
	}
    }
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level <= LEVEL_HERO
    &&   ch->ghost_level <= LEVEL_HERO)
    {
	if (!quiet)
	    act( "$n has arrived.", ch, NULL, NULL, TO_ROOM );
	if (IS_NPC(ch))
	{
	    if( ch->say_descr[0] != '\0')
	    {
		if (!quiet)
		act( "$n {a$T{x", ch, NULL, ch->say_descr, TO_ROOM );
	    }
	}
    }
    if (!quiet)
	do_look( ch, "auto" );

    return;
}

/* RW Enter movable exits */
void enter_exit( CHAR_DATA *ch, char *arg)
{    
    ROOM_INDEX_DATA *location; 
    int track;
    EXIT_DATA *pexit;

    /* nifty portal stuff */
    if (arg[0] != '\0')
    {
        ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;
	CHAR_DATA *fch, *fch_next;

        old_room = ch->in_room;

	portal = get_obj_list( ch, arg,  ch->in_room->contents );
	
	if (portal == NULL)
	{
	    send_to_char("Alas, you cannot go that way.\n\r",ch);
	    return;
	}

	if (portal->item_type != ITEM_EXIT) 
	{
	    send_to_char("Alas, you cannot go that way.\n\r",ch);
	    return;
	}

	location = get_room_index(portal->value[0]);

	if (location == NULL
	||  location == old_room
	||  !can_see_room(ch,location)
	||  !can_see_door(ch,(long)portal->value[1])
	||  (room_is_private(ch,location) && !IS_TRUSTED(ch,IMPLEMENTOR)))
	{
	    send_to_char("Alas, you cannot go that way.\n\r",ch);
	    return;
	}

	if ( IS_AFFECTED(ch, AFF_CHARM)
	&&   ch->master != NULL
	&&   old_room == ch->master->in_room )
	{
	    send_to_char( "What?  And leave your beloved master?\n\r", ch );
	    return;
	}

        if (IS_NPC(ch) && IS_SET(ch->act,ACT_AGGRESSIVE)
        &&  IS_SET(location->room_flags,ROOM_LAW))
        {
            send_to_char("You aren't allowed in the city.\n\r",ch);
            return;
        }

	if ( !IS_NPC(ch) )
	{
	    int move;

	    if ( old_room->sector_type == SECT_AIR
	    ||   location->sector_type == SECT_AIR )
	    {
		if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch)
		&& !(ch->spirit))
		{
		    if (location->fall == NULL)
		    {
			send_to_char( "You can't fly.\n\r", ch );
			return;
		    } else if ( ( pexit   = location->fall ) == NULL
			||   ( location = pexit->u1.to_room   ) == NULL 
			||	 !can_see_room(ch,pexit->u1.to_room))
		    {
			send_to_char( "You can't fly.\n\r", ch );
			return;
		    }
		}
	    }

	    if (( old_room->sector_type == SECT_WATER_NOSWIM
	    ||    location->sector_type == SECT_WATER_NOSWIM )
  	    &&    !IS_AFFECTED(ch,AFF_FLYING) && !(ch->spirit))
	    {
		OBJ_DATA *obj;
		bool found;

		/*
		* Look for a boat.
		*/
		found = FALSE;

		if (IS_IMMORTAL(ch))
		    found = TRUE;

		for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
		{
		    if ( obj->item_type == ITEM_BOAT )
		    {
			found = TRUE;
			break;
		    }
		}
		if ( !found )
		{
		    if (location->fall == NULL)
		    {
			send_to_char( "You need a boat to go there.\n\r", ch );
			return;
		    } else if ( ( pexit   = location->fall ) == NULL
			||   ( location = pexit->u1.to_room   ) == NULL 
			||	 !can_see_room(ch,pexit->u1.to_room))
		    {
			send_to_char( "You need a boat to go there.\n\r", ch );
			return;
		    }
		}
	    }

	    move = movement_loss[UMIN(SECT_MAX-1, old_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, location->sector_type)]
	     ;

            move /= 2;  /* i.e. the average */


	    /* conditional effects */
	    if (IS_AFFECTED(ch,AFF_FLYING) || IS_AFFECTED(ch,AFF_HASTE))
		move /= 2;

	    if (IS_AFFECTED(ch,AFF_SLOW))
		move *= 2;

	    if ( ch->move < move && !(ch->spirit))
	    {
		send_to_char( "You are too exhausted.\n\r", ch );
		return;
	    }

	    WAIT_STATE( ch, 1 );
	    if (!(ch->spirit))
	        ch->move -= move;
	}


	if ( !IS_AFFECTED(ch, AFF_SNEAK)
	&&   ch->invis_level <= LEVEL_HERO
	&&   ch->ghost_level <= LEVEL_HERO)
	{
	    act( "$n leaves $v.", ch,portal,NULL,TO_ROOM);
	}

	char_from_room(ch);
	char_to_room(ch, location);
	if (IS_NPC(ch) || !IS_IMMORTAL(ch))
	{
	    for (track = MAX_TRACK-1; track > 0; track--)
	    {
		ch->track_to[track] = ch->track_to[track-1];
		ch->track_from[track] = ch->track_from[track-1];
	    }
	    if (IS_AFFECTED(ch,AFF_FLYING) || ch->spirit)
	    {
		ch->track_from[0] = 0;
		ch->track_to[0] = 0;
	    } else {
		ch->track_from[0] = old_room->vnum;
		ch->track_to[0] = location->vnum;
	    }
	}

	if ( !IS_AFFECTED(ch, AFF_SNEAK)
	&&   ch->invis_level <= LEVEL_HERO
	&&   ch->ghost_level <= LEVEL_HERO)
	{
	    act("$n has arrived.",ch,NULL,NULL,TO_ROOM);
	    if (IS_NPC(ch))
	    {
		if( ch->say_descr[0] != '\0')
		{
		    act( "$n says '{a$T{x'", ch, NULL, ch->say_descr, TO_ROOM );
		}
	    }
	}

	do_look(ch,"auto");

	/* protect against circular follows */
	if (old_room == location)
	    return;

    if (ch->shadowed)
    {
	for ( fch = ch->shadower->in_room->people; fch != NULL; fch = fch->next_in_room )
	{
	    if (IS_NPC(fch))
		continue;
	    if (fch == ch->shadower)
		continue;
	    if (!IS_IMMORTAL(fch))
		continue;
	    if (fch->level < get_trust(ch->shadower))
		continue;
	    act( "$n slips out of the room.", ch->shadower, NULL, fch, TO_VICT );
	}
	char_from_room( ch->shadower );
	char_to_room( ch->shadower, location );
	for ( fch = ch->shadower->in_room->people; fch != NULL; fch = fch->next_in_room )
	{
	    if (IS_NPC(fch))
		continue;
	    if (fch == ch->shadower)
		continue;
	    if (!IS_IMMORTAL(fch))
		continue;
	    if (fch->level < get_trust(ch->shadower))
		continue;
	    act( "$n silently slips into the room.", ch->shadower, NULL, fch, TO_VICT );
	}
	act( "You follow $N.", ch->shadower, NULL, ch, TO_CHAR );
	do_look( ch->shadower, "auto" );
    }
    	for ( fch = old_room->people; fch != NULL; fch = fch_next )
    	{
            fch_next = fch->next_in_room;

            if (portal == NULL) 
                continue;
 
            if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
            &&   fch->position < POS_STANDING)
            	do_stand(fch,"");

            if ( fch->master == ch && fch->position == POS_STANDING
	    &&   can_see_room(fch,location))
            {
 
                if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
                &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
                {
                    act("You can't bring $N into the city.",
                    	ch,NULL,fch,TO_CHAR);
                    act("You aren't allowed in the city.",
                    	fch,NULL,NULL,TO_CHAR);
                    continue;
            	}
 
            	act( "You follow $N.", fch, NULL, ch, TO_CHAR );
		enter_exit(fch,arg);
            }
    	}
	return;
    }

    send_to_char("Alas, you cannot go that way.\n\r",ch);
    return;
}

void do_run( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    int door;
    int numrooms;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Run where?\n\r", ch );
	return;
    }
    ch->running = TRUE;
    if ( IS_AFFECTED( ch, AFF_SNEAK ) )
    {
	REMOVE_BIT( ch->affected_by, AFF_SNEAK );
    }

    if ( !str_prefix( arg, "north" ) ) door = 0;
    else if ( !str_prefix( arg, "east"  ) ) door = 1;
    else if ( !str_prefix( arg, "south" ) ) door = 2;
    else if ( !str_prefix( arg, "west"  ) ) door = 3;
    else if ( !str_prefix( arg, "up"    ) ) door = 4;
    else if ( !str_prefix( arg, "down"  ) ) door = 5;
    else
    {
	send_to_char( "Run where?\n\r", ch );
	ch->running = FALSE;
	return;
    }
    numrooms = number_range(1, UMAX(1, (ch->level/2)));
    for ( ; numrooms > 0; numrooms--)
    {
	move_char( ch, door, FALSE, TRUE );
	if (!ch->running)
	    numrooms = 0;
    }
    ch->running = FALSE;
    do_look( ch, "auto" );
    return;
}
/*
void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH, FALSE, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST, FALSE, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH, FALSE, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST, FALSE, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP, FALSE, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN, FALSE, FALSE );
    return;
}

 */

int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( (ch->alignment < 0)
	    &&   ( pexit = ch->in_room->exit[door+6] ) != NULL
            &&   IS_SET(pexit->exit_info, EX_ISDOOR)
            &&   pexit->keyword != NULL
            &&   is_name( arg, pexit->keyword ) )
	    {
                return door+6;
	    }
	    else if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
	    {
		return door;
	    }
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( (ch->alignment < 0)
    &&   (pexit = ch->in_room->exit[door+6] ) != NULL )
    {
	door += 6;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL
    || !can_see_room(ch, ch->in_room->exit[door]->u1.to_room)
    || !can_see_door(ch, ch->in_room->exit[door]->exit_info) )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}



void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
 	/* open portal */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1], EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED))
	    {
		send_to_char("It's already open.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_LOCKED))
	    {
		send_to_char("It's locked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_CLOSED);
	    act("You open $p.",ch,obj,NULL,TO_CHAR);
	    act("$n opens $p.",ch,obj,NULL,TO_ROOM);
	    return;
 	}

	/* 'open object' */
	if ( ( obj->item_type != ITEM_CONTAINER )
	&&   ( obj->item_type != ITEM_PIT ) )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act("You open $p.",ch,obj,NULL,TO_CHAR);
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{

	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||   IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's already closed.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_CLOSED);
	    act("You close $p.",ch,obj,NULL,TO_CHAR);
	    act("$n closes $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'close object' */
	if ( ( obj->item_type != ITEM_CONTAINER )
	&&   ( obj->item_type != ITEM_PIT ) )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	act("You close $p.",ch,obj,NULL,TO_CHAR);
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	if (IS_SET(pexit->exit_info, EX_NOCLOSE) )
	    { send_to_char("You can't do that.\n\r",	ch); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||  IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
	 	return;
	    }

	    if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
	    {
		send_to_char("It can't be locked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already locked.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_LOCKED);
	    act("You lock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n locks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'lock object' */
	if ( ( obj->item_type != ITEM_CONTAINER )
	&&   ( obj->item_type != ITEM_PIT ) )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	act("You lock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 || IS_SET(pexit->exit_info, EX_NOLOCK) )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
 	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already unlocked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'unlock object' */
	if ( ( obj->item_type != ITEM_CONTAINER )
	&&   ( obj->item_type != ITEM_PIT ) )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    if (ch->spirit)
    {
	send_to_char( "That's tough to do without flesh.\n\r", ch);
	return;
    }

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {	
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_PICKPROOF))
	    {
		send_to_char("You failed.\n\r",ch);
		return;
	    }

	    if ( !IS_NPC(ch) )
	    {
		int numper = 0;
		int numout = 0;
		int numgsn = 0;

		numper = number_percent( );
		numgsn = get_skill(ch,gsn_pick_lock);
		numout = numper;

		if (IS_SET(obj->value[1],EX_EASY))
		{
		    if (numout > numgsn)
		    {
			if (numgsn == 0)
			{
			    if (number_percent( ) < 25)
				numout = -1;
			} else {
			    if (number_percent( ) < 66)
				numout = -1;
			}
		    }
		}
		if (IS_SET(obj->value[1],EX_HARD))
		{
		    numout = (numper + ((101-numper)/4));
		}
		if (IS_SET(obj->value[1],EX_INFURIATING))
		{
		    numout = (numper + ((101-numper)/2));
		}
		if (numout > numgsn)
		{
		    send_to_char( "You failed.\n\r", ch);
		    check_improve(ch,gsn_pick_lock,FALSE,2);
		    return;
		}
	    }
	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
	    check_improve(ch,gsn_pick_lock,TRUE,2);
	    return;
	}

	/* 'pick object' */
	if ( ( obj->item_type != ITEM_CONTAINER )
	&&   ( obj->item_type != ITEM_PIT ) )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
        act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
        act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
	check_improve(ch,gsn_pick_lock,TRUE,2);
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "You failed.\n\r",             ch ); return; }
	if ( !IS_NPC(ch) )
	{
	    int numper = 0;
	    int numout = 0;
	    int numgsn = 0;

	    numper = number_percent( );
	    numgsn = get_skill(ch,gsn_pick_lock);
	    numout = numper;

	    if (IS_SET(pexit->exit_info, EX_EASY))
	    {
		if (numout > numgsn)
		{
		    if (numgsn == 0)
		    {
			if (number_percent( ) < 25)
			    numout = -1;
		    } else {
			if (number_percent( ) < 66)
			    numout = -1;
		    }
		}
	    }
	    if (IS_SET(pexit->exit_info, EX_HARD))
	    {
		numout = (numper + ((101-numper)/4));
	    }
	    if (IS_SET(pexit->exit_info, EX_INFURIATING))
	    {
		numout = (numper + ((101-numper)/2));
	    }
	    if (numout > numgsn)
	    {
		send_to_char( "You failed.\n\r", ch);
		check_improve(ch,gsn_pick_lock,FALSE,2);
		return;
	    }
	}

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,gsn_pick_lock,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}

void do_stand( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (argument[0] != '\0')
    {
	if (ch->position == POS_FIGHTING)
	{
	    send_to_char("Maybe you should finish fighting first?\n\r",ch);
	    return;
	}
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],STAND_AT)
	&&   !IS_SET(obj->value[2],STAND_ON)
	&&   !IS_SET(obj->value[2],STAND_IN)))
	{
	    send_to_char("You can't seem to find a place to stand.\n\r",ch);
	    return;
	}
	if (ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no room to stand on $p.",
		ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}
	ch->on = obj;
    }
    
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }
	
	if (obj == NULL)
	{
	    send_to_char( "You wake and stand up.\n\r", ch );
	    act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	   act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	   act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else 
	{
	    act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	do_look(ch,"auto");
	break;

    case POS_RESTING: case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char( "You stand up.\n\r", ch );
	    act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	    act("You stand at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act("You stand on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else
	{
	    act("You stand in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}

void do_rest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("You are already fighting!\n\r",ch);
	return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)
    {
        if (!IS_SET(obj->item_type,ITEM_FURNITURE) 
    	||  (!IS_SET(obj->value[2],REST_ON)
    	&&   !IS_SET(obj->value[2],REST_IN)
    	&&   !IS_SET(obj->value[2],REST_AT)))
    	{
	    send_to_char("You can't rest on that.\n\r",ch);
	    return;
    	}

        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
    	}
	
	ch->on = obj;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if (obj == NULL)
	{
	    send_to_char( "You wake up and start resting.\n\r", ch );
	    act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_AT))
	{
	    act_new("You wake up and rest at $p.",
		    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
	    act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_ON))
        {
            act_new("You wake up and rest on $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
            act_new("You wake up and rest in $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	if (obj == NULL)
	{
	    send_to_char( "You rest.\n\r", ch );
	    act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char("You rest.\n\r",ch);
	    act("$n rests.",ch,NULL,NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You rest at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests at $p.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You rest on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;
    }


    return;
}

void do_sit (CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("Maybe you should finish this fight first?\n\r",ch);
	return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)                                                              
    {
	if (!IS_SET(obj->item_type,ITEM_FURNITURE)
	||  (!IS_SET(obj->value[2],SIT_ON)
	&&   !IS_SET(obj->value[2],SIT_IN)
	&&   !IS_SET(obj->value[2],SIT_AT)))
	{
	    send_to_char("You can't sit on that.\n\r",ch);
	    return;
	}

	if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}

	ch->on = obj;
    }
    switch (ch->position)
    {
	case POS_SLEEPING:
            if (obj == NULL)
            {
            	send_to_char( "You wake and sit up.\n\r", ch );
            	act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
            	act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
            	act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else
            {
            	act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM);
            }

	    ch->position = POS_SITTING;
	    break;
	case POS_RESTING:
	    if (obj == NULL)
		send_to_char("You stop resting.\n\r",ch);
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits at $p.",ch,obj,NULL,TO_ROOM);
	    }

	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_STANDING:
	    if (obj == NULL)
    	    {
		send_to_char("You sit down.\n\r",ch);
    	        act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit down at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You sit down in $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down in $p.",ch,obj,NULL,TO_ROOM);
	    }
    	    ch->position = POS_SITTING;
    	    break;
    }
    return;
}

void do_sleep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING: 
	if (argument[0] == '\0' && ch->on == NULL)
	{
	    send_to_char( "You go to sleep.\n\r", ch );
	    act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	    ch->position = POS_SLEEPING;
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		obj = ch->on;
	    else
	    	obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == NULL)
	    {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	    }
	    if (obj->item_type != ITEM_FURNITURE
	    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
	    &&   !IS_SET(obj->value[2],SLEEP_IN)
	    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		send_to_char("You can't sleep on that!\n\r",ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		act_new("There is no room on $p for you.",
		    ch,obj,NULL,TO_CHAR,POS_DEAD);
		return;
	    }

	    ch->on = obj;
	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
	        act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
	        act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SLEEPING;
	}
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}

void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    act_new( "$n wakes you.", ch, NULL, victim, TO_VICT,POS_SLEEPING );
    do_stand(victim,"");
    return;
}

void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if (IS_AFFECTED(ch,AFF_SNEAK))
	return;

    if ( number_percent( ) < get_skill(ch,gsn_sneak))
    {
	check_improve(ch,gsn_sneak,TRUE,3);
	af.where     = TO_AFFECTS;
	af.type      = gsn_sneak;
	af.level     = ch->level; 
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
    }
    else
	check_improve(ch,gsn_sneak,FALSE,3);

    return;
}

void do_hide( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( number_percent( ) < get_skill(ch,gsn_hide))
    {
	SET_BIT(ch->affected_by, AFF_HIDE);
	check_improve(ch,gsn_hide,TRUE,3);
    }
    else
	check_improve(ch,gsn_hide,FALSE,3);

    return;
}

/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->shielded_by, SHD_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_recall( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    int track;

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET))
    {
	send_to_char("Only players can recall.\n\r",ch);
	return;
    }
	if ( IS_SET(ch->in_room->room_flags,ROOM_ARENA) )
	{
	send_to_char("You can't recall while in the arena!\n\r",ch);
	return;
	}
  
	act( "$n leaves to their recall.", ch, 0, 0, TO_ROOM );

    if ( ch->alignment < 0 )
    {
	if ( ( location = get_room_index( ROOM_VNUM_TEMPLEB ) ) == NULL )
	{
	    send_to_char( "You are completely lost.\n\r", ch );
	    return;
	}
    }
    else 
    {
	if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
	{
	    send_to_char( "You are completely lost.\n\r", ch );
	    return;
	}
    }

    if ( ch->alignment < 0 && !IS_SET( ch->act, PLR_TWIT ) )
    {
	location = get_room_index( home_table[ch->home].evil_recall );
    } else {
	location = get_room_index( home_table[ch->home].good_recall );
    }

    if (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)
    && !IS_SET(ch->master->act, PLR_TWIT))
    {
	if (ch->alignment < 0)
            location = get_room_index( home_table[ch->master->home].evil_recall );
	else
	    location = get_room_index( home_table[ch->master->home].good_recall );
    }

    if ( ( ch->level < 10 && ch->pcdata->tier < 1 ) && !IS_SET( ch->act, PLR_TWIT ) )
    {
	location = get_room_index( ROOM_VNUM_SCHOOL );
    }

    if (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)
    && ch->master->level < 10 && !IS_SET(ch->master->act, PLR_TWIT))
    {
	location = get_room_index( ROOM_VNUM_SCHOOL );
    }

    if ( ch->in_room == location )
	return;

    if ( ( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
    ||   IS_AFFECTED( ch, AFF_CURSE ) || ( ch->move <= 5 ) )
    &&   ( ch->level <= HERO ) && !( ch->spirit ) )
    {
	act( "$g has forsaken you.", ch, NULL, NULL, TO_CHAR );
	return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	skill = get_skill(ch,gsn_recall);

	if ( number_percent() < 80 * skill / 100 )
	{
	    check_improve(ch,gsn_recall,FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );
	check_improve(ch,gsn_recall,TRUE,4);
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
        if ( !IS_NPC(ch) )
        {
	    if (!IS_NPC(victim))
	    {
		if (ch->attacker == FALSE)
		    ch->pcdata->dflee++;
		else
		    ch->pcdata->aflee++;
	    }
	    if ( !IS_NPC(victim) && ch->attacker == TRUE)
	    {
		send_to_char( "The {RWrath of Thoth {YZAPS{x your butt on the way out!\n\r", ch);
		act( "$n is {Yzapped{x in the butt by a lightning bolt from above!", ch, NULL, NULL, TO_ROOM );
		ch->hit -= (ch->hit/4);
	    }
        }
	stop_fighting( ch, TRUE );
	do_mod_favor(ch, 2);
    }

    if (is_clan(ch)
    && (clan_table[ch->clan].hall != ROOM_VNUM_ALTAR)
    && !IS_SET(ch->act, PLR_TWIT) && !(ch->spirit))
	location = get_room_index( clan_table[ch->clan].hall );

    if (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)
    && is_clan(ch->master)
    && (clan_table[ch->master->clan].hall != ROOM_VNUM_ALTAR)
    && !IS_SET(ch->master->act, PLR_TWIT) && !(ch->master->spirit))
        location = get_room_index( clan_table[ch->master->clan].hall );

    if (global_gquest && ch->on_gquest && !IS_SET(ch->act, PLR_TWIT)
    && !(ch->spirit))
	location = get_room_index( ROOM_VNUM_QUEST_RECALL );

    if (global_gquest && IS_NPC(ch) && IS_SET(ch->act,ACT_PET)
    && ch->master->on_gquest && !IS_SET(ch->master->act, PLR_TWIT)
    && !(ch->master->spirit))
	location = get_room_index( ROOM_VNUM_QUEST_RECALL );

    if (!(ch->spirit))
	ch->move = (ch->move * 3) / 4;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    if (IS_NPC(ch) || !IS_IMMORTAL(ch))
    {
	for (track = MAX_TRACK-1; track > 0; track--)
	{
	    ch->track_to[track] = ch->track_to[track-1];
	    ch->track_from[track] = ch->track_from[track-1];
	}
	ch->track_from[0] = ch->in_room->vnum;
	ch->track_to[0] = 0;
    }

    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    
    if (ch->pet != NULL)
	do_recall(ch->pet,"");

    return;
}

void do_camp( CHAR_DATA *ch, char *argument )
{
    char arg [MIL];
    ROOM_INDEX_DATA *location;

    argument = one_argument( argument, arg );

    if ( IS_NPC(ch) ) 
    	return;

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax - camp here.\n\r", ch );
        return;
    }

    if ( ch->pcdata->camp == ch->in_room->vnum )
    {
        send_to_char( "But this is already your camp site!\n\r", ch );
        return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) ||
         IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
    {
        send_to_char( "You are unable to make this room your camp site.\n\r", ch );
        return;
    }

    if (!str_cmp(arg,"here") )
    {
        send_to_char( "This is now your new camping area.\n\r", ch );
        ch->pcdata->camp = ch->in_room->vnum;
        return;
    }

    if (!str_cmp(arg,"site") )
    {
        act( "$n leaves to they're camp site.", ch, NULL, NULL, TO_ROOM );
        location = get_room_index( ch->pcdata->camp );
        char_from_room( ch );
        char_to_room( ch, location );
        act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
        do_look( ch, "auto" );
        return;
    }

    do_camp(ch,"");
    return;
}

void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    CHAR_DATA *mob;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost;

    if ( IS_NPC(ch) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if (ch->spirit)
    {
	send_to_char( "Spirits cannot train.\n\r", ch);
	return;
    }


    if ( argument[0] == '\0' )
    {
	sprintf( buf, "You have %d training sessions.\n\r", ch->train );
	send_to_char( buf, ch );
	argument = "foo";
    }

    cost = 2;

    if ( !str_cmp( argument, "str" ) )
    {
	if ( ( class_table[ch->class].attr_prime == STAT_STR )
	|| ( ( ch->pcdata->tier >= 2 )
	&&   ( class_table[ch->clasb].attr_prime == STAT_STR ) ) )
	    cost    = 1;
	stat        = STAT_STR;
	pOutput     = "strength";
    }

    else if ( !str_cmp( argument, "int" ) )
    {
	if ( ( class_table[ch->class].attr_prime == STAT_INT )
	|| ( ( ch->pcdata->tier >= 2 )
	&&   ( class_table[ch->clasb].attr_prime == STAT_INT ) ) )
	    cost    = 1;
	stat	    = STAT_INT;
	pOutput     = "intelligence";
    }

    else if ( !str_cmp( argument, "wis" ) )
    {
	if ( ( class_table[ch->class].attr_prime == STAT_WIS )
	|| ( ( ch->pcdata->tier >= 2 )
	&&   ( class_table[ch->clasb].attr_prime == STAT_WIS ) ) )
	    cost    = 1;
	stat	    = STAT_WIS;
	pOutput     = "wisdom";
    }

    else if ( !str_cmp( argument, "dex" ) )
    {
	if ( ( class_table[ch->class].attr_prime == STAT_DEX )
	|| ( ( ch->pcdata->tier >= 2 )
	&&   ( class_table[ch->clasb].attr_prime == STAT_DEX ) ) )
	    cost    = 1;
	stat  	    = STAT_DEX;
	pOutput     = "dexterity";
    }

    else if ( !str_cmp( argument, "con" ) )
    {
	if ( ( class_table[ch->class].attr_prime == STAT_CON )
	|| ( ( ch->pcdata->tier >= 2 )
	&&   ( class_table[ch->clasb].attr_prime == STAT_CON ) ) )
	    cost    = 1;
	stat	    = STAT_CON;
	pOutput     = "constitution";
    }

    else if ( !str_cmp(argument, "hp" ) )
	cost = 1;

    else if ( !str_cmp(argument, "mana" ) )
	cost = 1;

    else if ( !str_cmp(argument, "move" ) )
        cost = 1;
 
    else
    {
	strcpy( buf, "You can train:" );
	if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
	    strcat( buf, " str" );
	if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
	    strcat( buf, " int" );
	if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
	    strcat( buf, " wis" );
	if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
	    strcat( buf, " dex" );
	if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
	    strcat( buf, " con" );
	strcat( buf, " hp mana move");

	strcat( buf, ".\n\r" );
	send_to_char( buf, ch );
	return;
    }

    if (!str_cmp("hp",argument))
    {
    	if ( cost > ch->train )
    	{
       	    send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }
 
	ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit +=10;
        act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }
 
    if (!str_cmp("mana",argument))
    {
        if ( cost > ch->train )
        {
            send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }

	ch->train -= cost;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 10;
        ch->mana += 10;
        act( "Your power increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's power increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if (!str_cmp("move",argument))
    {
        if ( cost > ch->train )
        {
            send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }
 
        ch->train -= cost;
        ch->pcdata->perm_move += 10;
        ch->max_move += 10;
        ch->move += 10;
        act( "Your endurance increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's endurance increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
    {
	act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
	return;
    }

    if ( cost > ch->train )
    {
	send_to_char( "You don't have enough training sessions.\n\r", ch );
	return;
    }

    ch->train		-= cost;
  
    ch->perm_stat[stat]		+= 1;
    act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
    act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
    return;
}
/*
 * a2557
 *
 * }
 *
 */

void do_push_drag( CHAR_DATA *ch, char *argument, char *verb )
{
char arg1[MIL];
char arg2[MIL];
char buf[MSL];
ROOM_INDEX_DATA *in_room;
ROOM_INDEX_DATA *to_room;
CHAR_DATA *victim;
EXIT_DATA *pexit;
OBJ_DATA *obj;
int door;

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );
victim = get_char_room(ch,arg1);
obj = get_obj_list( ch, arg1, ch->in_room->contents );

if ( arg1[0] == '\0' || arg2[0] == '\0' )
{
sprintf( buf, "%s whom or what where?\n\r", capitalize(verb));
send_to_char( buf, ch );
return;
}

if ( (!victim || !can_see(ch,victim))
&& (!obj || !can_see_obj(ch,obj)) )
{
sprintf(buf,"%s whom or what where?\n\r", capitalize(verb));
send_to_char( buf, ch );
return;
}

if ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) ) door = 0;
else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east"  ) ) door = 1;
else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) ) door = 2;
else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west"  ) ) door = 3;
else if ( !str_cmp( arg2, "u" ) || !str_cmp( arg2, "up"    ) ) door = 4;
else if ( !str_cmp( arg2, "d" ) || !str_cmp( arg2, "down"  ) ) door = 5;
else
{
sprintf( buf, "Alas, you cannot %s in that direction.\n\r", verb );
send_to_char( buf, ch );
return;
}

if ( obj )
{
in_room = obj->in_room;
if ( ( pexit   = in_room->exit[door] ) == NULL
||   ( to_room = pexit->u1.to_room   ) == NULL 
||	 !can_see_room(ch,pexit->u1.to_room))
{
sprintf( buf, "Alas, you cannot %s in that direction.\n\r", verb );
send_to_char( buf, ch );
return;
}

if ( IS_SET(pexit->exit_info, EX_CLOSED)
|| IS_SET(pexit->exit_info,EX_NOPASS) )
{
act( "You cannot $t it through the $d.", ch, verb, pexit->keyword, TO_CHAR );
act( "$n decides to $t $P around!", ch, verb, obj, TO_ROOM );
return;
}

act( "You attempt to $T $p out of the room.", ch, obj, verb, TO_CHAR );
act( "$n is attempting to $T $p out of the room.", ch, obj, verb, TO_ROOM );

if ( obj->weight >  (2 * can_carry_w (ch)) )
{
act( "$p is too heavy to $T.\n\r", ch, obj, verb, TO_CHAR);
act( "$n attempts to $T $p, but it is too heavy.\n\r", ch, obj, verb, TO_ROOM);
return;
}
if 	 ( !IS_IMMORTAL(ch)
||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) )
{
send_to_char( "It won't budge.\n\r", ch );
return;
}

if ( ch->move > 10 )
{
ch->move -= 10;
send_to_char( "You succeed!\n\r", ch );
act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
if (!str_cmp( verb, "drag" ))
{
act( "$n drags $p $T!", ch, obj, dir_name[door], TO_ROOM );
char_from_room( ch );
char_to_room( ch, pexit->u1.to_room );
do_look( ch, "auto" );
obj_from_room( obj );
obj_to_room( obj, to_room );
act( "$n drags $p into the room.", ch, obj, dir_name[door], TO_ROOM );
}
else if (!str_cmp( verb, "push" ))
{
act( "$p {Wflies{x $T!", ch, obj, dir_name[door], TO_ROOM );
act( "$p {Wflies{x $T!", ch, obj, dir_name[door], TO_CHAR );
char_from_room( ch );
char_to_room( ch, pexit->u1.to_room );
act( "You notice movement from nearby to the $T.",
ch, NULL, dir_name[door], TO_ROOM );
act( "$p {Wflies{x into the room!", ch, obj, dir_name[door], TO_ROOM );
char_from_room( ch );
char_to_room( ch, in_room );
obj_from_room( obj );
obj_to_room( obj, to_room );
}
}
else
{
sprintf( buf, "You are too tired to %s anything around!\n\r", verb );
send_to_char( buf, ch );
}
}
else
{
if ( ch == victim )
{
act( "You $t yourself about the room and look very silly.", ch, verb, NULL, TO_CHAR );
act( "$n decides to be silly and $t $mself about the room.", ch, verb, NULL, TO_ROOM );
return;
}

in_room = victim->in_room;
if ( ( pexit   = in_room->exit[door] ) == NULL
||   ( to_room = pexit->u1.to_room   ) == NULL 
||	 !can_see_room(victim,pexit->u1.to_room))
{
sprintf( buf, "Alas, you cannot %s them that way.\n\r", verb );
send_to_char( buf, ch );
return;
}

if (IS_SET(pexit->exit_info, EX_CLOSED)
&&  (!IS_AFFECTED(victim, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS)))
{
act( "You try to $t them through the $d.", ch, verb, pexit->keyword, TO_CHAR );
act( "$n decides to $t you around!", ch, verb, victim, TO_VICT );
act( "$n decides to $t $N around!", ch, verb, victim, TO_NOTVICT );
return;
}

act( "You attempt to $t $N out of the room.", ch, verb, victim, TO_CHAR );
act( "$n is attempting to $t you out of the room!", ch, verb, victim, TO_VICT );
act( "$n is attempting to $t $N out of the room.", ch, verb, victim, TO_NOTVICT );

if 	 ( !IS_IMMORTAL(ch)
||   (IS_NPC(victim)
&&	 (IS_SET(victim->act,ACT_TRAIN)
||	 IS_SET(victim->act,ACT_PRACTICE)
||	 IS_SET(victim->act,ACT_IS_HEALER)
||	 IS_SET(victim->imm_flags,IMM_SUMMON)
||	 victim->pIndexData->pShop ))
||   victim->in_room == NULL
||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
||   (!str_cmp( verb, "push" ) && victim->position != POS_STANDING)
|| 	 is_safe(ch,victim)
|| 	 (number_percent() > 90)
||   (victim->max_hit > (ch->max_hit + (get_curr_stat(ch,STAT_STR)*20))) )
{
send_to_char( "They won't budge.\n\r", ch );
return;
}

if ( ch->move > 10 )
{
ch->move -= 10;
send_to_char( "You succeed!\n\r", ch );
act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );
if (!str_cmp( verb, "drag" ))
{
move_char( ch, door, FALSE, FALSE );
act( "$n is dragged $T!", victim, NULL, dir_name[door], TO_ROOM );
act( "You are dragged $T!\n\r", victim, NULL, dir_name[door], TO_CHAR );
char_from_room( victim );
char_to_room( victim, pexit->u1.to_room );
do_look( victim, "auto" );
act( "$N drags $n into the room.", victim, NULL, ch, TO_NOTVICT );
}
else if (!str_cmp( verb, "push" ))
{
act( "$n {Wflies{x $T!", victim, NULL, dir_name[door], TO_ROOM );
act( "You {Wfly{x $T!\n\r", victim, NULL, dir_name[door], TO_CHAR );
char_from_room( victim );
char_to_room( victim, pexit->u1.to_room );
do_look( victim, "auto" );
act( "You notice movement from nearby to the $T.",
victim, NULL, dir_name[door], TO_ROOM );
act( "$n {Wflies{x into the room!", victim, NULL, NULL, TO_ROOM );
}
}
else
{
sprintf( buf, "You are too tired to %s anybody around!\n\r", verb );
send_to_char( buf, ch );
}
}

return;
}

void do_push( CHAR_DATA *ch, char *argument )
{
do_push_drag( ch, argument, "push" );
return;
}

void do_drag( CHAR_DATA *ch, char *argument )
{
do_push_drag( ch, argument, "drag" );
return;
}


