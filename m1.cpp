/* 
 * Copyright 2020 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "m1.h"
#include "m1helper.h"
#include "m2.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "globaldatabase.h"
#include <LatLon.h>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <unordered_map>
#include <iostream>
#include <cctype>


//global variable from "globaldatabase.h"
Global G_DATABASE;

bool load_map(std::string map_streets_database_filename) {
    
    bool load_successful = false; //Indicates whether the map has loaded 
                                  //successfully
    
    //convert pass-in string to fit OSM required format
    size_t suffix = map_streets_database_filename.find(".streets.bin");
    std::string map_path = map_streets_database_filename.substr(0, suffix);
    std::string mapPathOSM = map_path + ".osm.bin";
       
    std::cout << "load map: " << map_streets_database_filename << std::endl;
    load_successful = loadStreetsDatabaseBIN(map_streets_database_filename) && loadOSMDatabaseBIN(mapPathOSM);
    if (!load_successful) { 
        std::cout << "fail to load map" << std::endl;
        return false;
    }
    std::cout << "loading street" << std::endl;
    load_street();
    std::cout << "loading street segments" << std::endl;
    load_street_segment();
    std::cout << "loading database" << std::endl;
    load_database();
    std::cout << "load successful" << std::endl;
    loadSubwaysBus();
//    find_path_between_intersections(
//		          13, 51601, 0
//               );
//    CHECK(path_is_legal(28583, 106836, path));
//    find_path_between_intersections(
//		          3890,112447, 15
//               );
   return load_successful; 
    
}

void load_street(){
    for (int st = 0; st < getNumStreets(); st ++) {
        std::string street_name = getStreetName(st);
        //erase all the space in street names
        street_name.erase(std::remove_if(street_name.begin(), street_name.end(), ::isspace), street_name.end());
        //convert to lowercase
        //std::transform(street_name.begin(), street_name.end(), street_name.begin(), ::tolower);
        boost::algorithm::to_lower(street_name);
        //insert streets with names
        G_DATABASE.street_names.insert(std::make_pair(street_name,st));
    }
}

void load_street_segment(){
    for (int street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++){
        //get information of the street segment
        InfoStreetSegment this_segment = getInfoStreetSegment(street_segment_id);
        
        LatLon from_position = getIntersectionPosition(this_segment.from);
        LatLon to_position = getIntersectionPosition(this_segment.to);
        
        //if the street segment is straight
        if (this_segment.curvePointCount == 0){
            
            //first calculate the length 
            std::pair<LatLon, LatLon> segment_points (from_position, to_position);
            double length = find_distance_between_two_points(segment_points);
            G_DATABASE.street_segment_length.push_back(length);
            
            //get the travel time
            double speedLimit_mpers = this_segment.speedLimit * KILOMETER_TO_METER / HOUR_TO_SECOND;
            G_DATABASE.street_segment_travel_time.push_back(length / speedLimit_mpers);
        }    
        //if the street segment has curve points
        else{
            //distance between curve points
            double partial_length = 0;
            for (int i = 0; i < this_segment.curvePointCount - 1; i++){
                std::pair<LatLon, LatLon> curve_points (getStreetSegmentCurvePoint(i, street_segment_id), 
                        getStreetSegmentCurvePoint(i+1, street_segment_id));
                partial_length += find_distance_between_two_points(curve_points);
            }
            //from "from" to the first curve point
            LatLon first_cPoint = getStreetSegmentCurvePoint(0, street_segment_id);
            std::pair<LatLon, LatLon> from_points (from_position, first_cPoint);
            
            double from_length = find_distance_between_two_points(from_points);
            
           //last curve point to "to"
            LatLon last_cPoint = getStreetSegmentCurvePoint(this_segment.curvePointCount -1, street_segment_id);
            std::pair<LatLon, LatLon> to_points (last_cPoint, to_position);
            
            double to_length = find_distance_between_two_points(to_points);
            
            //total street segment length
            double length = from_length + partial_length + to_length;
            
            //store the length
            G_DATABASE.street_segment_length.push_back(length);
            
            //calculate the travel time
            double speedLimit_mpers = this_segment.speedLimit * KILOMETER_TO_METER / HOUR_TO_SECOND;
            G_DATABASE.street_segment_travel_time.push_back(length / speedLimit_mpers);
            G_DATABASE.speed_limit.push_back(speedLimit_mpers);
        }
    }
    double total_speed_limit = 0;
    for (auto current_segment = 0; current_segment != G_DATABASE.speed_limit.size(); current_segment++){
        total_speed_limit += G_DATABASE.speed_limit[current_segment];
    }

    G_DATABASE.average_speed_limit = total_speed_limit / G_DATABASE.speed_limit.size();
}

void load_database(){
    G_DATABASE.intersectionDatabase.resize(getNumIntersections());
    G_DATABASE.streetDatabase.resize(getNumStreets());
    G_DATABASE.connected_segment.resize(getNumIntersections());
    
    //Database initialization
    for (int current_intersection = 0; 
            current_intersection < getNumIntersections(); current_intersection++) {
        int count = getIntersectionStreetSegmentCount(current_intersection);
        
        for (int current_segment_of_intersection = 0;
                current_segment_of_intersection < count; current_segment_of_intersection++) {
            //for the "i"th intersection:
            //load the segments and the streetID of the segments
            StreetSegmentIndex segID = getIntersectionStreetSegment(current_intersection, current_segment_of_intersection);
            G_DATABASE.intersectionDatabase[current_intersection].segmentID.push_back(segID);
            InfoStreetSegment seg = getInfoStreetSegment(segID);
            G_DATABASE.intersectionDatabase[current_intersection].streetID.push_back(seg.streetID); 
            
            //std::vector<std::vector<std::pair<IntersectionIndex, StreetSegmentIndex>>> connected_segment;
            //find adjacents, oneWay condition has been considered
            if (seg.from == current_intersection) {
                auto iterator = G_DATABASE.connected_segment[current_intersection].find(seg.to);
                if (iterator == G_DATABASE.connected_segment[current_intersection].end()){
                    G_DATABASE.connected_segment[current_intersection].insert(std::make_pair(seg.to, segID));
                }else{
                    if (find_street_segment_travel_time(segID) < find_street_segment_travel_time(G_DATABASE.connected_segment[current_intersection].find(seg.to)->second)){
                        G_DATABASE.connected_segment[current_intersection].find(seg.to)->second = segID;
                    }
                }                
                G_DATABASE.intersectionDatabase[current_intersection].adjacentID.push_back(seg.to);
            } else if (seg.to == current_intersection && !seg.oneWay) {
                auto iterator = G_DATABASE.connected_segment[current_intersection].find(seg.from);
                if (iterator == G_DATABASE.connected_segment[current_intersection].end()){
                    G_DATABASE.connected_segment[current_intersection].insert(std::make_pair(seg.from, segID));
                }else{
                    if (find_street_segment_travel_time(segID) < find_street_segment_travel_time(G_DATABASE.connected_segment[current_intersection].find(seg.from)->second)){
                        G_DATABASE.connected_segment[current_intersection].find(seg.from)->second = segID;
                    }
                }    
                G_DATABASE.intersectionDatabase[current_intersection].adjacentID.push_back(seg.from);
            }
            
            //insert the segmentid into streetDatabase
            G_DATABASE.streetDatabase[seg.streetID].segmentID.push_back(segID);
            //insert the intersectionID into streetDatabase
            G_DATABASE.streetDatabase[seg.streetID].intersectionID.push_back(current_intersection);
            
        }
        
    }
    
    //remove duplicates of the street associated with intersections
    //of the intersectionDatabase 
    for (int current_intersection = 0; current_intersection < getNumIntersections(); current_intersection++) {
        removeDuplicates(G_DATABASE.intersectionDatabase[current_intersection].streetID);
        removeDuplicates(G_DATABASE.intersectionDatabase[current_intersection].adjacentID);
    }
    determine_bounds();
    
    for (int current_street = 0; current_street < getNumStreets(); current_street++) {
        removeDuplicates(G_DATABASE.streetDatabase[current_street].segmentID);
        removeDuplicates(G_DATABASE.streetDatabase[current_street].intersectionID);
        
        //define critical streets as major streets and highways
        bool largeIntersections = (G_DATABASE.streetDatabase[current_street].intersectionID.size() > 200 
                && getStreetName(current_street) != "<unknown>");
        bool highway = false;
        if (getInfoStreetSegment(G_DATABASE.streetDatabase[current_street].segmentID[0]).speedLimit > 80) {
            highway = true;
        }
        //find major streets to display street names (which have large number of intersections)
        if (highway || largeIntersections) {
            //it's a major street
            G_DATABASE.major_streets.push_back(current_street);
            
            //loop to find the average angle of a street segment and the r square of street angles
            //to determine if the segment is not "curved" graphically
            for (auto segment = 0; segment < G_DATABASE.streetDatabase[current_street].segmentID.size(); segment ++) {
                int segment_id = G_DATABASE.streetDatabase[current_street].segmentID[segment];
                InfoStreetSegment target = getInfoStreetSegment(segment_id);
                double angle1 = 0; 
                double angle2 = 0;
                int pointcount = target.curvePointCount;

                //get segment endpoints positions
                LatLon from = getIntersectionPosition(target.from);
                LatLon to = getIntersectionPosition(target.to);
                double xfrom = x_from_lon(from.lon() );
                double yfrom = y_from_lat(from.lat());
                double xto = x_from_lon(to.lon());
                double yto = y_from_lat(to.lat());
                double anglesum = 0;
                double squares = 0;
                //catagorize by curvepoint count
                if (pointcount == 0) {
                    segmentcurvature info;
                    info.average_angle = ( atan((yfrom - yto)/ (xfrom - xto) ) * RADIAN_TO_DEGREE);
                    info.r_squared = 0;
                    info.x_start = (xto + xfrom) / 2.0;
                    info.y_start = (yto + yfrom) / 2.0;
                    //insert to the G_DATABASE
                    G_DATABASE.major_streets_curvature.insert(std::make_pair(segment_id, info));
                } else if (pointcount == 1) {
                    
                    double xmid = x_from_lon(getStreetSegmentCurvePoint(0, segment_id).lon());
                    double ymid = y_from_lat(getStreetSegmentCurvePoint(0, segment_id).lat());
                    angle1 =  atan((ymid - yfrom)/(xmid - xfrom)) * RADIAN_TO_DEGREE;
                    angle2 =  atan((ymid - yto)/(xmid - xto)) * RADIAN_TO_DEGREE;
                    anglesum = angle1 + angle2;
                    squares += (angle2 - angle1) * (angle2 - angle1);
                    
                    segmentcurvature info;
                    info.average_angle = anglesum / 2;
                    info.r_squared = squares;
                    info.x_start = (xto + xfrom) / 2.0;
                    info.y_start = (yto + yfrom) / 2.0;
                    //insert to the G_DATABASE
                    G_DATABASE.major_streets_curvature.insert(std::make_pair(segment_id, info));
                } else { //multiple curvepoints
                    for (auto point = 0; point < pointcount - 1; point ++) {
                        LatLon firstlatlon = getStreetSegmentCurvePoint(point, segment_id);
                        LatLon secondlatlon = getStreetSegmentCurvePoint(point + 1, segment_id);
                        double x1 = x_from_lon(firstlatlon.lon() );
                        double y1 = y_from_lat(firstlatlon.lat() );
                        double x2 = x_from_lon(secondlatlon.lon() );
                        double y2 = y_from_lat(secondlatlon.lat() );
                        
                        //end points cases
                        if (point == 0) {
                            angle1 = atan( (y1 - yfrom)/(x1 - xfrom)) * RADIAN_TO_DEGREE;
                            angle2 = atan( (y2 - y1) / (x2 - x1) ) * RADIAN_TO_DEGREE;
                            squares += (angle2 - angle1) * (angle2 - angle1);
                            anglesum += angle1;
                            angle1 = angle2;
                        }
                        if (point == pointcount - 2) {
                            angle2 = atan( (yto - y2) / (xto - x2)) * RADIAN_TO_DEGREE;
                            squares += (angle2 - angle1) * (angle2 - angle1);
                            anglesum += angle1;
                            anglesum += angle2;
                            break;
                        } else {
                            angle2 = atan( (y2 - y1) / (x2 - x1) ) * RADIAN_TO_DEGREE;
                            squares += (angle2 - angle1) * (angle2 - angle1);
                            anglesum += angle1;
                            angle1 = angle2;
                        }
                    }
                    
                    segmentcurvature info;
                    info.average_angle = anglesum / (pointcount + 1);
                    info.r_squared = squares;
                    info.x_start = (xto + xfrom) / 2.0;
                    info.y_start = (yto + yfrom) / 2.0;
                    //insert to the G_DATABASE
                    G_DATABASE.major_streets_curvature.insert(std::make_pair(segment_id, info));
                }
            }
        }
    }
    
    
    //The below two data structures are for find_way_length 
    //use unordered map to improve performance
    //store OSMID, OSMNode*, and OSMWay* in the map
    for (int current_way = 0; current_way < getNumberOfWays(); current_way++ ){
        const OSMWay* wayid = getWayByIndex(current_way);
        OSMID id = wayid->id();
        G_DATABASE.wayDatabase.insert(std::make_pair(id, wayid));
    }
    
    for (int current_node = 0; current_node < getNumberOfNodes(); current_node++ ){
        const OSMNode* nodeid = getNodeByIndex(current_node);
        OSMID id = nodeid->id();
        G_DATABASE.nodeDatabase.insert(std::make_pair(id, nodeid));
    }
    
    for (int current_feature = 0; current_feature < getNumFeatures(); current_feature ++){
        G_DATABASE.featureDatabase.insert(std::make_pair(find_feature_area(current_feature), current_feature));
    }
    
    int total_POI = getNumPointsOfInterest();
    for (int POI_Index = 0; POI_Index < total_POI; POI_Index++){
        POI POI_info;
        POI_info.type = getPointOfInterestType(POI_Index);
        POI_info.name = getPointOfInterestName(POI_Index);
        POI_info.position = getPointOfInterestPosition(POI_Index);
        POI_info.OSMNodeID = getPointOfInterestOSMNodeID(POI_Index);
        G_DATABASE.POI_Database.push_back(POI_info);
    }
    
    /*for (int i; i < getNumIntersections(); i++){
        std::vector<int> adjacent = find_adjacent_intersections(i);
        for (int j; j < getIntersectionStreetSegmentCount(i); j++){
            for (int k; k < getIntersectionStreetSegmentCount())
        }
    }
    */
}

