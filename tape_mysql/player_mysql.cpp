/** $Id: player_mysql.cpp 858 2008-08-28 21:06:13Z d3g637 $
	@file player_mysql.cpp
	@defgroup player_mysql Template for a new object class
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
#include <ctype.h>
#include <string>
#include <sstream> 


#include "player_mysql.h"
 
#include "gridlabd.h"
#include "object.h"
#include "aggregate.h"


CLASS *player_mysql::oclass = NULL;
player_mysql *player_mysql::defaults = NULL;

#ifdef OPTIONAL
/* TODO: define this to allow the use of derived classes */
CLASS *PARENTplayer_mysql::pclass = NULL;
#endif

/* TODO: remove passes that aren't needed */
static PASSCONFIG passconfig = PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN;

/* TODO: specify which pass the clock advances */
static PASSCONFIG clockpass = PC_BOTTOMUP;

/* Class registration is only called once to register the class with the core */
player_mysql::player_mysql(MODULE *module)
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
		oclass = gl_register_class(module,"player_mysql",sizeof(player_mysql),passconfig);
		if (oclass==NULL)
			GL_THROW("unable to register object class implemented by %s", __FILE__);

		if (gl_publish_variable(oclass,
			/* TODO: add your published properties here */
					PT_double, "interval[s]", PADDR(dInterval),
					PT_char1024, "property", PADDR(property),
					PT_char1024, "table", PADDR(table),				     
					PT_char1024, "columns", PADDR(columns),				     
					PT_char1024, "file", PADDR(file),				     
			NULL)<1) GL_THROW("unable to publish properties in %s",__FILE__);
		defaults = this;
		memset(this,0,sizeof(player_mysql));
		/* TODO: set the default values of all properties here */
	}
}

/* Object creation is called once for each object that is created by the core */
int player_mysql::create(void)
{
	memcpy(this,defaults,sizeof(player_mysql));
	/* TODO: set the context-free initial value of properties, such as random distributions */
	return 1; /* return 1 on success, 0 on failure */
}

/* Object initialization is called once after all object have been created */
int player_mysql::init(OBJECT *parent)
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
  PROPERTY *props= link_properties(parent,this->property);
  this->target= props;
 
  this->propertyToColumn = new std::map<std::string, std::string>();


  std::vector<std::string> myProperties;
  char * item;
  char1024 list;
  PROPERTY *p=props;
  strcpy(list,this->columns); /* avoid destroying orginal list */
  for (item=strtok(list,","); item!= NULL; item=strtok(NULL,","))
    {
      if( p==NULL ){
	gl_error("property count and column count do not match");
	throw "die";
      }

      std::stringstream ss;
      ss<<item;
      myProperties.push_back(ss.str());


      this->propertyToColumn->insert ( std::pair<std::string, std::string>(p->name,ss.str()) );
      p=p->next;
    }
  db = new db_access(table, myProperties, true);
  
      if( !this->target ){
	gl_error("COULD NOT FIND PROPERTIES");
	  }
	/* TODO: set the context-dependent initial value of properties */
	return 1; /* return 1 on success, 0 on failure */
}
	 
/* Presync is called when the clock needs to advance on the first top-down pass */
TIMESTAMP player_mysql::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement pre-topdown behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Sync is called when the clock needs to advance on the bottom-up pass */
TIMESTAMP player_mysql::sync(TIMESTAMP t0, TIMESTAMP t1)
{
  OBJECT *obj = OBJECTHDR(this);
  char ts[64]="0"; /* 0 = INIT */
  sprintf(ts,"%" FMT_INT64 "d", t1);
  std::stringstream ss;
  ss<<ts;
  std::map<std::string, std::string> * values;
  values = this->db->read_properties( ss.str() );
  for (PROPERTY *p=this->target; p!=NULL; p=p->next)
    {


	std::map<std::string, std::string>::iterator it = this->propertyToColumn->find(p->name);
	std::string columnName = it->second;
	if( it == values->end() ){
	  std::cerr<<"can't find column for property %s"<<p->name;
	  throw "column error";
	}	
	std::map<std::string, std::string>::iterator it2 = values->find(columnName);
	if( it2 == values->end() ){
	  std::cerr<<"mismatched column %s"<<columnName;
	  throw "column error";
	}
	std::string value = it2->second;
	char1024 valueCstr;
	strcpy(valueCstr, value.c_str());
	gl_set_value(obj->parent,GETADDR(obj->parent,p),valueCstr,p); /* pointer => int64 */
	std::cout<<"SETTING "<<p->name<<" TO: "<<value;
    }

	  	
  return t1+15*60;
}

