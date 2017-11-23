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
		#include <winsock.h>
		#include <process.h>
	#else
		#include <sys/time.h>
		#include <unistd.h>
		#include <wait.h>
	#endif
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <signal.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_at		);
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_vload		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_transfer	);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_disconnect	);
DECLARE_DO_FUN(do_restore	);
DECLARE_DO_FUN(do_allpeace	);
DECLARE_DO_FUN(do_stance_set	);

bool	write_to_descriptor args( ( int desc, char *txt, int length ) );

/*
 * Local functions.
 */

void do_wiznet( CHAR_DATA *ch, char *argument )
{
   int flag;
   char buf[MAX_STRING_LENGTH];
   int col = 0;
                
   if ( argument[0] == '\0' )
/* Show wiznet options - just like channel command */
   {
        send_to_char("{VWELCOME TO WIZNET!!!\n\r", ch);
        send_to_char("   Option      Status\n\r",ch);
        send_to_char("---------------------\n\r",ch);
        /* list of all wiznet options */
        buf[0] = '\0';
        
        for (flag = 0; wiznet_table[flag].name != NULL; flag++)
        {
            if (wiznet_table[flag].level <= get_trust(ch))
            {
                sprintf( buf, "%-14s %s\t", wiznet_table[flag].name,
		IS_SET(ch->wiznet,wiznet_table[flag].flag) ? "{RON{V" : "OFF" );
                send_to_char(buf, ch);   
                col++;
                if (col==3)
                {
                  send_to_char("\n\r",ch);
                  col=0;
                }
            }
        }
/* To avoid color bleeding */
     send_to_char("{x",ch);
     return;
   }    
 
   if (!str_prefix(argument,"on"))
   {     
        send_to_char("{VWelcome to Wiznet!{x\n\r",ch);
        SET_BIT(ch->wiznet,WIZ_ON);
        return;
   }
                
   if (!str_prefix(argument,"off"))
   {
        send_to_char("{VSigning off of Wiznet.{x\n\r",ch);
        REMOVE_BIT(ch->wiznet,WIZ_ON);
        return;
   }
        
   flag = wiznet_lookup(argument);
        
   if (flag == -1 || get_trust(ch) < wiznet_table[flag].level) 
   {
        send_to_char("{VNo such option.{x\n\r",ch);
        return;
   }

   if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
   {
        sprintf(buf,"{VYou will no longer see %s on wiznet.{x\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
   }
   else  
   {
        sprintf(buf,"{VYou will now see %s on wiznet.{x\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        SET_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
   }
}

void do_levnet( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MSL];

    if (IS_NPC(ch))
	return;
    if (IS_IMMORTAL(ch))
    {
	do_wiznet(ch, argument);
	return;
    }
    if ( argument[0] == '\0' )
    {
      	if (IS_SET(ch->wiznet,WIZ_ON))
      	{
            send_to_char("Signing off of Levnet.\n\r",ch);
            REMOVE_BIT(ch->wiznet,WIZ_ON);
      	}
      	else
      	{
            send_to_char("Welcome to Levnet!\n\r",ch);
            SET_BIT(ch->wiznet,WIZ_ON);
      	}
      	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("Welcome to Levnet!\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("Signing off of Levnet.\n\r",ch);
	REMOVE_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    /* show levnet status */
    if (!str_prefix(argument,"status")) 
    {
	buf[0] = '\0';

	if (!IS_SET(ch->wiznet,WIZ_ON))
	    strcat(buf,"off ");

	for (flag = 0; levnet_table[flag].name != NULL; flag++)
	    if (IS_SET(ch->wiznet,levnet_table[flag].flag))
	    {
		strcat(buf,levnet_table[flag].name);
		strcat(buf," ");
	    }

	strcat(buf,"\n\r");

	send_to_char("Levnet status:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix(argument,"show"))
    /* list of all levnet options */
    {
	buf[0] = '\0';

	for (flag = 0; levnet_table[flag].name != NULL; flag++)
	{
	    if (levnet_table[flag].level <= get_trust(ch))
	    {
	    	strcat(buf,levnet_table[flag].name);
	    	strcat(buf," ");
	    }
	}

	strcat(buf,"\n\r");

	send_to_char("Levnet options available to you are:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }
   
    flag = levnet_lookup(argument);

    if (flag == -1 || get_trust(ch) < levnet_table[flag].level)
    {
	send_to_char("No such option.\n\r",ch);
	return;
    }
   
    if (IS_SET(ch->wiznet,levnet_table[flag].flag))
    {
	sprintf(buf,"You will no longer see %s on levnet.\n\r",
	        levnet_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->wiznet,levnet_table[flag].flag);
    	return;
    }
    else
    {
    	sprintf(buf,"You will now see %s on levnet.\n\r",
		levnet_table[flag].name);
	send_to_char(buf,ch);
    	SET_BIT(ch->wiznet,levnet_table[flag].flag);
	return;
    }

}


void copyover_recover args((void));

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) 
{
    char buf[MSL];
    DESCRIPTOR_DATA *d;

    sprintf(buf, "{V%s{x", string);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->connected == CON_PLAYING
	&&  (IS_HERO(d->character)
	||  (d->character->class >= MCLT_1))
	&&  IS_SET(d->character->wiznet,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch)
        {
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
	  	send_to_char("{Y-->{x ",d->character);
            act_new(buf,d->character,obj,ch,TO_CHAR,POS_DEAD);
        }
    }
 
    return;
}


void do_gquest( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );

    if ( ( arg1[0] == '\0' ) || ( !IS_IMMORTAL(ch) ) )
    {
	if (!global_gquest)
	{
	    send_to_char("There is no quest in progress.\n\r",ch);
	    return;
	}
	if (ch->on_gquest)
	{
	    send_to_char("You'll have to wait till the quest is over.\n\r",ch);
	    return;
	}
	ch->on_gquest = TRUE;
	send_to_char("Your quest flag is now on.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1, "mob"))
    {
	argument = one_argument( argument, arg1 );
	if ( arg1[0] == '\0' )
	{
	    send_to_char("Quest which mobile?\n\r",ch);
	    return;
	}
	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
	if (!IS_NPC(victim))
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}
	if (victim->on_gquest)
	{
	    send_to_char( "Quest flag removed.\n\r", ch );
	    victim->on_gquest = FALSE;
	} else
	{
	    send_to_char( "Quest flag set.\n\r", ch );
	    victim->on_gquest = TRUE;
	}
	return;
    }
    if (!str_cmp(arg1, "on"))
    {
	if (global_gquest)
	{
	    send_to_char("The global quest flag is already on.\n\r",ch);
	    return;
	}
	global_gquest = TRUE;
	send_to_char("The global quest flag is now on.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1, "off"))
    {
	if (!global_gquest)
	{
	    send_to_char("The global quest flag is not on.\n\r",ch);
	    return;
	}
	global_gquest = FALSE;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING )
	    {
		d->character->on_gquest = FALSE;
	    }
	}
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if (!IS_NPC(victim))
		continue;
	    if (!victim->on_gquest)
		continue;
	    victim->on_gquest = FALSE;
	}
	send_to_char("The global quest flag is now off.\n\r",ch);
	return;
    }
    do_gquest(ch, "");
    return;
}
void do_damq( CHAR_DATA *ch, char *argument ) {
    char arg1[MIL];
    argument = one_argument( argument, arg1 );
    if (!str_cmp(arg1, "on")) { global_damq = TRUE; return; }
    if (!str_cmp(arg1, "off")) { global_damq = FALSE; } return;
}
void do_prq( CHAR_DATA *ch, char *argument ) {
    char arg1[MIL];
    argument = one_argument( argument, arg1 );
    if (!str_cmp(arg1, "on")) { global_prq = TRUE; return; }
    if (!str_cmp(arg1, "off")) { global_prq = FALSE; } return;
}
void do_xpq( CHAR_DATA *ch, char *argument ) {
    char arg1[MIL];
    argument = one_argument( argument, arg1 );
    if (!str_cmp(arg1, "on")) { global_xpq = TRUE; return; }
    if (!str_cmp(arg1, "off")) { global_xpq = FALSE; } return;
}

/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int i,sn,vnum;

    if (ch->level > 9 || IS_NPC(ch))
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }

    if (ch->spirit)
    {
	send_to_char( "You have no flesh to carry anything!\n\r", ch);
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
	if (ch->carry_number + 1 > can_carry_n(ch))
	{
	    send_to_char("You can't carry any more items.\n\r",ch);
	    return;
	}
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LIGHT );
	act("$g gives you a light.",ch,NULL,NULL,TO_CHAR);
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
        if (ch->carry_number + 1 > can_carry_n(ch))
        {
            send_to_char("You can't carry any more items.\n\r",ch);
            return;
        }
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
        act("$g gives you a vest.",ch,NULL,NULL,TO_CHAR);
    }

    /* do the weapon thing */
    if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
    {
    	sn = 0; 
    	vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

        if (ch->carry_number + 1 > can_carry_n(ch))
        {
            send_to_char("You can't carry any more items.\n\r",ch);
            return;
        }

    	for (i = 0; weapon_table[i].name != NULL; i++)
    	{
	    if (ch->pcdata->learned[sn] < 
		ch->pcdata->learned[*weapon_table[i].gsn])
	    {
	    	sn = *weapon_table[i].gsn;
	    	vnum = weapon_table[i].vnum;
	    }
    	}

    	obj = create_object(get_obj_index(vnum),0);
     	obj_to_char(obj,ch);
    	equip_char(ch,obj,WEAR_WIELD);
        act("$g gives you a weapon.",ch,NULL,NULL,TO_CHAR);
    }

    if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL 
    ||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) 
    &&  (obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
        if (ch->carry_number + 1 > can_carry_n(ch))
        {
            send_to_char("You can't carry any more items.\n\r",ch);
            return;
        }
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
        act("$g gives you a shield.",ch,NULL,NULL,TO_CHAR);
    }
}

     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg[MIL], buf[MSL];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Nochannel whom?", ch );
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( victim->level >= ch->level )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have restored your channel privileges.\n\r", 
		      victim );
        send_to_char( "NOCHANNELS removed.\n\r", ch );
	sprintf(buf,"$N restores channels to %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have revoked your channel privileges.\n\r", 
		       victim );
        send_to_char( "NOCHANNELS set.\n\r", ch );
	sprintf(buf,"$N revokes %s's channels.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}

void do_authorize( CHAR_DATA *ch, char *argument )
{
    char arg[MIL], buf[MSL];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Authorize whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->newbie != 0 )
    {
	victim->newbie = 0;
	send_to_char( "The gods have given you full channel privileges.\n\r",
	    victim );
	send_to_char( "Newbie status removed.\n\r", ch );
	sprintf(buf,"$N gives channel access to %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    } else {
	send_to_char( "That player is not a newbie.\n\r", ch );
    }

    return;
}


void do_smote(CHAR_DATA *ch, char *argument )
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
    
    if (strstr(argument,ch->name) == NULL)
    {
	send_to_char("You must include your name in an smote.\n\r",ch);
	return;
    }
   
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
 
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;
 
        if ((letter = strstr(argument,vch->name)) == NULL)
        {
	    send_to_char(argument,vch);
	    send_to_char("\n\r",vch);
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
 
	send_to_char(temp,vch);
	send_to_char("\n\r",vch);
    }
 
    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];

    if ( !IS_NPC(ch) )
    {
	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
 
    if ( !IS_NPC(ch) )
    { 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument,ch->name) == NULL)
        {
            send_to_char("You must include your name.\n\r",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    sprintf(buf,"$N denies access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_quit( victim, "" );

    return;
}

void do_wipe( CHAR_DATA *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Wipe whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->comm, COMM_WIPED);
    sprintf(buf,"$N wipes access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_disconnect( ch, victim->name);

    return;
}


void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if (is_number(arg))
    {
	int desc;

	if (ch->level < MAX_LEVEL)
	{
	    return;
	}

	desc = atoi(arg);
    	for ( d = descriptor_list; d != NULL; d = d->next )
    	{
            if ( d->descriptor == desc )
            {
            	close_socket( d );
            	send_to_char( "Ok.\n\r", ch );
            	return;
            }
	}
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || (victim->level > ch->level && victim->level == MAX_LEVEL))
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (IS_SET(victim->act, PLR_KEY)
    && (ch->level < MAX_LEVEL) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d == ch->desc )
	    {
		close_socket( d );
		return;
	    }
	}
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}