void close_map() {
    //Clean-up your map related data structures here
    G_DATABASE.street_names.clear();
    G_DATABASE.street_segment_length.clear();
    G_DATABASE.street_segment_travel_time.clear();

    G_DATABASE.intersectionDatabase.clear();
    G_DATABASE.streetDatabase.clear();
    G_DATABASE.major_streets.clear();
    G_DATABASE.major_streets_curvature.clear();
    G_DATABASE.featureDatabase.clear();

    G_DATABASE.wayDatabase.clear();
    G_DATABASE.nodeDatabase.clear();
    
    G_DATABASE.starDatabase.clear();
       
    G_DATABASE.POI_Database.clear();
    
    G_DATABASE.subwayinfo.clear();
    G_DATABASE.subwaylineinfo.clear();
    G_DATABASE.subwayStationPrintedDatabase.clear();
    
    G_DATABASE.osm_bus_lines.clear();
    G_DATABASE.businfo.clear();
    G_DATABASE.buslineinfo.clear();
    G_DATABASE.connected_segment.clear();
    
    closeStreetDatabase();
    closeOSMDatabase();
}


//Returns the street names at the given intersection (includes duplicate street 
//names in returned vector)
std::vector<std::string> find_street_names_of_intersection(int intersection_id) {
    
    std::vector<std::string> names;
    
    if (!inRange(intersection_id, 0, getNumIntersections() - 1)) {
        std::cout << "Invalid Intersection ID" << std::endl;
        return names;
    }
    //the number of segments of the passed intersection
    int size = G_DATABASE.intersectionDatabase[intersection_id].segmentID.size();
    for (int current_segment = 0; current_segment < size; current_segment ++) {
        int segid = G_DATABASE.intersectionDatabase[intersection_id].segmentID[current_segment];
        InfoStreetSegment seg = getInfoStreetSegment(segid);
        names.push_back(getStreetName(seg.streetID));
    }
    
    return names;
    
}

