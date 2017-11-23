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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"


/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN (spec_breath_any);
DECLARE_SPEC_FUN (spec_breath_acid);
DECLARE_SPEC_FUN (spec_breath_fire);
DECLARE_SPEC_FUN (spec_breath_frost);
DECLARE_SPEC_FUN (spec_breath_gas);
DECLARE_SPEC_FUN (spec_breath_lightning);
DECLARE_SPEC_FUN (spec_cast_adept);
DECLARE_SPEC_FUN (spec_cast_cleric);
DECLARE_SPEC_FUN (spec_cast_judge);
DECLARE_SPEC_FUN (spec_cast_mage);
DECLARE_SPEC_FUN (spec_cast_undead);
DECLARE_SPEC_FUN (spec_executioner);
DECLARE_SPEC_FUN (spec_fido);
DECLARE_SPEC_FUN (spec_guard);
DECLARE_SPEC_FUN (spec_janitor);
DECLARE_SPEC_FUN (spec_mayor);
DECLARE_SPEC_FUN (spec_poison);
DECLARE_SPEC_FUN (spec_thief);
DECLARE_SPEC_FUN (spec_nasty);
DECLARE_SPEC_FUN (spec_troll_member);
DECLARE_SPEC_FUN (spec_ogre_member);
DECLARE_SPEC_FUN (spec_patrolman);
DECLARE_SPEC_FUN (spec_dog_pee);
DECLARE_SPEC_FUN (spec_cast_clan_adept);
// DECLARE_SPEC_FUN( spec_toll );
DECLARE_SPEC_FUN (spec_dark_magic);
DECLARE_SPEC_FUN (spec_wear);
DECLARE_SPEC_FUN (spec_circle);
DECLARE_SPEC_FUN (spec_prog_one);
DECLARE_SPEC_FUN (spec_ai_actor);
DECLARE_SPEC_FUN (spec_prog_two);
DECLARE_SPEC_FUN (spec_prog_three);
DECLARE_SPEC_FUN (spec_ai_magic);
DECLARE_SPEC_FUN (spec_clan_healer);
DECLARE_SPEC_FUN (spec_questmaster);	/* Vassago */

/*
 * the function table 
 */
const struct spec_type spec_table[] = {
  {"spec_wear", spec_wear},
  {"spec_prog_one", spec_prog_one},
  {"spec_ai_actor", spec_ai_actor},
  {"spec_prog_two", spec_prog_two},
  {"spec_prog_three", spec_prog_three},
  {"spec_ai_magic", spec_ai_magic},
  {"spec_circle", spec_circle},
  // { "spec_toll", spec_toll },
  {"spec_dark_magic", spec_dark_magic},
  {"spec_clan_healer", spec_clan_healer},
  {"spec_questmaster", spec_questmaster},	/* Vassago */
  {"spec_breath_any", spec_breath_any},
  {"spec_breath_acid", spec_breath_acid},
  {"spec_breath_fire", spec_breath_fire},
  {"spec_breath_frost", spec_breath_frost},
  {"spec_breath_gas", spec_breath_gas},
  {"spec_breath_lightning", spec_breath_lightning},
  {"spec_cast_adept", spec_cast_adept},
  {"spec_cast_cleric", spec_cast_cleric},
  {"spec_cast_judge", spec_cast_judge},
  {"spec_cast_mage", spec_cast_mage},
  {"spec_cast_undead", spec_cast_undead},
  {"spec_executioner", spec_executioner},
  {"spec_fido", spec_fido},
  {"spec_guard", spec_guard},
  {"spec_janitor", spec_janitor},
  {"spec_mayor", spec_mayor},
  {"spec_poison", spec_poison},
  {"spec_thief", spec_thief},
  {"spec_nasty", spec_nasty},
  {"spec_troll_member", spec_troll_member},
  {"spec_ogre_member", spec_ogre_member},
  {"spec_patrolman", spec_patrolman},
  {"spec_dog_pee", spec_dog_pee},
  {"spec_cast_clan_adept", spec_cast_clan_adept},
  {NULL, NULL}
};

/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN * spec_lookup (const char *name)
{
  int i;

  for (i = 0; spec_table[i].name != NULL; i++)
    {
      if (LOWER (name[0]) == LOWER (spec_table[i].name[0])
	  && !str_prefix (name, spec_table[i].name))
	return spec_table[i].function;
    }

  return 0;
}

char * spec_name (SPEC_FUN * function)
{
  int i;

  for (i = 0; spec_table[i].function != NULL; i++)
    {
      if (function == spec_table[i].function)
	return spec_table[i].name;
    }

  return NULL;
}

bool spec_troll_member (CHAR_DATA * ch)
{
  CHAR_DATA *vch, *victim = NULL;
  int count = 0;
  char *message;

  if (!IS_AWAKE (ch) || IS_AFFECTED (ch, AFF_CALM) || ch->in_room == NULL
      || IS_AFFECTED (ch, AFF_CHARM) || ch->fighting != NULL)
    return FALSE;

  /*
   * find an ogre to beat up 
   */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (!IS_NPC (vch) || ch == vch)
	continue;

      if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
	return FALSE;

      if (vch->pIndexData->group == GROUP_VNUM_OGRES
	  && ch->level > vch->level - 2 && !is_safe (ch, vch))
	{
	  if (number_range (0, count) == 0)
	    victim = vch;

	  count++;
	}
    }

  if (victim == NULL)
    return FALSE;

  /*
   * say something, then raise hell 
   */
  switch (number_range (0, 6))
    {
    default:
      message = NULL;
      break;
    case 0:
      message = "$n yells '{aI've been looking for you, punk!{x'";
      break;
    case 1:
      message = "With a scream of rage, $n attacks $N.";
      break;
    case 2:
      message =
	"$n says '{aWhat's slimy Ogre trash like you doing around here?{x'";
      break;
    case 3:
      message = "$n cracks his knuckles and says '{SDo ya feel lucky?{x'";
      break;
    case 4:
      message = "$n says '{aThere's no cops to save you this time!{x'";
      break;
    case 5:
      message = "$n says '{aTime to join your brother, spud.{x'";
      break;
    case 6:
      message = "$n says '{aLet's rock.{x'";
      break;
    }

  if (message != NULL)
    act (message, ch, NULL, victim, TO_ALL);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return TRUE;
}

bool spec_ogre_member (CHAR_DATA * ch)
{
  CHAR_DATA *vch, *victim = NULL;
  int count = 0;
  char *message;

  if (!IS_AWAKE (ch) || IS_AFFECTED (ch, AFF_CALM) || ch->in_room == NULL
      || IS_AFFECTED (ch, AFF_CHARM) || ch->fighting != NULL)
    return FALSE;

  /*
   * find an troll to beat up 
   */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (!IS_NPC (vch) || ch == vch)
	continue;

      if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
	return FALSE;

      if (vch->pIndexData->group == GROUP_VNUM_TROLLS
	  && ch->level > vch->level - 2 && !is_safe (ch, vch))
	{
	  if (number_range (0, count) == 0)
	    victim = vch;

	  count++;
	}
    }

  if (victim == NULL)
    return FALSE;

  /*
   * say something, then raise hell 
   */
  switch (number_range (0, 6))
    {
    default:
      message = NULL;
      break;
    case 0:
      message = "$n yells '{aI've been looking for you, punk!{x'";
      break;
    case 1:
      message = "With a scream of rage, $n attacks $N.'";
      break;
    case 2:
      message =
	"$n says '{aWhat's Troll filth like you doing around here?{x'";
      break;
    case 3:
      message = "$n cracks his knuckles and says '{SDo ya feel lucky?{x'";
      break;
    case 4:
      message = "$n says '{aThere's no cops to save you this time!{x'";
      break;
    case 5:
      message = "$n says '{aTime to join your brother, spud.{x'";
      break;
    case 6:
      message = "$n says '{aLet's rock.{x'";
      break;
    }

  if (message != NULL)
  act (message, ch, NULL, victim, TO_ALL);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return TRUE;
}

bool spec_patrolman (CHAR_DATA * ch)
{
  CHAR_DATA *vch, *victim = NULL;
  OBJ_DATA *obj;
  char *message;
  int count = 0;

  if (!IS_AWAKE (ch) || IS_AFFECTED (ch, AFF_CALM) || ch->in_room == NULL
      || IS_AFFECTED (ch, AFF_CHARM) || ch->fighting != NULL)
    return FALSE;

  /*
   * look for a fight in the room 
   */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch == ch)
	continue;

      if (vch->fighting != NULL)
	{			/* break it up! */
	  if (number_range (0, count) == 0)
	    victim = (vch->level > vch->fighting->level)
	      ? vch : vch->fighting;
	  count++;
	}
    }

  if (victim == NULL || (IS_NPC (victim) && victim->spec_fun == ch->spec_fun))
    return FALSE;

  if (((obj = get_eq_char (ch, WEAR_NECK_1)) != NULL
       && obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)
      || ((obj = get_eq_char (ch, WEAR_NECK_2)) != NULL
	  && obj->pIndexData->vnum == OBJ_VNUM_WHISTLE))
    {
      act ("You blow down hard on $p.", ch, obj, NULL, TO_CHAR);
      act ("$n blows on $p, ***WHEEEEEEEEEEEET***", ch, obj, NULL, TO_ROOM);

      for (vch = char_list; vch != NULL; vch = vch->next)
	{
	  if (vch->in_room == NULL)
	    continue;

	  if (vch->in_room != ch->in_room
	      && vch->in_room->area == ch->in_room->area)
	    send_to_char ("You hear a shrill whistling sound.\n\r", vch);
	}
    }

  switch (number_range (0, 6))
    {
    default:
      message = NULL;
      break;
    case 0:
      message = "$n yells '{aAll roit! All roit! break it up!{x'";
      break;
    case 1:
      message = "$n says '{aSociety's to blame, but what's a bloke to do?{x'";
      break;
    case 2:
      message = "$n mumbles '{abloody kids will be the death of us all.{x'";
      break;
    case 3:
      message = "$n shouts '{aStop that! Stop that!{T' and attacks.";
      break;
    case 4:
      message = "$n pulls out his billy and goes to work.";
      break;
    case 5:
      message = "$n sighs in resignation and proceeds to break up the fight.";
      break;
    case 6:
      message = "$n says '{aSettle down, you hooligans!{x'";
      break;
    }

  if (message != NULL)
    act (message, ch, NULL, NULL, TO_ALL);

  multi_hit (ch, victim, TYPE_UNDEFINED);

  return TRUE;
}

bool spec_dog_pee (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;

  if (!IS_AWAKE (ch))
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim != ch && can_see (ch, victim) && number_bits (1) == 0
	  && !IS_NPC (victim))
	break;
    }

  if (victim == NULL)
    return FALSE;

  switch (number_bits (3))
    {
    case 0:
      return FALSE;

    case 1:
      return FALSE;

    case 2:
      act ("$n lifts $s hind leg, and pees on $N's feet.", ch, NULL,
	   victim, TO_NOTVICT);
      act ("$n lifts $s hind leg, and pees on your feet.", ch, NULL,
	   victim, TO_VICT);
      return TRUE;
    }

  return FALSE;
}

bool spec_nasty (CHAR_DATA * ch)
{
  CHAR_DATA *victim, *v_next;
  long gold;

  if (!IS_AWAKE (ch))
    {
      return FALSE;
    }

  if (ch->position != POS_FIGHTING)
    {
      for (victim = ch->in_room->people; victim != NULL; victim = v_next)
	{
	  v_next = victim->next_in_room;
	  if (!IS_NPC (victim)
	      && (victim->level > ch->level)
	      && (victim->level < ch->level + 10))
	    {
	      do_backstab (ch, victim->name);
	      if (ch->position != POS_FIGHTING)
		do_murder (ch, victim->name);
	      /*
	       * should steal some coins right away? :) 
	       */
	      return TRUE;
	    }
	}
      return FALSE;		/* No one to attack */
    }

  /*
   * okay, we must be fighting.... steal some coins and flee 
   */
  if ((victim = ch->fighting) == NULL)
    return FALSE;		/* let's be paranoid.... */

  switch (number_bits (2))
    {
    case 0:
      act ("$n rips apart your coin purse, spilling your gold!",
	   ch, NULL, victim, TO_VICT);
      act ("You slash apart $N's coin purse and gather his gold.",
	   ch, NULL, victim, TO_CHAR);
      act ("$N's coin purse is ripped apart!", ch, NULL, victim, TO_NOTVICT);
      gold = victim->gold / 10;	/* steal 10% of his gold */
      victim->gold -= gold;
      ch->gold += gold;
      return TRUE;

    case 1:
      do_flee (ch, "");
      return TRUE;

    default:
      return FALSE;
    }
}

/*
 * Core procedure for dragons.
 */