void do_twit( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: twit <character>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( (victim->level >= ch->level) && (victim != ch) )
    {
	send_to_char( "Your command backfires!\n\r", ch );
	send_to_char( "You are now considered a TWIT.\n\r", ch );
	SET_BIT( ch->act, PLR_TWIT );
	return;
    }

    if ( IS_SET(victim->act, PLR_TWIT) )
    {
	send_to_char( "Someone beat you to it.\n\r", ch );
    } else
    {
	SET_BIT( victim->act, PLR_TWIT );
	send_to_char( "Twit flag set.\n\r", ch );
	send_to_char( "You are now considered a TWIT.\n\r", victim );
    }
    return;
}


void do_pardon( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: pardon <character>.\n\r", ch );
        return;
    }  

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }  

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
       
    if ( IS_SET(victim->act, PLR_TWIT) )
    {
        REMOVE_BIT( victim->act, PLR_TWIT );
        send_to_char( "Twit flag removed.\n\r", ch );
        send_to_char( "You are no longer a TWIT.\n\r", victim );
    }

    if ( IS_SET(victim->exbit1_flags, RECRUIT) )
    {
        REMOVE_BIT( victim->exbit1_flags, RECRUIT );
        send_to_char( "PK Newbie flag removed.\n\r", ch );
        send_to_char( "You are no longer a PK NEWBIE.\n\r", victim );
    }

    if ( IS_SET(victim->exbit1_flags, PK_VETERAN) )
    {
        REMOVE_BIT( victim->exbit1_flags, PK_VETERAN );
        send_to_char( "PK Veteran flag removed.\n\r", ch );
        send_to_char( "You are no longer a PK VETERAN.\n\r", victim );
    }

    if ( IS_SET(victim->exbit1_flags,PK_KILLER) )
    {
        REMOVE_BIT( victim->exbit1_flags,PK_KILLER );
        send_to_char( "PK Killer flag removed.\n\r", ch );
        send_to_char( "You are no longer a PK KILLER.\n\r", victim );
    }
		
    if ( IS_SET(victim->exbit1_flags,PK_KILLER2) )
    {
        REMOVE_BIT( victim->exbit1_flags,PK_KILLER2 );
        send_to_char( "PK Killer2 flag removed.\n\r", ch );
        send_to_char( "You are no longer a PK KILLER2.\n\r", victim );
    }		
			
    return;
}




void do_dmecho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "DM echo what?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
		send_to_char( "{RDM {M[ ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( " {M]{x\n\r",   d->character );
	}
    }

    return;
}

void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (d->character->level >= ch->level)
		send_to_char( "global> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_wecho( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Warn echo what?\n\r", ch );
	return;
    }

    sprintf(buf, "{z{B***{x {R%s{x {z{B***{x", argument);
    do_echo(ch, buf);
    do_echo(ch, buf);
    do_echo(ch, buf);
    do_restore(ch, "all");
    do_allpeace(ch, "");
    return;
}

void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
            if (d->character->level >= ch->level)
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Zone echo what?\n\r",ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (d->character->level >= ch->level)
		send_to_char("zone> ",d->character);
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (victim->level >= ch->level)
        send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}

void do_corner( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[MSL];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Corner whom?\n\r", ch);
	return;
    }

    sprintf(buf, "%s %d", arg, ROOM_VNUM_CORNER);
    do_transfer( ch, buf );

    return;
}

void do_arena( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[MSL];
    int vnum;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Arena whom?\n\r", ch);
        return;
    }

    vnum = number_range(9,35);
    if (vnum < 10)
	vnum = 10;
    if (vnum > 34)
	vnum = 34;

    sprintf(buf, "Transfering %s to vnum #%d\n\r", arg, vnum);
    send_to_char(buf,ch);
    buf[0] = '\0';
    sprintf(buf, "%s %d", arg, vnum);
    do_transfer( ch, buf );

    return;
}

void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) && (ch->level >= CREATOR))
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   ch->level >= d->character->ghost_level
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MSL];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( ch,location ) 
	&&  ch->level < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (( victim->level > ch->level
    && !IS_SET(ch->act, PLR_KEY) 
    && (victim->level != MAX_LEVEL))
    || ((IS_SET(victim->act, PLR_KEY)) 
    && (ch->level != MAX_LEVEL)))
    {
        send_to_char( "You failed!\n\r", ch);
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}

void do_allpeace( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   ch->level >= d->character->ghost_level
	&&   can_see( ch, d->character ) )
	{
	    char buf[MSL];
	    sprintf( buf, "%s peace", d->character->name );
	    do_at( ch, buf );
	}
    }
}

void do_wedpost( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: wedpost <char>\n\r",ch);
        return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch );
        return;
    }
    
    if (victim->wedpost)
    {
        send_to_char("They are no longer allowed to post wedding announcements.\n\r",ch);
        victim->wedpost = FALSE;
    }
    else
    {
        send_to_char("They are now allowed to post wedding announcements.\n\r",ch);
        victim->wedpost = TRUE;
    }
}

