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
#include "recycle.h"

/* command procedures needed */
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_give		);

BANK_DATA *bank_list;

void save_banklist(void)
{
    BANK_DATA *pbank;
    FILE *fp;
    bool found = FALSE;
     
    fclose( fpReserve );
    if ( ( fp = fopen( BANK_FILE, "w" ) ) == NULL )
    {
        perror( BANK_FILE );
    }
     
    for (pbank = bank_list; pbank != NULL; pbank = pbank->next)
    {
        found = TRUE;
        fprintf(fp,"%d %s\n%d %d %d\n%ld\n",
            pbank->passwd,pbank->name,pbank->bank,pbank->type,pbank->amount,
	    pbank->date);
     }
       
     fclose(fp);
     fpReserve = fopen( NULL_FILE, "r" );
     if (!found)
        unlink(BANK_FILE);
}
 
void load_banklist(void)
{
    FILE *fp;
    BANK_DATA *bank_last;
     
    if ( ( fp = fopen( BANK_FILE, "r" ) ) == NULL )
        return;
 
    bank_last = NULL;
    for ( ; ; )
    {
        BANK_DATA *pbank;
        if ( feof(fp) )
        {
            fclose( fp );
            return;
        }
 
        pbank = new_bank();
 
        pbank->passwd = fread_number(fp);
        pbank->name = str_dup(fread_word(fp));
        pbank->bank = fread_number(fp);
        pbank->type = fread_number(fp);
        pbank->amount = fread_number(fp);
	pbank->date = fread_number(fp);
        fread_to_eol(fp);
 
        if (bank_list == NULL)
            bank_list = pbank;
        else
            bank_last->next = pbank;
        bank_last = pbank;
    }
}    

void do_deposit( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    char buf[MSL];
    int amount;
    int bank;
    bool found = FALSE;

    if ( !is_number( argument ) )
    {
        send_to_char( "Deposit how much platinum?\n\r", ch );
        return;
    }

    for (bank = 0; bank < MAX_BANKS; bank++)
    {
        if (ch->in_room->vnum == bank_table[bank].vnum)
        {
            found = TRUE;
            break;
        }
    }

    if (!found)
    {
        send_to_char( "Does this look like a bank to you?\n\r", ch);
        return;
    }

    if (ch->spirit)
    {
	send_to_char( "That's tough to do without flesh.\n\r", ch);
	return;
    }

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_BANKER) )
            break;
    }

    if ( mob == NULL )
    {
        send_to_char( "The banker appears to be on a break.\n\r", ch );
        return;
    }

    if ( ( time_info.hour < bank_table[bank].open )
    ||   ( time_info.hour > bank_table[bank].close ) )
    {
        sprintf(buf, "{aSorry, our hours are %d:00%s to %d:00%s.{x",
            bank_table[bank].open == 0 ? 12 :
            bank_table[bank].open < 13 ? bank_table[bank].open :
            bank_table[bank].open - 12,
            bank_table[bank].open < 13 ? "am" : "pm",
            bank_table[bank].close == 0 ? 12 :
            bank_table[bank].close < 13 ? bank_table[bank].close :
            bank_table[bank].close - 12,
            bank_table[bank].close < 13 ? "am" : "pm");
        do_say(mob, buf);
        return;
    }

    if (is_compromised(ch, bank))
        change_banklist(ch, FALSE, bank, 0, 0, ch->name);
    amount = atoi(argument);
    if ( amount < 1 )
    {
        return;
    }
    if ( ( amount % 10 ) != 0 )
    {
        send_to_char( "Deposits must be made in multiples of 10 platinum.\n\r", ch);
        return;
    }
    if ( amount > ch->platinum )
    {
        send_to_char( "You don't have that much platinum.\n\r", ch);
        return;
    }
    if ( ( ch->balance[bank] + amount ) > 30000 )
    {
        sprintf(buf,
            "%s tells you '{aSorry, but I can only hold 30,000 coins per player.{x'\n\r",
            mob->short_descr);
        send_to_char( buf, ch);
        return;
    }

    ch->balance[bank] += amount;
    ch->platinum -= amount;
    sprintf(buf, "You give %d platinum coins to %s.\n\r",
        amount, mob->short_descr);
    send_to_char(buf, ch);
    sprintf(buf,
        "%s tells you '{aThank you, your new balance is {W%ld{a platinum.{x'\n\r",
        mob->short_descr, ch->balance[bank]);
    send_to_char(buf, ch);
    return;
}

