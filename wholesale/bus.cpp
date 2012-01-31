/** $Id: bus.cpp 858 2008-08-28 21:06:13Z d3g637 $
	@file bus.cpp
	@defgroup bus Template for a new object class
	@ingroup MODULENAME

	You can add an object class to a module using the \e add_class
	command:
	<verbatim>
	linux% add_class CLASS
	</verbatim>

	You must be in the module directory to do this.

 **/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
//#include "solver_matpower.h"
#include "bus.h"
#include "gen.h"
#include "line.h"
#include "gen_cost.h"
#include "wholesale.h"

CLASS *bus::oclass = NULL;
bus *bus::defaults = NULL;

#ifdef OPTIONAL
/* TODO: define this to allow the use of derived classes */
CLASS *PARENTbus::pclass = NULL;
#endif

/* TODO: remove passes that aren't needed */
static PASSCONFIG passconfig = PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN;

/* TODO: specify which pass the clock advances */
static PASSCONFIG clockpass = PC_BOTTOMUP;

/* Class registration is only called once to register the class with the core */
bus::bus(MODULE *module)
#ifdef OPTIONAL
/* TODO: include this if you are deriving this from a superclass */
: SUPERCLASS(module)
#endif
{
#ifdef OPTIONAL
	/* TODO: include this if you are deriving this from a superclass */
	pclass = SUPERCLASS::oclass;
#endif
	if (oclass==NULL)
	{
		oclass = gl_register_class(module,"bus",sizeof(bus),passconfig);
		if (oclass==NULL)
			GL_THROW("unable to register object class implemented by %s", __FILE__);
                
                // attributes of bus class. the names follow the MATPOWER Bus Data structure
		if (gl_publish_variable(oclass,
                        PT_int16, "BUS_I", PADDR(BUS_I),        // bus number
                        PT_int16, "BUS_TYPE", PADDR(BUS_TYPE),   // bus type
                                                                // 1 = PQ, 2= PV
                                                                // 3 = ref, 4 = isolated
                        PT_double, "PD[MW]", PADDR(PD),         // real power demand
                        PT_double, "QD[MVAr]", PADDR(QD),       // reactive power demand
                        PT_double, "GS[MW]", PADDR(GS),         // shunt conductance
                        PT_double, "BS[MVAr]", PADDR(BS),       // shunt susceptance
                        PT_int16, "BUS_AREA", PADDR(BUS_AREA),  // area number
                        PT_double, "VM", PADDR(VM),             // voltage magnitude
                        PT_double, "VA", PADDR(VA),             // voltage angle
                        PT_double, "BASE_KV[kV]", PADDR(BASE_KV),   // base voltage
                        PT_int16, "ZONE", PADDR(ZONE),          // lose zone
                        PT_double, "VMAX", PADDR(VMAX),         // maximum voltage magnitude
                        PT_double, "VMIN", PADDR(VMIN),         // minimum voltage magnitude
                        //PT_double, "length[ft]",PADDR(length),
		     /* TODO: add your published properties here */
		    NULL)<1) GL_THROW("unable to publish properties in %s",__FILE__);
		defaults = this;
		memset(this,0,sizeof(bus));
		/* TODO: set the default values of all properties here */
	}
}

/* Object creation is called once for each object that is created by the core */
int bus::create(void)
{
	memcpy(this,defaults,sizeof(bus));
	/* TODO: set the context-free initial value of properties, such as random distributions */
	return 1; /* return 1 on success, 0 on failure */
}

/* Object initialization is called once after all object have been created */
int bus::init(OBJECT *parent)
{
	/* TODO: set the context-dependent initial value of properties */
	return 1; /* return 1 on success, 0 on failure */
}

/* Presync is called when the clock needs to advance on the first top-down pass */
TIMESTAMP bus::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement pre-topdown behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Sync is called when the clock needs to advance on the bottom-up pass */
TIMESTAMP bus::sync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;

	
	unsigned int nbus = 0;
	unsigned int ngen = 0;
	unsigned int nbranch = 0;
	unsigned int ngencost = 0;
	unsigned int nareas = 0;

	
	// collect bus data
		
