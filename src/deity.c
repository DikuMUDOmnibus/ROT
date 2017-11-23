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

#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "merc.h"
#include "interp.h"
#include "tables.h"
#include "lookup.h"
#include "magic.h"
#include "recycle.h"



DEITY_DATA *first_deity;
DEITY_DATA *last_deity;


/* Local Routines */


    void 	fread_deity args ( ( DEITY_DATA *deity, FILE *fp ) );
    bool	load_deity args ( ( char *deity_file ) );
    

DEITY_DATA *get_deity( char *name )
{
	DEITY_DATA *deity;
	deity = first_deity;
	while (deity)
	{
		if ( !str_cmp( name, deity->name ) )
			return deity;
		else
			deity = deity->next;
	}
	return NULL;
}

void do_mod_favor( CHAR_DATA *ch, int mod_type )
{
	if( IS_NPC( ch ) || !ch->pcdata->deity )
		return;
/*
	if( ( ch->alignment - ch->pcdata->deity->alignment > 750
	   || ch->alignment - ch->pcdata->deity->alignment < 750 ) 
	   && ch->pcdata->deity->alignment != 0 )
	{
		ch->pcdata->favor -= 5;
		ch->pcdata->favor = URANGE( -2500,
			ch->pcdata->favor, 2500 );
		return;
	}
*/
	switch( mod_type )
	{
	case 0:
		ch->pcdata->favor += ch->pcdata->deity->steal;
		break;
	case 1:
		ch->pcdata->favor += ch->pcdata->deity->backstab;
		break;
	case 2:
		ch->pcdata->favor += ch->pcdata->deity->flee;
		break;
	case 3:
		ch->pcdata->favor += ch->pcdata->deity->sacrifice;
		break;
	case 4:
		ch->pcdata->favor += ch->pcdata->deity->death;
		break;
	case 5:
		ch->pcdata->favor += ch->pcdata->deity->pkill;
		break;
	case 6:
		ch->pcdata->favor += ch->pcdata->deity->aid;
		break;
	case 7:
		ch->pcdata->favor += ch->pcdata->deity->spell_aid;
		break;
	case 8:
		ch->pcdata->favor += ch->pcdata->deity->spell_harm;
		break;
	case 9:
		ch->pcdata->favor += ch->pcdata->deity->kill_race;
		break;
	}

	ch->pcdata->favor = URANGE( -2500, ch->pcdata->favor, 2500 );
	return;
}


#if defined (KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
					field = value;			\
					fMatch = TRUE;			\
					break;				\
				}



void fread_deity( DEITY_DATA *deity, FILE *fp )
{
	char buf[MSL];
	char *word;
	bool fMatch;

	for ( ; ; )
	{
		word = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER( word[0] ) )
		{
		case '*':
			fMatch = TRUE;
			fread_to_eol( fp );
			break;
		case 'A':
			KEY( "Aid",		deity->aid,
						fread_number( fp ) );
			KEY( "Align",		deity->alignment,
						fread_number( fp ) );
			KEY( "AfBy",		deity->affected_by,
						fread_flag( fp ) );
			break;
		case 'B':
			KEY( "Backstab", 	deity->backstab,
						fread_number( fp ) );
			break;
		case 'C':
			KEY( "Class",		deity->class,
						fread_number( fp ) );
			KEY( "Cobj",		deity->cost_obj,
						fread_number( fp ) );
			KEY( "Ccorpse",		deity->cost_corpse,
						fread_number( fp ) );
			KEY( "Cavatar",		deity->cost_avatar,
						fread_number( fp ) );
			KEY( "Crecall",         deity->cost_recall,
						fread_number( fp ) );
			break;
		case 'D':
			KEY( "Death",		deity->death,
						fread_number( fp ) );
			KEY( "Desc",		deity->desc,
						fread_string( fp ) );
			break;
		case 'E':
			if ( !str_cmp( word, "End" ) )
			{
				if ( !deity->name )
					deity->name = " ";
				if ( !deity->desc )
					deity->desc = " ";
				return;
			}
			break;
		case 'F':
			KEY( "Filename",	deity->filename,
						fread_string( fp ) );
			KEY( "Flee",		deity->flee,
						fread_number( fp ) );
			break;
		case 'I':
			KEY( "Immune",		deity->immune,
						fread_flag( fp ) );
			break;
		case 'K':
			KEY( "KillRace",	deity->kill_race,
						fread_number( fp ) );
			break;
		case 'N':
			KEY( "Name",		deity->name,
						fread_string( fp ) );
			break;
		case 'P':
			KEY( "Pkill",		deity->pkill,
						fread_number( fp ) );
			break;
		case 'R':
			KEY( "Race",		deity->race,
						fread_number( fp ) );
			KEY( "Resist",		deity->resist,
						fread_flag( fp ) );
			break;
		case 'S':
			KEY( "Sex",		deity->sex,
						fread_number( fp ) );
			KEY( "Sacrifice",	deity->sacrifice,
						fread_number( fp ) );
                        KEY( "ShBy",            deity->shielded_by,
                                                fread_flag( fp ) );
			KEY( "Steal",		deity->steal,
						fread_number( fp ) );
			KEY( "SpellAid",	deity->spell_aid,
						fread_number( fp ) );
			KEY( "SpellHarm",	deity->spell_harm,
						fread_number( fp ) );
			break;
		case 'V':
			KEY( "Vuln",		deity->vuln,
						fread_flag( fp ) );
			KEY( "Vobj",		deity->vnum_obj,
						fread_number( fp ) );
			KEY( "Vava",		deity->vnum_avatar,
						fread_number( fp ) );
			break;
		case 'W':
			KEY( "Worship",		deity->worshippers,
						fread_number( fp ) );
			break;
		}

		if( deity->name )
			smash_tilde( deity->name );


		if ( !fMatch )
		{
			sprintf( buf, "fread_deity: no match %s", word );
			bug( buf, 0 );
		}
	}
}


