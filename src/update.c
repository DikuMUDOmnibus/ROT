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
#include <string.h>
#include <time.h>
#include "merc.h"
#include "music.h"
#include "interp.h"

/*
 * Local functions.
 */
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
int	move_gain	args( ( CHAR_DATA *ch ) );
void	mobile_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	char_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	aggr_update	args( ( void ) );
void    quest_update    args( ( void ) );
void    auction_update    args( ( void ) );
void    who_html_update args( ( void ) );

extern int xpq_timer;
extern int prquest_timer;
extern int damq_timer;
/* used for saving */

int	save_number = 0;



/*
 * Advancement stuff.
 */
void change_level( CHAR_DATA *ch )
{
    char buf[MSL];
    int loc, i;
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;
    OBJ_DATA *obj;

    if (IS_NPC(ch))
	return;

    ch->pcdata->last_level = 
	( ch->played + (int) (current_time - ch->logon) ) / 3600;

    sprintf( buf, "the %s",
	title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );

    if (!IS_SET(ch->comm,COMM_TITLE_LOCK))
	set_title( ch, buf );

    if (IS_SET(ch->act, PLR_LQUEST))
	REMOVE_BIT(ch->act, PLR_LQUEST);

    if (ch->pcdata->tier != 2)
    {
	add_hp	= con_app[get_curr_stat(ch,STAT_CON)].hitp + 
		    number_range(class_table[ch->class].hp_min,
				 class_table[ch->class].hp_max );
	add_mana = number_range(4,(4*get_curr_stat(ch,STAT_INT)
				  + get_curr_stat(ch,STAT_WIS))/2);
	if (!class_table[ch->class].fMana)
	    add_mana /= 3;
    } else {
	add_hp	= con_app[get_curr_stat(ch,STAT_CON)].hitp + UMAX(
		    number_range(class_table[ch->class].hp_min,
				 class_table[ch->class].hp_max ),
		    number_range(class_table[ch->clasb].hp_min,
				 class_table[ch->clasb].hp_max ));
	add_mana = number_range(4,(4*get_curr_stat(ch,STAT_INT)
				  + get_curr_stat(ch,STAT_WIS))/2);
	if ((!class_table[ch->class].fMana)&&(!class_table[ch->clasb].fMana))
	    add_mana /= 3;
    }

    add_move	= number_range( 1, (get_curr_stat(ch,STAT_CON)
				  + get_curr_stat(ch,STAT_DEX))/5 );
    add_prac	= wis_app[get_curr_stat(ch,STAT_WIS)].practice;

    ch->can_aquest = 0;
    add_hp = add_hp * 9/10;
    add_hp = add_hp * 7/4;
    add_mana = add_mana * 9/10;
    add_mana = add_mana * 7/4;
    add_move = add_move * 9/10;
    add_move = add_move * 2;

    add_hp	= UMAX(  4, add_hp   );
    add_mana	= UMAX(  4, add_mana );
    add_move	= UMAX(  12, add_move );

    ch->max_hit 	+= add_hp;
    ch->max_mana	+= add_mana;
    ch->max_move	+= add_move;
    ch->practice	+= add_prac;
    ch->train		+= 1;

    ch->pcdata->perm_hit	+= add_hp;
    ch->pcdata->perm_mana	+= add_mana;
    ch->pcdata->perm_move	+= add_move;

    sprintf( buf, "Your gain is: %d/%d hp, %d/%d m, %d/%d mv %d/%d prac.\n\r",
	add_hp,		ch->max_hit,
	add_mana,	ch->max_mana,
	add_move,	ch->max_move,
	add_prac,	ch->practice
	);
   send_to_char( buf, ch );
   if (ch->hit < ch->max_hit/.25) { ch->hit = ( ( ch->hit + 0.75 ) + number_range( 5, ch->max_hit/2) ); }
   if (ch->mana < ch->max_mana/.25) { ch->mana = ( ( ch->mana + 0.75 ) + number_range( 5, ch->max_mana/2) ); }
   if (ch->move < ch->max_move/.25) { ch->move = ( ( ch->move + 0.75 ) + number_range( 5, ch->max_move/2) ); }
   if (ch->hit > ch->max_hit) { ch->hit = ch->max_hit; }
   if (ch->mana > ch->max_mana) { ch->mana = ch->max_mana; }
   if (ch->move > ch->max_move) { ch->move = ch->max_move; }
   	affect_strip        (ch,gsn_plague);    
	affect_strip        (ch,gsn_poison);    
	affect_strip        (ch,gsn_blindness);    
	affect_strip        (ch,gsn_sleep);    
	affect_strip        (ch,gsn_curse);   
 
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
	obj = get_eq_char(ch,loc);
	if (obj == NULL)
	    continue;
	if (obj->clan)
	{
	    for (i = 0; i < 4; i++)
	    {
		ch->armor[i] += apply_ac( ch->level-1, obj, loc, i );
		ch->armor[i] -= apply_ac( ch->level, obj, loc, i );
	    }
	}
    }
    if ((ch->level > 9) && (ch->newbie != 0))
    {
	ch->newbie = 0;
	send_to_char("{RYou now have full channel permissions.{x\n\r",ch);
    }
    if (IS_IMMORTAL(ch) && ch->clock)
	ch->clock = 0;
    if (ch->clock)
    {
	if (ch->clock <= ch->level)
	{
	    ch->clock = 0;
	    send_to_char("{RYou are now authorized to change your clan status again.{x\n\r",ch);
	}
    }
    if (IS_HERO(ch))
	ch->pcdata->advanced = 0;
    return;
}   

