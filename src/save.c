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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif


int rename(const char *oldfname, const char *newfname);
char *fix_string( const char *str );

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void	fwrite_pet	args( ( CHAR_DATA *pet, FILE *fp) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );



/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MIL];
    FILE *fp;

    if ( IS_NPC(ch) )
	return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL)
    {
	fclose(fpReserve);
	sprintf(strsave, "%s%s",GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave,"w")) == NULL)
	{
	    bug("Save_char_obj: fopen",0);
	    perror(strsave);
 	}

	fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
	    ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
    }
#endif

    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying != NULL )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	/* save the pets */
	if (ch->pet != NULL)
	    fwrite_pet(ch->pet,fp);
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    unlink(strsave);
    rename(TEMP_FILE,strsave);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, gn, pos;

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"	);

    fprintf( fp, "Name %s~\n",	ch->name		);
    fprintf( fp, "Id   %ld\n", ch->id			);
    fprintf( fp, "LogO %ld\n",	current_time		);
    fprintf( fp, "Vers %d\n",   5			);

    if (ch->short_descr && ch->short_descr[0] != '\0')
		fprintf( fp, "ShD  %s~\n",	ch->short_descr	);

    if (ch->pcdata->cname && ch->pcdata->cname[0] != '\0')
        fprintf( fp, "CNa  %s~\n",      ch->pcdata->cname);

    if (ch->long_descr && ch->long_descr[0] != '\0')
		fprintf( fp, "LnD  %s~\n",	ch->long_descr	);

    if (ch->description[0] != '\0')
		fprintf( fp, "Desc %s~\n",	fix_string(ch->description)	);

    if( ch->prompt != NULL
     || !str_cmp( ch->prompt,"<%hhp %mm %vmv %S> " )
     || !str_cmp( ch->prompt,"{c<%hhp %mm %vmv %S>{x " ) )
        fprintf( fp, "Prom %s~\n",      ch->prompt  	);
    fprintf( fp, "Race %s~\n", race_table[ch->race].name );
    if (ch->questpoints != 0)
	fprintf( fp, "QuestPnts %d\n", ch->questpoints  );
 //   if (ch->nextaquest != 0)
//	fprintf( fp, "AQuestNext %d\n", ch->nextaquest    );
    if (ch->nextquest != 0)
	fprintf( fp, "QuestNext %d\n", ch->nextquest    );
  //  else if (ch->aqcountdown == 0)