bool load_deity( char *deity_file )
{
	char filename[256];
	DEITY_DATA *deity;
	FILE *fp;

	bool found = FALSE;
	sprintf( filename, "%s%s", DEITY_DIR, deity_file );

	fp = fopen( filename, "r" );

	if ( fp )
	{
		for ( ; ; )
		{
			char letter;
			char *word;

			letter = fread_letter( fp );
			if ( letter == '*' )
			{
				fread_to_eol( fp );
				continue;
			} 

			if ( letter != '#' )
			{
				bug( "load_deity: # not found.", 0 );
				break;
			}
			word = fread_word( fp );
			if ( !str_cmp( word, "DEITY" ) )
			{
				deity = alloc_perm( sizeof(*deity) );  
				fread_deity( deity, fp );
				if ( !first_deity )
				{
					first_deity = deity;
					last_deity = deity;
				}
				else
				{
					last_deity->next = deity;
					deity->prev = last_deity;
					last_deity = deity;
				}
				found = TRUE;
				break;
			}
			else
			{
				char buf[MSL];
				sprintf( buf, "load_deity: bad line %s", word);
				bug( buf, 0 );
				break;
			}
		}
		fclose( fp );
	}
	return found;
} 

void boot_deities( )
{
	FILE *fpDeityList;
	char *filename;
	char deitylist[256];
	char buf[MSL];

	first_deity = NULL;
	last_deity = NULL;

	log_string( "Booting Deities.." );

	sprintf( deitylist, "%s%s", DEITY_DIR, DEITY_FILE );
	fpDeityList = fopen( deitylist, "r" );
	if ( !fpDeityList )
	{
		perror( deitylist );
		exit( 1 );
	}

	for ( ; ; )
	{
		filename = feof( fpDeityList) ? "$" : fread_word( fpDeityList );
		if ( filename[0] == '$' )
			break;
		log_string( filename );
		if ( !load_deity( filename ) )
		{
			sprintf( buf, "Cannot load deity file: %s", filename );
			bug( buf, 0 );
		}
	}
	fclose( fpDeityList );
	log_string( "Done Loading Deities " );
	return;
}


