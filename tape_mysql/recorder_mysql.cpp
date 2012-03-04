/** $Id: recorder_mysql.cpp 858 2008-08-28 21:06:13Z d3g637 $
	@file recorder_mysql.cpp
	@defgroup recorder_mysql Template for a new object class
	@ingroup MODULENAME

	You can add an object class to a module using the \e add_class
	command:
	<vebatim>
	linux% add_class CLASS
	</verbatim>

	You must be in the module directory to do this.

 **/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include <string>

#include "db_access.h"
#include "recorder_mysql.h"
#include "tape_mysql.h" 

#include "gridlabd.h"
#include "object.h"
#include "aggregate.h"

#include "lock.h"

CLASS *recorder_mysql::oclass = NULL;
recorder_mysql *recorder_mysql::defaults = NULL;

#ifdef OPTIONAL
/* TODO: define this to allow the use of derived classes */
CLASS *PARENTrecorder_mysql::pclass = NULL;
#endif

/* TODO: remove passes that aren't needed */
static PASSCONFIG passconfig = PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN;

/* TODO: specify which pass the clock advances */
static PASSCONFIG clockpass = PC_BOTTOMUP;


static void close_recorder_mysql(struct recorder_mysql *my){
}

static int recorder_open(OBJECT *obj)
{
}

/* Class registration is only called once to register the class with the core */
recorder_mysql::recorder_mysql(MODULE *module)
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
		oclass = gl_register_class(module,"recorder_mysql",sizeof(recorder_mysql),passconfig);
		if (oclass==NULL)
			GL_THROW("unable to register object class implemented by %s", __FILE__);

		if (gl_publish_variable(oclass,
					PT_double, "interval[s]", PADDR(dInterval),
					PT_char1024, "property", PADDR(property),
					PT_char32, "trigger", PADDR(trigger),
					PT_char1024, "file", PADDR(file),				     
					PT_int32, "limit", PADDR(limit),
		    NULL)<1) GL_THROW("unable to publish properties in %s",__FILE__);
		defaults = this;
		memset(this,0,sizeof(recorder_mysql));
		/* TODO: set the default values of all properties here */
	}
}

/* Object creation is called once for each object that is created by the core */
int recorder_mysql::create(void)
{
	memcpy(this,defaults,sizeof(recorder_mysql));
	/* TODO: set the context-free initial value of properties, such as random distributions */
	return 1; /* return 1 on success, 0 on failure */
}

/* Object initialization is called once after all object have been created */
int recorder_mysql::init(OBJECT *parent)
{

  this->interval = (int64)(this->dInterval/TS_SECOND);
  if (!db_access::is_connected()) {
    extern char256 db_name, db_host, db_user, db_pwd, db_port;
    char *name = (db_name == "") ? 0 : db_name;
    char *host = (db_host == "") ? 0 : db_host;
    char *user = (db_user == "") ? 0 : db_user;
    char *pwd = (db_pwd == "") ? 0 : db_pwd;
    db_access::init_connection(name, host, user, pwd, 
			       (unsigned int) db_port);
  }
  this->props = link_properties(parent,this->property);
  std::vector<std::string> myProperties;
  for (PROPERTY *p=props; p!=NULL; p=p->next)
    {
      myProperties.push_back(p->name);
      }
  db = new db_access(parent->name, myProperties, false);

  /* TODO: set the context-dependent initial value of properties */
  return 1; /* return 1 on success, 0 on failure */
}

/* Presync is called when the clock needs to advance on the first top-down pass */
TIMESTAMP recorder_mysql::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;

	std::cout<<"PRESYNCING MYSQL RECORDER"<<std::endl;
	/* TODO: implement pre-topdown behavior */
	return t1+15; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Sync is called when the clock needs to advance on the bottom-up pass */
TIMESTAMP recorder_mysql::sync(TIMESTAMP t0, TIMESTAMP t1)

{
  OBJECT *obj = OBJECTHDR(this);
    PROPERTY *p;
    int offset=0;
      int count=0;
   std::vector<std::string> values;
     char buffer[300];
     int size = 300;
     for (p=this->props; p!=NULL && offset<size-33; p=p->next)
       {
	 offset=gl_get_value(obj,GETADDR(obj,p),buffer,size-1,p); /* pointer => int64 */
	   buffer[offset]='\0';
          std::stringstream ss;
	    ss << buffer;
     values.push_back(ss.str());
       std::cout<<"READ PROPS"<<ss.str()<<std::endl;
          count++;
 }

 char ts[64]="0"; /* 0 = INIT */
 if (this->format==0)
 {
  if (this->last.ts>TS_ZERO)
  {
   DATETIME dt;
			gl_localtime(this->last.ts,&dt);
			gl_strtime(&dt,ts,sizeof(ts));
		}
		/* else leave INIT in the buffer */
	}else{
		sprintf(ts,"%" FMT_INT64 "d", this->last.ts);
	}
	std::stringstream ss;
	ss<<ts;
	this->db->write_properties( ss.str(), values );
	
	std::cout<<"SYNCING MYSQL RECORDER"<<std::endl;
	return t1+15; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}


int recorder_mysql::read_properties(OBJECT *obj, PROPERTY *prop, char *buffer, int size)
{
}


/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP recorder_mysql::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement post-topdown behavior */
	std::cout<<"POSTSYNCING MYSQL RECORDER"<<std::endl;
	return t1+15; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////

EXPORT int create_recorder_mysql(OBJECT **obj)
{
	try
	{
		*obj = gl_create_object(recorder_mysql::oclass);
		if (*obj!=NULL)
			return OBJECTDATA(*obj,recorder_mysql)->create();
	}
	catch (char *msg)
	{
		gl_error("create_recorder_mysql: %s", msg);
	}
	return 0;
}

EXPORT int init_recorder_mysql(OBJECT *obj, OBJECT *parent)
{
  std::cout<<"init C-Function"<<std::endl;
	try
	{
		if (obj!=NULL)
			return OBJECTDATA(obj,recorder_mysql)->init(parent);
	}
	catch (char *msg)
	{
		gl_error("init_recorder_mysql(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return 0;
}

EXPORT TIMESTAMP sync_recorder_mysql(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	recorder_mysql *my = OBJECTDATA(obj,recorder_mysql);
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
		gl_error("sync_recorder_mysql(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return TS_INVALID;


}


static int write_recorder(struct recorder_mysql *my, char *ts, char *value)
{
  std::cout<<"WRITING VALUE: "<<value<<std::endl;
  return 0;
}


static TIMESTAMP recorder_write(OBJECT *obj)
{
	struct recorder_mysql *my = OBJECTDATA(obj,struct recorder_mysql);
	char ts[64]="0"; /* 0 = INIT */
	if (my->format==0)
	{
		if (my->last.ts>TS_ZERO)
		{
			DATETIME dt;
			gl_localtime(my->last.ts,&dt);
			gl_strtime(&dt,ts,sizeof(ts));
		}
		/* else leave INIT in the buffer */
	}
	else{
		sprintf(ts,"%" FMT_INT64 "d", my->last.ts);
	}
	if ((my->limit>0 && my->samples > my->limit) /* limit reached */
		|| write_recorder(my, ts, my->last.value)==0) /* write failed */
	{
		close_recorder_mysql(my);
		my->status = TS_DONE;
	}
	else
		my->samples++;

	/* at this point we've written the sample to the normal recorder output */
	return TS_NEVER;
}
