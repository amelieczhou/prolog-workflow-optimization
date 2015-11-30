//cost optimization with deadline constraint problem
//a solution is the instance configuration for all tasks
#include "stdafx.h"
#include "astar.h"
#include <stack>

float deadlineEvalcost(DAG* dag){

	float totalcost = 0;
	float onehour = 3600.0; //seconds
	//round up to 1 hour, use transformations to fully utilize partial hours
	std::pair<vertex_iter,vertex_iter> vp;
	vp = vertices((*dag->g));
	for(; vp.first!=vp.second; vp.first++){
		float taskcost = 0;
		int config = (*dag->g)[*vp.first].assigned_type;
		for(int i=0; i<randomsize; i++)
			taskcost += (((*dag->g)[*vp.first].probestTime[config*randomsize+i]+OnDemandLag)/onehour)*priceOnDemand[config];//ceil?
		totalcost += taskcost / randomsize;
	}
	return totalcost;
}
bool deadlineEvalconstraint(DAG* dag){
	
	std::pair<vertex_iter,vertex_iter> vp;
	float* exeTime = (float*)malloc(randomsize*sizeof(float));
	//run this function after each task is assigned with a instance type
	//in this way, calculate as distribution
	int cumulativesize = 4000;
	vp = vertices((*dag->g));
	for(; vp.first!=vp.second; vp.first++){
		(*dag->g)[*vp.first].tag = false; //not yet calculate its cumulative time
		for(int i=0; i<cumulativesize; i++)
			(*dag->g)[*vp.first].cumulativeTime[i] = 0;
	}
	vp = vertices((*dag->g));
	float *tmpTime = (float*)malloc(cumulativesize*sizeof(float));
	for(; vp.first!=vp.second; vp.first++){
		for(int i=0; i<cumulativesize; i++)
			tmpTime[i] = 0;
		//find all its parents
		int config = (*dag->g)[*vp.first].assigned_type;
		in_edge_iterator in_i, in_end;
		edge_descriptor e;
		boost::tie(in_i, in_end) = in_edges(*vp.first, (*dag->g));
		if(in_i == in_end) {
			for(int i=0; i<cumulativesize; i++)
				(*dag->g)[*vp.first].cumulativeTime[i] = (*dag->g)[*vp.first].probestTime[config*randomsize+i]+OnDemandLag;
			(*dag->g)[*vp.first].tag = true;
		}
		else{
			for (; in_i != in_end; ++in_i) {
				e = *in_i;
				Vertex src = source(e, (*dag->g));					
				if(!(*dag->g)[src].tag) { //this task hasn't been assigned cumulative time
					printf("????\n");
					break;
				}else{
					for(int i=0; i<randomsize; i++){
						tmpTime[i]= tmpTime[i]>(*dag->g)[src].cumulativeTime[i]?tmpTime[i]:(*dag->g)[src].cumulativeTime[i];
					}
				}
			}
		}
		for(int i=0; i<randomsize; i++){
			(*dag->g)[*vp.first].cumulativeTime[i] = tmpTime[i] + (*dag->g)[*vp.first].probestTime[config*randomsize+i] + OnDemandLag;
		}
		(*dag->g)[*vp.first].tag = true;
	}
	//if(dag.type == montage)
		for(int i=0; i<randomsize; i++){
			exeTime[i] = (*dag->g)[*(vp.second-1)].cumulativeTime[i];////////////////////////////
	}
	free(tmpTime);
	int count = 0;
	for(int i=0; i<randomsize; i++){
		if(exeTime[i]<=dag->deadline)
			count ++;
	}
	float ratio = (float)count / (float)randomsize;
	free(exeTime);
	//if satisfy users' requirement, select as the lower bound
	if(ratio >= dag->meet_dl)
		return true;
	else return false;

	/////////////////////////monte carlo method/////////////////////////////
	/*int count = 0;
	for(int iter=0; iter<randomsize; iter++){
		float exeTime = 0;
		std::pair<vertex_iter,vertex_iter> vp;	
		//run this function after each task is assigned with a instance type
		//in this way, calculate as distribution
		vp = vertices((*dag->g));
		for(; vp.first!=vp.second; vp.first++)
			(*dag->g)[*vp.first].cumulativeTime[0] = 0;
		vp = vertices((*dag->g));
		for(; vp.first!=vp.second; vp.first++){			
			float tmpTime = 0;
			//find all its parents
			int config = (*dag->g)[*vp.first].assigned_type;
			in_edge_iterator in_i, in_end;
			edge_descriptor e;
			boost::tie(in_i, in_end) = in_edges(*vp.first, (*dag->g));
			if(in_i == in_end) {
				(*dag->g)[*vp.first].cumulativeTime[0] = (*dag->g)[*vp.first].probestTime[config*randomsize+iter]+OnDemandLag;
				(*dag->g)[*vp.first].tag = true;
			}
			else{
				for (; in_i != in_end; ++in_i) {
					e = *in_i;
					Vertex src = source(e, (*dag->g));					
					if(!(*dag->g)[src].tag) { //this task hasn't been assigned cumulative time
						printf("????\n");
						break;
					}else{
						tmpTime= tmpTime>(*dag->g)[src].cumulativeTime[0]?tmpTime:(*dag->g)[src].cumulativeTime[0];					
					}
				}
			}
			(*dag->g)[*vp.first].cumulativeTime[0] = tmpTime + (*dag->g)[*vp.first].probestTime[config*randomsize+iter] + OnDemandLag;		
			(*dag->g)[*vp.first].tag = true;
		}
		exeTime = (*dag->g)[*(vp.second-1)].cumulativeTime[0];
		if(exeTime <= dag->deadline)
			count++;
	}	
	float ratio = (float)count / (float)randomsize;
	//free(exeTime);
	//if satisfy users' requirement, select as the lower bound
	if(ratio >= dag->meet_dl)
		return true;
	else return false;*/
}