bool dragon (CHAR_DATA * ch, char *spell_name)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  int sn;

  if (ch->position != POS_FIGHTING)
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (3) == 0)
	break;
    }

  if (victim == NULL)
    return FALSE;

  if (ch->stunned)
    return FALSE;

  if ((sn = skill_lookup (spell_name)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any (CHAR_DATA * ch)
{
  if (ch->position != POS_FIGHTING)
    return FALSE;

  switch (number_bits (3))
    {
    case 0:
      return spec_breath_fire (ch);
    case 1:
    case 2:
      return spec_breath_lightning (ch);
    case 3:
      return spec_breath_gas (ch);
    case 4:
      return spec_breath_acid (ch);
    case 5:
    case 6:
    case 7:
      return spec_breath_frost (ch);
    }

  return FALSE;
}

bool spec_breath_acid (CHAR_DATA * ch)
{
  return dragon (ch, "acid breath");
}

bool spec_breath_fire (CHAR_DATA * ch)
{
  return dragon (ch, "fire breath");
}

bool spec_breath_frost (CHAR_DATA * ch)
{
  return dragon (ch, "frost breath");
}

bool spec_breath_gas (CHAR_DATA * ch)
{
  int sn;

  if (ch->position != POS_FIGHTING)
    return FALSE;

  if (ch->stunned)
    return FALSE;

  if ((sn = skill_lookup ("gas breath")) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, NULL, TARGET_CHAR);
  return TRUE;
}



bool spec_breath_lightning (CHAR_DATA * ch)
{
  return dragon (ch, "lightning breath");
}

bool spec_cast_adept (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;

  if (!IS_AWAKE (ch))
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim != ch && can_see (ch, victim) && number_bits (1) == 0
	  && !IS_NPC (victim) && victim->level < 31)
	break;
    }

  if (victim == NULL)
    return FALSE;

  switch (number_bits (4))
    {
    case 0:
      act ("$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM);
      spell_armor (skill_lookup ("armor"), ch->level, ch, victim,
		   TARGET_CHAR);
      return TRUE;

    case 1:
      act ("$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM);
      spell_bless (skill_lookup ("bless"), ch->level, ch, victim,
		   TARGET_CHAR);
      return TRUE;

    case 2:
      act ("$n utters the words 'judicandus noselacri'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_cure_blindness (skill_lookup ("cure blindness"), ch->level, ch,
			    victim, TARGET_CHAR);
      return TRUE;

    case 3:
      act ("$n utters the words 'judicandus dies'.", ch, NULL, NULL, TO_ROOM);
      spell_cure_light (skill_lookup ("cure light"), ch->level, ch, victim,
			TARGET_CHAR);
      return TRUE;

    case 4:
      act ("$n utters the words 'judicandus sausabru'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_cure_poison (skill_lookup ("cure poison"), ch->level, ch,
			 victim, TARGET_CHAR);
      return TRUE;

    case 5:
      act ("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM);
      spell_refresh (skill_lookup ("refresh"), ch->level, ch, victim,
		     TARGET_CHAR);
      return TRUE;

    case 6:
      act ("$n utters the words 'judicandus eugzagz'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_cure_disease (skill_lookup ("cure disease"), ch->level, ch,
			  victim, TARGET_CHAR);
      return TRUE;

    case 7:
      act ("$n utters the words 'judicandus eqtuyp'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_sanctuary (skill_lookup ("sanctuary"), ch->level, ch, victim,
		       TARGET_CHAR);
      return TRUE;

    case 8:
      act ("$n utters the words 'qwerty'.", ch, NULL, NULL, TO_ROOM);
      spell_shield (skill_lookup ("shield"),
		    ch->level, ch, victim, TARGET_CHAR);
      return TRUE;

    case 9:
      act ("$n utters the words 'saspell'.", ch, NULL, NULL, TO_ROOM);
      spell_heal (skill_lookup ("heal"), ch->level, ch, victim, TARGET_CHAR);
    }

  return FALSE;
}

bool spec_cast_clan_adept (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;

  if (!IS_AWAKE (ch))
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim != ch && can_see (ch, victim) && number_bits (1) == 0
	  && !IS_NPC (victim) && (victim->level < 51)
	  && (victim->clan == ch->in_room->clan))
	break;
    }

  if (victim == NULL)
    return FALSE;

  switch (number_bits (4))
    {
    case 0:
      act ("$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM);
      spell_armor (skill_lookup ("armor"), ch->level / 2, ch, victim,
		   TARGET_CHAR);
      return TRUE;

    case 1:
      act ("$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM);
      spell_bless (skill_lookup ("bless"), ch->level / 2, ch, victim,
		   TARGET_CHAR);
      return TRUE;

    case 2:
      act ("$n utters the words 'judicandus noselacri'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_cure_blindness (skill_lookup ("cure blindness"), ch->level / 2,
			    ch, victim, TARGET_CHAR);
      return TRUE;

    case 3:
      act ("$n utters the words 'judicandus dies'.", ch, NULL, NULL, TO_ROOM);
      spell_cure_light (skill_lookup ("cure light"), ch->level / 2, ch,
			victim, TARGET_CHAR);
      return TRUE;

    case 4:
      act ("$n utters the words 'judicandus sausabru'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_cure_poison (skill_lookup ("cure poison"), ch->level / 2, ch,
			 victim, TARGET_CHAR);
      return TRUE;

    case 5:
      act ("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM);
      spell_refresh (skill_lookup ("refresh"), ch->level / 2, ch, victim,
		     TARGET_CHAR);
      return TRUE;

    case 6:
      act ("$n utters the words 'judicandus eugzagz'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_cure_disease (skill_lookup ("cure disease"), ch->level / 2, ch,
			  victim, TARGET_CHAR);
      return TRUE;

    case 7:
      act ("$n utters the words 'judicandus eqtuyp'.", ch, NULL, NULL,
	   TO_ROOM);
      spell_sanctuary (skill_lookup ("sanctuary"), ch->level / 2, ch,
		       victim, TARGET_CHAR);
      return TRUE;

    case 8:
      act ("$n utters the words 'qwerty'.", ch, NULL, NULL, TO_ROOM);
      spell_shield (skill_lookup ("shield"),
		    ch->level / 2, ch, victim, TARGET_CHAR);
      return TRUE;

    case 9:
      act ("$n utters the words 'saspell'.", ch, NULL, NULL, TO_ROOM);
      spell_heal (skill_lookup ("heal"),
		  ch->level / 2, ch, victim, TARGET_CHAR);
    }

  return FALSE;
}

bool spec_cast_cleric (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;

  if (ch->position != POS_FIGHTING)
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }

  if (victim == NULL)
    return FALSE;

  for (;;)
    {
      int min_level;

      switch (number_bits (4))
	{
	case 0:
	  min_level = 0;
	  spell = "blindness";
	  break;
	case 1:
	  min_level = 3;
	  spell = "cause serious";
	  break;
	case 2:
	  min_level = 7;
	  spell = "earthquake";
	  break;
	case 3:
	  min_level = 9;
	  spell = "cause critical";
	  break;
	case 4:
	  min_level = 10;
	  spell = "dispel evil";
	  break;
	case 5:
	  min_level = 12;
	  spell = "curse";
	  break;
	case 6:
	  min_level = 12;
	  spell = "change sex";
	  break;
	case 7:
	  min_level = 13;
	  spell = "flamestrike";
	  break;
	case 8:
	case 9:
	case 10:
	  min_level = 15;
	  spell = "harm";
	  break;
	case 11:
	  min_level = 15;
	  spell = "plague";
	  break;
	default:
	  min_level = 16;
	  spell = "dispel magic";
	  break;
	}

      if (ch->level >= min_level)
	break;
    }

  if (ch->stunned)
    return FALSE;

  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}

bool spec_cast_judge (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;

  if (ch->position != POS_FIGHTING)
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }

  if (victim == NULL)
    return FALSE;

  if (ch->stunned)
    return FALSE;

  spell = "high explosive";
  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}



bool spec_cast_mage (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;

  if (ch->position != POS_FIGHTING)
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }

  if (victim == NULL)
    return FALSE;

  for (;;)
    {
      int min_level;

      switch (number_bits (4))
	{
	case 0:
	  min_level = 0;
	  spell = "blindness";
	  break;
	case 1:
	  min_level = 3;
	  spell = "chill touch";
	  break;
	case 2:
	  min_level = 7;
	  spell = "weaken";
	  break;
	case 3:
	  min_level = 8;
	  spell = "teleport";
	  break;
	case 4:
	  min_level = 11;
	  spell = "colour spray";
	  break;
	case 5:
	  min_level = 12;
	  spell = "change sex";
	  break;
	case 6:
	  min_level = 13;
	  spell = "energy drain";
	  break;
	case 7:
	case 8:
	case 9:
	  min_level = 15;
	  spell = "fireball";
	  break;
	case 10:
	  min_level = 20;
	  spell = "plague";
	  break;
	default:
	  min_level = 20;
	  spell = "acid blast";
	  break;
	}

      if (ch->level >= min_level)
	break;
    }

  if (ch->stunned)
    return FALSE;

  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}

bool spec_cast_undead (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;

  if (ch->position != POS_FIGHTING)
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
    }

  if (victim == NULL)
    return FALSE;

  for (;;)
    {
      int min_level;

      switch (number_bits (4))
	{
	case 0:
	  min_level = 0;
	  spell = "curse";
	  break;
	case 1:
	  min_level = 3;
	  spell = "weaken";
	  break;
	case 2:
	  min_level = 6;
	  spell = "chill touch";
	  break;
	case 3:
	  min_level = 9;
	  spell = "blindness";
	  break;
	case 4:
	  min_level = 12;
	  spell = "poison";
	  break;
	case 5:
	  min_level = 15;
	  spell = "energy drain";
	  break;
	case 6:
	  min_level = 18;
	  spell = "harm";
	  break;
	case 7:
	  min_level = 21;
	  spell = "teleport";
	  break;
	case 8:
	  min_level = 20;
	  spell = "plague";
	  break;
	default:
	  min_level = 18;
	  spell = "harm";
	  break;
	}

      if (ch->level >= min_level)
	break;
    }

  if (ch->stunned)
    return FALSE;

  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}

bool spec_executioner (CHAR_DATA * ch)
{
  char buf[MSL];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *crime;

  if (!IS_AWAKE (ch) || ch->fighting != NULL)
    return FALSE;

  crime = "";
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;

      if (!IS_NPC (victim) && IS_SET (victim->act, PLR_TWIT)
	  && can_see (ch, victim))
	{
	  crime = "TWIT";
	  break;
	}
    }

  if (victim == NULL)
    return FALSE;

  sprintf (buf, "{a%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!{x",
	   victim->name, crime);
  REMOVE_BIT (ch->comm, COMM_NOSHOUT);
  do_yell (ch, buf);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return TRUE;
}

bool spec_fido (CHAR_DATA * ch)
{
  OBJ_DATA *corpse;
  OBJ_DATA *c_next;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  if (!IS_AWAKE (ch))
    return FALSE;

  for (corpse = ch->in_room->contents; corpse != NULL; corpse = c_next)
    {
      c_next = corpse->next_content;
      if (corpse->item_type != ITEM_CORPSE_NPC)
	continue;

      act ("$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM);
      for (obj = corpse->contains; obj; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  obj_from_obj (obj);
	  obj_to_room (obj, ch->in_room);
	}
      extract_obj (corpse);
      return TRUE;
    }

  return FALSE;
}

bool spec_guard (CHAR_DATA * ch)
{
  char buf[MSL];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  CHAR_DATA *ech;
  char *crime;
  int max_evil;

  if (!IS_AWAKE (ch) || ch->fighting != NULL)
    return FALSE;

  max_evil = 300;
  ech = NULL;
  crime = "";

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;

      if (!IS_NPC (victim) && IS_SET (victim->act, PLR_TWIT)
	  && can_see (ch, victim))
	{
	  crime = "TWIT";
	  break;
	}

      if (victim->fighting != NULL
	  && victim->fighting != ch && victim->alignment < max_evil)
	{
	  max_evil = victim->alignment;
	  ech = victim;
	}
    }

  if (victim != NULL)
    {
      sprintf (buf, "{a%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!{x",
	       victim->name, crime);
      REMOVE_BIT (ch->comm, COMM_NOSHOUT);
      do_yell (ch, buf);
      multi_hit (ch, victim, TYPE_UNDEFINED);
      return TRUE;
    }

  if (ech != NULL)
    {
      act ("$n screams '{aPROTECT THE INNOCENT!!  BANZAI!!{x",
	   ch, NULL, NULL, TO_ROOM);
      multi_hit (ch, ech, TYPE_UNDEFINED);
      return TRUE;
    }

  return FALSE;
}



bool spec_janitor (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  OBJ_DATA *trash;
  OBJ_DATA *trash_next;

  if (!IS_AWAKE (ch))
    return FALSE;
  for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next ) 
    {
      trash_next = trash->next_content;
      if (!IS_SET (trash->wear_flags, ITEM_TAKE) || !can_loot (ch, trash))
	continue;
  if (trash->item_type == ITEM_DRINK_CON || trash->item_type == ITEM_TRASH || trash->cost < 10)
	{
	for (victim = ch->in_room->people; victim != NULL; victim = v_next)
		{
		v_next = victim->next_in_room;
		  if (victim != ch && can_see (ch, victim) && number_bits (1) == 0 && !IS_NPC (victim))
			{
			   switch (number_range(0, 2))
			   {
				 case 0:
				    act ("$n picks up some trash.", ch, NULL, NULL, TO_ROOM);
				    obj_from_room (trash);
				    obj_to_char (trash, ch);
				    break;
				 case 1:
				    if (!IS_NPC (victim) )
				    {
				    act ("{x$n says '{RAre you responsible for this mess $N?{x'",
				    ch, NULL, victim, TO_ROOM);
				    }
				    break;
				 case 2:
				    act ("$n picks up $p.", ch, trash, NULL, TO_ROOM);
				    obj_from_room (trash);
				    obj_to_char (trash, ch);
				    break;
	// act( "$n gets $p.", ch, obj, container, TO_ROOM );
	// obj_from_room( obj );
			   }
			}
			
	     }
		return TRUE;
	 }
    }

return FALSE;
}



bool spec_mayor (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;

  static const char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static const char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static const char *path;
  static int pos;
  static bool move;

  if (!move)
    {
      if (time_info.hour == 6)
	{
	  path = open_path;
	  move = TRUE;
	  pos = 0;
	}

      if (time_info.hour == 20)
	{
	  path = close_path;
	  move = TRUE;
	  pos = 0;
	}
    }

  if (ch->fighting != NULL)
    return spec_cast_mage (ch);
  if (!move || ch->position < POS_SLEEPING)
    return FALSE;

  switch (path[pos])
    {
    case '0':
    case '1':
    case '2':
    case '3':
      move_char (ch, path[pos] - '0', FALSE, FALSE);
      break;

    case 'W':
      ch->position = POS_STANDING;
      act ("$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM);
      break;

    case 'S':
      ch->position = POS_SLEEPING;
      act ("$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM);
      break;

    case 'a':
      act ("{x$n says '{mHello Honey!{x'", ch, NULL, NULL, TO_ROOM);
      break;

    case 'b':
      act
	("{x$n says '{mWhat a view!  I must do something about that dump!{x'",
	 ch, NULL, NULL, TO_ROOM);
      break;

    case 'c':
      act
	("{x$n says '{mVandals!  Youngsters have no respect for anything!{x'",
	 ch, NULL, NULL, TO_ROOM);
      break;

    case 'd':
			switch (number_range(0, 2))
			{
			   case 0:
			   for (victim = ch->in_room->people; victim != NULL; victim = v_next)
				 {
				    v_next = victim->next_in_room;
				    if (victim != ch && can_see (ch, victim) && !IS_NPC (victim))
			  {
				act ("$n turns to $N and says 'Please enjoy your stay in Midgaard.'", 
				ch, NULL, victim, TO_NOTVICT);
				act ("$n turns to you and says 'Please enjoy your stay in Midgaard.'", 
				ch, NULL, victim, TO_VICT);
			  }
				break;
				 }
			   case 1:
			   for (victim = ch->in_room->people; victim != NULL; victim = v_next)
				 {
				    v_next = victim->next_in_room;
				    if (victim != ch && can_see (ch, victim) && !IS_NPC (victim))
					  {
				act ("$n turns to $N and says 'Please enjoy your stay in Midgaard.'", 
				ch, NULL, victim, TO_NOTVICT);
				act ("$n turns to you and says 'Please enjoy your stay in Midgaard.'", 
				ch, NULL, victim, TO_VICT);
					  }
						break;
				 }
			   case 2:
			   act ("{x$n says '{mGood day, citizens!{x'", ch, NULL, NULL, TO_ROOM);
			   break;
			}

    case 'e':
			for (victim = ch->in_room->people; victim != NULL; victim = v_next)
			   {
				 v_next = victim->next_in_room;
				 if (victim != ch && can_see (ch, victim) && !IS_NPC (victim))
				    {
		    act ("You say 'Please stop by my office anytime $N!'", ch, NULL, victim, TO_CHAR);
		    act ("$n says 'Please stop by my office anytime $N!", ch, NULL, victim, TO_NOTVICT);
				    }
				 break;
			   }

    case 'E':
		act ("{x$n says '{mI hereby declare the city of Midgaard open!{x'", ch, NULL, NULL, TO_ROOM);
      //act ("{x$n says '{aI hereby declare the city of Midgaard closed!{x'", ch, NULL, NULL, TO_ROOM);
      break;
    case 'O':
      /*
       * do_unlock( ch, "gate" ); 
       */
      do_open (ch, "gate");
      break;

    case 'C':
      do_close (ch, "gate");
      /*
       * do_lock( ch, "gate" ); 
       */
      break;

    case '.':
      move = FALSE;
      break;
    }

  pos++;
  return FALSE;
}

bool spec_poison (CHAR_DATA * ch)
{
  CHAR_DATA *victim;

  if (ch->position != POS_FIGHTING
      || (victim = ch->fighting) == NULL || number_percent () > 2 * ch->level)
    return FALSE;

  if (ch->stunned)
    return FALSE;

  act ("You bite $N!", ch, NULL, victim, TO_CHAR);
  act ("$n bites $N!", ch, NULL, victim, TO_NOTVICT);
  act ("$n bites you!", ch, NULL, victim, TO_VICT);
  spell_poison (gsn_poison, ch->level, ch, victim, TARGET_CHAR);
  return TRUE;
}

bool spec_thief (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  long gold, silver;

  if (ch->position != POS_STANDING)
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;

      if (IS_NPC (victim)
	  || victim->level >= LEVEL_IMMORTAL
	  || number_bits (5) != 0 || !can_see (ch, victim))
	continue;

      if (IS_AWAKE (victim) && number_range (0, ch->level) == 0)
	{
	  act ("You discover $n's hands in your {zwallet!{x",
	       ch, NULL, victim, TO_VICT);
	  act ("$N discovers $n's hands in $S {zwallet!{x",
	       ch, NULL, victim, TO_NOTVICT);
	  return TRUE;
	}
      else
	{
	  gold =
	    victim->gold * UMIN (number_range (1, 20), ch->level / 2) / 100;
	  gold = UMIN (gold, ch->level * ch->level * 10);
	  ch->gold += gold;
	  victim->gold -= gold;
	  silver =
	    victim->silver * UMIN (number_range (1, 20), ch->level / 2) / 100;
	  silver = UMIN (silver, ch->level * ch->level * 25);
	  ch->silver += silver;
	  victim->silver -= silver;
	  return TRUE;
	}
    }

  return FALSE;
}

bool spec_prog_one (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  if (!IS_AWAKE (ch))
    return FALSE;
  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim != ch && can_see (ch, victim) && number_bits (1) == 0
	  && !IS_NPC (victim))
	break;
    }
  if (victim == NULL)
    return TRUE;
  switch (number_bits (1))
    {
    case 0:
      if (ch->position == POS_FIGHTING)
	{
	  if (IS_STANCE (victim, STANCE_NONE))
	    {
	      do_stance (ch, "bull");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_SERPENT))
	    {
	      do_stance (ch, "crab");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_CRANE))
	    {
	      do_stance (ch, "serpent");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_MONGOOSE))
	    {
	      do_stance (ch, "serpent");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_BULL))
	    {
	      do_stance (ch, "mongoose");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_CRAB))
	    {
	      do_stance (ch, "crane");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_MANTIS))
	    {
	      do_stance (ch, "serpent");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_TIGER))
	    {
	      do_stance (ch, "bull");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_DRAGON))
	    {
	      do_stance (ch, "mongoose");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_MONKEY))
	    {
	      do_stance (ch, "bull");
	      return TRUE;
	    }
	  else if (IS_STANCE (victim, STANCE_SWALLOW))
	    {
	      do_stance (ch, "crane");
	      return TRUE;
	    }
	}
      else
	{
	  do_get (ch, "all");
	  do_drop (ch, "corpse");
	  do_get (ch, "all corpse");
	  do_wear (ch, "all");
	  return TRUE;
	}
    }
  return FALSE;
}
// DB5




