/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Castain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*************************************************************************** 
*       ROT 1.4 is copyright 1996-1997 by Russ Walsh                       * 
*       By using this code, you have agreed to follow the terms of the     * 
*       ROT license, in the file doc/rot.license                           * 
***************************************************************************/

/*
 * Vehicle.c - ROM/ROT vehicle code. Ver. 2.05
 *
 * Code is Copyright 1997 by Dominic J. Eidson, code may be freely 
 * distributed and modified.
 * 
 */

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
/*
 * Local functions.
 */
int	find_hack_door	args( ( CHAR_DATA *ch, char *arg ) );
void	enter_hack_exit	args( ( CHAR_DATA *ch, OBJ_DATA *obj, char *arg ) );
void    move_vehicle	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int door ) );
bool	check_blind	args( ( CHAR_DATA *ch ) );
void    show_char_to_char args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
void	do_hack_exits	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
BUFFER * show_list_to_char      args( ( OBJ_DATA *list, CHAR_DATA *ch,
                                    bool fShort, bool fShowNothing ) );
void	enter_hack_exit args( ( CHAR_DATA *ch, OBJ_DATA *obj, char *arg) );
void 	do_hack_look	args( (CHAR_DATA *ch, char *argument ) );
void	do_look		args( (CHAR_DATA *ch, char *argument ) );

DECLARE_DO_FUN(do_stand         );

/* New function needed for proper working */

OBJ_DATA *get_obj_exit( char *argument, OBJ_DATA *list )
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
        if ( (obj->item_type == ITEM_EXIT) && is_name(arg, obj->name) )
        {
            if ( ++count == number )
                return obj;
        }
    }
    return NULL;
}

/*
 * Take care of exiting a vehicle
 */

void do_leave(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *room;
    char buf[MSL];

    /* Are we in a vehicle? */
    if(ch->in_room->inside_of == NULL)
    {
	send_to_char("You are not riding anything.",ch);
	return;
    }
    sprintf(buf,"%s leaves %s.",IS_NPC(ch) ? ch->short_descr : capitalize(ch->name), ch->in_room->inside_of->short_descr);
    act("You leave $T.",ch,NULL,ch->in_room->inside_of->short_descr,TO_CHAR);
    act(buf,ch,NULL,NULL,TO_ROOM);
    room = ch->in_room->inside_of->in_room;
    char_from_room(ch);
    char_to_room(ch,room);
    act(buf, ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");
}

/*
 * the fullowing function is tricky. We are using a portal object as 
 * vehicle. This means, we must dereference everything through 
 *  ch->in_room->inside_of, which is the vehicle object.
 * I am not yet sure how well this will work.
 */
void do_hack_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MSL];
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];
    BUFFER *outlist;
    int number,count;

    if(IS_NPC(ch) && !ch->in_room->inside_of)
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch);
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room->inside_of->in_room ) )
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->inside_of->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	send_to_char( "{e", ch);
	send_to_char( ch->in_room->inside_of->in_room->name, ch);
	send_to_char( "{x", ch);

	if (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT))
	{
	    sprintf(buf," [Room %d]",ch->in_room->inside_of->in_room->vnum);
	    send_to_char(buf,ch);
	}

	send_to_char( "\n\r", ch );

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{
	    send_to_char( "  ",ch);
	    send_to_char( ch->in_room->inside_of->in_room->description, ch );
	    if (ch->in_room->inside_of->in_room->vnum == chain)
	    {
		send_to_char("A huge black iron chain as thick as a tree trunk is drifting above the ground\n\r",ch);
		send_to_char("here.\n\r",ch);
	    }
	}

        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	{
	    send_to_char("\n\r",ch);
            do_hack_exits( ch, ch->in_room->inside_of->in_room, "auto" );
	}

	outlist = show_list_to_char( ch->in_room->inside_of->in_room->contents, ch, FALSE, FALSE );
	page_to_char( buf_string(outlist), ch );
	free_buf(outlist);
	show_char_to_char( ch->in_room->inside_of->in_room->people,ch );
	return;
    }
    return;
}

