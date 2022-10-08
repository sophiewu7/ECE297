/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <vector>
#include <string> 
#include <iostream>
#include <set>
#include <utility>
#include <queue>
#include <thread>
#include "globaldatabase.h"
#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "m3helper.h"
#include "m4.h"
#include "m4helper.h"
#include "check.h"

//multithreading

std::vector<node> node_List;
bool m4_comparator::operator()(const frontier first, const frontier second){
        return (node_List[first.nodeID].travel_time + node_List[first.nodeID].rest_time) > (node_List[second.nodeID].travel_time + node_List[second.nodeID].rest_time);
}

std::vector<CourierSubpath> traveling_courier(
		            const std::vector<DeliveryInfo>& deliveries,
	       	            const std::vector<int>& depots, 
		            const float turn_penalty, 
		            const float truck_capacity){
    
    auto clock_start_time = std::chrono::high_resolution_clock::now();
    
    
    //Check Validity of the Inputs
    if (deliveries.empty() || depots.empty()){
        std::vector<CourierSubpath> empty;
        return empty;
    }
    
    ///////////////////////////////////Determine which depot to start////////////////////////////////////////////////
    
    //calculate average latlon of all the pick up points that need to visit
    //to determine the best start depot
    double lat = 0.0;
    double lon = 0.0;

    //Check Validity of Delivery Item's Weight and Calculate LatLon for finding the best depot
    for (auto current_item = deliveries.begin(); current_item != deliveries.end(); current_item++){
        //If any one item is overweight return empty
        if (current_item -> itemWeight > truck_capacity){ 
            std::vector<CourierSubpath> empty;
            return empty;
        }
        lat += getIntersectionPosition(current_item -> pickUp).lat();
        lon += getIntersectionPosition(current_item -> pickUp).lon();
    }
        
    //lat = lat / deliveries.size();
    //lon = lon / deliveries.size();
    //LatLon average_latlon = LatLon (lat,lon);
        
    //Choose the depot that is closest to average_latlon to start
    //IntersectionIndex start = *depots.begin();
    //double min_lat = getIntersectionPosition(start).lat();
    //double shortest_distance = find_distance_between_two_points (std::make_pair(average_latlon, getIntersectionPosition(*depots.begin())));
    /*for (auto current_depot = std::next(depots.begin()); current_depot != depots.end(); current_depot++){
        if (find_distance_between_two_points(std::make_pair(average_latlon, getIntersectionPosition(*current_depot))) < shortest_distance){
            shortest_distance = find_distance_between_two_points(std::make_pair(average_latlon, getIntersectionPosition(*current_depot)));
            start = *current_depot;
        }
    }*/
    /*
    for (auto current_depot = std::next(depots.begin()); current_depot != depots.end(); current_depot++){
        if (getIntersectionPosition(*current_depot).lat() < min_lat){
            //shortest_distance = find_distance_between_two_points(std::make_pair(average_latlon, getIntersectionPosition(*current_depot)));
            min_lat = getIntersectionPosition(*current_depot).lat();
            start = *current_depot;
        }
    }*/
    
    ///////////////////////////////////Pre compute travel time and path////////////////////////////////////////////////
    //stores all intersections
    int total_deliveries = deliveries.size();
    
    std::vector<IntersectionIndex> picks_and_drops;
    std::vector<std::pair<bool, int>> pick_up_check(getNumIntersections());
    std::vector<std::pair<bool, int>> drop_off_check(getNumIntersections());
    for (int current_item = 0; current_item < total_deliveries; current_item++){
        picks_and_drops.push_back(deliveries[current_item].pickUp);
        picks_and_drops.push_back(deliveries[current_item].dropOff);
        pick_up_check[deliveries[current_item].pickUp] = std::make_pair(true, current_item);
        drop_off_check[deliveries[current_item].dropOff] = std::make_pair(true, current_item);
    }
    removeDuplicates(picks_and_drops);
    
    //initialize the map "delivery_actions" to keep track of which delivery is picked or
    //dropped at which station 
    //the detailed info is to be initialized in the while loop.
    std::unordered_map<IntersectionIndex, std::vector<stationInfo> > delivery_actions;
    std::unordered_map<IntersectionIndex, bool> intersection_is_drop_off;
    
    for (auto stop = 0; stop < picks_and_drops.size(); stop++) {
        std::vector<stationInfo> info;
        delivery_actions.insert(std::make_pair(picks_and_drops[stop], info));
        
    }    
    
    std::vector <std::multimap<double, std::pair<std::vector<StreetSegmentIndex>, int> > > pick_up_to;
    std::vector <std::multimap<double, std::pair<std::vector<StreetSegmentIndex>, int> > > drop_off_to;

    //we may want to pre-compute all the path travel time that might be used
    //double is the travel time computed by m3 function
    //int is the index in vector of the destination (not IntersectionID)
    std::vector<std::multimap<double, int>> pick_up_to_pick_up;
    std::vector<std::multimap<double, int>> pick_up_to_drop_off;
    std::vector<std::multimap<double, int>> drop_off_to_drop_off;
    std::vector<std::multimap<double, int>> drop_off_to_pick_up;
    
    //also record all the path
    std::vector<std::vector<std::vector<StreetSegmentIndex>>> pick_up_to_pick_up_path;
    std::vector<std::vector<std::vector<StreetSegmentIndex>>> pick_up_to_drop_off_path;
    std::vector<std::vector<std::vector<StreetSegmentIndex>>> drop_off_to_drop_off_path;
    std::vector<std::vector<std::vector<StreetSegmentIndex>>> drop_off_to_pick_up_path;
    
    //compute all the path and travel time here
    //compute pick up to pick up
    //#pragma omp parallel for
    for (int current = 0; current < deliveries.size(); current++){
        IntersectionIndex pick = deliveries[current].pickUp;
        IntersectionIndex drop = deliveries[current].dropOff;
        
        std::unordered_map<IntersectionIndex, std::vector<StreetSegmentIndex>> pick_map = multi_destination_path(
		          pick, 
                          picks_and_drops,
                          turn_penalty);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::unordered_map<IntersectionIndex, std::vector<StreetSegmentIndex>> drop_map = multi_destination_path(
		          drop, 
                          picks_and_drops,
                          turn_penalty);
        //std::cout<<"2"<<std::endl;
        
        std::multimap<double, int> this_picktopick_map;
        std::multimap<double, int> this_picktodrop_map;
        std::multimap<double, int> this_droptodrop_map;
        std::multimap<double, int> this_droptopick_map;
        
        std::vector<std::vector<StreetSegmentIndex>> this_picktopick_vector;
        std::vector<std::vector<StreetSegmentIndex>> this_picktodrop_vector; 
        std::vector<std::vector<StreetSegmentIndex>> this_droptodrop_vector;
        std::vector<std::vector<StreetSegmentIndex>> this_droptopick_vector;

        for (int chason = 0; chason < deliveries.size(); chason++){
            IntersectionIndex pick_inner = deliveries[chason].pickUp;
            IntersectionIndex drop_inner = deliveries[chason].dropOff;
            
            //std::cout<<pick_inner<<std::endl<<drop_inner<<std::endl;
            
            if (pick_map.find(pick_inner) == pick_map.end() || pick_map.find(drop_inner) == pick_map.end() || drop_map.find(drop_inner) == drop_map.end() || drop_map.find(pick_inner) == drop_map.end()){
                std::vector<CourierSubpath> no_path;
                return no_path;
            }

            std::vector<StreetSegmentIndex> picktopick_path = pick_map.find(pick_inner)->second;
            std::vector<StreetSegmentIndex> picktodrop_path = pick_map.find(drop_inner)->second;
            std::vector<StreetSegmentIndex> droptodrop_path = drop_map.find(drop_inner)->second;
            std::vector<StreetSegmentIndex> droptopick_path = drop_map.find(pick_inner)->second;

            this_picktopick_map.insert(std::make_pair(compute_path_travel_time(picktopick_path, turn_penalty), chason));
            this_picktodrop_map.insert(std::make_pair(compute_path_travel_time(picktodrop_path, turn_penalty), chason));
            this_droptodrop_map.insert(std::make_pair(compute_path_travel_time(droptodrop_path, turn_penalty), chason));
            this_droptopick_map.insert(std::make_pair(compute_path_travel_time(droptopick_path, turn_penalty), chason));

            this_picktopick_vector.push_back(picktopick_path);
            this_picktodrop_vector.push_back(picktodrop_path);
            this_droptodrop_vector.push_back(droptodrop_path);
            this_droptopick_vector.push_back(droptopick_path);
        }
        
        pick_up_to_pick_up.push_back(this_picktopick_map);
        pick_up_to_drop_off.push_back(this_picktodrop_map);
        drop_off_to_drop_off.push_back(this_droptodrop_map);
        drop_off_to_pick_up.push_back(this_droptopick_map);
        
        pick_up_to_pick_up_path.push_back(this_picktopick_vector);
        pick_up_to_drop_off_path.push_back(this_picktodrop_vector);
        drop_off_to_drop_off_path.push_back(this_droptodrop_vector);
        drop_off_to_pick_up_path.push_back(this_droptopick_vector);
    }
    
   
/////////////////////////////////////////////Greedy Algorithm//////////////////////////////////////////////////////////////////////////////    
        
//    std::vector<bool> delivered; //a vector tracking the delivery status of all the items
//    std::vector<bool> picked; //a vector tracking the pickup status of all the items
//    for (int current = 0; current < deliveries.size(); current ++){ //initialize with false
//        delivered.push_back(false);
//        picked.push_back(false);
//    }
//    
//    float current_weight = 0.0; //current net weight of items on truck
//        
//    std::vector<CourierSubpath> result_path;
//    
//    //starting from now on, use greedy
//    bool all_delivered = false;
//    bool drop_or_pick = false; //track current position, true for drop, false for pick
    
    std::vector <IntersectionIndex> first_p;
    for (auto current_p = deliveries.begin(); current_p != deliveries.end(); current_p++) {
        first_p.push_back(current_p->pickUp);
    }
    
    std::unordered_map<IntersectionIndex, std::vector < StreetSegmentIndex>> first_pick_map = multi_destination_path(
            depots[0],
            first_p,
            turn_penalty);

    std::multimap<double, int> first_multimap;

    for (auto i = 0; i < deliveries.size(); i++) {
        IntersectionIndex sophie = deliveries[i].pickUp;
        std::vector<StreetSegmentIndex> haha = first_pick_map.find(sophie)->second;
        first_multimap.insert(std::make_pair(compute_path_travel_time(haha, turn_penalty), i));
    }
    
    double chason = first_multimap.begin()->first;
    double chason_2 = std::numeric_limits<double>::max();
    CourierSubpath first_path;
    first_path.start_intersection = depots[0];
    first_path.end_intersection = deliveries[first_multimap.begin()->second].pickUp;
    first_path.subpath = first_pick_map.find(first_path.end_intersection)->second;
    std::vector<unsigned> indices;
    first_path.pickUp_indices = indices;
    int chason_index = first_multimap.begin()->second;
    int chason_index_two = first_multimap.begin()->second;
    
    CourierSubpath second_path;
    second_path.start_intersection = depots[0];
    second_path.end_intersection = deliveries[first_multimap.begin()->second].pickUp;
    second_path.subpath = first_pick_map.find(second_path.end_intersection)->second;
    std::vector<unsigned> second_indices;
    second_path.pickUp_indices = second_indices;
    
    if (depots.size() == 1){
        chason_index_two = -1;
    }
    for (int j = 1; j < depots.size(); j++) {

        std::unordered_map<IntersectionIndex, std::vector < StreetSegmentIndex>> chris_pick_map = multi_destination_path(
                depots[j],
                first_p,
                turn_penalty);

        std::multimap<double, int> chris_multimap;

        for (auto i = 0; i < deliveries.size(); i++) {
            IntersectionIndex sophie = deliveries[i].pickUp;
            std::vector<StreetSegmentIndex> chao = chris_pick_map.find(sophie)->second;
            chris_multimap.insert(std::make_pair(compute_path_travel_time(chao, turn_penalty), i));
        }
        
        if (chris_multimap.begin()->first < chason){
            first_path.start_intersection = depots[j];
            first_path.end_intersection = deliveries[chris_multimap.begin()->second].pickUp;
            first_path.subpath = chris_pick_map.find(first_path.end_intersection)->second;
            chason = chris_multimap.begin()->first;
            chason_index = chris_multimap.begin()->second;
        }
        else if (chris_multimap.begin()->first < chason_2 && chris_multimap.begin()->first > chason){
            second_path.start_intersection = depots[j];
            second_path.end_intersection = deliveries[chris_multimap.begin()->second].pickUp;
            second_path.subpath = chris_pick_map.find(second_path.end_intersection)->second;
            chason_2 = chris_multimap.begin()->first;
            chason_index_two = chris_multimap.begin()->second;
        }
    }
       
    //result_path.push_back(first_path);
    
    std::vector<int> chasons;
    chasons.push_back(chason_index);
    if (chason_index_two != -1){
        chasons.push_back(chason_index_two);
    }
    std::vector<std::vector<CourierSubpath>> result_path_thread(deliveries.size());
    std::vector<CourierSubpath> final_path_thread(deliveries.size());
    std::vector<IntersectionIndex> path_intersections;
    
    std::cout << "chasons size" << chasons.size() <<std::endl;
    
    #pragma omp parallel for
    for (int k = 0; k <chasons.size(); k++){
        int current_index = 0;
        if (k == 0){
            current_index = chason_index;
        }else if (k == 1 && chason_index_two != -1){
            current_index = chason_index_two;
        }else if (k == 1 && chason_index_two == -1){
            current_index = chason_index;
            continue;
        }
        std::vector<bool> delivered; //a vector tracking the delivery status of all the items
        std::vector<bool> picked; //a vector tracking the pickup status of all the items
        for (int current = 0; current < deliveries.size(); current ++){ //initialize with false
            delivered.push_back(false);
            picked.push_back(false);
        }

       float current_weight = 0.0; //current net weight of items on truck

       std::vector<CourierSubpath> result_path;

       //starting from now on, use greedy
       bool all_delivered = false;
       bool drop_or_pick = false; //track current position, true for drop, false for pick
        CourierSubpath final_path;
        //current_weight = 0;
        
        
        while (!all_delivered) {
            CourierSubpath this_path;

            if (drop_or_pick) { //currently at a dropoff
                //see what to drop and see what to pick (if this happens to be a pickup)
                std::vector<unsigned int> pick_up_indices;

                for (auto current_item = 0; current_item < deliveries.size(); current_item++) {
                    if (!delivered[current_item] && deliveries[current_item].dropOff == deliveries[current_index].dropOff && picked[current_item]) { //delivered
                        delivered[current_item] = true;
                        current_weight -= deliveries[current_item].itemWeight;
                    }
                    if (!picked[current_item] && (deliveries[current_item].pickUp == deliveries[current_index].dropOff) && (current_weight + deliveries[current_item].itemWeight <= truck_capacity)) { //can pick up
                        picked[current_item] = true;
                        current_weight += deliveries[current_item].itemWeight;
                        pick_up_indices.push_back(current_item);
                    }
                }

                double closest_drop_time = 999999999999;
                std::vector<StreetSegmentIndex> closest_drop_path;
                int closest_drop_index = -1;

                //the closest available drop_off location
                for (auto current_drop_off = drop_off_to_drop_off[current_index].begin(); current_drop_off != drop_off_to_drop_off[current_index].end(); current_drop_off++) {
                    if (!delivered[current_drop_off->second] && picked[current_drop_off->second]) { //not delivered and already picked up
                        closest_drop_time = current_drop_off -> first;
                        closest_drop_path = drop_off_to_drop_off_path[current_index][current_drop_off->second];
                        closest_drop_index = current_drop_off -> second;
                        break;
                    }
                }

                double closest_pick_time = 999999999999;
                std::vector<StreetSegmentIndex> closest_pick_path;
                int closest_pick_index = -1;

                //the closest available pick_up location
                for (auto current_pick_up = drop_off_to_pick_up[current_index].begin(); current_pick_up != drop_off_to_pick_up[current_index].end(); current_pick_up++) {
                    if (!picked[current_pick_up->second] && (current_weight + deliveries[current_pick_up->second].itemWeight) <= truck_capacity) { //not picked and won't exceed capacity
                        closest_pick_time = current_pick_up -> first;
                        closest_pick_path = drop_off_to_pick_up_path[current_index][current_pick_up->second];
                        closest_pick_index = current_pick_up -> second;
                        break;
                    }
                }

                if (closest_pick_index != -1 && closest_pick_time <= closest_drop_time) { //a pickup is closer
                    this_path.subpath = closest_pick_path;
                    this_path.start_intersection = deliveries[current_index].dropOff;
                    this_path.end_intersection = deliveries[closest_pick_index].pickUp;
                    this_path.pickUp_indices = pick_up_indices;
                    drop_or_pick = false;
                    current_index = closest_pick_index;
                    result_path.push_back(this_path); //record this subpath into result
                } else if (closest_drop_index != -1) { //a dropoff is closer;
                    this_path.subpath = closest_drop_path;
                    this_path.start_intersection = deliveries[current_index].dropOff;
                    this_path.end_intersection = deliveries[closest_drop_index].dropOff;
                    this_path.pickUp_indices = pick_up_indices;
                    drop_or_pick = true;
                    current_index = closest_drop_index;
                    result_path.push_back(this_path); //record this subpath into result
                }
            }
            else {
                std::vector<unsigned int> pick_up_indices;

                for (auto current_item = 0; current_item < deliveries.size(); current_item++) {
                    if (!delivered[current_item] && deliveries[current_item].dropOff == deliveries[current_index].pickUp && picked[current_item]) { //delivered
                        delivered[current_item] = true;
                        current_weight -= deliveries[current_item].itemWeight;
                    }
                    if (!picked[current_item] && (deliveries[current_item].pickUp == deliveries[current_index].pickUp) && (current_weight + deliveries[current_item].itemWeight <= truck_capacity)) { //can pick up
                        picked[current_item] = true;
                        current_weight += deliveries[current_item].itemWeight;
                        pick_up_indices.push_back(current_item);
                    }
                }

                double closest_drop_time = 999999999999;
                std::vector<StreetSegmentIndex> closest_drop_path;
                int closest_drop_index = -1;

                //the closest available drop_off location
                for (auto current_drop_off = pick_up_to_drop_off[current_index].begin(); current_drop_off != pick_up_to_drop_off[current_index].end(); current_drop_off++) {
                    if (!delivered[current_drop_off->second] && picked[current_drop_off->second]) { //not delivered and already picked up
                        closest_drop_time = current_drop_off -> first;
                        closest_drop_path = pick_up_to_drop_off_path[current_index][current_drop_off->second];
                        closest_drop_index = current_drop_off -> second;
                        break;
                    }
                }

                double closest_pick_time = 999999999999;
                std::vector<StreetSegmentIndex> closest_pick_path;
                int closest_pick_index = -1;

                //the closest available pick_up location
                for (auto current_pick_up = pick_up_to_pick_up[current_index].begin(); current_pick_up != pick_up_to_pick_up[current_index].end(); current_pick_up++) {
                    if (!picked[current_pick_up->second] && (current_weight + deliveries[current_pick_up->second].itemWeight) <= truck_capacity) { //not picked and won't exceed capacity
                        closest_pick_time = current_pick_up -> first;
                        closest_pick_path = pick_up_to_pick_up_path[current_index][current_pick_up->second];
                        closest_pick_index = current_pick_up -> second;
                        break;
                    }
                }

                if (closest_pick_index != -1 && closest_pick_time <= closest_drop_time) { //a pickup is closer
                    this_path.subpath = closest_pick_path;
                    this_path.start_intersection = deliveries[current_index].pickUp;
                    this_path.end_intersection = deliveries[closest_pick_index].pickUp;
                    this_path.pickUp_indices = pick_up_indices;
                    drop_or_pick = false;
                    current_index = closest_pick_index;
                    result_path.push_back(this_path); //record this subpath into result
                } else if (closest_drop_index != -1) { //a dropoff is closer;
                    this_path.subpath = closest_drop_path;
                    this_path.start_intersection = deliveries[current_index].pickUp;
                    this_path.end_intersection = deliveries[closest_drop_index].dropOff;
                    this_path.pickUp_indices = pick_up_indices;
                    drop_or_pick = true;
                    current_index = closest_drop_index;
                    result_path.push_back(this_path); //record this subpath into result
                }
            }

            //check if all the delivery is completed
            all_delivered = true;
            for (auto current_item = delivered.begin(); current_item != delivered.end(); current_item++) {
                if (*current_item == false) { //something not delivered yet
                    all_delivered = false;
                }
            }

        }
        //std::cout << "1" << std::endl;
        //record the first pickup into the returned result
       
        final_path.start_intersection = (drop_or_pick) ? deliveries[current_index].dropOff : deliveries[current_index].pickUp;


        std::vector <IntersectionIndex> last_d;
        for (auto current_d = depots.begin(); current_d != depots.end(); current_d++) {
            last_d.push_back(*current_d);
        }

        std::unordered_map<IntersectionIndex, std::vector < StreetSegmentIndex>> last_depot_map = multi_destination_path(
                final_path.start_intersection,
                last_d,
                turn_penalty);

        std::multimap<double, int> final_multimap;

        for (auto i = 0; i < depots.size(); i++) {
            IntersectionIndex haley = depots[i];
            std::vector<StreetSegmentIndex> hehe = last_depot_map.find(haley)->second;
            final_multimap.insert(std::make_pair(compute_path_travel_time(hehe, turn_penalty), i));
        }

        final_path.end_intersection = depots[final_multimap.begin()->second];
        final_path.subpath = last_depot_map.find(final_path.end_intersection)->second;
        std::vector<unsigned> final_indices;
        final_path.pickUp_indices = final_indices; //the first subpath never has any pickup (see m4.h)
        result_path.push_back(final_path);
        
        final_path_thread[k] = final_path;
        result_path_thread[k] = result_path;
        //std::cout << "2" << std::endl;
    }
    
    double result_time = 0;
    double start_time = 0, end_time = 0;
    
    std::vector<CourierSubpath> result_path_1;
    std::vector<CourierSubpath> result_path_2;
    result_path_1 = result_path_thread[0];
    result_path_1.insert(result_path_1.begin(), first_path);
    double result_time_1 = 0;
    double start_time_1 = 0, start_time_2 = 0;
    double end_time_1 = 0, end_time_2 = 0;
    result_path_2 = result_path_thread[1];
    result_path_2.insert(result_path_2.begin(), second_path);
    double result_time_2 = 0;
    CourierSubpath final_path_1 = final_path_thread[0];
    CourierSubpath final_path_2 = final_path_thread[1];
    CourierSubpath final_path;
    for (auto path = 0; path < result_path_1.size(); path++) {
        //path_intersections.push_back(result_path[path].start_intersection);
        if (path == result_path_1.size() - 1) {
            // path_intersections.push_back(result_path[path].end_intersection);
            end_time_1 = compute_path_travel_time(result_path_1[path].subpath, turn_penalty);
            result_time_1 += end_time;
        } else if (path == 0) {
            start_time_1 = compute_path_travel_time(result_path_1[path].subpath, turn_penalty);
            result_time_1 += start_time;
        } else {
            result_time_1 += compute_path_travel_time(result_path_1[path].subpath, turn_penalty);
        }
    }
    for (auto path = 0; path < result_path_2.size(); path++) {
        //path_intersections.push_back(result_path[path].start_intersection);
        if (path == result_path_2.size() - 1) {
            // path_intersections.push_back(result_path[path].end_intersection);
            end_time_2 = compute_path_travel_time(result_path_2[path].subpath, turn_penalty);
            result_time_2 += end_time;
        } else if (path == 0) {
            start_time_2 = compute_path_travel_time(result_path_2[path].subpath, turn_penalty);
            result_time_2 += start_time;
        } else {
            result_time_2 += compute_path_travel_time(result_path_2[path].subpath, turn_penalty);
        }
    }
    std::cout << "result time 1 " << result_time_1 << std::endl;
    std::cout << "result time 2 " << result_time_2 << std::endl;
    double current_fastest_time = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::vector<CourierSubpath> better_path;
    if (result_time_2 < result_time_1){
        better_path.clear();
        current_fastest_time = result_time_2;
        better_path = result_path_2;
        start_time = start_time_2;
        end_time = end_time_2;
        first_path = second_path;
        final_path = final_path_2;
        result_time = result_time_2;
    }else{
        better_path.clear();
        current_fastest_time = result_time_1;
        better_path = result_path_1;
        start_time = start_time_1;
        end_time = end_time_1;
        final_path = final_path_1;
        result_time = result_time_1;
    }
    
    path_intersections.clear();

    for (auto path = 0; path < better_path.size(); path++) {
        path_intersections.push_back(better_path[path].start_intersection);
        //std::cout<<better_path[path].start_intersection<<std::endl;
        if (path == better_path.size() - 1) {
            path_intersections.push_back(better_path[path].end_intersection);
        }
    }
    
    
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //std::cout << "greedy ends" << std::endl;
    for (auto i = 2; i < path_intersections.size() - 1; i ++) {
    //for (auto i = path_intersections.size() - 3; i >= 2; i -- ) {
        for (auto k = i + 1; k < path_intersections.size() - 1; k ++ ) {
        //for (auto k = path_intersections.size() - 2; k >= i+1; k -- ) {

            std::vector <IntersectionIndex> changed_path_intersections = path_intersections;
            bool legal = true;
            //std::cout <<"2opt" << std::endl;
            std::reverse(changed_path_intersections.begin() + i, changed_path_intersections.begin() + k);

            //bool new_all_delivered = false;
            std::vector<bool> new_delivered (deliveries.size(), false);
            std::vector<bool> new_picked (deliveries.size(), false);
            float new_current_weight = 0;
            std::vector<CourierSubpath> new_path;
            float new_time = start_time;
            //qian mian de na duan mei xie
            new_path.push_back(first_path);
            for (auto station = 1; station < changed_path_intersections.size() - 2; station ++) {
                IntersectionIndex current_station = changed_path_intersections[station];
                std::vector<unsigned> pick_up_indices;
               // if () { //is a drop off && 
                for (auto delivery = 0; delivery < deliveries.size(); delivery ++ ) {
                    if (deliveries[delivery].dropOff == current_station && new_picked[delivery] == true && new_delivered[delivery] == false) {
                        new_delivered[delivery] = true;
                        new_current_weight -= deliveries[delivery].itemWeight;
                    }
                }
                for (auto delivery = 0; delivery < deliveries.size(); delivery ++) {
                    if (deliveries[delivery].pickUp == current_station && new_picked[delivery] == false && (new_current_weight + deliveries[delivery].itemWeight) < truck_capacity) {
                        new_picked[delivery] = true;
                        new_current_weight += deliveries[delivery].itemWeight;
                        pick_up_indices.push_back(delivery);
                    }
                }

                CourierSubpath new_subpath;
                if (pick_up_check[current_station].first == true && pick_up_check[changed_path_intersections[station+1]].first == true) {
                    new_subpath.subpath = pick_up_to_pick_up_path[pick_up_check[current_station].second][pick_up_check[changed_path_intersections[station+1]].second];
                } else if (pick_up_check[current_station].first == true && pick_up_check[changed_path_intersections[station+1]].first != true) {
                    new_subpath.subpath = pick_up_to_drop_off_path[pick_up_check[current_station].second][drop_off_check[changed_path_intersections[station+1]].second];
                } else if (pick_up_check[current_station].first != true && pick_up_check[changed_path_intersections[station+1]].first != true) {
                    new_subpath.subpath = drop_off_to_drop_off_path[drop_off_check[current_station].second][drop_off_check[changed_path_intersections[station+1]].second];
                } else {
                    new_subpath.subpath = drop_off_to_pick_up_path[drop_off_check[current_station].second][pick_up_check[changed_path_intersections[station+1]].second];
                }


                new_subpath.start_intersection = changed_path_intersections[station];
                new_subpath.end_intersection = changed_path_intersections[station + 1];
                new_subpath.pickUp_indices = pick_up_indices;
                new_time += compute_path_travel_time(new_subpath.subpath, turn_penalty);
                new_path.push_back(new_subpath);


                if (station == changed_path_intersections.size() - 3) {
                    IntersectionIndex current_station1 = changed_path_intersections[station + 1];
                    for (auto delivery = 0; delivery < deliveries.size(); delivery ++ ) {
                        if (deliveries[delivery].dropOff == current_station1 && new_picked[delivery] == true && new_delivered[delivery] == false) {
                            new_delivered[delivery] = true;
                            new_current_weight -= deliveries[delivery].itemWeight;
                        }
                    }
                }
            }

            new_time += end_time;
            new_path.push_back(final_path);
            //changed_path_pool.push_back(changed_path_intersections);
            for (auto item = 0; item < new_delivered.size(); item ++) {
                if (new_delivered[item] == false) {
                    legal = false;
                }
            }
            if (new_time < current_fastest_time && legal) {
                current_fastest_time = new_time;
                better_path = new_path;
                //std::cout << "2 opt: newpath's current fastest time: " << current_fastest_time << std::endl;
                //std::cout << "newpath////////////////////" << std::endl;
            }
            auto now_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> ms = now_time - clock_start_time;
            if (ms.count() >= 49700) { 
              //  std::cout<<"time is going to end" << std::endl;//about to exceed time, return whatever we have
                return better_path;
            }
        }
        //std::vector<CourierSubpath> best_path = result_path;
    }
    if (path_intersections.size() < 7) return better_path;
  
    for (auto i = 2; i < path_intersections.size() - 5; i ++) {
        for (auto j = i + 2; j < path_intersections.size() - 3; j ++) {
            for (auto k = j + 2; k < path_intersections.size() - 1; k ++ ) {
                bool legal = true;
                std::vector<IntersectionIndex> copy_path_intersections = path_intersections;
                std::vector<IntersectionIndex> i_to_j_minus_one;
                std::vector<IntersectionIndex> j_to_k_minus_one;
                for (auto index = i; index < j; index ++) {
                    i_to_j_minus_one.push_back(copy_path_intersections[index]);
                }
                for (auto index = j; index < k; index ++) {
                    j_to_k_minus_one.push_back(copy_path_intersections[index]);
                }
                
                std::vector<IntersectionIndex> reverse_i_to_j_minus_one;
                std::vector<IntersectionIndex> reverse_j_to_k_minus_one;
                std::reverse(copy_path_intersections.begin() + i, copy_path_intersections.begin() + j - 1);
                std::reverse(copy_path_intersections.begin() + j, copy_path_intersections.begin() + k - 1);
                for (auto index = i; index < j; index ++) {
                    reverse_i_to_j_minus_one.push_back(copy_path_intersections[index]);
                }
                for (auto index = j; index < k; index ++) {
                    reverse_j_to_k_minus_one.push_back(copy_path_intersections[index]);
                }
                
                
                for (auto index = 2; index < 6; index ++) {
                    std::vector<IntersectionIndex> changed_path_intersections;
                    for (auto m = 0; m < i; m++) {
                        changed_path_intersections.push_back(copy_path_intersections[m]);
                    }
                    
                    if (index == 2) {
                        for (auto m = i; m < j; m++) {
                            changed_path_intersections.push_back(reverse_i_to_j_minus_one[m-i]);
                        }
                        for (auto m = j; m < k; m++) {
                            changed_path_intersections.push_back(reverse_j_to_k_minus_one[m-j]);
                        }
                    } 
                    else if (index == 3) {
                        for (auto m = j; m < k; m++) {
                            changed_path_intersections.push_back(j_to_k_minus_one[m-j]);
                        }
                        for (auto m = i; m < j; m++) {
                            changed_path_intersections.push_back(i_to_j_minus_one[m-i]);
                        }
                    } 
                    else if (index == 4) {
                        for (auto m = j; m < k; m++) {
                            changed_path_intersections.push_back(j_to_k_minus_one[m-j]);
                        }
                        for (auto m = i; m < j; m++) {
                            changed_path_intersections.push_back(reverse_i_to_j_minus_one[m-i]);
                        }
                    } 
                    else if (index == 5) {
                        for (auto m = j; m < k; m++) {
                            changed_path_intersections.push_back(reverse_j_to_k_minus_one[m-j]);
                        }
                        for (auto m = i; m < j; m++) {
                            changed_path_intersections.push_back(i_to_j_minus_one[m-i]);
                        }
                    } 
                   
                    for (auto m = k; m < path_intersections.size(); m++) {
                        changed_path_intersections.push_back(copy_path_intersections[m]);
                    }
                    //now the change_path_intersections is the path with 3opt but without checking legality
                    std::vector<bool> new_delivered (deliveries.size(), false);
                    std::vector<bool> new_picked (deliveries.size(), false);
                    float new_current_weight = 0;
                    std::vector<CourierSubpath> new_path;
                    float new_time = start_time;
                    //qian mian de na duan mei xie
                    new_path.push_back(first_path);
                    for (auto station = 1; station < changed_path_intersections.size() - 2; station ++) {
                        IntersectionIndex current_station = changed_path_intersections[station];
                        std::vector<unsigned> pick_up_indices;
                       // if () { //is a drop off && 
                        for (auto delivery = 0; delivery < deliveries.size(); delivery ++ ) {
                            if (deliveries[delivery].dropOff == current_station && new_picked[delivery] == true && new_delivered[delivery] == false) {
                                new_delivered[delivery] = true;
                                new_current_weight -= deliveries[delivery].itemWeight;
                            }
                        }
                        for (auto delivery = 0; delivery < deliveries.size(); delivery ++) {
                            if (deliveries[delivery].pickUp == current_station && new_picked[delivery] == false && (new_current_weight + deliveries[delivery].itemWeight) < truck_capacity) {
                                new_picked[delivery] = true;
                                new_current_weight += deliveries[delivery].itemWeight;
                                pick_up_indices.push_back(delivery);
                            }
                        }

                        CourierSubpath new_subpath;
                        if (pick_up_check[current_station].first == true && pick_up_check[changed_path_intersections[station+1]].first == true) {
                            new_subpath.subpath = pick_up_to_pick_up_path[pick_up_check[current_station].second][pick_up_check[changed_path_intersections[station+1]].second];
                        } else if (pick_up_check[current_station].first == true && pick_up_check[changed_path_intersections[station+1]].first != true) {
                            new_subpath.subpath = pick_up_to_drop_off_path[pick_up_check[current_station].second][drop_off_check[changed_path_intersections[station+1]].second];
                        } else if (pick_up_check[current_station].first != true && pick_up_check[changed_path_intersections[station+1]].first != true) {
                            new_subpath.subpath = drop_off_to_drop_off_path[drop_off_check[current_station].second][drop_off_check[changed_path_intersections[station+1]].second];
                        } else {
                            new_subpath.subpath = drop_off_to_pick_up_path[drop_off_check[current_station].second][pick_up_check[changed_path_intersections[station+1]].second];
                        }


                        new_subpath.start_intersection = changed_path_intersections[station];
                        new_subpath.end_intersection = changed_path_intersections[station + 1];
                        new_subpath.pickUp_indices = pick_up_indices;
                        //new_subpath.subpath = /////////?;
                        new_time += compute_path_travel_time(new_subpath.subpath, turn_penalty);
                        new_path.push_back(new_subpath);


                        if (station == changed_path_intersections.size() - 3) {
                            IntersectionIndex current_station1 = changed_path_intersections[station + 1];
                            for (auto delivery = 0; delivery < deliveries.size(); delivery ++ ) {
                                if (deliveries[delivery].dropOff == current_station1 && new_picked[delivery] == true && new_delivered[delivery] == false) {
                                    new_delivered[delivery] = true;
                                    new_current_weight -= deliveries[delivery].itemWeight;
                                }
                            }
                        }
                    }

                    new_time += end_time;
                    new_path.push_back(final_path);
                    //changed_path_pool.push_back(changed_path_intersections);
                    for (auto item = 0; item < new_delivered.size(); item ++) {
                        if (new_delivered[item] == false) {
                            legal = false;
                        }
                    }
                    if (new_time < current_fastest_time && legal) {
                        current_fastest_time = new_time;
                        better_path = new_path;
                        //std::cout << "3 opt: newpath's current fastest time: " << current_fastest_time << std::endl;
                    }
                    auto now_time = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> ms = now_time - clock_start_time;
                    if (ms.count() >= 49700) { 
                      //  std::cout<<"time is going to end" << std::endl;//about to exceed time, return whatever we have
                        return better_path;
                    }
                }
                
            }
        }
    }
    /*for (auto current_subpath = better_path.begin(); current_subpath!= better_path.end(); current_subpath++){
        std::cout<<current_subpath->start_intersection<< "   " << current_subpath->end_intersection<<std::endl;
    }*/
    
    return better_path;
}

