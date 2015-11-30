bool deadlineEvalconstraint(DAG* dag){
	int count = 0;
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
	free(exeTime);
	//if satisfy users' requirement, select as the lower bound
	if(ratio >= dag->meet_dl)
		return true;
	else return false;
}
