/* Grammar for db spec files
 * (C)2005, Dan Ponte
 */
/* $Amigan: fakedbfs/libfakedbfs/dbspec.y,v 1.8 2005/08/13 02:48:14 dcp1990 Exp $ */
%include {
#include <sqlite3.h>
#include <stdlib.h>
#include <fakedbfs.h>
#include <string.h>
#include <unistd.h>
RCSID("$Amigan: fakedbfs/libfakedbfs/dbspec.y,v 1.8 2005/08/13 02:48:14 dcp1990 Exp $")
extern int chrcnt, lincnt;
}
%token_type {Toke}
%extra_argument {Heads *heads}
specdef ::= sblocks.
sblocks ::= sblocks eblock.
sblocks ::= eblock.
blockx ::= block.
eblock ::= SCOLON.
eblock ::= blockx SCOLON.
block ::= enumblock.
block ::= catalogueblock.
catalogueblock ::= CATALOGUE_TYPE catname(A) aliasdef(B) OBRACE catelems CBRACE. {
		heads->curcath->name = A.str;
		switch(B.num) {
			case 0:
			heads->curcath->fmtname = A.str;
			break;
			case 1:
			heads->curcath->fmtname = B.str;
			case 2:
			heads->curcath->fmtname = A.str;
			heads->curcath->flags |= CATH_USES_FC;
			break;
		}
		heads->curcath->headelem = heads->catelemhead;
		heads->catelemhead = heads->lastcatel = heads->curcatel = NULL;
	}
catname(A) ::= uqstring(B). {
		A.str = B.str;
		if(find_cathead_by_name(heads->db_cath, B.str) != NULL || find_cathead_by_name(heads->cathead, B.str) != NULL) {
			fdbfs_t *in = (fdbfs_t *)heads->instance;
			ferr(in, die, "Error! catalogue already exists with name %s. ", B.str);
			break;
		}
		heads->curcath = allocz(sizeof(struct CatalogueHead));
		if(heads->cathead == NULL) {
			heads->cathead = heads->curcath;
			heads->lastcath = heads->curcath;
		} else {
			heads->lastcath->next = heads->curcath;
			heads->lastcath = heads->curcath;
		}
	}
headdoer ::= . {
		heads->curenumh->allsubs = heads->allsubelhead;
	}
typename(A) ::= uqstring(B). {
		A.str = B.str;
		if((heads->db_enumh != NULL && find_enumhead_by_name(heads->db_enumh, B.str) != NULL) ||
				(heads->enumhead != NULL && find_enumhead_by_name(heads->enumhead, B.str) != NULL)) {
			fdbfs_t *in = (fdbfs_t *)heads->instance;
			ferr(in, die, "Error! enum already exists with name %s. ", B.str);
			break;
		}
		heads->curenumh = allocz(sizeof(struct EnumHead));
		if(heads->enumhead == NULL) {
			heads->enumhead = heads->curenumh;
			heads->lastenumh = heads->curenumh;
		} else {
			heads->lastenumh->next = heads->curenumh;
			heads->lastenumh = heads->curenumh;
		}
		heads->curenumh->name = B.str;
	}
clearallsub ::= . {
		heads->allsubelhead = heads->lastallsubel = NULL;
	}
arguments ::= ALLSUB clearallsub OPAR allsubelements CPAR.
arguments ::= NOSUB OPAR CPAR.
arguments ::= .
enumelements ::= enumelements COMMA enumelement.
enumelements ::= enumelement.
enumelement(A) ::= string(B). {
		A.enumelem = allocz(sizeof(struct EnumElem));
		A.enumelem->fmtname = B.str;
		A.enumelem->name = normalise(B.str);
		A.enumelem->value = heads->lastevalue++;
		if(heads->enumelemhead == NULL) {
			heads->enumelemhead = A.enumelem;
			heads->lastenumel = A.enumelem;
		} else {
			heads->lastenumel->next = A.enumelem;
			heads->lastenumel = A.enumelem;
		}
	}
