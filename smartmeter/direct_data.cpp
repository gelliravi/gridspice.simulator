/** $Id: direct_data.cpp 858 2008-08-28 21:06:13Z d3g637 $
	@file direct_data.cpp
	@defgroup direct_data Template for a new object class
	@ingroup MODULENAME

	You can add an object class to a module using the \e add_class
	command:
	<verbatim>
	linux% add_class CLASS
	</verbatim>

	You must be in the module directory to do this.

 **/

#include "direct_data.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "gridlabd.h"
#include "lock.h"

CLASS *direct_data::oclass = NULL;
direct_data *direct_data::defaults = NULL;

#ifdef OPTIONAL
/* TODO: define this to allow the use of derived classes */
CLASS *PARENTdirect_data::pclass = NULL;
#endif

static const int MIN_TRAINING_DAYS = 30;

/* TODO: remove passes that aren't needed */
static PASSCONFIG passconfig = PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN;

/* TODO: specify which pass the clock advances */
static PASSCONFIG clockpass = PC_BOTTOMUP;

/* Class registration is only called once to register the class with the core */
direct_data::direct_data(MODULE *module)
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
		oclass = gl_register_class(module, "direct_data", sizeof(direct_data), passconfig);
		if (oclass==NULL) {
			GL_THROW("unable to register object class implemented by %s", __FILE__);
        }

		if (gl_publish_variable(oclass,
            PT_complex, "current_load", PADDR(current_load),
            PT_complex_array, "forecasted_load", PADDR(forecasted_load),
            PT_char32, "customer_id", PADDR(customer_id),
            PT_char256, "dr_flags", PADDR(dr_flags),
			NULL) < 1) {
            GL_THROW("unable to publish properties in %s",__FILE__);
        }
		defaults = this;
		memset(this, 0, sizeof(direct_data));
        // set defaults here if needed
	}
}

/* Object creation is called once for each object that is created by the core */
int direct_data::create(void)
{
	memcpy(this,defaults,sizeof(direct_data));
	/* TODO: set the context-free initial value of properties, such as random distributions */
	return 1; /* return 1 on success, 0 on failure */
}

// taken from house_e.cpp in residential module
complex *direct_data::get_complex(OBJECT *obj, char *name)
{
	PROPERTY *p = gl_get_property(obj, name);
	if (p == NULL || p->ptype != PT_complex)
		return NULL;
	return (complex *) GETADDR(obj,p);
}

/* Object initialization is called once after all object have been created */
int direct_data::init(OBJECT *parent)
{
    if (!db_access::is_connected()) {
        extern char256 db_name, db_host, db_user, db_pwd, db_port;
        char *name = (db_name == "") ? 0 : db_name;
        char *host = (db_host == "") ? 0 : db_host;
        char *user = (db_user == "") ? 0 : db_user;
        char *pwd = (db_pwd == "") ? 0 : db_pwd;
        db_access::init_connection(name, host, user, pwd, 
            (unsigned int) db_port);
    }
    db = new db_access(customer_id);

    DATETIME earliest_date, latest_date;
    
    // initialize the DATETIME objects with dummy values
    TIMESTAMP dummy_timestamp = 0;
    gl_localtime(dummy_timestamp, &earliest_date);
    gl_localtime(dummy_timestamp, &latest_date);

    // populate the DATETIME objects with real values 
    db->get_earliest_date(earliest_date);
    db->get_latest_date(latest_date);
    earliest_time = gl_mktime(&earliest_date);
    latest_time = gl_mktime(&latest_date);

    f = new forecaster(earliest_time, MIN_TRAINING_DAYS, db);
    earliest_time = round_to_day_start(earliest_time + 
        (MIN_TRAINING_DAYS * S_DAY_IN_SECONDS));

    OBJECT *obj = OBJECTHDR(this);
	if (parent != NULL && (gl_object_isa(parent,"triplex_meter", "powerflow") 
        || gl_object_isa(obj->parent, "triplex_node", "powerflow"))) {

        // link pPower to parent object
        pPower = get_complex(parent, "power_1");
        if (pPower == NULL) {
            GL_THROW("parent of direct_data doesn't implement power_1 field!");
        }

        // link shunt to parent object
        pShunt = get_complex(parent, "shunt_1");
        if (pShunt == NULL) {
            GL_THROW("parent of direct_data doesn't implement shunt_1 field!");
        }

        // link shunt to parent object
        pLine_I = get_complex(parent, "residential_nominal_current_1");
        if (pLine_I == NULL) {
            GL_THROW("parent of direct_data doesn't implement residential_nominal_current_1 field!");
        }

        // link pVoltage to parent object
        pVoltage = get_complex(parent, "voltage_12");
        if (pLine_I == NULL) {
            GL_THROW("parent of direct_data doesn't implement voltage_12field!");
        }

    }
    return 1;
}


