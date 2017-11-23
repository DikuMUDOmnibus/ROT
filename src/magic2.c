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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);

extern char *target_name;

void spell_farsight(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FARSIGHT) )
    {
        if (victim == ch)
          send_to_char("Your eyes are already as good as they get.\n\r",ch);
        else
          act("$N can see just fine.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FARSIGHT;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes jump into focus.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_protection_voodoo(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, SHD_PROTECT_VOODOO) )
    {
	return;
    }
    af.where	= TO_SHIELDS;
    af.type	= sn;
    af.level	= level;
    af.duration	= level;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector = SHD_PROTECT_VOODOO;
    affect_to_char( victim, &af );
    return;
}

void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;

        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
    ||	(is_clan(victim) && (!is_same_clan(ch,victim)
    ||  clan_table[victim->clan].independent)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   

    if (global_gquest && IS_NPC(victim) && victim->on_gquest)
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
	send_to_char("You lack the proper component for this spell.\n\r",ch);
	return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
     	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
     	act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
     	extract_obj(stone);
    }

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 2 + level / 25; 
    portal->value[3] = victim->in_room->vnum;

    obj_to_room(portal,ch->in_room);

    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;

    from_room = ch->in_room;
 
        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   (to_room = victim->in_room) == NULL
    ||   !can_see_room(ch,to_room) || !can_see_room(ch,from_room)
    ||   IS_SET(to_room->room_flags, ROOM_SAFE)
    ||	 IS_SET(from_room->room_flags,ROOM_SAFE)
    ||   IS_SET(to_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(to_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(from_room->room_flags,ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
    ||	 (is_clan(victim) && (!is_same_clan(ch,victim)
    ||   clan_table[victim->clan].independent)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   
 
    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch)
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }
 
    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
        extract_obj(stone);
    }

    /* portal one */ 
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level / 10;
    portal->value[3] = to_room->vnum;
 
    obj_to_room(portal,from_room);
 
    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
	return;

    /* portal two */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level/10;
    portal->value[3] = from_room->vnum;

    obj_to_room(portal,to_room);

    if (to_room->people != NULL)
    {
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
    }
}

void spell_empower( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *object;
    char buf[MSL];
    char *name;
    int new_sn;
    int mana;
    int newmana;
    int newtarget;

    if ( ( new_sn = find_spell( ch,target_name ) ) < 0 )
    {
        send_to_char( "What spell do you wish to bind?\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) )
    {
	if (ch->pcdata->learned[new_sn] == 0)
	{
	    send_to_char( "What spell do you wish to bind?\n\r", ch );
	    return;
	}
	if (ch->pcdata->tier != 2)
	{
	    if (ch->level < skill_table[new_sn].skill_level[ch->class])
	    {
		send_to_char( "What spell do you wish to bind?\n\r", ch );
		return;
	    }
	} else if ((ch->level < skill_table[new_sn].skill_level[ch->class])
		&& (ch->level < skill_table[new_sn].skill_level[ch->clasb]))
	{
	    send_to_char( "What spell do you wish to bind?\n\r", ch );
	    return;
	}
    }
    name = skill_table[new_sn].name;
    if (!strcmp(name, "empower") )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
    if (skill_table[new_sn].spell_fun == spell_null)
    {
	send_to_char( "That's not a spell.\n\r", ch );
	return;
    }
    newtarget = skill_table[new_sn].target;
    if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	mana = 50;
    else
    	mana = UMAX(
	    skill_table[sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->class] ) );

    if (!IS_NPC(ch) && (ch->pcdata->tier == 2))
    {
	if ((ch->level + 2 == skill_table[sn].skill_level[ch->class])
	|| ( ch->level + 2 == skill_table[sn].skill_level[ch->clasb]))
	    mana = 50;
	else
	    mana = UMAX(
		skill_table[sn].min_mana,
		100 / ( 2 + ch->level - UMIN(skill_table[sn].skill_level[ch->class] ,
					     skill_table[sn].skill_level[ch->clasb]) ) );
    }

    if (ch->level + 2 == skill_table[new_sn].skill_level[ch->class])
	newmana = 50;
    else
    	newmana = UMAX(
	    skill_table[new_sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[new_sn].skill_level[ch->class] ) );

    if (!IS_NPC(ch) && (ch->pcdata->tier == 2))
    {
	if ((ch->level + 2 == skill_table[new_sn].skill_level[ch->class])
	|| ( ch->level + 2 == skill_table[new_sn].skill_level[ch->clasb]))
	    newmana = 50;
	else
	    newmana = UMAX(
		skill_table[new_sn].min_mana,
		100 / ( 2 + ch->level - UMIN(skill_table[new_sn].skill_level[ch->class] ,
					     skill_table[new_sn].skill_level[ch->clasb]) ) );
    }

    if ( (ch->mana - mana - newmana) < 0)
    {
        send_to_char( "You do not have enough mana.\n\r", ch );
        return;
    }
    ch->mana -= newmana;

    if ((newtarget == TAR_CHAR_DEFENSIVE) || (newtarget == TAR_CHAR_SELF))
    {
	object = create_object(get_obj_index(OBJ_VNUM_POTION), 0);
    }
    else
    {
	object = create_object(get_obj_index(OBJ_VNUM_SCROLL), 0);
    }
    object->value[0] = ch->level;
    object->value[1] = new_sn;
    object->level = ch->level-5;

    sprintf( buf, "%s %s", object->name, name);
    free_string(object->name);
    object->name = str_dup(buf);

    sprintf( buf, "%s%s", object->short_descr, name);
    free_string(object->short_descr);
    object->short_descr = str_dup(buf);

    if ((newtarget == TAR_CHAR_DEFENSIVE) || (newtarget == TAR_CHAR_SELF))
    {
	sprintf( buf, "$n has created a potion of %s!",name);
	act(buf,ch,object,NULL,TO_ROOM);
	sprintf( buf, "You create a potion of %s!\n\r",name);
	send_to_char(buf,ch);
    }
    else
    {
	sprintf( buf, "$n has created a scroll of %s!",name);
	act(buf,ch,object,NULL,TO_ROOM);
	sprintf( buf, "You create a scroll of %s!\n\r",name);
	send_to_char(buf,ch);
    }
    obj_to_char(object,ch);
    return;
}

void spell_resurrect( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MSL];
    char arg[MSL];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *obj;
    OBJ_DATA *cobj;
    OBJ_DATA *obj_next;
    CHAR_DATA *pet;
    int	length;

    if ( ( obj = get_obj_here( ch, "corpse" ) ) == NULL )
    {
	send_to_char( "There's no corpse here.\n\r", ch );
	return;
    }
    if ( ch->pet != NULL )
    {
	send_to_char("You failed.\n\r",ch);
	return;
    }
    pMobIndex = get_mob_index( MOB_VNUM_CORPSE );
    pet = create_mobile( pMobIndex );
    if (!IS_SET(pet->act, ACT_PET) ) 
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) ) 
        SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x.\n\r",
	pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    free_string( pet->short_descr );
    pet->short_descr = str_dup( str_replace(obj->short_descr, "corpse", "zombie") );
    sprintf( buf, "%s", str_replace(obj->description, "corpse", "zombie") );
    length = strlen(buf)-12;
    strncpy( arg, buf, length);
    arg[length] = '\0';
    sprintf( buf, "%s standing here.\n\r", arg);
    free_string( pet->long_descr );
    pet->long_descr = str_dup( buf );
    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = UMAX(1, UMIN(109, ((ch->level/2)+(obj->level/2))));
    pet->max_hit = dice(moblev_table[pet->level].hpdice,
                        moblev_table[pet->level].hpsides)
                        + moblev_table[pet->level].hpbonus;
    pet->hit = pet->max_hit;
    pet->max_mana = dice(moblev_table[pet->level].hpdice,
                        moblev_table[pet->level].hpsides)
                        + moblev_table[pet->level].hpbonus;
    pet->mana = pet->max_mana;
    pet->damroll = moblev_table[pet->level].dambonus;
    pet->damage[DICE_NUMBER] = moblev_table[pet->level].damdice;
    pet->damage[DICE_TYPE] = moblev_table[pet->level].damsides;

    pet->armor[0] = moblev_table[pet->level].mobac * 10;
    pet->armor[1] = moblev_table[pet->level].mobac * 10;
    pet->armor[2] = moblev_table[pet->level].mobac * 10;
    pet->armor[3] = (moblev_table[pet->level].mobac + 4) * 10;
    {
        int i;

        for (i = 0; i < MAX_STATS; i ++)
            pet->perm_stat[i] = UMIN(25,11 + pet->level/4);

        if (IS_SET(pet->act,ACT_WARRIOR))
        {
            pet->perm_stat[STAT_STR] += 3;
            pet->perm_stat[STAT_INT] -= 1;
            pet->perm_stat[STAT_CON] += 2;
        }

        if (IS_SET(pet->act,ACT_THIEF))
        {
            pet->perm_stat[STAT_DEX] += 3;
            pet->perm_stat[STAT_INT] += 1;
            pet->perm_stat[STAT_WIS] -= 1;
        }

        if (IS_SET(pet->act,ACT_CLERIC))
        {
            pet->perm_stat[STAT_WIS] += 3;
            pet->perm_stat[STAT_DEX] -= 1;
            pet->perm_stat[STAT_STR] += 1;
        }

        if (IS_SET(pet->act,ACT_MAGE))
        {
            pet->perm_stat[STAT_INT] += 3;
            pet->perm_stat[STAT_STR] -= 1;
            pet->perm_stat[STAT_DEX] += 1;
        }
 
        if (IS_SET(pet->act,ACT_RANGER))
        {
            pet->perm_stat[STAT_STR] += 3;
            pet->perm_stat[STAT_CON] -= 1;
            pet->perm_stat[STAT_INT] += 1;
        }
 
        if (IS_SET(pet->act,ACT_DRUID))
        {
            pet->perm_stat[STAT_WIS] += 3;
            pet->perm_stat[STAT_STR] -= 1;
            pet->perm_stat[STAT_DEX] += 1;
        }
 
        if (IS_SET(pet->act,ACT_VAMPIRE))
        {
            pet->perm_stat[STAT_CON] += 3;
            pet->perm_stat[STAT_STR] += 1;
            pet->perm_stat[STAT_WIS] -= 1;
        }
     
        if (IS_SET(pet->off_flags,OFF_FAST))
            pet->perm_stat[STAT_DEX] += 2;
     
        pet->perm_stat[STAT_STR] += pet->size - SIZE_MEDIUM;
        pet->perm_stat[STAT_CON] += (pet->size - SIZE_MEDIUM) / 2;
    }

    for ( cobj = obj->contains; cobj != NULL; cobj = obj_next )
    {
	obj_next = cobj->next_content;
	obj_from_obj( cobj );
	obj_to_room( cobj, ch->in_room );
    }
    extract_obj( obj );
    sprintf( buf, "%s stands up and starts following you.\n\r", pet->short_descr);
    send_to_char( buf, ch);
    sprintf( buf, "%s stands up and starts following $n.", pet->short_descr);
    act( buf, ch, NULL, NULL, TO_ROOM);
    return;
}