//Returns true if you can get from intersection_ids.first to intersection_ids.second using a single 
//street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself
bool are_directly_connected(std::pair<int, int> intersection_ids) {
    
    int a = intersection_ids.first;
    int b = intersection_ids.second;
    
    //check if the passed intersection ids are invalid
    if (!inRange(a, 0, getNumIntersections() - 1) && !inRange(b, 0, getNumIntersections() - 1)) {
        std::cout << "Invalid Intersection IDs" << std::endl;
        return false;
    } else if (!inRange(a, 0, getNumIntersections() - 1)) {
        std::cout << "Invalid first Intersection ID" << std::endl;
        return false;
    } else if (!inRange(b, 0, getNumIntersections() - 1)) {
        std::cout << "Invalid second Intersection ID" << std::endl;
        return false;
    }
    
    //same intersection
    if (a == b) return true;
    //find adjacent intersections of the first
    std::vector<IntersectionIndex> adj = G_DATABASE.intersectionDatabase[a].adjacentID;
    for (int current_adjacent_intersection = 0; 
            current_adjacent_intersection < adj.size(); 
            current_adjacent_intersection ++) {
        //the second intersection is adjacent, thus connected
        if (adj[current_adjacent_intersection] == b) return true;
    }
    
    return false;
    
}

//Returns all intersections reachable by traveling down one street segment 
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections
std::vector<int> find_adjacent_intersections(int intersection_id) {
    
    //avoid invalid inputs
    if (!inRange(intersection_id, 0, getNumIntersections() - 1)) {
        std::cout << "Invalid Intersection ID" << std::endl;
        std::vector<int> empty;
        return empty;
    }
    //data structure defined in load_map
    return G_DATABASE.intersectionDatabase[intersection_id].adjacentID; 
    
}

