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



void do_pload( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA d;
    bool isChar = FALSE;
    char name[MIL];

    if (argument[0] == '\0')
    {
	send_to_char("Load who?\n\r", ch);
	return;
    }

    argument[0] = UPPER(argument[0]);
    argument = one_argument(argument, name);

    if ( get_char_world( ch, name ) != NULL )
    {
	send_to_char( "That person is allready connected!\n\r", ch );
	return;
    }

    isChar = load_char_obj(&d, name); /* char pfile exists? */

    if (!isChar)
    {
	send_to_char("Load Who? Are you sure? I cant seem to find them.\n\r", ch);
	return;
    }

    d.character->desc     = NULL;
    d.character->next     = char_list;
    char_list             = d.character;
    d.connected           = CON_PLAYING;
    reset_char(d.character);

    if ( d.character->in_room != NULL )
    {
	char_to_room( d.character, ch->in_room); /* put in room imm is in */
    }
	send_to_char( "Ok.\n\r", ch );
}

void do_punload( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char who[MIL];

    argument = one_argument(argument, who);

    if ( ( victim = get_char_world( ch, who ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (victim->desc != NULL)
    {
	send_to_char("I dont think that would be a good idea...\n\r", ch);
	return;
    }

    if (victim->was_in_room != NULL) /* return player and pet to orig room */
    {
	char_to_room(victim, victim->was_in_room);
	if (victim->pet != NULL)
	char_to_room(victim->pet, victim->was_in_room);
    }

    save_char_obj(victim);
    do_quit(victim,"");
    send_to_char( "Ok.\n\r", ch );

}

void do_award(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim;
   char buf   [MSL];
   char btype [MSL];
   char smesg [MSL];
   char arg1  [MSL];
   char arg2  [MSL];
   char arg3  [MSL];
   DESCRIPTOR_DATA *d;
   int value;
   

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg3 ) )
   {
     send_to_char( "Syntax: award <char> <type> <bonus>.\n\r", ch);
     send_to_char( "  <char> :  all, <name>             \n\r", ch);
     send_to_char( "  <type> :  aquest exp hit mana move \n\r", ch);
     send_to_char( "         :  platinum gold silver    \n\r", ch);
     send_to_char( "         :  train prac points quest \n\r", ch);
     send_to_char( "  <bonus>:  numeric value           \n\r", ch);
     return;
   }

    if ( !str_cmp( arg1, "all" ) )   
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( 
				d->connected == CON_PLAYING
            &&  d->character != ch
            &&  d->character->in_room != NULL
            &&  !IS_SET(d->character->comm,COMM_NOOOC)
            &&  can_see( ch, d->character )
			   )
            {
                char buf[MSL];
                sprintf( buf, "%s %s %s", d->character->name, arg2, arg3 );
                do_award( ch, buf );
            }
        }
        return;
    }

   /*  Restrictions on awarding  */

   if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
   {
     send_to_char( "That player is not here.\n\r", ch);
     return;
   }

   if ( IS_NPC( victim ) )
   {
     send_to_char( "Not on NPC's!\n\r", ch);
     return;
   }

   value = atoi( arg3 );

   if ( value == 0 )
   {
     send_to_char( "Value cannot be 0.\n\r", ch );
     return;
   }