void do_recover( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Recover whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim) && !IS_SET(ch->act,ACT_PET) )
    {
	send_to_char( "You can't recover NPC's.\n\r", ch );
	return;
    }

    if (( get_trust( victim ) > get_trust( ch )
    && !IS_SET(ch->act, PLR_KEY) 
    && (victim->level != MAX_LEVEL))
    || ((IS_SET(victim->act, PLR_KEY)) 
    && (ch->level != MAX_LEVEL)))
    {
        send_to_char( "You failed!\n\r", ch);
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
    {
	send_to_char( "They are fighting.\n\r", ch );
	return;
    }

    if ( victim->alignment < 0 )
    {
	if ( ( location = get_room_index( ROOM_VNUM_TEMPLEB ) ) == NULL )
	{
	    send_to_char( "The recall point seems to be missing.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
	{
	    send_to_char( "The recall point seems to be missing.\n\r", ch );
	    return;
	}
    }

    if (is_clan(victim)
    && (clan_table[victim->clan].hall != ROOM_VNUM_ALTAR)
    && !IS_SET(victim->act, PLR_TWIT))
	location = get_room_index( clan_table[victim->clan].hall );

    if (IS_NPC(victim) && IS_SET(ch->act,ACT_PET)
    && is_clan(victim->master)
    && (clan_table[victim->master->clan].hall != ROOM_VNUM_ALTAR)
    && !IS_SET(victim->master->act, PLR_TWIT)) 
        location = get_room_index( clan_table[victim->master->clan].hall );

    if ( victim->in_room == location )
    {
	act( "$N does not need recovering.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    &&   !IS_AFFECTED(victim, AFF_CURSE))
    {
	act( "$N does not need recovering.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a flash.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a flash of light.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has recovered you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    act( "$N has been recovered.", ch, NULL, victim, TO_CHAR);
    if (victim->pet != NULL)
	do_recover(victim->pet,"");
}

void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;
    
    if (IS_NPC(ch))
    {
	send_to_char( "NPC's cannot use this command.\n\r", ch);
	return;
    }
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( ch,location ) 
    &&  ch->level < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    char arg[MIL];
    int count = 0;

    if ( ( argument[0] == '\0' )
	&& ( IS_NPC(ch) ) )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }
    if ( ( argument[0] == '\0' )
	&& ( !ch->pcdata->recall ) ) 
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }
    if ( ( argument[0] == '\0' )
      && (ch->pcdata->recall) )
    {
	sprintf(arg, "%d", ch->pcdata->recall);
    }
    else
    {
	sprintf(arg, "%s", argument);
    }
    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    count = 0;
    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
        count++;

    if (!is_room_owner(ch,location) && room_is_private(ch,location) 
    &&  (count > 1 || ch->level < MAX_LEVEL))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if ((rch->level >= ch->invis_level)
	&& (rch->level >= ch->ghost_level))
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if ((rch->level >= ch->invis_level)
	&& (rch->level >= ch->ghost_level))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
    if ( ( argument[0] == '\0' )
      && (ch->pet != NULL) )
    {
	char_from_room( ch->pet );
	char_to_room( ch->pet, location );
    }
    do_look( ch, "auto" );
    return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }
 
    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if (!room_is_private( ch,location ))
    {
        send_to_char( "That room isn't private, use goto.\n\r", ch );
        return;
    }
 
    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if ((rch->level >= ch->invis_level)
	&& (rch->level >= ch->ghost_level))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    char_from_room( ch );
    char_to_room( ch, location );
 
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if ((rch->level >= ch->invis_level)
	&& (rch->level >= ch->ghost_level))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    do_look( ch, "auto" );
    return;
}

void do_ftick ( CHAR_DATA *ch, char *argument )
{
    update_handler( TRUE );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MIL];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }
   
   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private( ch,location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: {G'%s'{x\n\rArea: {B'%s'{x\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: {Y%d{x  Sector: {Y%d{x  Light: {Y%d{x  Healing: {Y%s%d{x  Mana: {Y%d{x\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	(location->heal_neg) ? "-": "",
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    if (location->clan)
    {
	sprintf( buf, "This room is owned by the [{%s%s{x] clan.\n\r",
	    clan_table[location->clan].pkill ? "B" : "M",
	    clan_table[location->clan].who_name);
	send_to_char( buf, ch );
    }

    if (location->guild != NULL)
    {
	GUILD_DATA *guild;

	send_to_char( "Guild Hall: ", ch);
	for (guild = location->guild; guild != NULL; guild = guild->next)
	{
	    sprintf(buf, "{%s%c{%s%c%c%c%c ",
		guild->guild < MCLT_1 ? "R" : "B",
		class_table[guild->guild].who_name[0],
		guild->guild < MCLT_1 ? "r" : "b",
		class_table[guild->guild].who_name[1],
		class_table[guild->guild].who_name[2],
		class_table[guild->guild].who_name[3],
		class_table[guild->guild].who_name[4]);
	    send_to_char(buf, ch);
	}
	send_to_char("{x\n\r", ch);
    }
    if (location->race != NULL)
    {
        RACE_DATA *race;

        send_to_char( "Racial Hall: ", ch);
        for (race = location->race; race != NULL; race = race->next)
        {
            sprintf(buf, "{G%s ",
                race_table[race->race].name);
            send_to_char(buf, ch);
        }
        send_to_char("{x\n\r", ch);
    }
/*
    if (location->warn[0] != '\0')
    {
	send_to_char( "Warning: ", ch);
	send_to_char( location->warn, ch);
	send_to_char( "\n\r", ch);
    }
*/
    sprintf( buf,
	"Room flags: {C%s{x.\n\rDescription:\n\r%s",
	room_bit_name(location->room_flags),
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: {B'", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'{x.\n\r", ch );
    }

    send_to_char( "Characters:{R", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (( get_trust(ch) >= rch->ghost_level)
	&& (can_see(ch,rch)))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".{x\n\rObjects:{G   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".{x\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf, "Door: {Y%d.{x  To: {Y%d.{x  Key: {Y%d.{x  Exit flags: {Y%d.{x\n\rKeyword: '%s',  Description: %s ", 
	    door, (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    pexit->key, pexit->exit_info, pexit->keyword,
	    pexit->description[0] != '\0' ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    return;
}




void do_olevel(CHAR_DATA *ch, char *argument)
{
    char buf[MIL];
    char level[MIL];
    char name[MIL];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    argument = one_argument(argument, level);
    if (level[0] == '\0')
    {
        send_to_char("Syntax: olevel <level>\n\r",ch);
        send_to_char("        olevel <level> <name>\n\r",ch);
        return;
    }

    argument = one_argument(argument, name);
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( obj->level != atoi(level) )
            continue;

	if ( name[0] != '\0' && !is_name(name, obj->name) )
	    continue;

        found = TRUE;
        number++;

        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
        &&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
                in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                number, obj->short_descr,in_obj->in_room->name,
                in_obj->in_room->vnum);
        else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);

        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);

        if (number >= max_found)
            break;
    }

    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}

void do_mlevel( CHAR_DATA *ch, char *argument )
{
    char buf[MIL];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char("Syntax: mlevel <level>\n\r",ch);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
        if ( victim->in_room != NULL
        &&   atoi(argument) == victim->level )
        {
            found = TRUE;
            count++;
            sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                IS_NPC(victim) ? victim->short_descr : victim->name,
                victim->in_room->vnum,
                victim->in_room->name );
            add_buf(buffer,buf);
        }
    }

    if ( !found )
        act( "You didn't find any mob of level $T.", ch, NULL, argument, TO_CHAR );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}


void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    sprintf( buf, "Name(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_type_name(obj->pIndexData), obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    if (obj->quest) 
	send_to_char("This is a quest item.\n\r",ch); 

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "someone",
	obj->wear_loc );
    send_to_char( buf, ch );
    
    sprintf( buf, "Values: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type )
    {
    	case ITEM_SCROLL: 
    	case ITEM_POTION:
    	case ITEM_PILL:
	    sprintf( buf, "Level %d spells of:", obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[1]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[2]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	    {
		send_to_char(" '",ch);
		send_to_char(skill_table[obj->value[4]].name,ch);
		send_to_char("'",ch);
	    }

	    send_to_char( ".\n\r", ch );
	break;

    	case ITEM_WAND: 
    	case ITEM_STAFF: 
	    sprintf( buf, "Has %d(%d) charges of level %d",
	    	obj->value[1], obj->value[2], obj->value[0] );
	    send_to_char( buf, ch );
      
	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    send_to_char( ".\n\r", ch );
	break;

	case ITEM_DRINK_CON:
	    sprintf(buf,"It holds %s-colored %s.\n\r",
		liq_table[obj->value[2]].liq_color,
		liq_table[obj->value[2]].liq_name);
	    send_to_char(buf,ch);
	    break;
		
      
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC): 
		    send_to_char("exotic\n\r",ch);
		    break;
	    	case(WEAPON_SWORD): 
		    send_to_char("sword\n\r",ch);
		    break;	
	    	case(WEAPON_DAGGER): 
		    send_to_char("dagger\n\r",ch);
		    break;
	    	case(WEAPON_SPEAR):
		    send_to_char("spear/staff\n\r",ch);
		    break;
	    	case(WEAPON_MACE): 
		    send_to_char("mace/club\n\r",ch);	
		    break;
	   	case(WEAPON_AXE): 
		    send_to_char("axe\n\r",ch);	
		    break;
	    	case(WEAPON_FLAIL): 
		    send_to_char("flail\n\r",ch);
		    break;
	    	case(WEAPON_WHIP): 
		    send_to_char("whip\n\r",ch);
		    break;
	    	case(WEAPON_POLEARM): 
		    send_to_char("polearm\n\r",ch);
		    break;
	    	default: 
		    send_to_char("unknown\n\r",ch);
		    break;
 	    }
	    if (obj->clan)
	    {
		sprintf( buf, "Damage is variable.\n\r");
	    } else
	    {
		if (obj->pIndexData->new_format)
		    sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
		else
		    sprintf( buf, "Damage is %d to %d (average %d)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    }
	    send_to_char( buf, ch );

	    sprintf(buf,"Damage noun is %s.\n\r",
		attack_table[obj->value[3]].noun);
	    send_to_char(buf,ch);
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }
	break;

    	case ITEM_ARMOR:
	    if (obj->clan)
	    {
		sprintf( buf, "Armor class is variable.\n\r");
	    } else
	    {
		sprintf( buf, 
		"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    }
	    send_to_char( buf, ch );
	break;

        case ITEM_CONTAINER:
        case ITEM_PIT:
            sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
            send_to_char(buf,ch);
            if (obj->value[4] != 100)
            {
                sprintf(buf,"Weight multiplier: %d%%\n\r",
		    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;
    }

    if (is_clan_obj(obj))
    {
	sprintf( buf, "This object is owned by the [{%s%s{x] clan.\n\r",
	    clan_table[obj->clan].pkill ? "B" : "M",
	    clan_table[obj->clan].who_name
	    );
	send_to_char( buf, ch );
    }

    if (is_class_obj(obj))
    {
	GUILD_DATA *guild;

	send_to_char( "Guild Object: ", ch);
	for (guild = obj->pIndexData->guild; guild != NULL; guild = guild->next)
	{
	    sprintf(buf, "{%s%c{%s%c%c%c%c ",
		guild->guild < MCLT_1 ? "R" : "B",
		class_table[guild->guild].who_name[0],
		guild->guild < MCLT_1 ? "r" : "b",
		class_table[guild->guild].who_name[1],
		class_table[guild->guild].who_name[2],
		class_table[guild->guild].who_name[3],
		class_table[guild->guild].who_name[4]);
	    send_to_char(buf, ch);
	}
	send_to_char("{x\n\r", ch);
    }

    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    sprintf(buf,", %d hours.\n\r",paf->duration);
	else
	    sprintf(buf,".\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS:
		    sprintf(buf,"Adds %s affect.\n",
			affect_bit_name(paf->bitvector));
		    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n",
                        weapon_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    sprintf(buf,"Adds %s object flag.\n",
			extra_bit_name(paf->bitvector));
		    break;
		case TO_IMMUNE:
		    sprintf(buf,"Adds immunity to %s.\n",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_RESIST:
		    sprintf(buf,"Adds resistance to %s.\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_VULN:
		    sprintf(buf,"Adds vulnerability to %s.\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_SHIELDS:
		    sprintf(buf,"Adds %s shield.\n",
			shield_bit_name(paf->bitvector));
		    break;
		default:
		    sprintf(buf,"Unknown bit %d: %d\n\r",
			paf->where,paf->bitvector);
		    break;
	    }
	    send_to_char(buf,ch);
	}
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n",
                        affect_bit_name(paf->bitvector));
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra_bit_name(paf->bitvector));
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_SHIELDS:
                    sprintf(buf,"Adds %s shield.\n",
                        shield_bit_name(paf->bitvector));
                    break;
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL];
    BUFFER *output;
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL 
    || (victim->level > ch->level && victim->level == MAX_LEVEL))
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    output = new_buf();

    if (!IS_NPC(victim))
    {
	sprintf( buf, "Name: %s\n\rSocket: %s\n\r",
	    victim->name, victim->pcdata->socket);
    } else
    {
	sprintf( buf, "Name: %s\n\rSocket: <mobile>\n\r",
	    victim->name);
    }
    add_buf(output,buf);

    sprintf( buf, 
	"Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name,
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    add_buf(output,buf);

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: %d  Killed: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	add_buf(output,buf);
    } else
    {
	if ( victim->pcdata->deity )
        {
            sprintf( buf, "Deity: %s   Favor: %d ", victim->pcdata->deity_name,
                victim->pcdata->favor );
            add_buf( output, buf );
        }
    }

    sprintf( buf, 
   	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    add_buf(output,buf);

    sprintf( buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	IS_NPC(ch) ? 0 : victim->practice );
    add_buf(output,buf);

    if (IS_NPC(victim) )
    {
	sprintf( buf,
	    "Lv: %d  Class: %s  Align: %d  Exp: %ld\n\r",
	    victim->level,       
	    "mobile",            
	    victim->alignment,
	    victim->exp );
    } else if (victim->pcdata->tier != 2)
    {
	sprintf( buf,
	    "Lv: %d  Class: %s  Align: %d  Exp: %ld\n\r",
	    victim->level,       
	    class_table[victim->class].name,            
	    victim->alignment,
	    victim->exp );
    } else
    {
	sprintf( buf,
	    "Lv: %d  Class: %s/%s  Align: %d  Exp: %ld\n\r",
	    victim->level,       
	    class_table[victim->class].name,            
	    class_table[victim->clasb].name,            
	    victim->alignment,
	    victim->exp );
    }
    add_buf(output,buf);

    sprintf( buf,
	"Platinum: %ld  Gold: %ld  Silver: %ld\n\r",
	victim->platinum, victim->gold, victim->silver );
    add_buf(output,buf);

    if (!IS_NPC(victim))
    {
	sprintf( buf,
	    "Bank-0: %ld  Bank-1: %ld  Bank-2: %ld  Bank-3: %ld\n\r",
	    victim->balance[0] > 0 ? victim->balance[0] : 0,
	    victim->balance[1] > 0 ? victim->balance[1] : 0,
	    victim->balance[2] > 0 ? victim->balance[2] : 0,
	    victim->balance[3] > 0 ? victim->balance[3] : 0 );
	add_buf(output,buf);
    }

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    add_buf(output,buf);

    sprintf( buf, 
	"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	size_table[victim->size].name, position_table[victim->position].name,
	victim->wimpy );
    add_buf(output,buf);

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	add_buf(output,buf);
    }
    sprintf( buf, "Fighting: %s\n\r",
	victim->fighting ? victim->fighting->name : "(none)" );
    add_buf(output,buf);

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d  Quest: %d\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_HUNGER],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->aqps );
	add_buf(output,buf);
    }

    sprintf( buf, "Carry number: %d  Carry weight: %ld\n\r",
	victim->carry_number, get_carry_weight(victim) / 10 );
    add_buf(output,buf);

    if (!IS_NPC(victim))
    {
    	sprintf( buf, 
	    "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
	    get_age(victim), 
	    (int) (victim->played + current_time - victim->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer );
	add_buf(output,buf);
    }

    sprintf(buf, "Act: %s\n\r",act_bit_name(victim->act));
    add_buf(output,buf);
    
    if (victim->comm)
    {
    	sprintf(buf,"Comm: %s\n\r",comm_bit_name(victim->comm));
	add_buf(output,buf);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
    	sprintf(buf, "Offense: %s\n\r",off_bit_name(victim->off_flags));
	add_buf(output,buf);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune: %s\n\r",imm_bit_name(victim->imm_flags));
	add_buf(output,buf);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "Resist: %s\n\r", imm_bit_name(victim->res_flags));
	add_buf(output,buf);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "Vulnerable: %s\n\r", imm_bit_name(victim->vuln_flags));
	add_buf(output,buf);
    }

    sprintf(buf, "Form: %s\n\rParts: %s\n\r", 
	form_bit_name(victim->form), part_bit_name(victim->parts));
    add_buf(output,buf);

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by %s\n\r", 
	    affect_bit_name(victim->affected_by));
	add_buf(output,buf);
    }

    if (victim->shielded_by)
    {
	sprintf(buf, "Shielded by %s\n\r", 
	    shield_bit_name(victim->shielded_by));
	add_buf(output,buf);
    }

    sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)");
    add_buf(output,buf);

    if (!IS_NPC(victim))
    {
	sprintf( buf, "Security: %d.\n\r", victim->pcdata->security );	/* OLC */
	send_to_char( buf, ch );					/* OLC */
    }

    sprintf( buf, "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    add_buf(output,buf);

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	sprintf(buf,"Mobile has special procedure %s.\n\r",
		spec_name(victim->spec_fun));
	add_buf(output,buf);
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    paf->where == TO_SHIELDS ?
	    shield_bit_name( paf->bitvector ) :
	    affect_bit_name( paf->bitvector ),
	    paf->level
	    );
	add_buf(output,buf);
    }
    page_to_char( buf_string(output), ch );
    free_buf(output);
    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MIL];
    char buf[MSL];
    BUFFER *output;
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	send_to_char("  vnum areas\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_slookup(ch,string);
	return;
    }

    if (!str_cmp(arg,"areas") || !str_cmp(arg,"area"))
    {
	output = new_buf();
	iAreaHalf = (top_area + 1) / 2;
	pArea1    = area_first;
	pArea2    = area_first;
	for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	    pArea2 = pArea2->next;

	for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	{
	    sprintf( buf, "%-26s {R%5d %5d{x  %-26s {R%5d %5d{x\n\r",
		pArea1->name, pArea1->min_vnum, pArea1->max_vnum,
		(pArea2 != NULL) ? pArea2->name : "",
		(pArea2 != NULL) ? pArea2->min_vnum : 0,
		(pArea2 != NULL) ? pArea2->max_vnum : 0);
	    add_buf(output,buf);
	    pArea1 = pArea1->next;
	    if ( pArea2 != NULL )
		pArea2 = pArea2->next;
	}
	page_to_char( buf_string(output), ch );
	free_buf(output);
 	return;
    }

    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MSL];
    char arg[MIL];
    BUFFER *output;
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    int count;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;
    count	= 0;
    output	= new_buf();

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		count++;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
		add_buf(output,buf);
	    }
	}
	if ( count >= 200 )
	    break;
    }

    if ( !found )
    {
	send_to_char( "No mobiles by that name.\n\r", ch );
    }
    else
    {
	page_to_char( buf_string(output), ch );
    }
    free_buf(output);
    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MSL];
    char arg[MIL];
    BUFFER *output;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    int count;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;
    count	= 0;
    output	= new_buf();

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		count++;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr );
		add_buf(output,buf);
	    }
	}
	if ( count >= 200 )
	    break;
    }

    if ( !found )
    {
	send_to_char( "No objects by that name.\n\r", ch );
    }
    else
    {
	page_to_char( buf_string(output), ch );
    }
    free_buf(output);
    return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MIL];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name )
        ||   ch->level < obj->level
	||   (obj->carried_by != NULL && !can_see(ch,obj->carried_by)))
            continue;
 
        found = TRUE;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                number, obj->short_descr,in_obj->in_room->name, 
	   	in_obj->in_room->vnum);
	else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);
 
        if (number >= max_found)
            break;
    }
 
    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	DESCRIPTOR_DATA *d;

	/* show characters logged */

	buffer = new_buf();
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->character != NULL && d->connected == CON_PLAYING
	    &&  d->character->in_room != NULL && can_see(ch,d->character)
	    &&  can_see_room(ch,d->character->in_room))
	    {
		victim = d->character;
		if ((victim->level <= CREATOR && ch->level <= CREATOR)
		    || ch->level > CREATOR)
		{
		    count++;
		    if (d->original != NULL)
			sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
		    else
			sprintf(buf,"%3d) %s is in %s [%d]\n\r",
			count, victim->name,victim->in_room->name,
			victim->in_room->vnum);
		    add_buf(buffer,buf);
		}
	    }
	}

        page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( victim->in_room != NULL
	&&   is_name( argument, victim->name ) )
	{
	    if ((victim->level <= CREATOR && ch->level <= CREATOR)
		|| ch->level > CREATOR)
	    {
		found = TRUE;
		count++;
		sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
		add_buf(buffer,buf);
	    }
	}
	if ( count >= 200 )
	    break;
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
    	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;

    if (ch->invis_level < LEVEL_HERO)
    {
    	sprintf( buf, "Reboot by %s.", ch->name );
    	do_echo( ch, buf );
    }
    do_force ( ch, "all save");
    do_save (ch, "");
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
    	close_socket(d);
    }
    
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;

    if (ch->invis_level < LEVEL_HERO)
    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    if (ch->invis_level < LEVEL_HERO)
    	do_echo( ch, buf );
    do_force ( ch, "all save");
    do_save (ch, "");
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	close_socket(d);
    }
    return;
}

