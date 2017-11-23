
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
/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.03. Please do not remove this notice from this file. *
****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"


#define QUEST_ITEM1 21500
#define QUEST_ITEM2 21501
#define QUEST_ITEM3 21502
#define QUEST_ITEM4 21503
#define QUEST_ITEM5 21504
#define QUEST_ITEM6 21505
#define QUEST_ITEM7 21510

#define QUEST_OBJQUEST1 21505
#define QUEST_OBJQUEST2 21506
#define QUEST_OBJQUEST3 21507
#define QUEST_OBJQUEST4 21508
#define QUEST_OBJQUEST5 21509
// #define QUEST_MOBQUEST1 28801

// Need a global define for the qxp timer. -- Arzlin
int xpq_timer;
int prquest_timer;
int damq_timer;
/* tables for level quest object names */
char *	const	adj_table	[] =
{
    "light ",
    "heavy ",
    "emaculate ",
    "dark ",
    "red ",
    "worn ",
    "old ",
    "golden ",
    "silver ",
    "tarnished ",
    "obsidian ",
    "black ",
    "ancient ",
    "smelly ",
    "stone ",
    "weathered ",
    "enchanted "
};

char *	const	qot_table	[] =
{
    " coin",
    " tablet",
    " parchment",
    " sword",
    " feather",
    " knife",
    " amulet",
    " ring",
    " bracelet",
    " necklace",
    " disc",
    " staff",
    " idol",
    " figurine",
    " scroll",
    " flame"
};

char *	const	pho_table	[] =
{
" ", "a", "ac", "ah", "aj", "ar", "as", 
"av", "ax", "abeh", "afil", "ahe", "akuh",
" ", "b", "ba", "be", "bi", "bo", "bu", 
"bh", "bl", "br", "bt", "bv", 
"baha", "buka", "bal", "baj", "bac", "bru", 
"bej", "beku", "bihu", "byhe", 
" ", "c", "ce", "ch", "ci", "ck", "ch", 
"cn", "co", "cr", "ct", "cu", "cx", "cyuz", 
"che", "cev", "cae", "cah", "caj", "cal", 
"cas", "cav", "cax", "caz", "ceb", 
" ", "d", "dh", "dr", "da", "de", "di", 
"do", "du", "duj", "dek", "daj", "dex", 
"dox", "duv", "dayh", "dyaz",
" ", "e", "ej", "ek", "el", "eh", "eruh", "etol", "edyh", 
" ", "f", "fl", "fr", "fej", "fah", "fir", 
"fan", "fyah", "feyr", "fha", "fah",
" ", "g", "ge", "gi", "gu", "guyh", "gla", 
"gna", "gra", "gnu", "geh", "gha", "ghe", "gem", "gen",
" ", "h", "ha", "ho", "hy", "hil", "hax", 
"hoz", "hev", "hav", "haj", "hej",
" ", "i", "ic", "ij", "ik", "il", "it", "iku", "ivo", "ixo", 
" ", "j", "jy", "jo", "ju", "jh", "jhu", 
"jhul", "jihr", "jehs", "jeht", "juhv", "jehax", "jahyz", "jai",
" ", "k", "ka", "ke", "ki", "ko", "ku", 
"ky", "kla", "kru", "kel", "kha", "kax", 
"ken", "kaj", "kau", "kae", "kaf", "kag", 
"kah", "kal", "kan", "kas", "kav",
" ", "l", "lev", "la", "le", "li", "lo", 
"lu", "ly", "leh", "lok", "ll", "lr", "ls", 
"lt", "lv", "ly", "lom", "lon", "loh", "luz", 
"lai", "lah", "laj", "lan", 
"lar", "las", "lat", "lav", "laz", "lah", 
" ", "m", "ma", "me", "mi", "mo", "mu", 
"my", "mc", "mun", "myha", "mhe", "mhu", 
"may", "max", "mal", "mab", "maf", "mag", 
"mah", "mai", "maj", "mak", "mar", "mas", "mev", "mav", 
" ", "n", "ny", "ni", "nah", "nac", "nai", 
"naj", "nak", "nav", "nev",
" ", "o", "oa", "oca", "ohr", "olu", "oti", 
"ovy", "oxu", "odoh", "ofet", "omn", "opa", "oqu", "oru", "obe", 
" ", "p", "pa", "pe", "pi", "po", "pu", 
"py", "phay", "phez", "pl", "pr", "pah", 
"pai", "paj", "pal", "pas", "pau", "pav", "pax", "paz", "phn", 
" ", "qu", "qua", "quy", "qub", "quc", "quh", 
"quj", "qul", "qum", "qun", "qur", "qut", "quv", "qux", "quz",
" ", "r", "ra", "re", "ri", "ro", "ru", "ry",
" ", "s", "sa", "se", "si", "so", "su", 
"sy", "shi", "sha", "shu", "shy", "sen", "sac", 
" ", "t", "ta", "te", "ti", "to", "tu", "ty",
" ", "u", "uc", "uh", "uj", "uk", "ul", 
"ur", "ubeh", "ucor", "urac", "ufeh", "uvo",
" ", "v", "va", "ve", "vi", "vo", "vu", "vy", 
" ", "w", "wa", "we", "wi", "wo", "wu", "wy", 
" ", "x", "xa", "xe", "xi", "xo", "xu", "xy", 
" ", "y", "yh", "yr", "yn", "yx", "yz", "yb", 
"yc", "yd", "yagh", "yabr", "yaqu",
" ", "z", "za", "ze", "zi", "zo", "zu", "zy",
};

/*
 * Local functions.
 */
char *	find_quest_mob		args( ( CHAR_DATA *ch, bool lquest ) );
char *	make_lquest_obj		args( ( CHAR_DATA *ch ) );
bool	is_suitable_qmob	args( ( CHAR_DATA *ch,
				    CHAR_DATA *victim, bool lquest ) );