void do_withdraw( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    char buf[MSL];
    int amount;
    int bank;
    bool found = FALSE;

    if ( !is_number( argument ) )
    {
        send_to_char( "Withdraw how much platinum?\n\r", ch );
        return;
    }  

    for (bank = 0; bank < MAX_BANKS; bank++)
    {
        if (ch->in_room->vnum == bank_table[bank].vnum)
        {
            found = TRUE;
            break;
        }
    }    

    if (!found)
    {
        send_to_char( "Does this look like a bank to you?\n\r", ch);
        return;
    }  

    if (ch->spirit)
    {
	send_to_char( "That's tough to do without flesh.\n\r", ch);
	return;
    }

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_BANKER) )
            break;
    }  
 
    if ( mob == NULL )
    {
        send_to_char( "The banker appears to be on a break.\n\r", ch );
        return;
    }

    if ( ( time_info.hour < bank_table[bank].open )
    ||   ( time_info.hour > bank_table[bank].close ) )
    {
        sprintf(buf, "{aSorry, our hours are %d:00%s to %d:00%s.{x",
            bank_table[bank].open == 0 ? 12 :
            bank_table[bank].open < 13 ? bank_table[bank].open :
            bank_table[bank].open - 12,
            bank_table[bank].open < 13 ? "am" : "pm",
            bank_table[bank].close == 0 ? 12 :
            bank_table[bank].close < 13 ? bank_table[bank].close :
            bank_table[bank].close - 12,
            bank_table[bank].close < 13 ? "am" : "pm");
        do_say(mob, buf);
        return;
    }
 
    if (is_compromised(ch, bank))
	change_banklist(ch, FALSE, bank, 0, 0, ch->name);
    amount = atoi(argument);
    if ( amount < 1 )
    {
        return;
    }
    if ( ( ( amount % 10 ) != 0 ) && ( ch->balance[bank] >= 10 ) )
    {
        send_to_char( "Withdrawals must be made in multiples of 10 platinum.\n\r", ch);
        return;
    }
    if ( amount > ch->balance[bank] )
    {
        send_to_char( "You don't have that much platinum in this bank.\n\r", ch);
        return;
    }
    if ( amount > 10 && amount == ch->balance[bank] )
        amount -= 10;

    if ( ( ch->platinum + amount ) > 50000 )
    {
        send_to_char( "You can't carry that much platinum.\n\r", ch);
        return;
    }
    ch->platinum += amount;
    ch->balance[bank] -= amount;
    sprintf(buf, "%s gives you %d platinum coins.\n\r",
        mob->short_descr, amount);
    send_to_char(buf, ch);
//	amount *= .02;
    amount /= 50;
    ch->balance[bank] -= amount;
    ch->balance[bank] = UMAX(ch->balance[bank], 0);
    return;
}

