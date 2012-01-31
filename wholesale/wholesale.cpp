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
#include "wholesale.h"
#include "bus.h"
#include "gen.h"
#include "line.h"
#include "gen_cost.h"

#include <vector>
using std::vector;
//#include "libopf.h"
//#include <stdlib.h>
//#include "matrix.h"
//#include "solver_matpower.h"

mxArray* initArray(double rdata[], int nRow, int nColumn) 
{
	mxArray* X = mxCreateDoubleMatrix(nRow, nColumn, mxREAL);
	memcpy(mxGetPr(X), rdata, nRow*nColumn*sizeof(double));
	return X;
}


/* Sync is called when the clock needs to advance on the bottom-up pass */
//int solver_matpower(double *rbus, unsigned int nbus, double *rgen, unsigned int ngen, 
//	double *rbranch, unsigned int nbranch, double *rgencost, unsigned int ngencost,
//	double *rareas,	unsigned int nareas)
int solver_matpower()
{	
	unsigned int nbus = 0;
	unsigned int ngen = 0;
	unsigned int nbranch = 0;
	unsigned int ngencost = 0;
	unsigned int nareas = 0;

	vector<bus> vec_bus;
	vector<gen> vec_gen;
	vector<line> vec_branch;
	vector<gen_cost> vec_gencost;	

	nbus = 9;
	ngen = 3;
	nbranch = 9;
	ngencost = 3;
	nareas = 1;	
	
	printf("========Getting Data=============\n");

	
	OBJECT *temp_obj = NULL;
	bus *list_bus;
	FINDLIST *bus_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"bus",FT_END);

	while (gl_find_next(bus_list,temp_obj)!=NULL)
	{
		temp_obj = gl_find_next(bus_list,temp_obj);
		list_bus = OBJECTDATA(temp_obj,bus);
		vec_bus.push_back(*list_bus);
		//printf("%d\n",list_bus->BUS_TYPE);
        };
        //printf("Total number: %d\n",vec_bus.size());
	//printf("=============BUS=============\n");

	gen *list_gen;
	FINDLIST *gen_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"gen",FT_END);
	temp_obj = NULL;
	
	while (gl_find_next(gen_list,temp_obj)!=NULL)
	{
		temp_obj = gl_find_next(gen_list,temp_obj);
		list_gen = OBJECTDATA(temp_obj,gen);
		vec_gen.push_back(*list_gen);
		//printf("%d\n",list_gen->GEN_BUS);
        };
        
	//printf("=============GEN=============\n");

	line *list_line;
	FINDLIST *line_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"line",FT_END);
	temp_obj = NULL;

	while (gl_find_next(line_list,temp_obj)!=NULL)	
	{
		temp_obj = gl_find_next(line_list,temp_obj);
		list_line = OBJECTDATA(temp_obj,line);
		vec_branch.push_back(*list_line);
		//printf("from %d to %d \n",list_line->F_BUS,list_line->T_BUS);
	}

	//printf("=============LINE============\n");

	gen_cost *list_gen_cost;
	FINDLIST *gen_cost_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"gen_cost",FT_END);
	temp_obj = NULL;

	while (gl_find_next(gen_cost_list,temp_obj)!=NULL)
	{
		temp_obj = gl_find_next(gen_cost_list,temp_obj);
		list_gen_cost = OBJECTDATA(temp_obj,gen_cost);
		vec_gencost.push_back(*list_gen_cost);
		//printf("model %d\n",list_gen_cost->MODEL);
	}

	//printf("===========GEN_COST==========\n");

	// get the size
	nbus = vec_bus.size();
	ngen = vec_gen.size();
	nbranch = vec_branch.size();
	ngencost = vec_gencost.size();

	// create arrays for input and allocate memory
	double *rbus;
	rbus = (double *) calloc(nbus*BUS_ATTR,sizeof(double));

	double *rgen;
	rgen = (double *) calloc(ngen*GEN_ATTR,sizeof(double));	

	double *rbranch;
	rbranch = (double *) calloc(nbranch*BRANCH_ATTR,sizeof(double));

	
	// insert data for rbus
	double *temp;
	temp = (double *) calloc(nbus*BUS_ATTR,sizeof(double));
	unsigned int counter = 0;
	vector<bus>::const_iterator iter = vec_bus.begin();


	printf("Running Test\n");
        libopfInitialize();
	mxArray* baseMVA = mxCreateDoubleMatrix(1,1,mxREAL);
	*mxGetPr(baseMVA) = 100.0;
/*
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
*/
	mxArray* bus = initArray(rbus, nbus, BUS_ATTR);	
/*
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

*/
	mxArray* gen = initArray(rgen, ngen, GEN_ATTR);
/*
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
*/
	mxArray* branch = initArray(rbranch, nbranch, BRANCH_ATTR);

	double rgencost[21] = {2,2,2,
			1500,2000,3000,
			0,0,0,
			3,3,3,
			0.11,0.085,0.1225,
			5,1.2,1,
			150,600,335};

	mxArray* gencost = initArray(rgencost, ngencost, GENCOST_ATTR);

	double rareas[2] = {1, 2};
	mxArray* areas = initArray(rareas, nareas, AREA_ATTR);


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

	return 0;

}


