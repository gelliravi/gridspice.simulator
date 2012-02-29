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

class player_mysql {
private:
	/* TODO: put private variables here */
protected:
	/* TODO: put unpublished but inherited variables */
public:
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