void do_protect( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Protect whom from snooping?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
	send_to_char("You can't find them.\n\r",ch);
	return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("Your snoop-proofing was just removed.\n\r",victim);
	REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
    else
    {
	act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("You are now immune to snooping.\n\r",victim);
	SET_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
}
  


void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MSL];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	if (!IS_TRUSTED(ch,IMPLEMENTOR))
	{
	    wiznet("$N stops being such a snoop.",
		ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	}
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(ch,victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That character is in a private room.\n\r",ch);
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) 
    || ( IS_SET(victim->comm,COMM_SNOOP_PROOF) && !IS_TRUSTED(ch,IMPLEMENTOR)))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    if (!IS_TRUSTED(ch,IMPLEMENTOR))
    {
	sprintf(buf,"$N starts snooping on %s",
	    (IS_NPC(ch) ? victim->short_descr : victim->name));
	wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    }
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MIL], buf[MSL];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if (victim->level > ch->level)
    {
	send_to_char("That character is too powerful for you to handle.\n\r",ch);
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(ch,victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( "You return to your original body. Type replay to see any missed tells.\n\r", ch );
    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 105)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 100)
	|| (IS_TRUSTED(ch,KNIGHT)    && obj->level <= 20)
	|| (IS_TRUSTED(ch,SQUIRE)   && obj->level ==  5))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}
	if (obj->item_type == ITEM_EXIT)
	{
	    send_to_char("You cannot clone an exit object.\n\r",ch);
	    return;
	}
	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MSL];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 100 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 90 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level > 85 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,KNIGHT))
	||  !IS_TRUSTED(ch,SQUIRE))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MIL];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	if (ch->level >= CREATOR)
	    send_to_char("  load voodoo <player>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"voodoo") && (ch->level >= CREATOR))
    {
	do_vload(ch,argument);
	return;
    }
    /* echo syntax */
    do_load(ch,"");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MSL];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL] ,arg2[MIL];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }
    if (pObjIndex->item_type == ITEM_EXIT)
    {
	send_to_char("You cannot load an exit object.\n\r",ch);
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_vload( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    char buf[MSL];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;
    bool found = FALSE;
    char *name;
   
    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0')
    {
        send_to_char( "Syntax: load voodoo <player>\n\r", ch );
        return;
    }
    
    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;

	wch = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg1,wch->name) && !found)
	{
	    if (IS_NPC(wch))
		continue;

	    if (wch->level > ch->level)
		continue;

	    found = TRUE;
 
	    if ( ( pObjIndex = get_obj_index( OBJ_VNUM_VOODOO ) ) == NULL )
	    {
		send_to_char( "Cannot find the voodoo doll vnum.\n\r", ch );
		return;
	    }
	    obj = create_object( pObjIndex, 0 );
	    name = wch->name;
	    sprintf( buf, obj->short_descr, name );
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( buf );
	    sprintf( buf, obj->description, name );
	    free_string( obj->description );
	    obj->description = str_dup( buf );
	    sprintf( buf, obj->name, name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	    if ( CAN_WEAR(obj, ITEM_TAKE) )
		obj_to_char( obj, ch );
	    else
		obj_to_room( obj, ch->in_room );
	    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
	    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }
    send_to_char("No one of that name is playing.\n\r",ch);
    return;
}

