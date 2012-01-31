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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "direct_data.h"

CLASS *direct_data::oclass = NULL;
direct_data *direct_data::defaults = NULL;

#ifdef OPTIONAL
/* TODO: define this to allow the use of derived classes */
CLASS *PARENTdirect_data::pclass = NULL;
#endif

/* TODO: remove passes that aren't needed */
static PASSCONFIG passconfig = PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN;

/* TODO: specify which pass the clock advances */
static PASSCONFIG clockpass = PC_BOTTOMUP;

const int INTERVAL_SIZE = 15; // load interval size, in minutes

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
            PT_complex, "current_power", PADDR(current_power),
            PT_char32, "customer_id", PADDR(customer_id),
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
    return 1;
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
    TIMESTAMP to_return = TS_NEVER;
    if (t1 < earliest_time) {
        to_return = earliest_time;
    } else if (t1 <= latest_time) {
        DATETIME dt;
        gl_localtime(t1, &dt);
        
        current_power = db->get_power_usage(dt);
        // increment time by INTERVAL_SIZE (in minutes) * 60 to get seconds
        to_return = t1 + (INTERVAL_SIZE * 60);
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