//	fprintf( fp, "AQuestNext %d\n", 0               );
    else if (ch->qcountdown == 0)
	fprintf( fp, "QuestNext %d\n", 0               );
    if (ch->clan)
    {
    	fprintf( fp, "Clan %s~\n",clan_table[ch->clan].name);
	if (ch->clead)
	    fprintf( fp, "Clead %s~\n",clan_table[ch->clan].name);
    }
    if (ch->clock)
	fprintf( fp, "Clck %d\n", ch->clock);
    if (!IS_NPC(ch))
    {
	if ( ch->pcdata->deity )
	{
	    fprintf( fp, "Deity  %s~\n",	ch->pcdata->deity_name);
	    fprintf( fp, "Favor  %d\n",		ch->pcdata->favor     );
	}
	if (ch->pcdata->advanced != 0)
	    fprintf( fp, "Advan  1\n");
    }
    if (ch->spirit != 0)
	fprintf( fp, "Spirit 1\n");
    if (ch->home != 0)
	fprintf( fp, "Home %s~\n",	home_table[ch->home].name );
    if (ch->home_none != 0)
	fprintf( fp, "Homn %d\n",	ch->home_none);
    if (!IS_NPC(ch) && (ch->can_aquest == 2))
    {
	fprintf( fp, "Qon 2\n");
	fprintf( fp, "Qob %s~\n", ch->pcdata->lquest_obj);
	fprintf( fp, "Qmo %s~\n", ch->pcdata->lquest_mob);
	fprintf( fp, "Qar %s~\n", ch->pcdata->lquest_are);
	fprintf( fp, "Qvn %d\n", ch->pcdata->quest_mob);
    }
    if (!IS_NPC(ch) && (ch->can_aquest == 1))
    {
	fprintf( fp, "Qon 1\n");
    }
    fprintf( fp, "Sex  %d\n",	ch->sex			);
    fprintf( fp, "Clas %s~\n",  class_table[ch->class].name );
    if (!IS_NPC(ch))
	if (ch->pcdata->tier >= 2)
	    fprintf( fp, "Clab %s~\n",  class_table[ch->clasb].name );
    fprintf( fp, "Levl %d\n",	ch->level		);
    if(!IS_NPC(ch))
	fprintf( fp, "Tier %d\n",  ch->pcdata->tier	);
    if (ch->trust != 0)
	fprintf( fp, "Tru  %d\n",	ch->trust	);
    fprintf( fp, "Sec  %d\n",    ch->pcdata->security	);	/* OLC */
    fprintf( fp, "Plyd %d\n",
	ch->played + (int) (current_time - ch->logon)	);
    if (ch->newbie != 0)
	fprintf( fp, "New 1\n");
    if(!IS_NPC(ch))
	fprintf( fp, "Notb  %ld %ld %ld %ld %ld %ld\n",		
	ch->pcdata->last_note,ch->pcdata->last_idea,ch->pcdata->last_penalty,
	ch->pcdata->last_news,ch->pcdata->last_changes,ch->pcdata->last_weddings);
    if(!IS_NPC(ch))
	fprintf( fp, "Mudl  %ld\n", ch->pcdata->last_mud );
    fprintf( fp, "Scro %d\n", 	ch->lines		);
    fprintf( fp, "Room %d\n",
        (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
        && ch->was_in_room != NULL )
            ? ch->was_in_room->vnum
            : ch->in_room == NULL ? 3001 : ch->in_room->vnum );

    fprintf( fp, "HMV  %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    if (ch->platinum > 0)
      fprintf( fp, "Plat %ld\n",	ch->platinum	);
    else
      fprintf( fp, "Plat %d\n", 0			); 
    if (ch->gold > 0)
      fprintf( fp, "Gold %ld\n",	ch->gold	);
    else
      fprintf( fp, "Gold %d\n", 0			); 
    if (ch->silver > 0)
	fprintf( fp, "Silv %ld\n",ch->silver		);
    else
	fprintf( fp, "Silv %d\n",0			);
    fprintf( fp, "Bala  %ld %ld %ld %ld %ld\n",
	ch->balance[0] > 0 ? ch->balance[0] : 0,
	ch->balance[1] > 0 ? ch->balance[1] : 0,
	ch->balance[2] > 0 ? ch->balance[2] : 0,
	ch->balance[3] > 0 ? ch->balance[3] : 0,
        ch->balance[4] > 0 ? ch->balance[4] : 0);
    fprintf( fp, "Exp  %ld\n",	ch->exp			);
    if (ch->qps != 0)
	fprintf( fp, "Qps  %d\n", ch->qps		);
    if (ch->aqps != 0)
	fprintf( fp, "AQps  %d\n", ch->aqps		);
    if (ch->act != 0)
	fprintf( fp, "Act  %s\n",   print_flags(ch->act));
    if (ch->act2 != 0)
	fprintf( fp, "Act2  %s\n",   print_flags(ch->act2));	
    if (ch->exbit1_flags != 0)
	fprintf( fp, "Exbit1  %s\n",   print_flags(ch->exbit1_flags));
    if (ch->affected_by != 0)
	fprintf( fp, "AfBy %s\n",   print_flags(ch->affected_by));
    if (ch->shielded_by != 0)
	fprintf( fp, "ShBy %s\n",   print_flags(ch->shielded_by));
    fprintf( fp, "Comm %s\n",       print_flags(ch->comm));
    if (ch->wiznet)
    	fprintf( fp, "Wizn %s\n",   print_flags(ch->wiznet));
    if (ch->invis_level)
	fprintf( fp, "Invi %d\n", 	ch->invis_level	);
    if (ch->incog_level)
	fprintf(fp,"Inco %d\n",ch->incog_level);
    if (ch->ghost_level)
	fprintf(fp,"Ghos %d\n",ch->ghost_level);
    fprintf( fp, "Pos  %d\n",	
	ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    if (ch->practice != 0)
    	fprintf( fp, "Prac %d\n",	ch->practice	);
    if (ch->train != 0)
	fprintf( fp, "Trai %d\n",	ch->train	);
    if (ch->saving_throw != 0)
	fprintf( fp, "Save  %d\n",	ch->saving_throw);
    fprintf( fp, "Alig  %d\n",	ch->alignment		);
    if (ch->hitroll != 0)
	fprintf( fp, "Hit   %d\n",	ch->hitroll	);
    if (ch->damroll != 0)
	fprintf( fp, "Dam   %d\n",	ch->damroll	);
    fprintf( fp, "ACs %d %d %d %d\n",	
	ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
    if (ch->wimpy !=0 )
	fprintf( fp, "Wimp  %d\n",	ch->wimpy	);
    fprintf( fp, "Attr %d %d %d %d %d\n",
	ch->perm_stat[STAT_STR],
	ch->perm_stat[STAT_INT],
	ch->perm_stat[STAT_WIS],
	ch->perm_stat[STAT_DEX],
	ch->perm_stat[STAT_CON] );

    fprintf (fp, "AMod %d %d %d %d %d\n",
	ch->mod_stat[STAT_STR],
	ch->mod_stat[STAT_INT],
	ch->mod_stat[STAT_WIS],
	ch->mod_stat[STAT_DEX],
	ch->mod_stat[STAT_CON] );

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum %d\n",	ch->pIndexData->vnum	);
    }
    else
    {
	fprintf( fp, "Pass %s~\n",	ch->pcdata->pwd		);
	if (ch->pcdata->bamfin[0] != '\0')
	    fprintf( fp, "Bin  %s~\n",	ch->pcdata->bamfin);
	if (ch->pcdata->bamfout[0] != '\0')
		fprintf( fp, "Bout %s~\n",	ch->pcdata->bamfout);
	if (ch->pcdata->who_descr[0] != '\0')
		fprintf( fp, "Whod %s~\n",	ch->pcdata->who_descr);
	fprintf( fp, "Titl %s~\n",	ch->pcdata->title	);
    	fprintf( fp, "Pnts %d\n",   	ch->pcdata->points      );
	fprintf( fp, "TSex %d\n",	ch->pcdata->true_sex	);
	fprintf( fp, "Awin %d\n",	ch->pcdata->awins	);
	fprintf( fp, "Alos %d\n",	ch->pcdata->alosses	);
	fprintf( fp, "Pkil %d\n",	ch->pcdata->pkills	);
	fprintf( fp, "Pdea %d\n",	ch->pcdata->pdeath	);
	fprintf( fp, "Afle %d\n",	ch->pcdata->aflee	);
	fprintf( fp, "Dfle %d\n",	ch->pcdata->dflee	);
	fprintf( fp, "LLev %d\n",	ch->pcdata->last_level	);
	fprintf( fp, "Reca %d\n",	ch->pcdata->recall	);
	fprintf( fp, "Camp %d\n",	ch->pcdata->camp	);
	fprintf( fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit, 
						   ch->pcdata->perm_mana,
						   ch->pcdata->perm_move);
	fprintf( fp, "Cnd  %d %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2],
	    ch->pcdata->condition[3] );

	fprintf (fp, "Stance   %d %d %d %d %d %d %d %d %d %d %d\n",
	    ch->stance[0], ch->stance[1], ch->stance[2], ch->stance[3],
	    ch->stance[4], ch->stance[5], ch->stance[6], ch->stance[7],
	    ch->stance[8], ch->stance[9], ch->stance[10] );

        /*
         * Write Colour Config Information.
         */
        fprintf (fp, "Coloura     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->text[2],
                 ch->pcdata->text[0],
                 ch->pcdata->text[1],
                 ch->pcdata->auction[2],
                 ch->pcdata->auction[0],
                 ch->pcdata->auction[1],
                 ch->pcdata->gossip[2],
                 ch->pcdata->gossip[0],
                 ch->pcdata->gossip[1],
                 ch->pcdata->music[2],
                 ch->pcdata->music[0],
                 ch->pcdata->music[1],
                 ch->pcdata->question[2],
                 ch->pcdata->question[0], ch->pcdata->question[1]);
        fprintf (fp, "Colourb     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->answer[2],
                 ch->pcdata->answer[0],
                 ch->pcdata->answer[1],
                 ch->pcdata->quote[2],
                 ch->pcdata->quote[0],
                 ch->pcdata->quote[1],
                 ch->pcdata->quote_text[2],
                 ch->pcdata->quote_text[0],
                 ch->pcdata->quote_text[1],
                 ch->pcdata->immtalk_text[2],
                 ch->pcdata->immtalk_text[0],
                 ch->pcdata->immtalk_text[1],
                 ch->pcdata->immtalk_type[2],
                 ch->pcdata->immtalk_type[0], ch->pcdata->immtalk_type[1]);
        fprintf (fp, "Colourc     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->info[2],
                 ch->pcdata->info[0],
                 ch->pcdata->info[1],
                 ch->pcdata->tell[2],
                 ch->pcdata->tell[0],
                 ch->pcdata->tell[1],
                 ch->pcdata->reply[2],
                 ch->pcdata->reply[0],
                 ch->pcdata->reply[1],
                 ch->pcdata->gtell_text[2],
                 ch->pcdata->gtell_text[0],
                 ch->pcdata->gtell_text[1],
                 ch->pcdata->gtell_type[2],
                 ch->pcdata->gtell_type[0], ch->pcdata->gtell_type[1]);
        fprintf (fp, "Colourd     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->room_title[2],
                 ch->pcdata->room_title[0],
                 ch->pcdata->room_title[1],
                 ch->pcdata->room_text[2],
                 ch->pcdata->room_text[0],
                 ch->pcdata->room_text[1],
                 ch->pcdata->room_exits[2],
                 ch->pcdata->room_exits[0],
                 ch->pcdata->room_exits[1],
                 ch->pcdata->room_things[2],
                 ch->pcdata->room_things[0],
                 ch->pcdata->room_things[1],
                 ch->pcdata->prompt[2],
                 ch->pcdata->prompt[0], ch->pcdata->prompt[1]);
        fprintf (fp, "Coloure     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->fight_death[2],
                 ch->pcdata->fight_death[0],
                 ch->pcdata->fight_death[1],
                 ch->pcdata->fight_yhit[2],
                 ch->pcdata->fight_yhit[0],
                 ch->pcdata->fight_yhit[1],
                 ch->pcdata->fight_ohit[2],
                 ch->pcdata->fight_ohit[0],
                 ch->pcdata->fight_ohit[1],
                 ch->pcdata->fight_thit[2],
                 ch->pcdata->fight_thit[0],
                 ch->pcdata->fight_thit[1],
                 ch->pcdata->fight_skill[2],
                 ch->pcdata->fight_skill[0], ch->pcdata->fight_skill[1]);
        fprintf (fp, "Colourf     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->wiznet[2],
                 ch->pcdata->wiznet[0],
                 ch->pcdata->wiznet[1],
                 ch->pcdata->say[2],
                 ch->pcdata->say[0],
                 ch->pcdata->say[1],
                 ch->pcdata->say_text[2],
                 ch->pcdata->say_text[0],
                 ch->pcdata->say_text[1],
                 ch->pcdata->tell_text[2],
                 ch->pcdata->tell_text[0],
                 ch->pcdata->tell_text[1],
                 ch->pcdata->reply_text[2],
                 ch->pcdata->reply_text[0], ch->pcdata->reply_text[1]);
        fprintf (fp, "Colourg     %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d\n",
                 ch->pcdata->auction_text[2],
                 ch->pcdata->auction_text[0],
                 ch->pcdata->auction_text[1],
                 ch->pcdata->gossip_text[2],
                 ch->pcdata->gossip_text[0],
                 ch->pcdata->gossip_text[1],
                 ch->pcdata->music_text[2],
                 ch->pcdata->music_text[0],
                 ch->pcdata->music_text[1],
                 ch->pcdata->question_text[2],
                 ch->pcdata->question_text[0],
                 ch->pcdata->question_text[1],
                 ch->pcdata->answer_text[2],
                 ch->pcdata->answer_text[0], ch->pcdata->answer_text[1]);

	/* write forgets */
        for (pos = 0; pos < MAX_FORGET; pos++)
	{
	    if (ch->pcdata->forget[pos] == NULL)
		break;

	    fprintf(fp,"Forge %s~\n",ch->pcdata->forget[pos]);
	}

	/* write dupes */
        for (pos = 0; pos < MAX_DUPES; pos++)
	{
	    if (ch->pcdata->dupes[pos] == NULL)
		break;

	    fprintf(fp,"Dupes %s~\n",ch->pcdata->dupes[pos]);
	}

	/* write alias */
        for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (ch->pcdata->alias[pos] == NULL
	    ||  ch->pcdata->alias_sub[pos] == NULL)
		break;

	    fprintf(fp,"Alias %s %s~\n",ch->pcdata->alias[pos],
		    ch->pcdata->alias_sub[pos]);
	}

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
	    {
		fprintf( fp, "Sk %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}

	for ( gn = 0; gn < MAX_GROUP; gn++ )
        {
            if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn])
            {
                fprintf( fp, "Gr '%s'\n",group_table[gn].name);
            }
        }
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type>= MAX_SKILL)
	    continue;
	
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
	    skill_table[paf->type].name,
	    paf->where,
	    paf->level,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
    
    fprintf(fp,"#PET\n");
    
    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    if (pet->clan)
        fprintf( fp, "Clan %s~\n",clan_table[pet->clan].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
    	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
    if (pet->platinum > 0)
    	fprintf(fp,"Plat %ld\n",pet->platinum);
    if (pet->gold > 0)
    	fprintf(fp,"Gold %ld\n",pet->gold);
    if (pet->silver > 0)
	fprintf(fp,"Silv %ld\n",pet->silver);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %ld\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
    	fprintf(fp, "Act  %s\n", print_flags(pet->act));
// if (pet->exbit1_flags != pet->pIndexData->exbit1_flags)
// fprintf(fp, "Exbit1  %s\n", print_flags(pet->exbit1_flags));
    if (pet->affected_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
    if (pet->shielded_by != pet->pIndexData->shielded_by)
    	fprintf(fp, "ShBy %s\n", print_flags(pet->shielded_by));
    if (pet->comm != 0)
    	fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
    	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
    	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n",
    	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
    	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
    	pet->perm_stat[STAT_CON]);
    fprintf(fp, "AMod %d %d %d %d %d\n",
    	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
    	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
    	pet->mod_stat[STAT_CON]);

    fprintf (fp, "PetStance   %d %d %d %d %d %d %d %d %d %d %d\n",
	pet->stance[0], pet->stance[1], pet->stance[2], pet->stance[3],
	pet->stance[4], pet->stance[5], pet->stance[6], pet->stance[7],
	pet->stance[8], pet->stance[9], pet->stance[10] );
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= MAX_SKILL)
    	    continue;
    	    
    	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
    	    skill_table[paf->type].name,
    	    paf->where, paf->level, paf->duration, paf->modifier,paf->location,
    	    paf->bitvector);
    }
    
    fprintf(fp,"End\n");
    return;
}
    