void do_randclan( CHAR_DATA *ch, char *argument )
{
    randomize_entrances( ROOM_VNUM_CLANS );
    send_to_char("Clan entrances have been moved.\n\r",ch);
    return;
}

void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	if (get_trust(ch) <= DEITY)
	{
	  send_to_char("Not against PC's!\n\r",ch);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    	if (victim->level > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_chlevel( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char buf[MIL];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: chlevel <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > 110 )
    {
	send_to_char( "Level must be 1 to 110.\n\r", ch );
	return;
    }
/*
    if ( ch == victim )
    {
	send_to_char( "You may not lower your own level.\n\r", ch );
	return;
    }
 */
    if ( ch->level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    if ( ch->level < victim->level )
    {
	send_to_char( "Its not nice to mess with the higher level imms.\n\r", ch );
	act("$n just tried to demote you!!",ch,NULL,victim,TO_VICT);
	return;
    }
/*
    if ( ch->trust < victim->level )
    {
	send_to_char( "Play nice with your elders!!\n\r", ch );
	act("$n doesn't like to play fair!!",ch,NULL,victim,TO_VICT);
	return;
    }
 */

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "{R******** {GOOOOHHHHHHHHHH  NNNNOOOO {R*******{x\n\r", victim );
	sprintf(buf, "{R**** {WYou've been demoted to level %d {R****{x\n\r", level );
	send_to_char(buf, victim);
	if ((victim->level > HERO) || (level > HERO))
	{
	    update_wizlist(victim, level);
	}
	temp_prac = victim->practice;
	victim->level    = 1;
	victim->exp      = exp_per_level(victim,victim->pcdata->points);
	victim->max_hit  = 100;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	change_level_quiet( victim );
	victim->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "{B******* {GOOOOHHHHHHHHHH  YYYYEEEESSS {B******{x\n\r", victim );
	sprintf(buf, "{B**** {WYou've been advanced to level %d {B****{x\n\r", level );
	send_to_char(buf, victim);
	if ((victim->level > HERO) || (level > HERO))
	{
	    update_wizlist(victim, level);
	}
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level += 1;
	change_level_quiet( victim );
    }
    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
		  * UMAX( 1, victim->level );
    victim->trust = 0;
    save_char_obj(victim);
    return;
}


void do_knight( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    CHAR_DATA *victim;
    int level;
    int iLevel;
 
    argument = one_argument( argument, arg1 );

    if (!IS_SET(ch->act, PLR_KEY))
    {
        send_to_char( "This function is not currently implemented.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: knight <char>.\n\r", ch );
        return;
    }
 
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }
 
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    level = 103;
 
    if ( level <= victim->level )
    {
	return;
    }
    else
    {
	act( "You touch $Ns shoulder with a sword called {GKnight's Faith{x.", ch,NULL,victim,TO_CHAR);
	act("$n touches your shoulder with a sword called {GKnight's Faith{x.",ch,NULL,victim,TO_VICT);
	act("$n touches $Ns shoulder with a sword called {GKnight's Faith{x.",ch,NULL,victim,TO_NOTVICT);
	act("$N glows with an unearthly light as $S mortality slips away.",ch,NULL,victim,TO_NOTVICT);
    }
    update_wizlist(victim, level);
    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
        send_to_char( "You raise a level!!  ", victim );
        victim->level += 1;
        change_level_quiet( victim );
    }
    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
                  * UMAX( 1, victim->level );
    victim->trust = 0;
    save_char_obj(victim);
    return;
}

void do_squire( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    CHAR_DATA *victim;
    int level;
    int iLevel;
 
    argument = one_argument( argument, arg1 );

    if (!IS_SET(ch->act, PLR_KEY))
    {
        send_to_char( "This function is not currently implemented.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: squire <char>.\n\r", ch );
        return;
    }
 
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }
 
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    level = 102;
 
    if ( level <= victim->level )
    {
	return;
    }
    else
    {
	act( "You touch $Ns shoulder with a sword called {BSquire's Faith{x.", ch,NULL,victim,TO_CHAR);
	act("$n touches your shoulder with a sword called {BSquire's Faith{x.",ch,NULL,victim,TO_VICT);
	act("$n touches $Ns shoulder with a sword called {BSquire's Faith{x.",ch,NULL,victim,TO_NOTVICT);
	act("$N glows with an unearthly light as $S mortality slips away.",ch,NULL,victim,TO_NOTVICT);
    }
    update_wizlist(victim, level);
    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
        send_to_char( "You raise a level!!  ", victim );
        victim->level += 1;
        change_level_quiet( victim );
    }
    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
                  * UMAX( 1, victim->level );
    victim->trust = 0;
    save_char_obj(victim);
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > 110 )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 110.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You may not lower your own trust level.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    if ( ch->level < victim->level )
    {
        send_to_char( "Its not nice to mess with the higher level imms.\n\r", ch);
        act("$n just tried to lower your trust.",ch,NULL,victim,TO_VICT);
        return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MIL], buf[MSL];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
	    if ( IS_SET(vch->act, PLR_NORESTORE) )
	    {
		act("$n attempts to restore you, but fails.",ch,NULL,vch,TO_VICT);
	    } else {
		affect_strip(vch,gsn_plague);
		affect_strip(vch,gsn_poison);
		affect_strip(vch,gsn_blindness);
		affect_strip(vch,gsn_sleep);
		affect_strip(vch,gsn_curse);

		vch->hit 	= vch->max_hit;
		vch->mana	= vch->max_mana;
		vch->move	= vch->max_move;
		update_pos( vch);
		act("$n has restored you.",ch,NULL,vch,TO_VICT);
	    }
        }

        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
        
        send_to_char("Room restored.\n\r",ch);
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL - 2 && !str_cmp(arg,"all"))
    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            if ( IS_SET(victim->act, PLR_NORESTORE) )  
            {
                act("$n attempts to restore you, but fails.",ch,NULL,victim,TO_VICT);          
            } else {
		affect_strip(victim,gsn_plague);
		affect_strip(victim,gsn_poison);
		affect_strip(victim,gsn_blindness);
		affect_strip(victim,gsn_sleep);
		affect_strip(victim,gsn_curse);
            
		victim->hit 	= victim->max_hit;
		victim->mana	= victim->max_mana;
		victim->move	= victim->max_move;
		update_pos( victim);
		if (victim->in_room != NULL)
		    act("$n has restored you.",ch,NULL,victim,TO_VICT);
	    }
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NORESTORE) )
    {
	act("$n attempts to restore you, but fails.",ch,NULL,victim,TO_VICT);
	send_to_char( "You failed.\n\r", ch );
	return;
    }
    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_immkiss( CHAR_DATA *ch, char *argument )
{
    char arg[MIL], buf[MSL];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if (arg[0] == '\0')
    {
	send_to_char( "Who do you want to kiss?\n\r", ch );
	return;
    }
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( ch->in_room != victim->in_room )
    {
	send_to_char( "Your lips aren't that long!\n\r", ch);
	return;
    }
    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n kisses you, and you feel a sudden rush of adrenaline.", ch, NULL, victim, TO_VICT );
    send_to_char( "You feel MUCH better now!\n\r", victim);
    send_to_char( "They feel MUCH better now!\n\r", ch);
    sprintf(buf,"$N immkissed %s",
        IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    return;
}

 	
void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
	sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}