/*
 * Thanks to Zrin for auto-exit part.
 */
void do_hack_exits(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument )
{
    extern char * const dir_name[];
    char buf[MSL];
    EXIT_DATA *pexit;
    bool found;
    bool round;
    bool fAuto;
    int door;
    int outlet;

    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if (fAuto)
	sprintf(buf,"[Exits:");
    else if (IS_IMMORTAL(ch))
	sprintf(buf,"Obvious exits from room %d:\n\r", room->vnum);
    else
	sprintf(buf,"Obvious exits:\n\r");

    found = FALSE;
    for ( door = 0; door < 6; door++ )
    {
	round = FALSE;
	outlet = door;
	if ( ( ch->alignment < 0 )
	&&   ( pexit = room->exit[door+6] ) != NULL)
	    outlet += 6;
	if ( ( pexit = room->exit[outlet] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room) 
	&&   !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    found = TRUE;
	    round = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, dir_name[outlet] );
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s",
		    capitalize( dir_name[outlet] ),
		    room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
			: pexit->u1.to_room->name
		    );
		if (IS_IMMORTAL(ch))
		    sprintf(buf + strlen(buf), 
			" (room %d)\n\r",pexit->u1.to_room->vnum);
		else
		    sprintf(buf + strlen(buf), "\n\r");
	    }
	}
	if (!round)
	{
	    OBJ_DATA *portal;
	    ROOM_INDEX_DATA *to_room;

	    portal = get_obj_exit( dir_name[door], room->contents );
	    if (portal != NULL)
	    {
		found = TRUE;
		round = TRUE;
		if ( fAuto )
		{
		    strcat( buf, " " );
		    strcat( buf, dir_name[door] );
		}
		else
		{
		    to_room = get_room_index(portal->value[0]);
		    sprintf( buf + strlen(buf), "%-5s - %s",
			capitalize( dir_name[door] ),
			room_is_dark( to_room )
			    ?  "Too dark to tell"
			    : to_room->name
			);
		    if (IS_IMMORTAL(ch))
			sprintf(buf + strlen(buf), 
			    " (room %d)\n\r",to_room->vnum);
		    else
			sprintf(buf + strlen(buf), "\n\r");
		}
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "]\n\r" );

    send_to_char( buf, ch );
    return;
}

void move_vehicle( CHAR_DATA *ch, OBJ_DATA *obj, int door )
{
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    char buf[MSL];
    CHAR_DATA *vch;


    in_room = obj->in_room;
    if ( ( pexit = in_room->exit[door] ) == NULL )
    {
	send_to_char("There is no way to go that direction.", ch );
	return;
    }
    if ( ( ch->alignment > 0 )
    &&   door < 6
    &&   ( pexit   = in_room->exit[door+6] ) != NULL )
    {
	door += 6;
    } else if ( ( pexit = in_room->exit[door] ) == NULL)
    {
	if (0)
	{
	    OBJ_DATA *portal;

	    portal = get_obj_list( ch, dir_name[door],  
				ch->in_room->inside_of->in_room->contents );
	    if (portal != NULL)
	    {
		enter_hack_exit( ch, portal, dir_name[door] );
		return;
	    }
	}
    }

    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(ch,pexit->u1.to_room))
    {
	return;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED))
    {
	return;
    }

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    send_to_char( "Vehicles can't fly.\n\r", ch);
	    return;
	}

	if ( in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM )
	{
	    send_to_char("You can't take a vehicle there!", ch);
	    return;
	}

    sprintf(buf, "%s rolls off %s.", capitalize(ch->in_room->inside_of->short_descr), dir_name[door]);

    for ( vch = ch->in_room->inside_of->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	send_to_char(buf,vch);
    }

    obj_from_room( obj );
    obj_to_room( obj, to_room );

    sprintf(buf, "%s rolls in.", capitalize(ch->in_room->inside_of->short_descr));

    for ( vch = ch->in_room->inside_of->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	send_to_char(buf,vch);
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
	do_hack_look( vch, "auto" );

    if (in_room == to_room) /* no circular follows */
	return;
}

