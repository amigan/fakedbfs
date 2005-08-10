%{
/* $Amigan: fakedbfs/libfakedbfs/dbspec.l,v 1.3 2005/08/10 03:28:00 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include <fakedbfs.h>
#include "dbspec.h"
#include <lexdefines.h>
RCSID("$Amigan: fakedbfs/libfakedbfs/dbspec.l,v 1.3 2005/08/10 03:28:00 dcp1990 Exp $")
int chrcnt = 0, lincnt = 1; /* not thread-safe, I know */
%}
%s comment
%%
.			++chrcnt; REJECT;
en			return EN;
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
"/*"			BEGIN(comment);
<comment>.		{}
<comment>"*/"		BEGIN(INITIAL);
\=			return EQUALS;
\<			return LTHAN;
\>			return GTHAN;
\{			return OBRACE;
\}			return CBRACE;
\(			return OPAR;
\)			return CPAR;
\,			return COMMA;
\"[^\n\"]+\"	yylval.string = strdup(yytext); return STRING;
[A-Za-z0-9_.-]+		yylval.string = strdup(yytext); return UQSTRING;
[A-Za-z0-9_-]+		yylval.string = strdup(yytext); return UQSTRING;
;			return SCOLON;
\n			chrcnt = 0; ++lincnt;/* DONOTHING */
[\t ]			/* DONOTHING */
.			/* DO nothing */
%%