void do_norestore( CHAR_DATA *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Norestore whom?\n\r", ch );
        return;
    }
       
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
       
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
       
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
       
    if ( IS_SET(victim->act, PLR_NORESTORE) )
    {
        REMOVE_BIT(victim->act, PLR_NORESTORE);
        send_to_char( "NORESTORE removed.\n\r", ch );
        sprintf(buf,"$N allows %s restores.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->act, PLR_NORESTORE);
        send_to_char( "NORESTORE set.\n\r", ch );
        sprintf(buf,"$N denys %s restores.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
       
    save_char_obj( victim );
 
    return;
}


void do_notitle( CHAR_DATA *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Notitle whom?\n\r", ch );
        return;
    }
       
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
       
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
       
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
       
    if ( IS_SET(victim->act, PLR_NOTITLE) )
    {
        REMOVE_BIT(victim->act, PLR_NOTITLE);
        send_to_char( "NOTITLE removed.\n\r", ch );
        sprintf(buf,"$N allows %s title.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	if ( IS_SET(victim->comm, COMM_TITLE_LOCK) )
	{
	    REMOVE_BIT(victim->comm, COMM_TITLE_LOCK);
	}
        SET_BIT(victim->act, PLR_NOTITLE);
        send_to_char( "NOTITLE set.\n\r", ch );
        sprintf(buf,"$N denys %s title.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
       
    save_char_obj( victim );
 
    return;
}

void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
	sprintf(buf,"$N restores emotes to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
	sprintf(buf,"$N revokes %s's emotes.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	send_to_char( "NOSHOUT removed.\n\r", ch );
	sprintf(buf,"$N restores shouts to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	send_to_char( "NOSHOUT set.\n\r", ch );
	sprintf(buf,"$N revokes %s's shouts.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MIL],buf[MSL];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
	sprintf(buf,"$N restores tells to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
	sprintf(buf,"$N revokes %s's tells.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL ) {
	    stop_fighting( rch, TRUE );
	    if (!IS_NPC(rch) ) {
		send_to_char( "Ok.\n\r", ch );
	    }
	}
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
    {
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 
    if ( newlock )
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	send_to_char("  set char  <name> <field> <value>\n\r",ch);
		send_to_char("  set stance	<name> <stance type> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"stance") )
    { 
	do_stance_set(ch,argument); 
	return; 
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }
    /* echo syntax */
    do_set(ch,"");
}



void do_stance_set( CHAR_DATA *ch, char *argument )
{ 
    char arg1 [MIL]; 
    char arg2 [MIL]; 
    char arg3 [MIL];
    CHAR_DATA *victim; 
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "  Syntax:\n\r",ch);
	send_to_char( "  set stance <name> <stance> <value>\n\r", ch);
	send_to_char( "  (use the name of the stance.)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    if ( value < 0 || value > 200 )
    {
	send_to_char( "Value range is 0 to 200.\n\r", ch );
	return;
    }

    if ( !str_cmp(arg2, "serpent") ) { victim->stance[1] = value; return; }
    else if (!str_cmp(arg2, "crane") ) { victim->stance[2] = value; return; }
    else if (!str_cmp(arg2, "crab") ) { victim->stance[3] = value; return; }
    else if (!str_cmp(arg2, "mongoose")) { victim->stance[4] = value; return; }
    else if (!str_cmp(arg2, "bull") ) { victim->stance[5] = value; return; }
    else if (!str_cmp(arg2, "mantis") ) { victim->stance[6] = value; return; }
    else if (!str_cmp(arg2, "dragon") ) { victim->stance[7] = value; return; }
    else if (!str_cmp(arg2, "tiger") ) { victim->stance[8] = value; return; }
    else if (!str_cmp(arg2, "monkey") ) { victim->stance[9] = value; return; }
    else if (!str_cmp(arg2, "swallow") ) { victim->stance[10] = value; return; }
    else if (!str_cmp(arg2, "all") ) 	{ 
					victim->stance[1] = value;
					victim->stance[2] = value;
					victim->stance[3] = value;
					victim->stance[4] = value;
					victim->stance[5] = value;
					victim->stance[6] = value;
					victim->stance[7] = value;
					victim->stance[8] = value;
					victim->stance[9] = value;
					victim->stance[10] = value;
					return; 
					}

}

void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}



void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch); 
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    str int wis dex con sex class clasb level\n\r",	ch );
	send_to_char( "    race group platinum gold silver hp\n\r",	ch );
	send_to_char( "    mana move prac align train thirst\n\r",	ch );
	send_to_char( "    hunger drunk full quest tier security\n\r",		ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL 
    || (victim->level > ch->level && victim->level == MAX_LEVEL))
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /* clear zones for mobs */
    victim->zone = NULL;

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"Strength range is 3 to %d\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_cmp( arg2, "security" ) )	/* OLC */
    {
	if ( IS_NPC(ch) )
	{
		send_to_char( "Si, claro.\n\r", ch );
		return;
	}

        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

	if ( value > ch->pcdata->security || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "Valid security is 0-%d.\n\r",
		    ch->pcdata->security );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity range is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }


    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	if (IS_NPC(victim))
	{ send_to_char("Mobiles have no class.\n\r",ch); return; }

	class = class_lookup(arg3);
	if ( class == -1 )
	{
	    char buf[MSL];

        	strcpy( buf, "Possible classes are: " );
        	for ( class = 0; class < MAX_CLASS; class++ )
        	{
            	    if ( class > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[class].name );
        	}
            strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->class = class;
	return;
    }

    if ( !str_prefix( arg2, "tier" ) )
    {
	if ( value < 0 || value > 3 )
	{
	    send_to_char( "Tier range is 0 to 3.\n\r", ch );
	    return;
	}
	victim->pcdata->tier = value;
	if (!IS_NPC(victim))
	    victim->pcdata->tier = value;
	return;
    }

    if ( !str_prefix( arg2, "clasb" ) )
    {
	int clasb;

	if (IS_NPC(victim))
	{ send_to_char("Mobiles have no clasb.\n\r",ch); return; }

	clasb = class_lookup(arg3);
	if ( clasb == -1 )
	{
	    char buf[MSL];

        	strcpy( buf, "Possible classes are: " );
        	for ( clasb = 0; clasb < MAX_CLASS; clasb++ )
        	{
            	    if ( clasb > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[clasb].name );
        	}
            strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->clasb = clasb;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > ch->level )
	{
	    sprintf(buf, "Level range is 0 to %d.\n\r", ch->level );
	    send_to_char(buf,ch);
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !str_prefix( arg2, "platinum" ) )
    {
	victim->platinum = value;
	return;
    }

    if ( !str_prefix( arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_prefix(arg2, "silver" ) )
    {
	victim->silver = value;
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 50000 )
	{
	    send_to_char( "Hp range is -10 to 50,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 50000 )
	{
	    send_to_char( "Mana range is 0 to 50,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 50000 )
	{
	    send_to_char( "Move range is 0 to 50,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 250 )
	{
	    send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 50 )
	{
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	if ( victim->pet != NULL )
	    victim->pet->alignment = victim->alignment;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_prefix( arg2, "hunger" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
 
        if ( value < -1 || value > 100 )
        {
            send_to_char( "Full range is -1 to 100.\n\r", ch );
            return;
        }
 
        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

    if ( !str_prefix( arg2, "quest" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "NPC's don't need quest points.\n\r", ch );
	    return;
	}

	victim->aqps = value;
	return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }
   
    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }


    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}

void do_rename( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    char buf [MSL];
    char name [MIL];
    char oldname [MIL];
    char strsave [MSL];
    CHAR_DATA *victim;
    int clan;
    int clead;

    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  rename <char> <new name>\n\r",ch);
	return;
    }
    
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if (victim->level >= ch->level)
    {
	send_to_char( "Nope, ain't gonna do it.\n\r", ch );
	return;
    }

    if ( check_char_exist( arg2 ) )
    {
	send_to_char( "That name is already in use, or is illegal.\n\r", ch );
	return;
    }
    sprintf(name, "%s", str_dup(capitalize(arg2)));
    sprintf(oldname, "%s", str_dup(victim->name));
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );
    clan = 0;
    clead = 0;
    if (is_clead(victim))
    {
	clead = victim->clead;
	update_clanlist(victim, victim->clead, FALSE, TRUE);
    }
    if (is_clan(victim))
    {
	clan = victim->clan;
	update_clanlist(victim, victim->clan, FALSE, FALSE);
    }
    victim->name = str_dup(name);
    victim->clead = clead;
    victim->clan = clan;
    if (clan != 0)
	update_clanlist(victim, victim->clan, TRUE, FALSE);
    if (clead != 0)
	update_clanlist(victim, victim->clan, TRUE, TRUE);
    do_save(victim, "");
    sprintf(buf, "Your name has been changed to '%s'.\n\r", name );
    send_to_char( buf, victim );
    send_to_char( "Ok.\n\r", ch);
    unlink(strsave);
    return;
}


void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MIL];
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MSL];
    char buf [MSL];
    char buf2 [MIL];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int cnt;
	unsigned plc;

    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long title who spec cname\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

	if ((victim->level >= ch->level) && (ch != victim))
	{
	    send_to_char("That will not be done.\n\r", ch);
	    return;
	}
    	
    	if ( !str_prefix( arg2, "who" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }
	    if ((ch->level < CREATOR)
	    && (victim->level < HERO)
	    && (victim->class < MCLT_1))
	    {
		send_to_char( "Not on 1st tier mortals.\n\r", ch);
		return;
	    }
	    buf[0] = '\0';
	    buf2[0] = '\0';
    	    victim->pcdata->who_descr = str_dup( "" );
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
		if (cnt >= 12)
		{
		    plc = strlen(arg3);
		}
	    }
	    sprintf(buf2, "%s{0", buf);
	    sprintf(buf, "%s", buf2);
	    while (cnt < 12)
	    {
		sprintf(buf2, "%s ", buf);
		sprintf(buf, "%s", buf2);
		cnt++;
	    }
    	    victim->pcdata->who_descr = str_dup(buf);
	    buf[0] = '\0';
	    buf2[0] = '\0';
    	    return;
    	}

	if (arg3[0] == '\0')
	{
	    do_string(ch,"");
	    return;
	}

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

        if ( !str_prefix( arg2, "cname" ) )
        {
            free_string( victim->pcdata->cname );
            victim->pcdata->cname = str_dup( arg3 );
            return;
        }

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
    	}
    }
    
    if (arg3[0] == '\0')
    {
	do_string(ch,"");
	return;
    }
    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}
    	if (obj->item_type == ITEM_EXIT)
	{
	    send_to_char("You cannot modify exit objects.\n\r",ch);
	    return;
	}
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }
    
    	
    /* echo bad use message */
    do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];
    OBJ_DATA *obj;
    int value;
    int clan;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    level weight cost timer clan\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }
    if (obj->item_type == ITEM_EXIT)
    {
	send_to_char( "You cannot modify exit objects.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "clan" ) )
    {
	if (!str_prefix(arg3,"none"))
	{
	    obj->clan = 0;
	    return;
	}
	if ((clan = clan_lookup(arg3)) == 0)
	{
	    send_to_char("No such clan exists.\n\r",ch);
	    return;
	}
	obj->clan = clan;
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	if (obj->item_type == ITEM_WEAPON)
	{
	    obj->value[0] = UMIN(MAX_WEAPON,value);
	    obj->value[0] = UMAX(0,obj->value[0]);
	    return;
	}
	if ((obj->item_type == ITEM_WAND)
	||  (obj->item_type == ITEM_STAFF)
	||  (obj->item_type == ITEM_POTION)
	||  (obj->item_type == ITEM_SCROLL)
	||  (obj->item_type == ITEM_PILL))
	{
	    obj->value[0] = UMIN(MAX_LEVEL,value);
	    obj->value[0] = UMAX(0,obj->value[0]);
	    return;
	}
	obj->value[0] = value;
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	if ((obj->item_type == ITEM_FOUNTAIN)
	||  (obj->item_type == ITEM_DRINK_CON))
	{
	    obj->value[2] = UMIN(MAX_LIQUID,value);
	    obj->value[2] = UMAX(0,obj->value[2]);
	    return;
	}
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	if (obj->item_type == ITEM_WEAPON)
	{
	    obj->value[3] = UMIN(MAX_DAMAGE_MESSAGE,value);
	    obj->value[3] = UMAX(0,obj->value[3]);
	    return;
	}
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	send_to_char( "Use the flag command instead.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	send_to_char( "Use the flag command instead.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( (get_trust( ch ) < CREATOR && (obj->pIndexData->level - 5) > value)
	&& !IS_SET(ch->act, PLR_KEY) )
	{
	    send_to_char("You may not lower an item more than 5 levels!\n\r",ch);
	    return;
	}
        if ( (get_trust( ch ) == CREATOR && (obj->pIndexData->level - 10) > value)
	&& !IS_SET(ch->act, PLR_KEY) )
        { 
            send_to_char("You may not lower an item more than 10 levels!\n\r",ch);
            return; 
        }
	obj->level = UMIN(MAX_LEVEL,value);
	obj->level = UMAX(0,obj->level);
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    char arg3 [MIL];
    ROOM_INDEX_DATA *location;
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private(ch,location) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	send_to_char( "Use the flag command instead.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}

/* Written by Stimpy, ported to rom2.4 by Silverhand 3/12
 *
 *      Added the other COMM_ stuff that wasn't defined before 4/16 -Silverhand
 */
void do_sockets( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *vch;
    DESCRIPTOR_DATA *d;
    char            buf  [ MAX_STRING_LENGTH ];
    char            buf2 [ MAX_STRING_LENGTH ];
    int             count;
    char *          st;
    char            idle[10];


    count       = 0;
    buf[0]      = '\0';
    buf2[0]     = '\0';

    strcat( buf2, "\n\r{y[{DN{cum   {DC{connected_State  {DI{cdle{y ] {DP{clayer {DN{came  {DH{cost{x\n\r" );
    strcat( buf2,"{y--------------------------------------------------------------------------{x\n\r");
    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->character && can_see( ch, d->character ) )
        {
            /* NB: You may need to edit the CON_ values */
            switch( d->connected )
            {
            case CON_PLAYING:              st = "    PLAYING    ";      break;
            case CON_GET_NAME:             st = "   Get Name    ";      break;
            case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd ";      break;
            case CON_CONFIRM_NEW_NAME:     st = " Confirm Name  ";      break;
            case CON_GET_NEW_PASSWORD:     st = "Get New Passwd ";      break;
            case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd ";      break;
            case CON_GET_NEW_RACE:         st = "  Get New Race ";      break;
            case CON_GET_NEW_SEX:          st = "  Get New Sex  ";      break;
            case CON_GET_NEW_CLASS:        st = " Get New Class ";      break;
            case CON_GET_ALIGNMENT:        st = " Get New Align ";      break;
            case CON_DEFAULT_CHOICE:       st = " Choosing Cust ";      break;
            case CON_GEN_GROUPS:           st = " Customization ";      break;
            case CON_PICK_WEAPON:          st = " Picking Weapon";      break;
            case CON_READ_IMOTD:           st = " Reading IMOTD ";      break;
            case CON_BREAK_CONNECT:        st = "   LINKDEAD    ";      break;
            case CON_READ_MOTD:            st = "  Reading MOTD ";      break;
            default:                       st = "   !UNKNOWN!   ";      break;
            }
            count++;

            /* Format "login" value... */
            vch = d->original ? d->original : d->character;

            if ( vch->timer > 0 )
                sprintf( idle, "%-2d", vch->timer );
            else
                sprintf( idle, "  " );

            sprintf( buf, "{y[{D%3d {c%s {D%7s {y] {c%-12s {D%-32.32s{x\n\r",
                    d->descriptor,
                    st,
                    idle,
                    ( d->original ) ? d->original->name
                    : ( d->character )  ? d->character->name
                    : "(None!)",
                    d->host );

            strcat( buf2, buf );

        }
    }

    sprintf( buf, "\n\r{y[{D%d {DP{clayer%s {DC{connected{y]{x\n\r", count, count == 1 ? "" : "s" );
    strcat( buf2, buf );
    send_to_char( buf2, ch );
    return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL];
    char arg2[MIL];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    if (!str_cmp(arg2,"reroll"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 2)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) 
	    &&	 vch->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
            &&   vch->level >= LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL 
	|| (victim->level >= ch->level && victim->level == MAX_LEVEL))
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

    	if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
        &&  room_is_private(ch,victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    	{
            send_to_char("That character is in a private room.\n\r",ch);
            return;
        }

	if (( get_trust( victim ) >= get_trust( ch )
	&& !IS_SET(ch->act, PLR_KEY)
	&& (victim->level != MAX_LEVEL))
	|| (!IS_NPC(victim)
	&& (IS_SET(victim->act, PLR_KEY))
	&& (ch->level != MAX_LEVEL)))
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MSL];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) 
    /* take the default path */

      if ( ch->invis_level)
      {
          ch->invis_level = 0;
          if (!IS_TRUSTED(ch,IMPLEMENTOR))
          {
	     act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
          }
          else 
          {
             act( "$n appears in a blinding {z{Wflash{x!", ch, NULL, NULL, TO_ROOM );
	  }
	  send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
      else
      {
          if (!IS_TRUSTED(ch,IMPLEMENTOR)) 
          {
	     act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  }
          else 
          { 
             act( "A {Wblinding white light{x envelops $n, then {z{Dvanishes{x.", ch, NULL, NULL, TO_ROOM );
          }
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
          ch->invis_level = get_trust(ch);
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
	  if (!IS_TRUSTED(ch,IMPLEMENTOR))
	  {
             act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  }
	  else
	  {
             act( "A {Wblinding white light{x envelops $n, then {z{Dvanishes{x.", ch, NULL, NULL, TO_ROOM );
	  }
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
          ch->reply = NULL;
          ch->invis_level = level;
      }
    }

    return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MSL];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You are no longer cloaked.\n\r", ch );
      }
      else
      {
          ch->incog_level = get_trust(ch);
          ch->ghost_level = 0;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          ch->ghost_level = 0;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    }
 
    return;
}

void do_ghost( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MSL];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->ghost_level)
      {
          ch->ghost_level = 0;
          act( "$n steps out from the mist.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You step out from the mist.\n\r", ch );
      }
      else
      {
          ch->ghost_level = get_trust(ch);
          ch->incog_level = 0;
          act( "$n vanishes into a mist.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You vanish into a mist.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Ghost level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->ghost_level = level;
          ch->incog_level = 0;
          act( "$n vanishes into a mist.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You vanish into a mist.\n\r", ch );
      }
    }
 
    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MIL];

    if (argument[0] == '\0')
    {
	if (ch->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->prefix);
	ch->prefix = str_dup("");
	return;
    }

    if (ch->prefix[0] != '\0')
    {
	sprintf(buf,"Prefix changed to %s.\r\n",argument);
	free_string(ch->prefix);
    }
    else
    {
	sprintf(buf,"Prefix set to %s.\r\n",argument);
    }

    ch->prefix = str_dup(argument);
}