void change_level_quiet( CHAR_DATA *ch )
{
    char buf[MSL];
    int loc, i;
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;
    OBJ_DATA *obj;

    if (IS_NPC(ch))
	return;

    ch->pcdata->last_level = 
	( ch->played + (int) (current_time - ch->logon) ) / 3600;

    sprintf( buf, "the %s",
	title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );

    if (!IS_SET(ch->comm,COMM_TITLE_LOCK))
	set_title( ch, buf );

    if (IS_SET(ch->act, PLR_LQUEST))
	REMOVE_BIT(ch->act, PLR_LQUEST);

    if (ch->pcdata->tier != 2)
    {
	add_hp	= con_app[get_curr_stat(ch,STAT_CON)].hitp + 
		    number_range(class_table[ch->class].hp_min,
				 class_table[ch->class].hp_max );
	add_mana = number_range(4,(4*get_curr_stat(ch,STAT_INT)
				  + get_curr_stat(ch,STAT_WIS))/2);
	if (!class_table[ch->class].fMana)
	    add_mana /= 3;
    } else {
	add_hp	= con_app[get_curr_stat(ch,STAT_CON)].hitp + UMAX(
		    number_range(class_table[ch->class].hp_min,
				 class_table[ch->class].hp_max ),
		    number_range(class_table[ch->clasb].hp_min,
				 class_table[ch->clasb].hp_max ));
	add_mana = number_range(4,(4*get_curr_stat(ch,STAT_INT)
				  + get_curr_stat(ch,STAT_WIS))/2);
	if ((!class_table[ch->class].fMana)&&(!class_table[ch->clasb].fMana))
	    add_mana /= 3;
    }
    add_move	= number_range( 1, (get_curr_stat(ch,STAT_CON)
				  + get_curr_stat(ch,STAT_DEX))/5 );
    add_prac	= wis_app[get_curr_stat(ch,STAT_WIS)].practice;

    ch->can_aquest = 0;
    add_hp = add_hp * 9/10;
    add_hp = add_hp * 7/4;
    add_mana = add_mana * 9/10;
    add_mana = add_mana * 7/4;
    add_move = add_move * 9/10;
    add_move = add_move * 2;

    add_hp	= UMAX(  4, add_hp   );
    add_mana	= UMAX(  4, add_mana );
    add_move	= UMAX(  12, add_move );

    ch->max_hit 	+= add_hp;
    ch->max_mana	+= add_mana;
    ch->max_move	+= add_move;
    ch->practice	+= add_prac;
    ch->train		+= 1;

    ch->pcdata->perm_hit	+= add_hp;
    ch->pcdata->perm_mana	+= add_mana;
    ch->pcdata->perm_move	+= add_move;

    for (loc = 0; loc < MAX_WEAR; loc++)
    {
	obj = get_eq_char(ch,loc);
	if (obj == NULL)
	    continue;
	if (obj->clan)
	{
	    for (i = 0; i < 4; i++)
	    {
		ch->armor[i] += apply_ac( ch->level-1, obj, loc, i );
		ch->armor[i] -= apply_ac( ch->level, obj, loc, i );
	    }
	}
    }
    if ((ch->level > 9) && (ch->newbie != 0))
    {
	ch->newbie = 0;
	send_to_char("{RYou now have full channel permissions.{x\n\r",ch);
    }
    if (IS_IMMORTAL(ch) && ch->clock) 
        ch->clock = 0;
    if (ch->clock) 
    { 
        if (ch->clock <= ch->level) 
        { 
            ch->clock = 0; 
            send_to_char("{RYou are now authorized to change your clan status again.{x\n\r",ch); 
        } 
    } 
    if (IS_HERO(ch))
	ch->pcdata->advanced = 0;
    return;
}   

