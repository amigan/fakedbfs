%{
/* $Amigan: fakedbfs/libfakedbfs/dbspec.l,v 1.2 2005/08/10 00:13:42 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include <fakedbfs.h>
#include "dbspec.h"
#include <lexdefines.h>
RCSID("$Amigan: fakedbfs/libfakedbfs/dbspec.l,v 1.2 2005/08/10 00:13:42 dcp1990 Exp $")
int chrcnt = 0, lincnt = 1; /* not thread-safe, I know */
%}
%%
.			++chrcnt; REJECT;
ENUM			return ENUM;
ALLSUB			return ALLSUB;
NOSUB			return NOSUB;
[Aa][Ss]		return AS;
sameas			return SAMEAS;
self			return SELF;
string			return STRINGDT;
number			return NUMBERDT;
image			return IMAGEDT;
binary			return BINARYDT;
fc			return FC;
\.sub			return DOTSUB;
\=			return EQUALS;
[\"\'\`]		return QUOTE;
\<			return LTHAN;
\>			return GTHAN;
\{			return OBRACE;
\}			return CBRACE;
\(			return OPAR;
\)			return CPAR;
\,			return COMMA;
"/*"			{
				register int c;
				for ( ; ; )
				{
					while ( (c = input()) != '*' && c != EOF );    /* eat up text of comment */

					if ( c == '*' )
					{
						while ( (c = input()) == '*' );
						if ( c == '/' )
							break;    /* found the end */
					}

					if ( c == EOF )
					{
						fprintf(stderr,  "EOF in comment" );
						break;
					}
				}
			}
[^\"\'\`\n]+		yylval.string = strdup(yytext); return STRING;
[A-Za-z0-9_.-]		yylval.string = strdup(yytext); return UQSTRING;
[A-Za-z0-9_-]		yylval.string = strdup(yytext); return UQSTRING;
;			return SCOLON;
\n			chrcnt = 0; ++lincnt;/* DONOTHING */
[\t ]			/* DONOTHING */
.			/* DO nothing */