std::unordered_map<IntersectionIndex, std::vector<StreetSegmentIndex>> multi_destination_path(
		          const IntersectionIndex intersect_id_start, 
                          const std::vector<IntersectionIndex> intersection_list,
                          const double turn_penalty){
    
    std::unordered_map<IntersectionIndex, std::vector<StreetSegmentIndex>> sum_of_path;
    std::vector<StreetSegmentIndex> path;
       
    int total_intersection = getNumIntersections();
    
    //clean the node list and resize it to make sure everything is ready
    node_List.clear();
    node_List.resize(total_intersection);
    
    //initialize the start intersection
    node_List[intersect_id_start].segment_ID = -1;
    node_List[intersect_id_start].parent_ID = -1;
    node_List[intersect_id_start].travel_time = 0.0;
    node_List[intersect_id_start].rest_time = 0.0;
        
    //bit map for marking positions of nodes
    std::vector<bool> frontier_check_m4;
    frontier_check_m4.clear();
    frontier_check_m4.resize(total_intersection);
    std::vector<bool> visited_check_m4;
    visited_check_m4.clear();
    visited_check_m4.resize(total_intersection);
     
    std::priority_queue<frontier, std::vector<frontier>, m4_comparator> priority;
    
    //put start into priority queue
    frontier frontier_start(intersect_id_start);
    priority.push(frontier_start);
    frontier_check_m4[intersect_id_start] = true;
    
    std::vector<bool>intersection_check(total_intersection);
    for (int current_item = 0; current_item < intersection_list.size(); current_item++){
        intersection_check[intersection_list[current_item]] = true;
    }
    
    while(!priority.empty()){
        
        //dequeue
        frontier top_frontier = priority.top();
        IntersectionIndex current_node = top_frontier.nodeID;
        
        priority.pop();
        if (visited_check_m4[current_node])
            continue; //skip the rest since already checked current node
                
        frontier_check_m4[current_node] = false;
        visited_check_m4[current_node] = true;
        
        //reached destination
        if (intersection_check[current_node]){
            intersection_check[current_node] = false;
            path.clear();
            IntersectionIndex end_intersection = current_node;
            //traverse backwards by insert at front
            while (end_intersection != intersect_id_start){
                path.insert(path.begin(), node_List[end_intersection].segment_ID);
                end_intersection = node_List[end_intersection].parent_ID;
            }    
            sum_of_path[current_node] = path;
        }
                
        int number_of_adjacents = G_DATABASE.intersectionDatabase[current_node].connected_adjacentID.size();
        
        for (IntersectionIndex connected = 0; connected < number_of_adjacents; connected++){
            IntersectionIndex adjacent_ID = G_DATABASE.intersectionDatabase[current_node].connected_adjacentID[connected];
            if (current_node == adjacent_ID) continue;
            if (visited_check_m4[adjacent_ID]) { 
                continue; //skip the rest
            }
            if(frontier_check_m4[adjacent_ID]) { //already calculated node
                //calculate new travel time without turn penalty
                double adjacent_travel_time = find_street_segment_travel_time(G_DATABASE.intersectionDatabase[current_node].connected_segmentID[connected]);
                //add turn penalty if street ID changes
                if (node_List[current_node].parent_ID != -1){
                    if (getInfoStreetSegment(node_List[current_node].segment_ID).streetID != getInfoStreetSegment(G_DATABASE.intersectionDatabase[current_node].connected_segmentID[connected]).streetID){
                        adjacent_travel_time += turn_penalty;
                    }
                    //update travel time
                    adjacent_travel_time += node_List[current_node].travel_time;
                }
                //update node if new travel time is shorter
                if (adjacent_travel_time  < node_List[adjacent_ID].travel_time){
                    node_List[adjacent_ID].segment_ID = G_DATABASE.intersectionDatabase[current_node].connected_segmentID[connected];
                    node_List[adjacent_ID].parent_ID = current_node;
                    node_List[adjacent_ID].travel_time = adjacent_travel_time;
                    node_List[adjacent_ID].rest_time = 0.0;
                    frontier updated_node(adjacent_ID);
                    priority.push(updated_node);
                }                
            }
            else {
                //calculate travel time
                double adjacent_travel_time = find_street_segment_travel_time(G_DATABASE.intersectionDatabase[current_node].connected_segmentID[connected]);
                node_List[adjacent_ID].segment_ID =G_DATABASE.intersectionDatabase[current_node].connected_segmentID[connected];
                node_List[adjacent_ID].parent_ID = current_node;
                //add turn penalty if street ID changes
                if (node_List[current_node].parent_ID != -1){
                    if (getInfoStreetSegment(node_List[current_node].segment_ID).streetID != getInfoStreetSegment(node_List[adjacent_ID].segment_ID).streetID){
                        adjacent_travel_time += turn_penalty;
                    }
                    //update travel time
                    adjacent_travel_time += node_List[current_node].travel_time;
                }                
                node_List[adjacent_ID].travel_time = adjacent_travel_time;
                node_List[adjacent_ID].rest_time = 0.0;
                
                frontier new_node(adjacent_ID);
                priority.push(new_node);
                frontier_check_m4[adjacent_ID] = true;
            }
        }
    
    }
    return sum_of_path;
}
