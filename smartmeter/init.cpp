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

double test_global;
EXPORT CLASS *init(CALLBACKS *fntable, MODULE *module, int argc, char *argv[])
{
	if (set_callback(fntable)==NULL)
	{
		errno = EINVAL;
		return NULL;
	}


    db_access::init_connection("lidb", 0, "lidb_user", "smartgrid!!", 0);
	gl_global_create("smartmeter::test_global", PT_double, &test_global, NULL);

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