//Returns all street segments for the given street
std::vector<int> find_street_segments_of_street(int street_id) {
    
    //avoid invalid inputs
    if (!inRange(street_id, 0, getNumStreets() - 1)) {
        std::cout << "Invalid Street ID" << std::endl;
        std::vector<int> empty;
        return empty;
    }
    
    //data structure defined in load_map
    return G_DATABASE.streetDatabase[street_id].segmentID; 
    
}

//Returns all intersections along the a given street
std::vector<int> find_intersections_of_street(int street_id) {
    //check if valid
    if (!inRange(street_id, 0, getNumStreets()-1)){
        std::vector<int> empty;
        std::cout<<"Street does not exist"<<std::endl;
        return empty;
    }
    //data structure defined in load_map
    return G_DATABASE.streetDatabase[street_id].intersectionID; 
    
}

//Return all intersection ids for two intersecting streets
//This function will typically return one intersection id.
std::vector<int> find_intersections_of_two_streets(std::pair<int, int> street_ids){
    //check if valid
    if (!inRange(street_ids.first, 0, getNumStreets()-1) || !inRange(street_ids.second, 0, getNumStreets()-1)){
        std::vector<int> empty;
        std::cout<<"Street does not exist"<<std::endl;
        return empty;
    }
    
    if (street_ids.first == street_ids.second) {
        return G_DATABASE.streetDatabase[street_ids.first].intersectionID;
    }
    
    std::vector<int> intersections_common;
    //all intersections in the first street
    std::vector<int> thefirst = G_DATABASE.streetDatabase[street_ids.first].intersectionID;
    
    //determine the common intersections by comparing second street to the first
    for (int i = 0; i < thefirst.size(); i++) {
        for (int j = 0; j < G_DATABASE.intersectionDatabase[thefirst[i]].streetID.size(); j++) {
            if (street_ids.second == G_DATABASE.intersectionDatabase[thefirst[i]].streetID[j]) {
                intersections_common.push_back(thefirst[i]);
            }
        }
    }
    
    return intersections_common; 
    
}