void gain_exp( CHAR_DATA *ch, int gain )
{
    char buf[MSL];

    if ( IS_NPC(ch) || ch->level >= LEVEL_HERO )
	return;

    if (IS_SET(ch->act, PLR_LQUEST))
	return;
    ch->exp = UMAX( exp_per_level(ch,ch->pcdata->points), ch->exp + gain );
    while ( ch->level < LEVEL_HERO && ch->exp >= 
	exp_per_level(ch,ch->pcdata->points) * (ch->level+1) )
    {
	if ((ch->level == 50)
	||  (ch->level == 100))
	{
	    ch->exp = exp_per_level(ch,ch->pcdata->points) * (ch->level+1);
	    ch->exp--;
	    sprintf(buf, "{RYou must now complete a level quest to reach level %d{x\n\r",
		ch->level+1);
	    send_to_char(buf, ch);
	    SET_BIT(ch->act, PLR_LQUEST);
	    buf[0] = '\0';
	    save_char_obj(ch);
	    return;
	}
	send_to_char( "You raise a level!!  ", ch );
	ch->level += 1;
	sprintf(buf,"$N has attained level %d!",ch->level);
	wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
	change_level( ch );
	save_char_obj(ch);
    }

    return;
}

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain =  5 + ch->level;
 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

	switch(ch->position)
	{
	    default : 		gain /= 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
	    case POS_RESTING:  					break;
	    case POS_FIGHTING:	gain /= 3;		 	break;
 	}
    }
    else
    {
	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->level/2); 
	if (ch->pcdata->tier != 2)
	{
	    gain += class_table[ch->class].hp_max - 10;
	} else
	{
	    gain += UMAX(class_table[ch->class].hp_max - 10,
			 class_table[ch->clasb].hp_max - 10);
	}
 	number = number_percent();
	if (number < get_skill(ch,gsn_fast_healing))
	{
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch,gsn_fast_healing,TRUE,8);
	}

	switch ( ch->position )
	{
	    default:	   	gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:  	gain /= 2;			break;
	    case POS_FIGHTING: 	gain /= 6;			break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    gain = gain * ch->in_room->heal_rate / 90;
    
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 90;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /=2 ;

    if (ch->in_room->heal_neg)
	return gain;

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = 5 + ch->level;
	switch (ch->position)
	{
	    default:		gain /= 2;		break;
	    case POS_SLEEPING:	gain = 3 * gain/2;	break;
   	    case POS_RESTING:				break;
	    case POS_FIGHTING:	gain /= 3;		break;
    	}
    }
    else
    {
	gain = ((get_curr_stat(ch,STAT_WIS) 
	      + get_curr_stat(ch,STAT_INT) + ch->level) / 3) * 2;
	number = number_percent();
	if (number < get_skill(ch,gsn_meditation))
	{
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
	        check_improve(ch,gsn_meditation,TRUE,8);
	}
	if (ch->pcdata->tier != 2)
	{
	    if (!class_table[ch->class].fMana)
		gain /= 2;
	} else {
	    if ((!class_table[ch->class].fMana)&&(!class_table[ch->clasb].fMana))
		gain /= 2;
	}
	switch ( ch->position )
	{
	    default:		gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:	gain /= 2;			break;
	    case POS_FIGHTING:	gain /= 6;			break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    gain = gain * ch->in_room->mana_rate / 90;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[4] / 90;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
	gain = UMAX( 15, ch->level );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);		break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;	break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
    }

    gain = gain * ch->in_room->heal_rate/90;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 90;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

    return UMIN(gain, ch->max_move - ch->move);
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
	return;

    condition				= ch->pcdata->condition[iCond];
    if (condition == -1)
        return;
    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_HUNGER:
	    send_to_char( "You are hungry.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are thirsty.\n\r", ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;
	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if ( !IS_NPC(ch) || ch->in_room == NULL || IS_AFFECTED(ch,AFF_CHARM))
	    continue;

	if (ch->in_room->area->empty && !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
	    continue;

	/* Examine call for special procedure */
	if ( ch->spec_fun != 0 )
	{
	    if ( (*ch->spec_fun) ( ch ) )
		continue;
	}

	if (ch->pIndexData->pShop != NULL) /* give him some platinum */
	    if ((ch->platinum * 100 + ch->gold) < ch->pIndexData->wealth)
	    {
		ch->platinum += ch->pIndexData->wealth * number_range(1,20)/5000000;
		ch->gold += ch->pIndexData->wealth * number_range(1,20)/50000;
	    }
	 
	/*
	 * Check triggers only if mobile still in default position
	 */
	if ( ch->position == ch->pIndexData->default_pos )
	{
	    /* Delay */
	    if ( HAS_TRIGGER( ch, TRIG_DELAY) 
	    &&   ch->mprog_delay > 0 )
	    {
		if ( --ch->mprog_delay <= 0 )
		{
		    mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_DELAY );
		    continue;
		}
	    } 
	    if ( HAS_TRIGGER( ch, TRIG_RANDOM) )
	    {
		if( mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_RANDOM ) )
		continue;
	    }
	}

	/* That's all for sleeping / busy monster, and empty zones */
	if ( ch->position != POS_STANDING )
	    continue;

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits( 6 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE) && can_loot(ch, obj)
		     && obj->cost > max  && obj->cost > 0)
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL) 
	&& number_bits(3) == 0
	&& ( door = number_bits( 5 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) 
	&& ( !IS_SET(ch->act, ACT_OUTDOORS)
	||   !IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)) 
	&& ( !IS_SET(ch->act, ACT_INDOORS)
	||   IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)))
	{
	    move_char( ch, door, FALSE, FALSE );
	}
    }

    return;
}



