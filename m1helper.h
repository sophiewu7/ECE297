/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m1helper.h
 * Author: hanyi13
 *
 * Created on January 29, 2020, 7:29 PM
 */

#ifndef M1HELPER_H
#define M1HELPER_H



#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <string>


constexpr double MAXLAT = 90.0;
constexpr double MINLAT = -90.0;  
constexpr double MAXLON = 180.0;
constexpr double MINLON = -180.0;
constexpr int HOUR_TO_SECOND = 3600;
constexpr int KILOMETER_TO_METER = 1000;



struct InfoIntersection {
    //segments that the intersection is directly connected
    std::vector<StreetSegmentIndex> segmentID; 
    //adjacent intersection id
    std::vector<IntersectionIndex> adjacentID; 
    //the streets that the intersection is on
    std::vector<StreetIndex> streetID;
    //position of lattitude and lontitude
    LatLon position;
    //name of intersection
    std::string name;
    //position of x and y coordinates
    float x;
    float y;
    bool highlight = false;
    
};

struct InfoStreet {
    //all segments the street contains
    std::vector<StreetSegmentIndex> segmentID;
    //all intersections the street contains
    std::vector<IntersectionIndex> intersectionID;
};

struct segmentcurvature {
    double r_squared;
    double average_angle;
    double x_start;
    double y_start;
};

struct XYcoord {
    double x;
    double y;
};


struct POI {
    std::string type;
    std::string name;
    LatLon position;
    OSMID OSMNodeID;
    bool highlight = false;
};

struct star{
    std::string type;
    std::string name;
    double x;
    double y;
    bool highlight = false;
};


struct subwaystation {
    std::string name;
    std::string color;
    OSMNode station_node;
};

struct subwaysegments {
    std::string color;
    OSMWay station_line;
};

struct busstation {
    std::string color;
    OSMNode station_node;
};

struct bussegments {
    std::string color;
    OSMWay station_line;
};



void removeDuplicates(std::vector<int>& thedata);

void load_street();
void load_street_segment();
void load_database();
void segmentNodesOSM ();
void loadSubwaysBus();
void loadBus();

std::pair<int, double> find_closest_intersection_with_length(LatLon my_position);
std::pair<int, double> find_closest_POI_with_length(LatLon my_position);

struct Global {
    //all global variables declared by a Global Structure;
    std::multimap<std::string, int> street_names;
    std::vector<double> street_segment_length;
    std::vector<double> street_segment_travel_time;

    std::vector<InfoIntersection> intersectionDatabase;
    std::vector<InfoStreet> streetDatabase;
    std::vector<int> major_streets;
    std::unordered_map <int, segmentcurvature > major_streets_curvature;
    std::map<double, FeatureIndex> featureDatabase;

    std::unordered_map<OSMID, const OSMWay*> wayDatabase;
    std::unordered_map<OSMID, const OSMNode*> nodeDatabase;
    
    std::vector<star> starDatabase;
    std::pair<double , double > mouseClick;
    
    std::vector<POI> POI_Database;
    
    std::vector<subwaystation> subwayinfo;
    std::vector<subwaysegments> subwaylineinfo;
    std::unordered_map<std::string, int> subwayStationPrintedDatabase;
    
    std::vector<const OSMRelation *> osm_bus_lines;
    std::vector<busstation> businfo;
    std::vector<bussegments> buslineinfo;
    
    std::vector<std::unordered_map<IntersectionIndex, StreetSegmentIndex>> connected_segment;
    
    std::vector<double>speed_limit;
    double max_lat;
    double min_lat;
    double max_lon;
    double min_lon;
    double avg_lat;
    double average_speed_limit;
    std::string current_map_path;
    double home_x = 0;
    double home_y = 0;
};



template<typename Type>
bool inRange(Type num, Type low, Type high) {
    if (num < low || num > high) {
        return false;
    }
    return true;
}

double x_from_lon(double lon);
double y_from_lat(double lat);
double lon_from_x(double x);
double lat_from_y(double y);

#endif /* M1HELPER_H */