/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */
    if ( (ch->level < obj->level - 2 && obj->level > 19
    && (obj->item_type != ITEM_CONTAINER
    &&  obj->item_type != ITEM_PIT))
    ||   obj->item_type == ITEM_KEY
    ||   (obj->item_type == ITEM_MAP && !obj->value[0]))
	return;

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");
    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
    if (obj->inscribed)
    {
	fprintf( fp,"Inscribed\n");
	fprintf( fp,"Inspname %s~\n", obj->spell_name);
	fprintf( fp,"Inspcount %d\n", obj->spell_count);
    }
    fprintf( fp, "Nest %d\n",	iNest	  	     );

    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
    	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr)
        fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description)
        fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type)
        fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight)
        fprintf( fp, "Wt   %d\n",	obj->weight		     );
    if ( obj->condition != obj->pIndexData->condition)
	fprintf( fp, "Cond %d\n",	obj->condition		     );
    if (obj->questowner != NULL && strlen(obj->questowner) > 1)
	fprintf( fp, "Questowner   %s~\n", obj->questowner           );
    /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );
    if (obj->level != obj->pIndexData->level)
        fprintf( fp, "Lev  %d\n",	obj->level	     );
    if (obj->timer != 0)
        fprintf( fp, "Time %d\n",	obj->timer	     );
    if (obj->quest != 0)
	fprintf( fp, "Quest %d\n",	obj->quest	     );
    fprintf( fp, "Cost %d\n",	obj->cost		     );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4]) 
    	fprintf( fp, "Val  %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;
        fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
            skill_table[paf->type].name,
            paf->where,
            paf->level,
            paf->duration,
            paf->modifier,
            paf->location,
            paf->bitvector
            );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    char strsave[MIL];
    char buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;

	buf[0] = '\0';
    ch = new_char();
    ch->pcdata = new_pcdata();

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->id				= get_pc_id();
    ch->race				= race_lookup("human");
    ch->act				= PLR_NOSUMMON;
    ch->comm				= COMM_COMBINE 
					| COMM_PROMPT
					| COMM_STORE;
    ch->prompt 				= str_dup("<%hhp %mm %vmv %S> ");
    ch->pcdata->confirm_delete		= FALSE;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->who_descr		= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->tier			= 0;
    ch->pcdata->advanced		= 0;
    ch->pcdata->is_aquest		= FALSE;
    ch->can_aquest			= 0;
    ch->pcdata->lquest_obj		= str_dup( "" );
    ch->pcdata->lquest_mob		= str_dup( "" );
    ch->pcdata->lquest_are		= str_dup( "" );
    ch->pcdata->quest_mob		= 0;
    ch->pcdata->quest_obj		= 0;
    for (stat =0; stat < MAX_STATS; stat++)
	ch->perm_stat[stat]		= 13;
    ch->pcdata->condition[COND_THIRST]	= 48; 
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->condition[COND_HUNGER]	= 48;
    ch->pcdata->security		= 0;	/* OLC */
    ch->pcdata->text[0] = (NORMAL);
    ch->pcdata->text[1] = (WHITE);
    ch->pcdata->text[2] = 0;
    ch->pcdata->auction[0] = (BRIGHT);
    ch->pcdata->auction[1] = (YELLOW);
    ch->pcdata->auction[2] = 0;
    ch->pcdata->auction_text[0] = (BRIGHT);
    ch->pcdata->auction_text[1] = (WHITE);
    ch->pcdata->auction_text[2] = 0;
    ch->pcdata->gossip[0] = (NORMAL);
    ch->pcdata->gossip[1] = (MAGENTA);
    ch->pcdata->gossip[2] = 0;
    ch->pcdata->gossip_text[0] = (BRIGHT);
    ch->pcdata->gossip_text[1] = (MAGENTA);
    ch->pcdata->gossip_text[2] = 0;
    ch->pcdata->music[0] = (NORMAL);
    ch->pcdata->music[1] = (RED);
    ch->pcdata->music[2] = 0;
    ch->pcdata->music_text[0] = (BRIGHT);
    ch->pcdata->music_text[1] = (RED);
    ch->pcdata->music_text[2] = 0;
    ch->pcdata->question[0] = (BRIGHT);
    ch->pcdata->question[1] = (YELLOW);
    ch->pcdata->question[2] = 0;
    ch->pcdata->question_text[0] = (BRIGHT);
    ch->pcdata->question_text[1] = (WHITE);
    ch->pcdata->question_text[2] = 0;
    ch->pcdata->answer[0] = (BRIGHT);
    ch->pcdata->answer[1] = (YELLOW);
    ch->pcdata->answer[2] = 0;
    ch->pcdata->answer_text[0] = (BRIGHT);
    ch->pcdata->answer_text[1] = (WHITE);
    ch->pcdata->answer_text[2] = 0;
    ch->pcdata->quote[0] = (NORMAL);
    ch->pcdata->quote[1] = (YELLOW);
    ch->pcdata->quote[2] = 0;
    ch->pcdata->quote_text[0] = (NORMAL);
    ch->pcdata->quote_text[1] = (GREEN);
    ch->pcdata->quote_text[2] = 0;
    ch->pcdata->immtalk_text[0] = (NORMAL);
    ch->pcdata->immtalk_text[1] = (CYAN);
    ch->pcdata->immtalk_text[2] = 0;
    ch->pcdata->immtalk_type[0] = (NORMAL);
    ch->pcdata->immtalk_type[1] = (YELLOW);
    ch->pcdata->immtalk_type[2] = 0;
    ch->pcdata->info[0] = (BRIGHT);
    ch->pcdata->info[1] = (YELLOW);
    ch->pcdata->info[2] = 1;
    ch->pcdata->say[0] = (NORMAL);
    ch->pcdata->say[1] = (GREEN);
    ch->pcdata->say[2] = 0;
    ch->pcdata->say_text[0] = (BRIGHT);
    ch->pcdata->say_text[1] = (GREEN);
    ch->pcdata->say_text[2] = 0;
    ch->pcdata->tell[0] = (NORMAL);
    ch->pcdata->tell[1] = (GREEN);
    ch->pcdata->tell[2] = 0;
    ch->pcdata->tell_text[0] = (BRIGHT);
    ch->pcdata->tell_text[1] = (GREEN);
    ch->pcdata->tell_text[2] = 0;
    ch->pcdata->reply[0] = (NORMAL);
    ch->pcdata->reply[1] = (GREEN);
    ch->pcdata->reply[2] = 0;
    ch->pcdata->reply_text[0] = (BRIGHT);
    ch->pcdata->reply_text[1] = (GREEN);
    ch->pcdata->reply_text[2] = 0;
    ch->pcdata->gtell_text[0] = (NORMAL);
    ch->pcdata->gtell_text[1] = (GREEN);
    ch->pcdata->gtell_text[2] = 0;
    ch->pcdata->gtell_type[0] = (NORMAL);
    ch->pcdata->gtell_type[1] = (RED);
    ch->pcdata->gtell_type[2] = 0;
    ch->pcdata->wiznet[0] = (NORMAL);
    ch->pcdata->wiznet[1] = (GREEN);
    ch->pcdata->wiznet[2] = 0;
    ch->pcdata->room_title[0] = (NORMAL);
    ch->pcdata->room_title[1] = (CYAN);
    ch->pcdata->room_title[2] = 0;
    ch->pcdata->room_text[0] = (NORMAL);
    ch->pcdata->room_text[1] = (WHITE);
    ch->pcdata->room_text[2] = 0;
    ch->pcdata->room_exits[0] = (NORMAL);
    ch->pcdata->room_exits[1] = (GREEN);
    ch->pcdata->room_exits[2] = 0;
    ch->pcdata->room_things[0] = (NORMAL);
    ch->pcdata->room_things[1] = (CYAN);
    ch->pcdata->room_things[2] = 0;
    ch->pcdata->prompt[0] = (NORMAL);
    ch->pcdata->prompt[1] = (CYAN);
    ch->pcdata->prompt[2] = 0;
    ch->pcdata->fight_death[0] = (BRIGHT);
    ch->pcdata->fight_death[1] = (RED);
    ch->pcdata->fight_death[2] = 0;
    ch->pcdata->fight_yhit[0] = (NORMAL);
    ch->pcdata->fight_yhit[1] = (GREEN);
    ch->pcdata->fight_yhit[2] = 0;
    ch->pcdata->fight_ohit[0] = (NORMAL);
    ch->pcdata->fight_ohit[1] = (YELLOW);
    ch->pcdata->fight_ohit[2] = 0;
    ch->pcdata->fight_thit[0] = (NORMAL);
    ch->pcdata->fight_thit[1] = (RED);
    ch->pcdata->fight_thit[2] = 0;
    ch->pcdata->fight_skill[0] = (BRIGHT);
    ch->pcdata->fight_skill[1] = (WHITE);
    ch->pcdata->fight_skill[2] = 0;

    found = FALSE;
    fclose( fpReserve );
    
    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
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
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );


    /* initialize race */
    if (found)
    {
	int i;

	if (ch->race == 0)
	    ch->race = race_lookup("human");

	ch->size = pc_race_table[ch->race].size;
	ch->dam_type = 17; /*punch */

	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[ch->race].skills[i] == NULL)
		break;
	    group_add(ch,pc_race_table[ch->race].skills[i],FALSE);
	}
	ch->affected_by = ch->affected_by|race_table[ch->race].aff;
	ch->shielded_by = ch->shielded_by|race_table[ch->race].shd;
	ch->imm_flags	= ch->imm_flags | race_table[ch->race].imm;
	ch->res_flags	= ch->res_flags | race_table[ch->race].res;
	ch->vuln_flags	= ch->vuln_flags | race_table[ch->race].vuln;
	ch->form	= race_table[ch->race].form;
	ch->parts	= race_table[ch->race].parts;
    }

	
    /* RT initialize skills */

    if (found && ch->version < 2)  /* need to add the new skills */
    {
	group_add(ch,"rom basics",FALSE);
	group_add(ch,class_table[ch->class].base_group,FALSE);
	if (ch->pcdata->tier == 2)
	    group_add(ch,class_table[ch->clasb].base_group,FALSE);
	group_add(ch,class_table[ch->class].default_group,TRUE);
	if (ch->pcdata->tier == 2)
	    group_add(ch,class_table[ch->clasb].default_group,TRUE);
	ch->pcdata->learned[gsn_recall] = 50;
	if (ch->pcdata->tier == 2)
	    ch->pcdata->learned[gsn_recall] = 75;
    }
 
    /* fix levels */
    if (found && ch->version < 3 && (ch->level > 35 || ch->trust > 35))
    {
	switch (ch->level)
	{
	    case(40) : ch->level = 110;	break;  /* imp -> imp */
	    case(39) : ch->level = 109; break;	/* god -> creator */
	    case(38) : ch->level = 108; break;	/* deity -> god */
	    case(37) : ch->level = 107; break;	/* angel -> demigod */
	}

        switch (ch->trust)
        {
            case(40) : ch->trust = 110;  break;	/* imp -> imp */
            case(39) : ch->trust = 109;  break;	/* god -> creator */
            case(38) : ch->trust = 108;  break;	/* deity -> god */
            case(37) : ch->trust = 107;  break;	/* angel -> demigod */
            case(36) : ch->trust = 101;  break;	/* hero -> hero */
        }
    }

    /* no first tier clan leaders less than level 70 */
    if (is_clead(ch) && (ch->class < MCLT_1) && (ch->level < 70))
    {
	update_clanlist(ch, ch->clan, FALSE, TRUE);
    }

    /* ream gold from old pfile type */
    if (found && ch->version < 4)
    {
	ch->gold   /= 100;
    }
    return found;
}