//Returns all street ids corresponding to street names that start with the given prefix
//The function should be case-insensitive to the street prefix. You should ignore spaces.
//For example, both "bloor " and "BloOrst" are prefixes to "Bloor Street East".
//If no street names match the given prefix, this routine returns an empty (length 0) 
//vector.
//You can choose what to return if the street prefix passed in is an empty (length 0) 
//string, but your program must not crash if street_prefix is a length 0 string.
std::vector<int> find_street_ids_from_partial_street_name(std::string street_prefix){
    
    std::vector<int> street_ids_from_partial_street_name;
    
    //empty string passed in
    if (street_prefix.empty()){ 
        return street_ids_from_partial_street_name;
    }
    
    //non-empty string
    
    //erase all the spaces
    street_prefix.erase(std::remove_if(street_prefix.begin(), street_prefix.end(), 
            isspace), street_prefix.end());
    
    //convert to lower case
    std::transform(street_prefix.begin(), street_prefix.end(), street_prefix.begin(), ::tolower);
    
    //use two iterators to determine the traverse range of global variable
    std::multimap<std::string, int>::iterator current_street = G_DATABASE.street_names.lower_bound(street_prefix);
    std::multimap<std::string, int>::iterator up = G_DATABASE.street_names.upper_bound(street_prefix);
    if (current_street != G_DATABASE.street_names.end()){
        do{
            std::string key = current_street->first;
            //compare partial name to first few characters of street name
            if (key.compare(0, street_prefix.size(), street_prefix) == 0){
                street_ids_from_partial_street_name.push_back(current_street->second);
            }
            else{
                break;
            }
            current_street++;
        }
        while (current_street != G_DATABASE.street_names.end() && current_street != up); //if out of range
    }   
    
    return street_ids_from_partial_street_name;
    
}

