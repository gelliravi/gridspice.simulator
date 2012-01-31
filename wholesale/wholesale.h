/* $Id: wholesale.h 683 2008-06-18 20:16:29Z d3g637 $ */

#ifndef _wholesale_H
#define _wholesale_H

#include <stdarg.h>
#include "gridlabd.h"
#include "matrix.h"
#include "libopf.h"

#ifndef ATTR_NUM
#define BUS_ATTR 13
#define GEN_ATTR 18
#define BRANCH_ATTR 13
#define GENCOST_ATTR 7
#define AREA_ATTR 2
#endif

/*** DO NOT DELETE THE NEXT LINE ***/
//NEWCLASSINC
inline mxArray* initArray(double rdata[], int nRow, int nColumn);
//int solver_matpower(double *rbus, unsigned int nbus, double *rgen, unsigned int ngen, 
//	double *rbranch, unsigned int nbranch, double *rgencost, unsigned int ngencost,
//	double *rareas,	unsigned int nareas);

int solver_matpower();

/* optional exports */
#ifdef OPTIONAL

/* TODO: define this function to enable checks routine */
EXPORT int check(void);

/* TODO: define this function to allow direct import of models */
EXPORT int import_file(char *filename);

/* TODO: define this function to allow direct export of models */
EXPORT int export_file(char *filename);

/* TODO: define this function to allow export of KML data for a single object */
EXPORT int kmldump(FILE *fp, OBJECT *obj);
#endif

#endif