/*
 * Check a player name for duplicate
 */
bool check_char_exist( char *name )
{
    char strsave[MIL];
    char buf[100];
    FILE *fp;
    bool found;
    int e;

	buf[0] = '\0';
    found = FALSE;
    fclose( fpReserve );
    
    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	found = TRUE;
    }
    if ( is_name( name,
	"all auto immortal immortals self someone something the you demise balance circle loner honor") )
    {
	found = TRUE;
    }
    for ( e = 1; e < MAX_CLAN; e++)
    {
        if (!str_prefix(clan_table[e].name, name))
	{
	    found = TRUE;
	}
    }
    if (!str_infix("immortal",name))
	found = TRUE;
    if (!str_infix(" ", name))
	found = TRUE;
    if (!str_infix("fuck",name))
	found = TRUE;
    if (!str_infix("shit",name))
	found = TRUE;
    if (!str_infix("asshole",name))
	found = TRUE;
    if (!str_infix("pussy",name))
	found = TRUE;
    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	found = TRUE;
 
#if defined(MSDOS)
    if ( strlen(name) >  8 )
	found = TRUE;
#endif
 
#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	found = TRUE;
#endif

    {
        char *pc;
        bool fIll,adjcaps = FALSE,cleancaps = FALSE;
        unsigned int total_caps = 0;
 
        fIll = TRUE;
        for ( pc = name; *pc != '\0'; pc++ )
        {
            if ( !isalpha(*pc) )
                found = TRUE;
 
            if ( isupper(*pc)) /* ugly anti-caps hack */
            {
                if (adjcaps)
                    cleancaps = TRUE;
                total_caps++;
                adjcaps = TRUE;
            }
            else
                adjcaps = FALSE;
 
            if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
                fIll = FALSE;
        }
 
        if ( fIll )
            found = TRUE;
 
        if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
            found = TRUE;
    }

    /*
     * Prevent players from being named after mobs.
     */
    {
        extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
        MOB_INDEX_DATA *pMobIndex;
        int iHash;
 
        for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
        {
            for ( pMobIndex  = mob_index_hash[iHash];
                  pMobIndex != NULL;
                  pMobIndex  = pMobIndex->next )
            {       
                if ( is_name( name, pMobIndex->player_name ) )
                    found = TRUE;
            }
        }
    }


    fpReserve = fopen( NULL_FILE, "r" );
    return found;
}

