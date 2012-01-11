/** $Id: gen.cpp 858 2008-08-28 21:06:13Z d3g637 $
	@file gen.cpp
	@defgroup gen Template for a new object class
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

#include "gen.h"

CLASS *gen::oclass = NULL;
gen *gen::defaults = NULL;

#ifdef OPTIONAL
/* TODO: define this to allow the use of derived classes */
CLASS *PARENTgen::pclass = NULL;
#endif

/* TODO: remove passes that aren't needed */
static PASSCONFIG passconfig = PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN;

/* TODO: specify which pass the clock advances */
static PASSCONFIG clockpass = PC_BOTTOMUP;

/* Class registration is only called once to register the class with the core */
gen::gen(MODULE *module)
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
		oclass = gl_register_class(module,"gen",sizeof(gen),passconfig);
		if (oclass==NULL)
			GL_THROW("unable to register object class implemented by %s", __FILE__);

		if (gl_publish_variable(oclass,
			/* TODO: add your published properties here */
                        PT_int16, "GEN_BUS", PADDR(GEN_BUS),    // bus number
                        PT_double, "PG[MW]", PADDR(PG),         // real power output
                        PT_double, "QG[MVAr]", PADDR(QG),       // reactive power output
                        PT_double, "QMAX[MVAr]", PADDR(QMAX),   // maximum reactive power output
                        PT_double, "QMIN[MVAr]", PADDR(QMIN),   // minimum reactive power output
                        PT_double, "VG", PADDR(VG),             // voltage magnitude setpoint
                        PT_double, "MBASE", PADDR(MBASE),       // total MVA base of machine, defaults to baseMVA
                        PT_double, "GEN_STATUS", PADDR(GEN_STATUS),//machine status
                                                                // >0 = machine in-service
                                                                // <=0 = machine out-of-service
                        PT_double, "PMAX[MW]", PADDR(PMAX),      // maximum real power output
                        PT_double, "PMIN[MW]", PADDR(PMIN),      // minimum real power output
                        PT_double, "PC1[MW]", PADDR(PC1),       // lower real power output of PQ capability curve
                        PT_double, "PC2[MW]", PADDR(PC2),       // upper real power output of PQ capability curve
                        PT_double, "QC1MIN[MVAr]", PADDR(QC1MIN),       // minimum reactive power output at PC1
                        PT_double, "QC1MAX[MVAr]", PADDR(QC1MAX),       // maximum reactive power output at PC1
			PT_double, "QC2MIN[MVAr]", PADDR(QC2MIN),       // minimum reactive power output at PC2
                        PT_double, "QC2MAX[MVAr]", PADDR(QC2MAX),       // maximum reactive power output at PC2
                        PT_double, "RAMP_AGC", PADDR(RAMP_AGC),         // ramp rate for load following/AGC
                        PT_double, "RAMP_10[MW]", PADDR(RAMP_10),           // ramp rate for 10 min reserves
                        PT_double, "RAMP_30[MW]", PADDR(RAMP_30),           // ramp rate for 30 min reserves
                        PT_double, "RAMP_Q[MW]", PADDR(RAMP_Q),           // ramp rate for reactive power
                        PT_double, "APF", PADDR(APF),                   // area participation factor
                        NULL)<1) GL_THROW("unable to publish properties in %s",__FILE__);
		defaults = this;
		memset(this,0,sizeof(gen));
		/* TODO: set the default values of all properties here */
	}
}

/* Object creation is called once for each object that is created by the core */
int gen::create(void)
{
	memcpy(this,defaults,sizeof(gen));
	/* TODO: set the context-free initial value of properties, such as random distributions */
	return 1; /* return 1 on success, 0 on failure */
}

/* Object initialization is called once after all object have been created */
int gen::init(OBJECT *parent)
{
	/* TODO: set the context-dependent initial value of properties */
	return 1; /* return 1 on success, 0 on failure */
}

/* Presync is called when the clock needs to advance on the first top-down pass */
TIMESTAMP gen::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement pre-topdown behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Sync is called when the clock needs to advance on the bottom-up pass */
TIMESTAMP gen::sync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement bottom-up behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP gen::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement post-topdown behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////

EXPORT int create_gen(OBJECT **obj)
{
	try
	{
		*obj = gl_create_object(gen::oclass);
		if (*obj!=NULL)
			return OBJECTDATA(*obj,gen)->create();
	}
	catch (char *msg)
	{
		gl_error("create_gen: %s", msg);
	}
	return 0;
}

EXPORT int init_gen(OBJECT *obj, OBJECT *parent)
{
	try
	{
		if (obj!=NULL)
			return OBJECTDATA(obj,gen)->init(parent);
	}
	catch (char *msg)
	{
		gl_error("init_gen(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return 0;
}

EXPORT TIMESTAMP sync_gen(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	gen *my = OBJECTDATA(obj,gen);
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
		gl_error("sync_gen(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return TS_INVALID;
}