void do_rob( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    CHAR_DATA *mob;
    OBJ_DATA *passbook;
    int amount;
    int bank;
    bool found = FALSE;

    if (IS_NPC(ch))
	return;

    for (bank = 0; bank < MAX_BANKS; bank++)
    {
        if (ch->in_room->vnum == bank_table[bank].vnum)
        {
            found = TRUE;
            break;
        }
    }
     
    if (!found)
    {
        send_to_char( "Does this look like a bank to you?\n\r", ch);
        return;
    }
     
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_BANKER) )
            break;
    }

    if ( mob == NULL )
    {
        send_to_char( "The banker appears to be on a break.\n\r", ch );
        return;
    }

    if ( ( time_info.hour < bank_table[bank].open )
    ||   ( time_info.hour > bank_table[bank].close ) )
    {
        sprintf(buf, "{aSorry, our hours are %d:00%s to %d:00%s.{x",
            bank_table[bank].open == 0 ? 12 :
            bank_table[bank].open < 13 ? bank_table[bank].open :
            bank_table[bank].open - 12,
            bank_table[bank].open < 13 ? "am" : "pm",
            bank_table[bank].close == 0 ? 12 :
            bank_table[bank].close < 13 ? bank_table[bank].close :
            bank_table[bank].close - 12,
            bank_table[bank].close < 13 ? "am" : "pm");
        do_say(mob, buf);
        return;
    }

    passbook = get_eq_char(ch,WEAR_HOLD);
    if (passbook == NULL || passbook->item_type != ITEM_PASSBOOK)
    {   
        send_to_char("You need to hold someones passbook to rob thier account.\n\r", ch);
        return;
    }
 
    if ( !is_number( argument ) )
    {
        send_to_char( "Rob how much platinum?\n\r", ch );
        return;
    }
    found = TRUE;
    amount = atoi(argument);
    if (amount < 1)
	found = FALSE;
    if (!is_othcomp(passbook->value[0],passbook->value[1],passbook->name))
	found = FALSE;
    if (amount > passbook->value[2])
	found = FALSE;
    if (bank != passbook->value[0])
	found = FALSE;
    act("$n gives $p to $N.", ch, passbook, mob, TO_ROOM);
    act("You give $p to $N.", ch, passbook, mob, TO_CHAR);
    if (found)
    {
	if (mob->platinum < amount)
	{
	    mob->platinum += amount;
	}
	sprintf(buf,"%d platinum %s", amount, ch->name);
	do_give(mob,buf);
	change_banklist(ch, FALSE, bank, amount, passbook->value[1], passbook->name);
    } else
    {
	change_banklist(ch, FALSE, passbook->value[0], 0, passbook->value[1], passbook->name);
	sprintf( buf, "{a%s is a {RTHIEF{a!{x", ch->name);
	REMOVE_BIT(mob->comm,COMM_NOSHOUT);
	do_yell( mob, buf );
    }
    extract_obj(passbook);
    return;
}

void change_banklist(CHAR_DATA *ch, bool add, int bank, int amount, int pwd, char *argument)
{
    char arg[MIL];
    BANK_DATA *prev;
    BANK_DATA *curr;
    DESCRIPTOR_DATA *d;
    OBJ_DATA *object;
    bool found = FALSE;
     
    one_argument( argument, arg );
    if ( !add && !str_cmp( capitalize( ch->name ), capitalize( arg ) ) )
    {
        prev = NULL;
        for ( curr = bank_list; curr != NULL; prev = curr, curr = curr->next )
        {
            if ( !str_cmp( capitalize( arg ), curr->name )
		&& (curr->type == 0)
		&& (curr->bank == bank))
            {
                if ( prev == NULL )
                    bank_list   = bank_list->next;
                else
                    prev->next = curr->next;
 
                free_bank(curr);
		for (d = descriptor_list; d != NULL; d = d->next)
		{
		    CHAR_DATA *victim;

		    if (d->connected != CON_PLAYING)
			continue;

		    victim = ( d->original != NULL ) ? d->original : d->character;

		    for ( object = victim->carrying; object != NULL; object = object->next_content )
		    {
			if (object->pIndexData->vnum == OBJ_VNUM_PASSBOOK)
			{
			    char oname[MIL];

			    one_argument(object->name, oname);
			    if (!str_cmp(oname, arg))
			    {
				found = TRUE;
				object->timer = 1;
			    }
			}
		    }
		}
                save_banklist();
            }
        }
    } else if ( !add ) 
    {
	CHAR_DATA *victim;
	char buf[MSL];

        prev = NULL;
        for ( curr = bank_list; curr != NULL; prev = curr, curr = curr->next )
        {
            if ( !str_cmp( capitalize( arg ), curr->name )
                && (curr->type == 0)
                && (curr->bank == bank))
            {
                if ( prev == NULL )
                    bank_list   = bank_list->next;
                else
                    prev->next = curr->next;
 
                free_bank(curr);
 
		if ((victim = get_char_mortal(ch, arg)) != NULL)
		{
		    if (amount > 0)
		    {
			sprintf(buf, "{RSomeone just stole %d platinum from one of your accounts!{x\n\r", amount);
			send_to_char(buf, victim);
			victim->balance[bank] -= amount;
			victim->balance[bank] = UMAX(victim->balance[bank],0);
		    }
		} else {
		    if (amount > 0)
		    {
			curr = new_bank();
			curr->name = str_dup( capitalize( arg ) );
			curr->amount = amount;
			curr->bank = bank;
			curr->type = 1;
			curr->passwd = pwd;
			curr->next = bank_list;
			curr->date = 0;
			bank_list = curr;
		    }
		}
		save_banklist();
	    }
	}
    } else
    {
        curr = new_bank();
        curr->name = str_dup( capitalize( arg ) );
        curr->bank = bank;
	curr->amount = amount;
	curr->type = 0;
	curr->passwd = pwd;
        curr->next = bank_list;
	curr->date = current_time;
        bank_list = curr;
        save_banklist();
    }
    return;
}

