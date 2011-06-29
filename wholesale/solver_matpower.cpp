/** $Id: solver_matpower.cpp 858 2008-08-28 21:06:13Z d3g637 $
	@file solver_matpower.cpp
	@defgroup solver_matpower Template for a new object class
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
#include "libopf.h"
#include <stdlib.h>
#include <stdio.h>
#include "matrix.h"
#include "solver_matpower.h"

CLASS *solver_matpower::oclass = NULL;
solver_matpower *solver_matpower::defaults = NULL;

#ifdef OPTIONAL
/* TODO: define this to allow the use of derived classes */
CLASS *PARENTsolver_matpower::pclass = NULL;
#endif

/* TODO: remove passes that aren't needed */
static PASSCONFIG passconfig = PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN;

/* TODO: specify which pass the clock advances */
static PASSCONFIG clockpass = PC_BOTTOMUP;

/* Class registration is only called once to register the class with the core */
solver_matpower::solver_matpower(MODULE *module)
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
		oclass = gl_register_class(module,"solver_matpower",sizeof(solver_matpower),passconfig);
		if (oclass==NULL)
			GL_THROW("unable to register object class implemented by %s", __FILE__);

		if (gl_publish_variable(oclass,
			PT_bool,"gencost",PADDR(enabled),
			NULL)<1) GL_THROW("unable to publish properties in %s",__FILE__);
		defaults = this;
		memset(this,0,sizeof(solver_matpower));
		/* TODO: set the default values of all properties here */
	}
}

/* Object creation is called once for each object that is created by the core */
int solver_matpower::create(void)
{
	memcpy(this,defaults,sizeof(solver_matpower));
	/* TODO: set the context-free initial value of properties, such as random distributions */
	return 1; /* return 1 on success, 0 on failure */
}

/* Object initialization is called once after all object have been created */
int solver_matpower::init(OBJECT *parent)
{
	/* TODO: set the context-dependent initial value of properties */
	return 1; /* return 1 on success, 0 on failure */
}

/* Presync is called when the clock needs to advance on the first top-down pass */
TIMESTAMP solver_matpower::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement pre-topdown behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

mxArray* initArray(double rdata[], int nRow, int nColumn) {
	mxArray* X = mxCreateDoubleMatrix(nRow, nColumn, mxREAL);
	memcpy(mxGetPr(X), rdata, nRow*nColumn*sizeof(double));
	return X;
}

/* Sync is called when the clock needs to advance on the bottom-up pass */
TIMESTAMP solver_matpower::sync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;

	printf("Running Test\n");
        libopfInitialize();
	mxArray* baseMVA = mxCreateDoubleMatrix(1,1,mxREAL);
	*mxGetPr(baseMVA) = 100.0;

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

	mxArray* bus = initArray(rbus, 9, 13);	

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


	mxArray* gen = initArray(rgen, 3, 18);

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

	mxArray* branch = initArray(rbranch, 9, 13);

	double rgencost[21] = {2,2,2,
			1500,2000,3000,
			0,0,0,
			3,3,3,
			0.11,0.085,0.1225,
			5,1.2,1,
			150,600,335};

	mxArray* gencost = initArray(rgencost, 3, 7);

	double rareas[2] = {1, 2};
	mxArray* areas = initArray(rareas, 1, 2);


	mxArray* busout;
	mxArray* genout;
	mxArray* branchout;
	mxArray* f;
	mxArray* success;



	mxArray* plhs[5];
	mxArray* prhs[6];
	plhs[0] = busout;
	plhs[1] = genout;
	plhs[2] = branchout;
	plhs[3] = f;
	plhs[4] = success;

	prhs[0] = baseMVA;
	prhs[1] = bus;
	prhs[2] = gen;
	prhs[3] = branch;
	prhs[4] = areas;
	prhs[5] = gencost;

	mlxOpf(0, plhs, 6, prhs);

	//printf("%d\n", mxGetNumberOfDimensions(busout));

	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP solver_matpower::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement post-topdown behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////

EXPORT int create_solver_matpower(OBJECT **obj)
{
	try
	{
		*obj = gl_create_object(solver_matpower::oclass);
		if (*obj!=NULL)
			return OBJECTDATA(*obj,solver_matpower)->create();
	}
	catch (char *msg)
	{
		gl_error("create_solver_matpower: %s", msg);
	}
	return 0;
}

EXPORT int init_solver_matpower(OBJECT *obj, OBJECT *parent)
{
	try
	{
		if (obj!=NULL)
			return OBJECTDATA(obj,solver_matpower)->init(parent);
	}
	catch (char *msg)
	{
		gl_error("init_solver_matpower(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return 0;
}

EXPORT TIMESTAMP sync_solver_matpower(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	solver_matpower *my = OBJECTDATA(obj,solver_matpower);
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
		gl_error("sync_solver_matpower(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return TS_INVALID;
}