void direct_data::update_day_ahead_forecast(TIMESTAMP t)
{
    double temp[96];
    for (int i = 0; i < 96; i++) {
        TIMESTAMP curr = t + (i * S_INTERVAL_IN_SECONDS);
        bool is_dr = false;
        if (strcmp(dr_flags, "") != 0) {
            is_dr = dr_flags[i] == '1';
        }
        temp[i] = f->predict_load(curr, is_dr);
        forecasted_load[i] = temp[i];
        // forecasted_load[i] = f->predict_load(curr, is_dr);
    }
    std::cout << std::endl << "DAY AHEAD FORECAST" << std::endl;
    for (int i = 0; i < 96; i++) {
        std::cout << "Interval " << i << ": " << temp[i] << std::endl;
    }
}

/* Presync is called when the clock needs to advance on the first top-down pass */
TIMESTAMP direct_data::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement pre-topdown behavior */

	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Sync is called when the clock needs to advance on the bottom-up pass */
TIMESTAMP direct_data::sync(TIMESTAMP t0, TIMESTAMP t1)
{
    // update load at system for current time
    TIMESTAMP to_return = TS_NEVER;

    if (t1 < earliest_time) {
        to_return = earliest_time;
    } else if (t1 <= latest_time) {

        // first check if we need to update forecast for next day
        DATETIME dt0, dt1;
        gl_localtime(t0, &dt0);
        gl_localtime(t1, &dt1);
        if (dt0.day != dt1.day) { // we're transitioning to a new day in the system
            f->update_forecast(t1);
            TIMESTAMP day_start = round_to_day_start(t1);
            update_day_ahead_forecast(day_start);
        }

        // increment time by INTERVAL_SIZE (in minutes) * 60 to get seconds
        to_return = t1 + S_INTERVAL_IN_SECONDS;

        current_load = db->get_power_usage(dt1);

        OBJECT *obj = OBJECTHDR(this);
        if (obj->parent != NULL) {
            LOCK_OBJECT(obj->parent);

            complex voltage = *pVoltage; // volts
            complex current = current_load / voltage;
            complex resistance = voltage / current;
            complex shunt = ((complex) 1) / resistance; // shunt = admittance

            pPower[0] = current_load;
            pPower[1] = current_load;
            pPower[2] = current_load;
            
            pLine_I[0] = current;
            pLine_I[1] = current;
            pLine_I[2] = current;

            pShunt[0] = shunt;
            pShunt[1] = shunt;
            pShunt[2] = shunt;

            UNLOCK_OBJECT(obj->parent);
        }
    }
    return to_return;
}

/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP direct_data::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement post-topdown behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////

EXPORT int create_direct_data(OBJECT **obj)
{
	try
	{
		*obj = gl_create_object(direct_data::oclass);
		if (*obj!=NULL)
			return OBJECTDATA(*obj,direct_data)->create();
	}
	catch (char *msg)
	{
		gl_error("create_direct_data: %s", msg);
	}
	return 0;
}

EXPORT int init_direct_data(OBJECT *obj, OBJECT *parent)
{
	try
	{
		if (obj!=NULL)
			return OBJECTDATA(obj,direct_data)->init(parent);
	}
	catch (char *msg)
	{
		gl_error("init_direct_data(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return 0;
}

EXPORT TIMESTAMP sync_direct_data(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	direct_data *my = OBJECTDATA(obj,direct_data);
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
		gl_error("sync_direct_data(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return TS_INVALID;
}