void deadline_initialFeasible(configstack* initialstate, DAG* dag, configstack* feasible){

	std::stack<configstack*> DAGstack;

	std::pair<vertex_iter, vertex_iter> vp;
	//profile section, find the cheapest initialstate according to the execution time of tasks on different instance type
	for(vp=vertices((*dag->g)); vp.first!=vp.second; vp.first++){
		//float tmp = (*dag->g)[*vp.first].estTime[0];
		//int initialtype = 0;
		////do the following because the performance distribution is not ideal as we expected 
		////if ideal, initialtype should be 0
		//for(int instype=0; instype<types; instype++)
		//	if((*dag->g)[*vp.first].estTime[instype]*pow(2.0,instype)<tmp){
		//		tmp = (*dag->g)[*vp.first].estTime[instype]*pow(2.0,instype);
		//		initialtype = instype;
		//	}
		//initialstate->configurations.push_back(initialtype);
		initialstate->configurations.push_back(0);
	}
	initialstate->taskno = -1;
	vp=vertices((*dag->g));
	int numoftasks = (*vp.second - *vp.first);
	float globalBestCost = 1000000;

	DAGstack.push(initialstate);
    bool continuesearch = true;
	//find a lower bound first
	do{
		for(int i=0; i<numoftasks; i++)
			(*dag->g)[i].assigned_type = DAGstack.top()->configurations[i];  
		bool evalresult = deadlineEvalconstraint(dag);
		//if satisfy users' requirement, select as the lower bound
		if(evalresult){
			float cost = deadlineEvalcost(dag);
			if(cost<globalBestCost)
				globalBestCost = cost;
			DAGstack.top()->fvalue = cost;
			continuesearch = false; //out of the while loop
			//printf("initial ratio in search prune: %f\n",ratio);
		}else{
			//Closeset.push_back(DAGstack.top());
			int nexttask = DAGstack.top()->taskno + 1;
			if(nexttask < numoftasks){
				configstack* state = new configstack();
				state->configurations=DAGstack.top()->configurations;
				state->taskno = nexttask;
				for(int t=0; t<types; t++){
					if(!DAGstack.top()->childcolor[t]){ //type t has not been visited
						state->configurations[nexttask] = t;
						DAGstack.top()->childcolor[t] = true;
						DAGstack.push(state);
						break;                                          
					}
					if(t == types-1){ //all types have been visited
						DAGstack.pop();
						//????
					}
				}
				/*for(int i=0; i<numoftasks; i++)
					(*dag->g)[i].assigned_type = DAGstack.top()->configurations[i];  */    
			}else{ //nexttask >= numoftasks                         
				DAGstack.pop();
					//?????
			}
		}
		if(DAGstack.empty()) {
			printf("cannot find one valid solution\n");
			exit(1);
		}
	}while(continuesearch);
	*feasible = *DAGstack.top();	
	while(!DAGstack.empty())//clear
		DAGstack.pop();


	return;
}