bool load_char_reroll( DESCRIPTOR_DATA *d, char *name )
{
    CHAR_DATA *ch;
    bool found;
    int stat;

    ch = new_char();
    ch->pcdata = new_pcdata();

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->id				= get_pc_id();
    ch->race				= race_lookup("human");
    ch->act				= PLR_NOSUMMON;
    ch->comm				= COMM_COMBINE 
					| COMM_PROMPT
					| COMM_STORE;
    ch->prompt 				= str_dup("<%hhp %mm %vmv %S> ");
    ch->pcdata->confirm_delete		= FALSE;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->who_descr		= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
	if ( ch->pcdata->tier == 2 && ch->level >= 101 ) { ch->pcdata->tier = 2;
	} else { ch->pcdata->tier = 0; }
	ch->pcdata->advanced		= 0;
	ch->pcdata->is_aquest		= FALSE;
    ch->can_aquest			= 0;
    ch->pcdata->lquest_obj		= str_dup( "" );
    ch->pcdata->lquest_mob		= str_dup( "" );
    ch->pcdata->lquest_are		= str_dup( "" );
    ch->pcdata->quest_mob		= 0;
    ch->pcdata->quest_obj		= 0;
    for (stat =0; stat < MAX_STATS; stat++)
	ch->perm_stat[stat]		= 13;
    ch->pcdata->condition[COND_THIRST]	= 48; 
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->condition[COND_HUNGER]	= 48;
    ch->pcdata->text[0] = (NORMAL);
    ch->pcdata->text[1] = (WHITE);
    ch->pcdata->text[2] = 0;
    ch->pcdata->auction[0] = (BRIGHT);
    ch->pcdata->auction[1] = (YELLOW);
    ch->pcdata->auction[2] = 0;
    ch->pcdata->auction_text[0] = (BRIGHT);
    ch->pcdata->auction_text[1] = (WHITE);
    ch->pcdata->auction_text[2] = 0;
    ch->pcdata->gossip[0] = (NORMAL);
    ch->pcdata->gossip[1] = (MAGENTA);
    ch->pcdata->gossip[2] = 0;
    ch->pcdata->gossip_text[0] = (BRIGHT);
    ch->pcdata->gossip_text[1] = (MAGENTA);
    ch->pcdata->gossip_text[2] = 0;
    ch->pcdata->music[0] = (NORMAL);
    ch->pcdata->music[1] = (RED);
    ch->pcdata->music[2] = 0;
    ch->pcdata->music_text[0] = (BRIGHT);
    ch->pcdata->music_text[1] = (RED);
    ch->pcdata->music_text[2] = 0;
    ch->pcdata->question[0] = (BRIGHT);
    ch->pcdata->question[1] = (YELLOW);
    ch->pcdata->question[2] = 0;
    ch->pcdata->question_text[0] = (BRIGHT);
    ch->pcdata->question_text[1] = (WHITE);
    ch->pcdata->question_text[2] = 0;
    ch->pcdata->answer[0] = (BRIGHT);
    ch->pcdata->answer[1] = (YELLOW);
    ch->pcdata->answer[2] = 0;
    ch->pcdata->answer_text[0] = (BRIGHT);
    ch->pcdata->answer_text[1] = (WHITE);
    ch->pcdata->answer_text[2] = 0;
    ch->pcdata->quote[0] = (NORMAL);
    ch->pcdata->quote[1] = (YELLOW);
    ch->pcdata->quote[2] = 0;
    ch->pcdata->quote_text[0] = (NORMAL);
    ch->pcdata->quote_text[1] = (GREEN);
    ch->pcdata->quote_text[2] = 0;
    ch->pcdata->immtalk_text[0] = (NORMAL);
    ch->pcdata->immtalk_text[1] = (CYAN);
    ch->pcdata->immtalk_text[2] = 0;
    ch->pcdata->immtalk_type[0] = (NORMAL);
    ch->pcdata->immtalk_type[1] = (YELLOW);
    ch->pcdata->immtalk_type[2] = 0;
    ch->pcdata->info[0] = (BRIGHT);
    ch->pcdata->info[1] = (YELLOW);
    ch->pcdata->info[2] = 1;
    ch->pcdata->say[0] = (NORMAL);
    ch->pcdata->say[1] = (GREEN);
    ch->pcdata->say[2] = 0;
    ch->pcdata->say_text[0] = (BRIGHT);
    ch->pcdata->say_text[1] = (GREEN);
    ch->pcdata->say_text[2] = 0;
    ch->pcdata->tell[0] = (NORMAL);
    ch->pcdata->tell[1] = (GREEN);
    ch->pcdata->tell[2] = 0;
    ch->pcdata->tell_text[0] = (BRIGHT);
    ch->pcdata->tell_text[1] = (GREEN);
    ch->pcdata->tell_text[2] = 0;
    ch->pcdata->reply[0] = (NORMAL);
    ch->pcdata->reply[1] = (GREEN);
    ch->pcdata->reply[2] = 0;
    ch->pcdata->reply_text[0] = (BRIGHT);
    ch->pcdata->reply_text[1] = (GREEN);
    ch->pcdata->reply_text[2] = 0;
    ch->pcdata->gtell_text[0] = (NORMAL);
    ch->pcdata->gtell_text[1] = (GREEN);
    ch->pcdata->gtell_text[2] = 0;
    ch->pcdata->gtell_type[0] = (NORMAL);
    ch->pcdata->gtell_type[1] = (RED);
    ch->pcdata->gtell_type[2] = 0;
    ch->pcdata->wiznet[0] = (NORMAL);
    ch->pcdata->wiznet[1] = (GREEN);
    ch->pcdata->wiznet[2] = 0;
    ch->pcdata->room_title[0] = (NORMAL);
    ch->pcdata->room_title[1] = (CYAN);
    ch->pcdata->room_title[2] = 0;
    ch->pcdata->room_text[0] = (NORMAL);
    ch->pcdata->room_text[1] = (WHITE);
    ch->pcdata->room_text[2] = 0;
    ch->pcdata->room_exits[0] = (NORMAL);
    ch->pcdata->room_exits[1] = (GREEN);
    ch->pcdata->room_exits[2] = 0;
    ch->pcdata->room_things[0] = (NORMAL);
    ch->pcdata->room_things[1] = (CYAN);
    ch->pcdata->room_things[2] = 0;
    ch->pcdata->prompt[0] = (NORMAL);
    ch->pcdata->prompt[1] = (CYAN);
    ch->pcdata->prompt[2] = 0;
    ch->pcdata->fight_death[0] = (BRIGHT);
    ch->pcdata->fight_death[1] = (RED);
    ch->pcdata->fight_death[2] = 0;
    ch->pcdata->fight_yhit[0] = (NORMAL);
    ch->pcdata->fight_yhit[1] = (GREEN);
    ch->pcdata->fight_yhit[2] = 0;
    ch->pcdata->fight_ohit[0] = (NORMAL);
    ch->pcdata->fight_ohit[1] = (YELLOW);
    ch->pcdata->fight_ohit[2] = 0;
    ch->pcdata->fight_thit[0] = (NORMAL);
    ch->pcdata->fight_thit[1] = (RED);
    ch->pcdata->fight_thit[2] = 0;
    ch->pcdata->fight_skill[0] = (BRIGHT);
    ch->pcdata->fight_skill[1] = (WHITE);
    ch->pcdata->fight_skill[2] = 0;

    found = FALSE;
    return found;
}