/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP player_mysql::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement post-topdown behavior */
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}


typedef struct { /**< complex number */
	double r; /**< the real part */
	double i; /**< the imaginary part */
	CNOTATION f; /**< the default notation used */
} complex_struct; 

PROPERTY *link_properties(OBJECT *obj, char *property_list)
{
	char *item;
	PROPERTY *first=NULL, *last=NULL;
	UNIT *unit = NULL;
	PROPERTY *prop;
	PROPERTY *target;
	char1024 list;
	complex_struct oblig;
	double scale;
	char256 pstr, ustr;
	char *cpart = 0;
	int64 cid = -1;
	printf("linking properties\n");
	strcpy(list,property_list); /* avoid destroying orginal list */
	for (item=strtok(list,","); item!=NULL; item=strtok(NULL,","))
	{
		
		prop = NULL;
		target = NULL;
		scale = 1.0;
		unit = NULL;
		cpart = 0;
		cid = -1;
		printf("Item: %s\n",item);
		// everything that looks like a property name, then read units up to ]
		while (isspace(*item)) item++;
		if(2 == sscanf(item,"%[A-Za-z0-9_.][%[^]\n,\0]", pstr, ustr)){
		  printf("Units: %s\n",ustr);
		  unit = gl_find_unit(ustr);
			if(unit == NULL){
				gl_error("recorder:%d: unable to find unit '%s' for property '%s'",obj->id, ustr,pstr);
				return NULL;
			}
			item = pstr;
		}
		prop = (PROPERTY*)malloc(sizeof(PROPERTY));
		
		/* branch: test to see if we're trying to split up a complex property */
		/* must occur w/ *cpart=0 before gl_get_property in order to properly reformat the property name string */
		cpart = strchr(item, '.');
		if(cpart != NULL){
			if(strcmp("imag", cpart+1) == 0){
	       		cid = (int)((int64)&(oblig.i) - (int64)&oblig);
				*cpart = 0;
			} else if(strcmp("real", cpart+1) == 0){
		     	cid = (int)((int64)&(oblig.r) - (int64)&oblig);
				*cpart = 0;
			} else {
				;
			}
		}

		target = gl_get_property(obj,item);

		if (prop!=NULL && target!=NULL)
		{
			if(unit != NULL && target->unit == NULL){
				gl_error("recorder:%d: property '%s' is unitless, ignoring unit conversion", obj->id, item);
			}
			else if(unit != NULL && 0 == gl_convert_ex(target->unit, unit, &scale))
			{
				gl_error("recorder:%d: unable to convert property '%s' units to '%s'", obj->id, item, ustr);
				return NULL;
			}
			if (first==NULL) first=prop; else last->next=prop;
			last=prop;
			memcpy(prop,target,sizeof(PROPERTY));
			if(unit) prop->unit = unit;
			prop->next = NULL;
		}
		else
		{
			gl_error("recorder: property '%s' not found", item);
			return NULL;
		}
		if(cid >= 0){ /* doing the complex part thing */
			prop->ptype = PT_double;
			(prop->addr) = (PROPERTYADDR)((int64)(prop->addr) + cid);
		}
	}
	return first;
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////
static OBJECT *last_player = NULL;
EXPORT int create_player_mysql(OBJECT **obj)
{
	try
	{
		*obj = gl_create_object(player_mysql::oclass);
		if (*obj!=NULL){
		  struct player_mysql *my = OBJECTDATA(*obj,struct player_mysql);
		  last_player = *obj;

		  strcpy(my->file,"");
		  strcpy(my->filetype,"txt");
		  strcpy(my->property,"(undefined)");
		  my->next.ts = TS_ZERO;
		  strcpy(my->next.value,"");
		  my->loopnum = 0;
		  my->loop = 0;
		  my->status = TS_INIT;
		  return 1;
		}
			
	}
	catch (char *msg)
	{
		gl_error("create_player_mysql: %s", msg);
	}
	return 0;
}

EXPORT int init_player_mysql(OBJECT *obj, OBJECT *parent)
{
	try
	{
		if (obj!=NULL)
			return OBJECTDATA(obj,player_mysql)->init(parent);
	}
	catch (char *msg)
	{
		gl_error("init_player_mysql(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return 0;
}

EXPORT TIMESTAMP sync_player_mysql(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	player_mysql *my = OBJECTDATA(obj,player_mysql);
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
		gl_error("sync_player_mysql(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
	}
	return TS_INVALID;
}