bool spec_ai_actor ( CHAR_DATA * ch )
{
    //char buf[MSL];
    char *spell;
    char *m;
    char *m1;
    char *m2;
    char *m3;
    char *m4;
    char *m5;
//    char *m6;
    char *m7;
    CHAR_DATA *victim; 
    CHAR_DATA *v_next;
    OBJ_DATA *object; 
    OBJ_DATA *obj2; 
    OBJ_DATA *object_next;
    bool drop_out = FALSE;
    int sn;
    if ( !IS_AWAKE ( ch ) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next ) {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits ( 2 ) == 0 )
	    break;
	if ( victim != ch && can_see ( ch, victim )
	     && number_bits ( 1 ) == 0 )
	    break;
	if ( victim == NULL )
	    return TRUE;
    }
    if ( victim == NULL )
	return FALSE;

    for ( object = ch->in_room->contents; object; object = object_next ) {
	object_next = object->next_content;
	if ( object == NULL )
	    continue;
	if ( !can_see_obj ( ch, object ) )
	    continue;
	if ( !IS_SET ( object->wear_flags, ITEM_TAKE ) )
	    continue;
	if ( object->item_type == ITEM_CORPSE_NPC )
	    continue;
	if ( object->item_type == ITEM_CORPSE_PC )
	    continue;

	if ( object->pIndexData->vnum == OVPKT1 ) {
		m1 = "$n says 'This will get me to the next level!'";
	    if ( m1 != NULL ) act ( m1, ch, NULL, victim, TO_ALL );
	    ch->level++;
	    extract_obj ( object );

	    m2 = "{xThe Mob Guildmaster appears and awards $n with a degree for the next level.";
	    break;
	    if ( m2 != NULL ) act ( m2, ch, NULL, victim, TO_ALL );
	}
	if ( object->pIndexData->vnum == IMPTOKEN ) {
	    do_wear ( ch, object->name );
	    extract_obj ( object );
	    ch->level += 10;
	    object = create_object ( get_obj_index ( MG_WEAPON ), 0 );
	    object->level = ch->level;
	    object->value[0] = number_range ( 0, 8 );
	    object->value[1] = ch->level / 3 + number_range ( 3, 12 );
	    object->value[2] = ch->level / 3 + number_range ( 3, 18 );
	    object->value[3] = number_range ( 0, 39 );
	    object->value[4] = 479;

	    if ( object->value[0] == 0 ) {
		object->name = "exotic";
	    }
	    if ( object->value[0] == 1 ) {
		object->name = "sword";
	    }
	    if ( object->value[0] == 2 ) {
		object->name = "dagger";
	    }
	    if ( object->value[0] == 3 ) {
		object->name = "spear";
	    }
	    if ( object->value[0] == 4 ) {
		object->name = "mace";
	    }
	    if ( object->value[0] == 5 ) {
		object->name = "axe";
	    }
	    if ( object->value[0] == 6 ) {
		object->name = "flail";
	    }
	    if ( object->value[0] == 7 ) {
		object->name = "whip";
	    }
	    if ( object->value[0] == 8 ) {
		object->name = "polearm";
	    }

	    object->short_descr = capitalize ( object->name );
	    object->description = capitalize ( object->name );

	    m3 = "says {a'Thank you very much sir, your kindness will go a long way.'";
	    if ( m3 != NULL ) act ( m3, ch, NULL, victim, TO_ALL );
	    m4 = "$n's PK token morphs into <<<MobGear>>.";
	    if ( m4 != NULL ) act ( m4, ch, NULL, victim, TO_ROOM );
	    send_to_char ( "Your MobGear arrives suddenly.\n\r", ch );
	    obj_to_char ( object, ch );
	    do_wear ( ch, object->name );
	    break;
	}

	if ( object->pIndexData->vnum == OVPKT2 ) {
	    do_wear ( ch, object->name );
	    extract_obj ( object );
	    if ( ( number_percent (  ) > 1 ) && ( number_percent (  ) < 85 ) ) {
		object = create_object ( get_obj_index ( MG_WEAPON ), 0 );
		object->level = ch->level - 10;
		object->value[0] = number_range ( 0, 8 );
		object->value[1] = ch->level / 5 + number_range ( 0, 8 );
		object->value[2] = ch->level / 5 + number_range ( 0, 12 );
		object->value[3] = number_range ( 0, 39 );
		object->value[4] = number_range ( 1, 199 );
	    } else if ( number_percent (  ) > 95 ) {
		object = create_object ( get_obj_index ( MG_WEAPON ), 0 );
		object->level = ch->level - 5;
		object->value[0] = number_range ( 0, 8 );
		object->value[1] = ch->level / 4 + number_range ( 0, 8 );
		object->value[2] = ch->level / 4 + number_range ( 0, 12 );
		object->value[3] = number_range ( 30, 39 );
		object->value[4] = number_range ( 200, 399 );
	    } else if ( number_percent (  ) > 85 && number_percent ( ) <= 95 ) {
		object = create_object ( get_obj_index ( MG_WEAPON ), 0 );
		object->level = ch->level - 8;
		object->value[0] = number_range ( 0, 8 );
		object->value[1] = ch->level / 3 + number_range ( 0, 8 );
		object->value[2] = ch->level / 3 + number_range ( 0, 12 );
		object->value[3] = number_range ( 20, 39 );
		object->value[4] = number_range ( 300, 512 );
	    }

	    if ( object->value[0] == 0 ) {
		object->name = "exotic";
	    }
	    if ( object->value[0] == 1 ) {
		object->name = "sword";
	    }
	    if ( object->value[0] == 2 ) {
		object->name = "dagger";
	    }
	    if ( object->value[0] == 3 ) {
		object->name = "spear";
	    }
	    if ( object->value[0] == 4 ) {
		object->name = "mace";
	    }
	    if ( object->value[0] == 5 ) {
		object->name = "axe";
	    }
	    if ( object->value[0] == 6 ) {
		object->name = "flail";
	    }
	    if ( object->value[0] == 7 ) {
		object->name = "whip";
	    }
	    if ( object->value[0] == 8 ) {
		object->name = "polearm";
	    }

	    object->short_descr = capitalize ( object->name );
	    object->description = capitalize ( object->name );

	    m5 = "$n's PK token morphs into <<<MobGear>>.";
	    if ( m5 != NULL ) act ( m5, ch, NULL, victim, TO_ROOM );
	    send_to_char ( "Your MobGear arrives suddenly.\n\r", ch );
	    obj_to_char ( object, ch );
	    do_wear ( ch, object->name );
	    break;
	}
	if ( ( object->item_type != ITEM_DRINK_CON
	       && object->item_type != ITEM_TRASH )
	     &&
	     !( ( IS_OBJ_STAT ( object, ITEM_ANTI_EVIL ) && IS_EVIL ( ch ) )
		|| ( IS_OBJ_STAT ( object, ITEM_ANTI_GOOD )
		     && IS_GOOD ( ch ) )
		|| ( IS_OBJ_STAT ( object, ITEM_ANTI_NEUTRAL )
		     && IS_NEUTRAL ( ch ) ) ) ) {
		//m6 = "$n picks up $p and examines it carefully.";
	  //  if ( m6 != NULL ) act ( m6, ch, NULL, victim, TO_ROOM );
	    obj_from_room ( object );
	    obj_to_char ( object, ch );
	    for ( obj2 = ch->carrying; obj2 != NULL;
		  obj2 = obj2->next_content ) {
		if ( obj2->wear_loc != WEAR_NONE && can_see_obj ( ch, obj2 )
		     && object->item_type == obj2->item_type
		     && ( object->wear_flags & obj2->
			  wear_flags & ~ITEM_TAKE ) != 0 )
		    break;
	    }
	    if ( !obj2 ) {
		switch ( object->item_type ) {
		    default:
			if ( number_percent (  ) > 40 ) {
			    m7 = "Hey, what a find!";
			} else if ( number_percent (  ) > 30 && number_percent (  ) <= 40 ) {
			    m7 = "Hmm, this may be useful.";
			} else if ( number_percent (  ) > 20 && number_percent (  ) <= 30 ) {
			    m7 = "Probably more trash.";
			} else if ( number_percent (  ) > 10 && number_percent (  ) <= 20 ) {
			    m7 = "Someone needs to clean this place up.";
			} else {
			    m7 = "I hope this is worth picking up.";
			}
			break;
		    case ITEM_FOOD:
			m7 = "This looks tasty!";
			do_eat ( ch, object->name );
			drop_out = TRUE;
			break;
		    case ITEM_WAND:
			m7 = "Wow, another magic wand!";
			break;
		    case ITEM_STAFF:
			m7 = "Heh, another magic staff!";
			break;
		    case ITEM_WEAPON:
			if ( number_percent (  ) > 40 ) {
			    m7 = "Hey, this looks like a major upgrade!";
			} else if ( number_percent (  ) > 30 ) {
			    m7 = "I'll see if it's any good or not.";
			} else {
			    m7 = "It's not the best but it'll do for now.";
			}
			do_wear ( ch, object->name );
			do_second ( ch, object->name );
			break;
		    case ITEM_ARMOR:
			if ( number_percent (  ) > 40 ) {
			    m7 = "Oooh... nice piece of armor!";
			} else if ( number_percent (  ) > 30 ) {
			    m7 = "I'll see if it's any good or not.";
			} else {
			    m7 = "It's not the best but it'll do for now.";
			}
			do_wear ( ch, object->name );
			break;
		    case ITEM_POTION:
			if ( ch->level >= 102 ) {
			    m7 = "Great!  I was feeling a little thirsty!";
			    act ( "You quaff $p.", ch, object, NULL,
				  TO_CHAR );
			    act ( "$n quaffs $p.", ch, object, NULL,
				  TO_ROOM );
			    obj_cast_spell ( object->value[1],
					     object->level, ch, ch, NULL );
			    obj_cast_spell ( object->value[2],
					     object->level, ch, ch, NULL );
			    obj_cast_spell ( object->value[3],
					     object->level, ch, ch, NULL );
			    extract_obj ( object );
			    drop_out = TRUE;
			    break;
			} else {
			    break;
			}
		    case ITEM_SCROLL:
			m7 = "Hmmm I wonder what this says?";
			act ( "You recite $p.", ch, object, NULL, TO_CHAR );
			act ( "$n recites $p.", ch, object, NULL, TO_ROOM );
			obj_cast_spell ( object->value[1], object->level, ch,
					 NULL, object );
			obj_cast_spell ( object->value[2], object->level, ch,
					 NULL, object );
			obj_cast_spell ( object->value[3], object->level, ch,
					 NULL, object );
			extract_obj ( object );
			drop_out = TRUE;
			break;
		if ( m7 != NULL ) act ( m7, ch, NULL, victim, TO_ROOM );
		}
		return TRUE;
	    }

	    if ( drop_out )
		return TRUE;
	    if ( ( object->level > obj2->level ) ) {
		if ( number_percent (  ) > 40 ) {
		    m = "$n says '{aNow THIS looks like an improvement!";
		} else if ( number_percent (  ) > 30 ) {
		    m = "$n says '{aI've been wanting one of these.";
		} else {
m = "$n says '{aFinally found it! the stolen artifact of the Ancients!.";
		}
		do_wear ( ch, "all" );
	    } else {
		if ( number_percent (  ) > 50 ) {
		    m = "$n says '{aI don't want this piece of junk!";
		} else if ( number_percent (  ) > 40 ) {
		    m = "$n says '{aI wouldn't wear this if I was naked.";
		} else {
		    m = "$n says '{aMan what a trash dump!";
		}
		do_donate ( ch, object->name );
	    }
	    return TRUE;
	    if ( m != NULL )
		act ( m, ch, NULL, victim, TO_ALL );
	}
    }

    bllmax = ch->stance[STANCE_BULL] == 200;
    crbmax = ch->stance[STANCE_CRAB] == 200;
    crnmax = ch->stance[STANCE_CRANE] == 200;
    srpmax = ch->stance[STANCE_SERPENT] == 200;
    mngmax = ch->stance[STANCE_MONGOOSE] == 200;
    mnkmax = ch->stance[STANCE_MONKEY] == 200;
    swlmax = ch->stance[STANCE_SWALLOW] == 200;
    mntmax = ch->stance[STANCE_MANTIS] == 200;
    drgmax = ch->stance[STANCE_DRAGON] == 200;
    tgrmax = ch->stance[STANCE_TIGER] == 200;

    bllsty = IS_STANCE ( victim, STANCE_BULL );
    srpsty = IS_STANCE ( victim, STANCE_SERPENT );
    crbsty = IS_STANCE ( victim, STANCE_CRAB );
    crnsty = IS_STANCE ( victim, STANCE_CRANE );
    mngsty = IS_STANCE ( victim, STANCE_MONGOOSE );
    mnksty = IS_STANCE ( victim, STANCE_MONKEY );
    swlsty = IS_STANCE ( victim, STANCE_SWALLOW );
    mntsty = IS_STANCE ( victim, STANCE_MANTIS );
    drgsty = IS_STANCE ( victim, STANCE_DRAGON );
    tgrsty = IS_STANCE ( victim, STANCE_TIGER );

    chbllsty = IS_STANCE ( ch, STANCE_BULL );
    chsrpsty = IS_STANCE ( ch, STANCE_SERPENT );
    chcrbsty = IS_STANCE ( ch, STANCE_CRAB );
    chcrnsty = IS_STANCE ( ch, STANCE_CRANE );
    chmngsty = IS_STANCE ( ch, STANCE_MONGOOSE );
    chmnksty = IS_STANCE ( ch, STANCE_MONKEY );
    chswlsty = IS_STANCE ( ch, STANCE_SWALLOW );
    chmntsty = IS_STANCE ( ch, STANCE_MANTIS );
    chdrgsty = IS_STANCE ( ch, STANCE_DRAGON );
    chtgrsty = IS_STANCE ( ch, STANCE_TIGER );

    bllmin = ch->stance[STANCE_BULL] < 200;
    crbmin = ch->stance[STANCE_CRAB] < 200;
    crnmin = ch->stance[STANCE_CRANE] < 200;
    srpmin = ch->stance[STANCE_SERPENT] < 200;
    mngmin = ch->stance[STANCE_MONGOOSE] < 200;
    mnkmin = ch->stance[STANCE_MONKEY] < 200;
    swlmin = ch->stance[STANCE_SWALLOW] < 200;
    mntmin = ch->stance[STANCE_MANTIS] < 200;
    drgmin = ch->stance[STANCE_DRAGON] < 200;
    tgrmin = ch->stance[STANCE_TIGER] < 200;

    switch ( number_bits ( 1 ) ) {

	case 0:
	    if ( ch->position == POS_FIGHTING ) {
		if ( IS_STANCE ( victim, STANCE_NONE ) ) {
		    if ( chbllsty ) {
			return FALSE;
		    } else {
			if ( bllmax && crbmax ) {
			    if ( chdrgsty ) {
				return FALSE;
			    } else {
				do_stance ( ch, "dragon" );
				return TRUE;
			    }
			} else {
			    do_stance ( ch, "bull" );
			    return TRUE;
			}
		    }
		} else if ( srpsty ) {
		    if ( chcrbsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "crab" );
			return TRUE;
		    }
		} else if ( crnsty ) {
		    if ( chsrpsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "serpent" );
			return TRUE;
		    }
		} else if ( mngsty ) {
		    if ( chsrpsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "serpent" );
			return TRUE;
		    }
		} else if ( bllsty ) {
		    if ( chmngsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "mongoose" );
			return TRUE;
		    }
		} else if ( crbsty ) {
		    if ( chcrnsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "crane" );
			return TRUE;
		    }
		} else if ( mntsty && bllmin && crbmin ) {
		    if ( chsrpsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "serpent" );
			return TRUE;
		    }
		} else if ( tgrsty ) {
		    if ( chtgrsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "bull" );
			return TRUE;
		    }
		} else if ( drgsty ) {
		    if ( chmngsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "mongoose" );
			return TRUE;
		    }
		} else if ( mnksty ) {
		    if ( chbllsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "bull" );
			return TRUE;
		    }
		} else if ( swlsty ) {
		    if ( chcrnsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "crane" );
			return TRUE;
		    }
		} else if ( mntsty && bllmax && crbmax ) {
		    if ( chdrgsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "dragon" );
			return TRUE;
		    }
		    return TRUE;
		}
	    } else {
		if ( IS_STANCE ( victim, STANCE_NONE ) ) {
		    if ( bllsty ) {
			do_circle ( ch, "" );
			return FALSE;
		    } else {
			if ( bllmax && crbmax ) {
			    if ( chdrgsty ) {
				return FALSE;
			    } else {
				do_stance ( ch, "dragon" );
				return TRUE;
			    }
			} else {
			    do_stance ( ch, "bull" );
			    return TRUE;
			}
		    }
		} else if ( srpsty ) {
		    if ( chcrbsty ) {
			do_circle ( ch, "" );
			return FALSE;
		    } else {
			do_stance ( ch, "crab" );
			return TRUE;
		    }
		} else if ( crnsty ) {
		    if ( chsrpsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "serpent" );
			return TRUE;
		    }
		} else if ( mngsty ) {
		    if ( chsrpsty ) {
			do_circle ( ch, "" );
			return FALSE;
		    } else {
			do_stance ( ch, "serpent" );
			return TRUE;
		    }
		} else if ( bllsty ) {
		    if ( chmngsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "mongoose" );
			return TRUE;
		    }
		} else if ( crbsty ) {
		    if ( chcrnsty ) {
			do_circle ( ch, "" );
			return FALSE;
		    } else {
			do_stance ( ch, "crane" );
			return TRUE;
		    }
		} else if ( mntsty && bllmin && crbmin ) {
		    if ( chsrpsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "serpent" );
			return TRUE;
		    }
		} else if ( tgrsty ) {
		    if ( chtgrsty ) {
			do_circle ( ch, "" );
			return FALSE;
		    } else {
			do_stance ( ch, "bull" );
			return TRUE;
		    }
		} else if ( drgsty ) {
		    if ( chmngsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "mongoose" );
			return TRUE;
		    }
		} else if ( mnksty ) {
		    if ( chbllsty ) {
			do_circle ( ch, "" );
			return FALSE;
		    } else {
			do_stance ( ch, "bull" );
			return TRUE;
		    }
		} else if ( swlsty ) {
		    if ( chcrnsty ) {
			return FALSE;
		    } else {
			do_stance ( ch, "crane" );
			return TRUE;
		    }
		} else if ( mntsty && bllmax && crbmax ) {
		    if ( chdrgsty ) {
			do_circle ( ch, "" );
			return FALSE;
		    } else {
			do_stance ( ch, "dragon" );
			return TRUE;
		    }
		    return TRUE;
		}
	    }
    }

    if ( ch->position != POS_FIGHTING ) {
	switch ( number_range ( 0, 22 ) ) {
	    default:
		m = NULL;
		break;
	    case 0:
	  if (number_percent () > 50){
		m = "$n yells '{aI've been looking for you, $N!{x'";
		break;
		}
	  if (number_percent () > 40 && number_percent () <= 50){
		m = "$n says '{a$N, you really need some protection around here.{x'";
		break;
		}
	    case 1:
	  if (ch->alignment > 300 && victim->alignment <= 1000){
		m = "$n says '{aFriend, we need to stick together.{x'";
		break;
		} else {
		m = "With a scream of rage, $n attacks $N.";
		do_kill ( ch, victim->name );
		break;
		}
	    case 2:
	  if (ch->alignment < -300 && victim->alignment >= -1000 && victim->alignment < -300){
		m = "$n says '{aThe Priests of the Dark Circle have a message for you.{x'";
		break;
		} else {
		m = "Join with us $N.";
		do_kill ( ch, victim->name );
		break;
		}
	    case 3:
		m = "$n cracks $s knuckles and says '{SSo your feeling lucky?{x'";
		break;
	    case 4:
	    case 5:
		m = "$n says '{aThe shadow of death surrounds you stranger.{x'";
		break;
	    case 6:
		m = "$n says '{aLet's get this over with I got a lot to do.{x'";
		break;
	    case 7:
	    case 8:
		m = "$n says '{aWhat're you doing around here?{x'";
		break;
	    case 9:
	    case 10:
		m = "$n says '{aThere's no one to save you this time!{x'";
		break;
	}
	if ( m != NULL )
	    act ( m, ch, NULL, victim, TO_ALL );
    }

    switch ( number_bits ( 3 ) ) {
	case 0:
	    if ( ch->position == POS_FIGHTING ) {
		switch ( number_range ( 1, 8 ) ) {
		    case 1:
			do_disarm ( ch, "" );
			break;
		    case 2:
		    case 3:
		    case 4:
		    case 5:
			m = "$n's eyes flare bright {rred{x for a moment.";
			spell_dispel_magic ( skill_lookup
					     ( "dispel magic" ), ch->level,
					     ch, victim, TARGET_CHAR );
			break;
		    case 6:
			break;
		    case 7:
		    case 8:
			m = "$n's eyes flare bright {Rred{x for a moment.";
			spell_harm ( skill_lookup ( "harm" ), ch->level,
				     ch, victim, TARGET_CHAR );
			break;
			if ( m != NULL )
			    act ( m, ch, NULL, victim, TO_ALL );
		}
	    }
	case 1:

	    if ( ch->position == POS_FIGHTING )
		switch ( number_range ( 1, 8 ) ) {
		    case 1:
m = "$n says '{aFool, do you think you stand a chance against me?";
			do_circle ( ch, "" );
			break;
		    case 2:
		    case 3:
		    case 4:
			m = "$n says '{aThis fight truely shall be your last, $N!";
			break;
		    case 5:
			if ( ch->hit < ( ch->max_hit * 0.1 ) ) {
			    m = "$n says '{aYou may defeat me this time $N, but beware!";
			    break;
			} else {
			    m = "$n says '{aI hear the horsemen coming for you $N!";
			    break;
			}
			if ( ch->position == POS_FIGHTING ) {
			    if ( IS_AFFECTED ( ch, AFF_FAERIE_FIRE ) ) {
				m = "$n's eyes flare bright red for a moment.";
				spell_dispel_magic ( skill_lookup
						     ( "dispel magic" ),
						     ch->level, ch, ch,
						     TAR_CHAR_SELF );
			    } else if ( IS_AFFECTED ( ch, AFF_POISON ) ) {
				m = "$n's eyes flare bright blue for a moment.";
				spell_cure_poison ( skill_lookup
						    ( "cure poison" ),
						    ch->level, ch, ch,
						    TAR_CHAR_SELF );
			    } else if ( IS_AFFECTED ( ch, AFF_BLIND ) ) {
				m = "$n's eyes flare bright blue for a moment.";
				spell_cure_blindness ( skill_lookup
						       ( "cure blindness" ),
						       ch->level, ch, ch,
						       TAR_CHAR_SELF );
			    } else if ( IS_AFFECTED ( ch, AFF_CURSE ) ) {
				m = "$n's eyes flare bright green for a moment.";
				spell_remove_curse ( skill_lookup
						     ( "remove curse" ),
						     ch->level, ch, ch,
						     TAR_CHAR_SELF );
			    } else if ( !IS_AFFECTED ( ch, SHD_SANCTUARY ) ) {
				m = "$n's eyes flare bright white for a moment.";
				spell_sanctuary ( skill_lookup
						  ( "sanctuary" ), ch->level,
						  ch, ch, TAR_CHAR_SELF );
			    } else
				if ( !IS_AFFECTED
				     ( ch, skill_lookup ( "frenzy" ) ) ) {
				m = "$n's eyes flare bright yellow for a moment.";
				spell_frenzy ( skill_lookup ( "frenzy" ),
					       ch->level, ch, ch,
					       TAR_CHAR_SELF );
			    } else
				if ( !IS_AFFECTED
				     ( ch, skill_lookup ( "bless" ) ) ) {
				m = "$n's eyes flare bright blue for a moment.";
				spell_bless ( skill_lookup ( "bless" ),
					      ch->level, ch, ch,
					      TAR_CHAR_SELF );
			    } else
				if ( !IS_AFFECTED
				     ( ch, skill_lookup ( "stone skin" ) ) ) {
				m = "$n's eyes flare dull cyan for a moment.";
				spell_stone_skin ( skill_lookup
						   ( "stone skin" ),
						   ch->level, ch, ch,
						   TAR_CHAR_SELF );
			    } else
				if ( !IS_AFFECTED
				     ( ch, skill_lookup ( "armor" ) ) ) {
				m = "$n's eyes flare bright Silver for a moment.";
				spell_armor ( skill_lookup ( "armor" ),
					      ch->level, ch, ch,
					      TAR_CHAR_SELF );
			    } else
				if ( !IS_AFFECTED
				     ( ch, skill_lookup ( "shield" ) ) ) {
				m = "$n's eyes flare bright cyan for a moment.";
				spell_shield ( skill_lookup ( "shield" ),
					       ch->level, ch, ch,
					       TAR_CHAR_SELF );
			    } else
				if ( !IS_AFFECTED
				     ( victim, AFF_FAERIE_FIRE ) ) {
				m = "$n's eyes flare bright pink for a moment.";
				spell_faerie_fire ( skill_lookup
						    ( "faerie fire" ),
						    ch->level, ch, victim,
						    TAR_CHAR_SELF );
			    } else if ( !IS_AFFECTED ( victim, AFF_BLIND ) ) {
				m = "$n's eyes flare dull grey for a moment.";
				spell_blindness ( skill_lookup
						  ( "blindness" ), ch->level,
						  ch, victim, TAR_CHAR_SELF );
			    } else if ( !IS_AFFECTED ( victim, AFF_CURSE ) )
				m = "$n's eyes flare black for a moment.";
			    spell_curse ( skill_lookup ( "curse" ), ch->level,
					  ch, victim, TAR_CHAR_SELF );
			    if ( m != NULL )
				act ( m, ch, NULL, victim, TO_ALL );
			} else if ( ch->hit < ( ch->max_hit * 0.5 ) ) {
			    m = "$n's eyes flare bright blue for a moment.";

			    if ( mngmax && crbmax ) {
				if ( chswlsty ) {
				    do_circle ( ch, "" );
				    return FALSE;
				} else {
				    do_stance ( ch, "swallow" );
				    return TRUE;
				}
				return TRUE;
			    } else {
				do_stance ( ch, "crane" );
			    }
			    spell_heal ( skill_lookup ( "heal" ), ch->level,
					 ch, ch, TAR_CHAR_SELF );

			    if ( m != NULL )
				act ( m, ch, NULL, victim, TO_ALL );
			} else if ( ch->hit < ( ch->max_hit * 0.25 ) ) {
			    do_flee ( ch, "" );

			} else if ( ch->hit < ( ch->max_hit * 0.1 ) ) {
			    m = "$n's eyes flare bright green for a moment.";
			    spell_teleport ( skill_lookup ( "teleport" ),
					     ch->level, ch, ch,
					     TAR_CHAR_SELF );
			}
			if ( m != NULL )
			    act ( m, ch, NULL, victim, TO_ALL );
		}
    }

    for ( ;; ) {
	int min_level;

	switch ( number_bits ( 4 ) ) {
	    case 0:
		min_level = 0;
		spell = "blindness";
		break;
	    case 1:
		min_level = 3;
		spell = "chill touch";
		break;
	    case 2:
		min_level = 7;
		spell = "weaken";
		break;
	    case 3:
		min_level = 8;
		spell = "teleport";
		break;
	    case 4:
		min_level = 11;
		spell = "colour spray";
		break;
	    case 5:
		min_level = 12;
		spell = "change sex";
		break;
	    case 6:
		min_level = 13;
		spell = "energy drain";
		break;
	    case 7:
		min_level = 90;
		spell = "gas breath";
		break;
	    case 8:
		min_level = 90;
		spell = "fire breath";
		break;
	    case 9:
		min_level = 15;
		spell = "fireball";
		break;
	    case 10:
		min_level = 20;
		spell = "plague";
		break;
	    default:
		min_level = 20;
		spell = "acid blast";
		break;
	}
	if ( ch->level >= min_level )
	    break;
    }

    if ( ch->stunned )
	do_stance ( ch, "" );
    return FALSE;

    if ( ( sn = skill_lookup ( spell ) ) < 0 )
	return FALSE;
    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, victim, TARGET_CHAR );
    do_get ( ch, "all corpse" );
    do_wear ( ch, "all" );
    if (((victim->alignment < ch->alignment-500) || ( victim->alignment > ch->alignment+500)) && (victim->level+ 5  > ch->level ) ) {
	do_kill ( ch, victim->name );
    }
    do_circle ( ch, "" );
    multi_hit ( ch, victim, gsn_circle );
    return TRUE;
}