/* RW Enter movable exits */
void enter_hack_exit( CHAR_DATA *ch, OBJ_DATA *obj, char *arg)
{    
    ROOM_INDEX_DATA *location; 

    if (arg[0] != '\0')
    {
        ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;

        old_room = obj->in_room;

	portal = get_obj_list( ch, arg,  obj->in_room->contents );
	
	if (portal == NULL)
	{
	    send_to_char("And how would you do that?\n\r",ch);
	    return;
	}

	if (portal->item_type != ITEM_EXIT) 
	{
	    send_to_char("And how would you do that?\n\r",ch);
	    return;
	}

	location = get_room_index(portal->value[0]);

	if (location == NULL
	||  location == old_room
	||  !can_see_room(ch,location) 
	||  (room_is_private(ch,location) && !IS_TRUSTED(ch,IMPLEMENTOR)))
	{
	    send_to_char("And how would you do that?\n\r",ch);
	    return;
	}

	if ( !IS_NPC(ch) )
	{

	    if ( old_room->sector_type == SECT_AIR
	    ||   location->sector_type == SECT_AIR )
	    {
		send_to_char("Vehicles can't fly.", ch);
		return;
	    }

	    if (( old_room->sector_type == SECT_WATER_NOSWIM
	    ||    location->sector_type == SECT_WATER_NOSWIM ))
	    {
		    send_to_char( "Vehicles can not go there.\n\r", ch );
		    return;
	    }

	}

	if ( !IS_AFFECTED(ch, AFF_SNEAK)
	&&   ch->invis_level <= LEVEL_HERO
	&&   ch->ghost_level <= LEVEL_HERO)
	{
	    act("$n rolls off $T.",ch,NULL,arg,TO_ROOM);
	}

	obj_from_room(obj);
	obj_to_room(obj, location);


	if ( !IS_AFFECTED(ch, AFF_SNEAK)
	&&   ch->invis_level <= LEVEL_HERO
	&&   ch->ghost_level <= LEVEL_HERO)
	{
	    act("$n rolls in.",ch,NULL,NULL,TO_ROOM);
	}

	do_hack_look(ch,"auto");

	/* protect against circular follows */
	if (old_room == location)
	    return;

	return;
    }

    send_to_char("Alas, you cannot go that way.\n\r",ch);
    return;
}


