    void drawPath(const GPath& path, const GPaint& paint) override {
	GPath::Edger edgerator(path);
	vector<Edge> edges(0);
	GPoint* endpts = new GPoint[2]; 
	GShader* shader = paint.getShader(); 
	GFilter* paint_filter = paint.getFilter(); 
	if(shader){
                 if(!shader -> setContext(current_layer.CTM)){
                         return;
                 }
        }

	while(edgerator.next(endpts) != GPath::Verb::kDone){  
		current_layer.CTM.mapPoints(endpts, endpts, 2); 
		vector<Edge> new_edges = Clipper(endpts[0], endpts[1], 1); //if all else fails, debug clipper :(
		for(int i = 0; i < new_edges.size(); i++){
			//printf("Pt0: (%f, %f) , Pt1: (%f, %f)\n", endpts[0].fX, endpts[0].fY, endpts[1].fX, endpts[1].fY); 
			printf("Points: P0: (%f, %f) , P1: (%f, %f) -> edge: (TopY: %d , BotY: %d , currX: %f, slope: %f)\n", endpts[0].fX, endpts[0].fY, endpts[1].fX, endpts[1].fY, new_edges.at(i).top_y, new_edges.at(i).bot_y, new_edges.at(i).curr_x, new_edges.at(i).slope); 	
			edges.push_back(new_edges.at(i));
		}   
	} 
		
		if(edges.size() >= 2){
			sort(edges.begin(), edges.end());
			printf("SORTED EDGES: \n");
			for(int i = 0; i < edges.size(); i++){
				printf("edge: (TopY: %d , BotY: %d , currX: %f , slope: %f , wind: %d)\n", edges.at(i).top_y, edges.at(i).bot_y, edges.at(i).curr_x, edges.at(i).slope, edges.at(i).winding); 
			}

			int top = edges.front().top_y, bottom = edges.back().bot_y; 

			for(int y = top; y < bottom;){ //may need to create variables for starting top and bottom due to resorts in x   

				int wind = 0, curr_index = 0; //not sure if all these should be in for-loop
				Edge curr_edge = edges.front(); //
                                Edge next; //

				//printf("New edge vector of Size %d:\n", edges.size()); 

				int x0, x1; 
				while(curr_edge.top_y <= y){
					if(wind == 0){
						x0 = GRoundToInt(curr_edge.curr_x); 
					}
					wind += curr_edge.winding; 
					if(wind == 0){
						x1 = GRoundToInt(curr_edge.curr_x);
						GPixel *sources = new GPixel[x1-x0]; 
						if(shader){
							shader -> shadeRow(x0, y, x1-x0, sources); 
						}
						else {
							for(int i = 0; i < x1-x0; i++){
								sources[i] = HelperFunctions::convertColorToPixel(paint.getColor()); 
							}
						}
						if(paint_filter){
							paint_filter -> filter(sources, sources, x1-x0); 
						}
						Blit(y, x0, x1, sources, paint.getBlendMode());
					}
					int next_index = curr_index + 1;
					if(next_index >= edges.size()){
						break; 
					}
					next = edges.at(next_index);
					if(curr_edge.bot_y == y + 1){
						edges.erase(edges.begin() + curr_index);
						next_index--;  
					}
					else {
						curr_edge.curr_x += curr_edge.slope; 
						resortBackwards(curr_index, edges); 
 
					}
					curr_edge = next;
					curr_index = next_index; 
				}
				y++; 
				while(curr_edge.top_y == y){
					int next_index = curr_index + 1; 
					if(next_index < edges.size()){
						next = edges.at(next_index); 
					}   
					//printf("Before resort, size = %zu\n", edges.size()); 
					//printf("curr_edge: %f and curr_index: %d\n", curr_edge.curr_x, curr_index); 
					//for(int i = 0; i < edges.size(); i++){printf("edge %d: %f\n", i, edges.at(i).curr_x);}
					resortBackwards(curr_index, edges); 
					if(next_index == edges.size()){
						break; 
					}
					curr_edge = next;
					curr_index = next_index; 
					//printf("After resort, size = %zu\n", edges.size()); 
					//for(int i = 0; i < edges.size(); i++){printf("edge %d: %f\n", i, edges.at(i).curr_x);}
					}
			  }
	}
	}
    
    