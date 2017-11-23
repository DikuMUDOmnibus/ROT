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
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"

float cpo_stat, vpo_stat;	//actual pay-off ratio for arena

const int vam_str[] = {
    6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5,
    6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5
};

/*
 * Local functions.
 */
void    check_assist    args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool    check_dodge     args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool    check_parry     args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool    check_shield_block     args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void    dam_message   args ( ( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, bool immune ) );
void    death_cry     args ( ( CHAR_DATA * ch, CHAR_DATA * killer ) );
void    group_gain    args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
int     xp_compute    args ( ( CHAR_DATA * gch, CHAR_DATA * victim, int total_levels ) );
bool    can_bypass    args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
int     dambonus      args ( ( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int stance ) );
void    special_move  args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool    can_counter   args ( ( CHAR_DATA * ch ) );
bool    is_safe       args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool    is_safe_mock  args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool    is_voodood    args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void    make_corpse   args ( ( CHAR_DATA * ch, CHAR_DATA * killer ) );
void    one_hit       args ( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary ) );
void    one_hit_mock  args ( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary ) );
void    mob_hit       args ( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt ) );
void    raw_kill      args ( ( CHAR_DATA * victim, CHAR_DATA * killer ) );
void    set_fighting  args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void    disarm        args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool    check_fade    args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

void check_arena args((CHAR_DATA * ch, CHAR_DATA * victim));

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update(void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for (ch = char_list; ch != NULL; ch = ch->next) {
	ch_next = ch->next;

	if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
	    continue;

	if (IS_AWAKE(ch) && ch->in_room == victim->in_room)
	    multi_hit(ch, victim, TYPE_UNDEFINED);
	else
	    stop_fighting(ch, FALSE);

	if ((victim = ch->fighting) == NULL)
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch, victim);

 	if ( IS_NPC( ch ) )
 	{
 	    if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
 		mp_percent_trigger( ch, victim, NULL, NULL, TRIG_FIGHT );
 	    if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
 		mp_hprct_trigger( ch, victim );
 	}
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA * ch, CHAR_DATA * victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next) {
	rch_next = rch->next_in_room;

	if (IS_AWAKE(rch) && rch->fighting == NULL) {

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch)
		&& IS_SET(rch->off_flags, ASSIST_PLAYERS)
		&& rch->level + 6 > victim->level) {
		do_emote(rch, "{Rscreams and attacks!{x");
		multi_hit(rch, victim, TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)) {
		if (!IS_NPC(rch) && !IS_NPC(victim)
		    && (!is_pkill(rch) || !is_pkill(victim))
		    && !IS_SET(victim->act, PLR_TWIT))
		    continue;

		if (((!IS_NPC(rch) && IS_SET(rch->act, PLR_AUTOASSIST))
		     || IS_AFFECTED(rch, AFF_CHARM))
		    && is_same_group(ch, rch)
		    && !is_safe(rch, victim))
		    multi_hit(rch, victim, TYPE_UNDEFINED);

		continue;
	    }

	    /* now check the NPC cases */

	    if (IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM))
	     {
		if ((IS_NPC(rch) && IS_SET(rch->off_flags, ASSIST_ALL))

		    || (IS_NPC(rch) && rch->group
			&& rch->group == ch->group)

		    || (IS_NPC(rch) && rch->race == ch->race
			&& IS_SET(rch->off_flags, ASSIST_RACE))

		    || (IS_NPC(rch) && IS_SET(rch->off_flags, ASSIST_ALIGN)
			&& ((IS_GOOD(rch) && IS_GOOD(ch))
			    || (IS_EVIL(rch) && IS_EVIL(ch))
			    || (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))

		    || (rch->pIndexData == ch->pIndexData
			&& IS_SET(rch->off_flags, ASSIST_VNUM)))
		 {
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;

		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next) {
			if (can_see(rch, vch)
			    && is_same_group(vch, victim)
			    && number_range(0, number) == 0) {
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL) {
			do_emote(rch, "{Rscreams and attacks!{x");
			multi_hit(rch, target, TYPE_UNDEFINED);
		    }
		}
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
    int chance;
    int count = 1;		// DB
    /* decrement the wait */
    if (ch->desc == NULL)
	ch->wait = UMAX(0, ch->wait - PULSE_VIOLENCE);

    if (ch->desc == NULL)
	ch->daze = UMAX(0, ch->daze - PULSE_VIOLENCE);


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    if (ch->stunned) {
	ch->stunned--;
	if (!ch->stunned) {
	    send_to_char("You regain your equilibrium.\n\r", ch);
	    act("$n regains $s equilibrium.", ch, NULL, NULL, TO_ROOM);
	}
	return;
    }

    if (IS_NPC(ch)) {
	mob_hit(ch, victim, dt);
	return;
    }

    one_hit(ch, victim, dt, FALSE);

    if (get_eq_char(ch, WEAR_SECONDARY)) {
	chance = (get_skill(ch, gsn_dual_wield) / 3) * 2;
	chance += 33;
	if (number_percent() < chance) {
	    one_hit(ch, victim, dt, TRUE);
	    if (get_skill(ch, gsn_dual_wield) != 0 && (!IS_NPC(ch)
						       && ch->level >=
						       skill_table
						       [gsn_dual_wield].
						       skill_level[ch->
								   class]))
	    {
		check_improve(ch, gsn_dual_wield, TRUE, 1);
	    } else if (!IS_NPC(ch) && (ch->pcdata->tier == 2)) {
		if (get_skill(ch, gsn_dual_wield) != 0 && (!IS_NPC(ch)
							   && ch->level >=
							   skill_table
							   [gsn_dual_wield].
							   skill_level[ch->
								       clasb]))
		{
		    check_improve(ch, gsn_dual_wield, TRUE, 1);
		}
	    }
	}
	if (ch->fighting != victim)
	    return;
    }

    if (ch->fighting != victim)
	return;

    if (IS_AFFECTED(ch, AFF_HASTE))
	one_hit(ch, victim, dt, FALSE);

    if (IS_STANCE(ch, STANCE_SERPENT) && number_percent() >
	ch->stance[STANCE_SERPENT] * 0.5)
	count += 1;
    else if (IS_STANCE(ch, STANCE_MANTIS) && number_percent() >
	     ch->stance[STANCE_MANTIS] * 0.5)
	count += 1;
    else if (IS_STANCE(ch, STANCE_TIGER) && number_percent() >
	     ch->stance[STANCE_TIGER] * 0.5)
	count += 3;
    if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle)
	return;

    chance = get_skill(ch, gsn_second_attack) / 2;

    if (IS_AFFECTED(ch, AFF_SLOW))
	chance /= 2;

    if (number_percent() < chance) {
	one_hit(ch, victim, dt, FALSE);
	check_improve(ch, gsn_second_attack, TRUE, 5);
	if (ch->fighting != victim)
	    return;
    } else {
	return;
    }

    chance = get_skill(ch, gsn_third_attack) / 2;

    if (IS_AFFECTED(ch, AFF_SLOW))
	chance /= 2;

    if (number_percent() < chance) {
	one_hit(ch, victim, dt, FALSE);
	check_improve(ch, gsn_third_attack, TRUE, 6);
	if (ch->fighting != victim)
	    return;
    } else {
	return;
    }

    chance = get_skill(ch, gsn_fourth_attack) / 2;

    if (IS_AFFECTED(ch, AFF_SLOW))
	chance /= 3;

    if (number_percent() < chance) {
	one_hit(ch, victim, dt, FALSE);
	check_improve(ch, gsn_fourth_attack, TRUE, 6);
	if (ch->fighting != victim)
	    return;
    } else {
	return;
    }

    chance = get_skill(ch, gsn_fifth_attack) / 2;

    if (IS_AFFECTED(ch, AFF_SLOW))
	chance = 0;

    if (ch->stance[0] > 0 && number_percent() == 1) {
	int stance = ch->stance[0];
	if (ch->stance[stance] >= 200) {
	    special_move(ch, victim);
	    return;
	}
    }
    if (number_percent() < chance) {
	one_hit(ch, victim, dt, FALSE);
	check_improve(ch, gsn_fifth_attack, TRUE, 6);
	if (ch->fighting != victim)
	    return;
    }

    return;
}

