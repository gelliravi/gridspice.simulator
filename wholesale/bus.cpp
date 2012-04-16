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
#include "lock.h"

#define TIME_INTERVAL 900

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
                        //PT_int16, "BUS_TYPE", PADDR(BUS_TYPE),   // bus type
                                                                // 1 = PQ, 2= PV
                                                                // 3 = ref, 4 = isolated (feeder)
			PT_enumeration, "BUS_TYPE", PADDR(BUS_TYPE),
				PT_KEYWORD, "PQ", 1,
				PT_KEYWORD, "PV", 2,
				PT_KEYWORD, "REF", 3,
				PT_KEYWORD, "ISOLATE", 4,
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
			PT_double, "LAM_P", PADDR(LAM_P),
			PT_double, "LAM_Q", PADDR(LAM_Q),
			PT_double, "MU_VMAX", PADDR(MU_VMAX),
			PT_double, "MU_VMIN", PADDR(MU_VMIN),
			PT_int16, "IFHEADER", PADDR(ifheader),	// if connected with distribution network
			PT_char1024, "HEADER", PADDR(header_name), // the name of header node of distribution network
                        //PT_double, "length[ft]",PADDR(length),
			
			// for feeder
			/*
			PT_double, "feeder1_PD", PADDR(feeder1_PD),
			PT_double, "feeder2_PD", PADDR(feeder2_PD),
			PT_double, "feeder3_PD", PADDR(feeder3_PD),
			PT_double, "feeder4_PD", PADDR(feeder4_PD),
			PT_double, "feeder5_PD", PADDR(feeder5_PD),
			PT_double, "feeder6_PD", PADDR(feeder6_PD),
			PT_double, "feeder7_PD", PADDR(feeder7_PD),
			PT_double, "feeder8_PD", PADDR(feeder8_PD),
			PT_double, "feeder9_PD", PADDR(feeder9_PD),
			PT_double, "feeder10_PD", PADDR(feeder10_PD),

			PT_double, "feeder1_QD", PADDR(feeder1_QD),
			PT_double, "feeder2_QD", PADDR(feeder2_QD),
			PT_double, "feeder3_QD", PADDR(feeder3_QD),
			PT_double, "feeder4_QD", PADDR(feeder4_QD),
			PT_double, "feeder5_QD", PADDR(feeder5_QD),
			PT_double, "feeder6_QD", PADDR(feeder6_QD),
			PT_double, "feeder7_QD", PADDR(feeder7_QD),
			PT_double, "feeder8_QD", PADDR(feeder8_QD),
			PT_double, "feeder9_QD", PADDR(feeder9_QD),
			PT_double, "feeder10_QD", PADDR(feeder10_QD),
			*/
			PT_complex,"CVoltageA",PADDR(CVoltageA),// complex voltage: Cvoltage.Mag() = VM, Cvoltage.Arg() = VA;
			PT_complex,"CVoltageB",PADDR(CVoltageB),// complex voltage: Cvoltage.Mag() = VM, Cvoltage.Arg() = VA;
			PT_complex,"CVoltageC",PADDR(CVoltageC),// complex voltage: Cvoltage.Mag() = VM, Cvoltage.Arg() = VA;
			PT_double,"V_nom",PADDR(V_nom),
			PT_complex,"feeder0", PADDR(feeder0),
			PT_complex,"feeder1", PADDR(feeder1),
			PT_complex,"feeder2", PADDR(feeder2),
			PT_complex,"feeder3", PADDR(feeder3),
			PT_complex,"feeder4", PADDR(feeder4),
			PT_complex,"feeder5", PADDR(feeder5),
			PT_complex,"feeder6", PADDR(feeder6),
			PT_complex,"feeder7", PADDR(feeder7),
			PT_complex,"feeder8", PADDR(feeder8),
			PT_complex,"feeder9", PADDR(feeder9),
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
	
	OBJECT *obj = OBJECTHDR(this);
	bus *tempbus;

	tempbus = OBJECTDATA(obj,bus);