void do_mquest (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Make a quest item of what?\n\r", ch );
        return;
    }
    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (IS_OBJ_STAT(obj,ITEM_QUEST))
    {
	REMOVE_BIT(obj->extra_flags,ITEM_QUEST);
	act("$p is no longer a quest item.",ch,obj,NULL,TO_CHAR);
    }
    else
    {
	SET_BIT(obj->extra_flags,ITEM_QUEST);
	act("$p is now a quest item.",ch,obj,NULL,TO_CHAR);
    }

    return;
}

void do_mpoint (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Make a questpoint item of what?\n\r", ch );
        return;
    }
    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (IS_OBJ_STAT(obj,ITEM_QUESTPOINT))
    {
	REMOVE_BIT(obj->extra_flags,ITEM_QUESTPOINT);
	act("$p is no longer a questpoint item.",ch,obj,NULL,TO_CHAR);
    }
    else
    {
	SET_BIT(obj->extra_flags,ITEM_QUESTPOINT);
	act("$p is now a questpoint item.",ch,obj,NULL,TO_CHAR);
    }

    return;
}

void do_gset (CHAR_DATA *ch, char *argument)
{
    if ( IS_NPC(ch) )
	return;

    if ( ( argument[0] == '\0' ) || !is_number( argument ) )
    {
        send_to_char( "Goto point cleared.\n\r", ch );
	ch->pcdata->recall = 0;
        return;
    }

    ch->pcdata->recall = atoi(argument);

    send_to_char( "Ok.\n\r", ch );

    return;
}

