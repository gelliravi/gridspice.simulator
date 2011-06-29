/** $Id: solver_matpower.h 683 2008-06-18 20:16:29Z d3g637 $
	@file solver_matpower.h
	@addtogroup solver_matpower
	@ingroup MODULENAME

 @{
 **/

#ifndef _solver_matpower_H
#define _solver_matpower_H

#include <stdarg.h>
#include "gridlabd.h"

class solver_matpower {
private:
	/* TODO: put private variables here */
protected:
	/* TODO: put unpublished but inherited variables */
public:
	/* TODO: put published variables here */
public:
	/* required implementations */
	solver_matpower(MODULE *module);
	int create(void);
	int init(OBJECT *parent);
	TIMESTAMP presync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP sync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP postsync(TIMESTAMP t0, TIMESTAMP t1);
        bool enabled;
public:
	static CLASS *oclass;
	static solver_matpower *defaults;
#ifdef OPTIONAL
	static CLASS *pclass; /**< defines the parent class */
	TIMESTAMP plc(TIMESTAMP t0, TIMESTAMP t1); /**< defines the default PLC code */
#endif
};

#endif

/**@}*/