allocer(A) ::= . {
		A.enumelem = allocz(sizeof(struct EnumElem));
		if(heads->enumelemhead == NULL) {
			heads->enumelemhead = A.enumelem;
			heads->lastenumel = A.enumelem;
		} else {
			heads->lastenumel->next = A.enumelem;
			heads->lastenumel = A.enumelem;
		}
		heads->lastsubval = heads->lastallsubval = 0;
		heads->subelhead = heads->lastsubel = NULL;
	}
enumelement(A) ::= string(B) allocer(X) OBRACE subelements(C) CBRACE. {
		X.enumelem->fmtname = B.str;
		X.enumelem->name = normalise(B.str);
		X.enumelem->subhead = C.subelem;
		X.enumelem->value = heads->lastevalue++;
		A.enumelem = X.enumelem;
	}

enumelement(A) ::= string(B) AS datatype(C). {
		A.enumelem = allocz(sizeof(struct EnumElem));
		A.enumelem->fmtname = B.str;
		A.enumelem->name = normalise(B.str);
		A.enumelem->other = 1;
		A.enumelem->othertype = C.num;
		A.enumelem->value = heads->lastevalue++;
		if(heads->enumelemhead == NULL) {
			heads->enumelemhead = A.enumelem;
			heads->lastenumel = A.enumelem;
		} else {
			heads->lastenumel->next = A.enumelem;
			heads->lastenumel = A.enumelem;
		}
	}
allsubelements ::= allsubelements COMMA allsubelement.
allsubelements ::= allsubelement.
//allsubelements ::= . {A.subelem = NULL;}
allsubelement(A) ::= subelem(B). {
		if(heads->lastallsubval < ALLSUBSTART)
			heads->lastallsubval = ALLSUBSTART;
		A.subelem = allocz(sizeof(struct EnumSubElem));
		A.subelem->name = (B.num == 0 ? B.str : NULL);
		A.subelem->value = heads->lastallsubval++;
		A.subelem->father = NULL;
		A.subelem->flags |= SUBE_IS_ALLSUB;
		if(B.num == 1) {
			A.subelem->flags |= SUBE_IS_SELF;
			A.subelem->name = "!SELF!";
		} else if(B.num == 2) { /* same as */
			A.subelem->flags |= SUBE_IS_SELF; /*for lack of a better way */
			free(B.str); /* must do when done */
		}
		if(heads->allsubelhead == NULL) {
			heads->allsubelhead = A.subelem;
			heads->lastallsubel = A.subelem;
		} else {
			heads->lastallsubel->next = A.subelem;
			heads->lastallsubel = A.subelem;
		}
	}
subelements ::= subelements COMMA subelement.
subelements ::= subelement.
//subelements(A) ::= . {A.subelem = NULL;}
subelement(A) ::= subelem(B). {
		A.subelem = allocz(sizeof(struct EnumSubElem));
		A.subelem->name = (B.num == 0 ? B.str : NULL);
		A.subelem->value = heads->lastsubval++;
		A.subelem->father = heads->lastenumel;
		if(B.num == 1) {
			A.subelem->flags |= SUBE_IS_SELF;
			A.subelem->name = "!RSELF!";
		} else if(B.num == 2) { /* same as */
			struct EnumElem *from;
			struct EnumSubElem *las;
			fdbfs_t *in = (fdbfs_t*)heads->instance;
			from = find_elem_by_name(heads->enumelemhead, B.str);
			if(from == NULL) {
				dump_enum_elem_list(heads->enumelemhead);
				free(A.subelem);
				heads->err = 1;
				ferr(in, die, "parser: ERROR line %%d, char %%d: cannot find element %s", lincnt, chrcnt, (B.str == NULL ? "(null)" : B.str));
				break;
			}
			heads->lastsubval--; /* undo what we just did */
			las = copy_sub_list(from->subhead, A.subelem,
				heads->lastenumel, &heads->lastsubval);
			if(heads->lastsubel != NULL)
				heads->lastsubel->next = A.subelem;
			heads->lastsubel = las;
			free(B.str); /* must do when done */
		}
		if(heads->subelhead == NULL) {
			heads->subelhead = A.subelem;
			if(B.num != 2)
				heads->lastsubel = A.subelem;
		} else if(B.num != 2) {
			if(heads->lastsubel != NULL)
				heads->lastsubel->next = A.subelem;
			heads->lastsubel = A.subelem;
		}
		A.subelem->next = heads->curenumh->allsubs; /* this should be null if
								none defined */
	}
