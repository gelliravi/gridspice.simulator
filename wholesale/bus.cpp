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

#include "../powerflow/node.h"


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
                                                                // 3 = ref, 4 = isolated (feeder)
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
			PT_int16, "IFHEADER", PADDR(ifheader),	// if connected with distribution network
			PT_char1024, "HEADER", PADDR(header_name), // the name of header node of distribution network
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
	OBJECT *obj = OBJECTHDR(this);
	bus *tempbus;
	
	tempbus = OBJECTDATA(obj,bus);
	unsigned int bus_num = tempbus->BUS_I;
	

	if (tempbus->ifheader == 1)
	{
		//gl_warning("Start to work with feeder\n");
		//printf("This is node %d\n",bus_num);
		// Find the object
		OBJECT *node_head = gl_get_object(tempbus->header_name);
		//printf("find the object\n");

		//Lock the object
		LOCK_OBJECT(node_head);
		//printf("lock the object\n");

		//get the power value
		node *head_node;
		head_node = OBJECTDATA(node_head,node);
		complex power_out = head_node->powerA;
		//printf("voltage A %f+j%f\n",head_node->voltageA.Re(),head_node->voltageA.Im());

		// update the object
		tempbus->PD += power_out.Re();
		tempbus->QD += power_out.Im();
		

		// running the OPF
		if (solver_matpower() == 1)
		{
			GL_THROW("OPF Failed at bus for distributon network");
		}
		
		// update the voltage
	
		// voltage A
		head_node->voltageA.SetPolar(tempbus->VM,tempbus->VA+0*PI/180);

		// voltage B
		head_node->voltageB.SetPolar(tempbus->VM,tempbus->VA+120*PI/180);

		// voltage C
		head_node->voltageC.SetPolar(tempbus->VM,tempbus->VA+240*PI/180);
	
		UNLOCK_OBJECT(node_head);

	}	
	

	if (tempbus->BUS_TYPE == 3) //ref bus
	{
		//printf("Bus number %d\n", tempbus->BUS_I);
		if (solver_matpower() == 1)
		{
			GL_THROW("OPF Failed at bus");
		}
	}
		

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
