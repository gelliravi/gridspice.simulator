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
#include "areas.h"
#include "baseMVA.h"




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
	unsigned int nbaseMVA = 0;

	vector<bus> vec_bus;
	vector<gen> vec_gen;
	vector<line> vec_branch;
	vector<areas> vec_areas;
	vector<gen_cost> vec_gencost;
	vector<baseMVA> vec_baseMVA;	


	
	printf("========Getting Data=============\n");

	// Get Bus objects
	OBJECT *temp_obj = NULL;
	bus *list_bus;
	FINDLIST *bus_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"bus",FT_END);

	while (gl_find_next(bus_list,temp_obj)!=NULL)
	{
		temp_obj = gl_find_next(bus_list,temp_obj);
		list_bus = OBJECTDATA(temp_obj,bus);
		vec_bus.push_back(*list_bus);
        };

	// Get Generator objects
	gen *list_gen;
	FINDLIST *gen_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"gen",FT_END);
	temp_obj = NULL;
	
	while (gl_find_next(gen_list,temp_obj)!=NULL)
	{
		temp_obj = gl_find_next(gen_list,temp_obj);
		list_gen = OBJECTDATA(temp_obj,gen);
		vec_gen.push_back(*list_gen);
        };

	// Get Line/Branch Objects
	line *list_line;
	FINDLIST *line_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"line",FT_END);
	temp_obj = NULL;

	while (gl_find_next(line_list,temp_obj)!=NULL)	
	{
		temp_obj = gl_find_next(line_list,temp_obj);
		list_line = OBJECTDATA(temp_obj,line);
		vec_branch.push_back(*list_line);
	}

	// Get Area Objects
	areas *list_areas;
	FINDLIST *areas_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"areas",FT_END);
	temp_obj = NULL;
	
	while (gl_find_next(areas_list,temp_obj) != NULL)
	{
		temp_obj = gl_find_next(areas_list,temp_obj);
		list_areas = OBJECTDATA(temp_obj,areas);
		vec_areas.push_back(*list_areas);
	}

	
	// Get Generator Cost objects
	gen_cost *list_gen_cost;
	FINDLIST *gen_cost_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"gen_cost",FT_END);
	temp_obj = NULL;

	while (gl_find_next(gen_cost_list,temp_obj)!=NULL)
	{
		temp_obj = gl_find_next(gen_cost_list,temp_obj);
		list_gen_cost = OBJECTDATA(temp_obj,gen_cost);
		vec_gencost.push_back(*list_gen_cost);

	}
	
	// Get Base Information object
	baseMVA *list_baseMVA;
	FINDLIST *baseMVA_list = gl_find_objects(FL_NEW,FT_CLASS,SAME,"baseMVA",FT_END);
	temp_obj = NULL;
	temp_obj = gl_find_next(baseMVA_list,temp_obj);
	list_baseMVA = OBJECTDATA(temp_obj,baseMVA);
	vec_baseMVA.push_back(*list_baseMVA);

	// Get the size of each class
	nbus = vec_bus.size();
	ngen = vec_gen.size();
	nbranch = vec_branch.size();
	ngencost = vec_gencost.size();
	nareas = vec_areas.size();
	nbaseMVA = vec_baseMVA.size();

	// create arrays for input and allocate memory
	double *rbus;
	rbus = (double *) calloc(nbus*BUS_ATTR,sizeof(double));

	double *rgen;
	rgen = (double *) calloc(ngen*GEN_ATTR,sizeof(double));	

	double *rbranch;
	rbranch = (double *) calloc(nbranch*BRANCH_ATTR,sizeof(double));

	double *rareas;
	rareas = (double *) calloc(nareas*AREA_ATTR,sizeof(double));

	double rbaseMVA;

	double *rgencost; // allocation of memory is in the following part
	
	

	
	// insert bus data for rbus
	vector<bus>::const_iterator iter_bus = vec_bus.begin();
	if (nbus > 1)
	{
		for (unsigned int i=0; i < nbus; i++)
		{
			rbus[i+0*nbus] = (double)iter_bus->BUS_I;
			rbus[i+1*nbus] = (double)iter_bus->BUS_TYPE;
			rbus[i+2*nbus] = iter_bus->PD;
			rbus[i+3*nbus] = iter_bus->QD;
			rbus[i+4*nbus] = iter_bus->GS;
			rbus[i+5*nbus] = iter_bus->BS;
			rbus[i+6*nbus] = (double)iter_bus->BUS_AREA;
			rbus[i+7*nbus] = iter_bus->VM;
			rbus[i+8*nbus] = iter_bus->VA;
			rbus[i+9*nbus] = iter_bus->BASE_KV;
			rbus[i+10*nbus] = (double)iter_bus->ZONE;
			rbus[i+11*nbus] = iter_bus->VMAX;
			rbus[i+12*nbus] = iter_bus->VMIN;
			iter_bus++;
		}
	}

	
	// insert data for rgen
	vector<gen>::const_iterator iter_gen = vec_gen.begin();
	for (unsigned int i = 0; i < ngen; i++)
	{
		rgen[i+0*ngen] = (double) iter_gen->GEN_BUS;
		rgen[i+1*ngen] = iter_gen->PG;
		rgen[i+2*ngen] = iter_gen->QG;
		rgen[i+3*ngen] = iter_gen->QMAX;
		rgen[i+4*ngen] = iter_gen->QMIN;
		rgen[i+5*ngen] = iter_gen->VG;
		rgen[i+6*ngen] = iter_gen->MBASE;
		rgen[i+7*ngen] = iter_gen->GEN_STATUS;
		rgen[i+8*ngen] = iter_gen->PMAX;
		rgen[i+9*ngen] = iter_gen->PMIN;
		rgen[i+10*ngen] = iter_gen->PC1;
		rgen[i+11*ngen] = iter_gen->PC2;
		rgen[i+12*ngen] = iter_gen->QC1MIN;
		rgen[i+13*ngen] = iter_gen->QC1MAX;
		rgen[i+14*ngen] = iter_gen->QC2MIN;
		rgen[i+15*ngen] = iter_gen->QC2MAX;
		rgen[i+16*ngen] = iter_gen->RAMP_AGC;
		rgen[i+17*ngen] = iter_gen->RAMP_10;
		rgen[i+18*ngen] = iter_gen->RAMP_30;
		rgen[i+19*ngen] = iter_gen->RAMP_Q;
		rgen[i+20*ngen] = iter_gen->APF;
		iter_gen++;
	}	



	// insert data for rbranch
	vector<line>::const_iterator iter_branch = vec_branch.begin();
	for (unsigned int i = 0; i < nbranch; i++)
	{
		rbranch[i+0*nbranch] = (double)iter_branch->F_BUS;
		rbranch[i+1*nbranch] = (double)iter_branch->T_BUS;
		rbranch[i+2*nbranch] = iter_branch->BR_R;
		rbranch[i+3*nbranch] = iter_branch->BR_X;
		rbranch[i+4*nbranch] = iter_branch->BR_B;
		rbranch[i+5*nbranch] = iter_branch->RATE_A;
		rbranch[i+6*nbranch] = iter_branch->RATE_B;		
		rbranch[i+7*nbranch] = iter_branch->RATE_C;
		rbranch[i+8*nbranch] = iter_branch->TAP;
		rbranch[i+9*nbranch] = iter_branch->SHIFT;
		rbranch[i+10*nbranch] = (double)iter_branch->BR_STATUS;
		rbranch[i+11*nbranch] = iter_branch->ANGMIN;
		rbranch[i+12*nbranch] = iter_branch->ANGMAX;
		iter_branch++;
	}

	
	// insert data for rareas
	vector<areas>::const_iterator iter_areas = vec_areas.begin();
	for (unsigned int i = 0; i < nareas; i++)
	{
		rareas[i+0*nareas] = (double)iter_areas->AREA;
		rareas[i+1*nareas] = (double)iter_areas->REFBUS;
		iter_areas++;
	} 

	// insert data for rbaseMVA
	vector<baseMVA>::const_iterator iter_baseMVA = vec_baseMVA.begin();
	rbaseMVA = iter_baseMVA->BASEMVA;


	// insert data for rgencost
	vector<gen_cost>::const_iterator iter_gencost = vec_gencost.begin();

	unsigned int max_order = 0;
	for (unsigned int i = 0; i<ngencost; i++)
	{
		if (iter_gencost->NCOST > max_order)
			max_order = iter_gencost->NCOST;
		iter_gencost++;
		
	}
	
	rgencost = (double *) calloc(ngencost*(GENCOST_ATTR+max_order),sizeof(double));
	
	iter_gencost = vec_gencost.begin();
	for (unsigned int i = 0; i<ngencost; i++)
	{
		rgencost[i+0*ngencost] = iter_gencost->MODEL;
		rgencost[i+1*ngencost] = iter_gencost->STARTUP;
		rgencost[i+2*ngencost] = iter_gencost->SHUTDOWN;
		rgencost[i+3*ngencost] = (double)iter_gencost->NCOST;
		string double_string(iter_gencost->COST);
		vector<string> v;
		v = split(double_string,',');
		for (unsigned int j = 0; j<v.size();j++)
		{
			rgencost[i+(4+j)*ngencost] = atof(v[j].c_str());
		}
		if (iter_gencost->NCOST != max_order)
		{
			for (unsigned int j = iter_gencost->NCOST; j < max_order; j++)
				rgencost[i+(4+j)*ngencost] = 0.0;
		}
		iter_gencost++;
	}

	// Run the Solver function
	printf("Running Test\n");
        libopfInitialize();
	//mxArray* basemva = initArray(rbaseMVA,nbaseMVA,BASEMVA_ATTR);
	mxArray* basemva = mxCreateDoubleMatrix(1,1,mxREAL);
	*mxGetPr(basemva) = rbaseMVA;

	// change to MATLAB MAT format
	mxArray* bus = initArray(rbus, nbus, BUS_ATTR);	
	mxArray* gen = initArray(rgen, ngen, GEN_ATTR);
	mxArray* branch = initArray(rbranch, nbranch, BRANCH_ATTR);
	mxArray* gencost = initArray(rgencost, ngencost, GENCOST_ATTR+max_order);
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

	prhs[0] = basemva;
	prhs[1] = bus;
	prhs[2] = gen;
	prhs[3] = branch;
	prhs[4] = areas;
	prhs[5] = gencost;

	mlxOpf(5, plhs, 6, prhs); // cout if first parameter is 0;

	return 0;

}

vector<string> split(const string s, char c)
{
	vector<string> v;	
	string::size_type i = 0;
	string::size_type j = s.find(c);
	while (j != string::npos)
	{
		v.push_back(s.substr(i,j-i));
		i = ++j;
		j = s.find(c,j);
		if (j == string::npos)
			v.push_back(s.substr(i,s.length()));
	}
	return v;
}

