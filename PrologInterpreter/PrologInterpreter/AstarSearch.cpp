#include "stdafx.h"
#include "astar.h"
#include "algorithm.h"
#include <ctime>
#include <cmath>
#include <utility>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stack>
#include <omp.h>

using namespace boost;

void SearchPrune::OfflineAstar(){

	if(flag == followsun){
		printf("cannot use astar search for followsun application\n");
		exit(1);
	}

	std::vector<configstack*> Openset;
	std::vector<configstack*> Closeset;
	std::vector<configstack> solutions;	

	//for the performance of each instance type
	float* random_sequential_io = (float*)malloc(types*randomsize*sizeof(float));
	float* random_random_io = (float*)malloc(types*randomsize*sizeof(float));
	float* random_network_up = (float*)malloc(types*randomsize*sizeof(float));
	float* random_network_down = (float*)malloc(types*randomsize*sizeof(float));
	float* random_tmp = (float*)malloc(types*10000*sizeof(float));
	//read from file	
	FILE* rFile;
	char str[1024];
	char buf[256];
	char *ptr, *ptr2;
	rFile = fopen("randio.csv","r");
	if(rFile == NULL){
		printf("cannot open randio.csv\n");
		exit(1);
	}
	for(int i=0; i<types*10000; i++){
		if(fgets(str,1024,rFile)!=NULL)
			random_tmp[i] = atof(str);
	}	
	for(int i=0; i<types; i++){
		for(int j=0; j<randomsize; j++){
			random_random_io[i*randomsize+j] = random_tmp[i*10000+j];//10000 is fixed
		}
	}
	rFile = fopen("seqio.csv","r");
	if(rFile == NULL){
		printf("cannot open seqio.csv\n");
		exit(1);
	}
	for(int i=0; i<types*10000; i++){
		if(fgets(str,1024,rFile)!=NULL)
			random_tmp[i] = atof(str);
	}	
	for(int i=0; i<types; i++){
		for(int j=0; j<randomsize; j++){
			random_sequential_io[i*randomsize+j] = random_tmp[i*10000+j];//10000 is fixed
		}
	}
	rFile = fopen("netup.csv","r");
	if(rFile == NULL){
		printf("cannot open netup.csv\n");
		exit(1);
	}
	for(int i=0; i<types*10000; i++){
		if(fgets(str,1024,rFile)!=NULL)
			random_tmp[i] = atof(str);
	}	
	for(int i=0; i<types; i++){
		for(int j=0; j<randomsize; j++){
			random_network_up[i*randomsize+j] = random_tmp[i*10000+j];//10000 is fixed
		}
	}
	rFile = fopen("netdown.csv","r");
	if(rFile == NULL){
		printf("cannot open netdown.csv\n");
		exit(1);
	}
	for(int i=0; i<types*10000; i++){
		if(fgets(str,1024,rFile)!=NULL)
			random_tmp[i] = atof(str);
	}	
	for(int i=0; i<types; i++){
		for(int j=0; j<randomsize; j++){
			random_network_down[i*randomsize+j] = random_tmp[i*10000+j];//10000 is fixed
		}
	}
	free(random_tmp);
	std::pair<vertex_iter, vertex_iter> vp;
	for(int i=0; i<dags.size(); i++){
		vp = vertices((*dags[i]->g));

		int quantile = dags[i]->meet_dl * randomsize;
		for(; vp.first != vp.second; vp.first++){
			//dag.g[*vp.first].probestTime = new float[types][randomsize];
			for(int t=0; t<types; t++){
				for(int j=0; j<randomsize; j++){
					(*dags[i]->g)[*vp.first].netUp[t*randomsize+j] = (*dags[i]->g)[*vp.first].trans_data * random_network_up[t*randomsize+j] / 8000;
					(*dags[i]->g)[*vp.first].netDown[t*randomsize+j] = (*dags[i]->g)[*vp.first].rec_data * random_network_down[t*randomsize+j] / 8000;
					(*dags[i]->g)[*vp.first].randomIO[t*randomsize+j] = (*dags[i]->g)[*vp.first].read_data / random_random_io[t*randomsize+j];
					(*dags[i]->g)[*vp.first].seqIO[t*randomsize+j] = (*dags[i]->g)[*vp.first].seq_data / random_sequential_io[t*randomsize+j];
					(*dags[i]->g)[*vp.first].probestTime[t*randomsize+j] = (*dags[i]->g)[*vp.first].cpuTime[t] + (*dags[i]->g)[*vp.first].netUp[t*randomsize+j]
						+ (*dags[i]->g)[*vp.first].netDown[t*randomsize+j] + (*dags[i]->g)[*vp.first].randomIO[t*randomsize+j] + (*dags[i]->g)[*vp.first].seqIO[t*randomsize+j];
					//(*dags[i]->g)[*vp.first].probestTime[t*randomsize+j] /= 60.0;
				}
				//calculate the estimate time as the expected value of the proestTime
				std::sort((*dags[i]->g)[*vp.first].probestTime+t*randomsize,(*dags[i]->g)[*vp.first].probestTime+(t+1)*randomsize-1);
				(*dags[i]->g)[*vp.first].estTime[t] = (*dags[i]->g)[*vp.first].probestTime[t*randomsize+quantile];
				printf("task: %d, type: %d, time: %f\n",*vp.first,t,(*dags[i]->g)[*vp.first].estTime[t]);
			}
			(*dags[i]->g)[*vp.first].assigned_type = 0;//initially all assigned to small
		}
	}

	configstack* initialstate = new configstack();
	configstack* feasible = new configstack();
	//float* exeTime = (float*)malloc(randomsize*sizeof(float));
	//first step: search for a feasible solution and use it as lower bound
	initialFeasible(initialstate,feasible);
	solutions.push_back(*feasible);
	Openset.push_back(initialstate);
	//Openset.push_back(feasible);
	for(int i=0; i<feasible->configurations.size(); i++)
		printf("%d, ",feasible->configurations[i]);
	float globalBestCost = feasible->fvalue;
	int numoftasks = feasible->configurations.size();

	//second step: start A* search using the found feasible solution	
	int searchcount = 0;
	while(!(Openset.empty() || searchcount > 10000)){
		//no need to sort, only find the smallest one is enough
		std::nth_element(Openset.begin(),Openset.end(),Openset.end(),configsortfunction);//sort from large to small,in order to reduce the erase complexity
		configstack* headnode = Openset.back();//back has the smallest fvalue

		//check if satisfy deadline 
		if(flag==deadlineapp)
			for(int i=0; i<numoftasks; i++)
				(*dags[0]->g)[i].assigned_type = headnode->configurations[i]; 
		bool constraint = solutionEvalconstraint(headnode);		
        //if satisfy users' requirement, select as the lower bound
		if(constraint){			 
			if(headnode->fvalue < globalBestCost){
				globalBestCost = headnode->fvalue;				
				solutions.push_back(*headnode);
				//remove headnode from openset and add it to closedset
				//printf("search prune find a solution with ratio: %f\n",ratio);
			}
        }
		Openset.erase(Openset.end()-1);
		Closeset.push_back(headnode);

		//for each neighboring node, how to include transformation!
		int nexttask = headnode->taskno + 1;
		if(nexttask < numoftasks){	
			int bound = types;
			int start = headnode->configurations[nexttask]+1;
			if(flag==ensemble) 	{
				bound = 2;
				start = 0;//only 0 or 1
			}
				
			for(int t=start; t<bound; t++){
				configstack* state = new configstack();
				state->taskno = nexttask;
				state->configurations = headnode->configurations;
				state->configurations[nexttask] = t;
				//is it a feasible solution?
				//dag.g[nexttask].assigned_type = t;		
				if(flag==deadlineapp){
					for(int ii=0; ii<numoftasks; ii++)
						(*dags[0]->g)[ii].assigned_type = state->configurations[ii];
				}
				float currentcost = solutionEvalcost(state);
				if(currentcost >= globalBestCost || std::find(Closeset.begin(),Closeset.end(),state) != Closeset.end()){//std::binary_search(Closeset.begin(),Closeset.end(),state)){
					//just ignore this configuration
				}else{
					std::vector<configstack*>::iterator iter = std::find(Openset.begin(),Openset.end(),state);
					if(iter == Openset.end()){//state is not in Openset
					//bool found = std::binary_search(Openset.begin(),Openset.end(),state)
					//if(!found){
						state->fvalue = currentcost;
						Openset.push_back(state);
					}else{
						//state is already in Openset
						printf("when would this happen?\n");
					}
				}				
			}//for t
		}	
		searchcount ++;
	}
	foundsolution = solutions.back();
	printf("Astar search return solution: \t ");
	for(int i=0; i<foundsolution.configurations.size(); i++)
		printf("%d, ",foundsolution.configurations[i]);
	printf("\n");
	//calculate the cumulative time distribution of the dag
	//float* cumulative=(float*)malloc(randomsize*sizeof(float));
	//solutionEvalconstraint(&solutions.back(),cumulative);
	//dags[0].cumulativetime = cumulative;

	//free(cumulative);
	free(random_sequential_io);
	free(random_random_io);
	free(random_network_up);
	free(random_network_down);
}
void SearchPrune::Simulate(){
	//execute according to the foundsolution
	if(flag == deadlineapp){
		//deadline application simulation
		std::pair<vertex_iter,vertex_iter> vp;
		for(vp=vertices(*dags[0]->g); vp.first!=vp.second; vp.first++)
			(*dags[0]->g)[*vp.first].assigned_type = foundsolution.configurations[*vp.first];
		Autoscaling* autoptimizer = new Autoscaling();
		autoptimizer->dags.push_back(dags[0]);
		autoptimizer->Simulate(false);

	}else if(flag == ensemble){
		EnsembleSPSS* ensembleopt = new EnsembleSPSS();		
		ensembleopt->plan = (int*)malloc(dags.size()*sizeof(int));
		for(int i=0; i<dags.size(); i++){
			ensembleopt->plan[i] = foundsolution.configurations[i];
			ensembleopt->dags.push_back( dags[i]);
		}
		ensembleopt->Simulate(false);
	
	}else if(flag == followsun){
	
	}else{
		printf("what's the application type for simulation?\n");
	}
}
bool configsortfunction(configstack* a, configstack* b)
{
	return (a->fvalue > b->fvalue); //sort from large to small
}


