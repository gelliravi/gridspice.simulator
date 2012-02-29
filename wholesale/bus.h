/** $Id: bus.h 683 2008-06-18 20:16:29Z d3g637 $
	@file bus.h
	@addtogroup bus
	@ingroup MODULENAME

 @{
 **/

#ifndef _bus_H
#define _bus_H

#include <stdarg.h>
#include "gridlabd.h"
#include "wholesale.h"

#define MAXSIZE 10000

class bus {
private:
	/* TODO: put private variables here */
protected:
	/* TODO: put unpublished but inherited variables */
public:
	/* TODO: put published variables here */
	//double length;
	static CLASS *oclass;
	static bus *defaults;
	
	//variable
	int 	BUS_I;
	int	BUS_TYPE;
	double 	PD;
	double 	QD;
	double 	GS;
	double 	BS;
	int	BUS_AREA;
	double 	VM;
	double 	VA;
	double	BASE_KV;
	int	ZONE;
	double 	VMAX;
	double 	VMIN;
	// only for the output
	double	LAM_P; //Lagrange multiplier on real power mismatch (u/MW)
	double 	LAM_Q; //Lagrange multiplier on reactive power mismatch (u/MVAr)
	double 	MU_VMAX; //Kuhn-Tucker multiplier on upper voltage limit (u/p.u.)
	double 	MU_VMIN; //Kuhn-Tucker multiplier on lower voltage limit (u/p.u.)

	// feed header	
	int 	ifheader;
	char	header_name[MAXSIZE]; 
	
public:
	/* required implementations */
	bus(MODULE *module);
	int create(void);
	int init(OBJECT *parent);
	TIMESTAMP presync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP sync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP postsync(TIMESTAMP t0, TIMESTAMP t1);


	
#ifdef OPTIONAL
	static CLASS *pclass; /**< defines the parent class */
	
	TIMESTAMP plc(TIMESTAMP t0, TIMESTAMP t1); /**< defines the default PLC code */
#endif

};

#endif

/**@}*/