/* procedure for all mobile attacks */
void mob_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
    int chance, number;
    CHAR_DATA *vch, *vch_next;

    one_hit(ch, victim, dt, FALSE);

    if (ch->fighting != victim)
	return;

    if (ch->stunned)
	return;

    /* Area attack -- BALLS nasty! */

    if (IS_SET(ch->off_flags, OFF_AREA_ATTACK)) {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch, vch, dt, FALSE);
	}
    }

    if (ch->fighting != victim)
	return;

    if (get_eq_char(ch, WEAR_SECONDARY)) {
	chance = (get_skill(ch, gsn_dual_wield) / 3) * 2;
	chance += 33;
	if (number_percent() < chance) {
	    one_hit(ch, victim, dt, TRUE);
	}
	if (ch->fighting != victim)
	    return;
    }

    if (IS_AFFECTED(ch, AFF_HASTE)
	|| (IS_SET(ch->off_flags, OFF_FAST) && !IS_AFFECTED(ch, AFF_SLOW)))
	one_hit(ch, victim, dt, FALSE);

    if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle)
	return;

    chance = get_skill(ch, gsn_second_attack) / 2;

    if (IS_AFFECTED(ch, AFF_SLOW) && !IS_SET(ch->off_flags, OFF_FAST))
	chance /= 2;

    if (number_percent() < chance) {
	one_hit(ch, victim, dt, FALSE);
	if (ch->fighting != victim)
	    return;
	chance = get_skill(ch, gsn_third_attack) / 2;

	if (IS_AFFECTED(ch, AFF_SLOW) && !IS_SET(ch->off_flags, OFF_FAST))
	    chance /= 2;

	if (number_percent() < chance) {
	    one_hit(ch, victim, dt, FALSE);
	    if (ch->fighting != victim)
		return;

	    chance = get_skill(ch, gsn_fourth_attack) / 2;

	    if (IS_AFFECTED(ch, AFF_SLOW)
		&& !IS_SET(ch->off_flags, OFF_FAST)) chance /= 3;

	    if (number_percent() < chance) {
		one_hit(ch, victim, dt, FALSE);
		if (ch->fighting != victim)
		    return;

		chance = get_skill(ch, gsn_fifth_attack) / 2;

		if (IS_AFFECTED(ch, AFF_SLOW)
		    && !IS_SET(ch->off_flags, OFF_FAST)) chance = 0;

		if (number_percent() < chance) {
		    one_hit(ch, victim, dt, FALSE);
		    if (ch->fighting != victim)
			return;
		}
	    }
	}
    }

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0, 2);

    if (number == 1 && IS_SET(ch->act, ACT_MAGE)) {
	/*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET(ch->act, ACT_CLERIC)) {
	/* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range(0, 8);

    switch (number) {
    case (0):
	if (IS_SET(ch->off_flags, OFF_BASH))
	    do_bash(ch, "");
	break;

    case (1):
	if (IS_SET(ch->off_flags, OFF_BERSERK)
	    && !IS_AFFECTED(ch, AFF_BERSERK)) do_berserk(ch, "");
	break;


    case (2):
	if (IS_SET(ch->off_flags, OFF_DISARM)
	    || (get_weapon_sn(ch) != gsn_hand_to_hand
		&& (IS_SET(ch->act, ACT_WARRIOR)
		    || IS_SET(ch->act, ACT_VAMPIRE)
		    || IS_SET(ch->act, ACT_THIEF)))) do_disarm(ch, "");
	break;

    case (3):
	if (IS_SET(ch->off_flags, OFF_KICK))
	    do_kick(ch, "");
	break;

    case (4):
	if (IS_SET(ch->off_flags, OFF_KICK_DIRT))
	    do_dirt(ch, "");
	break;

    case (5):
	if (IS_SET(ch->off_flags, OFF_TAIL)) {
	    do_tail(ch,"");
	}
	break;

    case (6):
	if (IS_SET(ch->off_flags, OFF_TRIP))
	    do_trip(ch, "");
	break;

    case (7):
	if (IS_SET(ch->off_flags, OFF_CRUSH)) {
	    do_crush(ch,"");
	}
	break;
    case (8):
	if (IS_SET(ch->off_flags, OFF_BACKSTAB)) {
	    do_backstab(ch, "");
	}
    }
}


/*
 * Hit one guy once.
 */
void one_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary)
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int stance;
    int diceroll;
    int sn, skill;
    int dam_type;
    bool result;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if (victim->position == POS_DEAD || ch->in_room != victim->in_room
	|| victim->spirit) return;

    /*
     * Figure out the type of damage message.
     * if secondary == true, use the second weapon.
     */
    if (!secondary)
	wield = get_eq_char(ch, WEAR_WIELD);
    else
	wield = get_eq_char(ch, WEAR_SECONDARY);
    if (dt == TYPE_UNDEFINED) {
	dt = TYPE_HIT;
	if (wield != NULL && wield->item_type == ITEM_WEAPON)
	    dt += wield->value[3];
	else
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
	if (wield != NULL)
	    dam_type = attack_table[wield->value[3]].damage;
	else
	    dam_type = attack_table[ch->dam_type].damage;
    else
	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch, sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if (IS_NPC(ch)) {
	thac0_00 = 20;
	thac0_32 = -4;		/* as good as a thief */
	if (IS_SET(ch->act, ACT_VAMPIRE))
	    thac0_32 = -30;
	else if (IS_SET(ch->act, ACT_DRUID))
	    thac0_32 = 0;
	else if (IS_SET(ch->act, ACT_RANGER))
	    thac0_32 = -4;
	else if (IS_SET(ch->act, ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act, ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act, ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act, ACT_MAGE))
	    thac0_32 = 6;
    } else {
	thac0_00 = class_table[ch->class].thac0_00;
	thac0_32 = class_table[ch->class].thac0_32;
	if (ch->pcdata->tier == 2) {
	    thac0_00 = UMIN(class_table[ch->class].thac0_00,
			    class_table[ch->clasb].thac0_00);
	    thac0_32 = UMIN(class_table[ch->class].thac0_32,
			    class_table[ch->clasb].thac0_32);
	}
    }
    thac0 = interpolate(ch->level, thac0_00, thac0_32);

    if (thac0 < 0)
	thac0 = thac0 / 2;

    if (thac0 < -5)
	thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill / 100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch, gsn_backstab));

    switch (dam_type) {
    case (DAM_PIERCE):
	victim_ac = GET_AC(victim, AC_PIERCE) / 10;
	break;
    case (DAM_BASH):
	victim_ac = GET_AC(victim, AC_BASH) / 10;
	break;
    case (DAM_SLASH):
	victim_ac = GET_AC(victim, AC_SLASH) / 10;
	break;
    default:
	victim_ac = GET_AC(victim, AC_EXOTIC) / 10;
	break;
    };

    if (((!strcmp(class_table[victim->class].name, "vampire"))
	 || (!strcmp(class_table[victim->class].name, "lich")))
	&& (IS_OUTSIDE(victim))
	&& (vam_str[time_info.hour] != 0)) {
	if ((time_info.hour > 6) && (time_info.hour < 18)) {
	    victim_ac += (victim_ac * (vam_str[time_info.hour] / 100));
	} else {
	    victim_ac -= (victim_ac * (vam_str[time_info.hour] / 100));
	}
    } else if (!IS_NPC(victim) && (victim->pcdata->tier == 2)) {
	if ((!strcmp(class_table[victim->clasb].name, "vampire"))
	    && (IS_OUTSIDE(victim))
	    && (vam_str[time_info.hour] != 0)) {
	    if ((time_info.hour > 6) && (time_info.hour < 18)) {
		victim_ac += (victim_ac * (vam_str[time_info.hour] / 100));
	    } else {
		victim_ac -= (victim_ac * (vam_str[time_info.hour] / 100));
	    }
	}

    }

    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;

    if (!can_see(ch, victim))
	victim_ac -= 4;

    if (victim->position < POS_FIGHTING)
	victim_ac += 4;

    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ((diceroll = number_bits(5)) >= 20);

    if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac)) {
	/* Miss. */
	damage(ch, victim, 0, dt, dam_type, TRUE);
	tail_chain();
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if (IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format) {
	    dam = number_range(ch->level / 2, ch->level * 3 / 2);
	    if (wield != NULL)
		dam += dam / 2;
		if (global_damq){ dam = dam*2; }
	} else
	    dam = dice(ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

    else {
	if (sn != -1)
	    check_improve(ch, sn, TRUE, 5);
	if (wield != NULL) {
	    if (wield->clan) {
		float adlev, inclev;
		int cntr;

		adlev = 8;
		inclev = .01;
		for (cntr = 0; cntr <= ch->level; cntr++) {
		    adlev += .57;
		    adlev += inclev;
		    inclev += .005;
		}
		cntr = (int) adlev;
		dam = dice(cntr / 3, 3) * skill / 100;
		if (global_damq){ dam = dam*2; }
	    } else {
		if (wield->pIndexData->new_format)
		    dam =
			dice(wield->value[1],
			     wield->value[2]) * skill / 100;
		else
		    dam =
			number_range(wield->value[1] * skill / 100,
				     wield->value[2] * skill / 100);
		if (global_damq){ dam = dam*2; }
	    }

	    if (get_eq_char(ch, WEAR_SHIELD) == NULL)	/* no shield = more */
		dam = dam * 11 / 10;
		if (global_damq){ dam = dam*2; }

	    /* sharpness! */
	    if (IS_WEAPON_STAT(wield, WEAPON_SHARP)) {
		int percent;

		if ((percent = number_percent()) <= (skill / 8))
		    dam = 2 * dam + (dam * 2 * percent / 100);
		if (global_damq){ dam = dam*2; }
	    }
	} else
	    dam =
		number_range(1 + 4 * skill / 100,
			     2 * ch->level / 3 * skill / 100);
		if (global_damq){ dam = dam*2; }
    }

    /*
     * Bonuses.
     */

    if (((!strcmp(class_table[ch->class].name, "vampire"))
	 || (!strcmp(class_table[ch->class].name, "lich")))
	&& (IS_OUTSIDE(ch))
	&& (vam_str[time_info.hour] != 0)) {
	if ((time_info.hour > 6) && (time_info.hour < 18)) {
	    dam -= (dam * (vam_str[time_info.hour] / 100));
		if (global_damq){ dam = dam*2; }
	} else {
	    dam += (dam * (vam_str[time_info.hour] / 100));
		if (global_damq){ dam = dam*2; }
	}
    } else if (!IS_NPC(ch) && (ch->pcdata->tier == 2)) {
	if ((!strcmp(class_table[ch->clasb].name, "vampire"))
	    && (IS_OUTSIDE(ch))
	    && (vam_str[time_info.hour] != 0)) {
	    if ((time_info.hour > 6) && (time_info.hour < 18)) {
		dam -= (dam * (vam_str[time_info.hour] / 100));
	    } else {
		dam += (dam * (vam_str[time_info.hour] / 100));
	    }
	}
		if (global_damq){ dam = dam*2; }
    }

    if (get_skill(ch, gsn_enhanced_damage) > 0) {
	diceroll = number_percent();
	if (diceroll <= get_skill(ch, gsn_enhanced_damage)) {
	    check_improve(ch, gsn_enhanced_damage, TRUE, 6);
	    if (str_cmp(class_table[ch->class].name, "gladiator"))
		dam += 2 * (dam * diceroll / 300);
	    else
		dam += 2 * (dam * diceroll / 200);
	}
		if (global_damq){ dam = dam*2; }
    }

    if (!IS_AWAKE(victim))
	dam *= 2;
    else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if (dt == gsn_backstab && wield != NULL) {
	if (wield->value[0] != 2)
	    dam *= 2 + (ch->level / 10);
	else
	    dam *= 2 + (ch->level / 8);
    }
    if (dt == gsn_circle && wield != NULL) {
	if (wield->value[0] != 2)
	    dam *= 1.5 + (ch->level / 15);
	else
	    dam *= 1.5 + (ch->level / 12);
    }
    dam += GET_DAMROLL(ch) * UMIN(100, skill) / 100;
		if (global_damq){ dam = dam*2; }

    if (dam <= 0)
	dam = 1;

    result = damage(ch, victim, dam, dt, dam_type, TRUE);

    /* but do we have a funky weapon? */
    if (result && wield != NULL) {
	int dam;

	if (ch->fighting == victim && IS_WEAPON_STAT(wield, WEAPON_POISON)) {
	    int level;
	    AFFECT_DATA *poison, af;

	    if ((poison = affect_find(wield->affected, gsn_poison)) ==
		NULL) level = wield->level;
	    else
		level = poison->level;

	    if (!saves_spell(level / 2, victim, DAM_POISON)) {
		send_to_char
		    ("{cYou feel {ypoison{c coursing through your veins.\n{x",
		     victim);
		act("$n is {ypoisoned{x by the venom on $p.", victim,
		    wield, NULL, TO_ROOM);

		af.where = TO_AFFECTS;
		af.type = gsn_poison;
		af.level = level * 3 / 4;
		af.duration = level / 2;
		af.location = APPLY_STR;
		af.modifier = -1;
		af.bitvector = AFF_POISON;
		affect_join(victim, &af);
	    }

	    /* weaken the poison if it's temporary */
	    if (poison != NULL) {
		poison->level = UMAX(0, poison->level - 2);
		poison->duration = UMAX(0, poison->duration - 1);

		if (poison->level == 0 || poison->duration == 0)
		    act("The {ypoison{x on $p has worn off.", ch, wield,
			NULL, TO_CHAR);
	    }
	}


	if (ch->fighting == victim
	    && IS_WEAPON_STAT(wield, WEAPON_VAMPIRIC)) {
	    dam = number_range(1, wield->level / 5 + 1);
	    act("$p draws life from $n.{x", victim, wield, NULL,
		TO_ROOM);
	    act("You feel $p drawing your life away.{x", victim, wield,
		NULL, TO_CHAR);
	    damage(ch, victim, dam, 0, DAM_NEGATIVE, FALSE);
	    ch->alignment = UMAX(-1000, ch->alignment - 1);
	    if (ch->pet != NULL)
		ch->pet->alignment = ch->alignment;
	    ch->hit += dam / 2;
	}

	if (ch->fighting == victim
	    && IS_WEAPON_STAT(wield, WEAPON_FLAMING)) {
	    dam = number_range(1, wield->level / 4 + 1);
	    act("$n is {rburned by $p.{x", victim, wield, NULL,
		TO_ROOM);
	    act("$p {rsears your flesh.{x", victim, wield, NULL,
		TO_CHAR);
	    fire_effect((void *) victim, wield->level / 2, dam,
			TARGET_CHAR);
	    damage(ch, victim, dam, 0, DAM_FIRE, FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield, WEAPON_FROST)) {
	    dam = number_range(1, wield->level / 6 + 2);
	    act("$p {cfreezes $n.{x", victim, wield, NULL, TO_ROOM);
	    act("The {Ccold touch of $p surrounds you with {Cice.{x",
		victim, wield, NULL, TO_CHAR);
	    cold_effect(victim, wield->level / 2, dam, TARGET_CHAR);
	    damage(ch, victim, dam, 0, DAM_COLD, FALSE);
	}

	if (ch->fighting == victim
	    && IS_WEAPON_STAT(wield, WEAPON_SHOCKING)) {
	    dam = number_range(1, wield->level / 5 + 2);
	    act("$n is struck by {Ylightning from $p.{x", victim,
		wield, NULL, TO_ROOM);
	    act("You are {Yshocked by $p.{x", victim, wield, NULL,
		TO_CHAR);
	    shock_effect(victim, wield->level / 2, dam, TARGET_CHAR);
	    damage(ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
	}
		if (global_damq){ dam = dam*2; }
    }
    if (ch->fighting == victim);
    {
	if (result) {
	    if (IS_SHIELDED(victim, SHD_ICE)) {
		if (!IS_SHIELDED(ch, SHD_ICE)) {
		    dt = skill_lookup("iceshield");
		    dam = number_range(5, 15);
		    damage(victim, ch, dam, dt, DAM_COLD, TRUE);
		}
	    }
	    if (IS_SHIELDED(victim, SHD_FIRE)) {
		if (!IS_SHIELDED(ch, SHD_FIRE)) {
		    dt = skill_lookup("fireshield");
		    dam = number_range(10, 20);
		    damage(victim, ch, dam, dt, DAM_FIRE, TRUE);
		}
	    }

	    if (!IS_NPC(ch)) {
		stance = ch->stance[0];
		if (IS_STANCE(ch, STANCE_NORMAL))
		    dam *= 1.75;
		else
		    dam = dambonus(ch, victim, dam, stance);
		dambonus(ch, victim, dam, stance);
		improve_stance(ch);
	    }

	    if (IS_NPC(ch)) {
		stance = ch->stance[0];
		if (IS_STANCE(ch, STANCE_NORMAL))
		    dam *= 1.95;
		else
		    dam = dambonus(ch, victim, dam, stance);
		dambonus(ch, victim, dam, stance);
		improve_stance(ch);
		improve_stance(ch);
	    }
	    if (IS_SHIELDED(victim, SHD_SHOCK)) {
		if (!IS_SHIELDED(ch, SHD_SHOCK)) {
		    dt = skill_lookup("shockshield");
		    dam = number_range(15, 25);
		    damage(victim, ch, dam, dt, DAM_LIGHTNING, TRUE);
		}
	    }
	}
		if (global_damq){ dam = dam*2; }
    }

    tail_chain();
    return;
}

/*
 * Mock hit one guy once.
 */
void one_hit_mock(CHAR_DATA * ch, CHAR_DATA * victim, int dt,
		  bool secondary)
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn, skill;
    int dam_type;
    bool result;

    sn = -1;


    /* just in case */
    if (ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
	return;

    /*
     * Figure out the type of damage message.
     * if secondary == true, use the second weapon.
     */
    if (!secondary)
	wield = get_eq_char(ch, WEAR_WIELD);
    else
	wield = get_eq_char(ch, WEAR_SECONDARY);
    if (dt == TYPE_UNDEFINED) {
	dt = TYPE_HIT;
	if (wield != NULL && wield->item_type == ITEM_WEAPON)
	    dt += wield->value[3];
	else
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
	if (wield != NULL)
	    dam_type = attack_table[wield->value[3]].damage;
	else
	    dam_type = attack_table[ch->dam_type].damage;
    else
	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch, sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if (IS_NPC(ch)) {
	thac0_00 = 20;
	thac0_32 = -4;		/* as good as a thief */
	if (IS_SET(ch->act, ACT_VAMPIRE))
	    thac0_32 = -30;
	else if (IS_SET(ch->act, ACT_DRUID))
	    thac0_32 = 0;
	else if (IS_SET(ch->act, ACT_RANGER))
	    thac0_32 = -4;
	else if (IS_SET(ch->act, ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act, ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act, ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act, ACT_MAGE))
	    thac0_32 = 6;
    } else {
	thac0_00 = class_table[ch->class].thac0_00;
	thac0_32 = class_table[ch->class].thac0_32;
	if (ch->pcdata->tier == 2) {
	    thac0_00 = UMIN(class_table[ch->class].thac0_00,
			    class_table[ch->clasb].thac0_00);
	    thac0_32 = UMIN(class_table[ch->class].thac0_32,
			    class_table[ch->clasb].thac0_32);
	}
    }
    thac0 = interpolate(ch->level, thac0_00, thac0_32);

    if (thac0 < 0)
	thac0 = thac0 / 2;

    if (thac0 < -5)
	thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill / 100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch, gsn_backstab));

    switch (dam_type) {
    case (DAM_PIERCE):
	victim_ac = GET_AC(victim, AC_PIERCE) / 10;
	break;
    case (DAM_BASH):
	victim_ac = GET_AC(victim, AC_BASH) / 10;
	break;
    case (DAM_SLASH):
	victim_ac = GET_AC(victim, AC_SLASH) / 10;
	break;
    default:
	victim_ac = GET_AC(victim, AC_EXOTIC) / 10;
	break;
    };

    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;

    if (!can_see(ch, victim))
	victim_ac -= 4;

    if (victim->position < POS_FIGHTING)
	victim_ac += 4;

    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ((diceroll = number_bits(5)) >= 20);

    if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac)) {
	/* Miss. */
	damage_mock(ch, victim, 0, dt, dam_type, TRUE);
	tail_chain();
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if (IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format) {
	    dam = number_range(ch->level / 2, ch->level * 3 / 2);
	    if (wield != NULL)
		dam += dam / 2;
	} else
	    dam = dice(ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

    else {
	if (sn != -1)
	    check_improve(ch, sn, TRUE, 5);
	if (wield != NULL) {
	    if (wield->clan) {
		float adlev, inclev;
		int cntr;

		adlev = 8;
		inclev = .01;
		for (cntr = 0; cntr <= ch->level; cntr++) {
		    adlev += .57;
		    adlev += inclev;
		    inclev += .005;
		}
		cntr = (int) adlev;
		dam = dice(cntr / 3, 3) * skill / 100;
	    } else {
		if (wield->pIndexData->new_format)
		    dam =
			dice(wield->value[1],
			     wield->value[2]) * skill / 100;
		else
		    dam =
			number_range(wield->value[1] * skill / 100,
				     wield->value[2] * skill / 100);
	    }

	    if (get_eq_char(ch, WEAR_SHIELD) == NULL)	/* no shield = more */
		dam = dam * 11 / 10;

	    /* sharpness! */
	    if (IS_WEAPON_STAT(wield, WEAPON_SHARP)) {
		int percent;

		if ((percent = number_percent()) <= (skill / 8))
		    dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	} else
	    dam =
		number_range(1 + 4 * skill / 100,
			     2 * ch->level / 3 * skill / 100);
    }

    /*
     * Bonuses.
     */
    if (get_skill(ch, gsn_enhanced_damage) > 0) {
	diceroll = number_percent();
	if (diceroll <= get_skill(ch, gsn_enhanced_damage)) {
	    check_improve(ch, gsn_enhanced_damage, TRUE, 6);
	    if (str_cmp(class_table[ch->class].name, "gladiator"))
		dam += 2 * (dam * diceroll / 300);
	    else
		dam += 2 * (dam * diceroll / 200);
	}
    }

    if (!IS_AWAKE(victim))
	dam *= 2;
    else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if (dt == gsn_backstab && wield != NULL) {
	if (wield->value[0] != 2)
	    dam *= 2 + (ch->level / 10);
	else
	    dam *= 2 + (ch->level / 8);
    }
    if (dt == gsn_circle && wield != NULL) {
	if (wield->value[0] != 2)
	    dam *= 1.5 + (ch->level / 15);
	else
	    dam *= 1.5 + (ch->level / 12);
    }
    dam += GET_DAMROLL(ch) * UMIN(100, skill) / 100;

    if (dam <= 0)
	dam = 1;

    result = damage_mock(ch, victim, dam, dt, dam_type, TRUE);

    tail_chain();
    return;
}

/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
	    int dam_type, bool show)
{
    OBJ_DATA *corpse;
    bool immune;
    int stance;

    if (victim->position == POS_DEAD)
	return FALSE;

    if (ch->spirit || victim->spirit)
	return FALSE;

    /*
     * Stop up any residual loopholes.
     */

    if (IS_NPC(ch)) {
	stance = ch->stance[0];
	if (IS_STANCE(ch, STANCE_NORMAL))
	    dam *= 1.75;
	else
	    dam = dambonus(ch, victim, dam, stance);
	dambonus(ch, victim, dam, stance);
	improve_stance(ch);
    }
    if (!IS_NPC(ch)) {
	stance = ch->stance[0];
	if (IS_STANCE(ch, STANCE_NORMAL))
	    dam *= 1.95;
	else
	    dam = dambonus(ch, victim, dam, stance);
	dambonus(ch, victim, dam, stance);
	improve_stance(ch);
    }
/*
    if (dam > 25000 && dt >= TYPE_HIT && !IS_IMMORTAL(ch)) {
	bug("Damage: %d: more than 25000 points!", dam);
	dam = 5200;
	if (!IS_IMMORTAL(ch)) {
	    OBJ_DATA *obj;
	    obj = get_eq_char(ch, WEAR_WIELD);
	send_to_char("Damage more than 25000 points!", ch);
	// send_to_char("{cYou {z{Breally{x{c shouldn't cheat.{x\n\r", ch);
	    if (obj != NULL)
		extract_obj(obj);
	    obj = get_eq_char(ch, WEAR_SECONDARY);
	    if (obj != NULL)
		extract_obj(obj);
	}

    }
 */

    /* damage reduction */
    if (dam > 35)
	dam = (dam - 35) / 2 + 35;
    if (dam > 80)
	dam = (dam - 80) / 2 + 80;

    if (victim != ch) {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if (is_safe(ch, victim))
	    return FALSE;

	if (ch->spirit || victim->spirit)
	    return FALSE;
	if (victim->position > POS_STUNNED) {
  	    if ( victim->fighting == NULL )
 	    {
  		set_fighting( victim, ch );
 		if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
 		    mp_percent_trigger( victim, ch, NULL, NULL, TRIG_KILL );
	    }
	    if (victim->timer <= 4)
		victim->position = POS_FIGHTING;
	}

	if (victim->position > POS_STUNNED) {
	    if (ch->fighting == NULL)
		set_fighting(ch, victim);
	}

	/*
	 * More charm stuff.
	 */
	if (victim->master == ch)
	    stop_follower(victim);
    }

    /*
     * Inviso attacks ... not.
     */
    if (IS_SHIELDED(ch, SHD_INVISIBLE)) {
	affect_strip(ch, gsn_invis);
	affect_strip(ch, gsn_mass_invis);
	REMOVE_BIT(ch->shielded_by, SHD_INVISIBLE);
	act("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }

    /*
     * Damage modifiers.
     */

    if (dam > 1 && !IS_NPC(victim)
	&& victim->pcdata->condition[COND_DRUNK] > 10)
	dam = 9 * dam / 10;

    if (dam > 1 && IS_SHIELDED(victim, SHD_SANCTUARY))
	dam /= 2;

    if (dam > 1 && ((IS_SHIELDED(victim, SHD_PROTECT_EVIL) && IS_EVIL(ch))
		    || (IS_SHIELDED(victim, SHD_PROTECT_GOOD)
			&& IS_GOOD(ch)))) dam -= dam / 4;

    immune = FALSE;


    /*
     * Check for parry, and dodge.
     */
    if (dt >= TYPE_HIT && ch != victim) {
	if (check_parry(ch, victim))
	    return FALSE;
	if (check_dodge(ch, victim))
	    return FALSE;
	if (check_shield_block(ch, victim))
	    return FALSE;
	if ( check_fade( ch, victim ))
	    return FALSE;

	if (IS_STANCE(victim, STANCE_CRANE) &&
	    victim->stance[STANCE_CRANE] > 100 && !can_counter(ch)
	    && !can_bypass(ch, victim))
	    return FALSE;
	if (IS_STANCE(victim, STANCE_MANTIS) &&
	    victim->stance[STANCE_MANTIS] > 100 && !can_counter(ch)
	    && !can_bypass(ch, victim))
	    return FALSE;
	if (IS_STANCE(victim, STANCE_MONGOOSE) &&
	    victim->stance[STANCE_MONGOOSE] > 100 && !can_counter(ch)
	    && !can_bypass(ch, victim))
	    return FALSE;
	if (IS_STANCE(victim, STANCE_SWALLOW) &&
	    victim->stance[STANCE_SWALLOW] > 100 && !can_counter(ch)
	    && !can_bypass(ch, victim))
	    return FALSE;


    }

    switch (check_immune(victim, dam_type)) {
    case (IS_IMMUNE):
	immune = TRUE;
	dam = 0;
	break;
    case (IS_RESISTANT):
	dam -= dam / 3;
	break;
    case (IS_VULNERABLE):
	dam += dam / 2;
	break;
    }

    if (ch->spirit || victim->spirit)
	return FALSE;

    if (show)
	dam_message(ch, victim, dam, dt, immune);

    if (dam == 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if (!IS_NPC(victim)
	&& victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
	victim->hit = 1;
    update_pos(victim);
    if (dt == gsn_feed && !victim->spirit) {
	ch->hit = UMIN(ch->hit + ((dam / 4) * 3), ch->max_hit);
	update_pos(ch);
    }

    switch (victim->position) {
    case POS_MORTAL:
	act("{c$n is mortally wounded, and will die soon, if not aided.{x",
	    victim, NULL, NULL, TO_ROOM);
	send_to_char
	    ("{cYou are mortally wounded, and will die soon, if not aided.{x\n\r",
	     victim);
	break;

    case POS_INCAP:
	act("{c$n is incapacitated and will slowly die, if not aided.{x",
	    victim, NULL, NULL, TO_ROOM);
	send_to_char
	    ("{cYou are incapacitated and will slowly {z{Rdie{x{c, if not aided.{x\n\r",
	     victim);
	break;

    case POS_STUNNED:
	act("{c$n is stunned, but will probably recover.{x",
	    victim, NULL, NULL, TO_ROOM);
	send_to_char("{cYou are stunned, but will probably recover.{x\n\r",
		     victim);
	break;

    case POS_DEAD:
	if ((IS_NPC(victim)) && (victim->die_descr[0] != '\0')) {
	    act("{c$n $T{x", victim, 0, victim->die_descr, TO_ROOM);
	} else {
	    act("{c$n is {CDEAD!!{x", victim, 0, 0, TO_ROOM);
	}
	send_to_char("{cYou have been {RKILLED!!{x\n\r\n\r", victim);

	break;

    default:
	if (dam > victim->max_hit / 4)
	    send_to_char("{cThat really did {RHURT!{x\n\r", victim);
	if (victim->hit < victim->max_hit / 4)
	    send_to_char("{cYou sure are {z{RBLEEDING!{x\n\r", victim);
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if (!IS_AWAKE(victim))
	stop_fighting(victim, FALSE);

    /*
     * Payoff for killing things.
     */
    if (victim->position == POS_DEAD) {
	group_gain(ch, victim);

	if (!IS_NPC(victim)) {
	    sprintf(log_buf, "%s killed by %s at %d",
		    victim->name,
		    (IS_NPC(ch) ? ch->short_descr : ch->name),
		    ch->in_room->vnum);
	    log_string(log_buf);

	    /*
	     * Dying penalty:
	     * 5/6 way back to previous level.
	     */
	    if (victim->exp > exp_per_level(victim, victim->pcdata->points)
		* victim->level)
		if (!IS_SET(victim->act, PLR_LQUEST))
		    gain_exp(victim, (5 *
				      (exp_per_level
				       (victim,
					victim->pcdata->points) *
				       victim->level - victim->exp) / 6) +
			     50);
	}

	sprintf(log_buf, "%s got toasted by %s at %s [room %d]",
		(IS_NPC(victim) ? victim->short_descr : victim->name),
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		ch->in_room->name, ch->in_room->vnum);

	if (IS_NPC(victim))
	    wiznet(log_buf, NULL, NULL, WIZ_MOBDEATHS, 0, 0);
	else
	    wiznet(log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);

         /*
          * Death trigger
          */
         if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
         {
             victim->position = POS_STANDING;
             mp_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH );
         }
 
	raw_kill(victim, ch);
	/* dump the flags */
	if (IS_SET(victim->exbit1_flags, RECRUIT)) {
	    REMOVE_BIT(victim->exbit1_flags, RECRUIT);
	}
	if (IS_SET(victim->exbit1_flags, PK_KILLER)) {
	    REMOVE_BIT(victim->exbit1_flags, PK_KILLER);
	    SET_BIT(ch->exbit1_flags, PK_LAWFUL);
	}
	if (ch != victim && !IS_NPC(ch) && (!is_same_clan(ch, victim)
					    || clan_table[victim->clan].
					    independent)) {
	    if (IS_SET(victim->act, PLR_TWIT))
		REMOVE_BIT(victim->act, PLR_TWIT);
	}

	if (!IS_NPC(ch) && !IS_NPC(victim)
	    && IS_SET(ch->in_room->room_flags, ROOM_ARENA)
	    && IS_SET(victim->in_room->room_flags, ROOM_ARENA)) {
	    check_arena(ch, victim);
	    return TRUE;
	}

	/* RT new auto commands */

	if (!IS_NPC(ch) && IS_NPC(victim)) {
	    OBJ_DATA *coins;

	    corpse = get_obj_list(ch, "corpse", ch->in_room->contents);

	    if (IS_SET(ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)	/* exists and not empty */
		do_get(ch, "all corpse");

	    if (IS_SET(ch->act, PLR_AUTOGOLD) && corpse && corpse->contains &&	/* exists and not empty */
		!IS_SET(ch->act, PLR_AUTOLOOT))
		if ((coins = get_obj_list(ch, "gcash", corpse->contains))
		    != NULL)
		    do_get(ch, "all.gcash corpse");

	    if (IS_SET(ch->act, PLR_AUTOSAC)) {
		if (IS_SET(ch->act, PLR_AUTOLOOT) && corpse
		    && corpse->contains) return TRUE;	/* leave if corpse has treasure */
		else
		    do_sacrifice(ch, "corpse");
	    }
	}

	return TRUE;
    }

    if (victim == ch)
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if (!IS_NPC(victim) && victim->desc == NULL) {
	if (number_range(0, victim->wait) == 0) {
	    do_recall(victim, "");
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if (IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2) {
	if ((IS_SET(victim->act, ACT_WIMPY) && number_bits(2) == 0
	     && victim->hit < victim->max_hit / 5)
	    || (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
		&& victim->master->in_room != victim->in_room))
	    do_flee(victim, "");
    }

    if (!IS_NPC(victim)
	&& victim->hit > 0
	&& victim->hit <= victim->wimpy
	&& victim->wait < PULSE_VIOLENCE / 2) do_flee(victim, "");

    tail_chain();
    return TRUE;
}

/*
 * Show damage from a mock hit.
 */
bool damage_mock(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
		 int dam_type, bool show)
{
    long immdam;
    bool immune;
    char buf1[256], buf2[256], buf3[256];
    const char *attack;

    if (victim->position == POS_DEAD)
	return FALSE;

    if (dam > 35)
	dam = (dam - 35) / 2 + 35;
    if (dam > 80)
	dam = (dam - 80) / 2 + 80;
    if (is_safe_mock(ch, victim))
	return FALSE;
    /*
     * Damage modifiers.
     */

    if (dam > 1 && !IS_NPC(victim)
	&& victim->pcdata->condition[COND_DRUNK] > 10)
	dam = 9 * dam / 10;

    if (dam > 1 && IS_SHIELDED(victim, SHD_SANCTUARY))
	dam /= 2;

    if (dam > 1 && ((IS_SHIELDED(victim, SHD_PROTECT_EVIL) && IS_EVIL(ch))
		    || (IS_SHIELDED(victim, SHD_PROTECT_GOOD)
			&& IS_GOOD(ch)))) dam -= dam / 4;

    immune = FALSE;


    switch (check_immune(victim, dam_type)) {
    case (IS_IMMUNE):
	immune = TRUE;
	dam = 0;
	break;
    case (IS_RESISTANT):
	dam -= dam / 3;
	break;
    case (IS_VULNERABLE):
	dam += dam / 2;
	break;
    }

    if (dt >= 0 && dt < MAX_SKILL)
	attack = skill_table[dt].noun_damage;
    else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
	attack = attack_table[dt - TYPE_HIT].noun;
    else {
	bug("Dam_message: bad dt %d.", dt);
	dt = TYPE_HIT;
	attack = attack_table[0].name;
    }
    immdam = 0;
    if (ch->level == MAX_LEVEL) {
	immdam = dam * 63;
    }
    if (ch == victim) {
	sprintf(buf1,
		"{y$n's {gmock {B%s{g would have done {R%d hp{g damage to {y$mself{g.{x",
		attack, dam);
	sprintf(buf2,
		"{yYour {gmock {B%s{g would have done {R%d hp{g damage to {yyourself{g.{x",
		attack, dam);
	act(buf1, ch, NULL, NULL, TO_ROOM);
	act(buf2, ch, NULL, NULL, TO_CHAR);
    } else if (ch->level < MAX_LEVEL) {
	sprintf(buf1,
		"{y$n's {gmock {B%s{g would have done {R%d hp{g damage to {y$N{g.{x",
		attack, dam);
	sprintf(buf2,
		"{yYour {gmock {B%s{g would have done {R%d hp{g damage to {y$N{g.{x",
		attack, dam);
	sprintf(buf3,
		"{y$n's {gmock {B%s{g would have done {R%d hp{g damage to {yyou{g.{x",
		attack, dam);
	act(buf1, ch, NULL, victim, TO_NOTVICT);
	act(buf2, ch, NULL, victim, TO_CHAR);
	act(buf3, ch, NULL, victim, TO_VICT);
    } else {
	sprintf(buf1,
		"{y$n's {gmock {B%s{g would have done {R%lu hp{g damage to {y$N{g.{x",
		attack, immdam);
	sprintf(buf2,
		"{yYour {gmock {B%s{g would have done {R%lu hp{g damage to {y$N{g.{x",
		attack, immdam);
	sprintf(buf3,
		"{y$n's {gmock {B%s{g would have done {R%lu hp{g damage to {yyou{g.{x",
		attack, immdam);
	act(buf1, ch, NULL, victim, TO_NOTVICT);
	act(buf2, ch, NULL, victim, TO_CHAR);
	act(buf3, ch, NULL, victim, TO_VICT);
    }

    tail_chain();
    return TRUE;
}

bool is_safe(CHAR_DATA * ch, CHAR_DATA * victim)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
	return FALSE;

    if (ch->spirit) {
	send_to_char("You need a body for that!\n\r", ch);
	return TRUE;
    }
    if (victim->spirit) {
	send_to_char("Spirits are difficult to harm.\n\r", ch);
	return TRUE;
    }
    /* killing mobiles */
    if (IS_NPC(victim)) {

	/* safe room? */
	if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
	    send_to_char("Not in this room.\n\r", ch);
	    return TRUE;
	}

	if (IS_SET(victim->exbit1_flags, RECRUIT)
	    || IS_SET(victim->exbit1_flags, PK_VETERAN)
	    || IS_SET(victim->exbit1_flags, PK_LAWFUL)
	    || IS_SET(victim->exbit1_flags, PK_KILLER)
	    || IS_SET(victim->exbit1_flags, PK_KILLER2)) {
	    return FALSE;
	}

	if (IS_SET(victim->in_room->room_flags, ROOM_ARENA))
	    return FALSE;
	if (IS_SET(victim->in_room->room_flags, ROOM_ARENA))
	    return FALSE;
	if (!IS_NPC(ch)
	    && IS_SET(victim->in_room->room_flags, ROOM_CLAN_ENT)) {
	    send_to_char("Not in this room.\n\r", ch);
	    return TRUE;
	}

	if (victim->pIndexData->pShop != NULL) {
	    send_to_char("The shopkeeper wouldn't like that.\n\r", ch);
	    return TRUE;
	}

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act, ACT_TRAIN)
	    || IS_SET(victim->act, ACT_PRACTICE)
	    || IS_SET(victim->act, ACT_IS_HEALER)
	    || IS_SET(victim->act, ACT_IS_BANKER)
	    || IS_SET(victim->act, ACT_IS_SATAN)
	    || IS_SET(victim->act, ACT_IS_PRIEST)) {
	    act("I don't think $g would approve.", ch, NULL, NULL,
		TO_CHAR);
	    return TRUE;
	}

	if (!IS_NPC(ch)) {
	    /* no pets */
	    if (IS_SET(victim->act, ACT_PET)) {
		act("But $N looks so cute and cuddly...",
		    ch, NULL, victim, TO_CHAR);
		return TRUE;
	    }

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim, AFF_CHARM) && ch != victim->master) {
		send_to_char("You don't own that monster.\n\r", ch);
		return TRUE;
	    }
	}
    }
    /* killing players */
    else {
	/* NPC doing the killing */
	if (IS_NPC(ch)) {
	    /* safe room check */
	    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
		send_to_char("Not in this room.\n\r", ch);
		return TRUE;
	    }

	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL
		&& ch->master->fighting != victim) {
		send_to_char("Players are your friends!\n\r", ch);
		return TRUE;
	    }
	}
	/* player doing the killing */
	else {
	    if (IS_SET(victim->in_room->room_flags, ROOM_ARENA))
		return FALSE;

	    if (IS_SET(victim->act, PLR_TWIT))
		return FALSE;

	    if (((victim->level > 19)
		 || ((victim->class >= MCLT_1)
		     && (victim->level > 14)))
		&& (is_voodood(ch, victim)))
		return FALSE;

	    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
		send_to_char("Not in this room.\n\r", ch);
		return TRUE;
	    }
	    if (ch->on_gquest) {
		send_to_char("Not while you are on a quest.\n\r", ch);
		return TRUE;
	    }
	    if (victim->on_gquest) {
		send_to_char("They are on a quest, leave them alone.\n\r",
			     ch);
		return TRUE;
	    }
	    if (!is_clan(ch)) {
		send_to_char
		    ("Join a clan if you want to fight players.\n\r", ch);
		return TRUE;
	    }

	    if (!is_pkill(ch)) {
		send_to_char
		    ("Your clan does not allow player fighting.\n\r", ch);
		return TRUE;
	    }

	    if (!is_clan(victim)) {
		send_to_char
		    ("They aren't in a clan, leave them alone.\n\r", ch);
		return TRUE;
	    }

	    if (!is_pkill(victim)) {
		send_to_char
		    ("They are in a no pkill clan, leave them alone.\n\r",
		     ch);
		return TRUE;
	    }

	    if (is_same_clan(ch, victim)) {
		send_to_char("You can't fight your own clan members.\n\r",
			     ch);
		return TRUE;
	    }

	    if (((ch->pcdata->tier == 2)
		 && (victim->pcdata->tier == 2))
		|| ((ch->pcdata->tier == 1)
		    && (victim->pcdata->tier == 1))
		|| ((ch->pcdata->tier == 0)
		    && (victim->pcdata->tier == 0))) {
		if (ch->level > victim->level + 10) {
		    send_to_char("Pick on someone your own size.\n\r", ch);
		    return TRUE;
		}
		if (ch->level < victim->level - 20) {
		    send_to_char
			("If you wish to die, there are less painful methods.\n\r",
			 ch);
		    return TRUE;
		}
	    }

	    if ((ch->pcdata->tier == 2)
		&& (victim->pcdata->tier == 0)) {
		send_to_char("Pick on someone your own size.\n\r", ch);
		return TRUE;
	    }

	    if ((ch->pcdata->tier == 0)
		&& (victim->pcdata->tier == 2)) {
		send_to_char
		    ("If you wish to die, there are less painful methods.\n\r",
		     ch);
		return TRUE;
	    }

	    if (((ch->pcdata->tier == 1)
		 && (victim->pcdata->tier == 0))
		|| ((ch->pcdata->tier == 2)
		    && (victim->pcdata->tier == 1))) {
		if (ch->level < (victim->level - 30)) {
		    send_to_char
			("If you wish to die, there are less painful methods.\n\r",
			 ch);
		    return TRUE;
		}
		if (ch->level > victim->level) {
		    send_to_char("Pick on someone your own size.\n\r", ch);
		    return TRUE;
		}
	    }

	    if (((ch->pcdata->tier == 0)
		 && (victim->pcdata->tier == 1))
		|| ((ch->pcdata->tier == 1)
		    && (victim->pcdata->tier == 2))) {
		if (ch->level < (victim->level - 10)) {
		    send_to_char
			("If you wish to die, there are less painful methods.\n\r",
			 ch);
		    return TRUE;
		}
		if (ch->level > (victim->level + 20)) {
		    send_to_char("Pick on someone your own size.\n\r", ch);
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

bool is_safe_mock(CHAR_DATA * ch, CHAR_DATA * victim)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;
    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
	return FALSE;
    if (ch->spirit) {
	send_to_char("You need a body for that!\n\r", ch);
	return TRUE;
    }
    if (victim->spirit) {
	send_to_char("Spirits are difficult to harm.\n\r", ch);
	return TRUE;
    }
    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
	send_to_char("Not in this room.\n\r", ch);
	return TRUE;
    }
    if (IS_NPC(victim)) {
	send_to_char("{RYou can only use this on a player.{x\n\r", ch);
	return TRUE;
    }
    return FALSE;
}

/*=======================================================================*
 * function: do_challenge                                                *
 * purpose: sends initial arena match query                              *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96                *
 *=======================================================================*/
// DB
void do_challenge(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MSL];
    DESCRIPTOR_DATA *d;


/* == First make all invalid checks == */
    if (IS_NPC(ch))
	return;

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
	&& (!IS_IMMORTAL(ch))) {
	send_to_char
	    ("Just keep your nose in the corner like a good little player.\n\r",
	     ch);
	return;
    }
    if (arena == FIGHT_START) {
	send_to_char
	    ("Sorry, some one else has already started a challenge, please try later.\n\r",
	     ch);
	return;
    }

    if (arena == FIGHT_BUSY) {
	send_to_char
	    ("Sorry, there is a fight in progress, please wait a few moments.\n\r",
	     ch);
	return;
    }

    if (arena == FIGHT_LOCK) {
	send_to_char("Sorry, the arena is currently locked from use.\n\r",
		     ch);
	return;
    }

    if (IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGED)) {
	send_to_char
	    ("You have already been challenged, either ACCEPT or DECLINE first.\n\r",
	     ch);
	return;
    }
    if (ch->hit < ch->max_hit) {
	send_to_char("You must be fully healed to fight in the arena.\n\r",
		     ch);
	return;
    }


    if (IS_AFFECTED(ch, SHD_INVISIBLE) || IS_AFFECTED(ch, AFF_SNEAK)
	|| IS_AFFECTED(ch, AFF_HIDE)) {
	send_to_char
	    ("You can't challenge while invis, sneaking, or hidden.\n\r",
	     ch);
	return;
    }

    if (argument[0] == '\0') {
	send_to_char("You must specify whom you wish to challenge.\n\r",
		     ch);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
	send_to_char("They are not playing.\n\r", ch);
	return;
    }

    if ((victim->in_room->vnum == ROOM_VNUM_CORNER)) {
	send_to_char("They are in the corner, leave them alone.\n\r", ch);
	return;
    }