void spell_conjure( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MSL];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *stone;
    CHAR_DATA *pet;
 
    if (IS_NPC(ch))
	return;

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_DEMON_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }

    if ( ch->pet != NULL )
    {
	send_to_char("You failed.\n\r",ch);
	return;
    }

    if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
	if (stone->value[0] < 1)
	{
	    act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
	    act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM);
	    act("It flares brightly and explodes into dust.",ch,stone,NULL,TO_CHAR);
	    act("It flares brightly and explodes into dust.",ch,stone,NULL,TO_ROOM);
	    extract_obj( stone );
	    return;
	}
    }

    pMobIndex = get_mob_index( MOB_VNUM_DEMON );
    pet = create_mobile( pMobIndex );
    if (!IS_SET(pet->act, ACT_PET) )
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) )
        SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x.\n\r",
        pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    if (!str_cmp(class_table[ch->class].name,"lich"))
    {
	sprintf(buf, "The skeletal warrior follows its master.\n\r{GThe mark of %s is on it's forehead.{x.\n\r",
	    ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );
	sprintf(buf, "skeletal warrior undead pet");
	free_string( pet->name );
	pet->name = str_dup( buf );
	sprintf(buf, "A demon is standing here.\n\r");
	free_string( pet->short_descr );
	pet->short_descr = str_dup( buf );
    }

