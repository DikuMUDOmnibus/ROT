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
#include <stdlib.h>
#include "merc.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_groups	);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);


/* used to get new skills */
void do_gain(CHAR_DATA *ch, char *argument)
{
    char buf[MSL];
    char arg[MIL];
    CHAR_DATA *trainer;
    int gn = 0, sn = 0;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people; 
	  trainer != NULL; 
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
	    break;

    if (trainer == NULL || !can_see(ch,trainer))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }

    if (ch->spirit)
    {
	send_to_char( "Spirits cannot gain.\n\r", ch);
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	do_say(trainer,"{aPardon me?{x");
	send_to_char("{aI offer the following gain options:{x\n\r",ch);
	send_to_char("{R   list     {aList the groups and skills you can gain.{x\n\r", ch);
	send_to_char("{R   convert  {aConverts 5 practices to 1 train.{x\n\r", ch);
	send_to_char("{R   study    {aConverts 1 train to 6 practices.{x\n\r", ch);
	send_to_char("{R   quest    {aConverts 3 quest points to 1 train.{x\n\r", ch);
	send_to_char("{R   points   {aGain creation points.{x\n\r", ch);
	send_to_char("{R   <{rname{R>   {aGain skill or group <name>{x\n\r\n\r", ch);
	return;
    }

    if (!str_prefix(arg,"list"))
    {
	int col;
	
	col = 0;

	sprintf(buf, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
	             "group","cost","group","cost","group","cost");
	send_to_char(buf,ch);

	for (gn = 0; gn < MAX_GROUP; gn++)
	{
	    bool nfound = FALSE;
	    int tmpskill = 999;

	    if (group_table[gn].name == NULL)
		break;

	    if (!ch->pcdata->group_known[gn]
	    &&  group_table[gn].rating[ch->class] > 0)
	    {
		nfound = TRUE;
		sprintf(buf,"{R%-18s %-5d {x",
		    group_table[gn].name,group_table[gn].rating[ch->class]);
		tmpskill = group_table[gn].rating[ch->class];
	    }
	    if ( (ch->pcdata->tier == 2)
	    && !ch->pcdata->group_known[gn]
	    &&  group_table[gn].rating[ch->clasb] > 0)
	    {
		sprintf(buf,"{B%-18s %-5d {x",
		    group_table[gn].name,group_table[gn].rating[ch->clasb]);
		if (nfound)
		{
		    sprintf(buf,"{G%-18s %-5d {x",
			group_table[gn].name,
			UMIN(tmpskill, group_table[gn].rating[ch->clasb]) );
		}
		nfound = TRUE;
	    }
	    if (nfound)
	    {
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
	    }
	}
	if (col % 3 != 0)
	    send_to_char("\n\r",ch);
	
	send_to_char("\n\r",ch);		

	col = 0;

        sprintf(buf, "{M%-18s {Y%-5s {M%-18s {Y%-5s {M%-18s {Y%-5s{x\n\r",
                     "skill","cost","skill","cost","skill","cost");
        send_to_char(buf,ch);
 
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
	    bool nfound = FALSE;
	    int tmpskill = 999;

            if (skill_table[sn].name == NULL)
                break;
 
            if (!ch->pcdata->learned[sn]
            &&  skill_table[sn].rating[ch->class] > 0
	    &&  skill_table[sn].spell_fun == spell_null)
            {
		nfound = TRUE;
                sprintf(buf,"{R%-18s %-5d {x",
                    skill_table[sn].name,skill_table[sn].rating[ch->class]);
		tmpskill = skill_table[sn].rating[ch->class];
            }
	    if ( (ch->pcdata->tier == 2)
	    &&  !ch->pcdata->learned[sn]
            &&  skill_table[sn].rating[ch->clasb] > 0
	    &&  skill_table[sn].spell_fun == spell_null)
            {
                sprintf(buf,"{B%-18s %-5d {x",
                    skill_table[sn].name,skill_table[sn].rating[ch->clasb]);
		if (nfound)
		{
		    sprintf(buf,"{G%-18s %-5d {x",
			skill_table[sn].name,
			UMIN(tmpskill, skill_table[sn].rating[ch->clasb]) );
		}
		nfound = TRUE;
            }
	    if (nfound)
	    {
                send_to_char(buf,ch);
                if (++col % 3 == 0)
                    send_to_char("\n\r",ch);
	    }
        }
        if (col % 3 != 0)
            send_to_char("\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"convert"))
    {
	if (ch->practice < 6)
	{
	    act("$N tells you '{aYou are not yet ready.{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N helps you apply your practice to training",
		ch,NULL,trainer,TO_CHAR);
	ch->practice -= 5;
	ch->train +=1 ;
	return;
    }

    if (!str_prefix(arg,"study"))
    {
	if (ch->train < 1)
	{
            act("$N tells you '{aYou are not yet ready.{x'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }

        act("$N helps you apply your training to practice",
                ch,NULL,trainer,TO_CHAR);
        ch->train -= 1;
	ch->practice += 5;
	return;
    }

    if (!str_prefix(arg,"quest"))
    {
	if (ch->aqps < 3)
	{
	    act("$N tells you '{aYou are not yet ready.{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N helps you apply your quest points to training",
		ch,NULL,trainer,TO_CHAR);
	ch->aqps -= 3;
	ch->train += 1;
	return;
    }

    if (!str_prefix(arg,"points"))
    {

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_LQUEST))
	{
	    act("$N tells you '{aYou may not gain points while on a level quest.{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (ch->train < 1)
	{
	    act("$N tells you '{aYou are not yet ready.{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (ch->pcdata->points <= 40)
	{
	    act("$N tells you '{aThere would be no point in that.{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp) >= 5000)
	{
	    sprintf(buf, "Go work off some of that exp first %s.",ch->name);
	    do_say(trainer,buf);
	    return;
	}

	act("$N trains you, and you feel more at ease with your skills.",
	    ch,NULL,trainer,TO_CHAR);

	ch->train -= 1;
	ch->pcdata->points -= 1;
	ch->exp = (long)exp_per_level(ch,ch->pcdata->points) * ch->level;
	return;
    }

    /* else add a group/skill */

    gn = group_lookup(argument);
    if (gn > 0)
    {
	if (ch->pcdata->group_known[gn])
	{
	    act("$N tells you '{aYou already know that group!{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (ch->pcdata->tier >= 2)
	{
	    if ( ( group_table[gn].rating[ch->class] <= 0 ) && ( group_table[gn].rating[ch->clasb] <= 0 ) ) {
		act("$N tells you '{MThat group is beyond your powers.{x'", ch,NULL,trainer,TO_CHAR);
		return;
	    }

	    if (ch->train < group_table[gn].rating[ch->class]) {
		act("$N tells you '{WYou are not yet ready for that group.{x'", ch,NULL,trainer,TO_CHAR);
		return;
	    }
	} 
	else {
	    if ( ( group_table[gn].rating[ch->class] <= 0 ) &&   ( group_table[gn].rating[ch->clasb] <= 0 ) ) {
		act("$N tells you '{mThat group is beyond your powers.{x'", ch,NULL,trainer,TO_CHAR);
		return;
	    }

	    if ( ( ch->train < group_table[gn].rating[ch->class] ) &&   ( ch->train < group_table[gn].rating[ch->clasb] ) ) {
		act("$N tells you '{wYou are not yet ready for that group.{x'", ch,NULL,trainer,TO_CHAR);
		return;
	    }
	}

	/* add the group */
	gn_add(ch,gn);
	act("$N trains you in the art of $t",
	    ch,group_table[gn].name,trainer,TO_CHAR);
	if (ch->pcdata->tier >= 2)
	{
	    ch->train -= group_table[gn].rating[ch->class];
	} else {
	    int tmptrain = 999;

	    if (group_table[gn].rating[ch->class] > 0)
		tmptrain = group_table[gn].rating[ch->class];
	    if (group_table[gn].rating[ch->clasb] > 0)
		tmptrain = UMIN(tmptrain, group_table[gn].rating[ch->clasb]);
	    ch->train -= tmptrain;
	}
	return;
    }

    sn = skill_lookup(argument);
    if (sn > -1)
    {
	if (skill_table[sn].spell_fun != spell_null)
	{
	    act("$N tells you '{aYou must learn the full group.{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

        if (ch->pcdata->learned[sn])
        {
            act("$N tells you '{aYou already know that skill!{x'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }
 
	if (ch->pcdata->tier != 2)
	{
	    if (skill_table[sn].rating[ch->class] <= 0)
	    {
		act("$N tells you '{aThat skill is beyond your powers.{x'",
		    ch,NULL,trainer,TO_CHAR);
		return;
	    }
 
	    if (ch->train < skill_table[sn].rating[ch->class])
	    {
		act("$N tells you '{aYou are not yet ready for that skill.{x'",
		    ch,NULL,trainer,TO_CHAR);
		return;
	    }
	} else {
	    if ( ( skill_table[sn].rating[ch->class] <= 0 )
	    &&   ( skill_table[sn].rating[ch->clasb] <= 0 ) )
	    {
		act("$N tells you '{aThat skill is beyond your powers.{x'",
		    ch,NULL,trainer,TO_CHAR);
		return;
	    }
 
	    if ( ( ch->train < skill_table[sn].rating[ch->class] )
	    &&   ( ch->train < skill_table[sn].rating[ch->clasb] ) )
	    {
		act("$N tells you '{aYou are not yet ready for that skill.{x'",
		    ch,NULL,trainer,TO_CHAR);
		return;
	    }
	}
 
        /* add the skill */
	ch->pcdata->learned[sn] = 1;
        act("$N trains you in the art of $t",
            ch,skill_table[sn].name,trainer,TO_CHAR);

	if (ch->pcdata->tier >= 2)
	{
	    ch->train -= skill_table[sn].rating[ch->class];
	} else {
	    int tmptrain = 999;;

	    if (skill_table[sn].rating[ch->class] > 0)
		tmptrain = skill_table[sn].rating[ch->class];
	    if (skill_table[sn].rating[ch->clasb] > 0)
		tmptrain = UMIN(tmptrain, skill_table[sn].rating[ch->clasb]);
	    ch->train -= tmptrain;
	}

        return;
    }

    act("$N tells you '{aI do not understand...{x'",ch,NULL,trainer,TO_CHAR);
}
    



/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument)
{
    char spell_list[LEVEL_HERO][MSL];
    char spell_columns[LEVEL_HERO];
    int sn,lev,mana;
    bool found = FALSE;
    bool nfound = FALSE;
    char buf[MSL];

    if (IS_NPC(ch))
      return;

    /* initilize data */
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
	spell_columns[lev] = 0;
	spell_list[lev][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL)
        break;

      nfound = FALSE;

      if (skill_table[sn].skill_level[ch->class] < LEVEL_HERO &&
	  skill_table[sn].spell_fun != spell_null &&
          ch->pcdata->learned[sn] > 0)
      {
	nfound = TRUE;
	found = TRUE;
	lev = skill_table[sn].skill_level[ch->class];
	if (ch->level < lev)
	  sprintf(buf,"{R%-18s  n/a      {x", skill_table[sn].name);
	else
	{
	  mana = UMAX(skill_table[sn].min_mana, 100/(2 + ch->level - lev));
	  sprintf(buf,"{R%-18s  %3d mana  {x",skill_table[sn].name,mana);
	}
      }
      if (ch->pcdata->tier == 2)
      {
	if (skill_table[sn].skill_level[ch->clasb] < LEVEL_HERO
	&& skill_table[sn].spell_fun != spell_null
	&& ch->pcdata->learned[sn] > 0)
	{
	    int nlev;

	    if (!nfound)
		lev = 999;
	    found = TRUE;
	    nlev = skill_table[sn].skill_level[ch->clasb];
	    if (ch->level < nlev && nlev < lev)
		{
		    sprintf(buf,"{B%-18s  n/a      {x", skill_table[sn].name);
	    	} 
		else if (nlev < lev)
		{
	 mana = UMAX(skill_table[sn].min_mana,100/(2+ch->level-nlev));
	    sprintf(buf,"{B%-18s  %3d mana  {x",skill_table[sn].name,mana);
	    	}
	    if (nfound)
	    {
		if (ch->level < nlev)
		{
		    if (nlev < lev)
		    {
			sprintf(buf,"{G%-18s  n/a      {x", skill_table[sn].name);
		    }
		} else {
		    if (nlev < lev)
		    {
			 mana = UMAX(skill_table[sn].min_mana, 100/(2 + ch->level - nlev));
			sprintf(buf,"{G%-18s  %3d mana  {x",skill_table[sn].name,mana);
		    }
		}
	    }
	    if (nlev < lev)
		lev = nlev;
	    nfound = TRUE;
	}
      }
      if (nfound)
      {
	if (spell_list[lev][0] == '\0')
	  sprintf(spell_list[lev],"\n\r{MLevel %2d: {x%s",lev,buf);
        else /* append */
	{
	  if ( ++spell_columns[lev] % 2 == 0)
            strcat(spell_list[lev],"\n\r          ");
	  strcat(spell_list[lev],buf);
        }
      }
    }

    /* return results */
 
    if (!found)
    {
      send_to_char("You know no spells.\n\r",ch);
      return;
    }
    
    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (spell_list[lev][0] != '\0')
	send_to_char(spell_list[lev],ch);
    send_to_char("\n\r",ch);
}


void do_bskills(CHAR_DATA *ch, char *argument)
{
    char skill_list[LEVEL_HERO][MSL];
    char skill_columns[LEVEL_HERO];
    int sn,lev;
    bool found = FALSE;
    bool nfound = FALSE;
    char buf[MSL];
 
    if (IS_NPC(ch))
      return;
 
    /* initilize data */
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
        skill_columns[lev] = 0;
        skill_list[lev][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL )
        break;

      nfound = FALSE;
 
      if (skill_table[sn].skill_level[ch->clasb] < LEVEL_HERO &&
	  skill_table[sn].spell_fun == spell_null &&
	  ch->pcdata->learned[sn] > 0)
      {
        found = TRUE;
        nfound = TRUE;
       lev = skill_table[sn].skill_level[ch->clasb];
        if (ch->level < lev)
          sprintf(buf,"{R%-18s n/a      {x", skill_table[sn].name);
        else
          sprintf(buf,"{R%-18s %3d%%      {x",skill_table[sn].name,
					 ch->pcdata->learned[sn]);
      }

      if (ch->pcdata->tier == 2)
      {
	if (skill_table[sn].skill_level[ch->clasb] < LEVEL_HERO &&
	    skill_table[sn].spell_fun == spell_null &&
	    ch->pcdata->learned[sn] > 0)
	{
	    int nlev;

	    if (!nfound)
		lev = 999;
	    found = TRUE;
	    nlev = skill_table[sn].skill_level[ch->clasb];
	    if (ch->level < nlev)
	    {
		    sprintf(buf,"{B%-18s n/a      {x", skill_table[sn].name);
	    } else {
		    sprintf(buf,"{B%-18s %3d%%      {x",skill_table[sn].name,
			ch->pcdata->learned[sn]);
	    }
	    if (nlev < lev)
		lev = nlev;
	    if (nfound)
	    {
		if (ch->level < lev)
		{
			sprintf(buf,"{G%-18s n/a      {x", skill_table[sn].name);
		} else {
			sprintf(buf,"{G%-18s %3d%%      {x",skill_table[sn].name,
			    ch->pcdata->learned[sn]);
		}
	    }
	    nfound = TRUE;
	}
      }
      if (nfound)
      {
        if (skill_list[lev][0] == '\0')
          sprintf(skill_list[lev],"\n\r{MLevel %2d: {x%s",lev,buf);
        else /* append */
        {
          if ( ++skill_columns[lev] % 2 == 0)
            strcat(skill_list[lev],"\n\r          ");
          strcat(skill_list[lev],buf);
        }
      }
    }
 
    /* return results */
 
    if (!found)
    {
      send_to_char("You know no skills.\n\r",ch);
      return;
    }
 
    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (skill_list[lev][0] != '\0')
        send_to_char(skill_list[lev],ch);
    send_to_char("\n\r",ch);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
    char skill_list[LEVEL_HERO][MSL];
    char skill_columns[LEVEL_HERO];
    int sn,lev;
    bool found = FALSE;
    bool nfound = FALSE;
    char buf[MSL];
 
    if (IS_NPC(ch))
      return;
 
    /* initilize data */
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
        skill_columns[lev] = 0;
        skill_list[lev][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL )
        break;

      nfound = FALSE;
 
      if (skill_table[sn].skill_level[ch->clasb] < LEVEL_HERO &&
	  skill_table[sn].spell_fun == spell_null &&
	  ch->pcdata->learned[sn] > 0)
      {
        found = TRUE;
        nfound = TRUE;
       lev = skill_table[sn].skill_level[ch->clasb];
        if (ch->level < lev)
          sprintf(buf,"{R%-18s n/a      {x", skill_table[sn].name);
        else
          sprintf(buf,"{R%-18s %3d%%      {x",skill_table[sn].name,
					 ch->pcdata->learned[sn]);
      }

      if (ch->pcdata->tier == 2)
      {
	if (skill_table[sn].skill_level[ch->clasb] < LEVEL_HERO &&
	    skill_table[sn].spell_fun == spell_null &&
	    ch->pcdata->learned[sn] > 0)
	{
	    int nlev;

	    if (!nfound)
		lev = 999;
	    found = TRUE;
	    nlev = skill_table[sn].skill_level[ch->clasb];
	    if (ch->level < nlev)
	    {
		    sprintf(buf,"{B%-18s n/a      {x", skill_table[sn].name);
	    } else {
		    sprintf(buf,"{B%-18s %3d%%      {x",skill_table[sn].name,
			ch->pcdata->learned[sn]);
	    }
	    if (nlev < lev)
		lev = nlev;
	    if (nfound)
	    {
		if (ch->level < lev)
		{
			sprintf(buf,"{G%-18s n/a      {x", skill_table[sn].name);
		} else {
			sprintf(buf,"{G%-18s %3d%%      {x",skill_table[sn].name,
			    ch->pcdata->learned[sn]);
		}
	    }
	    nfound = TRUE;
	}
      }
      if (nfound)
      {
        if (skill_list[lev][0] == '\0')
          sprintf(skill_list[lev],"\n\r{MLevel %2d: {x%s",lev,buf);
        else /* append */
        {
          if ( ++skill_columns[lev] % 2 == 0)
            strcat(skill_list[lev],"\n\r          ");
          strcat(skill_list[lev],buf);
        }
      }
    }
 
    /* return results */
 
    if (!found)
    {
      send_to_char("You know no skills.\n\r",ch);
      return;
    }
 
    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (skill_list[lev][0] != '\0')
        send_to_char(skill_list[lev],ch);
    send_to_char("\n\r",ch);
}


/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch)
{
    char buf[100];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    sprintf(buf,"{M%-18s {Y%-5s {M%-18s {Y%-5s {M%-18s {Y%-5s{x\n\r",
	"group","cp","group","cp","group","cp");
    send_to_char(buf,ch);

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
	bool found = FALSE;
	int tmpcost = 999;

	if (group_table[gn].name == NULL)
	    break;

        if (!ch->gen_data->group_chosen[gn] 
	&&  !ch->pcdata->group_known[gn]
	&&  group_table[gn].rating[ch->class] > 0)
	{
	    found = TRUE;
	    sprintf(buf,"{R%-18s %-5d {x",group_table[gn].name,
		group_table[gn].rating[ch->class]);
	    tmpcost = group_table[gn].rating[ch->class];
	}
	if (ch->pcdata->tier == 2)
	{
	    if (!ch->gen_data->group_chosen[gn] 
	    &&  !ch->pcdata->group_known[gn]
	    &&  group_table[gn].rating[ch->clasb] > 0)
	    {
		sprintf(buf,"{B%-18s %-5d {x",group_table[gn].name,
			group_table[gn].rating[ch->clasb]);
		if (group_table[gn].rating[ch->clasb] < tmpcost)
		{
		    tmpcost = group_table[gn].rating[ch->clasb];
		}
		if (found)
		{
		    sprintf(buf,"{G%-18s %-5d {x",group_table[gn].name,
			tmpcost);
		}
		found = TRUE;
	    }
	}
	if (found)
	{
	    send_to_char(buf,ch);
	    if (++col % 3 == 0)
		send_to_char("\n\r",ch);
	}
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    col = 0;
 
    sprintf(buf,"{M%-18s {Y%-5s {M%-18s {Y%-5s {M%-18s {Y%-5s{x\n\r",
	"skill","cp","skill","cp","skill","cp");
    send_to_char(buf,ch);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	bool found = FALSE;
	int tmpcost = 999;

        if (skill_table[sn].name == NULL)
            break;
 
        if (!ch->gen_data->skill_chosen[sn] 
	&&  ch->pcdata->learned[sn] == 0
	&&  skill_table[sn].spell_fun == spell_null
	&&  skill_table[sn].rating[ch->class] > 0)
        {
	    found = TRUE;
            sprintf(buf,"{R%-18s %-5d {x",skill_table[sn].name,
                                    skill_table[sn].rating[ch->class]);
	    tmpcost = skill_table[sn].rating[ch->class];
	}
	if (ch->pcdata->tier == 2)
	{
	    if (!ch->gen_data->skill_chosen[sn] 
	    &&  ch->pcdata->learned[sn] == 0
	    &&  skill_table[sn].spell_fun == spell_null
	    &&  skill_table[sn].rating[ch->clasb] > 0)
            {
		sprintf(buf,"{B%-18s %-5d {x",skill_table[sn].name,
			skill_table[sn].rating[ch->clasb]);
		if (skill_table[sn].rating[ch->clasb] < tmpcost)
		{
		    tmpcost = skill_table[sn].rating[ch->clasb];
		}
		if (found)
		{
		    sprintf(buf,"{G%-18s %-5d {x",skill_table[sn].name,
			tmpcost);
		}
		found = TRUE;
	    }
	}
	if (found)
	{
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
    send_to_char(buf,ch);
    sprintf(buf,"Experience per level: %ld\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}


void list_group_chosen(CHAR_DATA *ch)
{
    char buf[100];
    int gn,sn,col;
 
    if (IS_NPC(ch))
        return;
 
    col = 0;
 
    sprintf(buf,"{M%-18s {Y%-5s {M%-18s {Y%-5s {M%-18s {Y%-5s{x\n\r",
	"group","cp","group","cp","group","cp");
    send_to_char(buf,ch);
 
    for (gn = 0; gn < MAX_GROUP; gn++)
    {
	bool found = FALSE;

        if (group_table[gn].name == NULL)
            break;
 
        if (ch->gen_data->group_chosen[gn] 
	&&  group_table[gn].rating[ch->class] > 0)
        {
	    found = TRUE;
            sprintf(buf,"{R%-18s %-5d {x",group_table[gn].name,
                                    group_table[gn].rating[ch->class]);
        }
	if (ch->pcdata->tier == 2)
	{
	    if (ch->gen_data->group_chosen[gn] 
	    &&  group_table[gn].rating[ch->clasb] > 0)
	    {
		sprintf(buf,"{B%-18s %-5d {x",group_table[gn].name,
                                    group_table[gn].rating[ch->clasb]);
		if (found)
		{
		    sprintf(buf,"{G%-18s %-5d {x",group_table[gn].name,
                    UMIN(group_table[gn].rating[ch->class], group_table[gn].rating[ch->clasb]));
		}
		found = TRUE;
	    }
	}
	if (found)
	{
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }

    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);
 
    col = 0;
 
    sprintf(buf,"{M%-18s {Y%-5s {M%-18s {Y%-5s {M%-18s {Y%-5s{x\n\r",
	"skill","cp","skill","cp","skill","cp");
    send_to_char(buf,ch);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	bool found = FALSE;

        if (skill_table[sn].name == NULL)
            break;
 
        if (ch->gen_data->skill_chosen[sn] 
	&&  skill_table[sn].rating[ch->class] > 0)
        {
	    found = TRUE;
            sprintf(buf,"{R%-18s %-5d {x",skill_table[sn].name,
                                    skill_table[sn].rating[ch->class]);
	}
	if (ch->pcdata->tier == 2)
	{
	    if (ch->gen_data->skill_chosen[sn] 
	    &&  skill_table[sn].rating[ch->clasb] > 0)
	    {
		sprintf(buf,"{B%-18s %-5d {x",skill_table[sn].name,
                                    skill_table[sn].rating[ch->clasb]);
	    }
	    if (found)
	    {
		sprintf(buf,"{G%-18s %-5d {x",skill_table[sn].name,
                                    skill_table[sn].rating[ch->clasb]);
	    }
	    found = TRUE;
	}
	if (found)
	{
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);
 
    sprintf(buf,"{MCreation points: {C%d{x\n\r",ch->gen_data->points_chosen);
    send_to_char(buf,ch);
    sprintf(buf,"{MExperience per level: {C%ld{x\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}

long exp_per_level(CHAR_DATA *ch, int points)
{
    long expl,inc;

    if (IS_NPC(ch))
	return 1000; 

    expl = 1000;
    inc = 500;

    if (points < 40)
    {
	if (ch->pcdata->tier != 2)
	{
	    return 1000 * pc_race_table[ch->race].class_mult[ch->class]/100;
	} else {
	    long rpoint;

	    rpoint = ( ( ( 1000 * pc_race_table[ch->race].class_mult[ch->class]/100 )
		   +     ( 1000 * pc_race_table[ch->race].class_mult[ch->class]/100 )
		   +     ( 1000 * pc_race_table[ch->race].class_mult[ch->clasb]/100 ) )
		   /       3 );
	    return (long)rpoint;
	}
    }

    /* processing */
    points -= 40;

    while (points > 9)
    {
	expl += (long)inc;
        points -= 10;
        if (points > 9)
	{
	    expl += (long)inc;
	    inc *= 2;
	    points -= 10;
	}
    }

    expl += (long)points * (long)inc / 10;  

    if (ch->pcdata->tier != 2)
    {
	return (long)expl * (long)pc_race_table[ch->race].class_mult[ch->class]/100;
    } else {
	long rpoint;

	rpoint	= ( ( ( (long)expl * (long)pc_race_table[ch->race].class_mult[ch->class]/100 )
		+     ( (long)expl * (long)pc_race_table[ch->race].class_mult[ch->class]/100 )
		+     ( (long)expl * (long)pc_race_table[ch->race].class_mult[ch->clasb]/100 ) )
		/       3 );
	return (long)rpoint;
    }
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,char *argument)
{
    char arg[MIL];
    char buf[100];
    int gn,sn,i;
 
    if (argument[0] == '\0')
	return FALSE;

    if (IS_NPC(ch))
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (argument[0] == '\0')
	{
	    do_help(ch,"group help");
	    return TRUE;
	}

        do_help(ch,argument);
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("You must provide a skill name.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1)
	{
	    bool found = FALSE;

	    if (ch->gen_data->group_chosen[gn]
	    ||  ch->pcdata->group_known[gn])
	    {
		send_to_char("You already know that group!\n\r",ch);
		return TRUE;
	    }

	    if (group_table[gn].rating[ch->class] < 1)
	    {
		found = TRUE;
	    }
	    if (ch->pcdata->tier == 2)
		if (group_table[gn].rating[ch->clasb] > 0)
		    found = FALSE;
	    if (found)
	    {
	  	send_to_char("That group is not available.\n\r",ch);
	 	return TRUE;
	    }
	    found = FALSE;
	    if (exp_per_level(ch,ch->gen_data->points_chosen+group_table[gn].rating[ch->class]) > 500000)
	    {
		found = TRUE;
		send_to_char("That will take you over the Exp/Level limit.\n\r",ch);
		return TRUE;
	    }
	    if (ch->pcdata->tier == 2)
		if (exp_per_level(ch,ch->gen_data->points_chosen+group_table[gn].rating[ch->clasb]) <= 500000)
		    found = FALSE;
	    if (found)
	    {
		send_to_char("That will take you over the Exp/Level limit.\n\r",ch);
		return TRUE;
	    }

	    sprintf(buf,"%s group added\n\r",group_table[gn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->group_chosen[gn] = TRUE;
	    if (ch->pcdata->tier != 2)
	    {
		ch->gen_data->points_chosen += group_table[gn].rating[ch->class];
	    } else {
		if (group_table[gn].rating[ch->class] < 1)
		{
		    ch->gen_data->points_chosen += group_table[gn].rating[ch->clasb];
		} else if (group_table[gn].rating[ch->clasb] < 1)
		{
		    ch->gen_data->points_chosen += group_table[gn].rating[ch->class];
		} else {
		    ch->gen_data->points_chosen += UMIN(
			group_table[gn].rating[ch->class],
			group_table[gn].rating[ch->clasb]);
		}
	    }
	    gn_add(ch,gn);
	    if (ch->pcdata->tier != 2)
	    {
		ch->pcdata->points += group_table[gn].rating[ch->class];
	    } else {
		if (group_table[gn].rating[ch->class] < 1)
		{
		    ch->pcdata->points += group_table[gn].rating[ch->clasb];
		} else if (group_table[gn].rating[ch->clasb] < 1)
		{
		    ch->pcdata->points += group_table[gn].rating[ch->class];
		} else {
		    ch->pcdata->points += UMIN(
			group_table[gn].rating[ch->class],
			group_table[gn].rating[ch->clasb]);
		}
	    }
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1)
	{
	    bool found = FALSE;

	    if (ch->gen_data->skill_chosen[sn]
	    ||  ch->pcdata->learned[sn] > 0)
	    {
		send_to_char("You already know that skill!\n\r",ch);
		return TRUE;
	    }

	    if (skill_table[sn].rating[ch->class] < 1)
		found = TRUE;
	    if (ch->pcdata->tier == 2)
		if (skill_table[sn].rating[ch->class] > 0)
		    found = FALSE;
	    if (skill_table[sn].spell_fun != spell_null)
		found = TRUE;
	    if (found)
	    {
		send_to_char("That skill is not available.\n\r",ch);
		return TRUE;
	    }

	    sprintf(buf, "%s skill added\n\r",skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->skill_chosen[sn] = TRUE;
	    if (ch->pcdata->tier != 2)
	    {
		ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
	    } else {
		if (skill_table[sn].rating[ch->class] < 1)
		{
		    ch->gen_data->points_chosen += skill_table[sn].rating[ch->clasb];
		} else if (skill_table[sn].rating[ch->clasb] < 1)
		{
		    ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
		} else {
		    ch->gen_data->points_chosen += UMIN(
			skill_table[sn].rating[ch->class],
			skill_table[sn].rating[ch->clasb]);
		}
	    }
	    ch->pcdata->learned[sn] = 1;
	    if (ch->pcdata->tier != 2)
	    {
		ch->pcdata->points += skill_table[sn].rating[ch->class];
	    } else {
		if (skill_table[sn].rating[ch->class] < 1)
		{
		    ch->pcdata->points += skill_table[sn].rating[ch->clasb];
		} else if (skill_table[sn].rating[ch->clasb] < 1)
		{
		    ch->pcdata->points += skill_table[sn].rating[ch->class];
		} else {
		    ch->pcdata->points += UMIN(
			skill_table[sn].rating[ch->class],
			skill_table[sn].rating[ch->clasb]);
		}
	    }
	    return TRUE;
	}

	send_to_char("No skills or groups by that name...\n\r",ch);
	return TRUE;
    }

    if (!strcmp(arg,"drop"))
    {
	if (argument[0] == '\0')
  	{
	    send_to_char("You must provide a skill to drop.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1 && ch->gen_data->group_chosen[gn])
	{
	    send_to_char("Group dropped.\n\r",ch);
	    ch->gen_data->group_chosen[gn] = FALSE;
	    if (ch->pcdata->tier != 2)
	    {
		ch->gen_data->points_chosen -= group_table[gn].rating[ch->class];
	    } else {
		if (group_table[gn].rating[ch->class] < 1)
		{
		    ch->gen_data->points_chosen -= group_table[gn].rating[ch->clasb];
		} else if (group_table[gn].rating[ch->clasb] < 1)
		{
		    ch->gen_data->points_chosen -= group_table[gn].rating[ch->class];
		} else {
		    ch->gen_data->points_chosen -= UMIN(
			group_table[gn].rating[ch->class],
			group_table[gn].rating[ch->clasb]);
		}
	    }
	    gn_remove(ch,gn);
	    for (i = 0; i < MAX_GROUP; i++)
	    {
		if (ch->gen_data->group_chosen[gn])
		    gn_add(ch,gn);
	    }
	    if (ch->pcdata->tier != 2)
	    {
		ch->pcdata->points -= group_table[gn].rating[ch->class];
	    } else {
		if (group_table[gn].rating[ch->class] < 1)
		{
		    ch->pcdata->points -= group_table[gn].rating[ch->clasb];
		} else if (group_table[gn].rating[ch->clasb] < 1)
		{
		    ch->pcdata->points -= group_table[gn].rating[ch->class];
		} else {
		    ch->pcdata->points -= UMIN(
			group_table[gn].rating[ch->class],
			group_table[gn].rating[ch->clasb]);
		}
	    }
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1 && ch->gen_data->skill_chosen[sn])
	{
	    send_to_char("Skill dropped.\n\r",ch);
	    ch->gen_data->skill_chosen[sn] = FALSE;
	    if (ch->pcdata->tier != 2)
	    {
		ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
	    } else {
		if (skill_table[sn].rating[ch->class] < 1)
		{
		    ch->gen_data->points_chosen -= skill_table[sn].rating[ch->clasb];
		} else if (skill_table[sn].rating[ch->clasb] < 1)
		{
		    ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
		} else {
		    ch->gen_data->points_chosen -= UMIN(
			skill_table[sn].rating[ch->class],
			skill_table[sn].rating[ch->clasb]);
		}
	    }
	    ch->pcdata->learned[sn] = 0;
	    if (ch->pcdata->tier != 2)
	    {
		ch->pcdata->points -= skill_table[sn].rating[ch->class];
	    } else {
		if (skill_table[sn].rating[ch->class] < 1)
		{
		    ch->pcdata->points -= skill_table[sn].rating[ch->clasb];
		} else if (skill_table[sn].rating[ch->clasb] < 1)
		{
		    ch->pcdata->points -= skill_table[sn].rating[ch->class];
		} else {
		    ch->pcdata->points -= UMIN(
			skill_table[sn].rating[ch->class],
			skill_table[sn].rating[ch->clasb]);
		}
	    }
	    return TRUE;
	}

	send_to_char("You haven't bought any such skill or group.\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_help(ch,"premise");
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	list_group_costs(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"learned"))
    {
	list_group_chosen(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"info"))
    {
	do_groups(ch,argument);
	return TRUE;
    }

    return FALSE;
}
        

/* shows all groups, or the sub-members of a group */
void do_groups(CHAR_DATA *ch, char *argument)
{
    char buf[100];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    send_to_char("Use the class command (help class) for a listing\n\r",ch);
    send_to_char("more in tune with your current character.\n\r\n\r",ch);

    if (argument[0] == '\0')
    {   /* show all groups */
	
	for (gn = 0; gn < MAX_GROUP; gn++)
        {
	    if (group_table[gn].name == NULL)
		break;
	    if (ch->pcdata->group_known[gn])
	    {
		sprintf(buf,"%-20s ",group_table[gn].name);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
	    }
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
        sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	send_to_char(buf,ch);
	return;
     }

     if (!str_cmp(argument,"all"))    /* show all groups */
     {
        for (gn = 0; gn < MAX_GROUP; gn++)
        {
            if (group_table[gn].name == NULL)
                break;
	    sprintf(buf,"%-20s ",group_table[gn].name);
            send_to_char(buf,ch);
	    if (++col % 3 == 0)
            	send_to_char("\n\r",ch);
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
	return;
     }
	
     
     /* show the sub-members of a group */
     gn = group_lookup(argument);
     if (gn == -1)
     {
	send_to_char("No group of that name exist.\n\r",ch);
	send_to_char(
	    "Type 'groups all' or 'info all' for a full listing.\n\r",ch);
	return;
     }

     for (sn = 0; sn < MAX_IN_GROUP; sn++)
     {
	if (group_table[gn].spells[sn] == NULL)
	    break;
	sprintf(buf,"%-20s ",group_table[gn].spells[sn]);
	send_to_char(buf,ch);
	if (++col % 3 == 0)
	    send_to_char("\n\r",ch);
     }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
}

/* shows all groups, or the sub-members of a group available to your class*/
void do_class(CHAR_DATA *ch, char *argument)
{
    char arg[MIL];
    char buf[MSL];
    int gn,sn,tn,col,class,clasb;
    bool found = FALSE;

    if (IS_NPC(ch))
	return;

    col = 0;
    argument = one_argument(argument,arg);
    class = class_lookup(arg);
    if (class != -1)
	argument = one_argument(argument,arg);

    clasb = -1;
    if (arg[0] == '\0')
    {   /* show all groups */

	send_to_char( "Groups you currently have:\n\r", ch );
	send_to_char( "--------------------------\n\r", ch );
	for (gn = 0; gn < MAX_GROUP; gn++)
        {
	    if (group_table[gn].name == NULL)
		break;
	    if (ch->pcdata->group_known[gn])
	    {
		sprintf(buf,"%-20s ",group_table[gn].name);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
	    }
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
        sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	send_to_char(buf,ch);
	buf[0] = '\0';
	return;
     }

     if (!str_cmp(arg,"all"))    /* show all groups */
     {
	if (class == -1)
	{
	    send_to_char( "{MGroups available to your character:{x\n\r", ch );
	    class = ch->class;
	    if (ch->pcdata->tier == 2)
		clasb = ch->clasb;
	}
	else
	    send_to_char( "{M  Groups available to this class:{x\n\r", ch );
	send_to_char( "-----------------------------------\n\r", ch );
        for (gn = 0; gn < MAX_GROUP; gn++)
        {
	    found = FALSE;

            if (group_table[gn].name == NULL)
                break;
	    if (group_table[gn].rating[class] > 0)
	    {
		found = TRUE;
		sprintf(buf,"{R%-20s {x",group_table[gn].name);
	    }
	    if ( ( ch->pcdata->tier == 2 ) && ( clasb != -1 ) )
	    {
		if (group_table[gn].rating[clasb] > 0)
		{
		    sprintf(buf,"{B%-20s {x",group_table[gn].name);
		    if (found)
		    {
			sprintf(buf,"{G%-20s {x",group_table[gn].name);
		    }
		    found = TRUE;
		}
	    }
	    if (found)
	    {
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
	    }
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
	buf[0] = '\0';
	return;
     }

     if (!str_cmp(arg,"skill"))    /* show all skills */
     {
	if (class == -1)
	{
	    send_to_char( "{MSkills available to your character:{x\n\r", ch );
	    class = ch->class;
	    if (ch->pcdata->tier == 2)
		clasb = ch->clasb;
	}
	else
	    send_to_char( "{M  Skills available to this class:{x\n\r", ch );
	send_to_char( "-----------------------------------\n\r", ch );
	send_to_char( "{MLevel{x-{YSkill{x--------------- {YLevel{x-{YSkill{x---------------\n\r", ch );
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
	    found = FALSE;

            if (skill_table[sn].name == NULL)
                break;
 
            if (skill_table[sn].rating[class] > 0)
            {
		found = TRUE;
		sprintf(buf,"{R%-5d %-20s {x",
                    skill_table[sn].skill_level[class],skill_table[sn].name);
	    }
	    if ( ( ch->pcdata->tier == 2 ) && ( clasb != -1 ) )
	    {
		if (skill_table[sn].rating[clasb] > 0 )
		{
		    sprintf(buf,"{B%-5d %-20s {x",
			skill_table[sn].skill_level[clasb],skill_table[sn].name);
		    if (found)
		    {
			sprintf(buf,"{G%-5d %-20s {x",
			    UMIN(skill_table[sn].skill_level[class],
				 skill_table[sn].skill_level[clasb]),
			    skill_table[sn].name);
		    }
		    found = TRUE;
		}
	    }
	    if (skill_table[sn].spell_fun != spell_null)
		found = FALSE;
	    if (found)
	    {
                send_to_char(buf,ch);
		if (++col % 2 == 0)
		    send_to_char("\n\r",ch);
            }
        }
        if (col % 2 != 0)
            send_to_char("\n\r",ch);
	buf[0] = '\0';
	return;
     }

     /* show the sub-members of a group */
     gn = group_lookup(arg);
     if (gn == -1)
     {
	send_to_char("No group of that name exists.\n\r",ch);
	send_to_char("Type 'class all' for a full listing.\n\r",ch);
	return;
     }
     if ( (ch->pcdata->tier != 2) || (class != -1) )
     {
	if (class == -1)
	    class = ch->class;
	if (group_table[gn].rating[class] < 1)
	{
	    send_to_char("This class does not get that group.\n\r",ch);
	    send_to_char("Type 'class all' for a full listing.\n\r",ch);
	    return;
	}
     } else if (class != -1)
     {
	if (group_table[gn].rating[class] < 1)
	{
	    send_to_char("This class does not get that group.\n\r",ch);
	    send_to_char("Type 'class all' for a full listing.\n\r",ch);
	    return;
	}
     } else {
	class = ch->class;
	clasb = ch->clasb;
	if ( ( group_table[gn].rating[class] < 1 ) 
	&&   ( group_table[gn].rating[clasb] < 1 ) )
	{
	    send_to_char("This character does not get that group.\n\r",ch);
	    send_to_char("Type 'class all' for a full listing.\n\r",ch);
	    return;
	}
     }

    send_to_char( "{MSpells available in this group:{x\n\r", ch );
    send_to_char( "-------------------------------\n\r", ch );
    send_to_char( "{MLevel{x-{YSpell{x--------------- {MLevel{x-{YSpell{x---------------\n\r", ch );
    if (class == -1)
    {
	class = ch->class;
	if (ch->pcdata->tier == 2)
	    clasb = ch->clasb;
    }
    for (sn = 0; sn < MAX_IN_GROUP; sn++)
    {
	found = FALSE;
	if (group_table[gn].spells[sn] == NULL)
	    break;
	if ( ( tn = spell_avail_class( ch, group_table[gn].spells[sn] ) ) >= 0)
	{
	    found = TRUE;
	    sprintf(buf,"{R%-5d %-20s {x",
		skill_table[tn].skill_level[class], group_table[gn].spells[sn]);
	}
	if (ch->pcdata->tier == 2)
	{
	    if ( ( tn = spell_avail_clasb( ch, group_table[gn].spells[sn] ) ) >= 0)
	    {
		sprintf(buf,"{B%-5d %-20s {x",
		    skill_table[tn].skill_level[clasb], group_table[gn].spells[sn]);
	    }
	    if (found)
	    {
		sprintf(buf,"{G%-5d %-20s {x",
		    UMIN(skill_table[tn].skill_level[class],
			 skill_table[tn].skill_level[clasb]),
		    group_table[gn].spells[sn]);
	    }
	    found = TRUE;
	}
	if (found)
	{
	    send_to_char(buf,ch);
	    if (++col % 2 == 0)
		send_to_char("\n\r",ch);
	}
    }
    buf[0] = '\0';
    if ( col % 2 != 0 )
	send_to_char( "\n\r", ch );
}

int spell_avail( CHAR_DATA *ch, const char *name )
{
    /* checks to see if a spell is available to either class */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if (skill_table[sn].skill_level[ch->class] <= LEVEL_HERO)
		    return sn;
	    if (ch->pcdata->tier == 2)
	    {
		if (skill_table[sn].skill_level[ch->clasb] <= LEVEL_HERO)
		    return sn;
	    }
	}
    }
    return found;
}
int spell_avail_class( CHAR_DATA *ch, const char *name )
{
    /* checks to see if a spell is available to a primary class */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if (skill_table[sn].skill_level[ch->class] <= LEVEL_HERO)
		    return sn;
	}
    }
    return found;
}
int spell_avail_clasb( CHAR_DATA *ch, const char *name )
{
    /* checks to see if a spell is available to a secondary class */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    if (ch->pcdata->tier != 2)
	return found;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if (skill_table[sn].skill_level[ch->clasb] <= LEVEL_HERO)
		    return sn;
	}
    }
    return found;
}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance;
    char buf[100];
    bool found = FALSE;

    if (IS_NPC(ch))
	return;

    if (ch->level < skill_table[sn].skill_level[ch->class]
    ||  skill_table[sn].rating[ch->class] == 0
    ||  ch->pcdata->learned[sn] == 0
    ||  ch->pcdata->learned[sn] == 100)
    {
	found = TRUE;
    }
    if (ch->pcdata->tier != 2)
    {
	if (found)
	    return;   /* skill is not known */
    } else if (found) {
	if (ch->level < skill_table[sn].skill_level[ch->clasb]
	||  skill_table[sn].rating[ch->clasb] == 0
	||  ch->pcdata->learned[sn] == 0
	||  ch->pcdata->learned[sn] == 100)
	{
	    return;	/* skill is not known */
	}
    }

    /* check to see if the character has a chance to learn */
    chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
    if (ch->pcdata->tier != 2)
    {
	chance /= (		multiplier
			*	skill_table[sn].rating[ch->class] 
			*	4);
    } else {
	if (skill_table[sn].rating[ch->clasb] < 1)
	{
	    chance /= (		multiplier
			    *	skill_table[sn].rating[ch->class]
			    *	4);
	} else if (skill_table[sn].rating[ch->class] < 1)
	{
	    chance /= (		multiplier
			    *	skill_table[sn].rating[ch->clasb]
			    *	4);
	} else
	{
	    chance /= (		multiplier
			    *	UMIN(skill_table[sn].rating[ch->class],
				     skill_table[sn].rating[ch->clasb])
			    *	4);
	}
    }
    chance += ch->level;

    if (number_range(1,1000) > chance)
	return;

    /* now that the character has a CHANCE to learn, see if they really have */	

    if (success)
    {
	chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
	if (number_percent() < chance)
	{
	    sprintf(buf,"You have become better at %s!\n\r",
		    skill_table[sn].name);
	    send_to_char(buf,ch);
	if ( global_prq ){
	    ch->pcdata->learned[sn]++;
	    ch->pcdata->learned[sn]++;
	    ch->pcdata->learned[sn]++;
	    ch->pcdata->learned[sn]++;
	} else {
	    ch->pcdata->learned[sn]++;
	}
	    if (!IS_SET(ch->act, PLR_LQUEST))
	    {
		if (ch->pcdata->tier != 2)
		{
		    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
		} else
		{
		    if (skill_table[sn].rating[ch->clasb] < 1)
		    {
			gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
		    } else if (skill_table[sn].rating[ch->class] < 1)
		    {
			gain_exp(ch,2 * skill_table[sn].rating[ch->clasb]);
		    } else
		    {
			gain_exp(ch,2 * UMIN(skill_table[sn].rating[ch->class],
					 skill_table[sn].rating[ch->clasb]));
		    }
		}
	    }
	}
    }

    else
    {
	chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
	if (number_percent() < chance)
	{
	    sprintf(buf,
		"You learn from your mistakes, and your %s skill improves.\n\r",
		skill_table[sn].name);
	    send_to_char(buf,ch);
	if ( global_prq ){
	    ch->pcdata->learned[sn] += number_range(8,16);
	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
	} else {
	    ch->pcdata->learned[sn] += number_range(1,3);
	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
	}
	    if (!IS_SET(ch->act, PLR_LQUEST))
	    {
		if (ch->pcdata->tier != 2)
		{
		    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
		} else
		{
		    if (skill_table[sn].rating[ch->clasb] < 1)
		    {
			gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
		    } else if (skill_table[sn].rating[ch->class] < 1)
		    {
			gain_exp(ch,2 * skill_table[sn].rating[ch->clasb]);
		    } else
		    {
			gain_exp(ch,2 * UMIN(skill_table[sn].rating[ch->class],
					 skill_table[sn].rating[ch->clasb]));
		    }
		}
	    }
	}
    }
}

/* returns a group index number given the name */
int group_lookup( const char *name )
{
    int gn;
 
    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
        if ( group_table[gn].name == NULL )
            break;
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_prefix( name, group_table[gn].name ) )
            return gn;
    }
 
    return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn)
{
    int i;
    
    ch->pcdata->group_known[gn] = TRUE;
    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_add(ch,group_table[gn].spells[i],FALSE);
    }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for ( i = 0; i < MAX_IN_GROUP; i ++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
	group_remove(ch,group_table[gn].spells[i]);
    }
}
	
/* use for processing a skill or group for addition  */
void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn,gn;

    if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

    sn = skill_lookup(name);

    if (sn != -1)
    {
	if (ch->pcdata->learned[sn] == 0) /* i.e. not known */
	{
	    ch->pcdata->learned[sn] = 1;
	    if (deduct)
	    {
		if (ch->pcdata->tier != 2)
		{
		    ch->pcdata->points += skill_table[sn].rating[ch->class];
		} else
		{
		    if (skill_table[sn].rating[ch->clasb] < 1)
		    {
			ch->pcdata->points += skill_table[sn].rating[ch->class];
		    } else if (skill_table[sn].rating[ch->class] < 1)
		    {
			ch->pcdata->points += skill_table[sn].rating[ch->clasb];
		    } else
		    {
			ch->pcdata->points += UMIN(skill_table[sn].rating[ch->class],
						   skill_table[sn].rating[ch->clasb]);
		    }
		}
	    }

	}
	return;
    }
	
    /* now check groups */

    gn = group_lookup(name);

    if (gn != -1)
    {
	if (ch->pcdata->group_known[gn] == FALSE)  
	{
	    ch->pcdata->group_known[gn] = TRUE;
	    if (deduct)
	    {
		if (ch->pcdata->tier != 2)
		{
		    ch->pcdata->points += group_table[gn].rating[ch->class];
		} else
		{
		    if (group_table[gn].rating[ch->clasb] < 1)
		    {
			ch->pcdata->points += group_table[gn].rating[ch->class];
		    } else if (group_table[gn].rating[ch->class] < 1)
		    {
			ch->pcdata->points += group_table[gn].rating[ch->clasb];
		    } else
		    {
			ch->pcdata->points += UMIN(group_table[gn].rating[ch->class],
						   group_table[gn].rating[ch->clasb]);
		    }
		}
	    }
	}
	gn_add(ch,gn); /* make sure all skills in the group are known */
    }
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
{
    int sn, gn;
    
     sn = skill_lookup(name);

    if (sn != -1)
    {
	ch->pcdata->learned[sn] = 0;
	return;
    }
 
    /* now check groups */
 
    gn = group_lookup(name);
 
    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
    {
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
    }
}