/*
 if( IS_IMMORTAL(ch) )
  {
  send_to_char("Immortals are not allowed to battle in the arena.\n\r",ch);
  return;
  }
 */
    if (IS_NPC(victim) || victim == ch) {
	send_to_char("You cannot challenge NPC's, or yourself.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_BLIND)) {
	send_to_char("That person is blind right now.\n\r", ch);
	return;
    }

    if (IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGER)) {
	send_to_char("They have already challenged someone else.\n\r", ch);
	return;
    }
    if (victim->fighting != NULL) {
	send_to_char("That person is engaged in battle right now.\n\r",
		     ch);
	return;
    }

    if (victim->hit < victim->max_hit) {
	send_to_char
	    ("That player is not healthy enough to fight right now.\n\r",
	     ch);
	return;
    }


    if (IS_SET(victim->comm, COMM_AFK)) {
	send_to_char
	    ("That player is AFK at the moment, try them later.\n\r", ch);
	return;
    }

    if (victim->desc == NULL) {
	send_to_char
	    ("That player is linkdead at the moment, try them later.\n\r",
	     ch);
	return;
    }

    if (IS_SET(ch->comm, COMM_NOARENA)) {
	send_to_char
	    ("Why should you be allowed to challenge if no one can challenge you?\n\r",
	     ch);
	return;
    }

    if (IS_SET(victim->comm, COMM_NOARENA)) {
	send_to_char("That player is blocking all challenges.\n\r", ch);
	return;
    }

    if (victim->level <= 10) {
	send_to_char("That player is just a newbie!\n\r", ch);
	return;
    }
    if (ch->level > (victim->level + 50)) {
	send_to_char("That player would be no challenge at all!\r", ch);
	return;
    }
    if (ch->level < (victim->level - 50)) {
	act("{bThey laugh at you mercilessly!{x\r.",
	    ch, NULL, victim, TO_VICT);
	return;
    }

/* == Now for the challenge == */
    ch->challenged = victim;
    SET_BIT(ch->exbit1_flags, EXBIT1_CHALLENGER);
    victim->challenger = ch;
    SET_BIT(victim->exbit1_flags, EXBIT1_CHALLENGED);
    arena = FIGHT_START;
    send_to_char("Challenge has been sent\n\r", ch);
    act("{b$n{x has {ychallenged{x you to a {rdeath match{x.",
	ch, NULL, victim, TO_VICT);
    sprintf(buf,
	    "{W^{x{gArena{x{W^{x {R%s{x has {rchallenged{x {B%s{x to a duel.\n\r",
	    ch->name, victim->name);
    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING &&
	    (d->character != victim && d->character != ch)
	    && !IS_SET(d->character->comm, COMM_NOARENA)) {
	    send_to_char(buf, d->character);
	}
    }
    sprintf(buf, "type: {gACCEPT{x {b%s{x to meet the challenge.\n\r",
	    ch->name);
    send_to_char(buf, victim);
    sprintf(buf, "type: {gDECLINE{x {b%s{x to chicken out.\n\r", ch->name);
    send_to_char(buf, victim);
    return;
}

/*=======================================================================*
 * function: do_accept                                                   *
 * purpose: to accept the arena match, and move the players to the arena *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96                *
 *=======================================================================*/
void do_accept(CHAR_DATA * ch, char *argument)
{
    float vafl, vdfl, cafl, cdfl, vpd, vpk, cpd, cpk;
    float clvl, vlvl, cw, vw, clo, vlo;

// the preceding variables before this point can be redefined to use less space.
// all variables following need to be set at what they are.
    float cha_stat, vha_stat;	//base odds from arena wins
    float chl_stat, vhl_stat;	//base odds from level diff
    float chpk_stat, vhpk_stat;	//base odds from pk/pd ratio
    float cht_stat, vht_stat;	//Average odds of winning based
    //      off previous statistics


    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    char buf1[MSL];
    char buf2[MSL];
    char buf3[MSL];
    int char_room;
    int vict_room;
    char_room = number_range(27001, 27032);
    vict_room = number_range(27001, 27032);

    if (IS_NPC(ch))
	return;

    if (!IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGED)) {
	send_to_char("You have not been challenged.\n\r", ch);
	return;
    }

    if (arena == FIGHT_BUSY) {
	send_to_char
	    ("Sorry, there is a fight in progress, please wait a few moments.\n\r",
	     ch);
	return;
    }

    if (arena == FIGHT_LOCK) {
	send_to_char("Sorry, the arena is currently locked from use.\n\r",
		     ch);
	return;
    }

    if (argument[0] == '\0') {
	send_to_char
	    ("You must specify whose challenge you wish to accept.\n\r",
	     ch);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
	send_to_char("They aren't logged in!\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("You haven't challenged yourself!\n\r", ch);
	return;
    }

    if (!IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGER)
	|| victim != ch->challenger) {
	send_to_char("That player hasn't challenged you!\n\r", ch);
	return;
    }

    send_to_char("You have accepted the challenge!\n\r", ch);
    act("$n accepts your challenge!", ch, NULL, victim, TO_VICT);

    cw = ch->pcdata->awins;
    vw = victim->pcdata->awins;
    clo = ch->pcdata->alosses;
    vlo = victim->pcdata->alosses;
    clvl = ch->level;
    vlvl = victim->level;
    vafl = victim->pcdata->aflee;
    vdfl = victim->pcdata->dflee;
    cafl = ch->pcdata->aflee;
    cdfl = ch->pcdata->dflee;
    vpd = victim->pcdata->pdeath;
    vpk = victim->pcdata->pkills;
    cpd = ch->pcdata->pdeath;
    cpk = ch->pcdata->pkills;
    /* flee record is currently not taken into account  */
    /* until a fair adjustment can be decided using it. *
       vafl =  victim->pcdata->aflee; 
       vdfl =  victim->pcdata->dflee;
       cafl =  ch->pcdata->aflee;
       cdfl =  ch->pcdata->dflee;
       * can add later
     */

/* This sets base odds of winning to 1.  Needed to maintain accurate ratio's */
/* and to also keep this function from having 'division by zero' errors      */

    if (cw == 0) {
	cw = 1;
    }
    if (clo == 0) {
	clo = 1;
    }
    if (cpd == 0) {
	cpd = 1;
    }
    if (cpk == 0) {
	cpk = 1;
    }
    if (vw == 0) {
	vw = 1;
    }
    if (vlo == 0) {
	vlo = 1;
    }
    if (vpd == 0) {
	vpd = 1;
    }
    if (vpk == 0) {
	vpk = 1;
    }

    /* Sample odds from a test fight are commented in to the right
       Please take a look at these and notice how much of an extreme
       in odds you get by having each base stat use both peoples stats */

    /* Base odds from previous arena wins/loss ratio  */

    cha_stat = ((cw / clo) / (vw / vlo));
    /*  (1/4) / (100/10) = 0.025 */
    vha_stat = ((vw / vlo) / (cw / clo));
    /*  (100/10) / (1/4) = 40.00 */

    /* Base odds from pk/pdeath ratio */

    chpk_stat = ((cpk / cpd) / (vpk / vpd));
    /*  (1/5) / (2/20)  =  2.0  */
    vhpk_stat = ((vpk / vpd) / (cpk / cpd));
    /*  (2/20) / (1/5)  =  0.5  */

    /* Base odds from level differences               */
    chl_stat = ((clvl / vlvl));	//  20/40 = 0.5
    vhl_stat = ((vlvl / clvl));	//  40/20 = 2.0


    /* Calculate winning odds = AVG of other ratio's  */
    /* Does NOT always equal payoff ratio             */
    cht_stat = ((cha_stat + chpk_stat + chl_stat) / 3);	//  2.0 + 0.5 + .025 = 2.25/3 = .750
    vht_stat = ((vha_stat + vhpk_stat + vhl_stat) / 3);	//  0.5 + 2.0 + 40.0 = 42.5/3 = 14.x

    /* Enforce payoff ratio limits
       Max payoff is 300% profit
       Min payoff is 20%  profit */
    if (cha_stat > 3) {
	cha_stat = 3;
    }
    if (chl_stat > 3) {
	chl_stat = 3;
    }
    if (chpk_stat > 3) {
	chpk_stat = 3;
    }
    if (vha_stat > 3) {
	vha_stat = 3;
    }
    if (vhl_stat > 3) {
	vhl_stat = 3;
    }
    if (vhpk_stat > 3) {
	vhpk_stat = 3;
    }
    if (cha_stat < .2) {
	cha_stat = .2;
    }
    if (chl_stat < .2) {
	chl_stat = .2;
    }
    if (chpk_stat < .2) {
	chpk_stat = .2;
    }
    if (vha_stat < .2) {
	vha_stat = .2;
    }
    if (vhl_stat < .2) {
	vhl_stat = .2;
    }
    if (vhpk_stat < .2) {
	vhpk_stat = .2;
    }
    /*  the aditional +1 is set here to make sure the original bet 
       is also returned with the profits */
    /*bet is not deducted in the first place, so +1 removed */
    cpo_stat = ((cha_stat + chpk_stat + chl_stat) / 3);
    vpo_stat = ((vha_stat + vhpk_stat + vhl_stat) / 3);

    if (cpo_stat > 10) {
	cpo_stat = 10;
    }
    if (vpo_stat > 10) {
	vpo_stat = 10;
    }




    sprintf(buf1,
	    "^{gArena{x^ {b%s{x {m({y%d {gwins{m) ({y%d {rlosses{m){x Payoff odds %f\n\r",
	    victim->name, victim->pcdata->awins, victim->pcdata->alosses,
	    vht_stat);
    sprintf(buf2,
	    "^{gArena{x^ {b%s{x {m({y%d {gwins{m) ({y%d {rlosses{m){x Payoff odds %f\n\r",
	    ch->name, ch->pcdata->awins, ch->pcdata->alosses, cht_stat);
    strcpy(buf3,
	   "{x^{gArena{x^ To wager on the fight, type: {gbet{x ({yamount{x) ({bplayer name{x)\n\r");
    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING &&
	    (d->character != victim && d->character != ch)
	    && !IS_SET(d->character->comm, COMM_NOARENA)) {
	    send_to_char(buf1, d->character);
	    send_to_char(buf2, d->character);
	    send_to_char(buf3, d->character);
	    d->character->gladiator = NULL;
	}
    }
/* == now move them both to an arena for the fun == */
    send_to_char("You make your way into the arena.\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, get_room_index(char_room));
    do_look(ch, "auto");
    SET_BIT(ch->comm, COMM_NOCHANNELS);
    SET_BIT(ch->act, PLR_NORESTORE);
    send_to_char("You make your way to the arena.\n\r", victim);
    char_from_room(victim);
    char_to_room(victim, get_room_index(vict_room));
    do_look(victim, "auto");
    SET_BIT(victim->comm, COMM_NOCHANNELS);
    SET_BIT(victim->act, PLR_NORESTORE);
    arena = FIGHT_BUSY;
    return;
}

/*=======================================================================*

 if( !IS_SET(fighter->in_room->room_flags,ROOM_ARENA) )

 * function: do_decline                                                  *
 * purpose: to chicken out from a sent arena challenge                   *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96                *
 *=======================================================================*/
void do_decline(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MSL];
    DESCRIPTOR_DATA *d;


/*== make all invalid checks == */
    if (IS_NPC(ch))
	return;

    if (!IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGED)) {
	send_to_char
	    ("You have niether given a challenge or been challenged.\n\r",
	     ch);
	return;
    }

    if (argument[0] == '\0') {
	send_to_char
	    ("You must specify whose challenge you wish to decline.\n\r",
	     ch);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
	send_to_char("They aren't logged in!\n\r", ch);
	return;
    }

    if (!IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGER)
	|| victim != ch->challenger) {
	send_to_char("That player hasn't challenged you.\n\r", ch);
	return;
    }


/*== now actually decline == */
    victim->challenged = NULL;
    REMOVE_BIT(victim->exbit1_flags, EXBIT1_CHALLENGER);
    ch->challenger = NULL;
    REMOVE_BIT(ch->exbit1_flags, EXBIT1_CHALLENGED);



    arena = FIGHT_OPEN;
    send_to_char("Challenge declined!\n\r", ch);
    act("$n has withdrawn from the challenge.", ch, NULL, victim, TO_VICT);
    sprintf(buf,
	    "^{gArena{x^ {b%s{x {bhas withdrawn from the challenge{x.\n\r",
	    ch->name);
    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING &&
	    (d->character != victim && d->character != ch)
	    && !IS_SET(d->character->comm, COMM_NOARENA)) {
	    send_to_char(buf, d->character);
	}
    }

    return;
}

void do_dismiss(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MSL];
    DESCRIPTOR_DATA *d;


/*== make all invalid checks == */
    if (IS_NPC(ch))
	return;

    if (!IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGER)) {
	send_to_char
	    ("You have niether given a challenge or been challenged.\n\r",
	     ch);
	return;
    }

    if (argument[0] == '\0') {
	send_to_char
	    ("You must specify whose challenge you wish to decline.\n\r",
	     ch);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
	send_to_char("They aren't logged in!\n\r", ch);
	return;
    }

    if (!IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGER)
	|| victim != ch->challenger) {
	send_to_char("That player hasn't challenged you.\n\r", ch);
	return;
    }


/*== now actually decline == */
    victim->challenged = NULL;
    REMOVE_BIT(victim->exbit1_flags, EXBIT1_CHALLENGER);
    ch->challenger = NULL;
    REMOVE_BIT(ch->exbit1_flags, EXBIT1_CHALLENGED);



    arena = FIGHT_OPEN;
    send_to_char("Challenge declined!\n\r", ch);
    act("$n has withdrawn from the challenge.", ch, NULL, victim, TO_VICT);
    sprintf(buf,
	    "^{gArena{x^ {b%s{x {bhas withdrawn from the challenge{x.\n\r",
	    ch->name);
    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING &&
	    (d->character != victim && d->character != ch)
	    && !IS_SET(d->character->comm, COMM_NOARENA)) {
	    send_to_char(buf, d->character);
	}
    }

    return;
}

void do_arec(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MSL];
    DESCRIPTOR_DATA *d;


/* == First make all invalid checks == */
    if (IS_NPC(ch)) {
	send_to_char("Arec will not recover mobs in the arena!\n\r", ch);
	return;
    }

    if (argument[0] == '\0') {
	send_to_char
	    ("Syntax: Arec <player-name> to recover somebody stuck in the Arena.\n\r",
	     ch);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
	send_to_char("They are not playing.\n\r", ch);
	return;
    }


/* == Removing the flags  == */

    stop_fighting(victim, TRUE);
    char_from_room(victim);
    victim->hit = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos(victim);
    do_look(victim, "auto");
    char_to_room(victim, get_room_index(ROOM_VNUM_AWINNER));

    if (IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGER))
	REMOVE_BIT(ch->exbit1_flags, EXBIT1_CHALLENGER);
    if (IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGER))
	REMOVE_BIT(victim->exbit1_flags, EXBIT1_CHALLENGER);
    if (IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGED))
	REMOVE_BIT(victim->exbit1_flags, EXBIT1_CHALLENGED);
    if (IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGED))
	REMOVE_BIT(ch->exbit1_flags, EXBIT1_CHALLENGED);

    victim->challenger = NULL;
    victim->challenged = NULL;
    REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
    REMOVE_BIT(victim->act, PLR_NORESTORE);
    arena = FIGHT_OPEN;

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING &&
	    (d->character != victim && d->character != ch)) {
	    send_to_char(buf, d->character);
	}
    }
    send_to_char("Player recovered from the arena.\n\r", ch);
    return;
}

/*======================================================================*
 * function: do_bet                                                     *
 * purpose: to allow players to wager on the outcome of arena battles   *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96               *
 *======================================================================*/
void do_bet(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    char buf[MSL];
    CHAR_DATA *fighter;
    int wager;

    argument = one_argument(argument, arg);

    if (argument[0] == '\0' || !is_number(arg)) {
	send_to_char("Syntax: BET [amount] [player]\n\r", ch);
	return;
    }

    if (ch->gladiator != NULL) {
	send_to_char("You have already placed a bet on this fight.\n\r",
		     ch);
	return;
    }

/*== disable the actual fighters from betting ==*/
    if (IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGER)
	|| IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGED)) {
	send_to_char("You can't bet on this battle.\n\r", ch);
	return;
    }

    fighter = get_char_world(ch, argument);

/*== make sure the choice is valid ==*/

    if (fighter == NULL) {
	send_to_char("That player is not logged in.\n\r", ch);
	return;
    }

    if (IS_NPC(ch)) {
	send_to_char("Why bet on a mob? They aren't fighting...\n\r", ch);
	return;
    }

    if (!IS_SET(fighter->in_room->room_flags, ROOM_ARENA)) {
	send_to_char("That player is not in the arena.\n\r", ch);
	return;
    }

/*== do away with the negative number trickery ==*/
    if (!str_prefix("-", arg)) {
	send_to_char("Error: Invalid argument!\n\r", ch);
	return;
    }

    wager = atoi(arg);

    if (wager > 10001 || wager < 1) {
	send_to_char("Wager range is between 1 and 10000\n\r", ch);
	return;
    }

/*== make sure they have the cash ==*/
    if (wager > ch->gold + (ch->platinum * 100)) {
	send_to_char("You don't have that much gold to wager!\n\r", ch);
	return;
    }

/*== now set the info ==*/
    ch->gladiator = fighter;
    ch->pcdata->plr_wager = wager;
    sprintf(buf, "You have placed a {y%d{x gold wager on {b%s{x\n\r",
	    wager, fighter->name);
    send_to_char(buf, ch);
    return;
}

bool is_voodood(CHAR_DATA * ch, CHAR_DATA * victim)
{
    OBJ_DATA *object;
    bool found;

    if (ch->level > HERO)
	return FALSE;

    found = FALSE;
    for (object = victim->carrying; object != NULL;
	 object = object->next_content) {
	if (object->pIndexData->vnum == OBJ_VNUM_VOODOO) {
	    char arg[MIL];

	    one_argument(object->name, arg);
	    if (!str_cmp(arg, ch->name)) {
		return TRUE;
	    }
	}
    }
    return FALSE;
}

