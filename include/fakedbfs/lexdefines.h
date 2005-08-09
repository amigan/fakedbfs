/* Lex stuff */
#ifndef YYSTYPE
typedef union {
	char *string;
	int number;
} yystype;
#define YYSTYPE yystype
#define YYSTYPE_IS_TRIVIAL 1
#endif
YYSTYPE yylval;
