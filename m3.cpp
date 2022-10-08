/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "m1helper.h"
#include "globaldatabase.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "m3.h"
#include "m3helperS.h"

#include <vector>
#include <string> 
#include <iostream>
#include <set>
#include <utility>
#include <queue>

/*  
 * Running Tester: M3_Public_Func_Inter_Inter_Simple_Optimality_Interlaken_Switzerland
  Building   M3_Public_Func_Inter_Inter_Simple_Optimality_Interlaken_Switzerland
  Running    M3_Public_Func_Inter_Inter_Simple_Optimality_Interlaken_Switzerland
    load map: /cad2/ece297s/public/maps/interlaken_switzerland.streets.bin
    loading street
    loading street segments
    loading database
    load successful
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.interlaken_switzerland.cpp:77: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 12.45599705321318496
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.interlaken_switzerland.cpp:89: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 23.32163948485609239
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.interlaken_switzerland.cpp:133: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 16.85501184302877675
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.interlaken_switzerland.cpp:145: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 37.38523869222353113
    FAILURE: 1 out of 1 tests failed (4 failures).
    Test time: 0.00 seconds.
  Analyzing  M3_Public_Func_Inter_Inter_Simple_Optimality_Interlaken_Switzerland
    Testing (including setup) took: 0.13 sec

Running Tester: M3_Public_Func_Inter_Inter_Simple_Optimality_Interlaken_Switzerland
  Building   M3_Public_Func_Inter_Inter_Simple_Optimality_Interlaken_Switzerland
  Running    M3_Public_Func_Inter_Inter_Simple_Optimality_Interlaken_Switzerland
    load map: /cad2/ece297s/public/maps/interlaken_switzerland.streets.bin
    loading street
    loading street segments
    loading database
    load successful
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.interlaken_switzerland.cpp:77: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 12.45599705321318496
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.interlaken_switzerland.cpp:89: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 23.32163948485609239
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.interlaken_switzerland.cpp:133: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 16.85501184302877675
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.interlaken_switzerland.cpp:145: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 37.38523869222353113
    FAILURE: 1 out of 1 tests failed (4 failures).
    Test time: 0.00 seconds.
  Analyzing  M3_Public_Func_Inter_Inter_Simple_Optimality_Interlaken_Switzerland
    Testing (including setup) took: 0.14 sec
 * 
 * 
Running Tester: M3_Public_Func_Inter_Inter_Simple_Optimality_Toronto_Canada
  Building   M3_Public_Func_Inter_Inter_Simple_Optimality_Toronto_Canada
  Running    M3_Public_Func_Inter_Inter_Simple_Optimality_Toronto_Canada
    load map: /cad2/ece297s/public/maps/toronto_canada.streets.bin
    loading street
    loading street segments
    loading database
    load successful
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.toronto_canada.cpp:61: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 4.62426647177113903
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.toronto_canada.cpp:85: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 18.85224925437045229
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.toronto_canada.cpp:109: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 62.22903301067249515
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.toronto_canada.cpp:117: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 25.61771078977028537
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.toronto_canada.cpp:145: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 11.70982868575279845
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.toronto_canada.cpp:149: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 22.28322520455714439
    /cad2/ece297s/public//m3/tests/m3_func_public.inter_inter_simple_optimality.toronto_canada.cpp:157: error: Failure in find_path_between_intersections_simple_optimality: compute_path_travel_time(path, 0.00000000000000000) <= 6.44847057435294957
    FAILURE: 1 out of 1 tests failed (7 failures).
    Test time: 0.03 seconds.
  Analyzing  M3_Public_Func_Inter_Inter_Simple_Optimality_Toronto_Canada
    Testing (including setup) took: 8.06 sec


*/


