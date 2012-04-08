/** $Id: recorder_mysql.h 683 2008-06-18 20:16:29Z d3g637 $
	@file recorder_mysql.h
	@addtogroup recorder_mysql
	@ingroup MODULENAME

 @{
 **/

#ifndef _recorder_mysql_H
#define _recorder_mysql_H
#include <sstream> 
#include <stdarg.h>
#include "tape_mysql.h"

#include "gridlabd.h"



static TIMESTAMP recorder_write(OBJECT *obj);
static int write_recorder(struct recorder_mysql *my, char *ts, char *value);

PROPERTY *link_properties(OBJECT *obj, char *property_list);
class recorder_mysql {
private:
  db_access *db;

	/* TODO: put private variables here */
protected:
	/* TODO: put unpublished but inherited variables */
public:
  int read_properties(OBJECT *obj, PROPERTY *prop, char *buffer, int size);  
	/* TODO: put published variables here */
  char1024 file;
  
	char1024 multifile;
	char1024 multitempfile;
	FILE *multifp, *inputfp;
	int16 multirun_ct;
	char1024 multirun_header;
	char8 filetype;
	int16 format; /* 0=YYYY-MM-DD HH:MM:SS; 1=timestamp */
	double dInterval;
	TIMESTAMP interval;
	int32 limit;
	char1024 property;
	char1024 table;
	char1024 columns;
	PROPERTY *props;
	char1024 plotcommands;
	char32 xdata;

	char32 trigger;
	char8 delim;
	struct {
		TIMESTAMP ts;
		char1024 value;
	} last;
	TAPESTATUS status;
	int32 samples;
	PROPERTY *target;

public:
	/* required implementations */
	recorder_mysql(MODULE *module);
	int create(void);
	int init(OBJECT *parent);
	TIMESTAMP presync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP sync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP postsync(TIMESTAMP t0, TIMESTAMP t1);
public:
	static CLASS *oclass;
	static recorder_mysql *defaults;
#ifdef OPTIONAL
	static CLASS *pclass; /**< defines the parent class */
	TIMESTAMP plc(TIMESTAMP t0, TIMESTAMP t1); /**< defines the default PLC code */
#endif
};

#endif

/**@}*/