bool is_safe_spell(CHAR_DATA * ch, CHAR_DATA * victim, bool area)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
	return FALSE;

    if (ch->spirit)
	return TRUE;

    if (victim->spirit)
	return TRUE;

    /* killing mobiles */
    if (IS_NPC(victim)) {
	/* safe room? */
	if (IS_SET(victim->in_room->room_flags, ROOM_SAFE))
	    return TRUE;

	if (!IS_NPC(ch)
	    && IS_SET(victim->in_room->room_flags,
		      ROOM_CLAN_ENT)) return TRUE;

	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act, ACT_TRAIN)
	    || IS_SET(victim->act, ACT_PRACTICE)
	    || IS_SET(victim->act, ACT_IS_HEALER)
	    || IS_SET(victim->act, ACT_IS_BANKER)
	    || IS_SET(victim->act, ACT_IS_SATAN)
	    || IS_SET(victim->act, ACT_IS_PRIEST))
	    return TRUE;

	if (!IS_NPC(ch)) {
	    /* no pets */
	    if (IS_SET(victim->act, ACT_PET))
		return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim, AFF_CHARM)
		&& (area || ch != victim->master)) return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
	    if (victim->fighting != NULL
		&& !is_same_group(ch, victim->fighting)) return TRUE;
	} else {
	    /* area effect spells do not hit other mobs */
	    if (area && !is_same_group(victim, ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else {
	if (area && IS_IMMORTAL(victim) && victim->level > LEVEL_IMMORTAL)
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch)) {
	    /* charmed mobs and pets cannot attack players while owned */
	    if (((IS_AFFECTED(ch, AFF_CHARM)) & (ch->master != NULL))
		&& (ch->master->fighting != victim))
		return TRUE;

	    /* safe room? */
	    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE))
		return TRUE;

	    /* legal kill? -- mobs only hit players grouped with opponent */
	    if (ch->fighting != NULL
		&& !is_same_group(ch->fighting, victim)) return TRUE;
	}

	/* player doing the killing */
	else {
	    if (IS_SET(victim->in_room->room_flags, ROOM_ARENA))
		return FALSE;

	    if (IS_SET(victim->act, PLR_TWIT))
		return FALSE;

	    if (((victim->level > 19)
		 || ((victim->class >= MCLT_1)
		     && (victim->level > 14)))
		&& (is_voodood(ch, victim)))
		return FALSE;

	    if (!is_clan(ch))
		return TRUE;

	    if (!is_pkill(ch))
		return TRUE;

	    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE))
		return TRUE;

	    if (ch->on_gquest)
		return TRUE;

	    if (victim->on_gquest)
		return TRUE;

	    if (!is_clan(victim))
		return TRUE;

	    if (!is_pkill(victim))
		return TRUE;

	    if (is_same_clan(ch, victim))
		return TRUE;

	    if (((ch->pcdata->tier == 2)
		 && (victim->pcdata->tier == 2))
		|| ((ch->pcdata->tier == 1)
		    && (victim->pcdata->tier == 1))
		|| ((ch->pcdata->tier == 0)
		    && (victim->pcdata->tier == 0))) {
		if (ch->level > victim->level + 10) {
		    return TRUE;
		}
		if (ch->level < victim->level - 20) {
		    return TRUE;
		}
	    }

	    if ((ch->pcdata->tier == 2)
		&& (victim->pcdata->tier == 0)) {
		return TRUE;
	    }

	    if ((ch->pcdata->tier == 0)
		&& (victim->pcdata->tier == 2)) {
		return TRUE;
	    }

	    if (((ch->pcdata->tier == 1)
		 && (victim->pcdata->tier == 0))
		|| ((ch->pcdata->tier == 2)
		    && (victim->pcdata->tier == 1))) {
		if (ch->level < (victim->level - 30)) {
		    return TRUE;
		}
		if (ch->level > victim->level) {
		    return TRUE;
		}
	    }

	    if (((ch->pcdata->tier == 0)
		 && (victim->pcdata->tier == 1))
		|| ((ch->pcdata->tier == 1)
		    && (victim->pcdata->tier == 2))) {
		if (ch->level < (victim->level - 10)) {
		    return TRUE;
		}
		if (ch->level > (victim->level + 20)) {
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

/*
 * Check for parry.
 */
bool check_parry(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;
    int bill;

    if (!IS_AWAKE(victim))
	return FALSE;

    chance = get_skill(victim, gsn_parry) / 2;

    if (get_eq_char(victim, WEAR_WIELD) == NULL) {
	if (IS_NPC(victim))
	    chance /= 2;
	else
	    return FALSE;
    }

    if (victim->stunned)
	return FALSE;

    if (!can_see(ch, victim))
	chance /= 2;

    if (number_percent() >= chance + victim->level - ch->level)
	return FALSE;


    if (IS_STANCE(victim, STANCE_CRANE) &&
	victim->stance[STANCE_CRANE] > 0 && !can_counter(ch) &&
	!can_bypass(ch, victim))
	chance += (victim->stance[STANCE_CRANE] * 0.25);
    else if (IS_STANCE(victim, STANCE_MANTIS) &&
	     victim->stance[STANCE_MANTIS] > 0 && !can_counter(ch) &&
	     !can_bypass(ch, victim))
	chance += (victim->stance[STANCE_MANTIS] * 0.25);
/* fuk */
    bill = number_range(1, 24);
    if (bill == 1) {
	act("With cat like speed you parry $n's attack.", ch, NULL, victim,
	    TO_VICT);
	act("With cat like speed $N parries your attack.", ch, NULL,
	    victim, TO_CHAR);
	act("With cat like speed $n parries $N's attack.", ch, NULL,
	    victim, TO_NOTVICT);
	check_improve(victim, gsn_parry, TRUE, 6);
    } else if (bill == 2) {
	act("You move your weapon up parrying $n's attack.", ch, NULL,
	    victim, TO_VICT);
	act("$N moves $s weapon up parrying your blow.", ch, NULL, victim,
	    TO_CHAR);
	act("$n moves $s weapon up parrying $N's blow.", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(victim, gsn_parry, TRUE, 6);
    } else if (bill == 3) {
	act("You move your weapon to the side parrying $n's blow.", ch,
	    NULL, victim, TO_VICT);
	act("$N moves $s weapon to the side parrying your blow.", ch, NULL,
	    victim, TO_CHAR);
	act("$n moves $s weapon to the side parring $N's blow.", ch, NULL,
	    victim, TO_NOTVICT);
	check_improve(victim, gsn_parry, TRUE, 6);
    } else if (bill == 4) {
	act("You deftly move your weapon between $n's attack.", ch, NULL,
	    victim, TO_VICT);
	act("$N deftly moves $s weapon between your attack", ch, NULL,
	    victim, TO_CHAR);
	act("$n deftly moves $s weapon between $N's attack.", ch, NULL,
	    victim, TO_NOTVICT);

    } else if (bill == 5) {
	act("You spin your weapon in a defensive move parrying $n's blow.",
	    ch, get_eq_char(ch, WEAR_WIELD), victim, TO_VICT);
	act("$N spins $s weapon in a defensive move parrying your blow.",
	    ch, get_eq_char(ch, WEAR_WIELD), victim, TO_CHAR);
	act("$n spins $s weapon in a defensive move parrying $N's blow.",
	    ch, get_eq_char(ch, WEAR_WIELD), victim, TO_NOTVICT);
	check_improve(victim, gsn_parry, TRUE, 6);
    } else if (bill == 6) {
	act("With a bone-jarring skill you parry $n's blow.", ch, NULL,
	    victim, TO_VICT);
	act("With a bone-jarring skill $N parries your blow.", ch, NULL,
	    victim, TO_CHAR);
	act("With a bone-jarring skill $n parries $N's blow.", ch, NULL,
	    victim, TO_NOTVICT);
	check_improve(victim, gsn_parry, TRUE, 6);
    } else if (bill == 7) {
	act("You swiftly parry $n's attack.", ch, NULL, victim, TO_VICT);
	act("$N swiftly parries your attack.", ch, NULL, victim, TO_CHAR);
	act("$n swiftly parries $N's attack.", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(victim, gsn_parry, TRUE, 6);
    } else if (bill == 8) {
	act("Moving like a panther you parry $n's attack.", ch, NULL,
	    victim, TO_VICT);
	act("$N moves like a panther parrying your attack.", ch, NULL,
	    victim, TO_CHAR);
	act("$n moves like a panther parrying $N's attack.", ch, NULL,
	    victim, TO_NOTVICT);
	check_improve(victim, gsn_parry, TRUE, 6);
    }
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE(victim))
	return FALSE;


    chance = get_skill(victim, gsn_shield_block) / 5 + 3;


    if (get_eq_char(victim, WEAR_SHIELD) == NULL)
	return FALSE;

    if (number_percent() >= chance + victim->level - ch->level)
	return FALSE;

    if (victim->stunned)
	return FALSE;

    act("You block $n's attack with your shield.{x", ch, NULL, victim,
	TO_VICT);
    act("{h$N blocks your attack with a shield.{x", ch, NULL, victim,
	TO_CHAR);
    check_improve(victim, gsn_shield_block, TRUE, 6);
    return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;
    int bill;

    if (!IS_AWAKE(victim))
	return FALSE;

    chance = get_skill(victim, gsn_dodge) / 2;

    if (!can_see(victim, ch))
	chance /= 2;

    if (number_percent() >= chance + victim->level - ch->level)
	return FALSE;

    if (victim->stunned)
	return FALSE;


    if (IS_STANCE(victim, STANCE_MONGOOSE) &&
	victim->stance[STANCE_MONGOOSE] > 0 && !can_counter(ch) &&
	!can_bypass(ch, victim))
	(chance += victim->stance[STANCE_MONGOOSE] * 0.25);
    if (IS_STANCE(victim, STANCE_SWALLOW) &&
	victim->stance[STANCE_SWALLOW] > 0 && !can_counter(ch) &&
	!can_bypass(ch, victim))
	(chance += victim->stance[STANCE_SWALLOW] * 0.25);

    bill = number_range(1, 27);
    if (bill == 1) {
	act("You feint to the right dodging $n's blow.", ch, NULL, victim,
	    TO_VICT);
	act("$N feints to the right dodging your blow.", ch, NULL, victim,
	    TO_CHAR);
	act("$n feints to the right dodging $N's blow.", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(victim, gsn_dodge, TRUE, 6);
    } else if (bill == 2) {
	act("You quickly duck dodging $n's attack.", ch, NULL, victim,
	    TO_VICT);
	act("$N ducks quickly dodging your attack.", ch, NULL, victim,
	    TO_CHAR);
	act("$n ducks quickly dodging $N's attack.", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(victim, gsn_dodge, TRUE, 6);
    } else if (bill == 3) {
	act("You roll to the ground dodging $n's blow.", ch, NULL, victim,
	    TO_VICT);
	act("$N rolls to the ground dodging your blow.", ch, NULL, victim,
	    TO_CHAR);
	act("$n rolls to the ground dodging  $N's blow.", ch, NULL, victim,
	    TO_NOTVICT);
	act("$n leaps back up!", ch, NULL, victim, TO_NOTVICT);
	act("You leap back up!", ch, NULL, victim, TO_VICT);
	act("$N leaps back up!", ch, NULL, victim, TO_CHAR);
	check_improve(victim, gsn_dodge, TRUE, 6);
    } else if (bill == 4) {
	act("You dodge $n's useless attack.", ch, NULL, victim, TO_VICT);
	act("$N dodges your useless attack.", ch, NULL, victim, TO_CHAR);
	act("$n dodges $N's useless attack.", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(victim, gsn_dodge, TRUE, 6);
    } else if (bill == 5) {
	act("You bob and weave dodging $n's blow.", ch, NULL, victim,
	    TO_VICT);
	act("$N bobs and weaves dodging your blow.", ch, NULL, victim,
	    TO_CHAR);
	act("$n bobs and weaves dodging $N's blow.", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(victim, gsn_dodge, TRUE, 6);
    } else if (bill == 6) {
	act("You roll with the blow dodging $n's attack.", ch, NULL,
	    victim, TO_VICT);
	act("$N rolls with the blow dodging your attack.", ch, NULL,
	    victim, TO_CHAR);
	act("$n rolls with the blow dodging $N's attack.", ch, NULL,
	    victim, TO_NOTVICT);
    } else if (bill == 7) {
	act("You twist right dodging $n.", ch, NULL, victim, TO_VICT);
	act("$N twists right dodging your blow.", ch, NULL, victim,
	    TO_CHAR);
	act("$n twists right dodging $N's blow.", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(victim, gsn_dodge, TRUE, 6);
    } else if (bill == 8) {
	act("You twist left dodging $n.", ch, NULL, victim, TO_VICT);
	act("$N twists left dodging your blow.", ch, NULL, victim,
	    TO_CHAR);
	act("$n twists left dodging $N's blow.", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(victim, gsn_dodge, TRUE, 6);
    } else if (bill == 9) {
	act("You roll to the ground dodging $n's blow.", ch, NULL, victim,
	    TO_VICT);
	act("$N rolls to the ground dodging your blow.", ch, NULL, victim,
	    TO_CHAR);
	act("$n rolls to the ground dodging  $N's blow.", ch, NULL, victim,
	    TO_NOTVICT);
	act("$n deftly springs to their feet!", ch, NULL, victim,
	    TO_NOTVICT);
	act("deftly... you spring to your feet!", ch, NULL, victim,
	    TO_VICT);
	act("$N deftly springs to their feet!", ch, NULL, victim, TO_CHAR);
	check_improve(victim, gsn_dodge, TRUE, 6);
    }
    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos(CHAR_DATA * victim)
{
    if (victim->hit > 0) {
	if (victim->position <= POS_STUNNED)
	    victim->position = POS_STANDING;
	return;
    }

    if (IS_NPC(victim) && victim->hit < 1) {
	victim->position = POS_DEAD;
	return;
    }

    if (victim->hit <= -11) {

	if (IS_SET(victim->exbit1_flags, RECRUIT)
	    && (victim->pcdata->pdeath >= 5
		|| victim->pcdata->pkills >= 5)) {
	    send_to_char("Your skill at PK has improved.\n\r", victim);
	    REMOVE_BIT(victim->exbit1_flags, RECRUIT);
	    SET_BIT(victim->exbit1_flags, PK_VETERAN);
	}
	victim->position = POS_DEAD;
	victim->spirit = 1;
	return;
    }

    if (victim->hit <= -6)
	victim->position = POS_MORTAL;
    else if (victim->hit <= -3)
	victim->position = POS_INCAP;
    else
	victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting(CHAR_DATA * ch, CHAR_DATA * victim)
{
    if (ch->fighting != NULL) {
	bug("Set_fighting: already fighting", 0);
	return;
    }

    if (IS_AFFECTED(ch, AFF_SLEEP))
	affect_strip(ch, gsn_sleep);

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    ch->fighting = victim;
    ch->position = POS_FIGHTING;
    ch->stunned = 0;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting(CHAR_DATA * ch, bool fBoth)
{
    CHAR_DATA *fch;
    char buf[MSL];

    for (fch = char_list; fch != NULL; fch = fch->next) {
	if (fch == ch || (fBoth && fch->fighting == ch)) {
	    fch->fighting = NULL;
	    fch->position = IS_NPC(fch) ? fch->default_pos : POS_STANDING;
	    fch->stunned = 0;
	    update_pos(fch);
	    if (IS_SET(fch->comm, COMM_STORE))
		if (fch->tells) {
		    sprintf(buf, "You have {R%d{x tells waiting.\n\r",
			    fch->tells);
		    send_to_char(buf, fch);
		    send_to_char("Type 'replay' to see tells.\n\r", fch);
		}

	    if (IS_SET(ch->exbit1_flags, RECRUIT)
		&& (ch->pcdata->pdeath >= 5 || ch->pcdata->pkills >= 5)) {
		send_to_char("Your skill at PK has improved.\n\r", ch);
		REMOVE_BIT(ch->exbit1_flags, RECRUIT);
		SET_BIT(ch->exbit1_flags, PK_VETERAN);
	    }
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse(CHAR_DATA * ch, CHAR_DATA * killer)
{
    char buf[MSL];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    int gold, silver, platinum;
    int wearloc;

    if (IS_NPC(ch)) {
	if (IS_SET(ch->act, ACT_NO_BODY)) {
	    if (IS_SET(ch->act, ACT_NB_DROP)) {
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		    obj_next = obj->next_content;
		    obj_from_char(obj);
		    if (!IS_NPC(killer) && killer->pcdata->is_aquest) {
			obj->got_from = ch->pIndexData->vnum;
		    } else {
			obj->got_from = 0;
		    }
		    if (obj->item_type == ITEM_POTION)
			obj->timer = number_range(500, 1000);
		    if (obj->item_type == ITEM_SCROLL)
			obj->timer = number_range(1000, 2500);
		    if (IS_SET(obj->extra_flags, ITEM_ROT_DEATH)) {
			obj->timer = number_range(5, 10);
			REMOVE_BIT(obj->extra_flags, ITEM_ROT_DEATH);
		    }
		    REMOVE_BIT(obj->extra_flags, ITEM_VIS_DEATH);

		    if (IS_SET(obj->extra_flags, ITEM_INVENTORY))
			extract_obj(obj);
		    act("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
		    obj_to_room(obj, ch->in_room);
		}
	    }
	    return;
	}
	name = ch->short_descr;
	corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer = number_range(3, 6);
	if (ch->silver > 0 || ch->gold > 0 || ch->platinum > 0) {
	    obj_to_obj(create_money(ch->platinum, ch->gold, ch->silver),
		       corpse);
	    ch->platinum = 0;
	    ch->gold = 0;
	    ch->silver = 0;
	}
	corpse->cost = 0;
    } else {
	name = ch->name;
	corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer = number_range(25, 40);
	REMOVE_BIT(ch->act, PLR_CANLOOT);
	if (!is_clan(ch)) {
	    corpse->owner = str_dup(ch->name);
	    corpse->killer = NULL;
	} else {
	    corpse->owner = str_dup(ch->name);
	    corpse->killer = str_dup(killer->name);
	    if (ch->platinum > 1 || ch->gold > 1 || ch->silver > 1) {
		silver = number_range(0, ch->silver / 3);
		gold = number_range(0, ch->gold / 3);
		platinum = number_range(0, ch->platinum / 3);
		obj_to_obj(create_money
			   ((ch->platinum / 2) - platinum,
			    (ch->gold / 2) - gold,
			    (ch->silver / 2) - silver), corpse);
		ch->platinum -= ch->platinum / 2;
		ch->gold -= ch->gold / 2;
		ch->silver -= ch->silver / 2;
		obj_to_room(create_money(platinum, gold, silver),
			    ch->in_room);
		act("Some money spills across the floor.", ch, NULL, NULL,
		    TO_ROOM);
		act("Some of your money spills across the floor.", ch,
		    NULL, NULL, TO_CHAR);
	    }
	}

	corpse->cost = 0;

    }

    corpse->level = ch->level;

    sprintf(buf, corpse->short_descr, name);
    free_string(corpse->short_descr);
    corpse->short_descr = str_dup(buf);

    sprintf(buf, corpse->description, name);
    free_string(corpse->description);
    corpse->description = str_dup(buf);

    if (!IS_NPC(ch) && !IS_NPC(killer)) {
	wearloc = number_range(0, MAX_WEAR - 1);
	if ((obj = get_eq_char(ch, wearloc)) != NULL) {
	    if ((obj->wear_loc != WEAR_FLOAT)
		&& (obj->item_type != ITEM_PASSBOOK)
		&& (!obj->clan)
		&& (!obj->quest)) {
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		act("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
		act("$p falls to the floor.", ch, obj, NULL, TO_CHAR);
	    }
	}
	wearloc = number_range(0, MAX_WEAR - 1);
	if ((obj = get_eq_char(ch, wearloc)) != NULL) {
	    if ((obj->wear_loc != WEAR_FLOAT)
		&& (obj->item_type != ITEM_PASSBOOK)
		&& (!obj->clan)
		&& (!obj->quest)) {
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		act("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
		act("$p falls to the floor.", ch, obj, NULL, TO_CHAR);
	    }
	}
	wearloc = number_range(0, MAX_WEAR - 1);
	if ((obj = get_eq_char(ch, wearloc)) != NULL) {
	    if ((obj->wear_loc != WEAR_FLOAT)
		&& (obj->item_type != ITEM_PASSBOOK)
		&& (!obj->clan)
		&& (!obj->quest)) {
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		act("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
		act("$p falls to the floor.", ch, obj, NULL, TO_CHAR);
	    }
	}
    }

    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
	bool floating = FALSE;

	obj_next = obj->next_content;
	if (IS_OBJ_STAT(obj, ITEM_LQUEST))
	    continue;
	if (obj->pIndexData->vnum == OBJ_VNUM_QPOUCH)
	    continue;
	if (obj->wear_loc == WEAR_FLOAT)
	    floating = TRUE;

	obj_from_char(obj);
	if (IS_NPC(ch) && !IS_NPC(killer) && killer->pcdata->is_aquest) {
	    obj->got_from = ch->pIndexData->vnum;
	} else {
	    obj->got_from = 0;
	}
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500, 1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000, 2500);
	if (IS_SET(obj->extra_flags, ITEM_ROT_DEATH) && !floating) {
	    obj->timer = number_range(5, 10);
	    REMOVE_BIT(obj->extra_flags, ITEM_ROT_DEATH);
	}
	REMOVE_BIT(obj->extra_flags, ITEM_VIS_DEATH);

	if (obj->item_type == ITEM_PASSBOOK) {
	    change_banklist(ch, FALSE, obj->value[0], obj->value[1], 0,
			    obj->name);
	    extract_obj(obj);
	} else if (IS_SET(obj->extra_flags, ITEM_INVENTORY)) {
	    extract_obj(obj);
	} else if (floating) {
	    if (IS_OBJ_STAT(obj, ITEM_ROT_DEATH)) {	/* get rid of it! */
		if (obj->contains != NULL) {
		    OBJ_DATA *in, *in_next;

		    act("$p evaporates,scattering its contents.",
			ch, obj, NULL, TO_ROOM);
		    for (in = obj->contains; in != NULL; in = in_next) {
			in_next = in->next_content;
			obj_from_obj(in);
			obj_to_room(in, ch->in_room);
		    }
		} else
		    act("$p evaporates.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
	    } else {
		act("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
		obj_to_room(obj, ch->in_room);
	    }
	} else
	    obj_to_obj(obj, corpse);
    }

    if (!IS_NPC(ch)) {
	act("$p vanishes in a bright flash.", ch, corpse, NULL, TO_ROOM);
	act("Your corpse vanishes in a bright flash.", ch, corpse, NULL,
	    TO_CHAR);
	
	if (ch->level < 10 && ch->pcdata->tier < 1 )
	    obj_to_room(corpse, get_room_index(ROOM_VNUM_MORGUE_SCHOOL));
	else
	    obj_to_room(corpse,
			get_room_index(home_table[ch->home].morgue));
    } else {
	obj_to_room(corpse, ch->in_room);
    }
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 * PK Token modifications added by  D. A. C.
 */

void death_cry(CHAR_DATA * ch, CHAR_DATA * killer) 
{
    ROOM_INDEX_DATA *was_in_room;
    char buf[MSL];
    OBJ_DATA *token2, *token3, *token4, *token5, *token6;
//    CHAR_DATA *victim;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";
	if (!IS_SET(ch->act, ACT_NO_BODY)) {
	    switch (number_bits(4)) {
	    case 0:
		msg = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case 1:
		msg = "$n splatters blood on your armor.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case 2:
		if (IS_SET(ch->parts, PART_GUTS)) {
		    msg = "$n spills $s guts all over the floor.";
		    vnum = OBJ_VNUM_GUTS;
		}
		break;
	    case 3:
		if (IS_SET(ch->parts, PART_HEAD)) {
		    msg = "$n's severed head plops on the ground.";
		    vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	    case 4:
		if (IS_SET(ch->parts, PART_HEART)) {
		    msg = "$n's heart is torn from $s chest.";
		    vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	    case 5:
		if (IS_SET(ch->parts, PART_ARMS)) {
		    msg = "$n's arm is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	    case 6:
		if (IS_SET(ch->parts, PART_LEGS)) {
		    msg = "$n's leg is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_LEG;
		}
		break;
	    case 7:
		if (IS_SET(ch->parts, PART_BRAINS)) {
		    msg =
			"$n's head is shattered, and $s brains splash all over you.";
		    vnum = OBJ_VNUM_BRAINS;
		}
		break;
	    case 8:
		msg = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case 9:
		msg = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
	    }
	}

    else if (ch->level > 19) {
	switch (number_bits(4)) {
	case 0:
	    msg = "$n hits the ground ... DEAD.";
	    vnum = OBJ_VNUM_BLOOD;
	    break;
	case 1:
	    msg = "$n splatters blood on your armor.";
	    vnum = OBJ_VNUM_BLOOD;
	    break;
	case 2:
	    if (IS_SET(ch->parts, PART_GUTS)) {
		msg = "$n spills $s guts all over the floor.";
		vnum = OBJ_VNUM_GUTS;
	    }
	    break;
	case 3:
	    if (IS_SET(ch->parts, PART_HEAD)) {
		msg = "$n's severed head plops on the ground.";
		vnum = OBJ_VNUM_SEVERED_HEAD;
	    }
	    break;
	case 4:
	    if (IS_SET(ch->parts, PART_HEART)) {
		msg = "$n's heart is torn from $s chest.";
		vnum = OBJ_VNUM_TORN_HEART;
	    }
	    break;
	case 5:
	    if (IS_SET(ch->parts, PART_ARMS)) {
		msg = "$n's arm is sliced from $s dead body.";
		vnum = OBJ_VNUM_SLICED_ARM;
	    }
	    break;
	case 6:
	    if (IS_SET(ch->parts, PART_LEGS)) {
		msg = "$n's leg is sliced from $s dead body.";
		vnum = OBJ_VNUM_SLICED_LEG;
	    }
	    break;
	case 7:
	    if (IS_SET(ch->parts, PART_BRAINS)) {
		msg =
		    "$n's head is shattered, and $s brains splash all over you.";
		vnum = OBJ_VNUM_BRAINS;
	    }
	    break;
	case 8:
	    msg = "$n hits the ground ... DEAD.";
	    vnum = OBJ_VNUM_BLOOD;
	    break;
	case 9:
	    msg = "$n hits the ground ... DEAD.";
	    vnum = OBJ_VNUM_BLOOD;
	    break;
	case 10:
	    if (IS_SET(ch->parts, PART_HEAD)) {
		msg = "$n's severed head plops on the ground.";
		vnum = OBJ_VNUM_SEVERED_HEAD;
	    }
	    break;
	case 11:
	    if (IS_SET(ch->parts, PART_HEART)) {
		msg = "$n's heart is torn from $s chest.";
		vnum = OBJ_VNUM_TORN_HEART;
	    }
	    break;
	case 12:
	    if (IS_SET(ch->parts, PART_ARMS)) {
		msg = "$n's arm is sliced from $s dead body.";
		vnum = OBJ_VNUM_SLICED_ARM;
	    }
	    break;
	case 13:
	    if (IS_SET(ch->parts, PART_LEGS)) {
		msg = "$n's leg is sliced from $s dead body.";
		vnum = OBJ_VNUM_SLICED_LEG;
	    }
	    break;
	case 14:
	    if (IS_SET(ch->parts, PART_BRAINS)) {
		msg =
		    "$n's head is shattered, and $s brains splash all over you.";
		vnum = OBJ_VNUM_BRAINS;
	    }
	}
    }

    act(msg, ch, NULL, NULL, TO_ROOM);
    if ((vnum == 0) && !IS_SET(ch->act, ACT_NO_BODY) && (!IS_SET(ch->in_room->room_flags, ROOM_ARENA))) 
	{

	switch (number_bits(4)) {
	case 0:
	    if (killer == ch)  // This part here is where I want it to return if its a self kill
		return;
	    if ( !IS_NPC( ch ) ) {
		vnum = OVPKT1;
	    } else {
		; // vnum = vnum;
	    }
	    break;
	case 1:
	    if (killer == ch)
		return;
	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token4 = create_object(get_obj_index(OVPKT4), 0);

	    if (!IS_NPC(ch)
		&& token2 != NULL && token2->pIndexData->vnum == OVPKT2) {
		sprintf(buf, token4->short_descr, killer->name, ch->name);
		free_string(token4->short_descr);
		token4->short_descr = str_dup(buf);
		sprintf(buf, token4->description, killer->name, ch->name);
		free_string(token4->description);
		token4->description = str_dup(buf);
		sprintf(buf, token4->name, killer->name, ch->name);
		free_string(token4->name);
		token4->name = str_dup(buf);
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token4, killer);
		DAZE_STATE(killer, 17 + (5 * PULSE_VIOLENCE));
	    } else if ( !IS_NPC( ch ) ) {
		vnum = OVPKT1;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 2:
	    if (killer == ch)
		return;
		if ( !IS_NPC( ch ) ) {
		vnum = OVPKT1;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 3:
	    if (killer == ch)
		return;

	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token4 = create_object(get_obj_index(OVPKT4), 0);

	    if (!IS_NPC(ch)
		&& token2 != NULL && token2->pIndexData->vnum == OVPKT2) {
		sprintf(buf, token4->short_descr, killer->name, ch->name);
		free_string(token4->short_descr);
		token4->short_descr = str_dup(buf);
		sprintf(buf, token4->description, killer->name, ch->name);
		free_string(token4->description);
		token4->description = str_dup(buf);
		sprintf(buf, token4->name, killer->name, ch->name);
		free_string(token4->name);
		token4->name = str_dup(buf);
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token4, killer);
		DAZE_STATE(killer, 17 + (5 * PULSE_VIOLENCE));
	    } else if ( !IS_NPC( ch ) ) {
		vnum = OVPKT2;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 4:
	    if (killer == ch)
		return;
	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token4 = get_eq_char(killer, WEAR_NECK_1);
	    token5 = create_object(get_obj_index(OVPKT5), 0);
	    if (!IS_NPC(ch)
		&& token2 != NULL
		&& token2->pIndexData->vnum == OVPKT2
		&& token4 != NULL && token4->pIndexData->vnum == OVPKT4) {
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token5, killer);
		DAZE_STATE(killer, 24 + (5 * PULSE_VIOLENCE));
	    } else if ( !IS_NPC( ch ) ) {
		vnum = OVPKT1;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 5:
	    if (killer == ch)
		return;
		if ( !IS_NPC( ch ) ) {
		vnum = OVPKT1;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 6:
	    if (killer == ch)
		return;
	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token4 = get_eq_char(killer, WEAR_NECK_1);
	    token5 = create_object(get_obj_index(OVPKT5), 0);
	    if (!IS_NPC(ch)
		&& token2 != NULL
		&& token2->pIndexData->vnum == OVPKT2
		&& token4 != NULL && token4->pIndexData->vnum == OVPKT4) {
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token5, killer);
		DAZE_STATE(killer, 24 + (5 * PULSE_VIOLENCE));
	    } else if ( !IS_NPC( ch ) ) {
		vnum = OVPKT2;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 7:
	    if (killer == ch)
		return;
		if ( !IS_NPC( ch ) ) {
		vnum = OVPKT2;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 8:
	    if (killer == ch)
		return;
	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token4 = get_eq_char(killer, WEAR_NECK_1);
	    token5 = create_object(get_obj_index(OVPKT5), 0);
	    if (!IS_NPC(ch)
		&& token2 != NULL
		&& token2->pIndexData->vnum == OVPKT2
		&& token4 != NULL && token4->pIndexData->vnum == OVPKT4) {
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token5, killer);
		DAZE_STATE(killer, 24 + (5 * PULSE_VIOLENCE));
	    } else if ( !IS_NPC( ch ) ) {
		vnum = OVPKT2;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 9:
	    if (killer == ch)
		return;
	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token3 = create_object(get_obj_index(OVPKT3), 0);
	    if (!IS_NPC(ch)
		&& token2 != NULL && token2->pIndexData->vnum == OVPKT2) {
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token3, killer);
		do_wear(killer, "token");
		DAZE_STATE(killer, 12 + (5 * PULSE_VIOLENCE));
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 10:
	    if (killer == ch)
		return;
	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token4 = create_object(get_obj_index(OVPKT4), 0);
	    if (!IS_NPC(ch)
		&& token2 != NULL && token2->pIndexData->vnum == OVPKT2) {
		sprintf(buf, token4->short_descr, killer->name, ch->name);
		free_string(token4->short_descr);
		token4->short_descr = str_dup(buf);
		sprintf(buf, token4->description, killer->name, ch->name);
		free_string(token4->description);
		token4->description = str_dup(buf);
		sprintf(buf, token4->name, killer->name, ch->name);
		free_string(token4->name);
		token4->name = str_dup(buf);
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token4, killer);
		DAZE_STATE(killer, 17 + (5 * PULSE_VIOLENCE));
	    } else if ( !IS_NPC( ch ) ) {
		vnum = OVPKT2;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 11:
	    if (killer == ch)
		return;
	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token4 = get_eq_char(killer, WEAR_NECK_1);
	    token5 = create_object(get_obj_index(OVPKT5), 0);
	    if (!IS_NPC(ch)
		&& token2 != NULL
		&& token2->pIndexData->vnum == OVPKT2
		&& token4 != NULL && token4->pIndexData->vnum == OVPKT4) {
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token5, killer);
		DAZE_STATE(killer, 24 + (5 * PULSE_VIOLENCE));
	    } else if ( !IS_NPC( ch ) ) {
		vnum = OVPKT2;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	case 12:
	    if (killer == ch)
		return;
	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token4 = get_eq_char(killer, WEAR_NECK_1);
	    token5 = get_eq_char(killer, WEAR_BODY);
	    token6 = create_object(get_obj_index(OVPKT6), 0);
	    if (!IS_NPC(ch)
		&& token2 != NULL
		&& token2->pIndexData->vnum == OVPKT2
		&& token4 != NULL
		&& token4->pIndexData->vnum == OVPKT4
		&& token5 != NULL && token5->pIndexData->vnum == OVPKT5) {
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token6, killer);
		DAZE_STATE(killer, 32 + (5 * PULSE_VIOLENCE));
	    } else if ( !IS_NPC( ch ) ) {
		vnum = OVPKT2;
	    } else {
		vnum = OBJ_VNUM_BLOOD;
	    }
	    break;
	}
    }
    if (vnum != 0) {
	char buf[MSL];
	OBJ_DATA *obj;
	char *name;

	name = IS_NPC(ch) ? ch->short_descr : ch->name;
	obj = create_object(get_obj_index(vnum), 0);
	obj->timer = number_range(4, 7);
	if (!IS_NPC(ch)) {
	    obj->timer = number_range(12, 18);
	}
	if (vnum == OBJ_VNUM_BLOOD) {
	    obj->timer = number_range(1, 4);
	}
	if (IS_NPC(killer) && vnum == OVPKT1 ) {
	    obj->timer = number_range(1, 4);
	} 
	if (IS_NPC(killer) && vnum == OVPKT2 ) {
	    obj->timer = number_range(1, 4);
	} 
	if (!IS_NPC(killer) && vnum == OVPKT1 ) {
	    obj->timer = number_range(100, 200);
	}
	if (!IS_NPC(killer) && vnum == OVPKT2 ) {
	    obj->timer = number_range(100, 200);
	}
	if (vnum == OVPKT3) {
	    obj->timer = number_range(50, 100 * PULSE_VIOLENCE);
	}

	sprintf(buf, obj->short_descr, name);
	free_string(obj->short_descr);
	obj->short_descr = str_dup(buf);

	sprintf(buf, obj->description, name);
	free_string(obj->description);
	obj->description = str_dup(buf);

	sprintf(buf, obj->name, name);
	free_string(obj->name);
	obj->name = str_dup(buf);

	if (obj->item_type == ITEM_FOOD) {
	    if (IS_SET(ch->form, FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form, FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	if (IS_NPC(ch)) {
	    obj->value[4] = 0;
	} else {
	    obj->value[4] = 1;
	}

	obj_to_room(obj, ch->in_room);
    }

    if (IS_NPC(ch))
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for (door = 0; door <= 5; door++) {
	EXIT_DATA *pexit;

	if ((pexit = was_in_room->exit[door]) != NULL
	    && pexit->u1.to_room != NULL
	    && pexit->u1.to_room != was_in_room) {
	    ch->in_room = pexit->u1.to_room;
	    act(msg, ch, NULL, NULL, TO_ROOM);
	}
    }
    ch->in_room = was_in_room;

    return;
}

void raw_kill(CHAR_DATA * victim, CHAR_DATA * killer)
{
    int i;

    death_cry( victim, killer );
    stop_fighting(victim, TRUE);
/*do not make corps if character is in arena*/
    if (!IS_SET(victim->in_room->room_flags, ROOM_ARENA)) {
	make_corpse(victim, killer);
    }

    if (IS_NPC(victim) && !IS_NPC(killer)) {
	if ((killer->can_aquest == 2)
	    && (victim->pIndexData->vnum == killer->pcdata->quest_mob)) {
	    bool found = FALSE;
	    OBJ_DATA *object;
	    int level_vnum;
	    level_vnum = ((killer->level / 10) + 56);

	    for (object = killer->carrying; object != NULL;
		 object = object->next_content) {
		if (IS_OBJ_STAT(object, ITEM_LQUEST)
		    && (object->pIndexData->vnum == level_vnum))
		    found = TRUE;
	    }
	    if (!found) {
		char buf[MIL];
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;
		OBJ_DATA *pouch;
		sprintf(buf, "You quickly pick up the %s.\n\r",
			killer->pcdata->lquest_obj);
		send_to_char(buf, killer);
		object = create_object(get_obj_index(level_vnum), 0);
		sprintf(buf, "%s", killer->pcdata->lquest_obj);
		free_string(object->short_descr);
		object->short_descr = str_dup(buf);
		free_string(object->description);
		object->description = str_dup(buf);	// DB
		free_string(object->name);
		object->name = str_dup(buf);
		buf[0] = '\0';
		SET_BIT(object->extra_flags, ITEM_LQUEST);
		for (obj = killer->carrying; obj != NULL; obj = obj_next) {
		    obj_next = obj->next_content;
		    if (obj->pIndexData->vnum == OBJ_VNUM_QPOUCH) {
			obj_to_obj(object, obj);
			break;
		    } else {
			pouch = create_object(get_obj_index(OBJ_VNUM_QPOUCH), 0);
			obj_to_char(pouch, killer);
			send_to_char ("Your quest pouch returns to you.{x\n\r", killer);
			obj_to_obj(object, pouch);
			break;
		    }
		}
	    }
	}
    }
    if (IS_NPC(victim)) {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL - 1)].killed++;
	extract_char(victim, TRUE);
	if ( !IS_SET(victim->form,FORM_UNDEAD))
	check_spirit(victim, killer);
	return;
    }

    do_mod_favor(victim, 4);

    if (killer->race == victim->race)
	do_mod_favor(killer, 9);

/*do no give pkills, etc, for arena*/
    if (!IS_SET(victim->in_room->room_flags, ROOM_ARENA)) {
	if (!IS_NPC(killer)) {
	    if (strcmp(killer->pcdata->socket, victim->pcdata->socket)) {
		do_mod_favor(killer, 5);
		killer->pcdata->pkills++;
	    }
	    victim->pcdata->pdeath++;
	    if (!is_banklist(victim)) {
		int bank;

		bank = number_range(0, MAX_BANKS - 1);
		if ((number_range(1, 100) < 50) && victim->balance[bank]) {
		    char buf[MSL];
		    int amount;
		    int pwd;
		    OBJ_DATA *pbook;
		    EXTRA_DESCR_DATA *ed;

		    amount =
			(number_range(1, 75) / 100) *
			victim->balance[bank];
		    pwd = number_range(1, 20000);
		    pbook =
			create_object(get_obj_index(OBJ_VNUM_PASSBOOK), 0);
		    sprintf(buf, "%s %s", capitalize(victim->name),
			    pbook->name);
		    free_string(pbook->name);
		    pbook->name = str_dup(buf);
		    sprintf(buf,
			    "The passbook is covered with strange magical symbols that prevent your eyes\n\rfrom focusing on the inscriptions.  Only the word {B%s{x is legible.\n\r",
			    capitalize(victim->name));
		    ed = alloc_perm(sizeof(*ed));
		    ed->keyword = str_dup("passbook");
		    ed->description = str_dup(buf);
		    ed->next = pbook->extra_descr;
		    pbook->extra_descr = ed;
		    pbook->value[0] = bank;
		    pbook->value[1] = pwd;
		    pbook->value[2] = amount;
		    obj_to_char(pbook, killer);
		    send_to_char
			("{RA passbook appears in your inventory!{x\n\r",
			 killer);
		    change_banklist(killer, TRUE, bank, amount, pwd,
				    victim->name);
		}
	    }
	}
	extract_char(victim, FALSE);
	while (victim->affected)
	    affect_remove(victim, victim->affected);
	victim->affected_by = race_table[victim->race].aff;
	victim->shielded_by = race_table[victim->race].shd;
	for (i = 0; i < 4; i++)
	    victim->armor[i] = 100;
	victim->position = POS_STANDING;
	victim->hit = UMAX(1, victim->hit);
	victim->mana = UMAX(1, victim->mana);
	victim->move = UMAX(1, victim->move);
	return;
    }
}


void group_gain(CHAR_DATA * ch, CHAR_DATA * victim)
{
    char buf[MSL];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if (victim == ch)
	return;

    members = 0;
    group_levels = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
	if (is_same_group(gch, ch)) {
	    members++;
	    group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;
	}
    }

    if (members == 0) {
	bug("Group_gain: members.", members);
	members = 1;
	group_levels = ch->level;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if (!is_same_group(gch, ch) || IS_NPC(gch))
	    continue;

/*	Taken out, add it back if you want it
	if ( gch->level - lch->level >= 5 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -5 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}
*/

	xp = xp_compute(gch, victim, group_levels);
	if (global_xpq){ xp = xp*2; }
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_LQUEST))
	    xp = 0;
	sprintf(buf, "{BYou receive {W%d{B experience points.{x\n\r", xp);
	send_to_char(buf, gch);
	gain_exp(gch, xp);

	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
	    obj_next = obj->next_content;
	    if (obj->wear_loc == WEAR_NONE)
		continue;

	    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
		|| (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
		|| (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
		act("{cYou are {Wzapped{c by $p.{x", ch, obj, NULL,
		    TO_CHAR);
		act("$n is {Wzapped{x by $p.", ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
	    }
	}

	if (IS_SET(ch->exbit1_flags, PLR_QUESTOR) && IS_NPC(victim)) {
	    if (ch->questmob == victim->pIndexData->vnum) {
		send_to_char
		    ("You have almost completed your ADVENTURE!\n\r", ch);
		send_to_char
		    ("Return to the questmaster before your time runs out!\n\r",
		     ch);
		ch->questmob = -1;
	    }
	}
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute(CHAR_DATA * gch, CHAR_DATA * victim, int total_levels)
{
    int xp, base_exp;
    int align, level_range;
    int change;
    int time_per_level;

    level_range = victim->level - gch->level;
    if (!IS_NPC(gch)) {
//	if (gch->class >= MCLT_1)
//	    level_range -= 4;
	if (gch->pcdata->tier == 3) { level_range -= 4; }
	if (gch->pcdata->tier == 2) { level_range -= 3; }
	if (gch->pcdata->tier == 1) { level_range -= 2; }
	if (gch->pcdata->tier == 0) { level_range -= 1; }
    }

    /* compute the base exp */
    switch (level_range) {
    default:
	base_exp = 0;
	break;
    case -9:
	base_exp = 1;
	break;
    case -8:
	base_exp = 3;
	break;
    case -7:
	base_exp = 8;
	break;
    case -6:
	base_exp = 14;
	break;
    case -5:
	base_exp = 22;
	break;
    case -4:
	base_exp = 35;
	break;
    case -3:
	base_exp = 49;
	break;
    case -2:
	base_exp = 61;
	break;
    case -1:
	base_exp = 77;
	break;
    case 0:
	base_exp = 95;
	break;
    case 1:
	base_exp = 115;
	break;
    case 2:
	base_exp = 135;
	break;
    case 3:
	base_exp = 157;
	break;
    case 4:
	base_exp = 181;
	break;
    }

    if (level_range > 4)
	base_exp = 181 + 29 * (level_range - 4);

    /* do alignment computations */

    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act, ACT_NOALIGN)) {
	/* no change */
    }

    else if (align > 500) {	/* monster is more good than slayer */
	change =
	    (align - 500) * base_exp / 500 * gch->level / total_levels;
	change = UMAX(1, change);
	gch->alignment = UMAX(-1000, gch->alignment - change);
	if (gch->pet != NULL)
	    gch->pet->alignment = gch->alignment;
    }

    else if (align < -500) {	/* monster is more evil than slayer */
	change =
	    (-1 * align -
	     500) * base_exp / 500 * gch->level / total_levels;
	change = UMAX(1, change);
	gch->alignment = UMIN(1000, gch->alignment + change);
	if (gch->pet != NULL)
	    gch->pet->alignment = gch->alignment;
    }

    else {			/* improve this someday */

	change =
	    gch->alignment * base_exp / 500 * gch->level / total_levels;
	gch->alignment -= change;
	if (gch->pet != NULL)
	    gch->pet->alignment = gch->alignment;
    }

    /* calculate exp multiplier */
    if (IS_SET(victim->act, ACT_NOALIGN))
	xp = base_exp;

    else if (gch->alignment > 500) {	
	if (victim->alignment <= -750) xp = (base_exp * 4) / 3;
	else if (victim->alignment <= -500 && victim->alignment > -750) xp = (base_exp * 5) / 4;
	else if (victim->alignment <= -250 && victim->alignment > -500) xp = (base_exp * 6) / 5;
	else if (victim->alignment <= 250 && victim->alignment > -250) xp = (base_exp * 7) / 6;
	else if (victim->alignment <= 500 && victim->alignment > 250) xp = (base_exp * 8) / 7;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) {

	if (victim->alignment >= 750) xp = (base_exp * 4) / 3;
	else if (victim->alignment >= 500 && victim->alignment < 750) xp = (base_exp * 5) / 4;
	else if (victim->alignment >= 250 && victim->alignment < 500) xp = (base_exp * 6) / 5;
	else if (victim->alignment >= -250 && victim->alignment < 250) xp = (base_exp * 7) / 6;
	else if (victim->alignment >= -500 && victim->alignment < -250) xp = (base_exp * 8) / 7;
	else if (victim->alignment > 500)
	    xp = (base_exp * 11) / 10;

	else if (victim->alignment < -750)
	    xp = base_exp / 2;

	else if (victim->alignment < -500)
	    xp = (base_exp * 3) / 4;

	else if (victim->alignment < -250)
	    xp = (base_exp * 9) / 10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200) {	/* a little good */

	if (victim->alignment < -500)
	    xp = (base_exp * 6) / 5;

	else if (victim->alignment > 750)
	    xp = base_exp / 2;

	else if (victim->alignment > 0)
	    xp = (base_exp * 3) / 4;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) {	/* a little bad */
	if (victim->alignment > 500)
	    xp = (base_exp * 6) / 5;

	else if (victim->alignment < -750)
	    xp = base_exp / 2;

	else if (victim->alignment < 0)
	    xp = (base_exp * 3) / 4;

	else
	    xp = base_exp;
    }

    else {			/* neutral */


	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = (base_exp * 4) / 3;

	else if (victim->alignment < 200 && victim->alignment > -200)
	    xp = base_exp / 2;

	else
	    xp = base_exp;
    }

    /* more exp at the low levels */
    if (gch->level < 11)
	xp = 15 * xp / (gch->level + 4);

    /* less at high */
    if (gch->level > 60)
	xp = 15 * xp / (gch->level - 25);

    /* reduce for playing time */


    {
	/* compute quarter-hours per level */
	time_per_level = 4 *
	    (gch->played + (int) (current_time - gch->logon)) / 3600
	    / gch->level;

	time_per_level = URANGE(2, time_per_level, 12);
	if (gch->level < 25)	/* make it a curve */
	    time_per_level = UMAX(time_per_level, (25 - gch->level));
/*
 *	xp = xp * time_per_level / 12;
 */
    }
/*    xp = xp*.75;
 */
    /* randomize the rewards */
    xp = number_range(xp * 3 / 4, xp * 5 / 4);

    /* adjust for grouping */
    xp = xp * gch->level / (UMAX(1, total_levels - 1));

    if (!IS_NPC(gch) && !IS_NPC(victim))
	if (xp > 1)
	    xp = xp / 2;

    return xp;
}


void dam_message(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
		 bool immune)
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

    if (dam == 0) {
	vs = "miss";
	vp = "misses";
    } else if (dam <= 4) {
	vs = "scratch";
	vp = "scratches";
    } else if (dam <= 8) {
	vs = "graze";
	vp = "grazes";
    } else if (dam <= 12) {
	vs = "hit";
	vp = "hits";
    } else if (dam <= 16) {
	vs = "injure";
	vp = "injures";
    } else if (dam <= 20) {
	vs = "wound";
	vp = "wounds";
    } else if (dam <= 24) {
	vs = "maul";
	vp = "mauls";
    } else if (dam <= 28) {
	vs = "decimate";
	vp = "decimates";
    } else if (dam <= 32) {
	vs = "devastate";
	vp = "devastates";
    } else if (dam <= 36) {
	vs = "maim";
	vp = "maims";
    } else if (dam <= 40) {
	vs = "MUTILATE";
	vp = "MUTILATES";
    } else if (dam <= 44) {
	vs = "DISEMBOWEL";
	vp = "DISEMBOWELS";
    } else if (dam <= 48) {
	vs = "DISMEMBER";
	vp = "DISMEMBERS";
    } else if (dam <= 52) {
	vs = "MASSACRE";
	vp = "MASSACRES";
    } else if (dam <= 56) {
	vs = "MANGLE";
	vp = "MANGLES";
    } else if (dam <= 60) {
	vs = "*** DEMOLISH ***";
	vp = "*** DEMOLISHES ***";
    } else if (dam <= 75) {
	vs = "*** DEVASTATE ***";
	vp = "*** DEVASTATES ***";
    } else if (dam <= 100) {
	vs = "=== OBLITERATE ===";
	vp = "=== OBLITERATES ===";
    } else if (dam <= 125) {
	vs = ">>> ANNIHILATE <<<";
	vp = ">>> ANNIHILATES <<<";
    } else if (dam <= 150) {
	vs = "<<< ERADICATE >>>";
	vp = "<<< ERADICATES >>>";
    } else {
	vs = "do UNSPEAKABLE things to";
	vp = "does UNSPEAKABLE things to";
    }

    punct = (dam <= 24) ? '.' : '!';

    if (dt == TYPE_HIT) {
	if (ch == victim) {
	    sprintf(buf1, "$n %s $melf%c{x", vp, punct);
	    sprintf(buf2, "{hYou %s {hyourself%c{x", vs, punct);
	} else {
            sprintf(buf1, "{k$n %s {k$N%c {R[{k%d{R]{x", vp, punct, dam);
            sprintf(buf2, "{hYou %s {h$N%c {R[{h%d{R]{x", vs, punct, dam);
            sprintf(buf3, "{i$n %s {iyou%c {R[{i%d{R]{x", vp, punct, dam);
	}
    } else {
	if (dt >= 0 && dt < MAX_SKILL)
	    attack = skill_table[dt].noun_damage;
	else if (dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE)
	    attack = attack_table[dt - TYPE_HIT].noun;
	else {
	    bug("Dam_message: bad dt %d.", dt);
	    dt = TYPE_HIT;
	    attack = attack_table[0].name;
	}

	if (immune) {
	    if (ch == victim) {
		sprintf(buf1, "$n is unaffected by $s own %s.{x",
			attack);
		sprintf(buf2, "{hLuckily, you are immune to that.{x");
	    } else {
		sprintf(buf1, "$N is unaffected by $n's %s!{x", attack);
		sprintf(buf2, "{h$N is unaffected by your %s!{x", attack);
		sprintf(buf3, "$n's %s is powerless against you.{x",
			attack);
	    }
	} else {
	    if (ch == victim) {
		sprintf(buf1, "$n's %s %s $m%c{x", attack, vp, punct);
		sprintf(buf2, "{hYour %s %s you%c{x", attack, vp, punct);
	    } else {
		sprintf(buf1, "$n's %s %s $N%c {R[%d{R]{x", attack,
			vp, punct, dam);
		sprintf(buf2, "{hYour %s %s {h$N%c {R[{h%d{R]{x", attack,
			vp, punct, dam);
		sprintf(buf3, "$n's %s %s you%c {R[%d{R]{x", attack,
			vp, punct, dam);
	    }
	}
    }

    if (ch == victim) {
	act(buf1, ch, NULL, NULL, TO_ROOM);
	act(buf2, ch, NULL, NULL, TO_CHAR);
    } else {
	act(buf1, ch, NULL, victim, TO_NOTVICT);
	act(buf2, ch, NULL, victim, TO_CHAR);
	act(buf3, ch, NULL, victim, TO_VICT);
    }

    return;
}

void do_surrender( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    if ( (mob = ch->fighting) == NULL )
    {
	send_to_char( "But you're not fighting!\n\r", ch );
	return;
    }
    act( "You surrender to $N!", ch, NULL, mob, TO_CHAR );
    act( "$n surrenders to you!", ch, NULL, mob, TO_VICT );
    act( "$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT );
    stop_fighting( ch, TRUE );

    if ( !IS_NPC( ch ) && IS_NPC( mob ) 
    &&   ( !HAS_TRIGGER( mob, TRIG_SURR ) 
        || !mp_percent_trigger( mob, ch, NULL, NULL, TRIG_SURR ) ) )
    {
	act( "$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR );
	multi_hit( mob, ch, TYPE_UNDEFINED );
    }
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm(CHAR_DATA * ch, CHAR_DATA * victim)
{
    OBJ_DATA *obj;

    if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL)
	return;

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    if (IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
	act("{j$S weapon won't budge!{x", ch, NULL, victim, TO_CHAR);
	act("{j$n tries to disarm you, but your weapon won't budge!{x",
	    ch, NULL, victim, TO_VICT);
	act("$n tries to disarm $N, but fails.{x", ch, NULL, victim,
	    TO_NOTVICT);
	return;
    }

    act("{j$n DISARMS you and sends your weapon flying!{x",
	ch, NULL, victim, TO_VICT);
    act("{jYou disarm $N!{x", ch, NULL, victim, TO_CHAR);
    act("$n disarms $N!{x", ch, NULL, victim, TO_NOTVICT);

    obj_from_char(obj);
    if (IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_INVENTORY))
	obj_to_char(obj, victim);
    else {
	obj_to_room(obj, victim->in_room);
	if (IS_NPC(victim) && victim->wait == 0
	    && can_see_obj(victim, obj)) get_obj(victim, obj, NULL);
    }

    return;
}

void do_berserk(CHAR_DATA * ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch, gsn_berserk)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_BERSERK))
	|| (!IS_NPC(ch)
	    && ch->level <
	    skill_table[gsn_berserk].skill_level[ch->class])) {
	if (IS_NPC(ch)) {
	    send_to_char
		("{hYou turn {rred{h in the face, but nothing happens.{x\n\r",
		 ch);
	    return;
	}
	if (ch->pcdata->tier != 2) {
	    send_to_char
		("{hYou turn {rred{h in the face, but nothing happens.{x\n\r",
		 ch);
	    return;
	} else if (chance == 0
		   || (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_BERSERK))
		   || (!IS_NPC(ch)
		       && ch->level <
		       skill_table[gsn_berserk].skill_level[ch->clasb])) {
	    send_to_char
		("{hYou turn {rred{h in the face, but nothing happens.{x\n\r",
		 ch);
	    return;
	}
    }

    if (IS_AFFECTED(ch, AFF_BERSERK) || is_affected(ch, gsn_berserk)
	|| is_affected(ch, skill_lookup("frenzy"))) {
	send_to_char("{hYou get a little madder.{x\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CALM)) {
	send_to_char("{hYou're feeling to mellow to berserk.{x\n\r", ch);
	return;
    }

    if (ch->mana < 50) {
	send_to_char("{hYou can't get up enough energy.{x\n\r", ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit / ch->max_hit;
    chance += 25 - hp_percent / 2;

    if (number_percent() < chance) {
	AFFECT_DATA af;

	WAIT_STATE(ch, PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit, ch->max_hit);

	if (ch->shadow) {
	    ch->shadowing->shadowed = FALSE;
	    ch->shadowing->shadower = NULL;
	    ch->shadowing = NULL;
	    ch->shadow = FALSE;
	}
	send_to_char
	    ("{hYour pulse races as you are consumed by {rrage!{x\n\r",
	     ch);
	act("$n gets a {cw{gi{rl{yd look in $s eyes.{x", ch, NULL,
	    NULL, TO_ROOM);
	check_improve(ch, gsn_berserk, TRUE, 2);

	af.where = TO_AFFECTS;
	af.type = gsn_berserk;
	af.level = ch->level;
	af.duration = number_fuzzy(ch->level / 8);
	af.modifier = UMAX(1, ch->level / 5);
	af.bitvector = AFF_BERSERK;

	af.location = APPLY_HITROLL;
	affect_to_char(ch, &af);

	af.location = APPLY_DAMROLL;
	affect_to_char(ch, &af);

	af.modifier = UMAX(10, 10 * (ch->level / 5));
	af.location = APPLY_AC;
	affect_to_char(ch, &af);
    }

    else {
	WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("{hYour pulse speeds up, but nothing happens.{x\n\r",
		     ch);
	check_improve(ch, gsn_berserk, FALSE, 2);
    }
}

void do_voodoo(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    OBJ_DATA *doll;

    if (IS_NPC(ch))
	return;

    doll = get_eq_char(ch, WEAR_HOLD);
    if (doll == NULL || (doll->pIndexData->vnum != OBJ_VNUM_VOODOO)) {
	send_to_char("You are not holding a voodoo doll.\n\r", ch);
	return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Syntax: voodoo <action>\n\r", ch);
	send_to_char("Actions: pin trip throw\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "pin")) {
	do_vdpi(ch, doll->name);
	return;
    }

    if (!str_cmp(arg, "trip")) {
	do_vdtr(ch, doll->name);
	return;
    }

    if (!str_cmp(arg, "throw")) {
	do_vdth(ch, doll->name);
	return;
    }

    do_voodoo(ch, "");
}

void do_vdpi(CHAR_DATA * ch, char *argument)
{
    char arg1[MIL];
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    bool found = FALSE;

    argument = one_argument(argument, arg1);

    for (d = descriptor_list; d != NULL; d = d->next) {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING || !can_see(ch, d->character))
	    continue;

	wch = (d->original != NULL) ? d->original : d->character;

	if (!can_see(ch, wch))
	    continue;

	if (!str_cmp(arg1, wch->name) && !found) {
	    if (IS_NPC(wch))
		continue;

	    if (IS_IMMORTAL(wch) && (wch->level > ch->level)) {
		send_to_char("That's not a good idea.\n\r", ch);
		return;
	    }

	    if ((wch->level < 20) && !IS_IMMORTAL(ch)) {
		send_to_char("They are a little too young for that.\n\r",
			     ch);
		return;
	    }

	    if (IS_SHIELDED(wch, SHD_PROTECT_VOODOO)) {
		send_to_char
		    ("They are still realing from a previous voodoo.\n\r",
		     ch);
		return;
	    }

	    found = TRUE;

	    if (ch->shadow) {
		ch->shadowing->shadowed = FALSE;
		ch->shadowing->shadower = NULL;
		ch->shadowing = NULL;
		ch->shadow = FALSE;
	    }
	    send_to_char("You stick a pin into your voodoo doll.\n\r", ch);
	    act("$n sticks a pin into a voodoo doll.", ch, NULL, NULL,
		TO_ROOM);
	    send_to_char
		("{RYou double over with a sudden pain in your gut!{x\n\r",
		 wch);
	    act("$n suddenly doubles over with a look of extreme pain!",
		wch, NULL, NULL, TO_ROOM);
	    af.where = TO_SHIELDS;
	    af.type = skill_lookup("protection voodoo");
	    af.level = wch->level;
	    af.duration = 1;
	    af.location = APPLY_NONE;
	    af.modifier = 0;
	    af.bitvector = SHD_PROTECT_VOODOO;
	    affect_to_char(wch, &af);
	    return;
	}
    }
    send_to_char("Your victim doesn't seem to be in the realm.\n\r", ch);
    return;
}

void do_vdtr(CHAR_DATA * ch, char *argument)
{
    char arg1[MIL];
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    bool found = FALSE;

    argument = one_argument(argument, arg1);

    for (d = descriptor_list; d != NULL; d = d->next) {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING || !can_see(ch, d->character))
	    continue;

	wch = (d->original != NULL) ? d->original : d->character;

	if (!can_see(ch, wch))
	    continue;

	if (!str_cmp(arg1, wch->name) && !found) {
	    if (IS_NPC(wch))
		continue;

	    if (IS_IMMORTAL(wch) && (wch->level > ch->level)) {
		send_to_char("That's not a good idea.\n\r", ch);
		return;
	    }

	    if ((wch->level < 20) && !IS_IMMORTAL(ch)) {
		send_to_char("They are a little too young for that.\n\r",
			     ch);
		return;
	    }

	    if (IS_SHIELDED(wch, SHD_PROTECT_VOODOO)) {
		send_to_char
		    ("They are still realing from a previous voodoo.\n\r",
		     ch);
		return;
	    }

	    found = TRUE;

	    if (ch->shadow) {
		ch->shadowing->shadowed = FALSE;
		ch->shadowing->shadower = NULL;
		ch->shadowing = NULL;
		ch->shadow = FALSE;
	    }
	    send_to_char
		("You slam your voodoo doll against the ground.\n\r", ch);
	    act("$n slams a voodoo doll against the ground.", ch, NULL,
		NULL, TO_ROOM);
	    send_to_char("{RYour feet slide out from under you!{x\n\r",
			 wch);
	    send_to_char("{RYou hit the ground face first!{x\n\r", wch);
	    act
		("$n trips over $s own feet, and does a nose dive into the ground!",
		 wch, NULL, NULL, TO_ROOM);
	    af.where = TO_SHIELDS;
	    af.type = skill_lookup("protection voodoo");
	    af.level = wch->level;
	    af.duration = 1;
	    af.location = APPLY_NONE;
	    af.modifier = 0;
	    af.bitvector = SHD_PROTECT_VOODOO;
	    affect_to_char(wch, &af);
	    return;
	}
    }
    send_to_char("Your victim doesn't seem to be in the realm.\n\r", ch);
    return;
}

void do_vdth(CHAR_DATA * ch, char *argument)
{
    char arg1[MIL];
    char buf[MSL];
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    bool found = FALSE;
    int attempt;

    argument = one_argument(argument, arg1);

    for (d = descriptor_list; d != NULL; d = d->next) {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING || !can_see(ch, d->character))
	    continue;

	wch = (d->original != NULL) ? d->original : d->character;

	if (!can_see(ch, wch))
	    continue;

	if (!str_cmp(arg1, wch->name) && !found) {
	    if (IS_NPC(wch))
		continue;

	    if (IS_IMMORTAL(wch) && (wch->level > ch->level)) {
		send_to_char("That's not a good idea.\n\r", ch);
		return;
	    }

	    if ((wch->level < 20) && !IS_IMMORTAL(ch)) {
		send_to_char("They are a little too young for that.\n\r",
			     ch);
		return;
	    }

	    if (IS_SHIELDED(wch, SHD_PROTECT_VOODOO)) {
		send_to_char
		    ("They are still reeling from a previous voodoo.\n\r",
		     ch);
		return;
	    }

	    found = TRUE;

	    if (ch->shadow) {
		ch->shadowing->shadowed = FALSE;
		ch->shadowing->shadower = NULL;
		ch->shadowing = NULL;
		ch->shadow = FALSE;
	    }
	    send_to_char("You toss your voodoo doll into the air.\n\r",
			 ch);
	    act("$n tosses a voodoo doll into the air.", ch, NULL, NULL,
		TO_ROOM);
	    af.where = TO_SHIELDS;
	    af.type = skill_lookup("protection voodoo");
	    af.level = wch->level;
	    af.duration = 1;
	    af.location = APPLY_NONE;
	    af.modifier = 0;
	    af.bitvector = SHD_PROTECT_VOODOO;
	    affect_to_char(wch, &af);
	    if ((wch->fighting != NULL) || (number_percent() < 25)) {
		send_to_char
		    ("{RA sudden gust of wind throws you through the air!{x\n\r",
		     wch);
		send_to_char
		    ("{RYou slam face first into the nearest wall!{x\n\r",
		     wch);
		act
		    ("A sudden gust of wind picks up $n and throws $m into a wall!",
		     wch, NULL, NULL, TO_ROOM);
		return;
	    }
	    wch->position = POS_STANDING;
	    was_in = wch->in_room;
	    for (attempt = 0; attempt < 6; attempt++) {
		EXIT_DATA *pexit;
		int door;

		door = number_door();
		if ((pexit = was_in->exit[door]) == 0
		    || pexit->u1.to_room == NULL
		    || IS_SET(pexit->exit_info, EX_CLOSED)
		    || (IS_NPC(wch)
			&& IS_SET(pexit->u1.to_room->room_flags,
				  ROOM_NO_MOB))) continue;

		move_char(wch, door, FALSE, TRUE);
		if ((now_in = wch->in_room) == was_in)
		    continue;

		wch->in_room = was_in;
		sprintf(buf,
			"A sudden gust of wind picks up $n and throws $m to the %s.",
			dir_name[door]);
		act(buf, wch, NULL, NULL, TO_ROOM);
		send_to_char
		    ("{RA sudden gust of wind throws you through the air!{x\n\r",
		     wch);
		wch->in_room = now_in;
		act
		    ("$n sails into the room and slams face first into a wall!",
		     wch, NULL, NULL, TO_ROOM);
		do_look(wch, "auto");
		send_to_char
		    ("{RYou slam face first into the nearest wall!{x\n\r",
		     wch);
		return;
	    }
	    send_to_char
		("{RA sudden gust of wind throws you through the air!{x\n\r",
		 wch);
	    send_to_char
		("{RYou slam face first into the nearest wall!{x\n\r",
		 wch);
	    act
		("A sudden gust of wind picks up $n and throws $m into a wall!",
		 wch, NULL, NULL, TO_ROOM);
	    return;
	}
    }
    send_to_char("Your victim doesn't seem to be in the realm.\n\r", ch);
    return;
}

void do_bash(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_bash)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_BASH))) {
	send_to_char("Bashing? What's that?\n\r", ch);
	return;
    }

    if (!IS_NPC(ch)) {
	if (ch->pcdata->tier != 2) {
	    if (ch->level < skill_table[gsn_bash].skill_level[ch->class]) {
		send_to_char("Bashing? What's that?\n\r", ch);
		return;
	    }
	} else
	    if ((ch->level < skill_table[gsn_bash].skill_level[ch->class])
		&& (ch->level <
		    skill_table[gsn_bash].skill_level[ch->clasb])) {
	    send_to_char("Bashing? What's that?\n\r", ch);
	    return;
	}
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("But you aren't fighting anyone!\n\r", ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->position < POS_FIGHTING) {
	act("You'll have to let $M get back up first.", ch, NULL, victim,
	    TO_CHAR);
	return;
    }

    if (victim == ch) {
	send_to_char("You try to bash your brains out, but fail.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }

    if (!can_see(ch, victim)) {
	send_to_char
	    ("You get a running start, and slam right into a wall.\n\r",
	     ch);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    if ((ch->fighting == NULL)
	&& (!IS_NPC(ch))
	&& (!IS_NPC(victim))) {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10;


    /* stats */
    chance += get_curr_stat(ch, STAT_STR);
    chance -= (get_curr_stat(victim, STAT_DEX) * 4) / 3;
    chance -= GET_AC(victim, AC_BASH) / 25;
    /* speed */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE)) chance -= 30;

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC(victim)
	&& chance < get_skill(victim, gsn_dodge)) {	/*
	act("$n tries to bash you, but you dodge it.{x",ch,NULL,victim,TO_VICT);
	act("{h$N dodges your bash, you fall flat on your face.{x",ch,NULL,victim,TO_CHAR);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	return; */
	chance -= 3 * (get_skill(victim, gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance) {
	int dam;

	dam = number_range(2, 2 + 2 * ch->size + chance / 20);
	act("$n sends you sprawling with a powerful bash!{x", ch, NULL,
	    victim, TO_VICT);
	act("{hYou slam into $N, and send $M flying!{x", ch, NULL, victim,
	    TO_CHAR);
	act("$n sends $N sprawling with a powerful bash.{x", ch, NULL,
	    victim, TO_NOTVICT);
	check_improve(ch, gsn_bash, TRUE, 1);

	DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
	WAIT_STATE(ch, skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;
	damage(ch, victim, dam, gsn_bash, DAM_BASH, TRUE);
	chance = (get_skill(ch, gsn_stun) / 5);
	if (number_percent() < chance) {
	    chance = (get_skill(ch, gsn_stun) / 5);
	    if (number_percent() < chance) {
		victim->stunned = 2;
	    } else {
		victim->stunned = 1;
	    }
	    act("You are stunned, and have trouble getting back up!{x",
		ch, NULL, victim, TO_VICT);
	    act("{h$N is stunned by your bash!{x", ch, NULL, victim,
		TO_CHAR);
	    act("$N is having trouble getting back up.{x", ch, NULL,
		victim, TO_NOTVICT);
	    check_improve(ch, gsn_stun, TRUE, 1);
	}
    } else {
	damage(ch, victim, 0, gsn_bash, DAM_BASH, FALSE);
	act("{hYou fall flat on your face!{x", ch, NULL, victim, TO_CHAR);
	act("$n falls flat on $s face.{x", ch, NULL, victim, TO_NOTVICT);
	act("You evade $n's bash, causing $m to fall flat on $s face.{x",
	    ch, NULL, victim, TO_VICT);
	check_improve(ch, gsn_bash, FALSE, 1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch, skill_table[gsn_bash].beats * 3 / 2);
    }
}

void do_dirt(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_dirt)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK_DIRT))) {
	send_to_char("{hYou get your feet dirty.{x\n\r", ch);
	return;
    }

    if (!IS_NPC(ch)) {
	if (ch->pcdata->tier != 2) {
	    if (ch->level < skill_table[gsn_dirt].skill_level[ch->class]) {
		send_to_char("{hYou get your feet dirty.{x\n\r", ch);
		return;
	    }
	} else
	    if ((ch->level < skill_table[gsn_dirt].skill_level[ch->class])
		&& (ch->level <
		    skill_table[gsn_dirt].skill_level[ch->clasb])) {
	    send_to_char("{hYou get your feet dirty.{x\n\r", ch);
	    return;
	}
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("But you aren't in combat!\n\r", ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_BLIND)) {
	act("{h$E's already been blinded.{x", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (victim == ch) {
	send_to_char("Very funny.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }

    if (IS_SET(victim->exbit1_flags, RECRUIT)
	|| IS_SET(victim->exbit1_flags, PK_VETERAN)
	|| IS_SET(victim->exbit1_flags, PK_LAWFUL)
	|| IS_SET(victim->exbit1_flags, PK_KILLER)
	|| IS_SET(victim->exbit1_flags, PK_KILLER)
	|| IS_SET(ch->exbit1_flags, RECRUIT)
	|| IS_SET(ch->exbit1_flags, PK_VETERAN)
	|| IS_SET(ch->exbit1_flags, PK_LAWFUL)
	|| IS_SET(ch->exbit1_flags, PK_KILLER) ) {

	if (victim->fighting != NULL && IS_SET(ch->exbit1_flags, PK_LAWFUL) && !IS_NPC(victim)) {
	    send_to_char("So thats the way you like to play.\n\r", ch);
	    REMOVE_BIT(ch->exbit1_flags, PK_LAWFUL);
	    SET_BIT(ch->exbit1_flags, PK_KILLER);
	}
	    else if (victim->fighting != NULL && IS_SET(ch->exbit1_flags, PK_KILLER) && !IS_NPC(victim)) {
	    send_to_char("You really deserve this.\n\r", ch);
	    REMOVE_BIT(ch->exbit1_flags, PK_KILLER);
	    SET_BIT(ch->exbit1_flags, PK_KILLER2);
	}
	// send_to_char("Have mercy on them.\n\r", ch);
    }

    if ((ch->fighting == NULL)
	&& (!IS_NPC(ch))
	&& (!IS_NPC(victim))) {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= 2 * get_curr_stat(victim, STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE)) chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch (ch->in_room->sector_type) {
    case (SECT_INSIDE):
	chance -= 20;
	break;
    case (SECT_CITY):
	chance -= 10;
	break;
    case (SECT_FIELD):
	chance += 5;
	break;
    case (SECT_FOREST):
	break;
    case (SECT_HILLS):
	break;
    case (SECT_MOUNTAIN):
	chance -= 10;
	break;
    case (SECT_WATER_SWIM):
	chance = 0;
	break;
    case (SECT_WATER_NOSWIM):
	chance = 0;
	break;
    case (SECT_AIR):
	chance = 0;
	break;
    case (SECT_DESERT):
	chance += 10;
	break;
    }

    if (chance == 0) {
	send_to_char("{hThere isn't any dirt to kick.{x\n\r", ch);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    /* now the attack */
    if (number_percent() < chance) {
	AFFECT_DATA af;
	int dam;

	dam = number_range(2, 5);

	act("$n is blinded by the dirt in $s eyes!{x", victim, NULL,
	    NULL, TO_ROOM);
	act("$n kicks dirt in your eyes!{x", ch, NULL, victim, TO_VICT);
	damage(ch, victim, dam, gsn_dirt, DAM_NONE, TRUE);
	send_to_char("{DYou can't see a thing!{x\n\r", victim);
	check_improve(ch, gsn_dirt, TRUE, 2);
	WAIT_STATE(ch, skill_table[gsn_dirt].beats);

	af.where = TO_AFFECTS;
	af.type = gsn_dirt;
	af.level = ch->level;
	af.duration = 0;
	af.location = APPLY_HITROLL;
	af.modifier = -4;
	af.bitvector = AFF_BLIND;

	affect_to_char(victim, &af);
    } else {
	damage(ch, victim, 0, gsn_dirt, DAM_NONE, TRUE);
	check_improve(ch, gsn_dirt, FALSE, 2);
	WAIT_STATE(ch, skill_table[gsn_dirt].beats);
    }
}

void do_gouge(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);


    if ((chance = get_skill(ch, gsn_gouge)) == 0) {
	send_to_char("Gouge?  What's that?\n\r", ch);
	return;
    }

    if (!IS_NPC(ch)) {
	if (ch->pcdata->tier != 2) {
	    if (ch->level < skill_table[gsn_gouge].skill_level[ch->class]) {
		send_to_char("Gouge?  What's that?\n\r", ch);
		return;
	    }
	} else
	    if ((ch->level < skill_table[gsn_gouge].skill_level[ch->class])
		&& (ch->level <
		    skill_table[gsn_gouge].skill_level[ch->clasb])) {
	    send_to_char("Gouge?  What's that?\n\r", ch);
	    return;
	}
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("But you aren't in combat!\n\r", ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_BLIND)) {
	act("{h$E's already been blinded.{x", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (victim == ch) {
	send_to_char("Very funny.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    if ((ch->fighting == NULL)
	&& (!IS_NPC(ch))
	&& (!IS_NPC(victim))) {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= 2 * get_curr_stat(victim, STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE)) chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* now the attack */
    if (number_percent() < chance) {
	AFFECT_DATA af;
	int dam;

	dam = number_range(2, 8);

	act("$n is blinded by a poke in the eyes!{x", victim, NULL, NULL,
	    TO_ROOM);
	act("$n gouges at your eyes!{x", ch, NULL, victim, TO_VICT);
	damage(ch, victim, dam, gsn_gouge, DAM_NONE, TRUE);
	send_to_char("{DYou see nothing but stars!{x\n\r", victim);
	check_improve(ch, gsn_gouge, TRUE, 2);
	WAIT_STATE(ch, skill_table[gsn_gouge].beats);

	af.where = TO_AFFECTS;
	af.type = gsn_gouge;
	af.level = ch->level;
	af.duration = 0;
	af.location = APPLY_HITROLL;
	af.modifier = -4;
	af.bitvector = AFF_BLIND;

	affect_to_char(victim, &af);
    } else {
	damage(ch, victim, 0, gsn_gouge, DAM_NONE, TRUE);
	check_improve(ch, gsn_gouge, FALSE, 2);
	WAIT_STATE(ch, skill_table[gsn_gouge].beats);
    }
}

void do_trip(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_trip)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_TRIP))) {
	send_to_char("Tripping?  What's that?\n\r", ch);
	return;
    }

    if (!IS_NPC(ch)) {
	if (ch->pcdata->tier != 2) {
	    if (ch->level < skill_table[gsn_trip].skill_level[ch->class]) {
		send_to_char("Tripping?  What's that?\n\r", ch);
		return;
	    }
	} else
	    if ((ch->level < skill_table[gsn_trip].skill_level[ch->class])
		&& (ch->level <
		    skill_table[gsn_trip].skill_level[ch->clasb])) {
	    send_to_char("Tripping?  What's that?\n\r", ch);
	    return;
	}
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("But you aren't fighting anyone!\n\r", ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_FLYING)) {
	act("{h$S feet aren't on the ground.{x", ch, NULL, victim,
	    TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING) {
	act("{h$N is already down.{c", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    if (victim == ch) {
	send_to_char("{hYou fall flat on your face!{x\n\r", ch);
	WAIT_STATE(ch, 2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!{x", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if ((ch->fighting == NULL)
	&& (!IS_NPC(ch))
	&& (!IS_NPC(victim))) {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 10;	/* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= get_curr_stat(victim, STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE)) chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance) {
	int dam;

	dam = number_range(2, 2 + 2 * victim->size);

	act("$n trips you and you go down!{x", ch, NULL, victim,
	    TO_VICT);
	act("{hYou trip $N and $N goes down!{x", ch, NULL, victim,
	    TO_CHAR);
	act("$n trips $N, sending $M to the ground.{x", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(ch, gsn_trip, TRUE, 1);

	DAZE_STATE(victim, 2 * PULSE_VIOLENCE);
	WAIT_STATE(ch, skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch, victim, dam, gsn_trip, DAM_BASH, TRUE);
    } else {
	damage(ch, victim, 0, gsn_trip, DAM_BASH, TRUE);
	WAIT_STATE(ch, skill_table[gsn_trip].beats * 2 / 3);
	check_improve(ch, gsn_trip, FALSE, 1);
    }
}



void do_kill(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Kill whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    if (victim == ch) {
	send_to_char("{hYou hit yourself.  {z{COuch!{x\n\r", ch);
	multi_hit(ch, ch, TYPE_UNDEFINED);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (!IS_NPC(victim)) {
	/*
	   if ( !IS_SET(victim->act, PLR_TWIT) )
	   {
	   send_to_char( "You must MURDER a player.\n\r", ch );
	   return;
	   }
	 */
    }
    if (victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->position == POS_FIGHTING) {
	send_to_char("You do the best you can!\n\r", ch);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }

/* Autostancing - Loki */
    if (IS_SET(victim->extra, dd)) {
	if (victim->stance[11] == STANCE_SERPENT)
	    do_stance(victim, "serpent");
	else if (victim->stance[11] == STANCE_CRANE)
	    do_stance(victim, "crane");
	else if (victim->stance[11] == STANCE_CRAB)
	    do_stance(victim, "crab");
	else if (victim->stance[11] == STANCE_MONGOOSE)
	    do_stance(victim, "mongoose");
	else if (victim->stance[11] == STANCE_BULL)
	    do_stance(victim, "bull");
	else if (victim->stance[11] == STANCE_MANTIS)
	    do_stance(victim, "mantis");
	else if (victim->stance[11] == STANCE_DRAGON)
	    do_stance(victim, "dragon");
	else if (victim->stance[11] == STANCE_TIGER)
	    do_stance(victim, "tiger");
	else if (victim->stance[11] == STANCE_MONKEY)
	    do_stance(victim, "monkey");
	else if (victim->stance[11] == STANCE_SWALLOW)
	    do_stance(victim, "swallow");
    }

/* Autostancing - Loki */
    if (IS_SET(ch->extra, dd)) {
	if (ch->stance[11] == STANCE_SERPENT)
	    do_stance(ch, "serpent");
	else if (ch->stance[11] == STANCE_CRANE)
	    do_stance(ch, "crane");
	else if (ch->stance[11] == STANCE_CRAB)
	    do_stance(ch, "crab");
	else if (ch->stance[11] == STANCE_MONGOOSE)
	    do_stance(ch, "mongoose");
	else if (ch->stance[11] == STANCE_BULL)
	    do_stance(ch, "bull");
	else if (ch->stance[11] == STANCE_MANTIS)
	    do_stance(ch, "mantis");
	else if (ch->stance[11] == STANCE_DRAGON)
	    do_stance(ch, "dragon");
	else if (ch->stance[11] == STANCE_TIGER)
	    do_stance(ch, "tiger");
	else if (ch->stance[11] == STANCE_MONKEY)
	    do_stance(ch, "monkey");
	else if (ch->stance[11] == STANCE_SWALLOW)
	    do_stance(ch, "swallow");
    }
    if ((ch->fighting == NULL)
	&& (!IS_NPC(ch))
	&& (!IS_NPC(victim))) {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
    multi_hit(ch, victim, TYPE_UNDEFINED);
    return;
}

void do_mock(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Mock hit whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    if (is_safe_mock(ch, victim))
	return;

    if (victim->fighting != NULL) {
	send_to_char("{gThis player is busy at the moment.{x\n\r", ch);
	return;
    }

    if (ch->position == POS_FIGHTING) {
	send_to_char("{gYou've already got your hands full!{x\n\r", ch);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    one_hit_mock(ch, victim, TYPE_UNDEFINED, FALSE);

    return;
}

void do_murde(CHAR_DATA * ch, char *argument)
{
    send_to_char("If you want to {RMURDER{x, spell it out.\n\r", ch);
    return;
}



void do_murder(CHAR_DATA * ch, char *argument)
{
    char buf[MSL];
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Murder whom?\n\r", ch);
	return;
    }

    if (IS_NPC(ch))
	return;

    if (IS_AFFECTED(ch, AFF_CHARM))
	return;

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("Suicide is a mortal sin.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->position == POS_FIGHTING) {
	send_to_char("You do the best you can!\n\r", ch);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }

    if (IS_SET(victim->exbit1_flags, RECRUIT)
	|| IS_SET(victim->exbit1_flags, PK_VETERAN)
	|| IS_SET(victim->exbit1_flags, PK_LAWFUL)
	|| IS_SET(victim->exbit1_flags, PK_KILLER)
	|| IS_SET(ch->exbit1_flags, RECRUIT)
	|| IS_SET(ch->exbit1_flags, PK_VETERAN)
	|| IS_SET(ch->exbit1_flags, PK_LAWFUL)
	|| IS_SET(ch->exbit1_flags, PK_KILLER) ) {

	if (victim->fighting != NULL
	    && IS_SET(ch->exbit1_flags, PK_LAWFUL)) {
	    send_to_char("So thats the way you like to play.\n\r", ch);
	    REMOVE_BIT(ch->exbit1_flags, PK_LAWFUL);
	    SET_BIT(ch->exbit1_flags, PK_KILLER);
	}
	    else if (victim->fighting != NULL
		     && IS_SET(ch->exbit1_flags, PK_KILLER)) {
	    send_to_char("You really deserve this.\n\r", ch);
	    REMOVE_BIT(ch->exbit1_flags, PK_KILLER);
	    SET_BIT(ch->exbit1_flags, PK_KILLER2);
	}
	// send_to_char("Have mercy on them.\n\r", ch);
    }

/* Autostancing - Loki */
    if (IS_SET(victim->extra, dd)) {
	if (victim->stance[11] == STANCE_SERPENT)
	    do_stance(victim, "serpent");
	else if (victim->stance[11] == STANCE_CRANE)
	    do_stance(victim, "crane");
	else if (victim->stance[11] == STANCE_CRAB)
	    do_stance(victim, "crab");
	else if (victim->stance[11] == STANCE_MONGOOSE)
	    do_stance(victim, "mongoose");
	else if (victim->stance[11] == STANCE_BULL)
	    do_stance(victim, "bull");
	else if (victim->stance[11] == STANCE_MANTIS)
	    do_stance(victim, "mantis");
	else if (victim->stance[11] == STANCE_DRAGON)
	    do_stance(victim, "dragon");
	else if (victim->stance[11] == STANCE_TIGER)
	    do_stance(victim, "tiger");
	else if (victim->stance[11] == STANCE_MONKEY)
	    do_stance(victim, "monkey");
	else if (victim->stance[11] == STANCE_SWALLOW)
	    do_stance(victim, "swallow");
    }

/* Autostancing - Loki */
    if (IS_SET(ch->extra, dd)) {
	if (ch->stance[11] == STANCE_SERPENT)
	    do_stance(ch, "serpent");
	else if (ch->stance[11] == STANCE_CRANE)
	    do_stance(ch, "crane");
	else if (ch->stance[11] == STANCE_CRAB)
	    do_stance(ch, "crab");
	else if (ch->stance[11] == STANCE_MONGOOSE)
	    do_stance(ch, "mongoose");
	else if (ch->stance[11] == STANCE_BULL)
	    do_stance(ch, "bull");
	else if (ch->stance[11] == STANCE_MANTIS)
	    do_stance(ch, "mantis");
	else if (ch->stance[11] == STANCE_DRAGON)
	    do_stance(ch, "dragon");
	else if (ch->stance[11] == STANCE_TIGER)
	    do_stance(ch, "tiger");
	else if (ch->stance[11] == STANCE_MONKEY)
	    do_stance(ch, "monkey");
	else if (ch->stance[11] == STANCE_SWALLOW)
	    do_stance(ch, "swallow");
    }
    if ((ch->fighting == NULL)
	&& (!IS_NPC(ch))
	&& (!IS_NPC(victim))) {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
    if (IS_NPC(ch))
	sprintf(buf, "Help! I am being attacked by %s!", ch->short_descr);
    else
	sprintf(buf, "Help!  I am being attacked by %s!", ch->name);
    do_yell(victim, buf);
    multi_hit(ch, victim, TYPE_UNDEFINED);
    return;
}



void do_backstab(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Backstab whom?\n\r", ch);
	return;
    }

    if (ch->fighting != NULL) {
	send_to_char("{hYou're facing the wrong end.{x\n\r", ch);
	return;
    }

    else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("How can you sneak up on yourself?\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
	send_to_char
	    ("{hYou need to wield a primary weapon to backstab.{x\n\r",
	     ch);
	return;
    }

    if (victim->hit < victim->max_hit / 3) {
	act("$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }

    if (IS_SET(victim->exbit1_flags, RECRUIT)
	|| IS_SET(victim->exbit1_flags, PK_VETERAN)
	|| IS_SET(victim->exbit1_flags, PK_LAWFUL)
	|| IS_SET(victim->exbit1_flags, PK_KILLER)
	|| IS_SET(ch->exbit1_flags, RECRUIT)
	|| IS_SET(ch->exbit1_flags, PK_VETERAN)
	|| IS_SET(ch->exbit1_flags, PK_LAWFUL)
	|| IS_SET(ch->exbit1_flags, PK_KILLER) ) {
	if (victim->fighting != NULL
	    && IS_SET(ch->exbit1_flags, PK_LAWFUL)) {
	    send_to_char("So thats the way you like to play.\n\r", ch);
	    REMOVE_BIT(ch->exbit1_flags, PK_LAWFUL);
	    SET_BIT(ch->exbit1_flags, PK_KILLER);
	}
	    else if (victim->fighting != NULL
		     && IS_SET(ch->exbit1_flags, PK_KILLER)) {
	    send_to_char("You really deserve this.\n\r", ch);
	    REMOVE_BIT(ch->exbit1_flags, PK_KILLER);
	    SET_BIT(ch->exbit1_flags, PK_KILLER2);
	}
	// send_to_char("Have mercy on them.\n\r", ch);
    }

/* Autostancing - Loki */
    if (IS_SET(victim->extra, dd)) {
	if (victim->stance[11] == STANCE_SERPENT)
	    do_stance(victim, "serpent");
	else if (victim->stance[11] == STANCE_CRANE)
	    do_stance(victim, "crane");
	else if (victim->stance[11] == STANCE_CRAB)
	    do_stance(victim, "crab");
	else if (victim->stance[11] == STANCE_MONGOOSE)
	    do_stance(victim, "mongoose");
	else if (victim->stance[11] == STANCE_BULL)
	    do_stance(victim, "bull");
	else if (victim->stance[11] == STANCE_MANTIS)
	    do_stance(victim, "mantis");
	else if (victim->stance[11] == STANCE_DRAGON)
	    do_stance(victim, "dragon");
	else if (victim->stance[11] == STANCE_TIGER)
	    do_stance(victim, "tiger");
	else if (victim->stance[11] == STANCE_MONKEY)
	    do_stance(victim, "monkey");
	else if (victim->stance[11] == STANCE_SWALLOW)
	    do_stance(victim, "swallow");
    }

/* Autostancing - Loki */
    if (IS_SET(ch->extra, dd)) {
	if (ch->stance[11] == STANCE_SERPENT)
	    do_stance(ch, "serpent");
	else if (ch->stance[11] == STANCE_CRANE)
	    do_stance(ch, "crane");
	else if (ch->stance[11] == STANCE_CRAB)
	    do_stance(ch, "crab");
	else if (ch->stance[11] == STANCE_MONGOOSE)
	    do_stance(ch, "mongoose");
	else if (ch->stance[11] == STANCE_BULL)
	    do_stance(ch, "bull");
	else if (ch->stance[11] == STANCE_MANTIS)
	    do_stance(ch, "mantis");
	else if (ch->stance[11] == STANCE_DRAGON)
	    do_stance(ch, "dragon");
	else if (ch->stance[11] == STANCE_TIGER)
	    do_stance(ch, "tiger");
	else if (ch->stance[11] == STANCE_MONKEY)
	    do_stance(ch, "monkey");
	else if (ch->stance[11] == STANCE_SWALLOW)
	    do_stance(ch, "swallow");
    }
    if ((ch->fighting == NULL)
	&& (!IS_NPC(ch))
	&& (!IS_NPC(victim))) {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    if (get_skill(ch, gsn_backstab) == 0 && !IS_AWAKE(victim)) {
	WAIT_STATE(ch, skill_table[gsn_backstab].beats);
	multi_hit(ch, victim, gsn_backstab);
	if (!IS_NPC(ch))
	    do_mod_favor(ch, 1);
	return;
    }
    if (ch->shadow && (ch->shadowing == victim)) {
	WAIT_STATE(ch, skill_table[gsn_backstab].beats);
	multi_hit(ch, victim, gsn_backstab);
	if (!IS_NPC(ch))
	    do_mod_favor(ch, 1);
	return;
    }
    WAIT_STATE(ch, skill_table[gsn_backstab].beats);
    if (number_percent() < get_skill(ch, gsn_backstab)
	|| (get_skill(ch, gsn_backstab) >= 1 && !IS_AWAKE(victim))) {
	check_improve(ch, gsn_backstab, TRUE, 1);
	multi_hit(ch, victim, gsn_backstab);
	if (!IS_NPC(ch))
	    do_mod_favor(ch, 1);
    } else {
	check_improve(ch, gsn_backstab, FALSE, 1);
	damage(ch, victim, 0, gsn_backstab, DAM_NONE, TRUE);
    }

    return;
}

void do_circle(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance;

    if ((chance = get_skill(ch, gsn_circle)) == 0) {
	send_to_char("Circle?  What's that?\n\r", ch);
	return;
    }

    if (!IS_NPC(ch)) {
	if (ch->pcdata->tier != 2) {
	    if (ch->level < skill_table[gsn_circle].skill_level[ch->class]) {
		send_to_char("Circle?  What's that?\n\r", ch);
		return;
	    }
	} else
	    if (
		(ch->level <
		 skill_table[gsn_circle].skill_level[ch->class])
		&& (ch->level <
		    skill_table[gsn_circle].skill_level[ch->clasb])) {
	    send_to_char("Circle?  What's that?\n\r", ch);
	    return;
	}
    }

    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }

    if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
	send_to_char("You need to wield a primary weapon to circle.\n\r",
		     ch);
	return;
    }

    if (victim->hit < victim->max_hit / 6) {
	act("$N is hurt and suspicious ... you can't sneak around.",
	    ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }

    if (!can_see(ch, victim)) {
	send_to_char("You stumble blindly into a wall.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, skill_table[gsn_circle].beats);
    if (number_percent() < get_skill(ch, gsn_circle)
	|| (get_skill(ch, gsn_circle) >= 2 && !IS_AWAKE(victim))) {
	check_improve(ch, gsn_circle, TRUE, 1);
	act("$n circles around behind you.{x", ch, NULL, victim,
	    TO_VICT);
	act("{hYou circle around $N.{x", ch, NULL, victim, TO_CHAR);
	act("$n circles around behind $N.{x", ch, NULL, victim,
	    TO_NOTVICT);
	multi_hit(ch, victim, gsn_circle);
	if (!IS_NPC(ch))
	    do_mod_favor(ch, 1);
    } else {
	check_improve(ch, gsn_circle, FALSE, 1);
	act("$n tries to circle around you.{x", ch, NULL, victim,
	    TO_VICT);
	act("{h$N circles with you.{x", ch, NULL, victim, TO_CHAR);
	act("$n tries to circle around $N.{x", ch, NULL, victim,
	    TO_NOTVICT);
	damage(ch, victim, 0, gsn_circle, DAM_NONE, TRUE);
    }

    return;
}

void do_feed(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    int chance;

    if ((chance = get_skill(ch, gsn_feed)) == 0) {
	send_to_char("Feed?  What's that?\n\r", ch);
	return;
    }

    if (!IS_NPC(ch)) {
	if (ch->pcdata->tier != 2) {
	    if (ch->level < skill_table[gsn_feed].skill_level[ch->class]) {
		send_to_char("Feed?  What's that?\n\r", ch);
		return;
	    }
	} else
	    if ((ch->level < skill_table[gsn_feed].skill_level[ch->class])
		&& (ch->level <
		    skill_table[gsn_feed].skill_level[ch->clasb])) {
	    send_to_char("Feed?  What's that?\n\r", ch);
	    return;
	}
    }

    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }

    if (victim->hit < victim->max_hit / 6) {
	act("$N is hurt and suspicious ... you can't get close enough.",
	    ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, skill_table[gsn_feed].beats);
    if (number_percent() < get_skill(ch, gsn_feed) / 3
	|| (get_skill(ch, gsn_feed) >= 2 && !IS_AWAKE(victim))) {
	int dam;

	dam = number_range((((ch->level / 2) + (victim->level / 2)) / 2.5), (((ch->level / 2) + (victim->level / 2)) / 2.5) * 2.5);

	check_improve(ch, gsn_feed, TRUE, 1);
	act("$n bites you.{x", ch, NULL, victim, TO_VICT);
	act("{hYou bite $N.{x", ch, NULL, victim, TO_CHAR);
	act("$n bites $N.{x", ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, gsn_feed, DAM_NEGATIVE, TRUE);
    } else {
	check_improve(ch, gsn_feed, FALSE, 1);
	act("$n tries to bite you, but hits only air.{x", ch, NULL,
	    victim, TO_VICT);
	act("{hYou chomp a mouthfull of air.{x", ch, NULL, victim,
	    TO_CHAR);
	act("$n tries to bite $N.{x", ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, 0, gsn_feed, DAM_NEGATIVE, TRUE);
    }

    return;
}


void do_flee(CHAR_DATA * ch, char *argument)
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ((victim = ch->fighting) == NULL) {
	if (ch->position == POS_FIGHTING)
	    ch->position = POS_STANDING;
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }

    was_in = ch->in_room;
    for (attempt = 0; attempt < 6; attempt++) {
	EXIT_DATA *pexit;
	int door;
	int gdoor;

	door = number_door();
	if (((pexit = was_in->exit[door + 6]) != NULL)
	    && (ch->alignment < 0))
	    gdoor = door + 6;
	else
	    gdoor = door;

	if (((pexit = was_in->exit[gdoor]) == NULL)
	    || (IS_SET(ch->in_room->room_flags, ROOM_CLAN_ENT))) {
	    OBJ_DATA *portal;

	    portal = get_obj_exit(dir_name[door], was_in->contents);
	    if (portal == NULL) {
		continue;
	    }
	} else if ((pexit = was_in->exit[gdoor]) == 0
		   || pexit->u1.to_room == NULL
		   || IS_SET(pexit->exit_info, EX_CLOSED)
		   || number_range(0, ch->daze) != 0 || (IS_NPC(ch)
							 && IS_SET(pexit->
								   u1.
								   to_room->
								   room_flags,
								   ROOM_NO_MOB)))
		continue;

	move_char(ch, door, FALSE, FALSE);
	if ((now_in = ch->in_room) == was_in) {
	    continue;
	}

	ch->in_room = was_in;
	act("$n has {Yfled{x!", ch, NULL, NULL, TO_ROOM);
	if (!IS_NPC(ch)) {
	    if (!IS_NPC(victim)) {
		if (ch->attacker == FALSE)
		    ch->pcdata->dflee++;
		else
		    ch->pcdata->aflee++;
	    }
	    send_to_char("{BYou {Yflee{B from combat!{x\n\r", ch);
	    if (((ch->class == 2) || (ch->class == (MCLT_1) + 1))
		&& (number_percent() < 3 * (ch->level / 2))) {
		if (IS_NPC(victim) || ch->attacker == FALSE) {
		    send_to_char("You {Ysnuck away{x safely.\n\r", ch);
		} else {
		    send_to_char
			("You feel something singe your butt on the way out.\n\r",
			 ch);
		    act
			("$n is nearly {Yzapped{x in the butt by a lightning bolt from above!",
			 ch, NULL, NULL, TO_ROOM);
		    ch->hit -= (ch->hit / 8);
		}
	    } else {
		if (!IS_NPC(victim) && ch->attacker == TRUE) {
		    send_to_char
			("The {RWrath of Thoth {YZAPS{x your butt on the way out!\n\r",
			 ch);
		    act
			("$n is {Yzapped{x in the butt by a lightning bolt from above!",
			 ch, NULL, NULL, TO_ROOM);
		    ch->hit -= (ch->hit / 4);
		}
		if (!IS_NPC(ch) && !IS_SET(ch->act, PLR_LQUEST)) {
		    send_to_char("You lost 10 exp.\n\r", ch);
		    gain_exp(ch, -10);
		}
	    }
	}
	ch->in_room = now_in;
	stop_fighting(ch, TRUE);
	do_mod_favor(ch, 2);
	return;
    }

    send_to_char("{z{CPANIC!{x{B You couldn't escape!{x\n\r", ch);
    return;
}



void do_rescue(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
	send_to_char("Rescue whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("What about {Yfleeing{x instead?\n\r", ch);
	return;
    }

    if (!IS_NPC(ch) && IS_NPC(victim)) {
	send_to_char("Doesn't need your help!\n\r", ch);
	return;
    }

    if (ch->fighting == victim) {
	send_to_char("Too late.\n\r", ch);
	return;
    }

    if ((fch = victim->fighting) == NULL) {
	send_to_char("That person is not fighting right now.\n\r", ch);
	return;
    }

    if (IS_NPC(fch) && !is_same_group(ch, victim)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (ch->spirit) {
	send_to_char("That's tough to do without flesh.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, skill_table[gsn_rescue].beats);
    if (number_percent() > get_skill(ch, gsn_rescue)) {
	send_to_char("You fail the rescue.\n\r", ch);
	check_improve(ch, gsn_rescue, FALSE, 1);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    act("{yYou rescue $N!{x", ch, NULL, victim, TO_CHAR);
    act("{y$n rescues you!{x", ch, NULL, victim, TO_VICT);
    act("{y$n rescues $N!{x", ch, NULL, victim, TO_NOTVICT);
    do_mod_favor(ch, 6);
    check_improve(ch, gsn_rescue, TRUE, 1);

    stop_fighting(fch, FALSE);
    stop_fighting(victim, FALSE);

    set_fighting(ch, fch);
    set_fighting(fch, ch);
    return;
}




void do_stance(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    int selection;
    argument = one_argument(argument, arg);
    if (arg[0] == '\0') {
	if (ch->stance[0] == -1) {
	    ch->stance[0] = 0;
	    send_to_char("You drop into a street fighting stance.\n\r", ch);
	    act("$n drops into a street fighting stance.", ch, NULL, NULL,
		TO_ROOM);} else {
	    ch->stance[0] = -1;
	    send_to_char("You relax from your street fighting stance.\n\r",
			 ch);
	    act("$n relaxes from $s street fighting stance.", ch, NULL,
		NULL, TO_ROOM);
	}
	return;
    }
    if (!str_cmp(arg, "none")) {
	selection = STANCE_NONE;
	send_to_char("You drop into a boxing fighting stance.\n\r", ch);
	act("$n drops into a boxing fighting stance.", ch, NULL, NULL,
	    TO_ROOM);} else if (!str_cmp(arg, "serpent")) {
	selection = STANCE_SERPENT;
	send_to_char("You take up the serpent fighting stance.\n\r", ch);
	act("$n takes up the serpent fighting stance.", ch, NULL, NULL,
	    TO_ROOM);} else if (!str_cmp(arg, "crane")) {
	selection = STANCE_CRANE;
	send_to_char("You take up the crane fighting stance.\n\r", ch);
	act("$n takes up the crane fighting stance.", ch, NULL, NULL,
	    TO_ROOM);} else if (!str_cmp(arg, "crab")) {
	selection = STANCE_CRAB;
	send_to_char("You take up the crab fighting stance.\n\r", ch);
	act("$n takes up the crab fighting stance. ", ch, NULL, NULL,
	    TO_ROOM);} else if (!str_cmp(arg, "mongoose")) {
	selection = STANCE_MONGOOSE;
	send_to_char("You take up the mongoose fighting stance.\n\r", ch);
	act("$n takes up the mongoose fighting stance. ", ch, NULL, NULL,
	    TO_ROOM);} else if (!str_cmp(arg, "bull")) {
	selection = STANCE_BULL;
	send_to_char("You take up the bull fighting stance.\n\r", ch);
	act("$n takes up the bull fighting stance. ", ch, NULL, NULL,
	    TO_ROOM);} else {
	if (!str_cmp(arg, "mantis") && ch->stance[STANCE_CRANE] >= 200
	    && ch->stance[STANCE_SERPENT] >= 200) {
	    selection = STANCE_MANTIS;
	    send_to_char("You take up the mantis fighting stance.\n\r", ch);
	    act("$n takes up the mantis fighting stance.", ch, NULL, NULL,
		TO_ROOM);
	} else if (!str_cmp(arg, "dragon") && ch->stance[STANCE_BULL] >= 200
		   && ch->stance[STANCE_CRAB] >= 200) {
	    selection = STANCE_DRAGON;
	    send_to_char("You take up the dragon fighting stance.\n\r", ch);
	    act("$n takes up the dragon fighting stance.", ch, NULL, NULL,
		TO_ROOM);
	} else if (!str_cmp(arg, "tiger") && ch->stance[STANCE_BULL] >= 200
		   && ch->stance[STANCE_SERPENT] >= 200) {
	    selection = STANCE_TIGER;
	    send_to_char("You take up the tiger fighting stance.\n\r", ch);
	    act("$n takes up the tiger fighting stance.", ch, NULL, NULL,
		TO_ROOM);
	} else if (!str_cmp(arg, "monkey") && ch->stance[STANCE_CRANE] >= 200
		   && ch->stance[STANCE_MONGOOSE] >= 200) {
	    selection = STANCE_MONKEY;
	    send_to_char("You take up the monkey fighting stance.\n\r", ch);
	    act("$n takes up the monkey fighting stance.", ch, NULL, NULL,
		TO_ROOM);
	} else if (!str_cmp(arg, "swallow") && ch->stance[STANCE_CRAB] >= 200
		   && ch->stance[STANCE_MONGOOSE] >= 200) {
	    selection = STANCE_SWALLOW;
	    send_to_char("You take up the swallow fighting stance.\n\r",
			 ch);
	    act("$n takes up the swallow fighting stance.", ch, NULL, NULL,
		TO_ROOM);
	} else {
	    send_to_char("Syntax is: stance <stance>.\n\r", ch);
	    send_to_char
		("Stance being one of: None, Serpent, Crane, Crab, Mongoose, Bull.\n\r",
		 ch);
	    return;
	}
    }
    ch->stance[0] = selection;
    return;
}

void do_autostance(CHAR_DATA * ch, char *argument)
{
    char arg[MIL];
    int selection;

    one_argument(argument, arg);

    if (IS_NPC(ch))
	return;

    if (!str_cmp(arg, "none")) {
	selection = STANCE_NONE;
	send_to_char("You're autostance has been removed.\n\r", ch);
	REMOVE_BIT(ch->extra, dd);
    } else if (!str_cmp(arg, "serpent")) {
	selection = STANCE_SERPENT;
	send_to_char("Serpent stance set.\n\r", ch);
    } else if (!str_cmp(arg, "crane")) {
	selection = STANCE_CRANE;
	send_to_char("Crane stance set.\n\r", ch);
    } else if (!str_cmp(arg, "crab")) {
	selection = STANCE_CRAB;
	send_to_char("Crab stance set.\n\r", ch);
    } else if (!str_cmp(arg, "mongoose")) {
	selection = STANCE_MONGOOSE;
	send_to_char("Mongoose stance set.\n\r", ch);
    } else if (!str_cmp(arg, "bull")) {
	selection = STANCE_BULL;
	send_to_char("Bull stance set.\n\r", ch);
    } else {
	if (!str_cmp(arg, "mantis") && ch->stance[STANCE_CRANE] >= 200 &&
	    ch->stance[STANCE_SERPENT] >= 200) {
	    selection = STANCE_MANTIS;
	    send_to_char("Mantis stance set.\n\r", ch);
	}
	    else if (!str_cmp(arg, "dragon")
		     && ch->stance[STANCE_BULL] >= 200
		     && ch->stance[STANCE_CRAB] >= 200) {
	    selection = STANCE_DRAGON;
	    send_to_char("Dragon stance set.\n\r", ch);
	}
	    else if (!str_cmp(arg, "tiger") && ch->stance[STANCE_BULL] >= 200
		     && ch->stance[STANCE_SERPENT] >= 200) {
	    selection = STANCE_TIGER;
	    send_to_char("Tiger stance set.\n\r", ch);
	}
	    else if (!str_cmp(arg, "monkey")
		     && ch->stance[STANCE_CRANE] >= 200
		     && ch->stance[STANCE_MONGOOSE] >= 200) {
	    selection = STANCE_MONKEY;
	    send_to_char("Monkey stance set.\n\r", ch);
	}
	    else if (!str_cmp(arg, "swallow")
		     && ch->stance[STANCE_CRAB] >= 200
		     && ch->stance[STANCE_MONGOOSE] >= 200) {
	    selection = STANCE_SWALLOW;
	    send_to_char("Swallow  stance set.\n\r", ch);
	} else {
	    send_to_char("Syntax is: autostance <stance>.\n\r", ch);
	    send_to_char
		("Autotance being one of: None, Serpent, Crane, Crab, Mongoose, Bull.\n\r",
		 ch);
	    send_to_char
		("If you know them, they are:  Mantis, Dragon, Tiger, Monkey, Swallow.\n\r",
		 ch);
	    return;
	}
    }
    ch->stance[11] = selection;
    SET_BIT(ch->extra, dd);
    return;
}

int dambonus(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int stance)
{
    if (dam < 1)
	return 0;
    if (stance < 1)
	return dam;
    if (!IS_NPC(ch) && !can_counter(victim)) {
	if (IS_STANCE(ch, STANCE_MONKEY)) {
	    int mindam = dam * 0.25;
	    dam *= (ch->stance[STANCE_MONKEY] + 1) / 200;
	    if (dam < mindam)
		dam = mindam;
	} else if (IS_STANCE(ch, STANCE_BULL) && ch->stance[STANCE_BULL] > 100)
	    dam += dam * (ch->stance[STANCE_BULL] / 100);
	else if (IS_STANCE(ch, STANCE_DRAGON)
		 && ch->stance[STANCE_DRAGON] > 100) dam +=
		dam * (ch->stance[STANCE_DRAGON] / 100);
	else if (IS_STANCE(ch, STANCE_TIGER) && ch->stance[STANCE_TIGER] > 100)
	    dam += dam * (ch->stance[STANCE_TIGER] / 100);
	else if (ch->stance[0] > 0 && ch->stance[stance] < 100)
	    dam *= 0.5;
    }
    if (!IS_NPC(victim) && !can_counter(ch)) {
	if (IS_STANCE(victim, STANCE_CRAB)
	    && victim->stance[STANCE_CRAB] >
	    100) dam /= victim->stance[STANCE_CRAB] / 100;
	else if (IS_STANCE(victim, STANCE_DRAGON)
		 && victim->stance[STANCE_DRAGON] > 100)
	    dam /= victim->stance[STANCE_DRAGON] / 100;
	else if (IS_STANCE(victim, STANCE_SWALLOW)
		 && victim->stance[STANCE_SWALLOW] > 100)
	    dam /= victim->stance[STANCE_SWALLOW] / 100;
    }
    return dam;
}

bool can_counter(CHAR_DATA * ch)
{
    if (IS_STANCE(ch, STANCE_MONKEY))
	return TRUE;
    return FALSE;
}

bool can_bypass(CHAR_DATA * ch, CHAR_DATA * victim)
{
    if (IS_STANCE(ch, STANCE_SERPENT))
	return TRUE;
    else if (IS_STANCE(ch, STANCE_MANTIS))
	return TRUE;
    else if (IS_STANCE(ch, STANCE_TIGER))
	return TRUE;
    return FALSE;
}

void improve_stance(CHAR_DATA * ch)
{
    char buf[MIL];
    char bufskill[25];
    char stancename[10];
    int dice1;
    int dice2;
    int stance;

    dice1 = number_percent();
    dice2 = number_percent();
    stance = ch->stance[0];
    if (stance < 1 || stance > 10)
	return;
    if (ch->stance[stance] >= 200) {
	ch->stance[stance] = 200;
	return;
    }
    if ((dice1 > ch->stance[stance] && dice2 > ch->stance[stance])
	|| (dice1 == 100 || dice2 == 100))
	ch->stance[stance] += 1;
    else
	return;
    if (stance == ch->stance[stance])
	return;
    if (ch->stance[stance] == 1)
	sprintf(bufskill, "an apprentice of");
    else if (ch->stance[stance] == 26)
	sprintf(bufskill, "a trainee of");
    else if (ch->stance[stance] == 51)
	sprintf(bufskill, "a student of");
    else if (ch->stance[stance] == 76)
	sprintf(bufskill, "fairly experienced in");
    else if (ch->stance[stance] == 101)
	sprintf(bufskill, "well trained in");
    else if (ch->stance[stance] == 126)
	sprintf(bufskill, "highly skilled in");
    else if (ch->stance[stance] == 151)
	sprintf(bufskill, "an expert of");
    else if (ch->stance[stance] == 176)
	sprintf(bufskill, "a master of");
    else if (ch->stance[stance] == 200)
	sprintf(bufskill, "a grand master of");
    else
	return;

    if (stance == STANCE_SERPENT)
	sprintf(stancename, "serpent");
    else if (stance == STANCE_CRANE)
	sprintf(stancename, "crane");
    else if (stance == STANCE_CRAB)
	sprintf(stancename, "crab");
    else if (stance == STANCE_MONGOOSE)
	sprintf(stancename, "mongoose");
    else if (stance == STANCE_BULL)
	sprintf(stancename, "bull");
    else if (stance == STANCE_MANTIS)
	sprintf(stancename, "mantis");
    else if (stance == STANCE_DRAGON)
	sprintf(stancename, "dragon");
    else if (stance == STANCE_TIGER)
	sprintf(stancename, "tiger");
    else if (stance == STANCE_MONKEY)
	sprintf(stancename, "monkey");
    else if (stance == STANCE_SWALLOW)
	sprintf(stancename, "swallow");

    else
	return;
    sprintf(buf, "You are now %s the %s stance.\n\r", bufskill, stancename);
    send_to_char(buf, ch);
    return;
}

void special_move(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int dam = number_range(20, 40);

    if (dam < 20)
	dam = 20;

    switch (number_range(1, 7)) {
    default:
	return;
    case 1:
	act
	    ("You pull your hands into your waist then snap them into $N's stomach.",
	     ch, NULL, victim, TO_CHAR);
	act
	    ("$n pulls $s hands into $s waist then snaps them into your stomach.",
	     ch, NULL, victim, TO_VICT);
	act
	    ("$n pulls $s hands into $s waist then snaps them into $N's stomach.",
	     ch, NULL, victim, TO_NOTVICT);
	if (victim == NULL || victim->position == POS_DEAD)
	    return;
	act
	    ("You double over in agony, and fall to the ground gasping for breath.",
	     victim, NULL, NULL, TO_CHAR);
	act
	    ("$n doubles over in agony, and falls to the ground gasping for breath.",
	     victim, NULL, NULL, TO_ROOM);
	stop_fighting(victim, TRUE);
	victim->position = POS_STUNNED;
	break;
    case 2:
	act("You spin in a low circle, catching $N behind $S ankle.", ch,
	    NULL, victim, TO_CHAR);
	act("$n spins in a low circle, catching you behind your ankle.",
	    ch, NULL, victim, TO_VICT);
	act("$n spins in a low circle, catching $N behind $S ankle.", ch,
	    NULL, victim, TO_NOTVICT);
	if (victim == NULL || victim->position == POS_DEAD)
	    return;
	act("You crash to the ground, stunned.", victim, NULL, NULL,
	    TO_CHAR);
	act("$n crashes to the ground, stunned.", victim, NULL, NULL,
	    TO_ROOM);
	stop_fighting(victim, TRUE);
	victim->position = POS_STUNNED;
	break;
    case 3:
	act("You roll between $N's legs and flip to your feet.", ch, NULL,
	    victim, TO_CHAR);
	act("$n rolls between your legs and flips to $s feet.", ch, NULL,
	    victim, TO_VICT);
	act("$n rolls between $N's legs and flips to $s feet.", ch, NULL,
	    victim, TO_NOTVICT);
	act
	    ("You spin around and smash your elbow into the back of $N's head.",
	     ch, NULL, victim, TO_CHAR);
	act
	    ("$n spins around and smashes $s elbow into the back of your head.",
	     ch, NULL, victim, TO_VICT);
	act
	    ("$n spins around and smashes $s elbow into the back of $N's head.",
	     ch, NULL, victim, TO_NOTVICT);
	if (victim == NULL || victim->position == POS_DEAD)
	    return;
	act("You fall to the ground, stunned.", victim, NULL, NULL,
	    TO_CHAR);
	act("$n falls to the ground, stunned.", victim, NULL, NULL,
	    TO_ROOM);
	stop_fighting(victim, TRUE);
	victim->position = POS_STUNNED;
	break;
    case 4:
	act("You somersault over $N's head and land lightly on your toes.",
	    ch, NULL, victim, TO_CHAR);
	act("$n somersaults over your head and lands lightly on $s toes.",
	    ch, NULL, victim, TO_VICT);
	act("$n somersaults over $N's head and lands lightly on $s toes.",
	    ch, NULL, victim, TO_NOTVICT);
	act
	    ("You roll back onto your shoulders and kick both feet into $N's back.",
	     ch, NULL, victim, TO_CHAR);
	act
	    ("$n rolls back onto $s shoulders and kicks both feet into your back.",
	     ch, NULL, victim, TO_VICT);
	act
	    ("$n rolls back onto $s shoulders and kicks both feet into $N's back.",
	     ch, NULL, victim, TO_NOTVICT);
	act("You fall to the ground, stunned.", victim, NULL, NULL,
	    TO_CHAR);
	act("$n falls to the ground, stunned.", victim, NULL, NULL,
	    TO_ROOM);
	act("You flip back up to your feet.", ch, NULL, NULL, TO_CHAR);
	act("$n flips back up to $s feet.", ch, NULL, NULL, TO_ROOM);
	stop_fighting(victim, TRUE);
	victim->position = POS_STUNNED;
	break;
    case 5:
	act("You grab $N by the waist and hoist $M above your head.", ch,
	    NULL, victim, TO_CHAR);
	act("$n grabs $N by the waist and hoists $M above $s head.", ch,
	    NULL, victim, TO_NOTVICT);
	act("$n grabs you by the waist and hoists you above $s head.", ch,
	    NULL, victim, TO_VICT);
	if (victim == NULL || victim->position == POS_DEAD)
	    return;
	act("You crash to the ground, stunned.", victim, NULL, NULL,
	    TO_CHAR);
	act("$n crashes to the ground, stunned.", victim, NULL, NULL,
	    TO_ROOM);
	stop_fighting(victim, TRUE);
	victim->position = POS_STUNNED;
	break;
    case 6:
	act("You grab $N by the head and slam $S face into your knee.", ch,
	    NULL, victim, TO_CHAR);
	act("$n grabs you by the head and slams your face into $s knee.",
	    ch, NULL, victim, TO_VICT);
	act("$n grabs $N by the head and slams $S face into $s knee.", ch,
	    NULL, victim, TO_NOTVICT);
	if (victim == NULL || victim->position == POS_DEAD)
	    return;
	act("You crash to the ground, stunned.", victim, NULL, NULL,
	    TO_CHAR);
	act("$n crashes to the ground, stunned.", victim, NULL, NULL,
	    TO_ROOM);
	act("You flip back up to your feet.", ch, NULL, NULL, TO_CHAR);
	act("$n flips back up to $s feet.", ch, NULL, NULL, TO_ROOM);
	stop_fighting(victim, TRUE);
	victim->position = POS_STUNNED;
	break;
    case 7:
	act
	    ("You duck under $N's attack and pound your fist into $S stomach.",
	     ch, NULL, victim, TO_CHAR);
	act
	    ("$n ducks under your attack and pounds $s fist into your stomach.",
	     ch, NULL, victim, TO_VICT);
	act
	    ("$n ducks under $N's attack and pounds $s fist into $N's stomach.",
	     ch, NULL, victim, TO_NOTVICT);
	if (victim == NULL || victim->position == POS_DEAD)
	    return;
	act("You double over in agony.", victim, NULL, NULL, TO_CHAR);
	act("$n doubles over in agony.", victim, NULL, NULL, TO_ROOM);
	if (victim == NULL || victim->position == POS_DEAD)
	    return;
	stop_fighting(victim, TRUE);
	victim->position = POS_STUNNED;
	break;
    }
    return;
}

void do_left_hook(CHAR_DATA * ch, char *argument)
{

    CHAR_DATA *victim;
    int dam;

    if (!IS_NPC(ch)
	&& ch->level <
	skill_table[gsn_left_hook].skill_level[ch->
					       class])
    {send_to_char("You better leave that to the experts.\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }
    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }
    dam = number_range(3, ch->level * 2);
    WAIT_STATE(ch, skill_table[gsn_left_hook].beats);
    if (get_skill(ch, gsn_left_hook) > number_percent()) {
	damage(ch, victim, number_range(dam, (ch->level * 2.2)),
	       gsn_left_hook, DAM_BASH, TRUE);
	SET_BIT(victim->position, POS_STUNNED);
	check_improve(ch, gsn_left_hook, TRUE, 1);
    } else {
	damage(ch, victim, 0, gsn_left_hook, DAM_BASH, TRUE);
	check_improve(ch, gsn_left_hook, FALSE, 1);
    }
    return;
}

void do_kidney_punch(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    int dam;
    if (!IS_NPC(ch)
	&& ch->level <
	skill_table[gsn_kidney_punch].skill_level[ch->
						  class])
    {send_to_char("You better leave that to the experts.\n\r", ch);
	return;
    }
    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }
    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }
    dam = number_range(3, ch->level * 1.9);
    WAIT_STATE(ch, skill_table[gsn_kidney_punch].beats);
    if (get_skill(ch, gsn_kidney_punch) > number_percent()) {
	damage(ch, victim, number_range(dam, (ch->level * 2.1)),
	       gsn_kidney_punch, DAM_BASH, TRUE);
	SET_BIT(victim->position, POS_STUNNED);
	check_improve(ch, gsn_kidney_punch, TRUE, 1);
    } else {
	damage(ch, victim, 0, gsn_kidney_punch, DAM_BASH, TRUE);
	check_improve(ch, gsn_kidney_punch, FALSE, 1);
    }
    return;
}
void do_right_cross(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    int dam;
    if (!IS_NPC(ch)
	&& ch->level <
	skill_table[gsn_right_cross].skill_level[ch->
						 class])
    {send_to_char("You better leave that to the experts.\n\r", ch);
	return;
    }
    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }
    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }
    dam = number_range(2, ch->level * 2.1);
    WAIT_STATE(ch, skill_table[gsn_right_cross].beats);
    if (get_skill(ch, gsn_right_cross) > number_percent()) {
	damage(ch, victim, number_range(dam, (ch->level * 2.3)),
	       gsn_right_cross, DAM_BASH, TRUE);
	check_improve(ch, gsn_right_cross, TRUE, 1);
    } else {
	damage(ch, victim, 0, gsn_right_cross, DAM_BASH, TRUE);
	check_improve(ch, gsn_right_cross, FALSE, 1);
    }
    return;
}

void do_critical_strike(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    int dam;
    if (!IS_NPC(ch)
	&& ch->level <
	skill_table[gsn_critical_strike].skill_level[ch->
						     class])
    {send_to_char("You better leave that to the experts.\n\r", ch);
	return;
    }
    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }
    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }
    dam = number_range(12, ch->level * 2.5);
    WAIT_STATE(ch, skill_table[gsn_critical_strike].beats);
    if (get_skill(ch, gsn_critical_strike) > number_percent()) {
	damage(ch, victim, number_range(dam, (ch->level * 3.7)),
	       gsn_critical_strike, DAM_BASH, TRUE);
	SET_BIT(victim->position, POS_MORTAL);
	check_improve(ch, gsn_critical_strike, TRUE, 1);
    } else {
	damage(ch, victim, 0, gsn_critical_strike, DAM_BASH, TRUE);
	check_improve(ch, gsn_critical_strike, FALSE, 1);
    }
    return;
}

void do_jab(CHAR_DATA * ch, char *argument)
{

    CHAR_DATA *victim;
    int dam;

    if (!IS_NPC(ch)
	&& ch->level <
	skill_table[gsn_jab].skill_level[ch->
					 class])
    {send_to_char("You better leave that to the experts.\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }
    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }
    dam = number_range(3, ch->level * 1);
    WAIT_STATE(ch, skill_table[gsn_jab].beats);
    if (get_skill(ch, gsn_jab) > number_percent()) {
	damage(ch, victim, number_range(dam, (ch->level * 1.2)), gsn_jab,
	       DAM_BASH, TRUE);
	check_improve(ch, gsn_jab, TRUE, 1);
    } else {
	damage(ch, victim, 0, gsn_jab, DAM_BASH, TRUE);
	check_improve(ch, gsn_jab, FALSE, 1);
    }
    return;
}


void do_uppercut(CHAR_DATA * ch, char *argument)
{

    CHAR_DATA *victim;
    int dam;

    if (!IS_NPC(ch)
	&& ch->level <
	skill_table[gsn_uppercut].skill_level[ch->
					      class])
    {send_to_char("You better leave that to the experts.\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }
    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }
    dam = number_range(3, ch->level * 2.3);
    WAIT_STATE(ch, skill_table[gsn_uppercut].beats);
    if (get_skill(ch, gsn_uppercut) > number_percent()) {
	damage(ch, victim, number_range(dam, (ch->level * 2.5)),
	       gsn_uppercut, DAM_BASH, TRUE);
	SET_BIT(victim->position, POS_STUNNED);
	check_improve(ch, gsn_uppercut, TRUE, 1);
    } else {
	damage(ch, victim, 0, gsn_uppercut, DAM_BASH, TRUE);
	check_improve(ch, gsn_uppercut, FALSE, 1);
    }
    return;
}

void check_arena(CHAR_DATA * ch, CHAR_DATA * victim)
{
    DESCRIPTOR_DATA *d;		/* needed for Arena bet checking */
    char buf[MSL];
    static int payoff;
    sprintf(buf, "^{gArena{x^ {b%s{x has {rdefeated{x {b%s{x!\n\r",
	    ch->name, victim->name);
    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING &&
	    (d->character != victim && d->character != ch)
	    && !IS_SET(d->character->comm, COMM_NOARENA)) {
	    send_to_char(buf, d->character);
	}
    }

    ch->pcdata->awins += 1;
    victim->pcdata->alosses += 1;
    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING) {
	    if (d->character->gladiator == ch) {

		if (IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGER)) {
		    float odd1 = cpo_stat;
		    payoff = d->character->pcdata->plr_wager * (odd1);
		}
		if (IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGED)) {
		    float odd2 = vpo_stat;
		    payoff = d->character->pcdata->plr_wager * (odd2);
		}
/*Since there is no ratio for a loss (you lose what you bet) this second part may be
  fubbing things up - going to comment it out for now*
*if (IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGER))
	{ 
		float odd1=cpo_stat;
		payoff = d->character->pcdata->plr_wager * ( odd1 ); 
	}
if (IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGED))
	{ 
		float odd2=vpo_stat;
		payoff = d->character->pcdata->plr_wager * ( odd2 ); 
	}*/

		sprintf(buf,
			"You {gwon{x! Your wager: {y%d{x, payoff: {y%d{x\n\r",
			d->character->pcdata->plr_wager, payoff);
		send_to_char(buf, d->character);
		d->character->gold += payoff;
/* reset the betting info */
		d->character->gladiator = NULL;
		d->character->pcdata->plr_wager = 0;
		payoff = 0;
	    }
	    if (d->character->gladiator != ch
		&& d->character->pcdata->plr_wager >= 1) {
		int tmp = 0;
		sprintf(buf, "You {rlost{x! Your wager: {y%d{x\n\r",
			d->character->pcdata->plr_wager);
		send_to_char(buf, d->character);
		if (d->character->pcdata->plr_wager > d->character->gold) {
		    tmp = d->character->pcdata->plr_wager / 100;
		    d->character->pcdata->plr_wager -= (tmp * 100);
		}
		if (tmp > 0)
		    d->character->platinum -= tmp;
		d->character->gold -= d->character->pcdata->plr_wager;
/* reset the betting info */
		d->character->gladiator = NULL;
		d->character->pcdata->plr_wager = 0;
	    }
	}
    }

/* now move both fighters out of arena and back
to the regular "world" be sure to define 
ROOM_VNUM_AWINNER and ROOM_VNUM_ALOSER */

    stop_fighting(victim, TRUE);
    char_from_room(victim);
    char_to_room(victim, get_room_index(ROOM_VNUM_ALOSER));
    victim->hit = victim->max_hit;
    victim->mana = victim->max_mana;
// affect_strip(victim,gsn_plague);
//    affect_strip(victim,gsn_poison);
//   affect_strip(victim,gsn_blindness);
//  affect_strip(victim,gsn_sleep);
// affect_strip(victim,gsn_curse);
    victim->move = victim->max_move;
    update_pos(victim);
    do_look(victim, "auto");
    stop_fighting(ch, TRUE);
    char_from_room(ch);
    char_to_room(ch, get_room_index(ROOM_VNUM_AWINNER));
    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;
//   affect_strip(ch,gsn_plague);
//  affect_strip(ch,gsn_poison); 
// affect_strip(ch,gsn_blindness);
//affect_strip(ch,gsn_sleep);
// affect_strip(ch,gsn_curse);
    ch->move = ch->max_move;
    update_pos(ch);
    do_look(ch, "auto");

    if (IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGER))
	REMOVE_BIT(ch->exbit1_flags, EXBIT1_CHALLENGER);
    if (IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGER))
	REMOVE_BIT(victim->exbit1_flags, EXBIT1_CHALLENGER);
    if (IS_SET(victim->exbit1_flags, EXBIT1_CHALLENGED))
	REMOVE_BIT(victim->exbit1_flags, EXBIT1_CHALLENGED);
    if (IS_SET(ch->exbit1_flags, EXBIT1_CHALLENGED))
	REMOVE_BIT(ch->exbit1_flags, EXBIT1_CHALLENGED);

    ch->challenger = NULL;
    ch->challenged = NULL;
    victim->challenger = NULL;
    victim->challenged = NULL;
    REMOVE_BIT(ch->comm, COMM_NOCHANNELS);
    REMOVE_BIT(ch->act, PLR_NORESTORE);
    REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
    REMOVE_BIT(victim->act, PLR_NORESTORE);

    send_to_char("You have been restored.\n\r", ch);
    send_to_char("You have been restored.\n\r", victim);
    arena = FIGHT_OPEN;		/* clear the arena */
    return;
}

void do_kick(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;

    if (!IS_NPC(ch)) {
	if ((ch->pcdata->tier != 2)
	    && (ch->level < skill_table[gsn_kick].skill_level[ch->class])) {
	    send_to_char
		("You better leave the martial arts to fighters.\n\r", ch);
	    return;
	}
	if ((ch->pcdata->tier == 2)
	    && (ch->level < skill_table[gsn_kick].skill_level[ch->class])
	    && (ch->level < skill_table[gsn_kick].skill_level[ch->clasb])) {
	    send_to_char
		("You better leave the martial arts to fighters.\n\r", ch);
	    return;
	}
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	return;

    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }

    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, skill_table[gsn_kick].beats);
    if (get_skill(ch, gsn_kick) > number_percent()) {
	int dam;

	dam = number_range(1, ch->level);
	dam = number_range(dam, (ch->level * 1.5));

	damage(ch, victim, dam, gsn_kick, DAM_BASH, TRUE);
	check_improve(ch, gsn_kick, TRUE, 1);
    } else {
	damage(ch, victim, 0, gsn_kick, DAM_BASH, TRUE);
	check_improve(ch, gsn_kick, FALSE, 1);
    }
    return;
}




void do_disarm(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;

    hth = 0;

    if ((chance = get_skill(ch, gsn_disarm)) == 0) {
	send_to_char("You don't know how to disarm opponents.\n\r", ch);
	return;
    }

    if (get_eq_char(ch, WEAR_WIELD) == NULL
	&& ((hth = get_skill(ch, gsn_hand_to_hand)) == 0
	    || (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_DISARM)))) {
	send_to_char("You must wield a weapon to disarm.\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }

    if (ch->stunned) {
	send_to_char("You're still a little woozy.\n\r", ch);
	return;
    }

    if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL) {
	send_to_char("{hYour opponent is not wielding a weapon.{x\n\r",
		     ch);
	return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch, get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim, get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch, get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if (get_eq_char(ch, WEAR_WIELD) == NULL)
	chance = chance * hth / 150;
    else
	chance = chance * ch_weapon / 100;

    chance += (ch_vict_weapon / 2 - vict_weapon) / 2;

    /* dex vs. strength */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= 2 * get_curr_stat(victim, STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;

    chance /= 2;

    /* and now the attack */
    if (number_percent() < chance) {
	if (((chance = get_skill(victim, gsn_grip)) == 0)
	    || (!IS_NPC(victim)
		&& victim->level <
		skill_table[gsn_grip].skill_level[victim->class])) {
	    if (chance == 0 || IS_NPC(victim)) {
		WAIT_STATE(ch, skill_table[gsn_disarm].beats);
		disarm(ch, victim);
		check_improve(ch, gsn_disarm, TRUE, 1);
		return;
	    }
	    if ((victim->pcdata->tier != 2)
		&& (victim->level <
		    skill_table[gsn_grip].skill_level[victim->class])) {
		WAIT_STATE(ch, skill_table[gsn_disarm].beats);
		disarm(ch, victim);
		check_improve(ch, gsn_disarm, TRUE, 1);
		return;
	    }
	    if ((victim->pcdata->tier == 2)
		&& (victim->level <
		    skill_table[gsn_grip].skill_level[victim->class])
		&& (victim->level <
		    skill_table[gsn_grip].skill_level[victim->clasb])) {
		WAIT_STATE(ch, skill_table[gsn_disarm].beats);
		disarm(ch, victim);
		check_improve(ch, gsn_disarm, TRUE, 1);
		return;
	    }
	}
	if (number_percent() > (chance / 5) * 4) {
	    WAIT_STATE(ch, skill_table[gsn_disarm].beats);
	    disarm(ch, victim);
	    check_improve(ch, gsn_disarm, TRUE, 1);
	    check_improve(victim, gsn_grip, FALSE, 1);
	    return;
	}
	check_improve(victim, gsn_grip, TRUE, 1);
    }
    WAIT_STATE(ch, skill_table[gsn_disarm].beats);
    act("{hYou fail to disarm $N.{x", ch, NULL, victim, TO_CHAR);
    act("$n tries to disarm you, but fails.{x", ch, NULL, victim,
	TO_VICT);
    act("$n tries to disarm $N, but fails.{x", ch, NULL, victim,
	TO_NOTVICT);
    check_improve(ch, gsn_disarm, FALSE, 1);
    return;
}



void do_sla(CHAR_DATA * ch, char *argument)
{
    send_to_char("If you want to {RSLAY{x, spell it out.\n\r", ch);
    return;
}



void do_slay(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MIL];

    one_argument(argument, arg);
    if (arg[0] == '\0') {
	send_to_char("Slay whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (ch == victim) {
	send_to_char("Suicide is a mortal sin.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim) && victim->level >= get_trust(ch)) {
	send_to_char("{hYou failed.{c\n\r", ch);
	return;
    }

    if (ch->shadow) {
	ch->shadowing->shadowed = FALSE;
	ch->shadowing->shadower = NULL;
	ch->shadowing = NULL;
	ch->shadow = FALSE;
    }
    if (IS_NPC(victim) || get_trust(ch) >= CREATOR) {
	act("{hYou slay $M in cold blood!{x", ch, NULL, victim, TO_CHAR);
	act("$n slays you in cold blood!{x", ch, NULL, victim, TO_VICT);
	act("$n slays $N in cold blood!{x", ch, NULL, victim,
	    TO_NOTVICT);
	raw_kill(victim, ch);
    } else {
	act("$N wields a sword called '{z{RGodSlayer'!{x", ch, NULL,
	    victim, TO_CHAR);
	act("{hYou wield a sword called '{z{RGodSlayer{h'!{x", ch, NULL,
	    victim, TO_VICT);
	act("$N wields a sword called '{z{RGodSlayer'!{x", ch, NULL,
	    victim, TO_NOTVICT);
	act("$N's slice takes off your left arm!{x", ch, NULL, victim,
	    TO_CHAR);
	act("{hYour slice takes off $n's left arm!{x", ch, NULL, victim,
	    TO_VICT);
	act("$N's slice takes off $n's left arm!{x", ch, NULL, victim,
	    TO_NOTVICT);
	act("$N's slice takes off your right arm!{x", ch, NULL, victim,
	    TO_CHAR);
	act("{hYour slice takes off $n's right arm!{x", ch, NULL, victim,
	    TO_VICT);
	act("$N's slice takes off $n's right arm!{x", ch, NULL, victim,
	    TO_NOTVICT);
	act("$N's slice cuts off both of your legs!{x", ch, NULL, victim,
	    TO_CHAR);
	act("{hYour slice cuts off both of $n's legs!{x", ch, NULL, victim,
	    TO_VICT);
	act("$N's slice cuts off both of $n's legs!{x", ch, NULL, victim,
	    TO_NOTVICT);
	act("$N's slice beheads you!{x", ch, NULL, victim, TO_CHAR);
	act("{hYour slice beheads $n!{x", ch, NULL, victim, TO_VICT);
	act("$N's slice beheads $n!{x", ch, NULL, victim, TO_NOTVICT);
	act("You are DEAD!!!{x", ch, NULL, victim, TO_CHAR);
	act("{h$n is DEAD!!!{x", ch, NULL, victim, TO_VICT);
	act("$n is DEAD!!!{x", ch, NULL, victim, TO_NOTVICT);
	act("A sword called '{z{RGodSlayer{x' vanishes.", ch, NULL, victim,
	    TO_VICT);
	act("A sword called '{z{RGodSlayer{x' vanishes.", ch, NULL, victim,
	    TO_NOTVICT);
	raw_kill(ch, victim);
    }
    return;
}

void do_crush( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_tail].skill_level[ch->class] )
    {
	send_to_char( "Since when did you grow a tail?\n\r", ch );
      return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TAIL))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_tail].beats );
    if ( get_skill(ch,gsn_tail) > number_percent())
    {
	damage(ch,victim,number_range( 1, ch->level ), gsn_tail,DAM_SLASH,TRUE);
	check_improve(ch,gsn_tail,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_tail,DAM_SLASH,TRUE);
	check_improve(ch,gsn_tail,FALSE,1);
    }
//	check_killer(ch,victim);
    return;
}

bool check_fade( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_fade) / 2;

    if (!can_see(victim,ch))
	chance /= 3;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;
    act( "Your body fades from existence and avoids $n's attack.", ch, NULL, victim, TO_VICT );
    act( "$N's body fades from existence to avoid your attack.", ch, NULL, victim,  TO_CHAR );
    return TRUE;
}

void do_tail( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_tail].skill_level[ch->class] )
    {
	send_to_char( "Since when did you grow a tail?\n\r", ch );
      return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TAIL))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_tail].beats );
    if ( get_skill(ch,gsn_tail) > number_percent())
    {
	damage(ch,victim,number_range( 1, ch->level ), gsn_tail,DAM_SLASH,TRUE);
	check_improve(ch,gsn_tail,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_tail,DAM_SLASH,TRUE);
	check_improve(ch,gsn_tail,FALSE,1);
    }
//	check_killer(ch,victim);
    return;
}