//Returns the area of the given closed feature in square meters
//Assume a non self-intersecting polygon (i.e. no holes)
//Return 0 if this feature is not a closed polygon.
double find_feature_area(int feature_id){
    
    //check if valid
    if (!inRange(feature_id, 0, getNumFeatures()-1)){
        std::cout<<"Feature does not exist"<<std::endl;
        return 0.0;
    }
    
    double area = 0.0;
    int count = getFeaturePointCount(feature_id);
    double lat_total = 0.0;
    double lat_avg;
    double lat_first = getFeaturePoint(0,feature_id).lat();
    double lat_last = getFeaturePoint(getFeaturePointCount(feature_id)-1, feature_id).lat();
    double lon_first = getFeaturePoint(0,feature_id).lon();
    double lon_last = getFeaturePoint(getFeaturePointCount(feature_id)-1,feature_id).lon();
    
    //not a closed polygon
    if (lat_first != lat_last || lon_first != lon_last){
            return 0.0;
    }
    
    //a closed polygon
    //calculate average lat of all the points of this feature
    for (int current_point =0; current_point < count; current_point++){
        lat_total += getFeaturePoint(current_point,feature_id).lat();
    }
    lat_avg = lat_total / count;
    
    //compute area using shoelace formula
    for (int current_point =0; current_point< count; current_point++){
        int next_point = (current_point+1) % (count); //make sure j is less than total count
        //convert latlon to xy coordinates
        double x_current = DEGREE_TO_RADIAN * getFeaturePoint(current_point,feature_id).lon() * cos(DEGREE_TO_RADIAN * lat_avg);
        double y_current = DEGREE_TO_RADIAN * getFeaturePoint(current_point,feature_id).lat();
        double x_next = DEGREE_TO_RADIAN * getFeaturePoint(next_point,feature_id).lon() * cos(DEGREE_TO_RADIAN * lat_avg);
        double y_next = DEGREE_TO_RADIAN * getFeaturePoint(next_point,feature_id).lat();
        area += x_current * y_next;
        area -= y_current * x_next;
    }
    
    //multiply by R^2 to get area in terms of Earth
    area = area * EARTH_RADIUS_METERS * EARTH_RADIUS_METERS;
    
    //make sure result greater than zero
    return (area < 0? -area / 2.0 : area / 2.0); 
    
}

//Returns the distance between two coordinates in meters

double find_distance_between_two_points(std::pair<LatLon, LatLon> points){
    
    //Latitudes range from -90 to 90, and longitudes range from -180 to 180. 
    if (inRange(points.first.lat(), -90.0, 90.0) && inRange(points.second.lat(), -90.0, 90.0) && inRange(points.first.lon(), -180.0, 180.0) && inRange(points.second.lon(), -180.0, 180.0)){
        //convert lat and lon from degree to radian
        double points_first_lat_rad = DEGREE_TO_RADIAN * points.first.lat();
        double points_second_lat_rad = DEGREE_TO_RADIAN * points.second.lat();
        double points_first_lon_rad = DEGREE_TO_RADIAN * points.first.lon();
        double points_second_lon_rad = DEGREE_TO_RADIAN * points.second.lon();

        //calculate lat_average for x coordinate conversion
        double latitude_average = (points_first_lat_rad + points_second_lat_rad) / 2.0;

        //convert position from latlon to x and calculate x = lon(cos(latavg))
        double points_first_x = points_first_lon_rad * cos(latitude_average);
        double points_second_x = points_second_lon_rad * cos(latitude_average);

        //calculate the distance with the equation of d = R * sqrt((y2-y1)^2 + (y2-y1)^2)
        double deltay_component_squre = pow((points_second_lat_rad - points_first_lat_rad), 2);
        double deltax_component_square = pow((points_second_x - points_first_x), 2);

        return (EARTH_RADIUS_METERS * sqrt(deltay_component_squre + deltax_component_square));
    }
    else{
        std::cout << "Invalid Position" << std::endl;
        return 0;
    }
    
    
}


//Returns the length of the given street segment in meters 
//(see unload map for implementation of the data structure)
double find_street_segment_length(int street_segment_id){
    
    if (inRange(street_segment_id, 0, getNumStreetSegments() -1)){
        return G_DATABASE.street_segment_length[street_segment_id];  
    }
    else{
        std::cout << "Invalid Street Segment ID" << std::endl;
        return 0;
    }
    
}

//Returns the travel time to drive a street segment in seconds 
//(time = distance/speed_limit)
//(see unload map for implementation of the data structure)
double find_street_segment_travel_time(int street_segment_id){
    
    if (inRange(street_segment_id, 0, getNumStreetSegments() -1)){
        return G_DATABASE.street_segment_travel_time[street_segment_id];
    }
    else{
        std::cout << "Invalid Street Segment ID" << std::endl;
        return 0;
    }
    
}