/* Set the demon to casters level-15 and recalculate the dice rolls */
    pet->level = UMAX(0,ch->level - 15);
    pet->max_hit = dice(moblev_table[pet->level].hpdice,
			moblev_table[pet->level].hpsides)
			+ moblev_table[pet->level].hpbonus;
    pet->hit = pet->max_hit;
    pet->max_mana = dice(moblev_table[pet->level].hpdice,
                        moblev_table[pet->level].hpsides)
                        + moblev_table[pet->level].hpbonus;
    pet->mana = pet->max_mana;
    pet->damroll = moblev_table[pet->level].dambonus;
    pet->damage[DICE_NUMBER] = moblev_table[pet->level].damdice;
    pet->damage[DICE_TYPE] = moblev_table[pet->level].damsides;

    pet->armor[0] = moblev_table[pet->level].mobac * 10;
    pet->armor[1] = moblev_table[pet->level].mobac * 10;
    pet->armor[2] = moblev_table[pet->level].mobac * 10;
    pet->armor[3] = (moblev_table[pet->level].mobac + 4) * 10;

/* Now reset the demons stats */
    {
	int i;

        for (i = 0; i < MAX_STATS; i ++)
            pet->perm_stat[i] = UMIN(25,11 + pet->level/4);
            
        if (IS_SET(pet->act,ACT_WARRIOR))
        {
            pet->perm_stat[STAT_STR] += 3;
            pet->perm_stat[STAT_INT] -= 1;
            pet->perm_stat[STAT_CON] += 2;
        }
        
        if (IS_SET(pet->act,ACT_THIEF))
        {
            pet->perm_stat[STAT_DEX] += 3;
            pet->perm_stat[STAT_INT] += 1;
            pet->perm_stat[STAT_WIS] -= 1;
        }
        
        if (IS_SET(pet->act,ACT_CLERIC))
        {
            pet->perm_stat[STAT_WIS] += 3;
            pet->perm_stat[STAT_DEX] -= 1;
            pet->perm_stat[STAT_STR] += 1;
        }
        
        if (IS_SET(pet->act,ACT_MAGE))
        {
            pet->perm_stat[STAT_INT] += 3;
            pet->perm_stat[STAT_STR] -= 1;
            pet->perm_stat[STAT_DEX] += 1;
        }

        if (IS_SET(pet->act,ACT_RANGER))
        {
            pet->perm_stat[STAT_STR] += 3;
            pet->perm_stat[STAT_CON] -= 1;
            pet->perm_stat[STAT_INT] += 1;
        }
 
        if (IS_SET(pet->act,ACT_DRUID))
        {
            pet->perm_stat[STAT_WIS] += 3;
            pet->perm_stat[STAT_STR] -= 1;
            pet->perm_stat[STAT_DEX] += 1;
        }
 
        if (IS_SET(pet->act,ACT_VAMPIRE))
        {
            pet->perm_stat[STAT_CON] += 3;
            pet->perm_stat[STAT_STR] += 1;
            pet->perm_stat[STAT_WIS] -= 1;
        }
        
        if (IS_SET(pet->off_flags,OFF_FAST))
            pet->perm_stat[STAT_DEX] += 2;
            
        pet->perm_stat[STAT_STR] += pet->size - SIZE_MEDIUM;
        pet->perm_stat[STAT_CON] += (pet->size - SIZE_MEDIUM) / 2;
 
    }
    char_to_room( pet, ch->in_room );
    if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
        stone->value[0] = UMAX(0, stone->value[0]-1);
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_CHAR);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_ROOM);
    } else
    {  
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_CHAR);
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_ROOM);
    }
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = ch->level;
    pet->max_hit = pet->level * 30;
    pet->hit = pet->max_hit;
    pet->armor[0] = pet->level/2;
    pet->armor[1] = pet->level/2;
    pet->armor[2] = pet->level/2;
    pet->armor[3] = pet->level/3;

    return;
}

