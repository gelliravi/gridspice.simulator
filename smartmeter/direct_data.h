/** $Id: direct_data.h 683 2008-06-18 20:16:29Z d3g637 $
	@file direct_data.h
	@addtogroup direct_data
	@ingroup MODULENAME

 @{
 **/

#ifndef _direct_data_H
#define _direct_data_H

#include <stdarg.h>
#include "gridlabd.h"
#include "db_access.h"

class direct_data {
private:
	/* TODO: put private variables here */
    TIMESTAMP earliest_time;
    TIMESTAMP latest_time;
    db_access *db;
protected:
	/* TODO: put unpublished but inherited variables */
public:
    complex current_power;
    char32 customer_id;
	/* TODO: put published variables here */
public:
	/* required implementations */
	direct_data(MODULE *module);
	int create(void);
	int init(OBJECT *parent);
	TIMESTAMP presync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP sync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP postsync(TIMESTAMP t0, TIMESTAMP t1);
public:
	static CLASS *oclass;
	static direct_data *defaults;
#ifdef OPTIONAL
	static CLASS *pclass; /**< defines the parent class */
	TIMESTAMP plc(TIMESTAMP t0, TIMESTAMP t1); /**< defines the default PLC code */
#endif
};

#endif

/**@}*/