void save_deity( DEITY_DATA *deity )
{
	FILE *fp;
	char filename[256];
	char buf[MSL];

	if ( !deity )
	{
		bug( "save_deity: null deity pointer!", 0 );
		return;
	}

	if( !deity->filename || deity->filename[0] == '\0' )
	{
		sprintf( buf, "save_deity: %s has no filename", deity->name );
		bug( buf, 0 );
		return;
	}

	sprintf( filename, "%s%s", DEITY_DIR, deity->filename );

	fclose( fpReserve );
	fp = fopen( filename, "w" );
	if ( !fp )
	{
		bug( "save_deity: Opening Data File", 0 );
		perror( filename );
	}
	else
	{
		fprintf( fp, "#DEITY\n" );
		fprintf( fp, "Filename		%s~\n", deity->filename );
		fprintf( fp, "Name		%s~\n", deity->name );
		fprintf( fp, "Desc		%s~\n", deity->desc );
		fprintf( fp, "Vuln		%s\n", 
						print_flags( deity->vuln ));
		fprintf( fp, "Resist		%s\n", 
						print_flags( deity->resist ));
		fprintf( fp, "Immune		%s\n",
						print_flags( deity->immune ));
		fprintf( fp, "AfBy		%s\n",
						print_flags( deity->affected_by ) );              
		fprintf( fp, "ShBy		%s\n",
						print_flags( deity->shielded_by ) );
		fprintf( fp, "Sex 		%d\n", deity->sex );
		fprintf( fp, "Class		%d\n", deity->class );
		fprintf( fp, "Race		%d\n", deity->race );
		fprintf( fp, "Death             %d\n", deity->death );
		fprintf( fp, "Worship		%d\n", deity->worshippers );
		fprintf( fp, "Vobj		%d\n", deity->vnum_obj );
		fprintf( fp, "Vava		%d\n", deity->vnum_avatar );
		fprintf( fp, "Cobj		%d\n", deity->cost_obj );
		fprintf( fp, "Ccorpse		%d\n", deity->cost_corpse );
		fprintf( fp, "Cavatar		%d\n", deity->cost_avatar );
		fprintf( fp, "Crecall		%d\n", deity->cost_recall );
		fprintf( fp, "Align		%d\n", deity->alignment );
		fprintf( fp, "Steal		%d\n", deity->steal );
		fprintf( fp, "Backstab		%d\n", deity->backstab );
		fprintf( fp, "Flee		%d\n", deity->flee );
		fprintf( fp, "Sacrifice		%d\n", deity->sacrifice );
		fprintf( fp, "Death		%d\n", deity->death );
		fprintf( fp, "Aid		%d\n", deity->aid );
		fprintf( fp, "Pkill		%d\n", deity->pkill );
		fprintf( fp, "SpellAid		%d\n", deity->spell_aid );
		fprintf( fp, "SpellHarm		%d\n", deity->spell_harm );
		fprintf( fp, "KillRace		%d\n", deity->kill_race );
		fprintf( fp, "End\n" );
		fclose( fp );
	}
	fpReserve = fopen ( NULL_FILE, "r" );
	return;
}


	
void do_deities( CHAR_DATA *ch, char *argument )
{
	DEITY_DATA *deity;
	int count = 0;
	char buf[MSL];
	char buf2[MSL];


	if ( argument[0] == '\0' )
	{
		send_to_char ( "For specific information on a specific deity use the command: deities <deity>\n\r", ch ); 
		send_to_char ( "\n\rDeity                      Worshippers\n\r", ch );
		for ( deity = first_deity; deity; deity = deity->next )
		{
			sprintf( buf, "%s       %d\n\r", deity->name, 
				deity->worshippers );
			send_to_char( buf, ch );
			count++;
		}
	

		if ( !count )
		{
			send_to_char( "No deities exist on this world!\n\r", ch );
			return;
		}

		return;
	}

	deity = get_deity( argument );
	if ( !deity )
	{
		send_to_char( "That deity does not exist!", ch );
		return;
	}


	sprintf( buf2, "Deity: %s\n\rDescription:\n\r%s", deity->name, deity->desc );
	send_to_char( buf2, ch );
	return;
}



