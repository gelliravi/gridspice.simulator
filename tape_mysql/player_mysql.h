/** $Id: player_mysql.h 683 2008-06-18 20:16:29Z d3g637 $
	@file player_mysql.h
	@addtogroup player_mysql
	@ingroup MODULENAME

 @{
 **/

#ifndef _player_mysql_H
#define _player_mysql_H

#include <stdarg.h>
#include "gridlabd.h"
#include "object.h"
#include "aggregate.h"
#include "memory.h"
#include "tape_mysql.h"
class player_mysql {
private:
	/* TODO: put private variables here */
protected:
	/* TODO: put unpublished but inherited variables */
public:
	char1024 file; /**< the name of the player source */
	char8 filetype; /**< the type of the player source */
	char256 property; /**< the target property */
	int32 loop; /**< the number of time to replay the tape */
	/* private */
	TAPESTATUS status;
	int32 loopnum;
	struct {
		TIMESTAMP ts;
		char32 value;
	} next;
	PROPERTY *target;

	char lasterr[1024];


	double dInterval;
	/* TODO: put published variables here */
public:
	/* required implementations */
	player_mysql(MODULE *module);
	int create(void);
	int init(OBJECT *parent);
	TIMESTAMP presync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP sync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP postsync(TIMESTAMP t0, TIMESTAMP t1);
public:
	static CLASS *oclass;
	static player_mysql *defaults;
#ifdef OPTIONAL
	static CLASS *pclass; /**< defines the parent class */
	TIMESTAMP plc(TIMESTAMP t0, TIMESTAMP t1); /**< defines the default PLC code */
#endif
};

#endif

/**@}*/