//Returns the nearest intersection to the given position
int find_closest_intersection(LatLon my_position){
    
     if (inRange(my_position.lat(), MINLAT, MAXLAT) && inRange(my_position.lon(), MINLON, MAXLON )){
         
        int intersection_id = 0;
    
        //calculate the distance between me and the first intersection
        std::pair<LatLon, LatLon> intersection_distance_first (getIntersectionPosition(0),my_position);
        double length_first = find_distance_between_two_points(intersection_distance_first);

        //then find the closest intersection
        for (int i = 1; i < getNumIntersections(); i++){
            std::pair<LatLon, LatLon> intersection_distance_loop (getIntersectionPosition(i),my_position);
            double length_loop = find_distance_between_two_points(intersection_distance_loop);
            //store the shorter intersection id
            if (length_loop < length_first){
                intersection_id = i;
                length_first = length_loop;
            }
        }

        //return the closest intersection's id
        return intersection_id;
    }
    else{
        std::cout << "Invalid Position" << std::endl;
        return 0;
    }
    
    
}

std::pair<int, double> find_closest_intersection_with_length(LatLon my_position){
    
    std::pair <int,double> intersection;
     if (inRange(my_position.lat(), MINLAT, MAXLAT) && inRange(my_position.lon(), MINLON, MAXLON )){
         
        int intersection_id = 0;
    
        //calculate the distance between me and the first intersection
        std::pair<LatLon, LatLon> intersection_distance_first (getIntersectionPosition(0),my_position);
        double length_first = find_distance_between_two_points(intersection_distance_first);

        //then find the closest intersection
        for (int i = 1; i < getNumIntersections(); i++){
            std::pair<LatLon, LatLon> intersection_distance_loop (getIntersectionPosition(i),my_position);
            double length_loop = find_distance_between_two_points(intersection_distance_loop);
            //store the shorter intersection id
            if (length_loop < length_first){
                intersection_id = i;
                length_first = length_loop;
            }
        }

        //return the closest intersection's id
        intersection = std::make_pair(intersection_id, length_first);
        return intersection;
    }
    else{
        std::cout << "Invalid Position" << std::endl;
        return intersection;
    }
    
    
}

//Returns the street segments for the given intersection 
std::vector<int> find_street_segments_of_intersection(int intersection_id){
    if (inRange(intersection_id, 0, getNumIntersections() -1)){
        return G_DATABASE.intersectionDatabase[intersection_id].segmentID;
    }
    else{
        std::cout << "Invalid Intersection ID" << std::endl;
        std::vector<int> empty;
        return empty;
    }
}

//helper function defined in m1helper.h
void removeDuplicates(std::vector<int>& thedata) {
    
    std::vector<int>::iterator itr = thedata.begin();
    std::unordered_set<int> s;
    
    for (auto curr = thedata.begin(); curr != thedata.end(); ++curr) {
        if (s.insert(*curr).second)
            *itr++ = *curr;
    }
    
    thedata.erase(itr, thedata.end());
    
}


//Returns the length of the OSMWay that has the given OSMID, in meters.
//To implement this function you will have to access the OSMDatabaseAPI.h 
//functions.
double find_way_length(OSMID way_id){
    
    const OSMWay* way = G_DATABASE.wayDatabase.find(way_id)-> second;
    
    //the vector of the OSM ids of the OSM nodes in this way
    std::vector<OSMID> waymembers = getWayMembers(way);
    
    double length = 0;
    
    for (int current_way_member_index = 0; 
            current_way_member_index < waymembers.size() - 1; 
            current_way_member_index ++){
        OSMID member1 = waymembers[current_way_member_index];
        OSMID member2 = waymembers[current_way_member_index + 1];
        const OSMNode* node1 = G_DATABASE.nodeDatabase.find(member1)-> second;
        const OSMNode* node2 = G_DATABASE.nodeDatabase.find(member2)-> second;
        LatLon coordinates1 = getNodeCoords(node1);
        LatLon coordinates2 = getNodeCoords(node2);
        std::pair<LatLon, LatLon> curve_points (coordinates1, coordinates2);
        length += find_distance_between_two_points(curve_points);
    }
    
    return length; 
    
}


//Returns the nearest intersection to the given position and the distance between the point and the POI
std::pair<int, double> find_closest_POI_with_length(LatLon my_position){
    std::pair<int, double> POI;
     if (inRange(my_position.lat(), MINLAT, MAXLAT) && inRange(my_position.lon(), MINLON, MAXLON )){
         
        int POI_Index = 0;
    
        //calculate the distance between me and the first POI
        std::pair<LatLon, LatLon> POI_distance_first (getPointOfInterestPosition(0),my_position);
        double length_first = find_distance_between_two_points(POI_distance_first);

        //then find the closest POI
        for (int i = 1; i < getNumPointsOfInterest(); i++){
            std::pair<LatLon, LatLon> POI_distance_loop (getPointOfInterestPosition(i),my_position);
            double length_loop = find_distance_between_two_points(POI_distance_loop);
            //store the shorter POI id
            if (length_loop < length_first){
                POI_Index = i;
                length_first = length_loop;
            }
        }

        POI = std::make_pair(POI_Index, length_first);
        return POI;
    }
    else{
        std::cout << "Invalid Position" << std::endl;
        return POI;
    }  
}

