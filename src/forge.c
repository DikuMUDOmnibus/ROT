/***************************************************************************
 *      This snippet was written by Donut & Buba for the Khrooon Mud.      *
 *            Original Coded by Yago Diaz <yago@cerberus.uab.es>           *
 *	  (C) November 1996             		   	   	   *
 *	  (C) Last Modification September 1997          		   *
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 **************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1996 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@efn.org)                                  *
*           Gabrielle Taylor                                               *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

CHAR_DATA *  find_forger	args( ( CHAR_DATA * ch ) );

CHAR_DATA * find_forger ( CHAR_DATA *ch )
{
    CHAR_DATA * forger;

    for ( forger = ch->in_room->people; forger != NULL; forger = forger->next_in_room ) {
	if (!IS_NPC2(forger))
	    continue;

        if ( IS_NPC2(forger) && IS_SET(forger->act2, ACT2_FORGER) )
	    return forger;
    }

    if ( forger == NULL ) {
	send_to_char("You can't do that here.\n\r", ch);
	return NULL;
    }

    return NULL;
}


void do_forge( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *	obj;
    CHAR_DATA * forger;
    char	weapon[MAX_INPUT_LENGTH];
    char *	words;
    int		cost;

    forger = find_forger(ch);
				    
    if (!forger)
	return;

    if (argument[0] == '\0') {
	act("$N says 'You have to tell me the type of forge do you want on your weapon.'", ch, NULL, forger, TO_CHAR);
	send_to_char("You have to tell me the weapon you want to forge.\n\r", ch);
	send_to_char("My possible forges are:\n\r",ch);
	send_to_char("\n\r  flame     : flaming weapon           1 platinum\n\r",ch);
	send_to_char("  drain     : vampiric weapon          1 platinum\n\r",ch);
	send_to_char("  shocking  : electric weapon          1 platinum\n\r",ch);
	send_to_char("  frost     : frost weapon             1 platinum\n\r",ch);
	send_to_char("  sharp     : sharp weapon             1 platinum\n\r",ch);
	send_to_char("  vorpal    : vorpal weapon            1 platinum\n\r",ch);
	send_to_char("\n\rType 'forge <weapon> <type>' to forge the weapon.\n\r",ch);
	return;
    }
    
    argument = one_argument ( argument, weapon);

    if ( ( obj = get_obj_carry( ch, weapon ) ) == NULL ) {
        act ("$N says 'You're not carrying that.'", ch, NULL, forger, TO_CHAR);
        return;
    }
    
    if(obj->item_type != ITEM_WEAPON) {
        act ("$N says 'This is not a weapon!.'", ch, NULL, forger, TO_CHAR);
        return;
    }
  
    if ( argument[0] == '\0' ) {
        act ("$N says 'Pardon? type 'forge' to see the list of modifications.'", ch, NULL, forger, TO_CHAR);
	return;
    }    	  
   

    if (!str_prefix(argument,"flame")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_FLAMING) ) {
	    act("$N says '$p is already flaming.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "yrawz braoculo";
	cost  = 1;

	if ( cost > ch->platinum ) {
	    act ("$N says 'You do not have enough platinum for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_flame_blade( skill_lookup("flame blade"), 101, forger, obj, 0 );

	if ( IS_WEAPON_STAT ( obj, WEAPON_FLAMING) ) {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->platinum 	    -= cost;
	    forger->platinum  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }
    
    if (!str_prefix(argument,"drain")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_VAMPIRIC) ) {
	    act("$N says '$p is already vampiric.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "egruui braoculo";
	cost  = 1;

	if ( cost > ch->platinum ) {
	    act ("$N says 'You do not have enough platinum for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}
    
    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_drain_blade( skill_lookup("drain blade"), 101, forger, obj, 0 );

	if ( IS_WEAPON_STAT ( obj, WEAPON_VAMPIRIC) ) {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->platinum        -= cost;
	    forger->platinum  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"shocking")) {
	if (IS_WEAPON_STAT( obj, WEAPON_SHOCKING) ) {
	    act("$N says '$p is already electrical.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "gpaqtuio braoculo";
	cost  = 1;

	if ( cost > ch->platinum ) {
	    act ("$N says 'You do not have enough platinum for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_shocking_blade( skill_lookup("shocking blade"), 101, forger, obj, 0 );

	if ( IS_WEAPON_STAT ( obj, WEAPON_SHOCKING) ) {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->platinum        -= cost;
	    forger->platinum  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"frost")) {
	if (IS_WEAPON_STAT( obj, WEAPON_FROST) ) {
	    act("$N says '$p is already frost.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "yfagh braoculo";
	cost  = 1;

	if ( cost > ch->platinum ) {
	    act ("$N says 'You do not have enough platinum for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_frost_blade( skill_lookup("frost blade"), 101, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_FROST) ) {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->platinum 	    -= cost;
	    forger->platinum  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"sharp")) {
	if (IS_WEAPON_STAT( obj, WEAPON_SHARP) ) {
	    act("$N says '$p is already sharp.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "gpabras braoculo";
	cost  = 1;

	if ( cost > ch->platinum ) {
	    act ("$N says 'You do not have enough platinum for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_sharp_blade( skill_lookup("sharp blade"), 101, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_SHARP) ) {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->platinum 	    -= cost;
	    forger->platinum  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"vorpal")) {
	if (IS_WEAPON_STAT( obj, WEAPON_VORPAL) ) {
	    act("$N says '$p is already vorpal.'", ch, obj, forger, TO_CHAR);
	    return;
	}

	words = "zafsar braoculo";
	cost  = 1;

	if ( cost > ch->platinum ) {
	    act ("$N says 'You do not have enough platinum for my services'", ch, NULL, forger, TO_CHAR);
	    return;
	}

    	act("$n utters the words '$T'.", forger, NULL, words, TO_ROOM);

	spell_vorpal_blade( skill_lookup("vorpal blade"), 101, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_VORPAL) ) {
	    act ("$N gives $p to $n.", ch, obj, forger, TO_ROOM);
	    act ("$N says 'Take care with $p, now is a lot powerful.'", ch, obj, forger, TO_CHAR);
	    ch->platinum 	    -= cost;
	    forger->platinum  += cost;
	}
	else
	    act ("$N says 'I'm sorry I can't help you.'", ch, NULL, forger, TO_CHAR); 
	return;
    }

    act ("$N says 'Pardon? Type 'forge' to see the list of modifications.'", ch, NULL, forger, TO_CHAR);
    return;								 
}


void spell_drain_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA * obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
   
    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)) {
	send_to_char("This weapon is already vampiric.\n\r", ch);
	return;
    }

    if(IS_OBJ_STAT(obj,ITEM_BLESS)) {
	send_to_char("This weapon is too blessed.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_VAMPIRIC;
    affect_to_obj(obj, &af);

    act("$p carried by $n turns dark and vampiric.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you turns dark and vampiric.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_shocking_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA * obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING)) {
	send_to_char("This weapon is already electrical.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_SHOCKING;
    affect_to_obj(obj, &af);

    act("$p carried by $n sparks with electricity.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you sparks with electricity.", ch, obj, NULL, TO_CHAR);
    return;
}


void spell_flame_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FLAMING)) {
	send_to_char("This weapon is already flaming.\n\r", ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FROST)) {
	send_to_char("This weapon is too frost to handle this magic.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_FLAMING;
    affect_to_obj(obj, &af);

    act("$p carried by $n gets a fiery aura.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you gets a fiery aura.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_frost_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FROST)) {
	send_to_char("This weapon is already frost.\n\r", ch);
	return;
      }

    if(IS_WEAPON_STAT(obj,WEAPON_FLAMING)) {
	send_to_char("This weapon is too cold to handle this magic.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_FROST;
    affect_to_obj(obj, &af);

    act("$p carried by $n grows wickedly cold.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you grows wickedly cold.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_sharp_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_SHARP)) {
	send_to_char("This weapon is already sharp.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_SHARP;
    affect_to_obj(obj, &af);

    act("$p carried by $n looks newly honed.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you looks newly honed.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_vorpal_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return ;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_VORPAL)) {
	send_to_char("This weapon is already vorpal.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = level/2;
    af.duration = level;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_VORPAL;
    affect_to_obj(obj, &af);

    act("$p carried by $n gleams witch magical strength.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you gleams witch magical strength.", ch, obj, NULL, TO_CHAR);
    return;
}