subelem(A) ::= sedirective(B). {A.num = B.num; A.str = B.str;}
subelem(A) ::= STRING(B). {A.num = 0; A.str = B.str;}
sedirective(A) ::= LTHAN sedir(B) GTHAN. {A.num = B.num; A.str = B.str;}
sedir(A) ::= SELF. {A.num = 1;}
sedir(A) ::= SAMEAS uqstring(B). {
		A.num = 2; A.str = B.str; }
datatype(A) ::= STRINGDT. {
		A.num = (enum DataType)string;
	}
datatype(A) ::= NUMBERDT. {
		A.num = (enum DataType)number;
	}
datatype(A) ::= IMAGEDT. {
		A.num = (enum DataType)image;
	}
datatype(A) ::= BINARYDT. {
		A.num = (enum DataType)binary;
	}
string(A) ::= STRING(B). {
		A.str = B.str;
	}
uqstring(A) ::= UQSTRING(B). {
		A.str = B.str;
	}
catelems ::= catelems COMMA catelem.
catelems ::= catelem.
catelem(A) ::= string(B) aliasdef(C) AS catdatatype(D). {
		A.catelem = allocz(sizeof(*A.catelem));
		A.catelem->name = B.str;
		switch(C.num) {
			case 0:
			A.catelem->alias = B.str; /* if alias == name && !(flags &
							CATE_USES_FC, then don't free alias
						*/
			break;
			case 1:
			A.catelem->alias = C.str;
			case 2:
			A.catelem->alias = B.str;
			A.catelem->flags |= CATE_USES_FC;
			break;
		}
		switch((enum DataType)D.num) {
			case oenum:
			case oenumsub:
			A.catelem->enumptr = D.ehead;
			break;
			default:
			break;
		}
		A.catelem->type = (enum DataType)D.num;
	}
catdatatype(A) ::= datatype(B). {
		A.num = B.num;
	}
catdatatype(A) ::= EN ENUMNAME(B). {
		A.num = (enum DataType)oenum;
		A.ehead = find_enumhead_by_name(heads->enumhead, B.str);
		if(A.ehead == NULL) {
			fdbfs_t *in = (fdbfs_t*)heads->instance;
			ferr(in, die, "Can't find enum named %s. ", B.str);
			break;
		}
	}
catdatatype(A) ::= EN ENUMNAME(B) DOTSUB. {
		A.num = (enum DataType)oenumsub;
		A.ehead = find_enumhead_by_name(heads->enumhead, B.str);
		if(A.ehead == NULL) {
			fdbfs_t *in = (fdbfs_t*)heads->instance;
			ferr(in, die, "Can't find enum named %s. ", B.str);
			break;
		}

	}
aliasdef(A) ::= EQUALS aliasbd(B). {
		A.num = B.num;
	}
aliasdef(A) ::= EQUALS string(B). {
		A.num = 1;
		A.str = B.str;
	}
aliasdef(A) ::= . {A.num = 0;}
aliasbd ::= LTHAN abd GTHAN.
abd(A) ::= FC. { /* first letter uppercase */
		A.num = 2;
	}
enumblock ::= ENUM typename(A) arguments headdoer OBRACE enumelements CBRACE. {
		heads->curenumh->headelem = heads->enumelemhead;
		heads->curenumh->allsubs = heads->allsubelhead;
		/* heads->curenumh->name = A.str; */
		heads->enumelemhead = NULL;
		heads->lastenumel = NULL;
		heads->curenumel = NULL;
		heads->allsubelhead = NULL;
		heads->lastevalue = heads->lastallsubval = 0;
	}