/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MSL];
    DESCRIPTOR_DATA *d;
    int diff;

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  2:
	weather_info.sunlight = MOON_RISE;
	strcat( buf, "The {Wm{Doo{Wn{x lights up the {Dn{ci{Dg{ch{Dt {Cs{Wk{Cy{x.\n\r" );
	break;

    case  6:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "The {Dd{Wa{Dy{x has begun.\n\r" );
	break;

    case  7:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "The {Ys{Ru{Yn{x rises in the east.\n\r" );
	break;

	case  12:
	weather_info.sunlight = SUN_NOON;
	strcat( buf, "{x{yThe {x{YSun reaches {x{yit's zenith{x.\n\r" );
	break;

    case 18:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "The {Ys{Ru{Yn{x slowly disappears in the west.\n\r" );
	break;

    case 19:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "The {Dn{ci{Dg{ch{Dt{x has begun.\n\r" );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The {Cs{Wk{Cy{x is getting {Wc{Cl{Wo{Cu{Wd{Cy{x.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "It starts to {cr{Da{ci{Dn{x.\n\r" );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The {Cc{Wl{Co{Wu{Cd{Ws{x disappear.\n\r" );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "{WLightning{x flashes in the {Ws{Ck{Wy{x.\n\r" );
	    weather_info.sky = SKY_LIGHTNING;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The {Cr{Da{Ci{Dn{x stopped.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The {Wlightning{x has stopped.\n\r" );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		send_to_char( buf, d->character );
	}
    }

    return;
}



/*
 * Update all chars, including mobs.
*/
void char_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;

    ch_quit	= NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
	save_number = 0;


    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	ch_next = ch->next;

        if ( ch->timer > 30 )
            ch_quit = ch;
/*
	if ( ch->hit > 30000 ) { ch->hit  = 30000; }
	if ( ch->mana > 30000 ) { ch->mana  = 30000; }
	if ( ch->move > 30000 ) { ch->move  = 30000; }
 */
//	show_warning( ch );
	if ( IS_NPC(ch) && ch->hastimer )
	{
	    if ( ++ch->timer > 5 )
	    {
            	act("$n decays into dust.",ch,NULL,NULL,TO_ROOM);
            	extract_char(ch,TRUE);
	    }
	}

	if ( ch->position >= POS_STUNNED )
	{
            /* check to see if we need to go home */
            if (IS_NPC(ch) && ch->zone != NULL && ch->zone != ch->in_room->area
            && ch->desc == NULL &&  ch->fighting == NULL 
	    && !IS_AFFECTED(ch,AFF_CHARM) && number_percent() < 5)
            {
            	act("$n wanders on home.",ch,NULL,NULL,TO_ROOM);
            	extract_char(ch,TRUE);
            	continue;
            }

	    if (!ch->spirit && !IS_NPC(ch))
	    {
		if ( ch->in_room->heal_neg && !IS_IMMORTAL(ch) )
		    ch->hit -= hit_gain(ch);
	        else if ( ch->hit < ch->max_hit )
		    ch->hit  += hit_gain(ch);
	        else
		    ch->hit = ch->max_hit;

	        if ( ch->mana < ch->max_mana )
		    ch->mana += mana_gain(ch);
	        else
		    ch->mana = ch->max_mana;
	    }

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	    else
		ch->move = ch->max_move;
	}

	if ( !IS_NPC(ch) && ch->spirit)
	{
	    OBJ_DATA *obj;
	    bool found = FALSE;

	    for ( obj = object_list; obj != NULL; obj = obj->next )
	    {
		if (obj->item_type != ITEM_CORPSE_PC)
		    continue;

		if (str_cmp(obj->owner,ch->name))
		    continue;

		found = TRUE;
		break;
	    }

	    if (!found)
	    {
		ch->spirit = 0;
		send_to_char("A searing pain runs through your new body as it forms around you.\n\r",ch);
	    }
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
		    extract_obj( obj );
		}
	 	else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR);
	    }

	    if (IS_IMMORTAL(ch))
		ch->timer = 0;

	    if ( ++ch->timer >= 12 )
	    {
		if ( ch->was_in_room == NULL && ch->in_room != NULL )
		{
		    ch->was_in_room = ch->in_room;
		    if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );
		    act( "$n disappears into the void.",
			ch, NULL, NULL, TO_ROOM );
		    send_to_char( "You disappear into the void.\n\r", ch );
		    if (ch->level > 1)
		        save_char_obj( ch );
		    char_from_room( ch );
		    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		}
	    }

	    if (!ch->spirit)
	    {
	        gain_condition( ch, COND_DRUNK,  -1 );
	        gain_condition( ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2 );
	        gain_condition( ch, COND_THIRST, -1 );
	        gain_condition( ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1);
	    }
	}

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
            }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, ch );
			send_to_char( "\n\r", ch );
		    }
		}
	  
		affect_remove( ch, paf );
	    }
	}

	if (!IS_NPC(ch) && (ch->position == POS_SLEEPING)
	&& (!IS_SET(ch->comm,COMM_QUIET)))
	{
	    int dream;

	    dream = number_range(1, 100);

	    if (dream >= 65 && dream <= 100)
	    {
		send_to_char("zzzzzzzzzzzzzzzzzzzzzzzzz.\n\r",ch);
	    } else if (dream >= 25 && dream <= 50)
	    {
		DRM_INDEX_DATA *pDrmIndex;
		extern int top_drm_index;

		dream = number_range(0, top_drm_index-1);
		if ( (pDrmIndex = get_drm_index( dream ) ) != NULL )
		{
		    DREAM_OBJECT_DATA *pobj;
		    DREAM_ACT_DATA *pact;
		    DREAM_CHANGE_DATA *pchg;

		    send_to_char(pDrmIndex->description, ch);
		    for ( pact = pDrmIndex->act; pact != NULL; pact = pact->next)
		    {
			act(pact->string,ch,NULL,NULL,TO_ROOM);
		    }
		    for ( pchg = pDrmIndex->change; pchg != NULL; pchg = pchg->next )
		    {
			if (!str_cmp(pchg->keyword, "hit"))
			{
			    if (!str_cmp(pchg->value, "all"))
				ch->hit = UMAX(ch->hit, ch->max_hit);
			    else if (!str_cmp(pchg->value, "none"))
				ch->hit = UMIN(ch->hit, 1);
			    else if (!str_cmp(pchg->value, "half"))
				ch->hit = UMAX(1, ch->hit/2);
			    else if (!str_cmp(pchg->value, "double"))
				ch->hit = UMIN(ch->max_hit+(ch->max_hit/5), ch->hit*2);
			    else if (!str_cmp(pchg->value, "sub"))
				ch->hit = UMAX(1, ch->hit-(ch->hit/10));
			    else if (!str_cmp(pchg->value, "add"))
				ch->hit = UMIN(ch->hit+(ch->hit/10), ch->max_hit+(ch->max_hit/5));
			    else
			    {
				sprintf( log_buf, "BUG: bad dream change value: %s", pchg->value);
				log_string( log_buf );
			    }
			}
			else if (!str_cmp(pchg->keyword, "mana"))
			{
                            if (!str_cmp(pchg->value, "all"))
                                ch->mana = UMAX(ch->mana, ch->max_mana);
                            else if (!str_cmp(pchg->value, "none"))
                                ch->mana = UMIN(ch->mana, 1);
                            else if (!str_cmp(pchg->value, "half"))
                                ch->mana = UMAX(1, ch->mana/2);
                            else if (!str_cmp(pchg->value, "double"))
                                ch->mana = UMIN(ch->max_mana+(ch->max_mana/5), ch->mana*2);
                            else if (!str_cmp(pchg->value, "sub"))
                                ch->mana = UMAX(1, ch->mana-(ch->mana/10));
                            else if (!str_cmp(pchg->value, "add"))
                                ch->mana = UMIN(ch->mana+(ch->mana/10), ch->max_mana+(ch->max_mana/5));
                            else
                            {
                                sprintf( log_buf, "BUG: bad dream change value: %s", pchg->value);
                                log_string( log_buf );
                            }
                        }
                        else if (!str_cmp(pchg->keyword, "move"))
                        {
                            if (!str_cmp(pchg->value, "all"))
                                ch->move = UMAX(ch->move, ch->max_move);
                            else if (!str_cmp(pchg->value, "none"))
                                ch->move = UMIN(ch->move, 1);
                            else if (!str_cmp(pchg->value, "half"))
                                ch->move = UMAX(1, ch->move/2);
                            else if (!str_cmp(pchg->value, "double"))
                                ch->move = UMIN(ch->max_move+(ch->max_move/5), ch->move*2);
                            else if (!str_cmp(pchg->value, "sub"))
                                ch->move = UMAX(1, ch->move-(ch->move/10));
                            else if (!str_cmp(pchg->value, "add"))
                                ch->move = UMIN(ch->move+(ch->move/10), ch->max_move+(ch->max_move/5));
                            else
                            {
                                sprintf( log_buf, "BUG: bad dream change value: %s", pchg->value);
                                log_string( log_buf );
                            }
                        }
			else
                        {
                            sprintf( log_buf, "BUG: bad dream change keyword: %s", pchg->keyword);
                            log_string( log_buf );
                        }
		    }
                    if (pDrmIndex->move)
		    {
			ROOM_INDEX_DATA *location;

			if ( ( location = get_room_index( pDrmIndex->move ) ) != NULL )
			{
			    act("$n stands up and wanders off in $s sleep.",ch,NULL,NULL,TO_ROOM);
			    char_from_room( ch );
			    char_to_room( ch, location );
			}
		    }
		    for ( pobj = pDrmIndex->object; pobj != NULL; pobj = pobj->next )
		    {
			OBJ_INDEX_DATA *pObjIndex;
			OBJ_DATA *obj;
			int level;

			if ( ( pObjIndex = get_obj_index( pobj->vnum ) ) == NULL )
			{
			    sprintf( log_buf, "BUG: bad dream object vnum %d", pobj->vnum);
			    log_string( log_buf );
			    continue;
			}
			obj = create_object( pObjIndex, 0 );
			if (pobj->variable)
			{
			    level = ch->level;
			    obj->level = level;
			    if (pObjIndex->item_type == ITEM_WEAPON)
			    {
				float adlev, inclev;
				int cntr;

				adlev = 8;
				inclev = .01;
				for (cntr = 0; cntr <= level; cntr++)
				{
				    adlev += .57;
				    adlev += inclev;
				    inclev += .005;
				}
				cntr = (int)adlev;
				obj->value[1] = cntr/3;
				obj->value[2] = 3;
			    }
			    if (pObjIndex->item_type == ITEM_ARMOR)
			    {
				obj->value[0] = (level/6)+5;
				obj->value[1] = (level/6)+5;
				obj->value[2] = (level/6)+5;
				obj->value[3] = level/6;
			    }
			}
			if (pobj->t_min || pobj->t_max)
			{
			    if (pobj->t_max >= pobj->t_min)
			    {
				obj->timer = number_range(pobj->t_min, pobj->t_max);
			    }
			}
			if ( !str_cmp(pobj->roomchar, "char") )
			{
			    obj_to_char( obj, ch );
			} else if ( !str_cmp(pobj->roomchar, "room") )
			{
			    obj_to_room( obj, ch->in_room );
			} else
			{
			    sprintf( log_buf, "BUG: bad dream roomchar value %s", pobj->roomchar);
			    log_string( log_buf );
			}
		    }
		}
	    }
	}


	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */

        if (is_affected(ch, gsn_plague) && ch != NULL)
        {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int dam;

	    if (ch->in_room == NULL)
		continue;
            
	    act("$n writhes in agony as plague sores erupt from $s skin.",
		ch,NULL,NULL,TO_ROOM);
	    send_to_char("You writhe in agony from the plague.\n\r",ch);
            for ( af = ch->affected; af != NULL; af = af->next )
            {
            	if (af->type == gsn_plague)
                    break;
            }
        
            if (af == NULL)
            {
            	REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            	continue;
            }
        
            if (af->level == 1)
            	continue;
        
	    plague.where		= TO_AFFECTS;
            plague.type 		= gsn_plague;
            plague.level 		= af->level - 1; 
            plague.duration 	= number_range(1,2 * plague.level);
            plague.location		= APPLY_STR;
            plague.modifier 	= -5;
            plague.bitvector 	= AFF_PLAGUE;
        
            for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
            {
                if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
		&&  !IS_IMMORTAL(vch)
            	&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
            	{
            	    send_to_char("You feel hot and feverish.\n\r",vch);
            	    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	    affect_join(vch,&plague);
            	}
            }

	    dam = UMIN(ch->level,af->level/5+1);
	    ch->mana -= dam;
	    ch->move -= dam;
	    damage( ch, ch, dam, gsn_plague,DAM_DISEASE,FALSE);
        }
	else if ( IS_AFFECTED(ch, AFF_POISON) 
					&& ch != NULL 
					&& !IS_AFFECTED(ch,AFF_SLOW))
	{
	    AFFECT_DATA *poison;
	    poison = affect_find(ch->affected,gsn_poison);
	    if (poison != NULL)
	    {
	        act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
	        send_to_char( "You shiver and suffer.\n\r", ch );
	        damage(ch,ch,poison->level/10 + 1,gsn_poison,
		    DAM_POISON,FALSE);
	    }
	}

	else if ( ch->position == POS_INCAP && number_range(0,1) == 0)
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
	else if ( ch->position == POS_MORTAL )
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
	    save_char_obj(ch);

        if ( ch == ch_quit )
            do_quit( ch, "" );
    }

    return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	/* go through affects and decrement */
        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next    = paf->next;
            if ( paf->duration > 0 )
            {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)
                  paf->level--;  /* spell strength fades with time */
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( paf_next == NULL
                ||   paf_next->type != paf->type
                ||   paf_next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_obj )
                    {
			if (obj->carried_by != NULL)
			{
			    rch = obj->carried_by;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_CHAR);
			}
			if (obj->in_room != NULL 
			&& obj->in_room->people != NULL)
			{
			    rch = obj->in_room->people;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_ALL);
			}
                    }
                }

                affect_remove_obj( obj, paf );
            }
        }


	if ( obj->timer <= 0 || --obj->timer > 0 )
	    continue;

	switch ( obj->item_type )
	{
	default:              message = "$p crumbles into dust.";  break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
	case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
	case ITEM_CORPSE_PC:  message = "$p decays into dust."; break;
	case ITEM_FOOD:       message = "$p decomposes.";	break;
	case ITEM_POTION:     message = "$p has evaporated from disuse.";	
								break;
	case ITEM_PORTAL:     message = "$p fades out of existence."; break;
	case ITEM_CONTAINER: 
	case ITEM_PIT: 
	    if (CAN_WEAR(obj,ITEM_WEAR_FLOAT))
		if (obj->contains)
		    message = 
		"$p flickers and vanishes, spilling its contents on the floor.";
		else
		    message = "$p flickers and vanishes.";
	    else
		message = "$p crumbles into dust.";
	    break;
	}

	if ( obj->carried_by != NULL )
	{
	    if (IS_NPC(obj->carried_by) 
	    &&  obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->silver += obj->cost/5;
	    else
	    {
	    	act( message, obj->carried_by, obj, NULL, TO_CHAR );
		if ( obj->wear_loc == WEAR_FLOAT)
		    act(message,obj->carried_by,obj,NULL,TO_ROOM);
	    }
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    if (! (obj->in_obj && obj->in_obj->pIndexData->item_type == ITEM_PIT
	           && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
	    	act( message, rch, obj, NULL, TO_ROOM );
	    	act( message, rch, obj, NULL, TO_CHAR );
	    }
	}

        if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT)
	&&  obj->contains)
	{   /* save the contents */
     	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		else if (obj->carried_by)  /* carried */
		    if (obj->wear_loc == WEAR_FLOAT)
			if (obj->carried_by->in_room == NULL)
			    extract_obj(t_obj);
			else
			    obj_to_room(t_obj,obj->carried_by->in_room);
		    else
		    	obj_to_char(t_obj,obj->carried_by);

		else if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj);

		else /* to a room */
		    obj_to_room(t_obj,obj->in_room);
	    }
	}

	extract_obj( obj );
    }

    return;
}



void quest_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;
                                                                                                                                               
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->character != NULL && d->connected == CON_PLAYING)
        {
           ch = d->character;
        if (ch->nextquest > 0) 
	{ 
		ch->nextquest--;
       	if (ch->nextquest == 0) 
		{ 
		send_to_char("You may now quest again.\n\r",ch); 
		return; 
		} 
	}
    else if (IS_SET(ch->exbit1_flags, PLR_QUESTOR))
    {
        if (--ch->qcountdown <= 0)
        {
            char buf [MSL];
                                                                                                                                               
            ch->nextquest = 10;
            sprintf(buf, "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r",ch->nextquest);
            send_to_char(buf, ch);
            REMOVE_BIT(ch->exbit1_flags, PLR_QUESTOR);
            ch->questgiver = NULL;
            ch->qcountdown = 0;
            ch->questmob = 0;
        }

                                                                                                                                               
    if (ch->qcountdown > 0 && ch->qcountdown < 6)
    {
        send_to_char("Better hurry, you're almost out of time for your quest!\n\r",ch);
        return;
    }
    }
                                                                                                                                               
    }
    }
    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( IS_NPC(wch)
	||   wch->level >= LEVEL_IMMORTAL
	||   wch->in_room == NULL 
	||   wch->in_room->area->empty)
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next	= ch->next_in_room;

	    if ( !IS_NPC(ch)
	    ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
	    ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	    ||   IS_AFFECTED(ch,AFF_CALM)
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) 
	    ||   number_bits(1) == 0)
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC(vch)
		&&   vch->level < LEVEL_IMMORTAL
		&&   ch->level >= vch->level - 5 
		&&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   can_see( ch, vch ) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
		continue;

	    multi_hit( ch, victim, TYPE_UNDEFINED );
	}
    }

    return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( bool forced )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int	    pulse_music;

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= number_range(PULSE_AREA, 3 * PULSE_AREA / 2);
	/* pulse_area	= PULSE_AREA; */
	/* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
	area_update	( );
	quest_update	( ); 
    }


    if ( --pulse_music	  <= 0 )
    {
	pulse_music	= PULSE_MUSIC;
	song_update();
	who_html_update();
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update	( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if (forced)
    {
	pulse_point = 0;
    }
    forced = FALSE;
    	if (xpq_timer < 0) { xpq_timer++; }
	if (xpq_timer > 0) { xpq_timer = xpq_timer-1; }
	if (xpq_timer == 0) { global_xpq = FALSE; }
    	if (prquest_timer < 0) { prquest_timer++; }
	if (prquest_timer > 0) { prquest_timer = prquest_timer-1; }
	if (prquest_timer == 0) { global_prq = FALSE; }
    	if (damq_timer < 0) { damq_timer++; }
	if (damq_timer > 0) { damq_timer = damq_timer-1; }
	if (damq_timer == 0) { global_damq = FALSE; }
    if ( --pulse_point    <= 0 )
    {
	wiznet("TICK!",NULL,NULL,WIZ_TICKS,0,0);
	pulse_point	= number_range(PULSE_TICK / 3 * 2, 3 * PULSE_TICK / 2);
	/* pulse_point     = PULSE_TICK; */
	/* number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 ); */
	weather_update	( );
	obj_update	( );
	char_update	( );
	randomize_entrances ( ROOM_VNUM_CHAIN );
	randomize_entrances ( ROOM_VNUM_LADDER );
    }

    auction_update();
    aggr_update( );
    tail_chain( );
    return;
}
