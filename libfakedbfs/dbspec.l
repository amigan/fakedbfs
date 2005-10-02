%{
/* $Amigan: fakedbfs/libfakedbfs/dbspec.l,v 1.13 2005/10/02 16:43:15 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include "dbspec.h"
#include <lexdefines.h>
#define NODMALLOC 1
#define ISLEX	1
#include <fakedbfs.h>
RCSID("$Amigan: fakedbfs/libfakedbfs/dbspec.l,v 1.13 2005/10/02 16:43:15 dcp1990 Exp $")
int chrcnt = 0, lincnt = 1; /* not thread-safe, I know */
%}
%s comment
%%
.			++chrcnt; REJECT;
<comment>.		{}
en			return EN;
ENUM			return ENUM;
ALLSUB			return ALLSUB;
CATALOGUE		return CATALOGUE_TYPE;
NOSUB			return NOSUB;
[Aa][Ss]		return AS;
sameas			return SAMEAS;
self			return SELF;
string			return STRINGDT;
number			return NUMBERDT;
image			return IMAGEDT;
binary			return BINARYDT;
boolean			return NUMBERDT; /* have to */
real			return REALDT;
fc			return FC;
\.sub			return DOTSUB;
"/*"			BEGIN(comment);
<comment>"*/"		BEGIN(INITIAL);
\=			return EQUALS;
\<			return LTHAN;
\>			return GTHAN;
\{			return OBRACE;
\}			return CBRACE;
\(			return OPAR;
\)			return CPAR;
\,			return COMMA;
<INITIAL>\"[^"]*\"		{
				yylval.string = strdupq(yytext);
#ifdef PARSERDEBUG
				fprintf(stderr, "string %s\n", yylval.string);
#endif
				return STRING;
}
<INITIAL>[A-Za-z0-9_]+		{
				yylval.string = fstrdup(yytext);
#ifdef PARSERDEBUG
				fprintf(stderr, "string %s\n", yylval.string);
#endif
				return UQSTRING;
}
;			return SCOLON;
\n			chrcnt = 0; ++lincnt;/* DONOTHING */
[\t ]			/* DONOTHING */
.			/* DO nothing */
%%