// Returns a path (route) between the start intersection and the end
// intersection, if one exists. This routine should return the shortest path
// between the given intersections, where the time penalty to turn right or
// left is given by turn_penalty (in seconds).  If no path exists, this routine
// returns an empty (size == 0) vector.  If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the end intersection.
std::vector<StreetSegmentIndex> find_path_between_intersections(
		          const IntersectionIndex intersect_id_start, 
                          const IntersectionIndex intersect_id_end,
                          const double turn_penalty){
    
    std::vector<StreetSegmentIndex> invalid_path;
    
    if (intersect_id_start == intersect_id_end){
        return invalid_path;
    }
    
    if (!inRange(intersect_id_start, 0, getNumIntersections()) || !inRange(intersect_id_end, 0, getNumIntersections())){
        return invalid_path;
    }
    
    int total_intersection = getNumIntersections();
    
    nodeList.clear();
    nodeList.resize(total_intersection);
    
    std::vector<StreetSegmentIndex> path;
    
    nodeList[intersect_id_start].segment_ID = -1;
    nodeList[intersect_id_start].parent_ID = -1;
    nodeList[intersect_id_start].travel_time = 0.0;
    
    std::vector<bool> frontier_check(total_intersection);
    std::vector<bool> visited_check(total_intersection);
    
    //frontier frontier_T;
    
    std::priority_queue<frontier, std::vector<frontier>, comparator> priority;
    
    //put start into priority queue
    frontier frontier_start(intersect_id_start);
    priority.push(frontier_start);
    frontier_check[intersect_id_start] = true;
    
    while(!priority.empty()){
        //dequeue
        frontier top_frontier = priority.top();
        IntersectionIndex current_node = top_frontier.nodeID;
        frontier_check[current_node] = false;
        visited_check[current_node] = true;
        priority.pop();
        
        if (current_node == intersect_id_end){
            //traverse backwards
            while (current_node != intersect_id_start){
                auto iterator = path.begin();
                path.insert(iterator, nodeList[current_node].segment_ID);
                current_node = nodeList[current_node].parent_ID;
            }
//            for (auto current_segment = path.begin(); current_segment != path.end(); current_segment++){
//                std::cout<< "from" <<getInfoStreetSegment(*current_segment).from<< "to" << getInfoStreetSegment(*current_segment).to<<std::endl;
//            }
            return path;
        }
        
        int number_of_adjacents = G_DATABASE.intersectionDatabase[current_node].adjacentID.size();
        for (IntersectionIndex connected = 0; connected< number_of_adjacents; connected++){
            IntersectionIndex adjacent_ID = G_DATABASE.intersectionDatabase[current_node].adjacentID[connected];
            
            if (visited_check[adjacent_ID]){
                continue; //skip the rest
            }else if(frontier_check[adjacent_ID]){
                double adjacent_travel_time = find_street_segment_travel_time(G_DATABASE.connected_segment[current_node].find(adjacent_ID)->second);
                adjacent_travel_time += A_start_weight(current_node, adjacent_ID);
//                if (adjacent_travel_time < nodeList[adjacent_ID].travel_time){
                if (nodeList[current_node].parent_ID != -1){
                    if (getInfoStreetSegment(nodeList[current_node].segment_ID).streetID != getInfoStreetSegment(G_DATABASE.connected_segment[current_node].find(adjacent_ID)->second).streetID){
                        adjacent_travel_time += turn_penalty;
                    }
                    adjacent_travel_time += nodeList[current_node].travel_time;
                }
                if (adjacent_travel_time < nodeList[adjacent_ID].travel_time){
                    nodeList[adjacent_ID].segment_ID = G_DATABASE.connected_segment[current_node].find(adjacent_ID)->second;
                    nodeList[adjacent_ID].parent_ID = current_node;
                    nodeList[adjacent_ID].travel_time = adjacent_travel_time;
                    frontier updated_node(adjacent_ID);
                    priority.push(updated_node);
                }
            }else{
                double adjacent_travel_time = find_street_segment_travel_time(G_DATABASE.connected_segment[current_node].find(adjacent_ID)->second);
                adjacent_travel_time += A_start_weight(current_node, adjacent_ID);
                nodeList[adjacent_ID].segment_ID = G_DATABASE.connected_segment[current_node].find(adjacent_ID)->second;
                nodeList[adjacent_ID].parent_ID = current_node;
                if (nodeList[current_node].parent_ID != -1){
                    if (getInfoStreetSegment(nodeList[current_node].segment_ID).streetID != getInfoStreetSegment(nodeList[adjacent_ID].segment_ID).streetID){
                        adjacent_travel_time += turn_penalty;
                    }
                    adjacent_travel_time += nodeList[current_node].travel_time;
                }
                nodeList[adjacent_ID].travel_time = adjacent_travel_time;
                frontier new_node(adjacent_ID);
                priority.push(new_node);
                frontier_check[adjacent_ID] = true;
            }
        }
    }
    
    return invalid_path;
}

// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0.  The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path.  If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double compute_path_travel_time(const std::vector<StreetSegmentIndex>& path, 
                                const double turn_penalty){
    int path_size = path.size();
    double total_travel_time = 0;
    double street_segment_travel_time = 0;
    
    //check whether the path is empty
    if (path_size == 0){
        return 0; //which is 0 in this case.
    }else{
        street_segment_travel_time = find_street_segment_travel_time(path[0]);
        total_travel_time = street_segment_travel_time;
        if (path_size == 1){
            return total_travel_time;
        }else{
            for (int path_ID = 1; path_ID < path_size; path_ID++){
                street_segment_travel_time = find_street_segment_travel_time(path[path_ID]);
                total_travel_time += street_segment_travel_time;
            
                //check for turn penalty                            
                if (getInfoStreetSegment(path[path_ID]).streetID != getInfoStreetSegment(path[path_ID - 1]).streetID){
                    total_travel_time += turn_penalty;
                }
            }
            return total_travel_time;   
        }    
    }
}

// Returns the time required to "walk" along the path specified, in seconds.
// The path is given as a vector of street segment ids. The vector can be of
// size = 0, and in this case, it the function should return 0. The walking
// time is the sum of the length/<walking_speed> for each street segment, plus
// the given turn penalty, in seconds, per turn implied by the path. If there
// is no turn, then there is no penalty.  As mentioned above, going from Bloor
// Street West to Bloor street East is considered a turn
double compute_path_walking_time(const std::vector<StreetSegmentIndex>& path, 
                                 const double walking_speed, 
                                 const double turn_penalty){
   
    int path_size = path.size();
    double total_walking_time = 0;
    double street_segment_walking_time = 0;
    
    //check whether the path is empty
    if (path_size == 0){
        return 0; //which is 0 in this case.
    }else{
        street_segment_walking_time = G_DATABASE.street_segment_length[path[0]] / walking_speed;
        total_walking_time = street_segment_walking_time;
        if (path_size == 1){
            return total_walking_time;
        }else{
            for (int path_ID = 1; path_ID < path_size; path_ID++){
                street_segment_walking_time = G_DATABASE.street_segment_length[path[path_ID]] / walking_speed;
                total_walking_time += street_segment_walking_time;
                        
                //check for turn penalty                            
                if (getInfoStreetSegment(path[path_ID]).streetID != getInfoStreetSegment(path[path_ID - 1]).streetID){
                    total_walking_time += turn_penalty;
                }
            }
            return total_walking_time;   
        }    
    }
}



// This is an "uber pool"-like function. The idea is to minimize driving travel
// time by walking to a pick-up intersection (within walking_time_limit secs)
// from start_intersection while waiting for the car to arrive.  While walking,
// you can ignore speed limits of streets and just account for given
// walking_speed [m/sec]. However, pedestrians should also wait for traffic
// lights, so turn_penalty applies to whether you are driving or walking.
// Walking in the opposite direction of one-way streets is fine. Driving is
// NOT!  The routine returns a pair of vectors of street segment ids. The first
// vector is the walking path starting at start_intersection and ending at the
// pick-up intersection. The second vector is the driving path from pick-up
// intersection to end_interserction.  Note that a start_intersection can be a
// pick-up intersection. If this happens, the first vector should be empty
// (size = 0).  If there is no driving path found, both returned vectors should
// be empty (size = 0). 
// If the end_intersection turns out to be within the walking time limit, 
// you can choose what to return, but you should not crash. If your user 
// interface can call this function for that case, the UI should handle
// whatever you return in that case.
std::pair<std::vector<StreetSegmentIndex>, std::vector<StreetSegmentIndex>> 
         find_path_with_walk_to_pick_up(
                          const IntersectionIndex start_intersection, 
                          const IntersectionIndex end_intersection,
                          const double turn_penalty,
                          const double walking_speed, 
                          const double walking_time_limit){
    std::pair<std::vector<StreetSegmentIndex>, std::vector<StreetSegmentIndex>> invalid_path;
    return invalid_path;
}

double A_start_weight(IntersectionIndex current, IntersectionIndex destination){
    LatLon current_position = getIntersectionPosition(current);
    LatLon destination_position = getIntersectionPosition(destination);
    double rest_distance = find_distance_between_two_points(std::make_pair(current_position, destination_position));
    return rest_distance / G_DATABASE.average_speed_limit;
}

