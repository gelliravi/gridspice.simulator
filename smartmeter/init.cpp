// init.cpp

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include "gridlabd.h"

#include "smartmeter.h"
#include "direct_data.h"
#include "db_access.h"

//NEWCLASSINC

char256 db_name = "";
char256 db_host = "";
char256 db_user = "";
char256 db_pwd = "";
int16 db_port = 0;

EXPORT CLASS *init(CALLBACKS *fntable, MODULE *module, int argc, char *argv[])
{
	if (set_callback(fntable)==NULL)
	{
		errno = EINVAL;
		return NULL;
	}


    gl_global_create("smartmeter::db_name", PT_char256, &db_name, NULL);
	gl_global_create("smartmeter::db_host", PT_char256, &db_host, NULL);
    gl_global_create("smartmeter::db_user", PT_char256, &db_user, NULL);
    gl_global_create("smartmeter::db_pwd", PT_char256, &db_pwd, NULL);
    gl_global_create("smartmeter::db_port", PT_int16, &db_port, NULL);
	/* TODO: use gl_global_setvar, gl_global_getvar, and gl_global_find for access */

	/*** DO NOT EDIT NEXT LINE ***/
	new direct_data(module);
	//NEWCLASSDEF
	
	/* always return the first class registered */
	/* TODO this module will not compile until a class has been defined */
	return direct_data::oclass;
}


CDECL int do_kill()
{
	/* if global memory needs to be released, this is a good time to do it */
	return 0;
}