void do_devote( CHAR_DATA *ch, char *argument )
{
	char arg[MIL];
	DEITY_DATA *deity;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	if ( ch->level < 10 )
	{
		send_to_char( "You are not yet prepared for such devotion.\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Devote yourself to which deity?\n\r", ch );
		return;
	}

	if ( !str_cmp( arg, "none" ) )
	{
		AFFECT_DATA af;
		if ( !ch->pcdata->deity )
		{
			send_to_char( "You have already chosen to worhsip no deity.\n\r", ch );
			return;
		}
		--ch->pcdata->deity->worshippers;
		ch->pcdata->favor = -1000;
		send_to_char ( "A terrible curse afflicts you as you forsake your deity!\n\r", ch );
		if ( IS_SET( ch->affected_by, ch->pcdata->deity->affected_by ) )
		{
			REMOVE_BIT( ch->affected_by, ch->pcdata->deity->affected_by );
		}
                if ( IS_SET( ch->shielded_by, ch->pcdata->deity->shielded_by ) )                {
                        REMOVE_BIT( ch->shielded_by, ch->pcdata->deity->shielded_by );
                }
		if ( IS_SET ( ch->res_flags, ch->pcdata->deity->resist ) )
		{
			REMOVE_BIT( ch->res_flags, ch->pcdata->deity->resist );
		}
		if ( IS_SET ( ch->vuln_flags, ch->pcdata->deity->vuln ) )
		{
			REMOVE_BIT( ch->vuln_flags, ch->pcdata->deity->vuln );
		}

		if ( IS_AFFECTED( ch, gsn_blindness ) )
			affect_strip( ch, gsn_blindness );

		af.type 	= gsn_blindness;
		af.location	= APPLY_HITROLL;
		af.modifier	= -4;
		af.duration	= 125;
		af.bitvector	= AFF_BLIND;
		affect_to_char( ch, &af );
		save_deity( ch->pcdata->deity );
		send_to_char ( "You cease to worship any deity.\n\r", ch );
		ch->pcdata->deity = NULL;
		save_char_obj( ch );
		return;
	}

	deity = get_deity( arg );
	if( !deity )
	{
	    send_to_char( "No such deity exists.\n\r", ch );
	    return;
	}

	if( ch->pcdata->deity )
	{
	    send_to_char( "You are already devoted to a deity.\n\r", ch );
	    return;
	}

	if( ( deity->class != -1 ) && ( deity->class != ch->class ) )
	{
	    if (ch->pcdata->tier != 2)
	    {
		send_to_char( "That deity does not accept worshippers of your class.\n\r", ch );
		return;
	    } else if ( deity->class != ch->clasb )
	    {
		send_to_char( "That deity does not accept worshippers of either of your classes.\n\r", ch );
		return;
	    }
	}

	if ( ( deity->race != -1 ) && ( deity->race != ch->race ) )
	{
	    send_to_char( "That deity does not accept worshippers of your race.\n\r", ch );
	    return;
	}

	ch->pcdata->deity = deity;
	ch->pcdata->deity_name = deity->name;
	
	if ( !IS_SET( ch->affected_by, ch->pcdata->deity->affected_by ) )
	{
		SET_BIT( ch->affected_by, ch->pcdata->deity->affected_by );
	}
        if ( !IS_SET( ch->shielded_by, ch->pcdata->deity->shielded_by ) )
        {
                SET_BIT( ch->shielded_by, ch->pcdata->deity->shielded_by );
        }

	if ( !IS_SET( ch->res_flags, ch->pcdata->deity->resist ) )
	{
		SET_BIT( ch->res_flags, ch->pcdata->deity->resist );
	}

	if ( !IS_SET( ch->vuln_flags, ch->pcdata->deity->vuln ) )
	{
		SET_BIT( ch->vuln_flags, ch->pcdata->deity->vuln );
	}

	act( "Body and Soul, you devote yourself to $t", ch, ch->pcdata->deity->name, NULL, TO_CHAR );
	++ch->pcdata->deity->worshippers;
	save_deity( ch->pcdata->deity );
	save_char_obj( ch );
	return;
}