/*
	OBJECT *temp_obj = NULL;
	bus *list_bus;
	FINDLIST *bus_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"bus",FT_END);

	while (gl_find_next(bus_list,temp_obj)!=NULL)
	{
		temp_obj = gl_find_next(bus_list,temp_obj);
		list_bus = OBJECTDATA(temp_obj,bus);
		//vec_bus.push_back(OBJECTDATA(temp_obj,bus));
		printf("%d\n",list_bus->BUS_TYPE);
        };
        //printf("Total number: %d\n",vec_bus.size());
	printf("=============BUS=============\n");

	gen *list_gen;
	FINDLIST *gen_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"gen",FT_END);
	temp_obj = NULL;
	
	while (gl_find_next(gen_list,temp_obj)!=NULL)
	{
		temp_obj = gl_find_next(gen_list,temp_obj);
		list_gen = OBJECTDATA(temp_obj,gen);
		printf("%d\n",list_gen->GEN_BUS);
        };
        
	printf("=============GEN=============\n");

	line *list_line;
	FINDLIST *line_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"line",FT_END);
	temp_obj = NULL;

	while (gl_find_next(line_list,temp_obj)!=NULL)	
	{
		temp_obj = gl_find_next(line_list,temp_obj);
		list_line = OBJECTDATA(temp_obj,line);
		printf("from %d to %d \n",list_line->F_BUS,list_line->T_BUS);
	}

	printf("=============LINE============\n");

	gen_cost *list_gen_cost;
	FINDLIST *gen_cost_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"gen_cost",FT_END);
	temp_obj = NULL;

	while (gl_find_next(gen_cost_list,temp_obj)!=NULL)
	{
		temp_obj = gl_find_next(gen_cost_list,temp_obj);
		list_gen_cost = OBJECTDATA(temp_obj,gen_cost);
		printf("model %d\n",list_gen_cost->MODEL);
	}

	printf("===========GEN_COST==========\n");
	
	
        // Call solver_matpower

	double rbus[117] = {1,2,3,4,5,6,7,8,9,
		3,2,2,1,1,1,1,1,1,
		0,0,0,0,90,0,100,0,125,
		0,0,0,0,30,0,35,0,50,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,
		0,0,0,0,0,0,0,0,0,
		345,345,345,345,345,345,345,345,345,
		1,1,1,1,1,1,1,1,1,
		1.1,1.1,1.1,1.1,1.1,1.1,1.1,1.1,1.1,
		0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9}; 
	
	double rgen[54] = {1,2,3,
			0,163,85,
			0,0,0,
			300,300,300,
			-300,-300,-300,
			1,1,1,
			100,100,100,
			1,1,1,
			250,300,270,
			10,10,10,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0};
	
	double rbranch[117] = {1,4,5,3,6,7,8,8,9,
		4,5,6,6,7,8,2,9,4,
		0,0.017,0.039,0,0.0119,0.0085,0,0.032,0.01,
		0.0576,0.092,0.17,0.0586,0.1008,0.072,0.0625,0.161,0.085,
		0,0.158,0.358,0,0.209,0.149,0,0.306,0.176,
		250,250,150,300,150,250,250,250,250,
		250,250,150,300,150,250,250,250,250,
		250,250,150,300,150,250,250,250,250,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,
		-360,-360,-360,-360,-360,-360,-360,-360,-360,
		360,360,360,360,360,360,360,360,360};

	double rgencost[21] = {2,2,2,
			1500,2000,3000,
			0,0,0,
			3,3,3,
			0.11,0.085,0.1225,
			5,1.2,1,
			150,600,335};

	double rareas[2] = {1, 2};

	
	nbus = 9;
	ngen = 3;
	nbranch = 9;
	ngencost = 3;
	nareas = 1;	
*/
	//int a = solver_matpower(rbus,nbus,rgen,ngen,rbranch,nbranch,rgencost,ngencost,rareas,nareas);
	int a = solver_matpower();

	/* TODO: implement bottom-up behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP bus::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement post-topdown behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////

EXPORT int create_bus(OBJECT **obj)
{
	try
	{
		*obj = gl_create_object(bus::oclass);
		if (*obj!=NULL)
			return OBJECTDATA(*obj,bus)->create();
	}
	catch (char *msg)
	{
		gl_error("create_bus: %s", msg);
	}
	return 0;
}

EXPORT int init_bus(OBJECT *obj, OBJECT *parent)
{
	try
	{
		if (obj!=NULL)
			return OBJECTDATA(obj,bus)->init(parent);
	}
	catch (char *msg)
	{
		gl_error("init_bus(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return 0;
}

EXPORT TIMESTAMP sync_bus(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	bus *my = OBJECTDATA(obj,bus);
	try
	{
		switch (pass) {
		case PC_PRETOPDOWN:
			t2 = my->presync(obj->clock,t1);
			break;
		case PC_BOTTOMUP:
			t2 = my->sync(obj->clock,t1);
			break;
		case PC_POSTTOPDOWN:
			t2 = my->postsync(obj->clock,t1);
			break;
		default:
			GL_THROW("invalid pass request (%d)", pass);
			break;
		}
		if (pass==clockpass)
			obj->clock = t1;
		return t2;
	}
	catch (char *msg)
	{
		gl_error("sync_bus(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return TS_INVALID;
}
