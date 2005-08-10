/* Lex stuff */
/* $Amigan: fakedbfs/include/lexdefines.h,v 1.2 2005/08/10 00:13:42 dcp1990 Exp $ */
#ifndef YYSTYPE
typedef union {
	char *string;
	int number;
} yystype;
#define YYSTYPE yystype
#define YYSTYPE_IS_TRIVIAL 1
#endif
YYSTYPE yylval;