bool spec_prog_two (CHAR_DATA * ch)
{
  char buf[MSL];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;

  if (!IS_AWAKE (ch))
    return FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim->fighting == ch && number_bits (2) == 0)
	break;
      if (victim != ch && can_see (ch, victim) && number_bits (1) == 0)
	break;
      if (victim == NULL)
	return TRUE;
    }

  if (victim == NULL)
    return FALSE;

  bllmax = ch->stance[STANCE_BULL] == 200;
  crbmax = ch->stance[STANCE_CRAB] == 200;
  crnmax = ch->stance[STANCE_CRANE] == 200;
  srpmax = ch->stance[STANCE_SERPENT] == 200;
  mngmax = ch->stance[STANCE_MONGOOSE] == 200;
  mnkmax = ch->stance[STANCE_MONKEY] == 200;
  swlmax = ch->stance[STANCE_SWALLOW] == 200;
  mntmax = ch->stance[STANCE_MANTIS] == 200;
  drgmax = ch->stance[STANCE_DRAGON] == 200;
  tgrmax = ch->stance[STANCE_TIGER] == 200;

  bllsty = IS_STANCE (victim, STANCE_BULL);
  srpsty = IS_STANCE (victim, STANCE_SERPENT);
  crbsty = IS_STANCE (victim, STANCE_CRAB);
  crnsty = IS_STANCE (victim, STANCE_CRANE);
  mngsty = IS_STANCE (victim, STANCE_MONGOOSE);
  mnksty = IS_STANCE (victim, STANCE_MONKEY);
  swlsty = IS_STANCE (victim, STANCE_SWALLOW);
  mntsty = IS_STANCE (victim, STANCE_MANTIS);
  drgsty = IS_STANCE (victim, STANCE_DRAGON);
  tgrsty = IS_STANCE (victim, STANCE_TIGER);

  chbllsty = IS_STANCE (ch, STANCE_BULL);
  chsrpsty = IS_STANCE (ch, STANCE_SERPENT);
  chcrbsty = IS_STANCE (ch, STANCE_CRAB);
  chcrnsty = IS_STANCE (ch, STANCE_CRANE);
  chmngsty = IS_STANCE (ch, STANCE_MONGOOSE);
  chmnksty = IS_STANCE (ch, STANCE_MONKEY);
  chswlsty = IS_STANCE (ch, STANCE_SWALLOW);
  chmntsty = IS_STANCE (ch, STANCE_MANTIS);
  chdrgsty = IS_STANCE (ch, STANCE_DRAGON);
  chtgrsty = IS_STANCE (ch, STANCE_TIGER);

  bllmin = ch->stance[STANCE_BULL] < 200;
  crbmin = ch->stance[STANCE_CRAB] < 200;
  crnmin = ch->stance[STANCE_CRANE] < 200;
  srpmin = ch->stance[STANCE_SERPENT] < 200;
  mngmin = ch->stance[STANCE_MONGOOSE] < 200;
  mnkmin = ch->stance[STANCE_MONKEY] < 200;
  swlmin = ch->stance[STANCE_SWALLOW] < 200;
  mntmin = ch->stance[STANCE_MANTIS] < 200;
  drgmin = ch->stance[STANCE_DRAGON] < 200;
  tgrmin = ch->stance[STANCE_TIGER] < 200;

  switch (number_bits (1))
    {
    case 0:
      if (ch->position != POS_FIGHTING)
	{
	  if (IS_STANCE (victim, STANCE_NONE))
	    {
	      if (chbllsty)
		{
		  return FALSE;
		}
	      else
		{
		  if (bllmax && crbmax)
		    {
		      if (chdrgsty)
			{
			  return FALSE;
			}
		      else
			{
			  do_stance (ch, "dragon");
			  return TRUE;
			}
		    }
		  else
		    {
		      do_stance (ch, "bull");
		      return TRUE;
		    }
		}
	    }
	  else if (srpsty)
	    {
	      if (chcrbsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "crab");
		  return TRUE;
		}
	    }
	  else if (crnsty)
	    {
	      if (chsrpsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "serpent");
		  return TRUE;
		}
	    }
	  else if (mngsty)
	    {
	      if (chsrpsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "serpent");
		  return TRUE;
		}
	    }
	  else if (bllsty)
	    {
	      if (chmngsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "mongoose");
		  return TRUE;
		}
	    }
	  else if (crbsty)
	    {
	      if (chcrnsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "crane");
		  return TRUE;
		}
	    }
	  else if (mntsty && bllmin && crbmin)
	    {
	      if (chsrpsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "serpent");
		  return TRUE;
		}
	    }
	  else if (tgrsty)
	    {
	      if (chtgrsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "bull");
		  return TRUE;
		}
	    }
	  else if (drgsty)
	    {
	      if (chmngsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "mongoose");
		  return TRUE;
		}
	    }
	  else if (mnksty)
	    {
	      if (chbllsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "bull");
		  return TRUE;
		}
	    }
	  else if (swlsty)
	    {
	      if (chcrnsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "crane");
		  return TRUE;
		}
	    }
	  else if (mntsty && bllmax && crbmax)
	    {
	      if (chdrgsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "dragon");
		  return TRUE;
		}
	      return TRUE;
	    }
	}
      else
	{
	  if (IS_STANCE (victim, STANCE_NONE))
	    {
	      if (bllsty)
		{
		  do_circle (ch, "");
		  return FALSE;
		}
	      else
		{
		  if (bllmax && crbmax)
		    {
		      if (chdrgsty)
			{
			  return FALSE;
			}
		      else
			{
			  do_stance (ch, "dragon");
			  return TRUE;
			}
		    }
		  else
		    {
		      do_stance (ch, "bull");
		      return TRUE;
		    }
		}
	    }
	  else if (srpsty)
	    {
	      if (chcrbsty)
		{
		  do_circle (ch, "");
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "crab");
		  return TRUE;
		}
	    }
	  else if (crnsty)
	    {
	      if (chsrpsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "serpent");
		  return TRUE;
		}
	    }
	  else if (mngsty)
	    {
	      if (chsrpsty)
		{
		  do_circle (ch, "");
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "serpent");
		  return TRUE;
		}
	    }
	  else if (bllsty)
	    {
	      if (chmngsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "mongoose");
		  return TRUE;
		}
	    }
	  else if (crbsty)
	    {
	      if (chcrnsty)
		{
		  do_circle (ch, "");
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "crane");
		  return TRUE;
		}
	    }
	  else if (mntsty && bllmin && crbmin)
	    {
	      if (chsrpsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "serpent");
		  return TRUE;
		}
	    }
	  else if (tgrsty)
	    {
	      if (chtgrsty)
		{
		  do_circle (ch, "");
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "bull");
		  return TRUE;
		}
	    }
	  else if (drgsty)
	    {
	      if (chmngsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "mongoose");
		  return TRUE;
		}
	    }
	  else if (mnksty)
	    {
	      if (chbllsty)
		{
		  do_circle (ch, "");
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "bull");
		  return TRUE;
		}
	    }
	  else if (swlsty)
	    {
	      if (chcrnsty)
		{
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "crane");
		  return TRUE;
		}
	    }
	  else if (mntsty && bllmax && crbmax)
	    {
	      if (chdrgsty)
		{
		  do_circle (ch, "");
		  return FALSE;
		}
	      else
		{
		  do_stance (ch, "dragon");
		  return TRUE;
		}
	      return TRUE;
	    }
	}
    }
  switch (number_bits (3))
    {
    case 0:
      if (ch->position != POS_FIGHTING)
	{
	  switch (number_range (1, 8))
	    {
	    case 1:
	      do_disarm (ch, "");
	      break;
	    case 2:
	    case 3:
	    case 4:
	    case 5:
	      act ("$n's eyes flare bright {rred{x for a moment.", ch,
		   NULL, NULL, TO_ROOM);
	      spell_dispel_magic (skill_lookup ("dispel magic"), ch->level,
				  ch, victim, TARGET_CHAR);
	      break;
	    case 6:
	      break;
	    case 7:
	    case 8:
	      act ("$n's eyes flare bright {rred{x for a moment.", ch,
		   NULL, NULL, TO_ROOM);
	      spell_harm (skill_lookup ("harm"), ch->level, ch, victim,
			  TARGET_CHAR);
	      break;
	    }
	}
    case 1:
      if (ch->position == POS_FIGHTING)
	switch (number_range (1, 8))
	  {
	  case 1:
	    strcpy (buf, "Fool, do you think you stand a chance against me?");
	    do_say (ch, buf);
	    do_circle (ch, "");
	    break;
	  case 2:
	    if (IS_NPC (victim) && victim->position == POS_FIGHTING)
	      {
		sprintf (buf,
			 "I shall destroy you utterly for this, %s",
			 victim->short_descr);
	      }
	    else if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
	      {
		sprintf (buf,
			 "I shall destroy you utterly for this, %s",
			 victim->name);
	      }
	    do_say (ch, buf);
	    break;
	  case 3:
	    if (IS_NPC (victim) && victim->position == POS_FIGHTING)
	      {
		sprintf (buf, "%s shall pay for their arrogance!",
			 victim->short_descr);
	      }
	    else if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
	      {
		sprintf (buf, "%s shall pay for their arrogance!",
			 victim->name);
	      }
	    do_shout (ch, buf);
	    break;
	  case 4:
	    if (IS_NPC (victim) && victim->position == POS_FIGHTING)
	      {
		sprintf (buf,
			 "This fight truely shall be your last, %s!",
			 victim->short_descr);
	      }
	    else if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
	      {
		sprintf (buf,
			 "This fight truely shall be your last, %s!",
			 victim->name);
	      }
	    do_say (ch, buf);
	    break;
	  case 5:
	    if (ch->hit < (ch->max_hit * 0.1))
	      {
		if (IS_NPC (victim) && victim->position == POS_FIGHTING)
		  {
		    sprintf (buf,
			     "You may defeat me this time %s, but beware!",
			     victim->short_descr);
		  }
		else if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
		  {
		    sprintf (buf,
			     "You may defeat me this time %s, but beware!",
			     victim->name);
		  }
		do_say (ch, buf);
		break;
	      }
	    if (IS_NPC (victim) && victim->position == POS_FIGHTING)
	      {
		sprintf (buf, "I hear the horsemen coming for you %s!",
			 victim->short_descr);
	      }
	    else if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
	      {
		sprintf (buf, "I hear the horsemen coming for you %s!",
			 victim->name);
	      }
	    do_say (ch, buf);
	    break;
	  }
    case 2:
      if (ch->position == POS_FIGHTING)
	{
	  if (IS_AFFECTED (ch, AFF_FAERIE_FIRE))
	    {
	      act ("$n's eyes flare bright {rred{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_dispel_magic (skill_lookup ("dispel magic"),
				  ch->level, ch, ch, TAR_CHAR_SELF);
	    }
	  else if (IS_AFFECTED (ch, AFF_POISON))
	    {
	      act ("$n's eyes flare bright {bblue{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_cure_poison (skill_lookup ("cure poison"),
				 ch->level, ch, ch, TAR_CHAR_SELF);
	    }
	  else if (IS_AFFECTED (ch, AFF_BLIND))
	    {
	      act ("$n's eyes flare bright {bblue{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_cure_blindness (skill_lookup ("cure blindness"),
				    ch->level, ch, ch, TAR_CHAR_SELF);
	    }
	  else if (IS_AFFECTED (ch, AFF_CURSE))
	    {
	      act ("$n's eyes flare bright {ggreen{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_remove_curse (skill_lookup ("remove curse"),
				  ch->level, ch, ch, TAR_CHAR_SELF);
	    }
	  else if (!IS_AFFECTED (ch, SHD_SANCTUARY))
	    {
	      act ("$n's eyes flare bright {Wwhite{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_sanctuary (skill_lookup ("sanctuary"),
			       ch->level, ch, ch, TAR_CHAR_SELF);
	    }
	  else if (!is_affected (ch, skill_lookup ("frenzy")))
	    {
	      act ("$n's eyes flare bright {Yyellow{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_frenzy (skill_lookup ("frenzy"),
			    ch->level, ch, ch, TAR_CHAR_SELF);
	    }
	  else if (!is_affected (ch, skill_lookup ("bless")))
	    {
	      act ("$n's eyes flare bright {Bblue{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_bless (skill_lookup ("bless"), ch->level, ch, ch,
			   TAR_CHAR_SELF);
	    }
	  else if (!is_affected (ch, skill_lookup ("stone skin")))
	    {
	      act ("$n's eyes flare dull {ccyabn{x for a moment.", ch,
		   NULL, NULL, TO_ROOM);
	      spell_stone_skin (skill_lookup ("stone skin"), ch->level, ch,
				ch, TAR_CHAR_SELF);
	    }
	  else if (!is_affected (ch, skill_lookup ("armor")))
	    {
	      act ("$n's eyes flare bright {&Silver{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_armor (skill_lookup ("armor"), ch->level, ch, ch,
			   TAR_CHAR_SELF);
	    }
	  else if (!is_affected (ch, skill_lookup ("shield")))
	    {
	      act ("$n's eyes flare bright {Ccyan{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_shield (skill_lookup ("shield"),
			    ch->level, ch, ch, TAR_CHAR_SELF);
	    }
	  else if (!IS_AFFECTED (victim, AFF_FAERIE_FIRE))
	    {
	      act ("$n's eyes flare bright {Mpink{x for a moment.", ch,
		   NULL, NULL, TO_ROOM);
	      spell_faerie_fire (skill_lookup ("faerie fire"), ch->level,
				 ch, victim, TAR_CHAR_SELF);
	    }
	  else if (!IS_AFFECTED (victim, AFF_BLIND))
	    {
	      act ("$n's eyes flare dull {8grey{x for a moment.", ch,
		   NULL, NULL, TO_ROOM);
	      spell_blindness (skill_lookup ("blindness"), ch->level, ch,
			       victim, TAR_CHAR_SELF);
	    }
	  else if (!IS_AFFECTED (victim, AFF_CURSE))
	    {
	      act ("$n's eyes flare {8black{x for a moment.", ch, NULL,
		   NULL, TO_ROOM);
	      spell_curse (skill_lookup ("curse"), ch->level, ch, victim,
			   TAR_CHAR_SELF);
	    }
	  else if (ch->hit < (ch->max_hit * 0.5))
	    {

	      act ("$n's eyes flare bright {Bblue{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      if (mngmax && crbmax)
		{
		  if (chswlsty)
		    {
		      do_circle (ch, "");
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "swallow");
		      return TRUE;
		    }
		  return TRUE;
		}
	      else
		{
		  do_stance (ch, "crane");
		}

	      spell_heal (skill_lookup ("heal"), ch->level, ch, ch,
			  TAR_CHAR_SELF);
	    }
	  else if (ch->hit < (ch->max_hit * 0.25))
	    {
	      do_flee (ch, "");
	    }
	  else if (ch->hit < (ch->max_hit * 0.1))
	    {
	      act ("$n's eyes flare bright {Ggreen{x for a moment.",
		   ch, NULL, NULL, TO_ROOM);
	      spell_teleport (skill_lookup ("teleport"),
			      ch->level, ch, ch, TAR_CHAR_SELF);
	    }
	}
      // enter new case statement here
    }				// end of switch 


  for (;;)
    {
      int min_level;

      switch (number_bits (4))
	{
	case 0:
	  min_level = 0;
	  spell = "blindness";
	  break;
	case 1:
	  min_level = 3;
	  spell = "chill touch";
	  break;
	case 2:
	  min_level = 7;
	  spell = "weaken";
	  break;
	case 3:
	  min_level = 8;
	  spell = "teleport";
	  break;
	case 4:
	  min_level = 11;
	  spell = "colour spray";
	  break;
	case 5:
	  min_level = 12;
	  spell = "change sex";
	  break;
	case 6:
	  min_level = 13;
	  spell = "energy drain";
	  break;
	case 7:
	  min_level = 90;
	  spell = "gas breath";
	  break;
	case 8:
	  min_level = 90;
	  spell = "fire breath";
	  break;
	case 9:
	  min_level = 15;
	  spell = "fireball";
	  break;
	case 10:
	  min_level = 20;
	  spell = "plague";
	  break;
	default:
	  min_level = 20;
	  spell = "acid blast";
	  break;
	}

      if (ch->level >= min_level)
	break;
    }

  if (ch->stunned)
    do_stance (ch, "");
  return FALSE;

  if ((sn = skill_lookup (spell)) < 0)
    return FALSE;
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
  do_get (ch, "all corpse");
  do_wear (ch, "all");
  if (victim->alignment < -200 && victim->level > ch->level - 15)
    {
      do_kill (ch, victim->name);
    }
  do_circle (ch, "");
  multi_hit (ch, victim, gsn_circle);
  return TRUE;
}


      // LV1_A
      bool spec_prog_three (CHAR_DATA * ch)
      {
	char buf[MSL];
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char *spell;
	int sn;
	if (!IS_AWAKE (ch))
	  return FALSE;
	for (victim = ch->in_room->people; victim != NULL; victim = v_next)
	  {
	    v_next = victim->next_in_room;
	    if (victim->fighting == ch && number_bits (2) == 0)
	      break;
	    if (victim != ch && can_see (ch, victim) && number_bits (1) == 0)
	      break;
	    if (victim == NULL)
	      return TRUE;
	  }

	if (victim == NULL)
	  return FALSE;
	bllmax = ch->stance[STANCE_BULL] == 200;
	crbmax = ch->stance[STANCE_CRAB] == 200;
	crnmax = ch->stance[STANCE_CRANE] == 200;
	srpmax = ch->stance[STANCE_SERPENT] == 200;
	mngmax = ch->stance[STANCE_MONGOOSE] == 200;
	mnkmax = ch->stance[STANCE_MONKEY] == 200;
	swlmax = ch->stance[STANCE_SWALLOW] == 200;
	mntmax = ch->stance[STANCE_MANTIS] == 200;
	drgmax = ch->stance[STANCE_DRAGON] == 200;
	tgrmax = ch->stance[STANCE_TIGER] == 200;
	bllsty = IS_STANCE (victim, STANCE_BULL);
	srpsty = IS_STANCE (victim, STANCE_SERPENT);
	crbsty = IS_STANCE (victim, STANCE_CRAB);
	crnsty = IS_STANCE (victim, STANCE_CRANE);
	mngsty = IS_STANCE (victim, STANCE_MONGOOSE);
	mnksty = IS_STANCE (victim, STANCE_MONKEY);
	swlsty = IS_STANCE (victim, STANCE_SWALLOW);
	mntsty = IS_STANCE (victim, STANCE_MANTIS);
	drgsty = IS_STANCE (victim, STANCE_DRAGON);
	tgrsty = IS_STANCE (victim, STANCE_TIGER);
	chbllsty = IS_STANCE (ch, STANCE_BULL);
	chsrpsty = IS_STANCE (ch, STANCE_SERPENT);
	chcrbsty = IS_STANCE (ch, STANCE_CRAB);
	chcrnsty = IS_STANCE (ch, STANCE_CRANE);
	chmngsty = IS_STANCE (ch, STANCE_MONGOOSE);
	chmnksty = IS_STANCE (ch, STANCE_MONKEY);
	chswlsty = IS_STANCE (ch, STANCE_SWALLOW);
	chmntsty = IS_STANCE (ch, STANCE_MANTIS);
	chdrgsty = IS_STANCE (ch, STANCE_DRAGON);
	chtgrsty = IS_STANCE (ch, STANCE_TIGER);
	bllmin = ch->stance[STANCE_BULL] < 200;
	crbmin = ch->stance[STANCE_CRAB] < 200;
	crnmin = ch->stance[STANCE_CRANE] < 200;
	srpmin = ch->stance[STANCE_SERPENT] < 200;
	mngmin = ch->stance[STANCE_MONGOOSE] < 200;
	mnkmin = ch->stance[STANCE_MONKEY] < 200;
	swlmin = ch->stance[STANCE_SWALLOW] < 200;
	mntmin = ch->stance[STANCE_MANTIS] < 200;
	drgmin = ch->stance[STANCE_DRAGON] < 200;
	tgrmin = ch->stance[STANCE_TIGER] < 200;
	switch (number_bits (1))
	  {
	  case 0:
	    if (ch->position != POS_FIGHTING)
	      {
		if (IS_STANCE (victim, STANCE_NONE))
		  {
		    if (chbllsty)
		      {
			return FALSE;
		      }
		    else
		      {
			if (bllmax && crbmax)
			  {
			    if (chdrgsty)
			      {
				return FALSE;
			      }
			    else
			      {
				do_stance (ch, "dragon");
				return TRUE;
			      }
			  }
			else
			  {
			    do_stance (ch, "bull");
			    return TRUE;
			  }
		      }
		  }
		else if (srpsty)
		  {
		    if (chcrbsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "crab");
			return TRUE;
		      }
		  }
		else if (crnsty)
		  {
		    if (chsrpsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "serpent");
			return TRUE;
		      }
		  }
		else if (mngsty)
		  {
		    if (chsrpsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "serpent");
			return TRUE;
		      }
		  }
		else if (bllsty)
		  {
		    if (chmngsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "mongoose");
			return TRUE;
		      }
		  }
		else if (crbsty)
		  {
		    if (chcrnsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "crane");
			return TRUE;
		      }
		  }
		else if (mntsty && bllmin && crbmin)
		  {
		    if (chsrpsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "serpent");
			return TRUE;
		      }
		  }
		else if (tgrsty)
		  {
		    if (chtgrsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "bull");
			return TRUE;
		      }
		  }
		else if (drgsty)
		  {
		    if (chmngsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "mongoose");
			return TRUE;
		      }
		  }
		else if (mnksty)
		  {
		    if (chbllsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "bull");
			return TRUE;
		      }
		  }
		else if (swlsty)
		  {
		    if (chcrnsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "crane");
			return TRUE;
		      }
		  }
		else if (mntsty && bllmax && crbmax)
		  {
		    if (chdrgsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "dragon");
			return TRUE;
		      }
		    return TRUE;
		  }
	      }
	    else
	      {
		if (IS_STANCE (victim, STANCE_NONE))
		  {
		    if (bllsty)
		      {
			do_circle (ch, "");
			return FALSE;
		      }
		    else
		      {
			if (bllmax && crbmax)
			  {
			    if (chdrgsty)
			      {
				return FALSE;
			      }
			    else
			      {
				do_stance (ch, "dragon");
				return TRUE;
			      }
			  }
			else
			  {
			    do_stance (ch, "bull");
			    return TRUE;
			  }
		      }
		  }
		else if (srpsty)
		  {
		    if (chcrbsty)
		      {
			do_circle (ch, "");
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "crab");
			return TRUE;
		      }
		  }
		else if (crnsty)
		  {
		    if (chsrpsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "serpent");
			return TRUE;
		      }
		  }
		else if (mngsty)
		  {
		    if (chsrpsty)
		      {
			do_circle (ch, "");
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "serpent");
			return TRUE;
		      }
		  }
		else if (bllsty)
		  {
		    if (chmngsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "mongoose");
			return TRUE;
		      }
		  }
		else if (crbsty)
		  {
		    if (chcrnsty)
		      {
			do_circle (ch, "");
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "crane");
			return TRUE;
		      }
		  }
		else if (mntsty && bllmin && crbmin)
		  {
		    if (chsrpsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "serpent");
			return TRUE;
		      }
		  }
		else if (tgrsty)
		  {
		    if (chtgrsty)
		      {
			do_circle (ch, "");
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "bull");
			return TRUE;
		      }
		  }
		else if (drgsty)
		  {
		    if (chmngsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "mongoose");
			return TRUE;
		      }
		  }
		else if (mnksty)
		  {
		    if (chbllsty)
		      {
			do_circle (ch, "");
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "bull");
			return TRUE;
		      }
		  }
		else if (swlsty)
		  {
		    if (chcrnsty)
		      {
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "crane");
			return TRUE;
		      }
		  }
		else if (mntsty && bllmax && crbmax)
		  {
		    if (chdrgsty)
		      {
			do_circle (ch, "");
			return FALSE;
		      }
		    else
		      {
			do_stance (ch, "dragon");
			return TRUE;
		      }
		    return TRUE;
		  }
	      }
	  }
	switch (number_bits (3))
	  {
	  case 0:
	    if (ch->position != POS_FIGHTING)
	      {
		switch (number_range (1, 8))
		  {
		  case 1:
		    do_disarm (ch, "");
		    break;
		  case 2:
		  case 3:
		  case 4:
		  case 5:
		    act ("$n's eyes flare bright {rred{x for a moment.", ch,
			 NULL, NULL, TO_ROOM);
		    spell_dispel_magic (skill_lookup ("dispel magic"),
					ch->level, ch, victim, TARGET_CHAR);
		    break;
		  case 6:
		    break;
		  case 7:
		  case 8:
		    act ("$n's eyes flare bright {rred{x for a moment.", ch,
			 NULL, NULL, TO_ROOM);
		    spell_harm (skill_lookup ("harm"), ch->level, ch, victim,
				TARGET_CHAR);
		    break;
		  }
	      }
	  case 1:
	    if (ch->position == POS_FIGHTING)
	      switch (number_range (1, 8))
		{
		case 1:
		  strcpy (buf,
			  "Fool, do you think you stand a chance against me?");
		  do_say (ch, buf);
		  do_circle (ch, "");
		  break;
		case 2:
		  if (IS_NPC (victim) && victim->position == POS_FIGHTING)
		    {
		      sprintf (buf,
			       "I shall destroy you utterly for this, %s",
			       victim->short_descr);
		    }
		  else
		    if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
		    {
		      sprintf (buf,
			       "I shall destroy you utterly for this, %s",
			       victim->name);
		    }
		  do_say (ch, buf);
		  break;
		case 3:
		  if (IS_NPC (victim) && victim->position == POS_FIGHTING)
		    {
		      sprintf (buf, "%s shall pay for their arrogance!",
			       victim->short_descr);
		    }
		  else
		    if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
		    {
		      sprintf (buf, "%s shall pay for their arrogance!",
			       victim->name);
		    }
		  do_shout (ch, buf);
		  break;
		case 4:
		  if (IS_NPC (victim) && victim->position == POS_FIGHTING)
		    {
		      sprintf (buf,
			       "This fight truely shall be your last, %s!",
			       victim->short_descr);
		    }
		  else
		    if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
		    {
		      sprintf (buf,
			       "This fight truely shall be your last, %s!",
			       victim->name);
		    }
		  do_say (ch, buf);
		  break;
		case 5:
		  if (ch->hit < (ch->max_hit * 0.1))
		    {
		      if (IS_NPC (victim) && victim->position == POS_FIGHTING)
			{
			  sprintf (buf,
				   "You may defeat me this time %s, but beware!",
				   victim->short_descr);
			}
		      else
			if (!IS_NPC (victim)
			    && victim->position == POS_FIGHTING)
			{
			  sprintf (buf,
				   "You may defeat me this time %s, but beware!",
				   victim->name);
			}
		      do_say (ch, buf);
		      break;
		    }
		  if (IS_NPC (victim) && victim->position == POS_FIGHTING)
		    {
		      sprintf (buf, "I hear the horsemen coming for you %s!",
			       victim->short_descr);
		    }
		  else
		    if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
		    {
		      sprintf (buf, "I hear the horsemen coming for you %s!",
			       victim->name);
		    }
		  do_say (ch, buf);
		  break;
		}
	  case 2:
	    if (ch->position == POS_FIGHTING)
	      {
		if (IS_AFFECTED (ch, AFF_FAERIE_FIRE))
		  {
		    act ("$n's eyes flare bright {rred{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_dispel_magic (skill_lookup ("dispel magic"),
					ch->level, ch, ch, TAR_CHAR_SELF);
		  }
		else if (IS_AFFECTED (ch, AFF_POISON))
		  {
		    act ("$n's eyes flare bright {bblue{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_cure_poison (skill_lookup ("cure poison"),
				       ch->level, ch, ch, TAR_CHAR_SELF);
		  }
		else if (IS_AFFECTED (ch, AFF_BLIND))
		  {
		    act ("$n's eyes flare bright {bblue{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_cure_blindness (skill_lookup ("cure blindness"),
					  ch->level, ch, ch, TAR_CHAR_SELF);
		  }
		else if (IS_AFFECTED (ch, AFF_CURSE))
		  {
		    act ("$n's eyes flare bright {ggreen{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_remove_curse (skill_lookup ("remove curse"),
					ch->level, ch, ch, TAR_CHAR_SELF);
		  }
		else if (!IS_AFFECTED (ch, SHD_SANCTUARY))
		  {
		    act ("$n's eyes flare bright {Wwhite{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_sanctuary (skill_lookup ("sanctuary"),
				     ch->level, ch, ch, TAR_CHAR_SELF);
		  }
		else if (!is_affected (ch, skill_lookup ("frenzy")))
		  {
		    act ("$n's eyes flare bright {Yyellow{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_frenzy (skill_lookup ("frenzy"), ch->level, ch, ch,
				  TAR_CHAR_SELF);
		  }
		else if (!is_affected (ch, skill_lookup ("bless")))
		  {
		    act ("$n's eyes flare bright {Bblue{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_bless (skill_lookup ("bless"), ch->level, ch, ch,
				 TAR_CHAR_SELF);
		  }
		else if (!is_affected (ch, skill_lookup ("stone skin")))
		  {
		    act ("$n's eyes flare dull {ccyabn{x for a moment.", ch,
			 NULL, NULL, TO_ROOM);
		    spell_stone_skin (skill_lookup ("stone skin"), ch->level,
				      ch, ch, TAR_CHAR_SELF);
		  }
		else if (!is_affected (ch, skill_lookup ("armor")))
		  {
		    act ("$n's eyes flare bright {&Silver{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_armor (skill_lookup ("armor"), ch->level, ch, ch,
				 TAR_CHAR_SELF);
		  }
		else if (!is_affected (ch, skill_lookup ("shield")))
		  {
		    act ("$n's eyes flare bright {Ccyan{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_shield (skill_lookup ("shield"), ch->level, ch, ch,
				  TAR_CHAR_SELF);
		  }
		else if (!IS_AFFECTED (victim, AFF_FAERIE_FIRE))
		  {
		    act ("$n's eyes flare bright {Mpink{x for a moment.", ch,
			 NULL, NULL, TO_ROOM);
		    spell_faerie_fire (skill_lookup ("faerie fire"),
				       ch->level, ch, victim, TAR_CHAR_SELF);
		  }
		else if (!IS_AFFECTED (victim, AFF_BLIND))
		  {
		    act ("$n's eyes flare dull {8grey{x for a moment.", ch,
			 NULL, NULL, TO_ROOM);
		    spell_blindness (skill_lookup ("blindness"), ch->level,
				     ch, victim, TAR_CHAR_SELF);
		  }
		else if (!IS_AFFECTED (victim, AFF_CURSE))
		  {
		    act ("$n's eyes flare {8black{x for a moment.", ch, NULL,
			 NULL, TO_ROOM);
		    spell_curse (skill_lookup ("curse"), ch->level, ch,
				 victim, TAR_CHAR_SELF);
		  }
		else if (ch->hit < (ch->max_hit * 0.5))
		  {

		    act ("$n's eyes flare bright {Bblue{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    if (mngmax && crbmax)
		      {
			if (chswlsty)
			  {
			    do_circle (ch, "");
			    return FALSE;
			  }
			else
			  {
			    do_stance (ch, "swallow");
			    return TRUE;
			  }
			return TRUE;
		      }
		    else
		      {
			do_stance (ch, "crane");
		      }

		    spell_heal (skill_lookup ("heal"), ch->level, ch, ch,
				TAR_CHAR_SELF);
		  }
		else if (ch->hit < (ch->max_hit * 0.25))
		  {
		    do_flee (ch, "");
		  }
		else if (ch->hit < (ch->max_hit * 0.1))
		  {
		    act ("$n's eyes flare bright {Ggreen{x for a moment.",
			 ch, NULL, NULL, TO_ROOM);
		    spell_teleport (skill_lookup ("teleport"),
				    ch->level, ch, ch, TAR_CHAR_SELF);
		  }
	      }
	    // enter new case statement here
	  }			// end of switch 


	for (;;)
	  {
	    int min_level;
	    switch (number_bits (4))
	      {
	      case 0:
		min_level = 0;
		spell = "blindness";
		break;
	      case 1:
		min_level = 3;
		spell = "chill touch";
		break;
	      case 2:
		min_level = 7;
		spell = "weaken";
		break;
	      case 3:
		min_level = 8;
		spell = "teleport";
		break;
	      case 4:
		min_level = 11;
		spell = "colour spray";
		break;
	      case 5:
		min_level = 12;
		spell = "change sex";
		break;
	      case 6:
		min_level = 13;
		spell = "energy drain";
		break;
	      case 7:
		min_level = 90;
		spell = "gas breath";
		break;
	      case 8:
		min_level = 90;
		spell = "fire breath";
		break;
	      case 9:
		min_level = 15;
		spell = "fireball";
		break;
	      case 10:
		min_level = 20;
		spell = "plague";
		break;
	      default:
		min_level = 20;
		spell = "acid blast";
		break;
	      }

	    if (ch->level >= min_level)
	      break;
	  }

	if (ch->stunned)
	  do_stance (ch, "");
	return FALSE;
	if ((sn = skill_lookup (spell)) < 0)
	  return FALSE;
	(*skill_table[sn].spell_fun) (sn, ch->level, ch, victim, TARGET_CHAR);
	do_get (ch, "all corpse");
	do_wear (ch, "all");
	if (victim->alignment < -200 && victim->level > ch->level - 15)
	  {
	    do_kill (ch, victim->name);
	  }
	do_circle (ch, "");
	multi_hit (ch, victim, gsn_circle);
	return TRUE;
      }

      // DB7

      bool spec_circle (CHAR_DATA * ch)
      {
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	if (!IS_AWAKE (ch))
	  return FALSE;
	for (victim = ch->in_room->people; victim != NULL; victim = v_next)
	  {
	    v_next = victim->next_in_room;
	    if (victim != ch && can_see (ch, victim) && number_bits (1) == 0)
	      break;
	  }

	if (victim == NULL)
	  return FALSE;
	switch (number_bits (1))
	  {
	  case 0:
	    if (ch->position == POS_FIGHTING)
	      {
		if (IS_STANCE (victim, STANCE_NONE))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "bull");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_SERPENT))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "crab");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_CRANE))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "serpent");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_MONGOOSE))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "serpent");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_BULL))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "mongoose");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_CRAB))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "crane");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_MANTIS))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "serpent");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_TIGER))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "bull");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_DRAGON))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "mongoose");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_MONKEY))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "bull");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_SWALLOW))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "crane");
		    return TRUE;
		  }
		do_circle (ch, "");
	      }
	    else
	      {
		do_get (ch, "all");
		do_drop (ch, "corpse");
		do_get (ch, "all corpse");
		do_wear (ch, "all");
		if (victim->alignment > 200 && victim->level > ch->level - 10)
		  {
		    do_kill (ch, victim->name);
		  }
		return TRUE;
	      }
	  }

	return FALSE;
      }

      bool spec_wear (CHAR_DATA * ch)
      {
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	if (!IS_AWAKE (ch))
	  return FALSE;
	for (victim = ch->in_room->people; victim != NULL; victim = v_next)
	  {
	    v_next = victim->next_in_room;
	    if (victim != ch && can_see (ch, victim) && number_bits (1) == 0)
	      break;
	  }

	if (victim == NULL)
	  return FALSE;
	switch (number_bits (1))
	  {
	  case 0:
	    if (ch->position == POS_FIGHTING)
	      {
		if (IS_STANCE (victim, STANCE_NONE))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "bull");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_SERPENT))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "crab");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_CRANE))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "serpent");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_MONGOOSE))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "serpent");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_BULL))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "mongoose");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_CRAB))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "crane");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_MANTIS))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "serpent");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_TIGER))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "bull");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_DRAGON))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "mongoose");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_MONKEY))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "bull");
		    return TRUE;
		  }
		else if (IS_STANCE (victim, STANCE_SWALLOW))
		  {
		    do_stance (ch, "");
		    do_stance (ch, "crane");
		    return TRUE;
		  }

	      }
	    else
	      {
		do_get (ch, "all");
		do_drop (ch, "corpse");
		do_get (ch, "all corpse");
		do_wear (ch, "all");
		if (victim->alignment < -200
		    && victim->level > ch->level - 20)
		  {
		    do_kill (ch, victim->name);
		  }
		return TRUE;
	      }
	  }

	return FALSE;
      }

      bool spec_dark_magic (CHAR_DATA * ch)
      {
	OBJ_DATA *obj;
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	for (victim = ch->in_room->people; victim != NULL; victim = v_next)
	  {
	    v_next = victim->next_in_room;
	    if (victim != ch && can_see (ch, victim) && number_bits (1) == 0
		&& !IS_NPC (victim))
	      break;
	  }

	if (victim == NULL)
	  return FALSE;
	switch (number_bits (2))
	  {
	  case 0:
	    if (ch->position == POS_FIGHTING)
	      {
		do_wear (ch, "bone");
		do_recite (ch, "scroll");
		do_autostance (ch, "serpent");
		return TRUE;
	      }
	    else
	      {
		do_get (ch, "all");
		do_get (ch, "all corpse");
		do_wear (ch, "all");
		do_drop (ch, "white");
		do_sacrifice (ch, "white");
		do_drop (ch, "corpse");
		if ((obj = get_obj_here (ch, "corspe")) != NULL)
		  {
		    do_brandish (ch, "staff");
		  }
		if (victim->alignment > 200 && victim->level > ch->level - 20)
		  {
		    do_kill (ch, victim->name);
		  }
		return TRUE;
	      }

	  case 1:
	    if (ch->position == POS_FIGHTING)
	      {
		do_wear (ch, "bone");
		do_recite (ch, "scroll");
		do_autostance (ch, "serpent");
		return TRUE;
	      }

	    else
	      {
		do_wear (ch, "all");
		do_get (ch, "all");
		do_get (ch, "all corpse");
		do_drop (ch, "white");
		do_sacrifice (ch, "white");
		do_drop (ch, "corpse");
		do_wear (ch, "staff");
		do_brandish (ch, "staff");
		do_kill (ch, "good");
		return TRUE;
	      }
	  }

	return FALSE;
      }

      bool spec_questmaster (CHAR_DATA * ch)
      {
	if (ch->fighting != NULL)
	  return spec_prog_two (ch);
	return FALSE;
      }



      bool spec_clan_healer (CHAR_DATA * ch)
      {
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	if (!IS_AWAKE (ch))
	  return FALSE;
	for (victim = ch->in_room->people; victim != NULL; victim = v_next)
	  {
	    v_next = victim->next_in_room;
	    if (victim != ch && can_see (ch, victim) && number_bits (1) == 0
		&& !IS_NPC (victim) && victim->level < 201)
	      break;
	  }

	if (victim == NULL)
	  return FALSE;
	switch (number_bits (4))
	  {
	  case 0:
	    act ("$n utters the words 'saspell'.", ch, NULL, NULL, TO_ROOM);
	    spell_heal (skill_lookup ("heal"), ch->level, ch, victim,
			TARGET_CHAR);
	    act ("$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM);
	    spell_armor (skill_lookup ("armor"), ch->level, ch, victim,
			 TARGET_CHAR);
	    return TRUE;
	  case 1:
	    act ("$n utters the words 'judicandus eqtuyp'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_sanctuary (skill_lookup ("sanctuary"), ch->level, ch,
			     victim, TARGET_CHAR);
	    act ("$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM);
	    spell_bless (skill_lookup ("bless"), ch->level, ch, victim,
			 TARGET_CHAR);
	    return TRUE;
	  case 2:
	    act ("$n utters the words 'judicandus noselacri'.", ch, NULL,
		 NULL, TO_ROOM);
	    spell_cure_blindness (skill_lookup ("cure blindness"), ch->level,
				  ch, victim, TARGET_CHAR);
	    act ("$n utters the words 'judicandus dies'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_cure_light (skill_lookup ("cure light"), ch->level, ch,
			      victim, TARGET_CHAR);
	    act ("$n utters the words 'judicandus sausabru'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_cure_poison (skill_lookup ("cure poison"), ch->level, ch,
			       victim, TARGET_CHAR);
	    return TRUE;
	  case 3:
	    act ("$n utters the words 'judicandus dies'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_cure_light (skill_lookup ("cure light"), ch->level, ch,
			      victim, TARGET_CHAR);
	    return TRUE;
	  case 4:
	    act ("$n utters the words 'judicandus noselacri'.", ch, NULL,
		 NULL, TO_ROOM);
	    spell_cure_blindness (skill_lookup ("cure blindness"), ch->level,
				  ch, victim, TARGET_CHAR);
	    act ("$n utters the words 'judicandus sausabru'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_cure_poison (skill_lookup ("cure poison"), ch->level, ch,
			       victim, TARGET_CHAR);
	    return TRUE;
	  case 5:
	    act ("$n utters the words 'saspell'.", ch, NULL, NULL, TO_ROOM);
	    spell_heal (skill_lookup ("heal"), ch->level, ch, victim,
			TARGET_CHAR);
	    act ("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM);
	    spell_refresh (skill_lookup ("refresh"), ch->level, ch, victim,
			   TARGET_CHAR);
	    return TRUE;
	  case 6:
	    act ("$n utters the words 'judicandus noselacri'.", ch, NULL,
		 NULL, TO_ROOM);
	    spell_cure_blindness (skill_lookup ("cure blindness"), ch->level,
				  ch, victim, TARGET_CHAR);
	    act ("$n utters the words 'judicandus sausabru'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_cure_poison (skill_lookup ("cure poison"), ch->level, ch,
			       victim, TARGET_CHAR);
	    act ("$n utters the words 'judicandus eugzagz'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_cure_disease (skill_lookup ("cure disease"), ch->level, ch,
				victim, TARGET_CHAR);
	    return TRUE;
	  case 7:
	    act ("$n utters the words 'saspell'.", ch, NULL, NULL, TO_ROOM);
	    spell_heal (skill_lookup ("heal"), ch->level, ch, victim,
			TARGET_CHAR);
	    act ("$n utters the words 'saspell'.", ch, NULL, NULL, TO_ROOM);
	    spell_heal (skill_lookup ("heal"), ch->level, ch, victim,
			TARGET_CHAR);
	    act ("$n utters the words 'judicandus eqtuyp'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_sanctuary (skill_lookup ("sanctuary"), ch->level, ch,
			     victim, TARGET_CHAR);
	    return TRUE;
	  case 8:
	    act ("$n utters the words 'judicandus eqtuyp'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_sanctuary (skill_lookup ("sanctuary"), ch->level, ch,
			     victim, TARGET_CHAR);
	    act ("$n utters the words 'qwerty'.", ch, NULL, NULL, TO_ROOM);
	    spell_shield (skill_lookup ("shield"), ch->level, ch, victim,
			  TARGET_CHAR);
	    act ("$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM);
	    spell_armor (skill_lookup ("armor"), ch->level, ch, victim,
			 TARGET_CHAR);
	    return TRUE;
	  case 9:
	    act ("$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM);
	    spell_armor (skill_lookup ("armor"), ch->level, ch, victim,
			 TARGET_CHAR);
	    act ("$n utters the words 'saspell'.", ch, NULL, NULL, TO_ROOM);
	    spell_heal (skill_lookup ("heal"), ch->level, ch, victim,
			TARGET_CHAR);
	    return TRUE;
	  case 10:
	    act ("$n utters the words 'IceShield'.", ch, NULL, NULL, TO_ROOM);
	    spell_iceshield (skill_lookup ("iceshield"), ch->level, ch,
			     victim, TARGET_CHAR);
	    return TRUE;
	  case 11:
	    act ("$n utters the words 'FireShield'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_fireshield (skill_lookup ("fireshield"), ch->level, ch,
			      victim, TARGET_CHAR);
	    return TRUE;
	  case 12:
	    act ("$n utters the words 'ShockShield'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_shockshield (skill_lookup ("shockshield"), ch->level, ch,
			       victim, TARGET_CHAR);
	    return TRUE;
	  case 13:
	    do_get (ch, "all");
	    spell_enchant_armor (skill_lookup ("enchant armor"), ch->level,
				 ch, victim, TARGET_CHAR);
	    return TRUE;
	  case 14:
	    do_get (ch, "all");
	    spell_enchant_weapon (skill_lookup ("enchant weapon"), ch->level,
				  ch, victim, TARGET_CHAR);
	    return TRUE;
	  case 15:
	    act ("$n utters the words 'saspell'.", ch, NULL, NULL, TO_ROOM);
	    spell_heal (skill_lookup ("heal"), ch->level, ch, victim,
			TARGET_CHAR);
	    act ("$n utters the words 'saspell'.", ch, NULL, NULL, TO_ROOM);
	    spell_heal (skill_lookup ("heal"), ch->level, ch, victim,
			TARGET_CHAR);
	    act ("$n utters the word 'stone'.", ch, NULL, NULL, TO_ROOM);
	    spell_stone_skin (skill_lookup ("stone skin"), ch->level, ch,
			      victim, TARGET_CHAR);
	    act ("$n utters the words 'judicandus eqtuyp'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_sanctuary (skill_lookup ("sanctuary"), ch->level, ch,
			     victim, TARGET_CHAR);
	    act ("$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM);
	    spell_bless (skill_lookup ("bless"), ch->level, ch, victim,
			 TARGET_CHAR);
	    act ("$n utters the words 'qwerty'.", ch, NULL, NULL, TO_ROOM);
	    spell_shield (skill_lookup ("shield"), ch->level, ch, victim,
			  TARGET_CHAR);
	    act ("$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM);
	    spell_armor (skill_lookup ("armor"), ch->level, ch, victim,
			 TARGET_CHAR);
	    act ("$n utters the words 'IceShield'.", ch, NULL, NULL, TO_ROOM);
	    spell_iceshield (skill_lookup ("iceshield"), ch->level, ch,
			     victim, TARGET_CHAR);
	    act ("$n utters the words 'FireShield'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_fireshield (skill_lookup ("fireshield"), ch->level, ch,
			      victim, TARGET_CHAR);
	    act ("$n utters the words 'ShockShield'.", ch, NULL, NULL,
		 TO_ROOM);
	    spell_shockshield (skill_lookup ("shockshield"), ch->level, ch,
			       victim, TARGET_CHAR);
	    return TRUE;
	  }

	return FALSE;
      }


bool spec_ai_magic (CHAR_DATA * ch)
{
  char buf[MSL];
  OBJ_DATA *object;
  OBJ_DATA *obj2;
  OBJ_DATA *object_next;
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  bool drop_out = FALSE;

  for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
      v_next = victim->next_in_room;
      if (victim != ch && can_see (ch, victim) && !IS_NPC (victim))
	continue;
    }

  if (!IS_AWAKE (ch))
    return FALSE;

//*
/*
 *
 ************
 *
**/
// *


  for (object = ch->in_room->contents; object; object = object_next)
    {
      object_next = object->next_content;
      if (object == NULL)
	continue;

      if (!can_see_obj (ch, object))
	continue;

      if (!IS_SET (object->wear_flags, ITEM_TAKE))
	continue;

      if (object->item_type == ITEM_CORPSE_NPC)
	continue;

      if (object->item_type == ITEM_CORPSE_PC)
	continue;


//*
/*
 *
 ************
 *
**/
// *

      if (object->pIndexData->vnum == OVPKT1)
	{
	  do_say (ch, "This will get me to the next level!");
	  ch->level++;
	  extract_obj (object);
	  act ("You recieve a degree for the next level.", ch, NULL, NULL, TO_CHAR);
	  act ("{xThe Mob Guildmaster appears and awards $n with a degree for the next level.",
	     ch, ch->name, NULL, TO_ROOM);
	  break;
	}
      if (object->pIndexData->vnum == (OVPKT2 || IMPTOKEN))
	{
	  if (object->pIndexData->vnum == IMPTOKEN)
	    {
	      do_say (ch, "Thank you very much sir, your kindness will go a long way.");
	      ch->level++, ch->level++, ch->level++, ch->level++, ch->level++;
	      ch->level++, ch->level++, ch->level++, ch->level++, ch->level++;
	      object->level = ch->level;
	      object->value[0] = number_range (0, 8);
	      object->value[1] = ch->level / 3 + number_range (3, 12);
	      object->value[2] = ch->level / 3 + number_range (3, 18);
	      object->value[3] = number_range (0, 39);
	      object->value[4] = 479;
	      if (object->value[0] == 0)
		{ sprintf (buf, object->name, "Exotic"); }
	      if (object->value[0] == 1)
		{ sprintf (buf, object->name, "Sword"); }
	      if (object->value[0] == 2)
		{ sprintf (buf, object->name, "Dagger"); }
	      if (object->value[0] == 3)
		{ sprintf (buf, object->name, "Spear"); }
	      if (object->value[0] == 4)
		{ sprintf (buf, object->name, "Mace"); }
	      if (object->value[0] == 5)
		{ sprintf (buf, object->name, "Axe"); }
	      if (object->value[0] == 6)
		{ sprintf (buf, object->name, "Flail"); }
	      if (object->value[0] == 7)
		{ sprintf (buf, object->name, "Whip"); }
	      if (object->value[0] == 8)
		{ sprintf (buf, object->name, "Polearm"); }
	      free_string (object->name);
	      object->name = str_dup (buf);
	      if (object->value[0] == 0)
		{ sprintf (buf, object->short_descr, "Exotic"); }
	      if (object->value[0] == 1)
		{ sprintf (buf, object->short_descr, "Sword"); }
	      if (object->value[0] == 2)
		{ sprintf (buf, object->short_descr, "Dagger"); }
	      if (object->value[0] == 3)
		{ sprintf (buf, object->short_descr, "Spear"); }
	      if (object->value[0] == 4)
		{ sprintf (buf, object->short_descr, "Mace"); }
	      if (object->value[0] == 5)
		{ sprintf (buf, object->short_descr, "Axe"); }
	      if (object->value[0] == 6)
		{ sprintf (buf, object->short_descr, "Flail"); }
	      if (object->value[0] == 7)
		{ sprintf (buf, object->short_descr, "Whip"); }
	      if (object->value[0] == 8)
		{ sprintf (buf, object->short_descr, "Polearm"); }
	      free_string (object->short_descr);
	      object->short_descr = str_dup (buf);
	      if (object->value[0] == 0)
		{ sprintf (buf, object->description, "Exotic"); }
	      if (object->value[0] == 1)
		{ sprintf (buf, object->description, "Sword"); }
	      if (object->value[0] == 2)
		{ sprintf (buf, object->description, "Dagger"); }
	      if (object->value[0] == 3)
		{ sprintf (buf, object->description, "Spear"); }
	      if (object->value[0] == 4)
		{ sprintf (buf, object->description, "Mace"); }
	      if (object->value[0] == 5)
		{ sprintf (buf, object->description, "Axe"); }
	      if (object->value[0] == 6)
		{ sprintf (buf, object->description, "Flail"); }
	      if (object->value[0] == 7)
		{ sprintf (buf, object->description, "Whip"); }
	      if (object->value[0] == 8)
		{ sprintf (buf, object->description, "Polearm"); }
	      free_string (object->description);
	      object->description = str_dup (buf);
	      do_wear (ch, object->name);
	    }
	  else
	    {
	      if (object->pIndexData->vnum == OVPKT2)
		{
		  do_wear (ch, object->name);
		  extract_obj (object);
		  if (number_percent () > 50)
		    {
		      create_object (get_obj_index (MG_WEAPON), 0);
		      object->level = ch->level - 10;
		      object->value[0] = number_range (0, 8);
		      object->value[1] = ch->level / 5 + number_range (0, 8);
		      object->value[2] = ch->level / 5 + number_range (0, 12);
		      object->value[3] = number_range (0, 39);
		      object->value[4] = number_range (1, 512);
		      if (object->value[0] == 0)
			{ sprintf (buf, object->name, "Exotic"); }
		      if (object->value[0] == 1)
			{ sprintf (buf, object->name, "Sword"); }
		      if (object->value[0] == 2)
			{ sprintf (buf, object->name, "Dagger"); }
		      if (object->value[0] == 3)
			{ sprintf (buf, object->name, "Spear"); }
		      if (object->value[0] == 4)
			{ sprintf (buf, object->name, "Mace"); }
		      if (object->value[0] == 5)
			{ sprintf (buf, object->name, "Axe"); }
		      if (object->value[0] == 6)
			{ sprintf (buf, object->name, "Flail"); }
		      if (object->value[0] == 7)
			{ sprintf (buf, object->name, "Whip"); }
		      if (object->value[0] == 8)
			{ sprintf (buf, object->name, "Polearm"); }
		      free_string (object->name);
		      object->name = str_dup (buf);
		      if (object->value[0] == 0)
			{ sprintf (buf, object->short_descr, "Exotic"); }
		      if (object->value[0] == 1)
			{ sprintf (buf, object->short_descr, "Sword"); }
		      if (object->value[0] == 2)
			{ sprintf (buf, object->short_descr, "Dagger"); }
		      if (object->value[0] == 3)
			{ sprintf (buf, object->short_descr, "Spear"); }
		      if (object->value[0] == 4)
			{ sprintf (buf, object->short_descr, "Mace"); }
		      if (object->value[0] == 5)
			{ sprintf (buf, object->short_descr, "Axe"); }
		      if (object->value[0] == 6)
			{ sprintf (buf, object->short_descr, "Flail"); }
		      if (object->value[0] == 7)
			{ sprintf (buf, object->short_descr, "Whip"); }
		      if (object->value[0] == 8)
			{ sprintf (buf, object->short_descr, "Polearm"); }
		      free_string (object->short_descr);
		      object->short_descr = str_dup (buf);
		      if (object->value[0] == 0)
			{ sprintf (buf, object->description, "Exotic"); }
		      if (object->value[0] == 1)
			{ sprintf (buf, object->description, "Sword"); }
		      if (object->value[0] == 2)
			{ sprintf (buf, object->description, "Dagger"); }
		      if (object->value[0] == 3)
			{ sprintf (buf, object->description, "Spear"); }
		      if (object->value[0] == 4)
			{ sprintf (buf, object->description, "Mace"); }
		      if (object->value[0] == 5)
			{ sprintf (buf, object->description, "Axe"); }
		      if (object->value[0] == 6)
			{ sprintf (buf, object->description, "Flail"); }
		      if (object->value[0] == 7)
			{ sprintf (buf, object->description, "Whip"); }
		      if (object->value[0] == 8)
			{ sprintf (buf, object->description, "Polearm"); }
		      free_string (object->description);
		      object->description = str_dup (buf);
		    }
		  else
		    {
		      create_object (get_obj_index (MG_ARMOR), 0);
		      object->level = ch->level - 10;
		      if (number_percent () > 95)
			{
			  object->value[0] =
			    ch->level / 2 + (number_range (40, 100));
			  object->value[1] =
			    ch->level / 2 + (number_range (40, 100));
			  object->value[2] =
			    ch->level / 2 + (number_range (40, 100));
			  object->value[3] =
			    ch->level / 2 + (number_range (40, 100));
			}
		      else
			{
			  object->value[0] =
			    ch->level / 2 + (number_range (1, 30));
			  object->value[1] =
			    ch->level / 2 + (number_range (1, 30));
			  object->value[2] =
			    ch->level / 2 + (number_range (1, 30));
			  object->value[3] =
			    ch->level / 2 + (number_range (1, 30));
			}
		    }
		  act ("$n's PK token morphs into <<<MobGear>>.", ch,
		       NULL, NULL, TO_ROOM);
		  send_to_char ("Your MobGear arrives suddenly.\n\r", ch);
		  obj_to_char (object, ch);
		  do_wear (ch, object->name);
		  break;
		}

//*
/*
 *
 ************
 *
**/
// *

	      if ((object->item_type != ITEM_DRINK_CON && object->item_type != ITEM_TRASH)
		&& !((IS_OBJ_STAT (object, ITEM_ANTI_EVIL) 
		&& IS_EVIL (ch)) || (IS_OBJ_STAT (object, ITEM_ANTI_GOOD)
		&& IS_GOOD (ch)) || (IS_OBJ_STAT (object, ITEM_ANTI_NEUTRAL)
		&& IS_NEUTRAL (ch))))
		{
		  act ("$n picks up $p and examines it carefully.", ch, object, NULL, TO_ROOM);
		  obj_from_room (object);
		  obj_to_char (object, ch);
		  for (obj2 = ch->carrying; obj2 != NULL;
		       obj2 = obj2->next_content)
		    {
		      if (obj2->wear_loc != WEAR_NONE
			  && can_see_obj (ch, obj2)
			  && object->item_type == obj2->item_type
			  && (object->wear_flags & obj2->
			      wear_flags & ~ITEM_TAKE) != 0)
			break;
		    }
		  if (!obj2)
		    {
		      switch (object->item_type)
			{
			default:
			  if (number_percent () > 40)
			    { strcpy (buf, "Hey, what a find!"); }
			  else if (number_percent () > 30)
			    { strcpy (buf, "Hmm, this may be useful."); }
			  else if (number_percent () > 20)
			    { strcpy (buf, "Probably more trash."); }
			  else if (number_percent () > 10)
			    { strcpy (buf, "Someone needs to clean this place up."); }
			  else
			    { strcpy (buf, "I hope this is worth picking up."); }
			  do_say (ch, buf);
			  break;
			case ITEM_FOOD:
			  strcpy (buf, "This looks tasty!");
			  do_say (ch, buf);
			  do_eat (ch, object->name);
			  drop_out = TRUE;
			  break;
			case ITEM_WAND:
			  strcpy (buf, "Wow, another magic wand!");
			  do_say (ch, buf);
			  break;
			case ITEM_STAFF:
			  strcpy (buf, "Heh, another magic staff!");
			  do_say (ch, buf);
			  break;
			case ITEM_WEAPON:
			  if (number_percent () > 40)
			    { strcpy (buf, "Hey, this looks like a major upgrade!"); }
			  else if (number_percent () > 30)
			    { strcpy (buf, "I'll see if it's any good or not."); }
			  else
			    { strcpy (buf, "It's not the best but it'll do for now."); }
			  do_say (ch, buf);
			  do_wear (ch, object->name);
			  do_second (ch, object->name);
			  break;
			case ITEM_ARMOR:
			  if (number_percent () > 40)
			    { strcpy (buf, "Oooh... nice piece of armor!"); }
			  else if (number_percent () > 30)
			    { strcpy (buf, "I'll see if it's any good or not."); }
			  else
			    { strcpy (buf, "It's not the best but it'll do for now."); }
			  do_say (ch, buf);
			  do_wear (ch, object->name);
			  break;
			case ITEM_POTION:
			  if (ch->level >= 102)
			    { strcpy (buf, "Great!  I was feeling a little thirsty!");
			      do_say (ch, buf);
			      act ("You quaff $p.", ch, object, NULL, TO_CHAR);
			      act ("$n quaffs $p.", ch, object, NULL, TO_ROOM);
			      obj_cast_spell (object->value[1], object->level, ch, ch, NULL);
			      obj_cast_spell (object->value[2], object->level, ch, ch, NULL);
			      obj_cast_spell (object->value[3], object->level, ch, ch, NULL);
			      extract_obj (object);
			      drop_out = TRUE;
			      break;
			    }
			  else { break; }
			case ITEM_SCROLL:
			  strcpy (buf, "Hmmm I wonder what this says?");
			  do_say (ch, buf);
			  act ("You recite $p.", ch, object, NULL, TO_CHAR);
			  act ("$n recites $p.", ch, object, NULL, TO_ROOM);
			  obj_cast_spell (object->value[1], object->level,
					  ch, NULL, object);
			  obj_cast_spell (object->value[2], object->level,
					  ch, NULL, object);
			  obj_cast_spell (object->value[3], object->level,
					  ch, NULL, object);
			  extract_obj (object);
			  drop_out = TRUE;
			  break;
			}
		      return TRUE;
		    }

		  if (drop_out)
		    return TRUE;
		  if ((object->level > obj2->level))
		    {
		      if (number_percent () > 40)
			{ strcpy (buf, "Now THIS looks like an improvement!"); }
		      else if (number_percent () > 30)
			{ strcpy (buf, "I've been wanting one of these."); }
		      else
			{ strcpy (buf, "Finally found it!."); }
		      do_say (ch, buf);
		      do_wear (ch, "all");
		    }
		  else
		    {
		      if (number_percent () > 50)
			{ strcpy (buf, "I don't want this piece of junk!"); }
		      else if (number_percent () > 40)
			{ strcpy (buf, "I wouldn't wear this if I was naked."); }
		      else
			{ strcpy (buf, "Man what a trash dump!"); }
		      do_say (ch, buf);
		      do_donate (ch, object->name);
		    }
		  return TRUE;
		}

//*
/*
 *
 ************
 *
	bllmax = ch->stance[STANCE_BULL] == 200;
	crbmax = ch->stance[STANCE_CRAB] == 200;
	crnmax = ch->stance[STANCE_CRANE] == 200;
	srpmax = ch->stance[STANCE_SERPENT] == 200;
	mngmax = ch->stance[STANCE_MONGOOSE] == 200;
	mnkmax = ch->stance[STANCE_MONKEY] == 200;
	swlmax = ch->stance[STANCE_SWALLOW] == 200;
	mntmax = ch->stance[STANCE_MANTIS] == 200;
	drgmax = ch->stance[STANCE_DRAGON] == 200;
	tgrmax = ch->stance[STANCE_TIGER] == 200;
	bllsty = IS_STANCE (victim, STANCE_BULL);
	srpsty = IS_STANCE (victim, STANCE_SERPENT);
	crbsty = IS_STANCE (victim, STANCE_CRAB);
	crnsty = IS_STANCE (victim, STANCE_CRANE);
	mngsty = IS_STANCE (victim, STANCE_MONGOOSE);
	mnksty = IS_STANCE (victim, STANCE_MONKEY);
	swlsty = IS_STANCE (victim, STANCE_SWALLOW);
	mntsty = IS_STANCE (victim, STANCE_MANTIS);
	drgsty = IS_STANCE (victim, STANCE_DRAGON);
	tgrsty = IS_STANCE (victim, STANCE_TIGER);
	bllsty = IS_STANCE (victim, STANCE_BULL);
	chbllsty = IS_STANCE (ch, STANCE_BULL);
	chsrpsty = IS_STANCE (ch, STANCE_SERPENT);
	chcrbsty = IS_STANCE (ch, STANCE_CRAB);
	chcrnsty = IS_STANCE (ch, STANCE_CRANE);
	chmngsty = IS_STANCE (ch, STANCE_MONGOOSE);
	chmnksty = IS_STANCE (ch, STANCE_MONKEY);
	chswlsty = IS_STANCE (ch, STANCE_SWALLOW);
	chmntsty = IS_STANCE (ch, STANCE_MANTIS);
	chdrgsty = IS_STANCE (ch, STANCE_DRAGON);
	chtgrsty = IS_STANCE (ch, STANCE_TIGER);
	bllsty = IS_STANCE (victim, STANCE_BULL);
	chbllsty = IS_STANCE (ch, STANCE_BULL);
	bllmin = ch->stance[STANCE_BULL] < 200;
	crbmin = ch->stance[STANCE_CRAB] < 200;
	crnmin = ch->stance[STANCE_CRANE] < 200;
	srpmin = ch->stance[STANCE_SERPENT] < 200;
	mngmin = ch->stance[STANCE_MONGOOSE] < 200;
	mnkmin = ch->stance[STANCE_MONKEY] < 200;
	swlmin = ch->stance[STANCE_SWALLOW] < 200;
	mntmin = ch->stance[STANCE_MANTIS] < 200;
	drgmin = ch->stance[STANCE_DRAGON] < 200;
	tgrmin = ch->stance[STANCE_TIGER] < 200;

	bllmax = ch->stance[STANCE_BULL] == 200;
	bllsty = IS_STANCE (victim, STANCE_BULL);
	chbllsty = IS_STANCE (ch, STANCE_BULL);
	bllmin = ch->stance[STANCE_BULL] < 200;
	csbull = IS_STANCE (victim, STANCE_BULL) && IS_STANCE (ch, STANCE_BULL) != NULL

**/
// *


	  if (victim == NULL)
	    return FALSE;
	  if (victim != NULL)
	    {
	      if (IS_STANCE (victim, STANCE_NONE))
		{
		  if (IS_STANCE (ch, STANCE_BULL))
		    { return FALSE; }
		  else
		    {
		      if (ch->stance[STANCE_BULL] == 200
			  && ch->stance[STANCE_CRAB] == 200)
			{
			  if (IS_STANCE (ch, STANCE_DRAGON))
			    { return FALSE; }
			  else
			    {
			      do_stance (ch, "dragon");
			      return TRUE;
			    }
			}
		      else
			{
			  do_stance (ch, "bull");
			  return TRUE;
			}
		    }
		}
	      else if (IS_STANCE (victim, STANCE_SERPENT))
		{
		  if (IS_STANCE (ch, STANCE_CRAB))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "crab");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_CRANE))
		{
		  if (IS_STANCE (ch, STANCE_SERPENT))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "serpent");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_MONGOOSE))
		{
		  if (IS_STANCE (ch, STANCE_SERPENT))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "serpent");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_BULL))
		{
		  if (IS_STANCE (ch, STANCE_MONGOOSE))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "mongoose");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_CRAB))
		{
		  if (IS_STANCE (ch, STANCE_CRANE))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "crane");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_MANTIS)
		       && ch->stance[STANCE_BULL] < 200
		       && ch->stance[STANCE_CRAB] < 200)
		{
		  if (IS_STANCE (ch, STANCE_SERPENT))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "serpent");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_TIGER))
		{
		  if (IS_STANCE (ch, STANCE_TIGER))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "bull");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_DRAGON))
		{
		  if (IS_STANCE (ch, STANCE_MONGOOSE))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "mongoose");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_MONKEY))
		{
		  if (IS_STANCE (ch, STANCE_BULL))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "bull");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_SWALLOW))
		{
		  if (IS_STANCE (ch, STANCE_CRANE))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "crane");
		      return TRUE;
		    }
		}
	      else if (IS_STANCE (victim, STANCE_MANTIS)
		       && ch->stance[STANCE_BULL] == 200
		       && ch->stance[STANCE_CRAB] == 200)
		{
		  if (IS_STANCE (ch, STANCE_DRAGON))
		    {
		      return FALSE;
		    }
		  else
		    {
		      do_stance (ch, "dragon");
		      return TRUE;
		    }
		  do_get (ch, "all corpse");
		  do_get (ch, "all");
		  do_drop (ch, "corpse");
		  do_wear (ch, "all");
		  return TRUE;
		}
	    }