/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}
/* provided to free mem leaked with lookup routines */
/* -- Seronis */
#if defined(KEYL)
#undef KEYL
#endif

#define KEYL( literal, field, value, lookup ) \
if ( !str_cmp( word, literal ) ) \
{ \
char * temp = value; \
field = lookup(temp); \
free_string(temp); \
fMatch = TRUE; \
break; \
}

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( literal, field, value )                    \
                if ( !str_cmp( word, literal ) )    \
                {                    \
                    free_string(field);            \
                    field  = value;            \
                    fMatch = TRUE;            \
                    break;                \
                }

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MSL];
    char *word;
    bool fMatch;
    int count = 0;
    int dcount = 0;
    int fcount = 0;
    int lastlogoff = current_time;
    int percent;

    sprintf(buf,"Loading %s.",ch->name);
    if (strcmp(ch->name, "") )
    {
	log_string(buf);
    }

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,		fread_flag( fp ) );
	    KEY( "Act2",	ch->act2,		fread_flag( fp ) );
	    KEY( "Advan",	ch->pcdata->advanced,	fread_number( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,	fread_flag( fp ) );
	    KEY( "AfBy",	ch->affected_by,	fread_flag( fp ) );
	    KEY( "Afle",	ch->pcdata->aflee,	fread_number( fp ) );
	    KEY( "Alos",	ch->pcdata->alosses,	fread_number( fp ));
	    KEY( "Awin",	ch->pcdata->awins,	fread_number( fp ));
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Alig",	ch->alignment,		fread_number( fp ) );
	    KEY( "AQps",	ch->aqps,		fread_number( fp ) );

	    if (!str_cmp( word, "Alia"))
	    {
		if (count >= MAX_ALIAS)
		{
		    fread_to_eol(fp);
		    fMatch = TRUE;
		    break;
		}

		ch->pcdata->alias[count] 	= str_dup(fread_word(fp));
		ch->pcdata->alias_sub[count]	= str_dup(fread_word(fp));
		count++;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp( word, "Alias"))
            {
                if (count >= MAX_ALIAS)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }
 
                ch->pcdata->alias[count]        = str_dup(fread_word(fp));
                ch->pcdata->alias_sub[count]    = fread_string(fp);
                count++;
                fMatch = TRUE;
                break;
            }

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word,"ACs"))
	    {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp(word, "Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                    paf->type = sn;
 
                paf->where  = fread_number(fp);
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = ch->affected;
                ch->affected    = paf;
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
	    {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
	    {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEYS( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEYS( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEYS( "Bin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEYS( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    if ( !str_cmp( word, "Bal" ) )
	    {
		ch->balance[0] = fread_number(fp);
		ch->balance[1] = fread_number(fp);
		ch->balance[2] = fread_number(fp);
		ch->balance[3] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "Bala" ) )
            {
                ch->balance[0] = fread_number(fp);
                ch->balance[1] = fread_number(fp);
                ch->balance[2] = fread_number(fp);
                ch->balance[3] = fread_number(fp);
                ch->balance[4] = fread_number(fp);
                fMatch = TRUE;
                break;
            }
	    break;

	case 'C':
	    KEY( "Camp",	ch->pcdata->camp, 	fread_number( fp ) );
	    KEYL( "Clas",       ch->class, 		fread_string( fp ), class_lookup );
	    KEYL( "Clab",       ch->clasb, 		fread_string( fp ), class_lookup );
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    KEY( "Cla",		ch->class,		fread_number( fp ) );
	    KEYL( "Clan",	ch->clan,		fread_string( fp ), clan_lookup );
	    KEYL( "Clead",	ch->clead,		fread_string( fp ), clan_lookup );
	    KEY( "Clck",	ch->clock,		fread_number( fp ) );
            KEYS( "CNa",         ch->pcdata->cname,      fread_string( fp ) );

	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
            if (!str_cmp(word,"Cnd"))
            {
                ch->pcdata->condition[0] = fread_number( fp );
                ch->pcdata->condition[1] = fread_number( fp );
                ch->pcdata->condition[2] = fread_number( fp );
		ch->pcdata->condition[3] = fread_number( fp );
                fMatch = TRUE;
                break;
            }
                if (!str_cmp (word, "Coloura"))
                {
                    LOAD_COLOUR (text)
                        LOAD_COLOUR (auction)
                        LOAD_COLOUR (gossip)
                        LOAD_COLOUR (music)
                        LOAD_COLOUR (question) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourb"))
                {
                    LOAD_COLOUR (answer)
                        LOAD_COLOUR (quote)
                        LOAD_COLOUR (quote_text)
                        LOAD_COLOUR (immtalk_text)
                        LOAD_COLOUR (immtalk_type) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourc"))
                {
                    LOAD_COLOUR (info)
                        LOAD_COLOUR (tell)
                        LOAD_COLOUR (reply)
                        LOAD_COLOUR (gtell_text)
                        LOAD_COLOUR (gtell_type) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourd"))
                {
                    LOAD_COLOUR (room_title)
                        LOAD_COLOUR (room_text)
                        LOAD_COLOUR (room_exits)
                        LOAD_COLOUR (room_things)
                        LOAD_COLOUR (prompt) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Coloure"))
                {
                    LOAD_COLOUR (fight_death)
                        LOAD_COLOUR (fight_yhit)
                        LOAD_COLOUR (fight_ohit)
                        LOAD_COLOUR (fight_thit)
                        LOAD_COLOUR (fight_skill) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourf"))
                {
                    LOAD_COLOUR (wiznet)
                        LOAD_COLOUR (say)
                        LOAD_COLOUR (say_text)
                        LOAD_COLOUR (tell_text)
                        LOAD_COLOUR (reply_text) fMatch = TRUE;
                    break;
                }
                if (!str_cmp (word, "Colourg"))
                {
                    LOAD_COLOUR (auction_text)
                        LOAD_COLOUR (gossip_text)
						LOAD_COLOUR (music_text)
                        LOAD_COLOUR (question_text)
                        LOAD_COLOUR (answer_text) fMatch = TRUE;
                    break;
                }
	    KEY("Comm",		ch->comm,		fread_flag( fp ) ); 
          
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Dam",		ch->damroll,		fread_number( fp ) );
	    KEYS( "Deity",	ch->pcdata->deity_name,	fread_string( fp ) );
	    KEYS( "Description",	ch->description,	fread_string( fp ) );
	    KEYS( "Desc",	ch->description,	fread_string( fp ) );
	    KEY( "Dfle",	ch->pcdata->dflee,	fread_number( fp ) );
	    if (!str_cmp( word, "Dupes"))
            {
                if (dcount >= MAX_DUPES)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }
 
                ch->pcdata->dupes[dcount]        = fread_string(fp);
                dcount++;
                fMatch = TRUE;
            }
	    break;

	case 'E':
		KEY( "Exbit1",        ch->exbit1_flags,               fread_flag( fp ) );
	    if ( !str_cmp( word, "End" ) )
	    {
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

		percent = UMIN(percent,100);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
        	    ch->hit	+= (ch->max_hit - ch->hit) * percent / 100;
        	    ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
        	    ch->move    += (ch->max_move - ch->move)* percent / 100;
    		}
		return;
	    }
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    break;

	case 'F':
	    KEY( "Favor",	ch->pcdata->favor,	fread_number( fp ) );
	    if (!str_cmp( word, "Forge"))
            {
                if (fcount >= MAX_FORGET)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }
 
                ch->pcdata->forget[fcount]        = fread_string(fp);
                fcount++;
                fMatch = TRUE;
            }
	    break;

	case 'G':
	    KEY( "Ghos",	ch->ghost_level,	fread_number( fp ) );
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
            if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr"))
            {
                int gn;
                char *temp;
 
                temp = fread_word( fp ) ;
                gn = group_lookup(temp);
                /* gn    = group_lookup( fread_word( fp ) ); */
                if ( gn < 0 )
                {
                    fprintf(stderr,"%s",temp);
                    bug( "Fread_char: unknown group. ", 0 );
                }
                else
		    gn_add(ch,gn);
                fMatch = TRUE;
            }
	    break;

	case 'H':
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Hit",		ch->hitroll,		fread_number( fp ) );
	    KEYL( "Home",	ch->home, fread_string( fp ), home_lookup );
	    KEY( "Homn",	ch->home_none,		fread_number( fp ) );

	    if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
            {
                ch->pcdata->perm_hit	= fread_number( fp );
                ch->pcdata->perm_mana   = fread_number( fp );
                ch->pcdata->perm_move   = fread_number( fp );
                fMatch = TRUE;
                break;
            }
      
	    break;

	case 'I':
	    KEY( "Id",		ch->id,			fread_number( fp ) );
	    KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Inco",	ch->incog_level,	fread_number( fp ) );
	    KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "LastLevel",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "Lev",		ch->level,		fread_number( fp ) );
	    KEY( "Levl",	ch->level,		fread_number( fp ) );
            if (!str_cmp(word,"LogO"))
            {
		lastlogoff		= fread_number( fp );
		ch->llogoff		= (time_t) lastlogoff;
		fMatch = TRUE;
		break;
	    }
	    KEYS( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEYS( "LnD",		ch->long_descr,		fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "Mudl",	ch->pcdata->last_mud,	fread_number( fp ) );
	    break;

	case 'N':
	    KEYS( "Name",	ch->name,		fread_string( fp ) );
	    KEY( "New",		ch->newbie,		fread_number( fp ) );
	    KEY( "Note",	ch->pcdata->last_note,	fread_number( fp ) );
	    if (!str_cmp(word,"Not"))
	    {
		ch->pcdata->last_note			= fread_number(fp);
		ch->pcdata->last_idea			= fread_number(fp);
		ch->pcdata->last_penalty		= fread_number(fp);
		ch->pcdata->last_news			= fread_number(fp);
		ch->pcdata->last_changes		= fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    if (!str_cmp(word,"Notb"))
	    {
		ch->pcdata->last_note			= fread_number(fp);
		ch->pcdata->last_idea			= fread_number(fp);
		ch->pcdata->last_penalty		= fread_number(fp);
		ch->pcdata->last_news			= fread_number(fp);
		ch->pcdata->last_changes		= fread_number(fp);
		ch->pcdata->last_weddings		= fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'P':
	    KEYS( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEYS( "Pass",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Plat",	ch->platinum,		fread_number( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Plyd",	ch->played,		fread_number( fp ) );
	    KEY( "Points",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Pnts",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Pos",		ch->position,		fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prac",	ch->practice,		fread_number( fp ) );
            KEYS( "Prompt",      ch->prompt,             fread_string( fp ) );
 	    KEYS( "Prom",	ch->prompt,		fread_string( fp ) );
	    KEY( "Pkil",	ch->pcdata->pkills,	fread_number( fp ) );
	    KEY( "Pdea",	ch->pcdata->pdeath,	fread_number( fp ) );
	    break;

	case 'Q':
		KEY( "QuestPnts",	ch->questpoints,        fread_number( fp ) );
		KEY( "QuestNext",	ch->nextquest,          fread_number( fp ) );
	    KEY( "Qps",		ch->qps,		fread_number( fp ) );
	    KEY( "Qon",		ch->can_aquest,		fread_number( fp ) );
	    KEYS( "Qob",		ch->pcdata->lquest_obj,	fread_string( fp ) );
	    KEYS( "Qmo",		ch->pcdata->lquest_mob,	fread_string( fp ) );
	    KEYS( "Qar",		ch->pcdata->lquest_are,	fread_string( fp ) );
	    KEY( "Qvn",		ch->pcdata->quest_mob,	fread_number( fp ) );

	case 'R':
	    KEYL( "Race",        ch->race,	fread_string( fp ), race_lookup );
	    KEY( "Reca",	ch->pcdata->recall, fread_number( fp ) );

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Save",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Scro",	ch->lines,		fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "ShBy",	ch->shielded_by,	fread_flag( fp ) );
	    KEY( "ShieldedBy",	ch->shielded_by,	fread_flag( fp ) );
	    KEYS( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
	    KEYS( "ShD",		ch->short_descr,	fread_string( fp ) );
	    KEY( "Sec",         ch->pcdata->security,	fread_number( fp ) );	/* OLC */
            KEY( "Silv",        ch->silver,             fread_number( fp ) );
	    KEY( "Spirit",	ch->spirit,		fread_number( fp ) );

	    if ( !str_cmp( word, "Stance" ) )
	    {
		ch->stance[0]   = fread_number( fp );
		ch->stance[1]   = fread_number( fp );
		ch->stance[2]   = fread_number( fp );
		ch->stance[3]   = fread_number( fp );
		ch->stance[4]   = fread_number( fp );
		ch->stance[5]   = fread_number( fp );
		ch->stance[6]   = fread_number( fp );
		ch->stance[7]   = fread_number( fp );
		ch->stance[8]   = fread_number( fp );
		ch->stance[9]   = fread_number( fp );
		ch->stance[10]  = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;
		char *temp;

		value = fread_number( fp );
		temp = fread_word( fp ) ;
		sn = skill_lookup(temp);
		/* sn    = skill_lookup( fread_word( fp ) ); */
		if ( sn < 0 )
		{
		    fprintf(stderr,"%s",temp);
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		    ch->pcdata->learned[sn] = value;
		fMatch = TRUE;
	    }

	    break;

	case 'T':
	    KEY( "Tier",	ch->pcdata->tier,	fread_number( fp ) );
	    KEY( "TrueSex",     ch->pcdata->true_sex,  	fread_number( fp ) );
	    KEY( "TSex",	ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "Trai",	ch->train,		fread_number( fp ) );
	    KEY( "Trust",	ch->trust,		fread_number( fp ) );
	    KEY( "Tru",		ch->trust,		fread_number( fp ) );

	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		ch->pcdata->title = fread_string( fp );
    		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    KEY( "Version",     ch->version,		fread_number ( fp ) );
	    KEY( "Vers",	ch->version,		fread_number ( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEYS( "Whod",	ch->pcdata->who_descr,	fread_string( fp ) );
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wizn",	ch->wiznet,		fread_flag( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    bug( "Fread_char: no match.", 0 );
	    bug( word, 0 );
	    fread_to_eol( fp );
	}
    }
    if( ch->pcdata->deity_name != NULL )
	ch->pcdata->deity = get_deity( ch->pcdata->deity_name );
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
    	    KEY( "Act",		pet->act,		fread_flag(fp));
    	    KEY( "AfBy",	pet->affected_by,	fread_flag(fp));
    	    KEY( "Alig",	pet->alignment,		fread_number(fp));
    	    
    	    if (!str_cmp(word,"ACs"))
    	    {
    	    	int i;
    	    	
    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }
    	    
    	    if (!str_cmp(word,"AffD"))
    	    {
    	    	AFFECT_DATA *paf;
    	    	int sn;
    	    	
    	    	paf = new_affect();
    	    	
    	    	sn = skill_lookup(fread_word(fp));
    	     	if (sn < 0)
    	     	    bug("Fread_char: unknown skill.",0);
    	     	else
    	     	   paf->type = sn;
    	     	   
    	     	paf->level	= fread_number(fp);
    	     	paf->duration	= fread_number(fp);
    	     	paf->modifier	= fread_number(fp);
    	     	paf->location	= fread_number(fp);
    	     	paf->bitvector	= fread_number(fp);
    	     	paf->next	= pet->affected;
    	     	pet->affected	= paf;
    	     	fMatch		= TRUE;
    	     	break;
    	    }

            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                   paf->type = sn;
 
		paf->where	= fread_number(fp);
                paf->level      = fread_number(fp);
                paf->duration   = fread_number(fp);
                paf->modifier   = fread_number(fp);
                paf->location   = fread_number(fp);
                paf->bitvector  = fread_number(fp);
                paf->next       = pet->affected;
                pet->affected   = paf;
                fMatch          = TRUE;
                break;
            }
    	     
    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'C':
             KEYL( "Clan",       pet->clan,       fread_string(fp), clan_lookup);
    	     KEY( "Comm",	pet->comm,		fread_flag(fp));
    	     break;
    	     
    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEYS( "Desc",	pet->description,	fread_string(fp));
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
		    percent = UMIN(percent,100);
    		    pet->hit	+= (pet->max_hit - pet->hit) * percent / 100;
        	    pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
        	    pet->move   += (pet->max_move - pet->move)* percent / 100;
    		}
    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;
    	     
    	 case 'G':
    	     KEY( "Gold",	pet->gold,		fread_number(fp));
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
    	     	pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
    	     KEYS( "LnD",	pet->long_descr,	fread_string(fp));
	     KEY( "LogO",	lastlogoff,		fread_number(fp));
    	     break;
    	     
    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
    	     break;
    	     
    	case 'P':
    	     KEY( "Plat",	pet->platinum,		fread_number(fp));
    	     KEY( "Pos",	pet->position,		fread_number(fp));

	if ( !str_cmp( word, "PetStance" ) )
	{
	    pet->stance[0]   = fread_number( fp );
	    pet->stance[1]   = fread_number( fp );
	    pet->stance[2]   = fread_number( fp );
	    pet->stance[3]   = fread_number( fp );
	    pet->stance[4]   = fread_number( fp );
	    pet->stance[5]   = fread_number( fp );
	    pet->stance[6]   = fread_number( fp );
	    pet->stance[7]   = fread_number( fp );
	    pet->stance[8]   = fread_number( fp );
	    pet->stance[9]   = fread_number( fp );
	    pet->stance[10]  = fread_number( fp );
	    fMatch = TRUE;
	    break;
	}
    	     break;
    	     
	case 'R':
    	    KEYL( "Race",	pet->race, fread_string(fp), race_lookup);
    	    break;
 	    
    	case 'S' :
    	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEYS( "ShD",		pet->short_descr,	fread_string(fp));
    	    KEY( "ShBy",	pet->shielded_by,	fread_flag(fp));
            KEY( "Silv",        pet->silver,            fread_number( fp ) );
    	    break;
    	    
    	if ( !fMatch )
    	{
    	    bug("Fread_pet: no match.",0);
    	    fread_to_eol(fp);
    	}
    	
    	}
    }
}

extern	OBJ_DATA	*obj_free;

void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
        int vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum )  == NULL )
	{
            bug( "Fread_obj: bad vnum %d.", vnum );
	    obj = create_object(get_obj_index(OBJ_VNUM_BAG),-1);
	    new_format = TRUE;
	}
        else
	{
	    obj = create_object(get_obj_index(vnum),-1);
	    new_format = TRUE;
	}
	    
    }

    if (obj == NULL)  /* either not found or old style */
    {
    	obj = new_obj();
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );
		obj->questowner 	= str_dup( "" );
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if (!str_cmp(word,"AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_obj: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_obj: unknown skill.",0);
                else
                    paf->type = sn;
 
		paf->where	= fread_number( fp );
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = obj->affected;
                obj->affected   = paf;
                fMatch          = TRUE;
                break;
            }
	    break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEYS( "Description",	obj->description,	fread_string( fp ) );
	    KEYS( "Desc",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    free_obj(obj);
		    return;
		}
		else
	        {
		    if ( !fVnum )
		    {
			free_obj( obj );
			obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
		    }

		    if (!new_format)
		    {
		    	obj->next	= object_list;
		    	object_list	= obj;
		    	obj->pIndexData->count++;
		    }

		    if (!obj->pIndexData->new_format 
		    && obj->item_type == ITEM_ARMOR
		    &&  obj->value[1] == 0)
		    {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new)
		    {
			int wear;
			
			wear = obj->wear_loc;
			extract_obj(obj);

			obj = create_object(obj->pIndexData,0);
			obj->wear_loc = wear;
		    }
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
			obj_to_char( obj, ch );
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
		    if (obj->pIndexData->vnum == OBJ_VNUM_QDIAMOND)
		    {
			ch->platinum++;
			extract_obj( obj );
		    }
		    return;
		}
	    }
	    break;

	case 'I':
	    if ( !str_cmp( word, "Inscribed"))
	    {
		obj->inscribed = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }
	    KEYS( "Inspname",	obj->spell_name,	fread_string( fp ) );
	    KEY( "Inspcount",	obj->spell_count,	fread_number( fp ) );
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEYS( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    if ( !str_cmp( word,"Oldstance" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;
		    
	case 'Q':
	    KEY( "Quest",	obj->quest,		fread_number( fp ) );
	    break;

	case 'S':
	    KEYS( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEYS( "ShD",		obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
    if (obj->pIndexData->vnum == OBJ_VNUM_QDIAMOND)
    {
	ch->platinum++;
	extract_obj( obj );
	return;
    }
    if (IS_OBJ_STAT(obj,ITEM_QUEST) && !IS_IMMORTAL(ch))
    {
	if (!global_gquest)
	{
	REMOVE_BIT(obj->extra_flags,ITEM_QUEST);
	    obj->quest = 1;
	}
    }
    if (obj->pIndexData->vnum == OBJ_VNUM_PASSBOOK)
    {
	if (!is_othcomp(obj->value[0], obj->value[1], obj->name))
	extract_obj( obj );
    }

}