void conv(float* array1, float* array2, float* result, int length1, int length2){
	int resultlength = length1 + length2 - 1;
	for(int index=0; index < resultlength; index++){
		float tmp = 0.0;
		for(int k=0; k<length1; k++){
			if(index >= k && (index-k)<length2)
				tmp += array1[k]*array2[index-k];
		}
		result[index]=tmp;
	}
}
void calmaxdistr(float* array1, float* array2, float* result, int length1, int length2){
	int length = length1>length2?length1:length2;
	for(int i=0; i<length; i++)
		result[i] = 0.0;
	for(int i=0; i<length1; i++){
		for(int j=0; j<length2; j++){
			if(array1[i]!=0)
				printf("");
			if(i>=j){
				result[i] += array1[i]*array2[j]; 
			}else{//i<j
				result[j] += array1[i]*array2[j];
			}
		}
	}
}

float SearchPrune::solutionEvalcost(configstack* config)
{
	if(flag==deadlineapp)//deadline use case
		return deadlineEvalcost(dags[0]);
	else if(flag==ensemble){//ensemble use case
		std::vector<DAG*> pdags;
		for(int i=0; i<dags.size(); i++)
			pdags.push_back(dags[i]);
		return ensembleEvalcost(pdags,config);
	}
	//else if(flag==followsun)//follow the sun use case
	//	return followsunEvalcost();
	else {
		printf("solutionEval error\n");
		exit(1);
	}
}
bool SearchPrune::solutionEvalconstraint(configstack* config)
{
	if(flag==deadlineapp)//deadline use case
		return deadlineEvalconstraint(dags[0]);
	else if(flag==ensemble){//ensemble use case
		std::vector<DAG*> pdags;
		for(int i=0; i<dags.size(); i++)
			pdags.push_back(dags[i]);
		return ensembleEvalconstraint(pdags,config);
	}
	//else if(flag==followsun)//follow the sun use case
	//	return followsunEvalconstraint();
	else {
		printf("solutionEval error\n");
		exit(1);
	}
}
void SearchPrune::initialFeasible(configstack* config,configstack* feasible){
	if(flag==deadlineapp)
		return deadline_initialFeasible(config, dags[0],feasible);//only one dag in dags
	else if(flag==ensemble){
		std::vector<DAG*> pdags;
		for(int i=0; i<dags.size(); i++)
			pdags.push_back(dags[i]);
		return ensemble_initialFeasible(config,pdags,feasible);
	}
	//else if(flag==followsun)
	//	return followsun_initialFeasible(config,feasible);
	else{
		printf("initialFeasible error\n");
		exit(1);
	}
}