void do_north( CHAR_DATA *ch, char *argument )
{
    /*
    if(ch->in_room->inside_of)
	send_to_char("You are riding.\n\r",ch);
    else
     */
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

/* RT Enter portals */



void do_enter( CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *location;

    if ( ch->fighting != NULL )
        return;

    /* nifty portal stuff */
    if (argument[0] != '\0')
    {
        ROOM_INDEX_DATA *old_room;
        OBJ_DATA *portal;
        CHAR_DATA *fch, *fch_next;

        old_room = ch->in_room;

        portal = get_obj_list( ch, argument,  ch->in_room->contents );

// steve
	
	if (portal == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
/*
     if (portal->item_type != ITEM_PORTAL || portal->item_type != ITEM_VEHICLE
        ||  (IS_SET(portal->value[1],EX_CLOSED) && !IS_TRUSTED(ch,KNIGHT)))
        {
            send_to_char("You can't seem to find a way in.\n\r",ch);
            return;
        }
 */

	if (portal->item_type == ITEM_CORPSE_PC && !str_cmp(portal->owner, ch->name) && ch->spirit)
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_next;

	    for ( obj = portal->contains;  obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		obj_from_obj( obj );
		if ( obj->item_type == ITEM_MONEY)
		{
		    if (obj->value[0] > 0)
			add_cost(ch,obj->value[0],VALUE_SILVER);
		    if (obj->value[1] > 0)
			add_cost(ch,obj->value[1],VALUE_GOLD);
		    if (obj->value[2] > 0)
			add_cost(ch,obj->value[2],VALUE_PLATINUM);
		    extract_obj( obj );
		} else {
		    obj_to_char( obj, ch );
		}
	    }
	    extract_obj( portal );
	    ch->spirit = 0;
	    send_to_char("You reach down and touch your corpse, and are suddenly\n\r",ch);
	    send_to_char("sucked inside.  You wake up to the agonizing pain of your last\n\r",ch);
	    send_to_char("battle, and slowly climb to your feet.\n\r",ch);
	    return;
	}


if(portal->item_type == ITEM_PORTAL)
{
        if (!IS_TRUSTED(ch,KNIGHT) && !IS_SET(portal->value[2],GATE_NOCURSE)
        &&  (IS_AFFECTED(ch,AFF_CURSE)
        ||   IS_SET(old_room->room_flags,ROOM_NO_RECALL)))
        {
            send_to_char("Something prevents you from leaving...\n\r",ch);
            return;
        }

        if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[3] == -1)
        {
            location = get_random_room(ch);
            portal->value[3] = location->vnum; /* for record keeping :) */
        }
        else if (IS_SET(portal->value[2],GATE_BUGGY) && (number_percent() < 5))
            location = get_random_room(ch);
        else
            location = get_room_index(portal->value[3]);

        if (location == NULL
        ||  location == old_room
        ||  !can_see_room(ch,location)
		||  !can_see_door(ch,(long)portal->value[1]) 
        ||  (room_is_private(ch,location) && !IS_TRUSTED(ch,IMPLEMENTOR)))
        {
           act("$p doesn't seem to go anywhere.",ch,portal,NULL,TO_CHAR);
           return;
        }

        if (IS_NPC(ch) && IS_SET(ch->act,ACT_AGGRESSIVE)
        &&  IS_SET(location->room_flags,ROOM_LAW))
        {
            send_to_char("Something prevents you from leaving...\n\r",ch);
            return;
        }

        act("$n steps into $p.",ch,portal,NULL,TO_ROOM);

        if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
            act("You enter $p.",ch,portal,NULL,TO_CHAR);
        else
            act("You walk through $p and find yourself somewhere else...",
                ch,portal,NULL,TO_CHAR);

        char_from_room(ch);
        char_to_room(ch, location);

        if (IS_SET(portal->value[2],GATE_GOWITH)) /* take the gate along */
        {
            obj_from_room(portal);
            obj_to_room(portal,location);
        }

        if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
            act("$n has arrived.",ch,portal,NULL,TO_ROOM);
        else
            act("$n has arrived through $p.",ch,portal,NULL,TO_ROOM);

        do_look(ch,"auto");

        /* charges */
        if (portal->value[0] > 0)
        {
            portal->value[0]--;
            if (portal->value[0] == 0)
                portal->value[0] = -1;
        }

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

            if (portal == NULL || portal->value[0] == -1)
            /* no following through dead portals */
                continue;

            if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
            &&   fch->position < POS_STANDING)
                do_stand(fch,"");

            if ( fch->master == ch && fch->position == POS_STANDING)
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
                do_enter(fch,argument);
            }
        }

        if (portal != NULL && portal->value[0] == -1)
        {
            act("$p fades out of existence.",ch,portal,NULL,TO_CHAR);
            if (ch->in_room == old_room)
                act("$p fades out of existence.",ch,portal,NULL,TO_ROOM);
            else if (old_room->people != NULL)
            {
                act("$p fades out of existence.",
                    old_room->people,portal,NULL,TO_CHAR);
                act("$p fades out of existence.",
                    old_room->people,portal,NULL,TO_ROOM);
            }
            extract_obj(portal);
        }
        return;
    }

    if(portal->item_type == ITEM_VEHICLE)
    {
        if (!IS_TRUSTED(ch,KNIGHT) && (IS_AFFECTED(ch,AFF_CURSE)
        ||   IS_SET(old_room->room_flags,ROOM_NO_RECALL)))
        {
            send_to_char("Something prevents you from leaving...\n\r",ch);
            return;
        }

            location = get_room_index(portal->value[3]);

        if (location == NULL
        ||  location == old_room
        ||  !can_see_room(ch,location)
        ||  (room_is_private(ch,location) && !IS_TRUSTED(ch,IMPLEMENTOR)))
        {
           act("$p doesn't seem to go anywhere.",ch,portal,NULL,TO_CHAR);
           return;
        }

        if (IS_NPC(ch) && IS_SET(ch->act,ACT_AGGRESSIVE)
        &&  IS_SET(location->room_flags,ROOM_LAW))
        {
            send_to_char("Something prevents you from leaving...\n\r",ch);
            return;
        }

        act("$n enters $p.",ch,portal,NULL,TO_ROOM);


        char_from_room(ch);
        char_to_room(ch, location);

        act("$n has entered $p.",ch,portal,NULL,TO_ROOM);

        do_look(ch,"auto");

        /* protect against circular follows */
        if (old_room == location)
            return;

	if(portal->item_type == ITEM_VEHICLE)
	{
	    ch->in_room->inside_of = portal;
	}

        for ( fch = old_room->people; fch != NULL; fch = fch_next )
        {
            fch_next = fch->next_in_room;

            if (portal == NULL || portal->value[0] == -1)
            /* no following through dead portals */
                continue;

            if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
            &&   fch->position < POS_STANDING)
                do_stand(fch,"");

            if ( fch->master == ch && fch->position == POS_STANDING)
            {

                if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
                &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
                {
                    act("You can't take $N into vehicles.",
                        ch,NULL,fch,TO_CHAR);
                    act("You aren't allowed in vehicles.",
                        fch,NULL,NULL,TO_CHAR);
                    continue;
                }

                act( "You follow $N.", fch, NULL, ch, TO_CHAR );
                do_enter(fch,argument);
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

    }
    send_to_char("Nope, can't do it.\n\r",ch);
    return;
}


