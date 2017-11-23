/***************************************************************************
 *  File: olc_act.c                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14k, bet 50m66, bet 100k

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.

*/


int advatoi args ((const char *s));
int parsebet args ((const int currentbet, const char *argument));

int advatoi (const char *s)
/*
  util function, converts an 'advanced' ASCII-number-string into a number.
  Used by parsebet() but could also be used by do_give or do_wimpy.

  Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not just
  numbers. The letters multiply whatever is left of them by 1,000 and
  1,000,000 respectively. Example:

  14k = 14 * 1,000 = 14,000
  23m = 23 * 1,000,0000 = 23,000,000

  If any digits follow the 'k' or 'm', the are also added, but the number
  which they are multiplied is divided by ten, each time we get one left. This
  is best illustrated in an example :)

  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.

*/

{

/* the pointer to buffer stuff is not really necessary, but originally I
   modified the buffer, so I had to make a copy of it. What the hell, it 
   works:) (read: it seems to work:)
*/

  char string[MAX_INPUT_LENGTH]; /* a buffer to hold a copy of the argument */
  char *stringptr = string; /* a pointer to the buffer so we can move around */
  char tempstring[2];       /* a small temp buffer to pass to atoi*/
  int number = 0;           /* number to be returned */
  int multiplier = 0;       /* multiplier used to get the extra digits right */


  strcpy (string,s);        /* working copy */

  while ( isdigit (*stringptr)) /* as long as the current character is a digit */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      number = (number * 10) + atoi (tempstring); /* add to current number */
      stringptr++;                                /* advance */
  }

  switch (UPPER(*stringptr)) {
      case 'K'  : multiplier = 1000;    number *= multiplier; stringptr++; break;
      case 'M'  : multiplier = 1000000; number *= multiplier; stringptr++; break;
      case '\0' : break;
      default   : return 0; /* not k nor m nor NUL - return 0! */
  }

  while ( isdigit (*stringptr) && (multiplier > 1)) /* if any digits follow k/m, add those too */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      multiplier = multiplier / 10;  /* the further we get to right, the less are the digit 'worth' */
      number = number + (atoi (tempstring) * multiplier);
      stringptr++;
  }

  if (*stringptr != '\0' && !isdigit(*stringptr)) /* a non-digit character was found, other than NUL */
    return 0; /* If a digit is found, it means the multiplier is 1 - i.e. extra
                 digits that just have to be ignore, liked 14k4443 -> 3 is ignored */


  return (number);
}


int parsebet (const int currentbet, const char *argument)
{

  int newbet = 0;                /* a variable to temporarily hold the new bet */
  char string[MAX_INPUT_LENGTH]; /* a buffer to modify the bet string */
  char *stringptr = string;      /* a pointer we can move around */

  strcpy (string,argument);      /* make a work copy of argument */


  if (*stringptr)               /* check for an empty string */
  {

    if (isdigit (*stringptr)) /* first char is a digit assume e.g. 433k */
      newbet = advatoi (stringptr); /* parse and set newbet to that value */

    else
      if (*stringptr == '+') /* add ?? percent */
      {
        if (strlen (stringptr) == 1) /* only + specified, assume default */
          newbet = (currentbet * 125) / 100; /* default: add 25% */
        else
          newbet = (currentbet * (100 + atoi (++stringptr))) / 100; /* cut off the first char */
      }
      else
        {
        printf ("considering: * x \n\r");
        if ((*stringptr == '*') || (*stringptr == 'x')) /* multiply */ {
          if (strlen (stringptr) == 1) /* only x specified, assume default */
            newbet = currentbet * 2 ; /* default: twice */
          else /* user specified a number */
            newbet = currentbet * atoi (++stringptr); /* cut off the first char */
        }
  }
  }
  return newbet;        /* return the calculated bet */
}