/*	
	tempbus->feeder1_PD = tempbus->feeder2_PD = tempbus->feeder3_PD = tempbus->feeder4_PD = tempbus->feeder5_PD = 0;
		tempbus->feeder6_PD = tempbus->feeder7_PD = tempbus->feeder8_PD = tempbus->feeder9_PD = tempbus->feeder10_PD = 0;

		tempbus->feeder1_QD = tempbus->feeder2_QD = tempbus->feeder3_QD = tempbus->feeder4_QD = tempbus->feeder5_QD = 0;
		tempbus->feeder6_QD = tempbus->feeder7_QD = tempbus->feeder8_QD = tempbus->feeder9_QD = tempbus->feeder10_QD = 0;
*/
/*
	tempbus->feeder0 = complex();
	tempbus->feeder1 = complex();
	tempbus->feeder2 = complex();
	tempbus->feeder3 = complex();
	tempbus->feeder4 = complex();
	tempbus->feeder5 = complex();
	tempbus->feeder6 = complex();
	tempbus->feeder7 = complex();
	tempbus->feeder8 = complex();
	tempbus->feeder9 = complex();
*/
	setObjectValue_Double2Complex(obj,"feeder0",0,0);
	setObjectValue_Double2Complex(obj,"feeder1",0,0);
	setObjectValue_Double2Complex(obj,"feeder2",0,0);
	setObjectValue_Double2Complex(obj,"feeder3",0,0);
	setObjectValue_Double2Complex(obj,"feeder4",0,0);
	setObjectValue_Double2Complex(obj,"feeder5",0,0);
	setObjectValue_Double2Complex(obj,"feeder6",0,0);
	setObjectValue_Double2Complex(obj,"feeder7",0,0);
	setObjectValue_Double2Complex(obj,"feeder8",0,0);
	setObjectValue_Double2Complex(obj,"feeder9",0,0);


	

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
	OBJECT *obj = OBJECTHDR(this);
	bus *tempbus;

	tempbus = OBJECTDATA(obj,bus);

	if (tempbus->ifheader == 1)
	{
		LOCK_OBJECT(obj);		
		double sum_PD, sum_QD;
/*
		sum_PD = tempbus->feeder1_PD + tempbus->feeder2_PD + tempbus->feeder3_PD + tempbus->feeder4_PD + tempbus->feeder5_PD;
		sum_PD += tempbus->feeder6_PD + tempbus->feeder7_PD + tempbus->feeder8_PD + tempbus->feeder9_PD + tempbus->feeder10_PD;
		sum_QD = tempbus->feeder1_QD + tempbus->feeder2_QD + tempbus->feeder3_QD + tempbus->feeder4_QD + tempbus->feeder5_QD;
		sum_QD += tempbus->feeder6_QD + tempbus->feeder7_QD + tempbus->feeder8_QD + tempbus->feeder9_QD + tempbus->feeder10_QD;

		tempbus->feeder1_PD = tempbus->feeder2_PD = tempbus->feeder3_PD = tempbus->feeder4_PD = tempbus->feeder5_PD = 0;
		tempbus->feeder6_PD = tempbus->feeder7_PD = tempbus->feeder8_PD = tempbus->feeder9_PD = tempbus->feeder10_PD = 0;

		tempbus->feeder1_QD = tempbus->feeder2_QD = tempbus->feeder3_QD = tempbus->feeder4_QD = tempbus->feeder5_QD = 0;
		tempbus->feeder6_QD = tempbus->feeder7_QD = tempbus->feeder8_QD = tempbus->feeder9_QD = tempbus->feeder10_QD = 0;
*/

		sum_PD = tempbus->feeder0.Re() + tempbus->feeder1.Re() + tempbus->feeder2.Re() + tempbus->feeder3.Re() + tempbus->feeder4.Re() + tempbus->feeder5.Re() + tempbus->feeder6.Re() + tempbus->feeder7.Re() + tempbus->feeder8.Re() + tempbus->feeder9.Re();

		sum_QD = tempbus->feeder0.Im() + tempbus->feeder1.Im() + tempbus->feeder2.Im() + tempbus->feeder3.Im() + tempbus->feeder4.Im() + tempbus->feeder5.Im() + tempbus->feeder6.Im() + tempbus->feeder7.Im() + tempbus->feeder8.Im() + tempbus->feeder9.Im();

		UNLOCK_OBJECT(obj);

		setObjectValue_Double2Complex(obj,"feeder0",0,0);
		setObjectValue_Double2Complex(obj,"feeder1",0,0);
		setObjectValue_Double2Complex(obj,"feeder2",0,0);
		setObjectValue_Double2Complex(obj,"feeder3",0,0);
		setObjectValue_Double2Complex(obj,"feeder4",0,0);
		setObjectValue_Double2Complex(obj,"feeder5",0,0);
		setObjectValue_Double2Complex(obj,"feeder6",0,0);
		setObjectValue_Double2Complex(obj,"feeder7",0,0);
		setObjectValue_Double2Complex(obj,"feeder8",0,0);
		setObjectValue_Double2Complex(obj,"feeder9",0,0);
		

		//tempbus->PD = sum_PD;
		//tempbus->QD = sum_QD;

		// Unit of Power in MATPOWER is MW and MVAr
		setObjectValue_Double(obj,"PD",sum_PD/1000000);
		setObjectValue_Double(obj,"QD",sum_QD/1000000);

		
		
		//cout<<"sum_PD"<<sum_PD<<"   "<<tempbus->PD<<endl;
		//cout<<"sum_QD"<<sum_QD<<"   "<<tempbus->QD<<endl;

	}

	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Sync is called when the clock needs to advance on the bottom-up pass */