//load the location of subways and buses
void loadSubwaysBus() {

    // Loop through all OSM relations
     for (unsigned relation = 0; relation < getNumberOfRelations(); relation++) {
        const OSMRelation *currRel = getRelationByIndex(relation);

        // Check the tag of the currRel
        for (unsigned tag = 0; tag < getTagCount(currRel); tag++) {
            std::pair<std::string, std::string> tagPair = getTagPair(currRel, tag);

            // Push relations with the route=subway tag
            if (tagPair.first == "route" && tagPair.second == "subway") {
                std::string subwaycolor;
                // Get subway line color and name
                for (unsigned innertag = 0; innertag < getTagCount(currRel); innertag++) {
                    std::pair<std::string, std::string> innertagPair = getTagPair(currRel, innertag);

                    if (innertagPair.first == "colour") {
                        subwaycolor = innertagPair.second;
                    } 
                }
                std::vector<TypedOSMID> route_members = getRelationMembers(currRel);
                for (unsigned route = 0; route < route_members.size(); route++) {
                    // A member of type node represents a subway station
                    if (route_members[route].type() == TypedOSMID::Node) {
                        // Node lookup by OSMID
                        const OSMNode *currNode =  G_DATABASE.nodeDatabase.find(route_members[route])-> second;
                        std::string station_name;
                        
                        for (unsigned innertag = 0; innertag < getTagCount(currNode); innertag++) {
                            std::pair<std::string, std::string> innertagPair = getTagPair(currNode, innertag);

                            if (innertagPair.first == "name") {
                                station_name = innertagPair.second;
                            } 
                        }
                        
                        //relevant information found and load to database
                        subwaystation newinfo;
                        newinfo.name = station_name;
                        newinfo.color = subwaycolor;
                        newinfo.station_node = *currNode;
                        G_DATABASE.subwayinfo.push_back(newinfo);

                    } else if (route_members[route].type() == TypedOSMID::Way) {
                        const OSMWay *currWay =  G_DATABASE.wayDatabase.find(route_members[route])-> second;
                        
                        //relevant information found and load to database
                        subwaysegments newline;
                        newline.color = subwaycolor;
                        newline.station_line = *currWay;
                        G_DATABASE.subwaylineinfo.push_back(newline);
                    }
                }
                break;
            } 
            else if ((G_DATABASE.current_map_path.size() == 0 || G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/toronto_canada.streets.bin")
                    && tagPair.first == "route" && (tagPair.second == "bus" || tagPair.second == "streetcar")) {
                std::string buscolor;
                // Get subway line color and name
                for (unsigned innertag = 0; innertag < getTagCount(currRel); innertag++) {
                    std::pair<std::string, std::string> innertagPair = getTagPair(currRel, innertag);

                    if (innertagPair.first == "colour") {
                        buscolor = innertagPair.second;
                    } 
                }
                std::vector<TypedOSMID> route_members = getRelationMembers(currRel);
                for (unsigned route = 0; route < route_members.size(); route++) {
                    // A member of type node represents a subway station
                    if (route_members[route].type() == TypedOSMID::Node) {
                        // Node lookup by OSMID
                        const OSMNode *currNode =  G_DATABASE.nodeDatabase.find(route_members[route])-> second;
                        
                        //relevant information found and load to database
                        busstation newinfo;
                        newinfo.color = buscolor;
                        newinfo.station_node = *currNode;
                        G_DATABASE.businfo.push_back(newinfo);

                    } else if (route_members[route].type() == TypedOSMID::Way) {
                        const OSMWay *currWay =  G_DATABASE.wayDatabase.find(route_members[route])-> second;
                        
                        //relevant information found and load to database
                        bussegments newline;
                        newline.color = buscolor;
                        newline.station_line = *currWay;
                        G_DATABASE.buslineinfo.push_back(newline);
                    }
                }
                break;
            }
        }
    }
    
}

/*
 *  convert latlon to 2d coordinates
 */

double x_from_lon(double lon) {
    return lon * (cos (G_DATABASE.avg_lat * DEGREE_TO_RADIAN));
}

double y_from_lat(double lat) {
    return lat;
}

double lon_from_x(double x) {
    return x / (cos(G_DATABASE.avg_lat * DEGREE_TO_RADIAN));
}

double lat_from_y(double y) {
    return y;
}