void spell_animate( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MSL];
    char arg[MSL];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    CHAR_DATA *pet;
    int	length;

    if  ((obj->pIndexData->vnum > 17)
	|| (obj->pIndexData->vnum < 12))
    {
	send_to_char( "That's not a body part!\n\r", ch );
	return;
    }

    pMobIndex = get_mob_index( MOB_VNUM_ANIMATE );
    pet = create_mobile( pMobIndex );
    SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GIt's branded with the mark of %s.{x.\n\r",
	obj->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    free_string( pet->short_descr );
    pet->short_descr = str_dup( obj->short_descr );
    free_string( pet->name );
    pet->name = str_dup( obj->name );
    sprintf( buf, "%s", obj->description);
    length = strlen(buf)-12;
    strncpy( arg, buf, length);
    arg[length] = '\0';
    sprintf( buf, "%s floating here.\n\r", arg);
    free_string( pet->long_descr );
    pet->long_descr = str_dup( buf );
    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    obj_from_char( obj );
    sprintf( buf, "%s floats up and starts following you.\n\r", pet->short_descr);
    send_to_char( buf, ch);
    sprintf( buf, "%s floats up and starts following $n.", pet->short_descr);
    act( buf, ch, NULL, NULL, TO_ROOM);
    return;
}

void spell_iceshield( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int duration;

    if (IS_SHIELDED(victim, SHD_ICE))
    {
	if(victim == ch)
	    send_to_char("You are already surrounded by an {Cicy{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by an {Cicy{x shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    duration = level/6;

    if (!IS_IMMORTAL(victim))
    {
	bool found = TRUE;

	if (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	{
	    duration = level/12;
	    found = FALSE;
	}
	if (victim->pcdata->tier == 2)
	{
	    if (skill_table[sn].skill_level[victim->clasb] > LEVEL_HERO)
	    {
		if (found)
		{
		    duration = level/6;
		} else
		{
		    duration = level/12;
		}
	    } else {
		if (found)
		{
		    duration = level/3;
		} else
		{
		    duration = level/6;
		}
	    }
	}
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = duration;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_ICE;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by an {Cicy{x shield.\n\r", victim);
   act("$n is surrounded by an {Cicy{x shield.",victim, NULL,NULL,TO_ROOM);
   return;
}

void spell_fireshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
   
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int duration;

    if (IS_SHIELDED(victim, SHD_FIRE))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by a {Rfirey{x shield.\r\n", ch);
	else
	    act("$N is already surrounded by a {Rfiery{x shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    duration = level/6;

    if (!IS_IMMORTAL(victim))
    {
	bool found = TRUE;

	if (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	{
	    duration = level/12;
	    found = FALSE;
	}
	if (victim->pcdata->tier == 2)
	{
	    if (skill_table[sn].skill_level[victim->clasb] > LEVEL_HERO)
	    {
		if (found)
		{
		    duration = level/6;
		} else
		{
		    duration = level/12;
		}
	    } else {
		if (found)
		{
		    duration = level/3;
		} else
		{
		    duration = level/6;
		}
	    }
	}
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = duration;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_FIRE;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Rfiery{x shield.\n\r", victim);
    act("$n is surrounded by a {Rfiery{x shield.",victim, NULL,NULL,TO_ROOM);
    return;

}

void spell_shockshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int duration;

    if (IS_SHIELDED(victim, SHD_SHOCK))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded in a {Bcrackling{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by a {Bcrackling{x shield.",ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    duration = level/6;

    if (!IS_IMMORTAL(victim))
    {
	bool found = TRUE;

	if (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	{
	    duration = level/12;
	    found = FALSE;
	}
	if (victim->pcdata->tier == 2)
	{
	    if (skill_table[sn].skill_level[victim->clasb] > LEVEL_HERO)
	    {
				if (found)
				{
						duration = level/6;
				} 
				else
				{
						duration = level/12;
				}
	    } 
		else 
		{
				if (found)
				{
						duration = level/3;
				} 
				else
				{
						duration = level/6;
				}
		}
	}
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = duration;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_SHOCK;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Bcrackling{x field.\n\r",victim);
    act("$n is surrounded by a {Bcrackling{x shield.",victim, NULL,NULL, TO_ROOM);
    return;
}

void spell_quest_pill( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (IS_NPC(victim))
	return;

    victim->aqps++;
    victim->qps++;
    victim->qps++;
    victim->qps++;
    victim->qps++;
    victim->qps++;
    victim->questpoints++;
    victim->questpoints++;
    victim->questpoints++;
    victim->questpoints++;
    victim->questpoints++;
    victim->questpoints++;
    victim->questpoints++;
    victim->questpoints++;
    victim->questpoints++;
    victim->questpoints++;
    send_to_char( "{YYou've gained an {RAutoQuest Point{Y!{x\n\r", victim );
    send_to_char( "{YYou've gained five Global Quest Points!!{x\n\r", victim );
    send_to_char( "{YYou've gained ten Player Quest Points!!{x\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok {R!{B!{Y!{x{z{b.{x\n\r", ch );
    return;
}

void spell_voodoo( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    char name[MIL];
    char buf[MSL];
    OBJ_DATA *bpart;
    OBJ_DATA *doll;

    bpart = get_eq_char(ch,WEAR_HOLD);
    if  ((bpart == NULL)
    ||   (bpart->pIndexData->vnum < 12)
    ||   (bpart->pIndexData->vnum > 17))
    {
	send_to_char("You are not holding a body part.\n\r",ch);
	return;
    }
    if (bpart->value[4] == 0)
    {
	send_to_char("This body part is from a mobile.\n\r",ch);
	return;
    }
    do_mod_favor(ch, 8);
    one_argument(bpart->name, name);
    doll = create_object(get_obj_index(OBJ_VNUM_VOODOO), 0);
    sprintf( buf, doll->short_descr, name );
    free_string( doll->short_descr );
    doll->short_descr = str_dup( buf );
    sprintf( buf, doll->description, name );
    free_string( doll->description );
    doll->description = str_dup( buf );
    sprintf( buf, doll->name, name );
    free_string( doll->name );
    doll->name = str_dup( buf );
    act( "$p morphs into a voodoo doll",ch,bpart,NULL,TO_CHAR);
    obj_from_char( bpart );
    obj_to_char(doll,ch);
    equip_char(ch,doll,WEAR_HOLD);
    act( "$n has created $p!", ch, doll, NULL, TO_ROOM );
    return;
}

void spell_shadow( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *gch;

    if (IS_NPC(victim))
    {
	send_to_char("You can only use this on other players.\n\r",ch);
	return; 
    }
    if ( ( time_info.hour < 6 ) || ( time_info.hour > 18 ) )
    {
	send_to_char( "You can only use this during the day.\n\r", ch );
	return;
    }
    if (victim == ch)
    {
	send_to_char("You move toward your shadow, but it moves away from you.\n\r",ch);
	return; 
    }
    if (IS_IMMORTAL(victim) || (victim->level < get_trust(ch)))
    {
	send_to_char("Spell failed.\n\r",ch);
	return; 
    }

    if (ch->shadow || victim->shadowed)
    {
	send_to_char("Spell failed.\n\r",ch);
	return; 
    }

    ch->shadow = TRUE;
    ch->shadowing = victim;
    victim->shadowed = TRUE;
    victim->shadower = ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if (IS_NPC(gch))
	    break;
	if (gch == ch)
	    continue;
	if (!IS_IMMORTAL(gch))
	    continue;
	if (gch->level < get_trust(ch))
	    continue;
	act( "$n slips out of the room.", ch, NULL, gch, TO_VICT );
    }
    char_from_room( ch );
    char_to_room( ch, victim->in_room );
    do_look( victim, "auto" );
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if (IS_NPC(gch))
	    break;
	if (gch == ch)
	    continue;
	if (!IS_IMMORTAL(gch))
	    continue;
	if (gch->level < get_trust(ch))
	    continue;
	act( "$n silently slips into the room.", ch, NULL, NULL, TO_ROOM );
    }
    act("You silently slip into %N's shadow.", ch, NULL, victim, TO_CHAR);
    return;
}

/*
void spell_summon_lgolem( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *gch;
    CHAR_DATA *golem;
    AFFECT_DATA af;
    int i=0;

    if (is_affected(ch,sn))
    {
	send_to_char("You lack the power to summon another golem right now.\n\r", ch);
	return;
    }

    send_to_char("You attempt to summon a lesser golem.\n\r",ch);
    act("$n attempts to summon a lesser golem.",ch,NULL,NULL,TO_ROOM);

    for (gch = char_list; gch != NULL; gch = gch->next)
    {
    if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM) && gch->master == ch && ( gch->pIndexData->vnum == MOB_VNUM_LGOLEM ) )
    {
    i++;
    if (i > 5)
    {
	send_to_char("More golems are more than you can control!\n\r",ch);
	return;
    }
    }
    }
    golem = create_mobile( get_mob_index(MOB_VNUM_LGOLEM) );

    for (i = 0; i < MAX_STATS; i ++)
    golem->perm_stat[i] = UMIN(25,15 + ch->level/10);
    golem->perm_stat[STAT_STR] += 3;
    golem->perm_stat[STAT_INT] -= 1;
    golem->perm_stat[STAT_CON] += 2;
    golem->max_hit = IS_NPC(ch)? URANGE(ch->max_hit,1 * ch->max_hit,30000) : UMIN( (2 * ch->pcdata->perm_hit) + 400,30000);
    golem->hit = golem->max_hit;
    golem->max_mana = IS_NPC(ch)? ch->max_mana : ch->pcdata->perm_mana;
    golem->mana = golem->max_mana;
    golem->level = ch->level;
    for (i=0; i < 3; i++)
    golem->armor[i] = interpolate(golem->level,100,-100);
    golem->armor[3] = interpolate(golem->level,100,0);
    golem->gold = 0;
    golem->timer = 0;
    golem->damage[DICE_NUMBER] = 3;
    golem->damage[DICE_TYPE] = 10;
    golem->damage[DICE_BONUS] = ch->level / 2;

    char_to_room(golem,ch->in_room);
    send_to_char("You summoned a lesser golem!\n\r",ch);
    act("$n summons a lesser golem!",ch,NULL,NULL,TO_ROOM);

    af.where              = TO_AFFECTS;
    af.type               = sn;
    af.level              = level;
    af.duration           = 25;
    af.bitvector          = 0;
    af.modifier           = 0;
    af.location           = APPLY_NONE;
    affect_to_char(ch, &af);
    SET_BIT(golem->affected_by, AFF_CHARM);
    golem->master = golem->leader = ch;
DB * */

void spell_rasp(int sn,int level,CHAR_DATA *ch,void *vo,int target,int dam)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int mana;

    if ( saves_spell( level, victim,DAM_MENTAL) )
    {
	send_to_char("You feel fingers trying to pick your brain.\n\r",victim);  	
	send_to_char("They are unaffected by your rasp!\n\r",ch);  	
	WAIT_STATE( ch,1.75 * PULSE_VIOLENCE );
	return;
    }
    if ((victim->mana >= 250 ) && (victim->level >= 10))
    {
	dam = victim->mana/2 * UMIN (number_range(1,2),((ch->level)+(ch->mana))/4);
	mana = number_range(ch->level,victim->mana)/4;
	ch->mana	+= mana;
	victim->mana	-= mana;  
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
    ch->attacker = TRUE;
    victim->attacker = FALSE;
    }
	send_to_char("You feel your brain being squeezed.\n\r",victim);
	send_to_char("{R{zMm{x{rmm{x{R{zMm{x{rmm{x{R{zMm{x{rmm{x.\n\r",ch);
	damage( ch, victim, dam, sn, DAM_MENTAL, TRUE);
	WAIT_STATE( ch,0.75 * PULSE_VIOLENCE );
	return;
}



void spell_acid_arrow( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo; 
    int dam; 
    dam = dice( level, 12 );

    if ( saves_spell( level, victim, DAM_ACID ) ) dam /= 2; 
    damage( ch, victim, dam, sn, DAM_ACID, TRUE); 
    return; 
} 

void spell_etheral_fist( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ 
    CHAR_DATA *victim = (CHAR_DATA *) vo; 
    int dam; 
    dam = dice( level, 12 );

    if ( saves_spell( level, victim, DAM_ENERGY ) ) dam /= 2;
    act("A fist of black, otherworldly ether rams into $N, leaving $M looking stunned!",ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ENERGY,TRUE); 
    return; 
}

void spell_spectral_furor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ 
    CHAR_DATA *victim = (CHAR_DATA *) vo; 
    int dam; 
    dam = dice( level, 8 );

    if ( saves_spell( level, victim, DAM_ENERGY ) ) dam /= 2;
    act("The fabric of the cosmos strains in fury about $N!",ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ENERGY,TRUE); 
    return; 
}

void spell_disruption( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 9 );

    if ( saves_spell( level, victim, DAM_ENERGY ) ) dam /= 2;
    act("A weird energy encompasses $N, causing you to question $S continued existence.", ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ENERGY,TRUE);
    return;
}

void spell_sonic_resonance( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 7 );

    if ( saves_spell( level, victim, DAM_ENERGY ) ) dam /= 2;
    act("A cylinder of kinetic energy enshrouds $N causing $S to resonate.", ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ENERGY,TRUE);
    WAIT_STATE( victim, skill_table[sn].beats );
    return;
}
/* mental */
void spell_mind_wrack( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 7 );

    if ( saves_spell( level, victim, DAM_MENTAL ) ) dam /= 2;
    act("$n stares intently at $N, causing $N to seem very lethargic.", ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_MENTAL,TRUE);
    return;
}

void spell_mind_wrench( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 9 );

    if ( saves_spell( level, victim, DAM_MENTAL ) ) dam /= 2;
    act("$n stares intently at $N, causing $N to seem very hyperactive.", ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_MENTAL,TRUE);
    return;
}

/* acid */
void spell_sulfurus_spray( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 7 );

    if ( saves_spell( level, victim, DAM_ACID ) ) dam /= 2;
    act("A stinking spray of sulfurous liquid rains down on $N." ,ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
}

void spell_caustic_font( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 9 );

    if ( saves_spell( level, victim, DAM_ACID ) ) dam /= 2;
    act("A fountain of caustic liquid forms below $N.  The smell of $S degenerating tissues is revolting! ", ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
}

void spell_acetum_primus( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 8 );

    if ( saves_spell( level, victim, DAM_ACID ) ) dam /= 2;
    act("A cloak of primal acid enshrouds $N, sparks form as it consumes all it touches. ",ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
}

/*  Electrical  */
void spell_galvanic_whip( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 7 );

    if ( saves_spell( level, victim, DAM_LIGHTNING ) ) dam /= 2;
    act("$n conjures a whip of ionized particles, which lashes ferociously at $N.", ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_LIGHTNING,TRUE);
    return;
}

void spell_magnetic_trust( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 8 );

    if ( saves_spell( level, victim, DAM_LIGHTNING ) ) dam /= 2;
    act("An unseen energy moves nearby, causing your hair to stand on end!", ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_LIGHTNING,TRUE);
    return;
}


void spell_meteor( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return;
    }

    send_to_char( "A dark shadow falls across the land.\n\r", ch );
    act( "$n raises $s arms and a shadow falls across the land.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	    {
		if ( ( ch->fighting == NULL )
		&& ( !IS_NPC( ch ) )
		&& ( !IS_NPC( vch ) ) )
		{
		    ch->attacker = TRUE;
		    vch->attacker = FALSE;
		}
		if (IS_AFFECTED(vch,AFF_PASS_DOOR))
		    damage(ch,vch,(level/2),sn,DAM_BASH,TRUE);
		else
		    damage( ch,vch,level + dice(100, 500), sn, DAM_BASH,TRUE);
	    }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "A dark shadow falls across the land.\n\r", vch );
    }

    return;
}
void spell_quantum_spike( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice( level, 9 );

    if ( saves_spell( level, victim, DAM_LIGHTNING ) ) dam /= 2;
    act("$N seems to dissolve into tiny unconnected particles, then is painfully reassembled.", ch,NULL,victim,TO_NOTVICT);
    damage( ch, victim, dam, sn,DAM_LIGHTNING,TRUE);
    return;
}

/*
SPELL: Ionwave
DESC: Sends a wave of energy from the caster in all directions.
EFFECT: Everyone NOT the caster.
AUTHOR: The Mage.
*/
void spell_ionwave( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *pChar, *pChar_next;
    int dam;
    pChar_next = NULL;

    for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
    {
	pChar_next = pChar->next_in_room;
    if ( !is_safe( ch, pChar ) && (pChar != ch))
    {
	act( "$n sends a huge wave of energy out! The energy burns you{x!", ch, NULL, pChar, TO_VICT    );
	dam = number_range( 25, 100 );
    if ( saves_spell( ch->level, pChar, DAM_ENERGY ) ) dam /= 2;
	damage( ch, pChar, dam, sn, DAM_ENERGY,TRUE);
    }
    }
    return;
}

/*
SPELL: Vaccine
DESC: Adds resistance to DISEASE.
EFFECT: Target.
AUTHOR: The Mage.
*/
void spell_vaccine( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
     CHAR_DATA *victim = (CHAR_DATA *) vo;
     AFFECT_DATA af;

     if ( is_affected( victim, sn ) || (IS_SET(ch->res_flags,RES_DISEASE)))
     {
     	if (victim == ch)
     	send_to_char("You are already vaccinated.\n\r",ch);
     else
     	act("$N is already vaccinated.",ch,NULL,victim,TO_CHAR);
     return;
     }
     af.where     = TO_RESIST;
     af.type      = sn;
     af.level     = level;
     af.duration  = level;
     af.location  = APPLY_NONE;
     af.modifier  = 0;
     af.bitvector = RES_DISEASE;
     affect_to_char( victim, &af );
     send_to_char( "{BYou feel someone vaccinating you.\n\r{x", victim );
     if ( ch != victim )
     	act("{B$N{x is vaccinated by your magic.",ch,NULL,victim,TO_CHAR);
     return;
}

/*
SPELL: Banshee Scream
DESC: Sends a blast of SOUND out in all directions from the caster.
EFFECT: SOUND damage to everyone not the CASTER.
AUTHOR: The Mage.
*/
void spell_banshee_scream( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *pChar, *pChar_next;
    int dam;
    pChar_next = NULL;

/* This spell will when used.. wipe out your mana */
    for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
    {
	pChar_next = pChar->next_in_room;
    if ( !is_safe( ch, pChar ) && (pChar != ch))
    {
	act( "$n screams a horrible sound! Your ears pop{x!", ch, NULL, pChar, TO_VICT    );
	dam = number_range( 25, 100 );
    if ( saves_spell( ch->level, pChar, DAM_SOUND ) ) dam /= 2;
    damage( ch, pChar, dam, sn, DAM_SOUND,TRUE);
    }
    }
    return;
}
/* ================================ */
/*
SPELL: Sunbeam
DESC: Sends a blast of HOT LIGHT in a specific direction from the
SUN to a target..
EFFECT: Specific target.  Does LIGHT damage.
AUTHOR: The Mage.
*/
void spell_sunbeam( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return;
    }

    if ( weather_info.sky > SKY_CLOUDLESS )
    {
	send_to_char( "You need good sunny weather.\n\r", ch );
	return;
    }

    dam = number_range( 25, 100 );

    if ( saves_spell( level, victim, DAM_LIGHT ) ) dam /= 2;
    damage( ch, victim, dam, sn,DAM_LIGHT,TRUE);
    return;
}
