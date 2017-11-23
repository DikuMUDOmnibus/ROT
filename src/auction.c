/***************************************************************************
 *  This file contains auction code developed by Brian Babey, and any      *
 *  communication regarding it should be sent to [bbabey@iname.com]        *
 *  Web Address: http://www.erols.com/bribe/                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "magic.h"
#include "bet.h"

DECLARE_DO_FUN( do_auction );

/* put an item on auction, or see the stats on the current item or bet */
void do_auction (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (argument, arg1);

	if (IS_NPC(ch)) /* NPC can be extracted at any time and thus can't auction! */
		return;

    if (arg1[0] == '\0') {
        if (auction->item != NULL)
        {
            /* show item data here */
            if (auction->bet > 0)
                sprintf (buf, "Current bet on this item is %d gold.\n\r",auction->bet);
            else
                sprintf (buf, "No bets on this item have been received.\n\r");
            send_to_char (buf,ch);
            spell_identify (0, LEVEL_HERO - 1, ch, auction->item, TAR_OBJ_INV); /* uuuh! */
            return;
        }
        else
        {
            send_to_char ("Auction WHAT?\n\r",ch);
            return;
        }
    }

    if (IS_IMMORTAL(ch) && !str_cmp(arg1,"stop")) {
    if (auction->item == NULL)
    {
        send_to_char ("There is no auction going on you can stop.\n\r",ch);
        return;
    }
    else /* stop the auction */
    {
  sprintf (buf,"Sale of %s has been stopped by %s. Item confiscated.", auction->item->short_descr, ch->name );
        talk_auction (buf);
        obj_to_char (auction->item, ch);
        auction->item = NULL;
        if (auction->buyer != NULL) /* return money to the buyer */
        {
            auction->buyer->gold += auction->bet;
            send_to_char ("Your money has been returned.\n\r",auction->buyer);
        }
        return;
    }
    }

    if (!str_cmp(arg1,"bet") ) {
        if (auction->item != NULL)
        {
            int newbet;

            /* make - perhaps - a bet now */
            if (argument[0] == '\0')
            {
                send_to_char ("Bet how much?\n\r",ch);
                return;
            }

            if (ch == auction->seller)
            {
                send_to_char ("You cannot bid on your own items auctioned!\n\r",ch);
                return;
            }

            newbet = parsebet (auction->bet, argument);
            printf ("Bet: %d\n\r",newbet);

            if (newbet < (auction->bet + 100))
            {
                send_to_char ("You must at least bid 100 coins over the current bet.\n\r",ch);
                return;
            }

            if (newbet > ch->gold)
            {
                send_to_char ("You don't have that much money!\n\r",ch);
                return;
            }

            /* the actual bet is OK! */

            /* return the gold to the last buyer, if one exists */
            if (auction->buyer != NULL)
                auction->buyer->gold += auction->bet;

            ch->gold -= newbet; /* substract the gold - important :) */
            auction->buyer = ch;
            auction->bet   = newbet;
            auction->going = 0;
            auction->pulse = PULSE_AUCTION; /* start the auction over again */

            sprintf (buf,"A bet of %d gold has been received on %s.\n\r",newbet,auction->item->short_descr);
            talk_auction (buf);
            return;


        }
        else
        {
            send_to_char ("There isn't anything being auctioned right now.\n\r",ch);
            return;
        }
    }
/* finally... */

   if ((obj = get_obj_carry (ch, arg1)) == NULL)
   {
      send_to_char ("You aren't carrying that item.\n\r", ch);
      return;
   }

    if (auction->item == NULL)
    switch (obj->item_type)
    {

    default:
        act ("You cannot auction $Ts.",ch, NULL, item_type_name (obj->pIndexData), TO_CHAR);
        return;

    case ITEM_WEAPON:
    case ITEM_ARMOR:
    case ITEM_STAFF:
    case ITEM_WAND:
    case ITEM_SCROLL:
        obj_from_char (obj);
        auction->item = obj;
        auction->bet = 0;
        auction->buyer = NULL;
        auction->seller = ch;
        auction->pulse = PULSE_AUCTION;
        auction->going = 0;

        sprintf (buf, "A new item has been received: %s.", obj->short_descr);
        talk_auction (buf);

        return;

    } /* switch */
    else
    {
        act ("Try again later - $p is being auctioned right now!",ch,auction->item,NULL,TO_CHAR);
        return;
    }
}

/* FILE: update.c */

/* the auction update - another very important part*/

void auction_update (void)
{
    char buf[MAX_STRING_LENGTH];

    if (auction->item != NULL)
        if (--auction->pulse <= 0) /* decrease pulse */
        {
            auction->pulse = PULSE_AUCTION;
            switch (++auction->going) /* increase the going state */
            {
            case 1 : /* going once */
            case 2 : /* going twice */
            if (auction->bet > 0)
                sprintf (buf, "%s: going %s for %d.", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"), auction->bet);
            else
                sprintf (buf, "%s: going %s (no bid for this item received yet).", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"));

            talk_auction (buf);
            break;

            case 3 : /* SOLD! */

            if (auction->bet > 0)
            {
                sprintf (buf, "%s sold to %s for %d.",
                    auction->item->short_descr,
                    IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name,
                    auction->bet);
                talk_auction(buf);
                obj_to_char (auction->item,auction->buyer);
                act ("The auctioneer appears before you in a puff of smoke and hands you $p.",
                     auction->buyer,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n, and hands $m $p",
                     auction->buyer,auction->item,NULL,TO_ROOM);

                auction->seller->gold += auction->bet; /* give him the money */

                auction->item = NULL; /* reset item */

            }
            else /* not sold */
            {
                sprintf (buf, "No bets received for %s - object has been removed.",auction->item->short_descr);
                talk_auction(buf);
                act ("The auctioneer appears before you to return $p to you.",
                      auction->seller,auction->item,NULL,TO_CHAR);
                act ("The auctioneer appears before $n to return $p to $m.",
                      auction->seller,auction->item,NULL,TO_ROOM);
                obj_to_char (auction->item,auction->seller);
                auction->item = NULL; /* clear auction */

            } /* else */

            } /* switch */
        } /* if */
} /* func */