void do_supplicate( CHAR_DATA *ch, char *argument )
{
	char arg[MIL];

	one_argument( argument, arg );
	if( IS_NPC(ch) || !ch->pcdata->deity )
	{
		send_to_char( "You have no deity to supplicate to.\n\r", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Supplicate for what?\n\r", ch );
		return;
	}

	if ( !str_cmp( arg, "corpse" ) )
	{
		char buf2[MSL];
		char buf3[MSL];
		OBJ_DATA *obj;
		bool found;

		if( ch->pcdata->favor < ch->pcdata->deity->cost_corpse )
		{
			send_to_char( "You are not favored enough for a corpse retrieval.\n\r", ch );
			return;
		}

		found = FALSE;
		sprintf( buf3, " " );
		sprintf( buf2, "the corpse of %s", ch->name );
		for ( obj = object_list; obj; obj = obj->next )
		{
			if( obj->in_room && !str_cmp( buf2 , obj->short_descr )
				&& ( (int) obj->pIndexData->vnum == 11 ) )
			{
				found = TRUE;
				obj_from_room(obj);
				obj_to_room( obj, ch->in_room );
				ch->pcdata->favor -= ch->pcdata->deity->cost_corpse;

			}
		}

		if ( !found )
		{
			send_to_char( "No corpse of yours litters the world...\n\r", ch );
			return;
		}
		return;
	}

	if ( !str_cmp( arg, "avatar" ) )
	{
		MOB_INDEX_DATA *pMobIndex;
		CHAR_DATA *victim;

		if ( ch->pcdata->favor < ch->pcdata->deity->cost_avatar)
		{
			send_to_char( "You are not favored enough for an Avatar.\n\r", ch );
			return;
		}

		if ( ch->pcdata->deity->vnum_avatar < 1 )
		{
			send_to_char( "Your deity does not have an Avatar to summon.\n\r", ch );
			return;
		}

		pMobIndex = get_mob_index( ch->pcdata->deity->vnum_avatar );
		victim = create_mobile( pMobIndex );
		char_to_room( victim, ch->in_room );
		victim->master = ch;
		victim->leader = ch;
		SET_BIT( victim->act, ACT_PET );
		ch->pet = victim;
		add_follower( victim, ch );
		ch->pcdata->favor -= ch->pcdata->deity->cost_avatar;
		return;
	}

	if ( !str_cmp( arg, "object" ) )
	{
		OBJ_DATA *obj;
		OBJ_INDEX_DATA *pObjIndex;

		if ( ch->pcdata->favor < ch->pcdata->deity->cost_obj )
		{ 
			send_to_char( "You are not favored enough for that.\n\r", ch );
			return;
		}

		if ( ch->pcdata->deity->vnum_obj < 1 )
		{
			send_to_char( "Your deity does not have a sigil of worship.\n\r", ch );
			return;
		}

		pObjIndex = get_obj_index( ch->pcdata->deity->vnum_obj );

		obj = create_object( pObjIndex, ch->level );
		if ( CAN_WEAR( obj, ITEM_TAKE ) )
			obj_to_char( obj, ch );
		else
			obj_to_room( obj, ch->in_room );

		act("$n weaves $p from divine matter!", ch, obj, NULL, TO_ROOM);
		act("You weave $p from divine matter!", ch, obj, NULL, TO_CHAR);

		ch->pcdata->favor -= ch->pcdata->deity->cost_obj;
		return;
	}

	if ( !str_cmp( arg, "recall" ) )
	{

		ROOM_INDEX_DATA *location;

		if ( ch->pcdata->favor < ch->pcdata->deity->cost_recall )
		{
			send_to_char( "You do not have enough favor for such a supplication.\n\r", ch );
			return;
		} 
		
		location = NULL;
		
		if ( IS_NPC( ch ) )
			return;

		if ( ch->alignment < 0 )
		{
			if ( ( location = get_room_index( ROOM_VNUM_TEMPLEB ) ) == NULL )
			{
				send_to_char( "You are completely lost.\n\r", ch );
				return;
			}
		}
		else
		{
			if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
			{
				send_to_char( "You are completely lost.\n\r", ch );
				return;
			}
		}

		if ( ch->in_room == location )
			return;


		if ( is_clan(ch) )
			location = get_room_index( clan_table[ch->clan].hall );

		if ( !location )
		{
			send_to_char( "You are completely lost.\n\r", ch );
			return;
		}

		act("$n disappears in a column of divine power.", ch, NULL, NULL, TO_ROOM );
		char_from_room( ch );
		char_to_room( ch, location );
		if ( ch->pet )
		{
			char_from_room( ch->pet );
			char_to_room( ch->pet, location );
		}

		act("$n appears in the room from a column of divine mist.", ch, NULL, NULL, TO_ROOM );
		do_look( ch, "auto" );
		ch->pcdata->favor -= ch->pcdata->deity->cost_recall;
		return;
	}

	send_to_char( "You cannot supplicate for that.\n\r", ch );
	return;
}