void do_wizslap( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;
    AFFECT_DATA af;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "WizSlap whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim->level >= ch->level )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
    pRoomIndex = get_random_room(victim);

    act( "$n slaps you, sending you reeling through time and space!", ch, NULL, victim, TO_VICT);
    act( "$n slaps $N, sending $M reeling through time and space!", ch, NULL, victim, TO_NOTVICT );
    act( "You send $N reeling through time and space!", ch, NULL, victim, TO_CHAR );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n crashes to the ground!", victim, NULL, NULL, TO_ROOM );
    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("weaken");
    af.level     = 105;
    af.duration  = 5;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (105 / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    do_look( victim, "auto" );
    return;
}

void do_pack ( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;
    OBJ_DATA *pack;
    OBJ_DATA *obj;
    int i;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Send a survival pack to whom?\n\r", ch );
	return;
    }

    if (strcmp(ch->name, argument))
    {
	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
    } else {
	victim = ch;
    }

    if ( (victim->level >= 10) && (ch->level < DEMI) )
    {
	send_to_char("They don't need one at thier level.\n\r", ch);
	return;
    }

    if (!can_pack(victim) )
    {
	send_to_char("They already have a survival pack.\n\r",ch);
	return;
    }

    if (victim->spirit)
    {
	send_to_char( "Spirits can't carry anything.\n\r", ch);
	return;
    }

    pack = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_PACK), 0 );
    pack->level = 5;

    for (i = 0; i < 7; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_A), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_B), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_C), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_D), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_E), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_F), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_G), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_H), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_I), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_J), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_K), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_L), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_M), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_N), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_O), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_P), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_Q), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_R), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_S), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_T), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_U), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_V), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_W), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_X), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );

    obj_to_char( pack, victim );

    send_to_char("Ok.\n\r", ch);
    act( "$p suddenly appears in your inventory.", ch, pack, victim, TO_VICT);
    return;
}


bool can_pack(CHAR_DATA *ch)
{
    OBJ_DATA *object;
    bool found;
 
    if ( ch->desc == NULL )
        return TRUE;
 
    if ( ch->level > HERO )
	return TRUE;

    /*
     * search the list of objects.
     */
    found = TRUE;
    for ( object = ch->carrying; object != NULL; object = object->next_content )
    {
    	if (object->pIndexData->vnum == OBJ_VNUM_SURVIVAL_PACK)
	    found = FALSE;
    }
    if (found)
	return TRUE;
 
    return FALSE;
}

void do_dupe(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MIL],arg2[MSL];
    char buf[MSL];
    int pos;
    bool found = FALSE;

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument,arg);
    one_argument(argument,arg2);
    
    if (arg[0] == '\0')
    {
	send_to_char("Dupe whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	if (victim->pcdata->dupes[0] == NULL)
	{
	    send_to_char("They have no dupes set.\n\r",ch);
	    return;
	}
	send_to_char("They currently have the following dupes:\n\r",ch);

	for (pos = 0; pos < MAX_DUPES; pos++)
	{
	    if (victim->pcdata->dupes[pos] == NULL)
		break;

	    sprintf(buf,"    %s\n\r",victim->pcdata->dupes[pos]);
	    send_to_char(buf,ch);
	}
	return;
    }

    for (pos = 0; pos < MAX_DUPES; pos++)
    {
	if (victim->pcdata->dupes[pos] == NULL)
	    break;

	if (!str_cmp(arg2,victim->pcdata->dupes[pos]))
	{
	    found = TRUE;
	}
    }

    if (found)
    {
	found = FALSE;
	for (pos = 0; pos < MAX_DUPES; pos++)
	{
	    if (victim->pcdata->dupes[pos] == NULL)
		break;

	    if (found)
	    {
		victim->pcdata->dupes[pos-1]		= victim->pcdata->dupes[pos];
		victim->pcdata->dupes[pos]		= NULL;
		continue;
	    }

	    if(!strcmp(arg2,victim->pcdata->dupes[pos]))
	    {
		send_to_char("Dupe removed.\n\r",ch);
		free_string(victim->pcdata->dupes[pos]);
		victim->pcdata->dupes[pos] = NULL;
		found = TRUE;
	    }
	}
	return;
    }

    for (pos = 0; pos < MAX_DUPES; pos++)
    {
	if (victim->pcdata->dupes[pos] == NULL)
	    break;
     }

     if (pos >= MAX_DUPES)
     {
	send_to_char("Sorry, they've reached the limit for dupes.\n\r",ch);
	return;
     }
  
     /* make a new dupe */
     victim->pcdata->dupes[pos]		= str_dup(arg2);
     sprintf(buf,"%s now has the dupe %s set.\n\r",victim->name,arg2);
     send_to_char(buf,ch);
}

#define CH(descriptor)  ((descriptor)->original ? \
	      (descriptor)->original : (descriptor)->character)
#define COPYOVER_FILE "copyover.data"
#define EXE_FILE      "../src/ROT2"


/*  Copyover - Original idea: Fusion of MUD++
 *  Adapted to Diku by Erwin S. Andreasen, <erwin@pip.dknet.dk>
 *  http://pip.dknet.dk/~pip1773
 *  Changed into a ROM patch after seeing the 100th request for it :)
 *****************************************************************************
 *  Adapted to RoT by Synwulfe (synwulfe@alltel.net)
 *  http://saturn.planetmud.com/~synwulfe/
 *  Signal handling and crash recovery tied into copyover
 */
void do_copyover (CHAR_DATA *ch, char * argument)
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [100], buf2[100];
	extern int port,control; /* db.c */
									
	fp = fopen (COPYOVER_FILE, "w");
										
	if (!fp)
	{
		send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
		logfprint ("Could not write to copyover file: %s", COPYOVER_FILE);
		perror ("do_copyover:fopen");
		return;
	}
										
	/* Consider changing all saved areas here, if you use OLC */
	/* do_asave (NULL, ""); - autosave changed areas */
										
	sprintf (buf, "\n\r *** Copyover by %s - please remain seated! ***\n\r", ch->name);
										
	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next)
	{
		CHAR_DATA * och = CH (d);
		d_next = d->next; /* We delete from the list , so need to save this */
										
	if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
	{
	  write_to_descriptor(d->descriptor, "\n\rSorry, we are rebooting, come back in a few minutes\n\r",0);
          close_socket (d); /* throw'em out */
	}
	else
	{
		fprintf (fp, "%d %s %s\n", d->descriptor, och->name,d->host);
	#if 0                /* This is not necessary for ROM */
	if (och->level == 1)
	{
	    write_to_descriptor (d->descriptor, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
	    advance_level (och);
	    och->level++; /* Advance_level doesn't do that */
	}
	#endif           
	    save_char_obj (och);
	    write_to_descriptor (d->descriptor, buf, 0);
	}
	}
										
	fprintf (fp, "-1\n");
	fclose (fp);

	/* Close reserve and other always-open files and release other resources */
	fclose (fpReserve);

	/* exec - descriptors are inherited */
	sprintf (buf, "%d", port);
	sprintf (buf2, "%d", control);

	execl (EXE_FILE, "ROT2", buf, "copyover", buf2, (char *)NULL);

	/* Failed - sucessful exec will not return */
	perror ("do_copyover: execl");
	send_to_char ("Copyover FAILED!\n\r",ch);
										
	/* Here you might want to reopen fpReserve */
	fpReserve = fopen (NULL_FILE, "r");
}
extern int port,control;

/* Recover from a copyover - load players */
void copyover_recover ()
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char name [100];
	char host[MSL];
	int desc;
	bool fOld;
	
	logfprint ("Reboot recovery initiated");
	
	fp = fopen (COPYOVER_FILE, "r");
	
	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror ("copyover_recover:fopen");
		logfprint ("Copyover file not found. Exitting.\n\r");
		exit (1);
	}
	
	unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading	*/
	
	for (;;)
	{
		fscanf (fp, "%d %s %s\n", &desc, name, host);
		if (desc == -1)
			break;
		
		/* Change this to write_to_descriptor(); if not using MCCP compression - Synwulfe */
		/* Write something, */		
		if (!write_to_descriptor(desc, "\n\r\033[1;33m==[RoT2]==\033[0m\n\r",0))
		{
#if defined(WIN32)
			closesocket (desc); 
#else
			close (desc); 
#endif
			continue;
		}
		
		d = new_descriptor();
		d->descriptor = desc;
		d->host = str_dup (host);
		d->next = descriptor_list;
		descriptor_list = d;
		d->connected = CON_COPYOVER_RECOVER; 
		/* -15, so close_socket frees the char */
		/* Now, find the pfile */
		fOld = load_char_obj (d, name);
		if (!fOld) /* Player file not found?! */
		{
			write_to_descriptor(desc, 
				"\n\rSomehow, your character was lost in the reboot. Sorry.\n\r", 0);
			close_socket (d);			
		}
		else 
		{
			write_to_descriptor(desc, "\n\r\033[1;32m==[RoT2]==\033[0m\n\r",0);
			/* Just In Case */
			if (!d->character->in_room)
				d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);
			d->character->pcdata->socket = str_dup( d->host );

			/* Insert in the char_list */
			d->character->next = char_list;
			char_list = d->character;
			char_to_room (d->character, d->character->in_room);
			do_look (d->character, "auto");
			act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
			d->connected = CON_PLAYING;
			reset_char(d->character); 
			if (d->character->pet != NULL)
			{
				char_to_room(d->character->pet,d->character->in_room);
				act("$n materializes!.",d->character->pet,NULL,NULL,TO_ROOM);
			}
		}
	}
	fclose (fp);
}

