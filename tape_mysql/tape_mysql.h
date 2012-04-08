/* $Id: tape_mysql.h 683 2008-06-18 20:16:29Z d3g637 $ */

#ifndef _tape_mysql_H
#define _tape_mysql_H

#include <stdarg.h>
#include "gridlabd.h"



/* tape global controls */
static char timestamp_format[32]="%Y-%m-%d %H:%M:%S";
typedef enum {VT_INTEGER, VT_DOUBLE, VT_STRING} VARIABLETYPE;
typedef enum {TS_INIT, TS_OPEN, TS_DONE, TS_ERROR} TAPESTATUS;
typedef enum {FT_FILE, FT_ODBC, FT_MEMORY} FILETYPE;
typedef enum {SCREEN, EPS, GIF, JPG, PDF, PNG, SVG} PLOTFILE;

typedef struct s_tape_operations {
	int (*open)(void *my, char *fname, char *flags);
	char *(*read)(void *my,char *buffer,unsigned int size);
	int (*write)(void *my, char *timestamp, char *value);
	int (*rewind)(void *my);
	void (*close)(void *my);
} TAPEOPS;

typedef struct s_tape_funcs {
	char256 mode;
	void *hLib;
	TAPEOPS *player;
	TAPEOPS *shaper;
	TAPEOPS *recorder;
	TAPEOPS *collector;
	TAPEOPS *histogram;
	struct s_tape_funcs *next;
} TAPEFUNCS;

CDECL TAPEFUNCS *get_ftable(char *mode);

PROPERTY *link_properties(OBJECT *obj, char *property_list);

typedef struct {
	char *name;
	VARIABLETYPE type;
	void *addr;
	double min, max;
} VARMAP;

typedef struct s_recobjmap {
	OBJECT *obj;
	PROPERTY prop; // must be an instance
	struct s_recobjmap *next;
} RECORDER_MAP;

/*** DO NOT DELETE THE NEXT LINE ***/
//NEWCLASSINC



/* optional exports */
#ifdef OPTIONAL

/* TODO: define this function to enable checks routine */
EXPORT int check(void);

/* TODO: define this function to allow direct import of models */
EXPORT int import_file(char *filename);

/* TODO: define this function to allow direct export of models */
EXPORT int export_file(char *filename);

/* TODO: define this function to allow export of KML data for a single object */
EXPORT int kmldump(FILE *fp, OBJECT *obj);
#endif

#endif