void do_drive(CHAR_DATA *ch, char *argument)
{
     int door;
    if(ch->in_room->inside_of == NULL)
    {
	send_to_char("You are not riding anything.\n\r",ch);
	return;
    }
    if(argument[0] == '\0')
    {
	send_to_char("Syntax: drive <direction>\n\r",ch);
	return;
    }
    

    door = find_hack_door(ch, argument);

    move_vehicle(ch, ch->in_room->inside_of, door);

    return;
}

int find_hack_door( CHAR_DATA *ch, char *arg )
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
        &&   ( pexit = ch->in_room->inside_of->in_room->exit[door+6] ) != NULL
        &&   IS_SET(pexit->exit_info, EX_ISDOOR)
        &&   pexit->keyword != NULL
        &&   is_name( arg, pexit->keyword ) )
            {
                return door+6;
            }
     else if ( ( pexit = ch->in_room->inside_of->in_room->exit[door] ) != NULL
            &&   IS_SET(pexit->exit_info, EX_ISDOOR)
            &&   pexit->keyword != NULL
            &&   is_name( arg, pexit->keyword ) )
            {
                return -1;
            }
        }
        act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
        return -1;
    }

    if ( (ch->alignment < 0)
    &&   (pexit = ch->in_room->inside_of->in_room->exit[door+6] ) != NULL )
    {
        door += 6;
    }
    if ( ( pexit = ch->in_room->inside_of->in_room->exit[door] ) == NULL )
    {
        act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
        return -1;
    }
    return door;
}