/*
 *
 *   if ( !str_cmp( arg2, "COMMAND" ) )
 *   {
 *
 *	   if ( value < LOWERVALUE || value > HIGHVALUE )
 *	   {
 *	     send_to_char( "Award range is -LOWVALUE to HIGHVALUE.\n\r", ch );
 *	     return;
 *	   }
 * 		  sprintf( btype, "WORDS" );         
 *        type how you want bonus to show im message
 *        sprintf( smesg, "showmessage");    
 *        use SHOWMESSAGE to activate bonus, NOSHOW to deactivate temporarily
 *        victim->STAT += value;          
 *        replace STAT with charecter variable to bonus
 *   } 
 *   
 */
   if ( !str_cmp( arg2, "aquest" ) )
   {

           if ( value < -100 || value > 1000 )
           {
             send_to_char( "Award range is -100 to 1000.\n\r", ch );
             return;
           }
           sprintf( btype, "Auto Quest Points" );
           sprintf( smesg, "showmessage");
           victim->questpoints += value;
   }


   else if ( !str_cmp( arg2, "quest" ) )
   {

	   if ( value < -100 || value > 1000 )
	   {
	     send_to_char( "Award range is -100 to 1000.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Quest Points" );
	   sprintf( smesg, "showmessage");
	   victim->aqps += value;
   }

   else if ( !str_cmp( arg2, "exp" ) )
   {
	   if ( value < -50000 || value > 50000 )
	   {
	     send_to_char( "Award range is -50000 to 50000.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Exp Points" );
	   sprintf( smesg, "noshow");         
	   victim->exp += value;             
   }

   else if ( !str_cmp( arg2, "hit" ) )
   {	   
	   if ( value < -5000 || value > 5000 )
	   {
	     send_to_char( "Award range is -5000 to 5000.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Health Points" );
	   sprintf( smesg, "showmessage");
	   victim->max_hit += value;
   }

   else if ( !str_cmp( arg2, "mana" ) )
   {
	   if ( value < -5000 || value > 5000 )
	   {
	     send_to_char( "Award range is -5000 to 5000.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Mana Points" );
	   sprintf( smesg, "showmessage");
	   victim->max_mana += value;
   }

   else if ( !str_cmp( arg2, "move" ) )
   {
	   if ( value < -5000 || value > 5000 )
	   {
	     send_to_char( "Award range is -5000 to 5000.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Movement Points" );
	   sprintf( smesg, "showmessage");
	   victim->max_move += value;
   }

   else if ( !str_cmp( arg2, "train" ) )
   {
	   if ( value < -100 || value > 100 )
	   {
	     send_to_char( "Award range is -100 to 100.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Training Sessions" );
	   sprintf( smesg, "showmessage");
	   victim->train += value;
   }

   else if ( !str_cmp( arg2, "prac" ) )
   {
	   if ( value < -100 || value > 100 )
	   {
	     send_to_char( "Award range is -100 to 100.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Practice Sessions" );
	   sprintf( smesg, "showmessage");
	   victim->practice += value;
   }

   else if ( !str_cmp( arg2, "points" ) )
   {
	   if ( value < -50 || value > 50 )
	   {
	     send_to_char( "Award range is -50 to 50.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Creation Points" );
	   sprintf( smesg, "showmessage");
	   victim->pcdata->points += value;
   }

   else if ( !str_cmp( arg2, "silver" ) )
   {
	   if ( value < -5000 || value > 5000 )
	   {
	     send_to_char( "Award range is -5000 to 5000.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Silver Coin" );
	   sprintf( smesg, "showmessage");
	   victim->silver += value;
   }

   else if ( !str_cmp( arg2, "gold" ) )
   {
	   if ( value < -5000 || value > 5000 )
	   {
	     send_to_char( "Award range is -5000 to 5000.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Gold Coin" );
	   sprintf( smesg, "showmessage");
	   victim->gold += value;
   }

   else if ( !str_cmp( arg2, "platinum" ) )
   {
	   if ( value < -5000 || value > 5000 )
	   {
	     send_to_char( "Award range is -5000 to 5000.\n\r", ch );
	     return;
	   }
	   sprintf( btype, "Platinum Coin" );
	   sprintf( smesg, "showmessage");
	   victim->platinum += value;
   }

   else
   {
	   sprintf( buf, "Sorry but %s awards is not an option.\n\r", arg2 );
	   send_to_char( buf, ch );
	   
	   sprintf( log_buf, "Ya damn coders..  fix the award command." );
	   wiznet(log_buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	   return;
   }

   if ( !str_cmp( smesg, "showmessage" ) )
   {
	   sprintf( buf, "{wYou have awarded %s {Y%d{w %s!\n\r", victim->name, value, btype);
	   send_to_char(buf, ch);

	   if ( value >0 )
	   {
		   sprintf( buf, "{wYou have been awarded {Y%d{w %s by %s!{x\n\r", value, btype, ch->name);
		   send_to_char( buf, victim );
		   sprintf( log_buf, "{R$N awards %d %s to %s{x", value, btype, victim->name);
		   wiznet( log_buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,get_trust(ch));
	   }
	   else
	   {
		   sprintf( buf, "{wYou have been penalized {Y%d{w %s by %s!{x\n\r", value, btype, ch->name);
	       send_to_char( buf, victim );
		   sprintf( log_buf, "$N penalizes %s %d %s.", victim->name, value, btype );
		   wiznet(log_buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	   }
	   return;
   }
   else
   {
	   sprintf( buf, "Sorry, %s awards are not implimented yet.\n\r", btype);
	   send_to_char( buf, ch);
	   sprintf( log_buf, "Ya damned coders..  fix the award command." );
	   wiznet(log_buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
   }
   return;
}

void do_nuke(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char buf[MSL];
   char arg1 [MAX_INPUT_LENGTH];
   char   strsave[MAX_STRING_LENGTH];

   argument = one_argument( argument, arg1 );
   victim = get_char_world(ch, arg1);

   if ( arg1[0] == '\0' )
   {
        send_to_char("Syntax: nuke <player name>\n\r",ch);
        return;
   }
   if (victim == NULL)
   {
      send_to_char("They must be playing.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's\n\r", ch);
      return;
   }
   if (IS_IMMORTAL(victim))/* nukeing immortals is the job of the MUDs owner */
   {
   if ( !str_cmp(ch->name, "Valar"))/* change the name here from Virus to whom ever the owner of the MUD is. */
   {
     send_to_char("Authorization granted.{/", ch);
     sprintf(buf, "Thank you %s, for using the {GU B Glass{x ver 1.0 nukeing system.\n\r", ch->name);
     send_to_char( buf, ch );
     send_to_char("{YYour character has been deleted.{x\n\r", victim);
     sprintf( strsave, "%s%s", PLAYER_DIR, capitalize(victim->name ) );
     unlink(strsave);
     sprintf(buf, "{R%s has been fired by %s{x.", victim->name, ch->name);
     do_echo(ch, buf);   
     sprintf( strsave, "%s%s", GOD_DIR, capitalize(victim->name) );
     unlink(strsave);
     sprintf(buf, "[*****] NUKE: %s has been fired.", victim->name);
     log_string(buf);
     d = victim->desc;
     extract_char( victim, TRUE );
     if ( d != NULL )
     close_socket( d );
     return;
   }
   else
   {
        send_to_char("Only IMPS has the Authorization to Nuke Immortal characters.",ch);
        sprintf(buf, "%s just attempted to NUKE you, but thats ok cuz %s doesn't have that authority.", ch->name,
        ch->sex == SEX_MALE ? "he" : ch->sex == SEX_FEMALE ? "she" : "it");
        send_to_char(buf, victim);
        return;
   }
   }
   if (!IS_NPC(victim))/* if an immortal has the command the CAN nuke playing characters */
   {
      sprintf(buf, "Thank you %s, for using the {GU B Glass{x ver 1.0 nukeing system.\n\r", ch->name);
      send_to_char( buf, ch );
      send_to_char("{YYour character has been deleted.{x\n\r", victim);
      sprintf( strsave, "%s%s", PLAYER_DIR, capitalize(victim->name ) );
      unlink(strsave);
      sprintf(buf, "%s pulls out a {rBIG RED{x NUKE button, and NUKES %s.", ch->name, victim->name);
      do_echo(ch, buf); 
      sprintf(buf, "[*****] NUKE: %s has been nuked by %s", victim->name, ch->name);
      log_string(buf);
      d = victim->desc;
      extract_char( victim, TRUE );
      if ( d != NULL )
      close_socket( d );
      return;
   }
}

void do_qspell(CHAR_DATA *ch, char *argument)
{
 CHAR_DATA *vch;
  char arg[MAX_INPUT_LENGTH];
 DESCRIPTOR_DATA *d;

  argument = one_argument(argument,arg);

  if (IS_NPC(ch))
    return;                                  
 if (arg[0] == '\0')
  { send_to_char("Spellup whom?\n\r",ch);
  return; }
 

    if ( !str_cmp( arg, "world" ) && (ch->level >= SQUIRE))
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character != ch
            &&   d->character->in_room != NULL
            &&   ch->level >= d->character->ghost_level
            &&   can_see( ch, d->character ) )
            {                                                      
     char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s", d->character->name, arg );
                do_qspell( ch, buf );
            }
}
       return;
    }
  if (str_cmp("room",arg))
  { if ((vch = get_char_world(ch,arg)) == NULL)
    { send_to_char("They aren't here.\n\r",ch);
    return; }}
  else
    vch = ch; 

  if (!str_cmp("room",arg))
  for (vch = ch->in_room->people;vch;vch = vch->next_in_room)
  { if (vch == ch)
    continue;
    if (IS_NPC(vch))
      continue;
    spell_shockshield(skill_lookup("shockshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_fireshield(skill_lookup("fireshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_iceshield(skill_lookup("iceshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_shield(skill_lookup("shield"),ch->level,ch,vch,TARGET_CHAR);
    spell_armor(skill_lookup("armor"),ch->level,ch,vch,TARGET_CHAR);
    spell_sanctuary(skill_lookup("sanctuary"),ch->level,ch,vch,TARGET_CHAR);
    spell_fly(skill_lookup("fly"),ch->level,ch,vch,TARGET_CHAR);
    spell_frenzy(skill_lookup("frenzy"),ch->level,ch,vch,TARGET_CHAR);
    spell_giant_strength(skill_lookup("giant strength"),ch->level,ch,vch,TARGET_CHAR);
    spell_bless(skill_lookup("bless"),ch->level,ch,vch,TARGET_CHAR);
    spell_haste(skill_lookup("haste"),ch->level,ch,vch,TARGET_CHAR);
  }
  else
  { spell_shockshield(skill_lookup("shockshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_fireshield(skill_lookup("fireshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_iceshield(skill_lookup("iceshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_shield(skill_lookup("shield"),ch->level,ch,vch,TARGET_CHAR);
    spell_armor(skill_lookup("armor"),ch->level,ch,vch,TARGET_CHAR);
    spell_sanctuary(skill_lookup("sanctuary"),ch->level,ch,vch,TARGET_CHAR);
    spell_fly(skill_lookup("fly"),ch->level,ch,vch,TARGET_CHAR);
    spell_frenzy(skill_lookup("frenzy"),ch->level,ch,vch,TARGET_CHAR);
    spell_giant_strength(skill_lookup("giant strength"),ch->level,ch,vch,TARGET_CHAR);
    spell_bless(skill_lookup("bless"),ch->level,ch,vch,TARGET_CHAR);
    spell_haste(skill_lookup("haste"),ch->level,ch,vch,TARGET_CHAR);
  }
  return;
}  

void do_confiscate( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Confiscate what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if(IS_NPC(victim))
    {
  	send_to_char("You can't use confiscate on NPCs.\n\r",ch);
  	return;
    }

    if( ( obj = get_obj_carry( victim , arg1 ) ) == NULL )
  	obj = get_obj_wear( victim, arg1 );

    if( obj == NULL )
    {
  	send_to_char("They aren't carrying/wearing that.\n\r",ch);
  	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    act("$n confiscates your $p.",ch,obj,victim,TO_VICT);
    act("$n confiscates $N's $p.",ch,obj,victim,TO_NOTVICT);
    act("You confiscate $N's $p.",ch,obj,victim,TO_CHAR);
}