//*
/*
 *
 ************
 *
**/
// *


	  switch (number_bits (3))
	    {
	    case 0:
	      if (ch->position != POS_FIGHTING)
		{
		  switch (number_range (1, 8))
		    {
		    case 1:
		      do_disarm (ch, "");
		      break;
		    case 2:
		    case 3:
		    case 4:
		    case 5:
		      act ("$n's eyes flare bright {rred{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_dispel_magic (skill_lookup ("dispel magic"),
					  ch->level, ch, victim, TARGET_CHAR);
		      break;
		    case 6:
		      break;
		    case 7:
		    case 8:
		      act ("$n's eyes flare bright {rred{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_harm (skill_lookup ("harm"), ch->level, ch,
				  victim, TARGET_CHAR);
		      break;
		    }
		}
	    case 1:
	      if (ch->position == POS_FIGHTING)
		switch (number_range (1, 6))
		  {
		  case 1:
	//	    strcpy (buf,
	//		    "Fool, do you think you stand a chance against me?");
		    do_say (ch, "Fool, do you think you stand a chance against me?");
	//	    do_say (ch, buf);
		    do_circle (ch, "");
		    break;
		  case 2:
		    if (IS_NPC (victim) && victim->position == POS_FIGHTING)
		      {
			sprintf (buf, "I shall destroy you utterly for this, %s",
				 victim->short_descr);
		      }
		    else if (!IS_NPC (victim) && victim->position == POS_FIGHTING)
		      {
			sprintf (buf, "I shall destroy you utterly for this, %s",
				 victim->name);
		      }
		    do_say (ch, buf);
		    break;
		  case 3:
		    if (IS_NPC (victim) && victim->position == POS_FIGHTING)
		      {
			sprintf (buf,
				 "%s shall pay for their arrogance!",
				 victim->short_descr);
		      }
		    else if (!IS_NPC (victim)
			     && victim->position == POS_FIGHTING)
		      {
			sprintf (buf,
				 "%s shall pay for their arrogance!",
				 victim->name);
		      }
		    do_shout (ch, buf);
		    break;
		  case 4:
		    if (IS_NPC (victim) && victim->position == POS_FIGHTING)
		      {
			sprintf (buf,
				 "This fight truely shall be your last, %s!",
				 victim->short_descr);
		      }
		    else if (!IS_NPC (victim)
			     && victim->position == POS_FIGHTING)
		      {
			sprintf (buf,
				 "This fight truely shall be your last, %s!",
				 victim->name);
		      }
		    do_say (ch, buf);
		    break;
		  case 5:
		    if (ch->hit < (ch->max_hit * 0.1))
		      {
			if (IS_NPC (victim)
			    && victim->position == POS_FIGHTING)
			  {
			    sprintf (buf,
				     "You may defeat me this time %s, but beware!",
				     victim->short_descr);
			  }
			else if (!IS_NPC (victim)
				 && victim->position == POS_FIGHTING)
			  {
			    sprintf (buf,
				     "You may defeat me this time %s, but beware!",
				     victim->name);
			  }
			do_say (ch, buf);
			break;
		      }
		  case 6:
		    if (victim->hit < (victim->max_hit * 0.1))
		      {
			if (IS_NPC (victim)
			    && victim->position == POS_FIGHTING)
			  {
			    sprintf (buf,
				     "I hear the horsemen coming for you %s!",
				     victim->short_descr);
			  }
			else if (!IS_NPC (victim)
				 && victim->position == POS_FIGHTING)
			  {
			    sprintf (buf,
				     "I hear the horsemen coming for you %s!",
				     victim->name);
			  }
			do_say (ch, buf);
			break;
		      }
		    // case 7: 
		  }
//*
/*
 *
 ************
 *
**/
// *
	    case 2:
	      if (ch->position == POS_FIGHTING)
		{
		  if (IS_AFFECTED (ch, AFF_FAERIE_FIRE))
		    {
		      act ("$n's eyes flare bright {rred{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_dispel_magic (skill_lookup ("dispel magic"),
					  ch->level, ch, ch, TAR_CHAR_SELF);
		    }
		  else if (IS_AFFECTED (ch, AFF_POISON))
		    {
		      act ("$n's eyes flare bright {bblue{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_cure_poison (skill_lookup ("cure poison"),
					 ch->level, ch, ch, TAR_CHAR_SELF);
		    }
		  else if (IS_AFFECTED (ch, AFF_BLIND))
		    {
		      act ("$n's eyes flare bright {bblue{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_cure_blindness (skill_lookup ("cure blindness"),
					    ch->level, ch, ch, TAR_CHAR_SELF);
		    }
		  else if (IS_AFFECTED (ch, AFF_CURSE))
		    {
		      act ("$n's eyes flare bright {ggreen{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_remove_curse (skill_lookup ("remove curse"),
					  ch->level, ch, ch, TAR_CHAR_SELF);
		    }
		  if (!IS_AFFECTED (ch, SHD_SANCTUARY))
		    {
		      act ("$n's eyes flare bright {Wwhite{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_sanctuary (skill_lookup ("sanctuary"), ch->level,
				       ch, ch, TAR_CHAR_SELF);
		    }
		  if (!is_affected (ch, skill_lookup ("frenzy")))
		    {
		      act ("$n's eyes flare bright {Yyellow{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_frenzy (skill_lookup ("frenzy"), ch->level, ch,
				    ch, TAR_CHAR_SELF);
		    }
		  if (!is_affected (ch, skill_lookup ("bless")))
		    {
		      act ("$n's eyes flare bright {Bblue{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_bless (skill_lookup ("bless"), ch->level, ch, ch,
				   TAR_CHAR_SELF);
		    }
		  if (!is_affected (ch, skill_lookup ("stone skin")))
		    {
		      act ("$n's eyes flare dull {ccyabn{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_stone_skin (skill_lookup ("stone skin"),
					ch->level, ch, ch, TAR_CHAR_SELF);
		    }
		  if (!is_affected (ch, skill_lookup ("armor")))
		    {
		      act ("$n's eyes flare bright {&Silver{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_armor (skill_lookup ("armor"), ch->level, ch, ch,
				   TAR_CHAR_SELF);
		    }
		  if (!is_affected (ch, skill_lookup ("shield")))
		    {
		      act ("$n's eyes flare bright {Ccyan{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_shield (skill_lookup ("shield"), ch->level, ch,
				    ch, TAR_CHAR_SELF);
		    }
		  if (!IS_AFFECTED (victim, AFF_FAERIE_FIRE))
		    {
		      act ("$n's eyes flare bright {Mpink{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_faerie_fire (skill_lookup ("faerie fire"),
					 ch->level, ch, victim,
					 TAR_CHAR_SELF);
		    }
		  if (!IS_AFFECTED (victim, AFF_BLIND))
		    {
		      act ("$n's eyes flare dull {8grey{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_blindness (skill_lookup ("blindness"),
				       ch->level, ch, victim, TAR_CHAR_SELF);
		    }
		  else if (!IS_AFFECTED (victim, AFF_CURSE))
		    {
		      act ("$n's eyes flare {8black{x for a moment.", ch,
			   NULL, NULL, TO_ROOM);
		      spell_curse (skill_lookup ("curse"), ch->level, ch,
				   victim, TAR_CHAR_SELF);
		    }
		  else if (ch->hit < (ch->max_hit * 0.5))
		    {

		      act ("$n's eyes flare bright {Bblue{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      if (mngmax && crbmax)
			{
			  if (chswlsty)
			    {
			      do_circle (ch, "");
			      return FALSE;
			    }
			  else
			    {
			      do_stance (ch, "swallow");
			      return TRUE;
			    }
			  return TRUE;
			}
		      else
			{
			  do_stance (ch, "crane");
			}

		      spell_heal (skill_lookup ("heal"), ch->level, ch, ch,
				  TAR_CHAR_SELF);
		    }
		  else if (ch->hit < (ch->max_hit * 0.25))
		    {
		      do_flee (ch, "");
		    }
		  else if (ch->hit < (ch->max_hit * 0.1))
		    {
		      act ("$n's eyes flare bright {Ggreen{x for a moment.",
			   ch, NULL, NULL, TO_ROOM);
		      spell_teleport (skill_lookup ("teleport"), ch->level,
				      ch, ch, TAR_CHAR_SELF);
		    }

		}
	    }
	    }
	    }
}
return FALSE;
}