void generate_quest     args(( CHAR_DATA *ch, CHAR_DATA *questman ));
bool quest_level_diff   args(( int clevel, int mlevel));
bool chance             args(( int num ));
ROOM_INDEX_DATA         *find_location( CHAR_DATA *ch, char *arg );

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;  
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf  [MSL];
	char buf2 [MIL];
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [9];
    int cnt;
    unsigned int plc;
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0')
    {
   sprintf(buf, "{c%s, {BQuest commands are{b: {RPoints Info Time Request Complete List Buy Quit and Aquit, {mFor more information, type {r'{x{WHelp Autoquest{x'.",
				ch->name); 
				send_to_char(buf, ch);
        		return;
    }
    if (!strcmp(arg1, "info"))
    {
        if (IS_SET(ch->exbit1_flags, PLR_QUESTOR))
        {
            if (ch->questmob == -1 && ch->questgiver->short_descr != NULL)
            {
		sprintf(buf,"Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r",ch->questgiver->short_descr);
                send_to_char(buf, ch);
            }
            else if (ch->questobj > 0)
            {
                questinfoobj = get_obj_index(ch->questobj);
                if (questinfoobj != NULL)
                {
		    sprintf(buf, "You are on a quest to recover the fabled %s!\n\r",questinfoobj->name);
                    send_to_char(buf, ch);
                }
                else send_to_char("You aren't currently on a quest.\n\r",ch);
                return;
            }
            else if (ch->questmob > 0)
            {
                questinfo = get_mob_index(ch->questmob);
                if (questinfo != NULL)
                {
                    sprintf(buf, "You are on a quest to slay the dreaded %s!\n\r",questinfo->short_descr);
                    send_to_char(buf, ch);
                }
                else send_to_char("You aren't currently on a quest.\n\r",ch);
                return;
            }
        }
        else
            send_to_char("You aren't currently on a quest.\n\r",ch);
        return;
    }
    if (!strcmp(arg1, "points"))
    {
        sprintf(buf, "You have %d quest points.\n\r",ch->questpoints);
        send_to_char(buf, ch);
        return;
    }
    else if (!strcmp(arg1, "time"))
    {
        if (!IS_SET(ch->exbit1_flags, PLR_QUESTOR))
        {
            send_to_char("You aren't currently on a quest.\n\r",ch);
            if (ch->nextquest > 1)
            {
                sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r",ch->nextquest);
                send_to_char(buf, ch);
            }
            else if (ch->nextquest == 1)
            {
                sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
                send_to_char(buf, ch);
            }
        }
        else if (ch->aqcountdown > 0)
        {
            sprintf(buf, "Time left for current aquest: %d\n\r",ch->aqcountdown);
            send_to_char(buf, ch);
        }
        else if (ch->qcountdown > 0)
        {
            sprintf(buf, "Time left for current quest: %d\n\r",ch->qcountdown);
            send_to_char(buf, ch);
        }
        return;
    }

	if (!strcmp(arg1, "quit")) 
    {
		if (!IS_SET(ch->exbit1_flags,PLR_QUESTOR))
		{
	    	send_to_char("You aren't currently on a quest.\n\r",ch);
		return;
		}
		sprintf(buf,"Being a difficult quest it is understandable.\n\r");
		send_to_char(buf, ch);
		sprintf(buf,"One minute has been added to your quest time.\n\r");
		send_to_char(buf, ch);
	    	REMOVE_BIT(ch->exbit1_flags,PLR_QUESTOR);
	    	ch->questgiver = NULL;
	    	ch->qcountdown = 0;
	    	ch->questmob = 0;
		ch->questobj = 0;
	    	ch->nextquest += 1;
		return;
	}

	if (!strcmp(arg1, "aquit")) 
    	{
	/*
		if (!IS_SET(ch->act,PLR_LQUEST ) && (!ch->pcdata->is_aquest ) ) 
		{
		    send_to_char("You aren't currently on a aquest.\n\r",ch);
		    return;
		}
		else 
		{
			sprintf(buf,"Being a difficult quest it is understandable.\n\r");
			REMOVE_BIT(ch->act,PLR_LQUEST);
			ch->can_aquest = 2;
			ch->pcdata->is_aquest = FALSE;			
       
			return;
		}
	 */
 
                if (!IS_SET(ch->act, PLR_LQUEST ) && (!ch->pcdata->is_aquest ) )
                {
                    send_to_char("You aren't currently on a aquest.\n\r",ch);
                    return;
                }
                else
                {
                sprintf(buf,"Being a difficult quest it is understandable.\n\r");
                send_to_char(buf, ch);
                REMOVE_BIT(ch->act, PLR_LQUEST);
                ch->pcdata->is_aquest = FALSE;
                if ( ch->level == 50 || ch->level == 100 )
                {
                ch->can_aquest = 1;
                SET_BIT(ch->act, PLR_LQUEST);
                }
                else
                {
                ch->can_aquest = 2;
                }
                return;
                }
	}

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
        if (!IS_NPC(questman)) continue;
        if (questman->spec_fun == spec_lookup( "spec_questmaster" )) break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" ))
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

    if ( questman->fighting != NULL)
    {
        send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }

    ch->questgiver = questman;

    if (!strcmp(arg1, "list"))
    {
        act( "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM);
        act ("You ask $N for a list of quest items.",ch, NULL, questman, TO_CHAR);
	do_help(ch,"questlist");
        return;
    }
/*
	case 3:

	    token2 = get_eq_char(killer, WEAR_HOLD);
	    token4 = create_object(get_obj_index(OVPKT4), 0);

	    if (!IS_NPC(ch)
		&& token2 != NULL && token2->pIndexData->vnum == OVPKT2) {
		sprintf(buf, token4->short_descr, killer->name, ch->name);
		free_string(token4->short_descr, "fight.c/death_cry #4");
		token4->short_descr = str_dup(buf);
		sprintf(buf, token4->description, killer->name, ch->name);
		free_string(token4->description, "fight.c/death_cry #5");
		token4->description = str_dup(buf);
		sprintf(buf, token4->name, killer->name, ch->name);
		free_string(token4->name, "fight.c/death_cry #6");
		token4->name = str_dup(buf);
		obj_from_char(token2);
		extract_obj(token2);
		obj_to_char(token4, killer);
		DAZE_STATE(killer, 17 + (5 * PULSE_VIOLENCE));
	    } else if (!IS_NPC(ch)) {
		vnum = OVPKT2;
	    } else {
		vnum = 0;
	    }
	    break;
 */

    else if (!strcmp(arg1, "buy"))
    {
        if (arg2[0] == '\0')
        {
            send_to_char("To buy an item, type 'QUEST BUY <item>'.\n\r",ch);
            return;
        }
        if (is_name(arg2, "amulet"))
        {
            if (ch->questpoints >= 375)
            {
                ch->questpoints -= 375;
                obj = create_object(get_obj_index(QUEST_ITEM1),ch->level);
            }
            else
            {
            sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
            do_say(questman,buf);
            return;
            }
        }
        else if (is_name(arg2, "shield"))
        {
            if (ch->questpoints >= 375)
            {
                ch->questpoints -= 375;
                obj = create_object(get_obj_index(QUEST_ITEM2),ch->level);
            }
            else
            {
	sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
	do_say(questman,buf);
	return;
            }
        }

        else if (is_name(arg2, "ring"))
        {
            if (ch->questpoints >= 425)
            {
                ch->questpoints -= 425;
                obj = create_object(get_obj_index(QUEST_ITEM6),ch->level);
            }
            else
            {
	sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
	do_say(questman,buf);
	return;
            }
        }

        else if (is_name(arg2, "sword"))
        {
            if (ch->questpoints >= 425)
            {
                ch->questpoints -= 425;
                obj = create_object(get_obj_index(QUEST_ITEM3),ch->level);
		REMOVE_BIT(obj->extra_flags, ITEM_LQUEST);
            }
            else
            {
	sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
	do_say(questman,buf);
	return;
            }
        }

        else if (is_name(arg2, "decanter endless sustenance"))
        {
            if (ch->questpoints >= 550)
            {
                ch->questpoints -= 550;
                obj = create_object(get_obj_index(QUEST_ITEM4),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "rocking chair"))
        {
            if (ch->questpoints >= 500)
            {
                ch->questpoints -= 500;
                obj = create_object(get_obj_index(QUEST_ITEM5),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }

        else if (is_name(arg2, "token"))
        {
            if (ch->questpoints >= 10)
            {
                ch->questpoints -= 10;
                obj = create_object(get_obj_index(QUEST_ITEM7),ch->level);
            }
            else
            {
	sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
	do_say(questman,buf);
	return;
            }
        }



        else if (is_name(arg2, "pconvert player"))
        {
            if (ch->qps >= 100)
            {
                ch->qps -= 100;
                ch->questpoints += 500;
         	act( "$N gives 500 questpoints to $n.", ch, NULL, questman,TO_ROOM );
         	act( "$N gives you 500 questpoints.",   ch, NULL, questman,TO_CHAR );
                return;
            }
            else
            {
         	sprintf(buf, "Sorry, %s, but you don't have enough Global quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }

        else if (is_name(arg2, "gconvert"))
        {
            if (ch->questpoints >= 500)
            {
                ch->questpoints -= 500;
                ch->qps += 100;
         	act( "$N gives 100 Global qps to $n.", ch, NULL, questman,TO_ROOM );
         	act( "$N gives you 100 Global qps.",   ch, NULL, questman,TO_CHAR );
                return;
            }
            else
            {
         	sprintf(buf, "Sorry, %s, but you don't have enough Player quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }

        else if (is_name(arg2, "aquest global"))
        {

 		if (((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp) >= 5000)
		{
		sprintf(buf, "Go work off some of that exp first %s.",ch->name);
		do_say(questman,buf);
		return;
		}

		if (ch->exp < 5000)
		{
		sprintf(buf, "I don't think your experienced enough yet %s.",ch->name);
		do_say(questman,buf);
		return;
		}
			else if (ch->class < MCLT_1 && (ch->level < 10))
			{
				sprintf(buf, "Why don't you go outside and play %s.",ch->name);
               			do_say(questman,buf);
				return;
			}
			else if (ch->aqps < 10 && ch->aqps > 0)
			{
			sprintf(buf, "You don't have enough for that %s!",ch->name);
            		do_say(questman,buf);
			return;
			}
			else if (ch->aqps <= 0 )
			{
			sprintf(buf, "%s, You've run out of auto quest points!",ch->name);
            		do_say(questman,buf);
			return;
			}
			else if (ch->exp >= 5000)
            {
                ch->aqps -= 10;
                ch->qps += 100;
         	act( "$N gives 100 Global Quest points to $n.", ch, NULL, questman,TO_ROOM );
         	act( "$N gives you 100 Global qps.",   ch, NULL, questman,TO_CHAR );
                return;
            }
            else
            {
         	sprintf(buf, "Sorry, %s, but you don't have enough Auto Quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }

        else if (is_name(arg2, "global"))
        {

 		if (((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp) >= 5000)
		{
		sprintf(buf, "Go work off some of that exp first %s.",ch->name);
		do_say(questman,buf);
		return;
		}

		if (ch->exp < 10000)
		{
		sprintf(buf, "I don't think your experienced enough yet %s.",ch->name);
		do_say(questman,buf);
		return;
		}
		else if ((ch->level == LEVEL_HERO) && (ch->class < MCLT_2))
		{
		sprintf(buf, "%s! Your going to have to find another way to get those.",ch->name);
		do_say(questman,buf);
		return;
		}
			else if (ch->class < MCLT_1 && (ch->level < 10))
			{
			sprintf(buf, "Why don't you go outside and play %s.",ch->name);
                	do_say(questman,buf);
			return;
			}
			else if (ch->exp >= 5000)
            {
                ch->exp -= 5000;
                ch->qps += 50;
         	act( "$N gives 50 Global qps to $n.", ch, NULL, questman,TO_ROOM );
         	act( "$N gives you 50 Global qps.",   ch, NULL, questman,TO_CHAR );
                return;
            }
            else
            {
         	sprintf(buf, "Sorry, %s, but you don't have enough Experience for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }

    if ( !str_prefix( arg2, "reroll" ) )
    {
	int clasb;
	if (IS_NPC(ch))
		{ send_to_char("Mobiles have no clasb.\n\r",ch); return; }
	if ( ch->level < 101 && ch->pcdata->tier < 1 )
		{ send_to_char( "Get to level 101 first.\n\r", ch); return; }
	clasb = class_lookup(arg3);
	if ( clasb == -1 )
	{
		char buf[MSL];
		strcpy( buf, "Possible classes are: " );
	  for ( clasb = 0; clasb < MAX_CLASS; clasb++ )
		{
	    if ( clasb > 0 ) 
		strcat( buf, " " ); strcat( buf, class_table[clasb].name );
		}
		strcat( buf, ".\n\r" );
		send_to_char(buf,ch);
		return;
	}
	if ( ch->level > 101 ) {
		ch->clasb = clasb; 
		return;
		} else { 
		ch->clasb = clasb; 
		do_remove( ch, "all");
		ch->exp = 1000, ch->level -= 100, change_level( ch );
		ch->hit = 100, ch->max_hit = 100, ch->pcdata->perm_hit = 100; 
		ch->max_mana = 100, ch->pcdata->perm_mana = 100; 
		ch->max_move = 100, ch->pcdata->perm_move = 100;
		return;
		}
    }
    
        else if (!str_prefix(arg2, "who" ))  
    	
    	{
	    if ( IS_NPC(ch) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }
	    if (ch->level < 101)
	    {
		send_to_char( "Get to level 101 first.\n\r", ch);
		return;
	    }
	    buf[0] = '\0';
	    buf2[0] = '\0';
    	    ch->pcdata->who_descr = str_dup( capitalize( "" ) );
	    if (arg3[0] == '\0')
	    {
		return;
	    }
	    cnt = 0;
	    for(plc = 0; plc < strlen(arg3); plc++)
	    {
		if (arg3[plc] != '{')
		{
		    if (buf[0] == '\0')
		    {
			sprintf(buf2, "%c", arg3[plc]);
		    } else
		    {
			sprintf(buf2, "%s%c", buf, arg3[plc]);
		    }
		    sprintf(buf, "%s", buf2);
		    cnt++;
		} else if (arg3[plc+1] == '{')
		{
		    if (buf[0] == '\0')
		    {
			sprintf(buf2, "{{");
		    } else
		    {
			sprintf(buf2, "%s{{", buf);
		    }
		    sprintf(buf, "%s", buf2);
		    cnt++;
		    plc++;
		} else
		{
		    if (buf[0] == '\0')
		    {
			sprintf(buf2, "{%c", arg3[plc+1]);
		    } else
		    {
			sprintf(buf2, "%s{%c", buf, arg3[plc+1]);
		    }
		    sprintf(buf, "%s", buf2);
		    plc++;
		}
		/*
		if (cnt >= 10)
		{
		    plc = strlen(arg3);
		}
		 */
	    }
	    sprintf(buf2, "%s{0", buf);
	    sprintf(buf, "%s", buf2);
	    while (cnt < 10)
	    {
		sprintf(buf2, "%s ", buf);
		sprintf(buf, "%s", buf2);
		cnt++;
	    }
	    ch->pcdata->who_descr = str_dup (capitalize(buf)); 
	    buf[0] = '\0';
	    buf2[0] = '\0';
    	    return;
    	}
        else if (is_name(arg2, "wv1"))
	{
	    OBJ_DATA *obj;
    for ( questman = ch->in_room->people; 
	  questman != NULL; 
	  questman = questman->next_in_room)
    if (IS_NPC(questman) && (questman->spec_fun == spec_lookup("spec_questmaster")))
	    break;
    if (questman == NULL || !can_see(ch,questman))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }
    if ( ( obj = get_obj_carry( ch, arg3 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }
    if (ch->qps < 200) 
    {
	send_to_char( "V1 cost's 200 gquest points, you do not have enough.\n\r", ch);
	return;
    }
    smash_tilde( argument );
    sprintf(buf, "%s{x", argument );
    act( "You give $p to $N.", ch, obj, questman, TO_CHAR );
    act( "$n gives $p to $N.", ch, obj, questman, TO_NOTVICT );
    if (obj->item_type != ITEM_WEAPON ) 
    {
	send_to_char( "Item must be a weapon.\n\r", ch );
	return;
    }
    if (obj->value[1] >= 28) 
    {
	send_to_char( "You've hit the limit I can give you $N.\n\r", ch);
	return;
    }
    else if (obj->value[1] >= 25 && obj->level < 90 ) 
    {
	send_to_char( "You've hit the limit I can give you at your level $N.\n\r", ch);
	return;
    }
    else if (obj->value[1] >= 15 && obj->level < 15 ) 
    {
	send_to_char( "You've hit the limit I can give you at your level $N.\n\r", ch);
	return;
    }

    obj->value[1]++ ;
    ch->qps-=200;
    act( "$N gives $p to you.", ch, obj, questman, TO_CHAR );
    act( "$N gives $p to $n.", ch, obj, questman, TO_NOTVICT );
    return;
    }

    else if (is_name(arg2, "wv2"))
    {
	OBJ_DATA *obj;
    	for ( questman = ch->in_room->people; 
	questman != NULL; 
	questman = questman->next_in_room)

    if (IS_NPC(questman) && (questman->spec_fun == spec_lookup("spec_questmaster")))
	 break;

    if (questman == NULL || !can_see(ch,questman))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }
    if ( ( obj = get_obj_carry( ch, arg3 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }
    if (ch->qps < 200) 
    {
	send_to_char( "V2 cost's 200 gquest points, you do not have enough.\n\r", ch);
	return;
    }
    smash_tilde( argument );
    sprintf(buf, "%s{x", argument );
    act( "You give $p to $N.", ch, obj, questman, TO_CHAR );
    act( "$n gives $p to $N.", ch, obj, questman, TO_NOTVICT );
    if ( obj->item_type != ITEM_WEAPON )
    {
	send_to_char( "Item must be a weapon.\n\r", ch );
	return;
    }
    if (obj->value[2] >= 28) 
    {
	send_to_char( "You've hit the limit I can give you $N.\n\r", ch);
	return;
    }
    else if (obj->value[2] >= 25 && obj->level < 90 ) 
    {
	send_to_char( "You've hit the limit I can give you at your level $N.\n\r", ch);
	return;
    }
    else if (obj->value[2] >= 15 && obj->level < 15 ) 
    {
	send_to_char( "You've hit the limit I can give you at your level $N.\n\r", ch);
	return;
    }
    obj->value[2]++ ;
    ch->qps-=200;
    act( "$N gives $p to you.", ch, obj, questman, TO_CHAR );
    act( "$N gives $p to $n.", ch, obj, questman, TO_NOTVICT );
    return;
}

    else if (is_name(arg2, "av0"))
    {
	OBJ_DATA *obj;

    for ( questman = ch->in_room->people; 
	  questman != NULL; 
	  questman = questman->next_in_room)

    if (IS_NPC(questman) && (questman->spec_fun == spec_lookup("spec_questmaster")))
	    break;

    if (questman == NULL || !can_see(ch,questman))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }
    if ( ( obj = get_obj_carry( ch, arg3 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }
    if (ch->qps < 200) 
    {
	send_to_char( "Armor bonus's cost 200 gquest points, you do not have enough.\n\r", ch);
	return;
    }
    smash_tilde( argument );
    sprintf(buf, "%s{x", argument );
    act( "You give $p to $N.", ch, obj, questman, TO_CHAR );
    act( "$n gives $p to $N.", ch, obj, questman, TO_NOTVICT );
    if ( obj->item_type != ITEM_ARMOR )
    {
	send_to_char( "Item must be a weapon.\n\r", ch );
	return;
    }
    if (obj->value[0] >= 100 && obj->pIndexData->vnum != OVPKT5 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    else if (obj->value[0] >= 75 && obj->level < 90 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    else if (obj->value[0] >= 40 && obj->level < 15 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    obj->value[0]++ ; obj->value[0]++ ; obj->value[0]++ ; obj->value[0]++ ;
    obj->value[0]++ ; obj->value[0]++ ; obj->value[0]++ ; obj->value[0]++ ;
    obj->value[0]++ ; obj->value[0]++ ; 
    ch->qps-=200;
    act( "$N gives $p to you.", ch, obj, questman, TO_CHAR );
    act( "$N gives $p to $n.", ch, obj, questman, TO_NOTVICT );
    return;
}

    else if (is_name(arg2, "av1"))
    {
	    OBJ_DATA *obj;

    for ( questman = ch->in_room->people; 
	  questman != NULL; 
	  questman = questman->next_in_room)

    if (IS_NPC(questman) && (questman->spec_fun == spec_lookup("spec_questmaster")))
	    break;

    if (questman == NULL || !can_see(ch,questman))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }
    if ( ( obj = get_obj_carry( ch, arg3 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }
    if (ch->qps < 200) 
    {
	send_to_char( "Armor bonus's cost 200 gquest points, you do not have enough.\n\r", ch);
	return;
    }
    smash_tilde( argument );
    sprintf(buf, "%s{x", argument );
    act( "You give $p to $N.", ch, obj, questman, TO_CHAR );
    act( "$n gives $p to $N.", ch, obj, questman, TO_NOTVICT );
    if (obj->item_type != ITEM_ARMOR ) 
    {
	send_to_char( "Item must be armor.\n\r", ch );
	return;
    }
    if (obj->value[1] >= 100 && obj->pIndexData->vnum != OVPKT5 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    else if (obj->value[1] >= 75 && obj->level < 90 ) 
    {
	sprintf(buf, "You've hit the limit I can give you at your level %s.\n\r", ch->name);
	do_say(questman,buf);

	return;
    }
    else if (obj->value[1] >= 40 && obj->level < 15 ) 
    {
	sprintf(buf, "You've hit the limit I can give you at your level %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    obj->value[1]++ ; obj->value[1]++ ; obj->value[1]++ ; obj->value[1]++ ;
    obj->value[1]++ ; obj->value[1]++ ; obj->value[1]++ ; obj->value[1]++ ;
    obj->value[1]++ ; obj->value[1]++ ;
    ch->qps-=200;
    act( "$N gives $p to you.", ch, obj, questman, TO_CHAR );
    act( "$N gives $p to $n.", ch, obj, questman, TO_NOTVICT );
    return;
}

    else if (is_name(arg2, "av2"))
    {
	    OBJ_DATA *obj;

    for ( questman = ch->in_room->people; 
	  questman != NULL; 
	  questman = questman->next_in_room)

    if (IS_NPC(questman) && (questman->spec_fun == spec_lookup("spec_questmaster")))
	    break;

    if (questman == NULL || !can_see(ch,questman))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }
    if ( ( obj = get_obj_carry( ch, arg3 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }
    if (ch->qps < 200) 
    {
	send_to_char( "Armor bonus's cost 200 gquest points, you do not have enough.\n\r", ch);
	return;
    }
    smash_tilde( argument );
    sprintf(buf, "%s{x", argument );
    act( "You give $p to $N.", ch, obj, questman, TO_CHAR );
    act( "$n gives $p to $N.", ch, obj, questman, TO_NOTVICT );
    if ( obj->item_type != ITEM_ARMOR )
    {
	send_to_char( "Item must be armor.\n\r", ch );
	return;
    }
    if (obj->value[2] >= 100 && obj->pIndexData->vnum != OVPKT5 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    else if (obj->value[2] >= 75 && obj->level < 90 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    else if (obj->value[2] >= 40 && obj->level < 15 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    obj->value[2]++ ; obj->value[2]++ ; obj->value[2]++ ; obj->value[2]++ ;
    obj->value[2]++ ; obj->value[2]++ ; obj->value[2]++ ; obj->value[2]++ ;
    obj->value[2]++ ; obj->value[2]++ ;
    ch->qps-=200;
    act( "$N gives $p to you.", ch, obj, questman, TO_CHAR );
    act( "$N gives $p to $n.", ch, obj, questman, TO_NOTVICT );
    return;
}

    else if (is_name(arg2, "av3"))
    {
	OBJ_DATA *obj;

    for ( questman = ch->in_room->people; 
	  questman != NULL; 
	  questman = questman->next_in_room)

    if (IS_NPC(questman) && (questman->spec_fun == spec_lookup("spec_questmaster")))
	    break;

    if (questman == NULL || !can_see(ch,questman))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }
    if ( ( obj = get_obj_carry( ch, arg3 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }
    if (ch->qps < 200) 
    {
	send_to_char( "Armor bonus's cost 200 gquest points, you do not have enough.\n\r", ch);
	return;
    }
    smash_tilde( argument );
    sprintf(buf, "%s{x", argument );
    act( "You give $p to $N.", ch, obj, questman, TO_CHAR );
    act( "$n gives $p to $N.", ch, obj, questman, TO_NOTVICT );
    if ( obj->item_type != ITEM_ARMOR )
    {
	send_to_char( "Item must be armor.\n\r", ch );
	return;
    }
    if (obj->value[3] >= 100 && obj->pIndexData->vnum != OVPKT5 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    else if (obj->value[3] >= 75 && obj->level < 90 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    else if (obj->value[3] >= 40 && obj->level < 15 ) 
    {
	sprintf( buf, "You've hit the limit I can give you %s.\n\r", ch->name);
	do_say(questman,buf);
	return;
    }
    obj->value[3]++ ; obj->value[3]++ ; obj->value[3]++ ; obj->value[3]++ ;
    obj->value[3]++ ; obj->value[3]++ ; obj->value[3]++ ; obj->value[3]++ ;
    obj->value[3]++ ; obj->value[3]++ ;
    ch->qps-=200;
    act( "$N gives $p to you.", ch, obj, questman, TO_CHAR );
    act( "$N gives $p to $n.", ch, obj, questman, TO_NOTVICT );
    return;
}

    else if (is_name(arg2, "experience"))
    {

	if ((ch->level < 40) && (ch->class < MCLT_1))
	{
		sprintf(buf,"Save your coins till you get to level 40 %s.",ch->name);
		do_say(questman,buf);
		return;
	}
	else if (ch->platinum >= 1000)
        {
                ch->platinum -= 1000;
                ch->exp += 1000;
         	act( "$N gives 1000 Experience to $n.", ch, NULL, questman,TO_ROOM );
         	act( "$N gives you 1000 Experience.",   ch, NULL, questman,TO_CHAR );
                return;
        }
        else
        {
         	sprintf(buf, "Sorry, %s, but you don't have enough experience for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }

        else if (is_name(arg2, "dam"))
        {
	if ( is_name(arg3, "10" )){ damq_timer += 1000;
            if (ch->questpoints >= 10) { ch->questpoints -= 10; do_damq(questman, "on");
	sprintf(buf, "%s has activated the double damage quest flag", ch->name); do_dmecho(questman,buf);
                return;
            } else {
            	sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            } }
        }

        else if (is_name(arg2, "practice pr"))
        {
	if ( is_name(arg3, "10" )){ prquest_timer += 1000;
            if (ch->questpoints >= 10) { ch->questpoints -= 10; do_prq(questman, "on");
	sprintf(buf, "%s has activated the practice quest flag", ch->name); do_dmecho(questman,buf);
                return;
            } else {
            	sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            } }
        }

        else if (is_name(arg2, "expq exp xp"))
        {
	if ( is_name(arg3, "10" )){ xpq_timer += 1000;
            if (ch->questpoints >= 10) { ch->questpoints -= 10; do_xpq(questman, "on");
	sprintf(buf, "%s has activated the experience quest flag", ch->name); do_dmecho(questman,buf);
                return;
            } else {
            	sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            } }
        }

        else if (is_name(arg2, "practices pracs prac practice"))
        {
            if (ch->questpoints >= 500)
            {
                ch->questpoints -= 500;
                ch->practice += 70;
            	act( "$N gives 70 practices to $n.", ch, NULL, questman,TO_ROOM );
            	act( "$N gives you 70 practices.",   ch, NULL, questman,TO_CHAR );
                return;
            }
            else
            {
            	sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }



        else if (is_name(arg2, "platinum pp"))
        {
            if (ch->qps >= 100)
            {
                ch->qps -= 100;
                ch->platinum += 1000;
		act( "$N gives 1000 platinum pieces to $n.", ch, NULL,questman, TO_ROOM );
act("$N has 1000 in platinum transfered from $s Swiss account to your balance.",ch,NULL,questman,TO_CHAR);
                return;
            }
            else
            {
		sprintf(buf,"Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else
        {
            sprintf(buf, "Let me check my storeroom for that item, %s.",ch->name);
	    WAIT_STATE( ch, 8 );
            do_say(questman, buf);
        }
        if (obj != NULL)
        {
            act( "$N rustles around looking for the $p, 'AHA! Here it is $n.", ch, obj, questman, TO_ROOM );
            act( "$N gives $p to $n.", ch, obj, questman, TO_ROOM );
            act( "$N gives you $p.",   ch, obj, questman, TO_CHAR );
            obj_to_char(obj, ch);
        }
        return;
    }

    else if (!strcmp(arg1, "request") && !IS_NPC(ch))
    {
        act( "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
        act ("You ask $N for a quest.",ch, NULL, questman, TO_CHAR);

        if (IS_SET(ch->exbit1_flags, PLR_QUESTOR))
        {
            sprintf(buf, "But you're already on a quest!");
            do_say(questman, buf);
            return;
        }
	 

        if (ch->nextquest > 0)
        {
            sprintf(buf, "You're very brave, %s, but let someone else have a chance.",ch->name);
            do_say(questman, buf);
            sprintf(buf, "Come back later.");
            do_say(questman, buf);
            return;
        }
        sprintf(buf, "Thank you, brave %s!",ch->name);
        do_say(questman, buf);
        ch->questmob = 0;
        ch->questobj = 0;
        generate_quest(ch, questman);
        if (ch->questmob > 0 || ch->questobj > 0)
        {
            ch->qcountdown = number_range(25,45);
            SET_BIT(ch->exbit1_flags, PLR_QUESTOR);
            sprintf(buf, "You have %d minutes to complete this quest.",ch->qcountdown);
            do_say(questman, buf);
            sprintf(buf, "May the gods go with you!");
            do_say(questman, buf);
        }
        return;
    }
    else if (!strcmp(arg1, "complete"))
    {
	act( "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM);
	act ("You inform $N you have completed $s quest.",ch, NULL, questman, TO_CHAR);
        if (ch->questgiver != questman)
        {
	sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
        do_say(questman,buf);
        return;
        }

        if (IS_SET(ch->exbit1_flags, PLR_QUESTOR))
        {
            if (ch->questmob == -1 && ch->qcountdown > 0)
            {
                int reward, pointreward, pracreward;
                reward = number_range(5,24);
                pointreward = number_range(12,25);
                sprintf(buf, "Congratulations on completing your quest!");
                do_say(questman,buf);
         	sprintf(buf, "As a reward, I am giving you %d quest points, and %d platinum.",pointreward,reward);
                do_say(questman,buf);
                if (chance(15))
                {
                    pracreward = number_range(5,34);
                    sprintf(buf, "You gain %d practices!\n\r",pracreward);
                    send_to_char(buf, ch);
                    ch->practice += pracreward;
                }

                REMOVE_BIT(ch->exbit1_flags, PLR_QUESTOR);
                ch->questgiver = NULL;
                ch->qcountdown = 0;
                ch->questmob = 0;
                ch->questobj = 0;
                ch->nextquest = 10;  
                ch->platinum += reward;
                ch->questpoints += pointreward;

                return;
            }
            else if (ch->questobj > 0 && ch->qcountdown > 0)
            {
                bool obj_found = FALSE;

                for (obj = ch->carrying; obj != NULL; obj= obj_next)
                {
                    obj_next = obj->next_content;

                    if (obj != NULL && obj->pIndexData->vnum == ch->questobj)
                    {
                        obj_found = TRUE;
                        break;
                    }
                }
                if (obj_found == TRUE)
                {
                    int reward, pointreward, pracreward;
                    reward = number_range(5,34);
                    pointreward = number_range(25,75);
                    act("You hand $p to $N.",ch, obj, questman, TO_CHAR);
                    act("$n hands $p to $N.",ch, obj, questman, TO_ROOM);
                    sprintf(buf, "Congratulations on completing your quest!");
                    do_say(questman,buf);
                    sprintf(buf,"As a reward, I am giving you %d quest points, 		    and %d platinum.",pointreward,reward);
                    do_say(questman,buf);
                    if (chance(15))
                    {
                        pracreward = number_range(5,34);
                        sprintf(buf, "You gain %d practices!\n\r",pracreward);
                        send_to_char(buf, ch);
                        ch->practice += pracreward;
                    }
                    REMOVE_BIT(ch->exbit1_flags, PLR_QUESTOR);
                    extract_obj(obj);
                    ch->questgiver = NULL;
                    ch->qcountdown = 0;
                    ch->questmob = 0;
                    ch->questobj = 0;
                    ch->nextquest = 10;
                    ch->platinum += reward;
                    ch->questpoints += pointreward;
                    return;
                }
                else
                {
		sprintf(buf,"You haven't completed the quest yet, hurry there is still time!");
                do_say(questman, buf);
                return;
                }
                return;
            }
            if ((ch->questmob > 0 || ch->questobj > 0) && ch->qcountdown > 0) 
            {
		sprintf(buf, "You haven't completed the quest yet, but there is still time!");
                do_say(questman, buf);
                return;
            }
        }
        if (ch->qcountdown > 0)
        sprintf(buf,"But you didn't complete your quest in time!");
        else if (!IS_SET(ch->exbit1_flags, PLR_QUESTOR))
        sprintf(buf, "You have to REQUEST a quest first, %s.",ch->name);
        do_say(questman, buf);
        return;
    }

    send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\n\r",ch);
    send_to_char("For more information, type 'HELP QUEST'.\n\r",ch);
    return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *questitem;
    char buf [MSL];

    for (victim = char_list; victim != NULL; victim = victim->next )
	{
	if (!IS_NPC(victim)) continue;
        if (quest_level_diff(ch->level, victim->level) == TRUE
            	&& victim->pIndexData != NULL
            	&& victim->pIndexData->pShop == NULL
            	&& !IS_SET(victim->imm_flags, IMM_SUMMON)
    		&& !IS_SET(victim->act, ACT_TRAIN)
    		&& !IS_SET(victim->act, ACT_PRACTICE)
    		&& !IS_SET(victim->act, ACT_IS_HEALER)
		&& !IS_SET(victim->act, ACT_PET)
		&& !IS_SET(victim->affected_by, AFF_CHARM)
		&& !IS_SET(victim->affected_by, SHD_INVISIBLE)
		&& chance(15) ) break; 
	
	}
	
    if ( victim == NULL  )
    {
        do_say(questman, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, "Try again later.");
        ch->nextquest = 2;
        return;
    }

    if ( ( room = find_location( ch, victim->name ) ) == NULL )
    {
	sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, buf);
        sprintf(buf, "Try again later.");
        do_say(questman, buf);
        ch->nextquest = 2; 
        return;
    }


    if (chance(40))
    {
        int objvnum = 0;

        switch(number_range(0,5))
        {
            case 0:
            objvnum = QUEST_OBJQUEST1;
            break;

            case 1:
            objvnum = QUEST_OBJQUEST2;
            break;

            case 2:
            objvnum = QUEST_OBJQUEST3;
            break;

            case 3:
            objvnum = QUEST_OBJQUEST4;
            break;

            case 4:
            objvnum = QUEST_OBJQUEST5;
            break;

    //        case 5:
            // objvnum = QUEST_MOBQUEST1;
            // break;
        }

        questitem = create_object( get_obj_index(objvnum), ch->level );
        obj_to_room(questitem, room);
        ch->questobj = questitem->pIndexData->vnum;

        sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!",questitem->short_descr);
        do_say(questman, buf);
        do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");

        sprintf(buf, "Look in the general area of %s for %s!",room->area->name, room->name);
        do_say(questman, buf);
        return;
    }


    else
    {
    switch(number_range(0,1))
    {
        case 0:
	sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.",victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "This threat must be eliminated!");
        do_say(questman, buf);
        break;

        case 1:
	sprintf(buf, "One of the Realm's most heinous criminals, %s, has escaped from the dungeon!",victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "Since the escape, %s has murdered %d civillians!",victim->short_descr, number_range(2,20));
        do_say(questman, buf);
        do_say(questman,"The penalty for this crime is death and you are to deliver the sentence!");
        break;
    }

    if (room->name != NULL)
    {
        sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",victim->short_descr,room->name);
        do_say(questman, buf);

        sprintf(buf, "That location is in the general area of %s.",room->area->name);
        do_say(questman, buf);
    }
    ch->questmob = victim->pIndexData->vnum;
    }
    return;
}


bool quest_level_diff(int clevel, int mlevel)
{

if (clevel < 5 && mlevel < 9) { return TRUE; }
else if   (clevel > 4 && clevel < 11 && mlevel < 16) { return TRUE; }
else if   (clevel > 10 && clevel < 15 && mlevel > 10 && mlevel < 20) { return TRUE; }
else if   (clevel > 14 && clevel < 25 && mlevel > 20 && mlevel < 30) { return TRUE; }
else if   (clevel > 24 && clevel < 35 && mlevel > 30 && mlevel < 40) { return TRUE; }
else if   (clevel > 34 && clevel < 45 && mlevel > 40 && mlevel < 50) { return TRUE; }
else if   (clevel > 44 && clevel < 55 && mlevel > 50 && mlevel < 60) { return TRUE; }
else if   (clevel > 54 && clevel < 65 && mlevel > 60 && mlevel < 70) { return TRUE; }
else if   (clevel > 64 && clevel < 75 && mlevel > 70 && mlevel < 80) { return TRUE; }
else if   (clevel > 74 && clevel < 85 && mlevel > 80 && mlevel < 90) { return TRUE; }
else if   (clevel > 84 && clevel < 95 && mlevel > 90 && mlevel < 100) { return TRUE; }
else if   (clevel > 94 && clevel < 105 && mlevel >100 && mlevel < 110) { return TRUE; }
else if   (clevel > 104 && clevel < 115 && mlevel > 110 && mlevel < 120) { return TRUE; }
else if   (clevel > 114 && clevel < 125 && mlevel > 120 && mlevel < 130) { return TRUE; }
else if   (clevel > 124 && clevel < 135 && mlevel > 130 && mlevel < 140) { return TRUE; }
else if   (clevel > 134 && clevel < 145 && mlevel > 140 && mlevel < 150) { return TRUE; }
else if   (clevel > 144 && clevel < 155 && mlevel > 150 && mlevel < 160) { return TRUE; }
else if   (clevel > 154 && clevel < 165 && mlevel > 160 && mlevel < 170) { return TRUE; }
else if   (clevel > 164 && clevel < 175 && mlevel > 170 && mlevel < 180) { return TRUE; }
else if   (clevel > 174 && clevel < 185 && mlevel > 180 && mlevel < 190) { return TRUE; }
else if   (clevel > 184 && clevel < 195 && mlevel > 190 && mlevel < 200) { return TRUE; }
else if   (clevel > 194 && clevel < 205 && mlevel > 200 && mlevel < 210) { return TRUE; }
else if   (clevel > 204 && clevel < 208 && mlevel > 205 && mlevel < 211) { return TRUE; }
else if   (clevel > 207 && clevel < 211 && mlevel > 210 && mlevel < 212) { return TRUE; }
else if   (clevel > 210 && clevel < 215 && mlevel > 211 && mlevel < 212) { return TRUE; }
else if	  (clevel > 214 && mlevel < 217 ) { return TRUE; }
else return FALSE;

}

void do_aquest(CHAR_DATA *ch, char *argument)
{
    char buf[MSL];
    CHAR_DATA *questmaster;
    MOB_INDEX_DATA *pMobIndex;
    char *which_mob;
    char mob_vnum[MIL];

    if (IS_NPC(ch))
	return;

    buf[0] = '\0';
    /* find a questmaster */
    for ( questmaster = ch->in_room->people; 
	  questmaster != NULL; 
	  questmaster = questmaster->next_in_room)
	if (IS_NPC(questmaster) && IS_SET(questmaster->act,ACT_QUESTMASTER))
	    break;

    if (questmaster == NULL || !can_see(ch,questmaster))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }

    if (ch->spirit)
    {
	send_to_char( "Spirits cannot autoquest.\n\r", ch);
	return;
    }

    if ((ch->pcdata->tier == 0) && (ch->level < 5))
    {
	send_to_char( "You must be at least level 5 to autoquest.\n\r", ch);
	return;
    }
/*
    if ((ch->can_aquest == 1) && (ch->level < LEVEL_HERO)
    && !IS_SET(ch->act, PLR_LQUEST) && (!ch->pcdata->is_aquest))
    {
	send_to_char( "You may not autoquest again till next level.\n\r", ch);
	return;
    }
 */
    if (!can_see(questmaster,ch))
    {
	act("$N tells you '{aI must see you to judge your worthiness to quest{x'."
	    ,ch,NULL,questmaster,TO_CHAR);
	return;
    }

    if (IS_SET(ch->act, PLR_LQUEST))
    {
	int level_vnum;
	char *object_name;

	level_vnum = ((ch->level/10) + 56);
	if (ch->can_aquest == 2)
	{
	    bool found = FALSE;
	    OBJ_DATA *object;
	    OBJ_INDEX_DATA *pObjIndex;
				/*
				 * 
				 * 
				 */
    for ( object = ch->carrying; object != NULL; object = object->next_content )
    {
	if (IS_OBJ_STAT(object,ITEM_LQUEST) && (object->pIndexData->vnum == level_vnum))
	found = TRUE;
	extract_obj( object );
    }
   if (found)
   {
	act("$N tells you '{aWell done, my child, you may now advance to the next level{x'.",ch,NULL,questmaster,TO_CHAR);
	ch->can_aquest = 0;
	ch->pcdata->is_aquest = FALSE;
	pObjIndex = get_obj_index( OBJ_VNUM_QUESTPILL );
	object = create_object( pObjIndex, 0 );
	obj_to_char( object, ch );
	act( "$n gives $p to $N.", questmaster, object, ch, TO_NOTVICT );
	act( "$n gives you $p.",   questmaster, object, ch, TO_VICT    );
	ch->exp += 1;
	REMOVE_BIT(ch->act, PLR_LQUEST);
	ch->level += 1;
	change_level( ch );
	if (ch->level == LEVEL_HERO)
    {
    OBJ_DATA *obj_next;

	sprintf(buf, "Please welcome the realms newest Hero -> %s", capitalize(ch->name) );
	do_grats(questmaster, buf);
	buf[0] = '\0';

    for ( object = ch->carrying; object != NULL; object = obj_next )
    {
	obj_next = object->next_content;
    if (IS_OBJ_STAT(object,ITEM_LQUEST))
    {
	extract_obj( object );
    }
    }

    for ( object = ch->carrying; object != NULL; object = obj_next )
    {
    obj_next = object->next_content;

	/*
	if (object->pIndexData->vnum == OBJ_VNUM_QPOUCH)
	{
		extract_obj( object );
		break;
	}
	*/
	}
		act("$N takes your quest pouch from you.",ch,NULL,questmaster,TO_CHAR);
		act("$N tells you '{aI can't yet give you an award for reaching Hero, because Russ hasn't finished my code. (so blame HIM, not ME!){x'.",ch,NULL,questmaster,TO_CHAR);
	}
		return;
	}
	    sprintf(buf, "$N tells you '{aYour level quest is not yet complete,\n\rplease see me when you have the %s{x'.", str_dup(ch->pcdata->lquest_obj));
	    act(buf,ch,NULL,questmaster,TO_CHAR);
	    buf[0] = '\0';
	    return;
	}


	ch->can_aquest = 2;
	act("$N tells you:",ch,NULL,questmaster,TO_CHAR);

	send_to_char("  {aGood day to you, my child, the following task will allow you to{x\n\r", ch);
	send_to_char("  {aadvance to the next level.{x\n\r\n\r", ch);

	object_name = make_lquest_obj(ch);
	which_mob = find_quest_mob(ch, TRUE);
	which_mob = one_argument(which_mob, mob_vnum);
	if ((pMobIndex = get_mob_index(atoi(mob_vnum))) == NULL )
	{
	    sprintf(buf, "  {aI can't seem to find a suitable enemy for your level.{x\n\r\n\r");
	    send_to_char(buf, ch);
	    buf[0] = '\0';
	    return;
	}

	ch->pcdata->is_aquest = FALSE;
	ch->pcdata->lquest_obj = str_dup(object_name);
	ch->pcdata->lquest_mob = str_dup(pMobIndex->short_descr);
	ch->pcdata->lquest_are = str_dup(which_mob);
	ch->pcdata->quest_mob = pMobIndex->vnum;
	sprintf(buf, "  {aYou must find the {W%s{a and bring it back to me.{x\n\r", str_dup(ch->pcdata->lquest_obj));
	send_to_char(buf, ch);
	sprintf(buf, "  {aI have heard rumors that it was last seen in {W%s{a,{x\n\r", str_dup(ch->pcdata->lquest_are));
	send_to_char(buf, ch);
	sprintf(buf, "  {aand that {W%s{a was bragging of it's powers.{x\n\r\n\r", str_dup(ch->pcdata->lquest_mob));
	send_to_char(buf, ch);
	buf[0] = '\0';
	return;
    }

    if (ch->pcdata->is_aquest)
    {
	bool found = FALSE;
	OBJ_DATA *object;
	OBJ_INDEX_DATA *pObjIndex;

	for ( object = ch->carrying; object != NULL; object = object->next_content )
	{
	    if ((object->pIndexData->vnum == ch->pcdata->quest_obj)
	    && (object->got_from == ch->pcdata->quest_mob))
		found = TRUE;
		// extract_obj( object );
	}
	if (found)
	{
	    act("$N tells you '{aWell done, my child, here is your reward{x'." ,ch,NULL,questmaster,TO_CHAR);
	    ch->pcdata->is_aquest = FALSE;
	    pObjIndex = get_obj_index( OBJ_VNUM_QUESTPILL );
	    object = create_object( pObjIndex, 0 );
	    obj_to_char( object, ch );
	    act( "$n gives $p to $N.", questmaster, object, ch, TO_NOTVICT );
	    act( "$n gives you $p.",   questmaster, object, ch, TO_VICT    );
	    return;
	}
	act("$N tells you '{aYour quest is not yet complete{x'.",ch,NULL,questmaster,TO_CHAR);
	return;
    }

    act("$N tells you:",ch,NULL,questmaster,TO_CHAR);

    send_to_char("  {aGood day to you, my child, I require that you complete the{x\n\r", ch);
    send_to_char("  {afollowing task for a quest point.{x\n\r\n\r", ch);

    which_mob = find_quest_mob(ch, FALSE);
    which_mob = one_argument(which_mob, mob_vnum);
    if ((pMobIndex = get_mob_index(atoi(mob_vnum))) == NULL )
    {
	sprintf(buf, "  {aI can't seem to find a suitable enemy for your level.{x\n\r\n\r");
	send_to_char(buf, ch);
	buf[0] = '\0';
	return;
    }

    {
	OBJ_INDEX_DATA *pObjIndex;

	if ((pObjIndex = get_obj_index(ch->pcdata->quest_obj)) == NULL )
	{
	    sprintf(buf, "  {aI can't seem to find a suitable enemy for your level.{x\n\r\n\r");
	    send_to_char(buf, ch);
	    buf[0] = '\0';
	    return;
	}
	ch->can_aquest = 1;
	ch->pcdata->is_aquest = TRUE;
	ch->pcdata->quest_mob = pMobIndex->vnum;
	ch->pcdata->lquest_obj = str_dup(pObjIndex->short_descr);
	ch->pcdata->lquest_mob = str_dup(pMobIndex->short_descr);
	ch->pcdata->lquest_are = str_dup(which_mob);
	sprintf(buf, "  {aYou must retrieve {W%s{a from {W%s{a.{x\n\r", str_dup(pObjIndex->short_descr), str_dup(pMobIndex->short_descr));
	send_to_char(buf, ch);
	sprintf(buf, "  {aThis creature makes it's home in {W%s{a.{x\n\r\n\r", str_dup(which_mob));
	send_to_char(buf, ch);
	buf[0] = '\0';
    }
    return;

}

/* Find a nice, suitable mob to kill. */

char *find_quest_mob( CHAR_DATA *ch, bool lquest )
{
    MOB_INDEX_DATA *pMobIndex;
    int min_lev;
    int max_lev;
    int timeout;

    /*	if a level quest, go as high as 10 levels above the player and
	2 levels below the player.  If an autoquest, go as high as
	6 above and as low as 6 below.
    */
    if (!lquest)
    {
	min_lev = ch->level - 2;
	max_lev = ch->level + 8;
    } 
	else 
	{
	min_lev = ch->level - 2;
	max_lev = ch->level + 5;
    }

    /* First, let's just choose 500 mobs at random */

    for (timeout = 0; timeout < 500; timeout++)
    {
	CHAR_DATA *victim;

	while ((pMobIndex = get_mob_index(number_range(1, 32767))) == NULL)
	{ }
/* note changes here, could have been causing players to be sent
 * on quests that were too hard for them, Artimus1
 */
	if ( pMobIndex->level < min_lev )
	    break;          /* was continue, was that right? Artimus */
	if ( pMobIndex->level > max_lev )
	    break;          /* was continue, was that right? Artimus */

	/* No shopkeepers */
	if ( pMobIndex->pShop != NULL )
	    break;          /* was continue, was that right? Artimus */

	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   is_name( pMobIndex->player_name, victim->name ) )
	    {
		if (!is_suitable_qmob(ch, victim, lquest))
		    continue;
		break;
	    }
	}
	if (victim == NULL)
	    break;          /* was continue, was that right? Artimus */
	else
	{
	    char rt[MSL];

	    sprintf(rt, "%d %s", victim->pIndexData->vnum,
		str_dup(victim->in_room->area->name));
	    return str_dup(rt);
	}
    }

    /* No suitable mobs found, try a different method.
       This time, we start at a random number and work in
       both directions till we find a suitable mob.
    */

    {
	int vnumlo;
	int vnumhi;

	vnumlo = number_range(1, 32767);
	vnumhi = vnumlo;
	for ( ; vnumlo >= 0 && vnumhi <= 32766; vnumlo--, vnumhi++)
	{
	    CHAR_DATA *victim;
	    if (vnumlo < 0) {vnumlo = 0;}
	    if (vnumhi > 32766) {vnumhi = 32766;}
	    if ((pMobIndex = get_mob_index(vnumlo)) != NULL)
	    {
	      for ( ; ; )
	      {
		if ( pMobIndex->level < min_lev )
		    break;
		if ( pMobIndex->level > max_lev )
		    break;

		/* No shopkeepers */
		if ( pMobIndex->pShop != NULL )
		    break;

		for (victim = char_list; victim != NULL; victim = victim->next)
		{
		    if ( victim->in_room != NULL
		    &&   is_name( pMobIndex->player_name, victim->name ) )
		    {
			if (!is_suitable_qmob(ch, victim, lquest))
			    continue;
			break;
		    }
		}
		if (victim == NULL)
		    break;
		else
		{
		    char rt[MSL];

		    sprintf(rt, "%d %s", victim->pIndexData->vnum,
			str_dup(victim->in_room->area->name));
		    return str_dup(rt);
		}
	      }
	    }
	    if ((pMobIndex = get_mob_index(vnumhi)) != NULL)
	    {
	      for ( ; ; )
	      {
		if ( pMobIndex->level < min_lev )
		    break;
		if ( pMobIndex->level > max_lev )
		    break;

		/* No shopkeepers */
		if ( pMobIndex->pShop != NULL )
		    break;

		for (victim = char_list; victim != NULL; victim = victim->next)
		{
		    if ( victim->in_room != NULL
		    &&   is_name( pMobIndex->player_name, victim->name ) )
		    {
			if (!is_suitable_qmob(ch, victim, lquest))
			    continue;
			break;
		    }
		}
		if (victim == NULL)
		    break;
		else
		{
		    char rt[MSL];

		    sprintf(rt, "%d %s", victim->pIndexData->vnum,
			str_dup(victim->in_room->area->name));
		    return str_dup(rt);
		}
	      }
	    }
	}
    }
    /* Still no suitable mobs found, give up */

    return "0 0";
}


/* Check to see if victim is a suitable quest mob.
   No PCs, Pets, Trainers/Guildmasters, Healers, Bankers,
   Priests, Questmasters, Clan Guards, Charmed Mobs,
   or mobs in safe rooms, school, imm area, midgaard,
   or clan rooms.
*/
bool is_suitable_qmob(CHAR_DATA *ch, CHAR_DATA *victim, bool lquest)
{
    if (!IS_NPC(victim))
	return FALSE;
    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	return FALSE;
    if (!can_see(ch, victim))
	return FALSE;
    if (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_IS_HEALER)
    ||  IS_SET(victim->act,ACT_PET)
    ||  IS_SET(victim->act,ACT_QUESTMASTER)
    ||  IS_SET(victim->act,ACT_IS_BANKER)
    ||  IS_SET(victim->act,ACT_IS_SATAN)
    ||  IS_SET(victim->act,ACT_IS_PRIEST))
	return FALSE;
    if (IS_SET(victim->in_room->room_flags,ROOM_CLAN_ENT))
	return FALSE;
    if (IS_AFFECTED(victim,AFF_CHARM))
	return FALSE;
    if (victim->in_room->area->min_vnum == ROOM_VNUM_SCHOOL)
	return FALSE;
    if (victim->in_room->area->min_vnum == ROOM_VNUM_CHAT)
	return FALSE;
    if (victim->in_room->area->min_vnum == ROOM_VNUM_MIDGAARD)
	return FALSE;
    if (victim->in_room->area->min_vnum == ROOM_VNUM_CLANS)
	return FALSE;

    /*	Since the quest object is created in the players
	inventory in a level quest, the mob does not
	have to leave a corpse, nor does it have to be
	carrying anything.  But in an autoquest,
	a no_body mob is unacceptable, and it must have
	a suitable object for the quest.
    */
    if (!lquest)
    {
	OBJ_DATA *obj;

	if (IS_SET(victim->act,ACT_NO_BODY))
	    return FALSE;
	if (victim->carrying == NULL)
	    return FALSE;
	for ( obj = victim->carrying; obj != NULL; obj = obj->next_content )
	{
	    if (!can_see_obj(ch, obj))
		continue;
	    if ((obj->item_type != ITEM_LIGHT)
	    &&  (obj->item_type != ITEM_WAND)
	    &&  (obj->item_type != ITEM_STAFF)
	    &&  (obj->item_type != ITEM_WEAPON)
	    &&  (obj->item_type != ITEM_TREASURE)
	    &&  (obj->item_type != ITEM_ARMOR)
	    &&  (obj->item_type != ITEM_CLOTHING)
	    &&  (obj->item_type != ITEM_TRASH)
	    &&  (obj->item_type != ITEM_CONTAINER)
	    &&  (obj->item_type != ITEM_DRINK_CON)
	    &&  (obj->item_type != ITEM_WARP_STONE)
	    &&  (obj->item_type != ITEM_JEWELRY)
	    &&  (obj->item_type != ITEM_DEMON_STONE))
		continue;

	    if ((IS_OBJ_STAT(obj, ITEM_NODROP))
	    ||  (IS_OBJ_STAT(obj, ITEM_NOREMOVE))
	    ||  (IS_OBJ_STAT(obj, ITEM_INVENTORY))
	    ||  (IS_OBJ_STAT(obj, ITEM_ROT_DEATH))
	    ||  (IS_OBJ_STAT(obj, ITEM_VIS_DEATH))
	    ||  (IS_OBJ_STAT(obj, ITEM_MELT_DROP))
	    ||  (IS_OBJ_STAT(obj, ITEM_NOUNCURSE))
	    ||  (IS_OBJ_STAT(obj, ITEM_QUEST))
	    ||  (IS_OBJ_STAT(obj, ITEM_FORCED))
	    ||  (IS_OBJ_STAT(obj, ITEM_QUESTPOINT))
	    ||  (IS_OBJ_STAT(obj, ITEM_LQUEST)))
		continue;

	    ch->pcdata->quest_obj = obj->pIndexData->vnum;
	    return TRUE;
	}
	return FALSE;
    }

    return TRUE;
}


/* Make an object name for a level quest. */

char *make_lquest_obj( CHAR_DATA *ch )
{
    char lquest_obj[MSL];
    char lobj_adj[100];
    char lobj_pho[100];
    char lobj_typ[100];
    int phonum = 0;
    int phochoice = 0;
    int pholast = -1;

    lquest_obj[0] = '\0';
    if (number_range(0, 100) < 19)
    {
	sprintf(lobj_adj, "%s", adj_table[number_range(0, 9)]);
    } else {
	lobj_adj[0] = '\0';
    }
    phonum = number_range(2, 4);
    lobj_pho[0] = '\0';
    for ( ; phonum > 0; phonum--)
    {
	while ((phochoice = number_range(0, 88)) == pholast) { }
	strcat(lobj_pho, pho_table[phochoice]);
	pholast = phochoice;
    }
    sprintf(lobj_typ, "%s", qot_table[number_range(0, 4)]);
    sprintf(lquest_obj, "%s%s%s", lobj_adj, (capitalize(lobj_pho)), lobj_typ);
    return str_dup(lquest_obj);
}