TIMESTAMP bus::sync(TIMESTAMP t0, TIMESTAMP t1)
{
	//TIMESTAMP t2 = TS_NEVER;
	TIMESTAMP t2 = t1+TIME_INTERVAL;
	OBJECT *obj = OBJECTHDR(this);
	bus *tempbus;
	
	tempbus = OBJECTDATA(obj,bus);
	//unsigned int bus_num = tempbus->BUS_I;
	
/*
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
	
*/
	if (tempbus->BUS_TYPE == 3) //ref bus
	{
/*		
		OBJECT *temp_obj = NULL;
		bus *iter_bus;
		FINDLIST *bus_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"bus",FT_END);
		printf("Before simulation!\n");
		while (gl_find_next(bus_list,temp_obj)!=NULL)
		{
			temp_obj = gl_find_next(bus_list,temp_obj);
			iter_bus = OBJECTDATA(temp_obj,bus);
			printf("Bus %d; PD %f; QD %f; VM %f; VA %f;\n",iter_bus->BUS_I,iter_bus->PD,iter_bus->QD,iter_bus->VM,iter_bus->VA);
		};
*/
		
		//Run OPF solver
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
/*	OBJECT *obj = OBJECTHDR(this);
	bus *tempbus;
	
	tempbus = OBJECTDATA(obj,bus);
	unsigned int bus_num = tempbus->BUS_I;
	if (tempbus->BUS_TYPE == 3)
	{
		OBJECT *temp_obj = NULL;
		bus *iter_bus;
		FINDLIST *bus_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"bus",FT_END);
		printf("After simulation\n");
		while (gl_find_next(bus_list,temp_obj)!=NULL)
		{
			temp_obj = gl_find_next(bus_list,temp_obj);
			iter_bus = OBJECTDATA(temp_obj,bus);
			printf("Bus %d; PD %f; QD %f; VM %f; VA %f;\n",iter_bus->BUS_I,iter_bus->PD,iter_bus->QD,iter_bus->VM,iter_bus->VA);
		};
	}
*/
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