bool is_compromised(CHAR_DATA *ch, int bank)
{
    BANK_DATA *curr;
    bool found = FALSE;

    for ( curr = bank_list; curr != NULL; curr = curr->next )
    {
	if ( !str_cmp( capitalize( ch->name ), curr->name ) )
	{
	    if ( ( bank == curr->bank ) && ( curr->type == 0) )
		found = TRUE;
	}
    }
    return found;
}

void check_robbed( CHAR_DATA *ch )
{
    BANK_DATA *curr;
    BANK_DATA *prev;
    char buf[MSL];

    prev = NULL;
    for ( curr = bank_list; curr != NULL; prev = curr, curr = curr->next )
    {
	if ( !str_cmp( capitalize( ch->name ), curr->name ) )
	{
	    if (curr->type == 1)
	    {
		sprintf(buf, "{RSomeone stole %d platinum from one of your accounts!{x\n\r", curr->amount);
		send_to_char(buf, ch);
		ch->balance[curr->bank] -= curr->amount;
		ch->balance[curr->bank] = UMAX(ch->balance[curr->bank],0);
                if ( prev == NULL )
                    bank_list   = bank_list->next;
                else
                    prev->next = curr->next;
 
                free_bank(curr);
		save_banklist();
	    }
	}
    }
}

bool is_banklist( CHAR_DATA *ch )
{
    BANK_DATA *curr;
    bool found = FALSE; 
 
    for ( curr = bank_list; curr != NULL; curr = curr->next )
    {
        if ( !str_cmp( capitalize( ch->name ), curr->name ) )
        {
	    if (curr->type == 0)
		found = TRUE; 
        }
    }   
    return found;
}

bool is_othcomp(int bank, int pass, char *argument)
{
    char arg[MIL];
    BANK_DATA *curr;
    bool found = FALSE;

    one_argument( argument, arg );
    for ( curr = bank_list; curr != NULL; curr = curr->next )
    {
        if ( !str_cmp( capitalize( arg ), curr->name ) )
        {
            if ( ( bank == curr->bank ) && ( curr->type == 0) )
	    {
		if (pass == curr->passwd)
		    found = TRUE;
	    }
        }
    }    
    return found;
}

void expire_banks ( void )
{
    BANK_DATA *curr;
    BANK_DATA *prev;
    long diff;

    prev = NULL;
    for ( curr = bank_list; curr != NULL; prev = curr, curr = curr->next )
    {
	if (curr->date != 0)
	{
	    diff = (long)current_time - (long)curr->date;
	    if (diff > 172800)
	    {
		if ( prev == NULL )
		    bank_list   = bank_list->next;
		else
		    prev->next = curr->next;

		free_bank(curr);
		expire_banks();
		return;
	    }
	}
    }
    save_banklist();
    return;
}

