/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */ 

#include <gtk/gtk.h>
#include <gtk/gtkcomboboxtext.h>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/control.hpp"
#include "m1.h"
#include "m1helper.h"
#include "m2.h"
#include "globaldatabase.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "point.hpp"
#include "color.hpp"
#include "libcurl.h"
#include "math.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include "m3.h"
//Global Variables
bool TRANSIT_MODE = false;
bool DRIVE_MODE = false;
bool GENERAL_MODE = true;

std::vector<XYcoord> DRAWNPOINTS;

std::vector<extBus> REALTIME_BUS;
std::vector<extClosure> REALTIME_CLOSURE;


/* The following functions are: 
 * Functions for drawing map's components: features, street, point of interests, buildings, etc
 * Function for printing the names of the above.
 * Related helper Functions
 */ 

//Draw Map
void draw_map() {
    
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    ezgl::rectangle initial_world ({x_from_lon(G_DATABASE.min_lon), y_from_lat(G_DATABASE.min_lat)}, 
                                    {x_from_lon(G_DATABASE.max_lon),y_from_lat(G_DATABASE.max_lat)});
    application.add_canvas("MainCanvas",draw_main_canvas, initial_world);
    
    application.run(initial_setup, act_on_mouse_press, nullptr, act_on_key_press);
    
}

//Print Map Names (Only display the city name at the default zoom level)
void print_map_name(ezgl::renderer *g, int level){
    
    g->set_color(ezgl::MAP_NAME);
    g->set_text_rotation(TEXT_ROTATION_DEFAULT);
    g->set_font_size(55);
    
    if (level == -2){
        if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/iceland.streets.bin"){
            g->draw_text({ICELAND_X, ICELAND_Y}, "Iceland");
        }
    }
    if (level == -1){
        if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/hong-kong_china.streets.bin"){
            g->set_font_size(40);
            g->draw_text({SHENZHEN_X, SHENZHEN_Y}, "Shenzhen");
            g->draw_text({MACAU_X, MACAU_Y}, "Macau");
            g->draw_text({HONGKONG_X, HONGKONG_Y}, "Hong Kong");
            g->draw_text({GUANGZHOU_X, GUANGZHOU_Y}, "Guangzhou");
        }else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/tokyo_japan.streets.bin"){
            g->draw_text({TOKYO_X, TOKYO_Y}, "Tokyo");            
        }
    }
    if (level == 0){
        if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin"){
            g->draw_text({RIO_X, RIO_Y}, "Rio de Janeiro");
        }
    }
    if (level == 1){
        if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/beijing_china.streets.bin"){
            g->draw_text({BEIJING_X, BEIJING_Y}, "Beijing");
        }
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/toronto_canada.streets.bin"){
            g->draw_text({TORONTO_X, TORONTO_Y}, "Toronto");
        }
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/sydney_australia.streets.bin"){
            g->draw_text({SYDNEY_X, SYDNEY_Y}, "Sydney");
        }
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/singapore.streets.bin"){
            g->draw_text({SINGAPORE_X, SINGAPORE_Y}, "Singapore");
        }
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/new-york_usa.streets.bin"){
            g->draw_text({NEW_YORK_X, NEW_YORK_Y}, "New York");
        }
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/new-delhi_india.streets.bin"){
            g->draw_text({NEW_DELHI_X, NEW_DELHI_Y}, "New Delhi");
        }
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/moscow_russia.streets.bin"){
            g->draw_text({MOSCOW_X, MOSCOW_Y}, "Moscow");
        }
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/london_england.streets.bin"){
            g->draw_text({LONDON_X, LONDON_Y}, "London");
        }
        else{
            g->draw_text({TORONTO_X, TORONTO_Y}, "Toronto");
        }
    }
    if (level == 2){
        if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/cairo_egypt.streets.bin"){
            g->draw_text({CAIRO_X,CAIRO_Y}, "Cairo");    
        }
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/hamilton_canada.streets.bin"){
            g->draw_text({HAMILTON_X, HAMILTON_Y}, "Hamilton");
           
        }
    }
    if (level == 4){
        if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/saint-helena.streets.bin"){
            g->draw_text({SAINT_HELENA_X, SAINT_HELENA_Y}, "Saint Helena");
        }
    }

}

//Draw the main canvas to display the map
void draw_main_canvas(ezgl::renderer *g) {
    
    //Define zoom levels
    int level;
    zoom_levels(g, level);
    
    ezgl::rectangle rec = g->get_visible_world();
    //clear and refresh drawnpoints once drawing
    DRAWNPOINTS.clear();

    //Set font according to cities
    std::string font_name;
    if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/tehran_iran.streets.bin"){ //Special consideration for Iran
        font_name = "Noto Kufi Arabic"; //however, not able to display English
    }
    else{
        font_name = "Noto Sans CJK SC";
    }
    g->format_font(font_name, ezgl::font_slant::normal, ezgl::font_weight::normal);

    /* 
    * The following functions are called for drawing the map
    * Order matters otherwise small objects might get overdrawn by bigger objects
    */ 
    
    //draw the canvas
    draw_main_canvas_blank(g, rec);
    
    //draw feature
    draw_features(rec, g, level);
    
    //draw street
    draw_streets(rec, g, level);
    
//     std::vector<StreetSegmentIndex> hi = find_path_between_intersections(
//		            13, 
//                            51601,
//                            0);
    
  //  draw_path_segments(rec, g, hi);
    
    //draw and highlight intersections
    highlight_intersections(rec, g);
    
    //draw subways & bus from OSM database
    draw_subways(g, level);
    draw_bus(g, level);
    
    //draw subways & bus from libcurl
    draw_current_bus(g, level);
    draw_current_closure(g, level);
    
    //draw point of interest
    draw_POI(g, level);
    
    //draw starred location
    draw_star(g, level);
    
    //print major street name in low zoom level
    draw_major_street_names(rec, g, level);
    
    //print feature name
    print_feature_name(g, level);
    
    
    //print cities' name
    print_map_name(g, level);
    
    G_DATABASE.subwayStationPrintedDatabase.clear();

}

//Draw the blank background
void draw_main_canvas_blank(ezgl::renderer *g, ezgl::rectangle rec) {
    
    g->set_color(ezgl::BLANK_CANVAS);
    g->fill_rectangle({rec.m_first.x, rec.m_first.y}, 
                        {rec.m_second.x, rec.m_second.y});
    
}

//Draw streets
void draw_streets(ezgl::rectangle rec, ezgl::renderer *g, int level) {
    
    //draw segments first
    for (int street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++){
        
        InfoStreetSegment street_segment_info = getInfoStreetSegment(street_segment_id);  
        
        //catagorize by speedlimit to set street line width
        if (street_segment_info.speedLimit >= 90) {
            
            g->set_color(ezgl::HIGHEST_SPEED);
            if (level <= 5) g->set_line_width(5);
            else if (level <= 7) g->set_line_width(6);
            else if (level <= 9) g->set_line_width(9);
            else if (level <= 11) g->set_line_width(11);
            else if (level <= 13) g->set_line_width(16);
            else g->set_line_width(25);
            
        } else if (street_segment_info. speedLimit >= 60) { 
            
            g->set_color(ezgl::MEDIUM_SPEED);
            if (level <= 3) g->set_line_width(0);
            else if (level <= 5) g->set_line_width(2);
            else if (level <= 9) g->set_line_width(4);
            else if (level <= 11) g->set_line_width(8);
            else if (level <= 13) g->set_line_width(11);
            else g->set_line_width(13);
            
        } else {
            
            g->set_color(ezgl::LOWEST_SPEED);
            if (level <= 3) continue;
            else if (level <= 5) g->set_line_width(0);
            else if (level <= 9) g->set_line_width(2);
            else if (level <= 11) g->set_line_width(6);
            else if (level <= 13) g->set_line_width(9);
            else g->set_line_width(11);
            
        }
        //call helper function to draw segments
        draw_segment_helper(rec, g, street_segment_info, street_segment_id);  
    }
    
    //draw segment names on top of segments
    for (int street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++) {
        InfoStreetSegment street_segment_info = getInfoStreetSegment(street_segment_id);  
        draw_segment_name(rec, g, street_segment_info, street_segment_id, level);
    }
    
}

//Draw street segment helper
void draw_segment_helper(ezgl::rectangle rec, ezgl::renderer *g, InfoStreetSegment& street_segment_info, int street_segment_id) {
    
    //xy coordinates of two endpoints of passed-in segments
    double xsegfrom = G_DATABASE.intersectionDatabase[street_segment_info.from].x;
    double ysegfrom = G_DATABASE.intersectionDatabase[street_segment_info.from].y;
    double xsegto = G_DATABASE.intersectionDatabase[street_segment_info.to].x;
    double ysegto = G_DATABASE.intersectionDatabase[street_segment_info.to].y;
    
    if (street_segment_info.curvePointCount == 0) {
        //check if it is within world 
        if (crossScreen(rec, {xsegfrom, ysegfrom}, {xsegto, ysegto})) 
            g->draw_line({xsegfrom, ysegfrom}, {xsegto, ysegto});

    } else if (street_segment_info.curvePointCount == 1) {
        //xy coordinate of the only curvepoint
        double x_mid= x_from_lon(getStreetSegmentCurvePoint(0, street_segment_id).lon() );
        double y_mid = y_from_lat(getStreetSegmentCurvePoint(0, street_segment_id).lat() );
        
        //draw two subsegments
        if (crossScreen(rec, {xsegfrom, ysegfrom}, {x_mid, y_mid})) 
            g->draw_line({xsegfrom, ysegfrom}, {x_mid, y_mid});
        if (crossScreen(rec, {x_mid, y_mid}, {xsegto, ysegto})) 
            g->draw_line({x_mid, y_mid}, {xsegto, ysegto});

    } else {
        //multiple curvepoints
        for (int pointIndex = 0; pointIndex < street_segment_info.curvePointCount - 1; pointIndex++) {
            double x_from = x_from_lon( getStreetSegmentCurvePoint(pointIndex, street_segment_id).lon() );
            double y_from = y_from_lat( getStreetSegmentCurvePoint(pointIndex, street_segment_id).lat() );
            double x_to = x_from_lon(getStreetSegmentCurvePoint(pointIndex + 1, street_segment_id).lon());
            double y_to = y_from_lat(getStreetSegmentCurvePoint(pointIndex + 1, street_segment_id).lat());
            
            //draw middle subsegments
            if (crossScreen(rec, {x_from, y_from}, {x_to, y_to}) ) 
                g->draw_line({x_from, y_from}, {x_to, y_to});
            //draw the two end_subsegments
            if (pointIndex == 0 && crossScreen(rec, {x_from, y_from}, {xsegfrom, ysegfrom}) ) 
                g->draw_line({xsegfrom, ysegfrom}, {x_from, y_from});
            if (pointIndex == street_segment_info.curvePointCount - 2 && crossScreen(rec, {x_to, y_to}, {xsegto, ysegto}) ) 
                g->draw_line({x_to, y_to}, {xsegto, ysegto});        
        }
    }
}

//Draw street segment name in lower level
void draw_segment_name(ezgl::rectangle rec, ezgl::renderer *g, InfoStreetSegment& segement_info, int segment_id, int level) {
    
    //angle of the street between two endpoint(not accurate by now)
    double x_from= G_DATABASE.intersectionDatabase[segement_info.from].x;
    double y_from = G_DATABASE.intersectionDatabase[segement_info.from].y;
    double x_to = 0.0;
    double y_to = 0.0;
    double x_difference = 0.0;
    double y_difference = 0.0;
    double x_mid = 0.0;
    double y_mid = 0.0;
    double text_degree = 0.0;
    double text_x_bound = 0.0;
    
    int numOfCurvePoint = segement_info.curvePointCount;
    
    //straight street
    if (numOfCurvePoint == 0) {
        x_to = G_DATABASE.intersectionDatabase[segement_info.to].x;
        y_to = G_DATABASE.intersectionDatabase[segement_info.to].y;
    
        x_difference = x_to - x_from;
        y_difference = y_to - y_from;
    
        x_mid = (x_from + x_to) / 2;
        y_mid = (y_from + y_to) / 2;
    
        text_degree = (RADIAN_TO_DEGREE) * atan(y_difference / x_difference);
        
        //to make street name not crowded
        text_x_bound = sqrt(pow(x_difference, 2) + pow(y_difference, 2)) / 2;
        
        draw_segment_name_helper(g, segement_info, x_difference, y_difference, x_mid,  y_mid, text_degree, text_x_bound, TEXT_BOUND_Y_SMALL_LEVEL, level);
                
    }else{
        //have curvepoints
        for (int curvePoint = 0; curvePoint < numOfCurvePoint; curvePoint++){
            
            x_to = x_from_lon(getStreetSegmentCurvePoint(curvePoint, segment_id).lon());
            y_to = y_from_lat(getStreetSegmentCurvePoint(curvePoint, segment_id).lat());
            
            if (!crossScreen(rec, {x_to, y_to}, {x_from, y_from})) {//won't appear in window, don't draw
                x_from = x_to;
                y_from = y_to;
                continue;
            }
            
            x_difference = x_to - x_from;
            y_difference = y_to - y_from;
    
            x_mid = (x_from + x_to) / 2;
            y_mid = (y_from + y_to) / 2;
    
            //determine text rotation and bound condition
            text_degree = (RADIAN_TO_DEGREE) * atan(y_difference / x_difference);
            text_x_bound = sqrt(pow(x_difference, 2) + pow(y_difference, 2)) / 2;
            
            draw_segment_name_helper(g, segement_info, x_difference, y_difference, x_mid,  y_mid, text_degree,  text_x_bound, TEXT_BOUND_Y_SMALL_LEVEL, level);
            
            x_from = x_to;
            y_from = y_to;
        }      
    }  
    
}

//helper function for printing street name
void draw_segment_name_helper(ezgl::renderer *g, InfoStreetSegment& segment_info, double x_difference, double y_difference, 
                                    double x_mid, double y_mid, double text_degree, double text_x_bound, double text_y_bound, int level){
    
    g->set_color(60,60,60);
    
    //give street name different font size under different zoom level
    
    if (level >= 12) g->set_font_size(12);
    else g->set_font_size(8);
    
    //rotate the text accordingly
    g->set_text_rotation(text_degree);
                
    //Only print street that has a name
    if (getStreetName(segment_info.streetID)!= "<unknown>"){
        //handle one way street
        if (segment_info.oneWay){
            if (rightOneWay(x_difference, y_difference)){
                std::string rightOneWay_name = " -> " + getStreetName(segment_info.streetID) + " -> ";
                g->draw_text({x_mid, y_mid}, rightOneWay_name, text_x_bound, text_y_bound);
            }
            else{
                std::string leftOneWay_name = " <- " + getStreetName(segment_info.streetID) + " <- ";
                g->draw_text({x_mid, y_mid}, leftOneWay_name, text_x_bound, text_y_bound);
            }
        }
        //not a one way street
        else{
            std::string streetName = getStreetName(segment_info.streetID);
            g->draw_text({x_mid, y_mid}, streetName, text_x_bound, text_y_bound);
        }
    }
    
}

//use to display major street names in small zoom levels
void draw_major_street_names(ezgl::rectangle rec, ezgl::renderer *g, int level) {
    
    if (level <= 2 || level >= 9) return;
    g->set_color(ezgl::GREY25);
    
    double screenX = rec.m_second.x - rec.m_first.x;
    double screenY = rec.m_second.y - rec.m_first.y;
    //defined length to seperate names in a distance
    double length;
    if (level <= 4)
        length = (sqrt((screenX*screenX) + (screenY * screenY)))/6.0;
    else 
        length = (sqrt((screenX*screenX) + (screenY * screenY)))/8.0;
    
    for (auto street = 0; street < G_DATABASE.major_streets.size(); street ++) {
        std::string name = getStreetName(G_DATABASE.major_streets[street]);
        //current major street
        std::vector<int> segments = G_DATABASE.streetDatabase[G_DATABASE.major_streets[street]].segmentID; //segment indices
        int curr_street_drawn = 0;
        for (auto current_segment = 0; current_segment < segments.size(); current_segment ++) {
            //current segment's id 
            int id = segments[current_segment];
            segmentcurvature info = G_DATABASE.major_streets_curvature.find(id)->second;
            double r_squared = info.r_squared;
            double average_angle = info.average_angle;
            
            if (withinScreen(rec, info.x_start, info.y_start) && curr_street_drawn < 2) {
                bool close = false;
                if (DRAWNPOINTS.size() > 0) {
                    double distance;
                    //loop to find if other names are drawn within length of the current segment location
                    for (auto drawn = 0; drawn < DRAWNPOINTS.size(); drawn++) {
                        double x_drawn = DRAWNPOINTS[drawn].x;
                        double y_drawn = DRAWNPOINTS[drawn].y;
                        double delta_x_square = (x_drawn - info.x_start) * (x_drawn - info.x_start);
                        double delta_y_square = (y_drawn - info.y_start) * (y_drawn - info.y_start);
                        distance = sqrt(delta_x_square + delta_y_square);
                        if (distance < length) {
                            close = true;
                            break;
                        }
                    }
                }
                //if no other names drawn within set length 
                //if the segment is not curved graphically
                if (!close && r_squared < 5) {
                    if (level <= 4) g->set_font_size(7);
                    else  g->set_font_size(8);
                    g->set_text_rotation(average_angle);
                    g->draw_text({info.x_start, info.y_start}, name);
                    //update number of drawn points and number of curr_street drawn names
                    DRAWNPOINTS.push_back({info.x_start, info.y_start});
                    curr_street_drawn++;
                }
            }
        }      
    }
}

//Help to determine the direction of one way street 
bool rightOneWay(double x_difference, double y_difference){
    
    if ((x_difference > 0) || (x_difference = 0 && y_difference > 0)){
        return true;
    }else{
        return false; //left one way
    }
    
}

//Draw points of interest
void draw_POI(ezgl::renderer *g, int level) {
    
    ezgl::rectangle rec = g->get_visible_world();   
    ezgl::rectangle screen = g->get_visible_screen();  
 
    //load png icon for unclicked POI
    ezgl::surface *general = ezgl::renderer::load_png("libstreetmap/resources/general.png");
    ezgl::surface *airport = ezgl::renderer::load_png("libstreetmap/resources/airport.png");
    ezgl::surface *atm = ezgl::renderer::load_png("libstreetmap/resources/atm.png");
    ezgl::surface *bank = ezgl::renderer::load_png("libstreetmap/resources/bank.png");
    ezgl::surface *bus = ezgl::renderer::load_png("libstreetmap/resources/busstop.png");
    ezgl::surface *cafe = ezgl::renderer::load_png("libstreetmap/resources/cafe.png");
    ezgl::surface *church = ezgl::renderer::load_png("libstreetmap/resources/church.png");
    ezgl::surface *fastfood = ezgl::renderer::load_png("libstreetmap/resources/fastfood.png");
    ezgl::surface *firestation = ezgl::renderer::load_png("libstreetmap/resources/fireman.png");
    ezgl::surface *gas = ezgl::renderer::load_png("libstreetmap/resources/gas.png");
    ezgl::surface *hospital = ezgl::renderer::load_png("libstreetmap/resources/hospital.png");
    ezgl::surface *library = ezgl::renderer::load_png("libstreetmap/resources/library.png");
    ezgl::surface *parking = ezgl::renderer::load_png("libstreetmap/resources/parking.png");
    ezgl::surface *restaurant = ezgl::renderer::load_png("libstreetmap/resources/restaurant.png");
    ezgl::surface *theater = ezgl::renderer::load_png("libstreetmap/resources/theater.png");
    ezgl::surface *university = ezgl::renderer::load_png("libstreetmap/resources/university.png");
  
    //load png icon for clicked POI (highlight)
    ezgl::surface *general_hl = ezgl::renderer::load_png("libstreetmap/resources/generalH.png");
    ezgl::surface *airport_hl = ezgl::renderer::load_png("libstreetmap/resources/airportH.png");
    ezgl::surface *atm_hl = ezgl::renderer::load_png("libstreetmap/resources/atmH.png");
    ezgl::surface *bank_hl = ezgl::renderer::load_png("libstreetmap/resources/bankH.png");
    ezgl::surface *bus_hl = ezgl::renderer::load_png("libstreetmap/resources/busstopH.png");
    ezgl::surface *cafe_hl = ezgl::renderer::load_png("libstreetmap/resources/cafeH.png");
    ezgl::surface *church_hl = ezgl::renderer::load_png("libstreetmap/resources/churchH.png");
    ezgl::surface *fastfood_hl = ezgl::renderer::load_png("libstreetmap/resources/fastfoodH.png");
    ezgl::surface *firestation_hl = ezgl::renderer::load_png("libstreetmap/resources/firemanH.png");
    ezgl::surface *gas_hl = ezgl::renderer::load_png("libstreetmap/resources/gasH.png");
    ezgl::surface *hospital_hl = ezgl::renderer::load_png("libstreetmap/resources/hospitalH.png");
    ezgl::surface *library_hl = ezgl::renderer::load_png("libstreetmap/resources/libraryH.png");
    ezgl::surface *parking_hl = ezgl::renderer::load_png("libstreetmap/resources/parkingH.png");
    ezgl::surface *restaurant_hl = ezgl::renderer::load_png("libstreetmap/resources/restaurantH.png");
    ezgl::surface *theater_hl = ezgl::renderer::load_png("libstreetmap/resources/theaterH.png");
    ezgl::surface *university_hl = ezgl::renderer::load_png("libstreetmap/resources/universityH.png");
    
    //Only drawn POI for zoom level greater than 10
    if (level > 10){
        
            int total_POI = getNumPointsOfInterest();
            
            //Loops through all POI
            for (int POI_Index = 0; POI_Index < total_POI; POI_Index++){
                
                //get each POI's x and y location
                double x_pos = x_from_lon(G_DATABASE.POI_Database[POI_Index].position.lon());
                double y_pos = y_from_lat(G_DATABASE.POI_Database[POI_Index].position.lat());
                if (withinScreen(rec, x_pos, y_pos)){
                    //find specific POI that we has special icon for
                    if (G_DATABASE.POI_Database[POI_Index].type == "terminal" ||
                            G_DATABASE.POI_Database[POI_Index].type == "aerodrome" ){
                        //check if the POI is highlighted
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(airport_hl, icon_shift(rec, screen, {x_pos, y_pos}));
                            //print POI name if highlighted
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(airport,icon_shift(rec, screen, {x_pos, y_pos}));
                        }
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "atm" ){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(atm_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(atm, icon_shift(rec, screen, {x_pos, y_pos}));
                        }
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "bank" ){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(bank_hl, icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(bank,icon_shift(rec, screen, {x_pos, y_pos}));
                        } 
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "bus_station" ){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(bus_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(bus,icon_shift(rec, screen, {x_pos, y_pos}));
                        }       
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "cafe" ){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(cafe_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(cafe,icon_shift(rec, screen, {x_pos, y_pos}));
                        }
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "church" ){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(church_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(church,icon_shift(rec, screen, {x_pos, y_pos}));
                        }
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "fast_food" ){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(fastfood_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(fastfood,icon_shift(rec, screen, {x_pos, y_pos}));
                        }                 
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "fire_station" ){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(firestation_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(firestation,icon_shift(rec, screen, {x_pos, y_pos}));
                        }                      
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "fuel" ){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(gas_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(gas,icon_shift(rec, screen, {x_pos, y_pos}));
                        }    
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "doctors" ||
                                G_DATABASE.POI_Database[POI_Index].type == "hospital"){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(hospital_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(hospital,icon_shift(rec, screen, {x_pos, y_pos}));
                        }
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "library" ||
                                G_DATABASE.POI_Database[POI_Index].type == "books"){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(library_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(library,icon_shift(rec, screen, {x_pos, y_pos}));
                        }
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "parking"){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(parking_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(parking,icon_shift(rec, screen, {x_pos, y_pos}));
                        }               
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "restaurant" ||
                                G_DATABASE.POI_Database[POI_Index].type == "food_court"){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(restaurant_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(restaurant,icon_shift(rec, screen, {x_pos, y_pos}));
                        }   
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "theater" ){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(theater_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(theater,icon_shift(rec, screen, {x_pos, y_pos}));
                        }                
                    }
                    else if (G_DATABASE.POI_Database[POI_Index].type == "university" ||
                            G_DATABASE.POI_Database[POI_Index].type == "school" ||
                            G_DATABASE.POI_Database[POI_Index].type == "college"){
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(university_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(university,icon_shift(rec, screen, {x_pos, y_pos}));
                        }         
                    }
                    else{//all other POI that has a general icon for
                        if (G_DATABASE.POI_Database[POI_Index].highlight){
                            g->draw_surface(general_hl,icon_shift(rec, screen, {x_pos, y_pos}));
                            print_POI_name(POI_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(general,icon_shift(rec, screen, {x_pos, y_pos}));
                        }   
                    }
                }
            }
       }
    
    //free normal icons
    ezgl::renderer::free_surface(general);
    ezgl::renderer::free_surface(airport);
    ezgl::renderer::free_surface(atm);
    ezgl::renderer::free_surface(bank);
    ezgl::renderer::free_surface(bus);
    ezgl::renderer::free_surface(cafe);
    ezgl::renderer::free_surface(church);
    ezgl::renderer::free_surface(fastfood);
    ezgl::renderer::free_surface(firestation);
    ezgl::renderer::free_surface(gas);
    ezgl::renderer::free_surface(hospital);
    ezgl::renderer::free_surface(library);
    ezgl::renderer::free_surface(parking);
    ezgl::renderer::free_surface(restaurant);
    ezgl::renderer::free_surface(theater);
    ezgl::renderer::free_surface(university);
    
    //free highlighted icons
    ezgl::renderer::free_surface(general_hl);
    ezgl::renderer::free_surface(airport_hl);
    ezgl::renderer::free_surface(atm_hl);
    ezgl::renderer::free_surface(bank_hl);
    ezgl::renderer::free_surface(bus_hl);
    ezgl::renderer::free_surface(cafe_hl);
    ezgl::renderer::free_surface(church_hl);
    ezgl::renderer::free_surface(fastfood_hl);
    ezgl::renderer::free_surface(firestation_hl);
    ezgl::renderer::free_surface(gas_hl);
    ezgl::renderer::free_surface(hospital_hl);
    ezgl::renderer::free_surface(library_hl);
    ezgl::renderer::free_surface(parking_hl);
    ezgl::renderer::free_surface(restaurant_hl);
    ezgl::renderer::free_surface(theater_hl);
    ezgl::renderer::free_surface(university_hl);

}

//Print point of interest name
void print_POI_name(int POI_Index, ezgl::renderer *g, double x_pos, double y_pos){
    
    g->set_color(ezgl::BLACK);
    g->set_text_rotation(TEXT_ROTATION_DEFAULT);
    g->set_font_size(10);
    g->draw_text({x_pos, y_pos}, G_DATABASE.POI_Database[POI_Index].name);
    
}

//Draw highlight clicked or found intersections
void highlight_intersections(ezgl::rectangle rec, ezgl::renderer *g){
    
    struct intersection_data{
        LatLon position;
        bool hightlight = false;
    };
    
    for (auto intersection_index = 0; intersection_index < getNumIntersections(); intersection_index ++) {
        double xpos = G_DATABASE.intersectionDatabase[intersection_index].x;
        double ypos = G_DATABASE.intersectionDatabase[intersection_index].y;
        
        //not in screen, don't draw
        if (!withinScreen(rec, xpos - INTERSECTION_RADIUS, ypos - INTERSECTION_RADIUS) && !withinScreen(rec, xpos + INTERSECTION_RADIUS, ypos + INTERSECTION_RADIUS) 
            && !withinScreen(rec, xpos + INTERSECTION_RADIUS, ypos - INTERSECTION_RADIUS) && !withinScreen(rec, xpos - INTERSECTION_RADIUS, ypos + INTERSECTION_RADIUS)) {
            continue;
        }
            
        if (G_DATABASE.intersectionDatabase[intersection_index].highlight){ //is highlighted
            g->set_color(ezgl::RED);
        }
        else{ //not highlighted
            g->set_color(ezgl::NORMAL_INTERSECTION);
        }
        
        g->fill_arc({xpos, ypos}, INTERSECTION_RADIUS, 0, 360);
    }
    
}

//Draw stared locations
void draw_star(ezgl::renderer *g, int level){
    
    ezgl::rectangle rec = g->get_visible_world();   
    ezgl::rectangle screen = g->get_visible_screen();  
    
    //load path for unhighlighted star png
    ezgl::surface *star_food = ezgl::renderer::load_png("libstreetmap/resources/star_yellow.png");
    ezgl::surface *star_work = ezgl::renderer::load_png("libstreetmap/resources/star_red.png");
    ezgl::surface *star_study = ezgl::renderer::load_png("libstreetmap/resources/star_green.png");
    ezgl::surface *star_transit = ezgl::renderer::load_png("libstreetmap/resources/star_blue.png");
    ezgl::surface *star_home = ezgl::renderer::load_png("libstreetmap/resources/home.png");
    ezgl::surface *star_general = ezgl::renderer::load_png("libstreetmap/resources/pin.png");
    
    //load path for highlighted star png
    ezgl::surface *star_food_hl = ezgl::renderer::load_png("libstreetmap/resources/star_yellowH.png");
    ezgl::surface *star_work_hl = ezgl::renderer::load_png("libstreetmap/resources/star_redH.png");
    ezgl::surface *star_study_hl = ezgl::renderer::load_png("libstreetmap/resources/star_greenH.png");
    ezgl::surface *star_transit_hl = ezgl::renderer::load_png("libstreetmap/resources/star_blueH.png");
    ezgl::surface *star_home_hl = ezgl::renderer::load_png("libstreetmap/resources/homeH.png");
    ezgl::surface *star_general_hl = ezgl::renderer::load_png("libstreetmap/resources/pinH.png");
    
    //Only drawn starred locations for zoom level greater than 3
    if (level > 3){
        
            int total_star = G_DATABASE.starDatabase.size();
            //Loops through all starred locations
            for (int star_Index = 0; star_Index < total_star; star_Index++){
                //get each star's x and y location
                double x_pos = G_DATABASE.starDatabase[star_Index].x;
                double y_pos = G_DATABASE.starDatabase[star_Index].y;
                //load different png for different types of star location if it is within current screen
                if (withinScreen(rec, x_pos, y_pos)){
                    if (G_DATABASE.starDatabase[star_Index].type == "food" ){
                        //check if the POI is highlighted
                        if (G_DATABASE.starDatabase[star_Index].highlight){
                            g->draw_surface(star_food_hl, icon_shift(rec, screen, {x_pos, y_pos}));
                            //print POI name if highlighted
                            print_star_name(star_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(star_food, icon_shift(rec, screen, {x_pos, y_pos}));
                        }
                    }

                    else if (G_DATABASE.starDatabase[star_Index].type == "work" ){
                        if (G_DATABASE.starDatabase[star_Index].highlight){
                            g->draw_surface(star_work_hl, icon_shift(rec, screen, {x_pos, y_pos}));
                            print_star_name(star_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(star_work, icon_shift(rec, screen, {x_pos, y_pos}));
                        }
                    }

                    else if (G_DATABASE.starDatabase[star_Index].type == "study" ){
                        if (G_DATABASE.starDatabase[star_Index].highlight){
                            g->draw_surface(star_study_hl, icon_shift(rec, screen, {x_pos, y_pos}));
                            print_star_name(star_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(star_study, icon_shift(rec, screen, {x_pos, y_pos}));
                        } 
                    }

                    else if (G_DATABASE.starDatabase[star_Index].type == "transit" ){
                        if (G_DATABASE.starDatabase[star_Index].highlight){
                            g->draw_surface(star_transit_hl, icon_shift(rec, screen, {x_pos, y_pos}));
                            print_star_name(star_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(star_transit, icon_shift(rec, screen, {x_pos, y_pos}));
                        }       
                    } 

                    else if (G_DATABASE.starDatabase[star_Index].type == "general" ){
                        if (G_DATABASE.starDatabase[star_Index].highlight){
                            g->draw_surface(star_general_hl, icon_shift(rec, screen, {x_pos, y_pos}));
                            print_star_name(star_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(star_general, icon_shift(rec, screen, {x_pos, y_pos}));
                        }       
                    }   

                    else if (G_DATABASE.starDatabase[star_Index].type == "home" ){ //can only created by right click
                        if (G_DATABASE.starDatabase[star_Index].highlight){
                            g->draw_surface(star_home_hl, icon_shift(rec, screen, {x_pos, y_pos}));
                            print_star_name(star_Index, g, x_pos, y_pos);
                        }else{
                            g->draw_surface(star_home, icon_shift(rec, screen, {x_pos, y_pos}));
                        }       
                    }   
                }
            }
       }
    
    //free normal icons
    ezgl::renderer::free_surface(star_food);
    ezgl::renderer::free_surface(star_work);
    ezgl::renderer::free_surface(star_study);
    ezgl::renderer::free_surface(star_transit);
    ezgl::renderer::free_surface(star_home);
    ezgl::renderer::free_surface(star_general);
    
    
    //free highlighted icons
    ezgl::renderer::free_surface(star_food_hl);
    ezgl::renderer::free_surface(star_work_hl);
    ezgl::renderer::free_surface(star_study_hl);
    ezgl::renderer::free_surface(star_transit_hl);
    ezgl::renderer::free_surface(star_home_hl);
    ezgl::renderer::free_surface(star_general_hl);

}

//Print the name of star besides the icon if a star is highlighted
void print_star_name(int star_Index, ezgl::renderer *g, double x_pos, double y_pos){
    
    g->set_color(ezgl::BLACK);
    g->set_text_rotation(TEXT_ROTATION_DEFAULT);
    g->set_font_size(10);
    g->draw_text({x_pos, y_pos}, G_DATABASE.starDatabase[star_Index].name);
    
}

//Draw open and closed features
void draw_features (ezgl::rectangle rec, ezgl::renderer *g, int level) {
    
    //traverse all features that need to be drawn at current zooming rate
    auto current_feature = G_DATABASE.featureDatabase.rbegin();
        
    //reversely traverse the map to draw the larger features first
    while (current_feature != G_DATABASE.featureDatabase.rend()){
        
        FeatureType current_feature_type =  getFeatureType(current_feature->second);
        //if zoom level  > 8, draw every building
        //always draw every green land, river, lake, etc.
        if (current_feature_type == Building && level <= 8){
            current_feature++;
            continue;
        }
        
        feature_color(g, current_feature_type); //determine filling color for this feature
        int count = getFeaturePointCount(current_feature->second);
        int inScreen = 0; //if any point of this feature is in screen
        
        if (current_feature->first > 0.0){ //closed feature, directly fill polygon
            
            std::vector <ezgl::point2d> points;
            for (int current_point =0; current_point < count; current_point++){
                
                //convert latlon to xy coordinates, then add point2d into the vector
                double current_x = x_from_lon(getFeaturePoint(current_point,current_feature->second).lon());
                double current_y = y_from_lat(getFeaturePoint(current_point,current_feature->second).lat());
            
                inScreen += withinScreen(rec, current_x, current_y);
            
                ezgl::point2d current_point2d(current_x, current_y);
                points.push_back(current_point2d);
                
            }
            if ((points.size() > 1 && inScreen > 0) || current_feature_type != Building) { 
                
                //only draw features that have more than 1 point in total and at least 1 point can be seen in screen
                g->fill_poly(points);
                
            }
            
        }
        else{ //not closed feature (open), connect two points with lines
            
            for (int current_point =0; current_point < count - 1; current_point++){
                
                //need two points to draw a line
                double current_x = x_from_lon(getFeaturePoint(current_point,current_feature->second).lon());
                double current_y = y_from_lat(getFeaturePoint(current_point,current_feature->second).lat());
                double next_x = x_from_lon(getFeaturePoint(current_point + 1,current_feature->second).lon());
                double next_y = y_from_lat(getFeaturePoint(current_point + 1,current_feature->second).lat());
                ezgl::point2d current_point2d(current_x, current_y);
                ezgl::point2d next_point2d(next_x, next_y);
                //always draw the line no matter if they can be seen in screen
                g->draw_line(current_point2d, next_point2d); 
                
            }
        }
        current_feature++;
    }
    
}

//Determine current drawing color for a feature polygon
void feature_color (ezgl::renderer *&g, FeatureType type){
    
    //all the following RGB values are accessed from Google's color selection tool
    //we'd like to take this opportunity to appreciate this tool!
    if (type == Park){
        g->set_color(ezgl::SELF_GREEN);
    }
    else if (type == Beach){
        g->set_color(ezgl::BEACH_YELLOW);
    }
    else if (type == Lake){
        g->set_color(ezgl::SELF_BLUE);
    }
    else if (type == River){
        g->set_color(ezgl::SELF_BLUE);
    }
    else if (type == Island){
        g->set_color(ezgl::BLANK_CANVAS);
    }
    else if (type == Building){
        g->set_color(ezgl::BUILDING);
    }
    else if (type == Greenspace){
        g->set_color(ezgl::SELF_GREEN);
    }
    else if (type == Golfcourse){
        g->set_color(ezgl::SELF_GREEN);
    }
    else if (type == Stream){
        g->set_color(ezgl::SELF_BLUE);
    }
    else if (type == Unknown){
        g->set_color(ezgl::BLACK);
    }
    
}

//Print feature name
void print_feature_name(ezgl::renderer *g, int level){

    //Print name of selected feature types
    for (int feature_index = 0; feature_index < getNumFeatures(); feature_index++){
        //Only print feature that is has a name and only print name after zoom level 9
        if (getFeatureName(feature_index) != "<noname>" && (level > 9 || (level >= 3 && find_feature_area(feature_index) >= LARGER_FEATURE))){
            //feature type is park
            if (getFeatureType(feature_index) == Park){
                int totalFeaturePoint = getFeaturePointCount(feature_index);
                print_feature_name_helper(g, feature_index, totalFeaturePoint);
                g->set_color(ezgl::DARK_GREEN);
            //feature type is beach
            }else if (getFeatureType(feature_index) == Beach){
                int totalFeaturePoint = getFeaturePointCount(feature_index);
                print_feature_name_helper(g, feature_index, totalFeaturePoint);
                g->set_color(ezgl::BLACK);
            //feature type is greenspace
            }else if (getFeatureType(feature_index) == Greenspace){
                int totalFeaturePoint = getFeaturePointCount(feature_index);
                print_feature_name_helper(g, feature_index, totalFeaturePoint);
                g->set_color(ezgl::DARK_GREEN);
            //feature type is golfcourse
            }else if (getFeatureType(feature_index) == Golfcourse){
                int totalFeaturePoint = getFeaturePointCount(feature_index);
                print_feature_name_helper(g, feature_index, totalFeaturePoint);
                g->set_color(ezgl::DARK_GREEN);
            }
        }
    }
    
    //print Lake Ontario
    g->set_color(ezgl::LAKE_ONTARIO);
    g->set_text_rotation(TEXT_ROTATION_DEFAULT);
    g->set_font_size(12);
    g->draw_text({LAKE_ONTARIO_X, LAKE_ONTARIO_Y}, "Lake Ontario", TEXT_BOUND_X_DEFAULT, TEXT_BOUND_Y_DEFAULT);
    
}

//Helper functions for print feature name
void print_feature_name_helper(ezgl::renderer *g, int feature_index, int totalFeaturePoint){
    
    double x_total = 0.0;
    double y_total = 0.0;
    double x_average = 0.0;
    double y_average = 0.0;
    double x_text_bound = 0.0;

    ezgl::rectangle rec = g->get_visible_world();  
    double screenX = rec.m_second.x - rec.m_first.x;
    double screenY = rec.m_second.y - rec.m_first.y;
    //defined length to seperate names in a distance
    double length = (sqrt((screenX*screenX) + (screenY * screenY)))/10.0;
    
    //calculate average x and y position
    for (int featurePoint = 0; featurePoint < getFeaturePointCount(feature_index); featurePoint++){
        x_total += x_from_lon(getFeaturePoint(featurePoint, feature_index).lon());
        y_total += y_from_lat(getFeaturePoint(featurePoint, feature_index).lat());
    }

    x_average = x_total / totalFeaturePoint;
    y_average = y_total / totalFeaturePoint;
    
    //set the bound for draw text
    x_text_bound = sqrt(pow(x_average, 2) + pow(y_average, 2));
    
    //draw feature name if it is within the screen
    if (withinScreen(rec, x_average, y_average)){
        //check if other names are too close to this position
        bool close = false;
        double distance;
        for (auto drawn = 0; drawn < DRAWNPOINTS.size(); drawn++) {
            double x_drawn = DRAWNPOINTS[drawn].x;
            double y_drawn = DRAWNPOINTS[drawn].y;
            double delta_x_square = (x_drawn - x_average) * (x_drawn - x_average);
            double delta_y_square = (y_drawn - y_average) * (y_drawn - y_average);
            distance = sqrt(delta_x_square + delta_y_square);
            if (distance < length) {
                close = true;
                break;
            }
        }
        //if too close, will not draw it
        if (close) return;
        g->set_text_rotation(TEXT_ROTATION_DEFAULT);
        g->set_font_size(10);
        g->draw_text({x_average, y_average}, getFeatureName(feature_index), x_text_bound, TEXT_BOUND_Y_DEFAULT);
        DRAWNPOINTS.push_back({x_average, y_average});
    }
      
}

//Draw OSM subways
void draw_subways(ezgl::renderer *g, int level) {
    
    if (DRIVE_MODE) return; //only draw subways in common mode and transit mode
    if (level < 3) return;  //only draw subways when level >= 3
    
    ezgl::rectangle rec = g->get_visible_world();
    
    //draw the subway lines (ways)
    for (auto segment = 0; segment < G_DATABASE.subwaylineinfo.size(); segment++) {
        
        const OSMWay* curr_way = & G_DATABASE.subwaylineinfo[segment].station_line;
        
        //store sub-segments' OSMID in a vector, then get the coordinates
        std::vector<OSMID> osm_way_members = getWayMembers(curr_way);
        
        for (auto member = 0; member < osm_way_members.size() - 1; member++) {
            
            //find two nodes, which are the endpoints of the current sub-segment member
            const OSMNode* node1 = G_DATABASE.nodeDatabase.find(osm_way_members[member])-> second;
            const OSMNode* node2 = G_DATABASE.nodeDatabase.find(osm_way_members[member + 1])->second;
            double x1 = x_from_lon(getNodeCoords(node1).lon());
            double y1 = y_from_lat(getNodeCoords(node1).lat());
            double x2 = x_from_lon(getNodeCoords(node2).lon());
            double y2 = y_from_lat(getNodeCoords(node2).lat());
            
            //set the subway color
            std::string curr_color = G_DATABASE.subwaylineinfo[segment].color;
            subway_color(g, curr_color);
            
            //draw the subway line
            if (crossScreen(rec, {x1, y1}, {x2, y2})) {
                g->set_line_width(2);
                g->draw_line({x1,y1}, {x2,y2});
            }
        } 
    }
    
    //draw the subway stations (nodes)
    for (auto station = 0; station < G_DATABASE.subwayinfo.size(); station ++) {
        //the xy coodinates of station nodes
        double xpos = x_from_lon(G_DATABASE.subwayinfo[station].station_node.coords().lon());
        double ypos = y_from_lat(G_DATABASE.subwayinfo[station].station_node.coords().lat());
        //set the radius of "circle" representing station
        double radius;
        if (level <= 3) radius = 0.0005;
        else if (level <= 5) radius = 0.0002;
        else if (level <= 8) radius = 0.00012;
        else if (level <= 10) radius = 0.00006;
        else if (level <= 12) radius = 0.00003;
        else radius = 0.000015;
        // be consistent with the subway line color
        std::string curr_color = G_DATABASE.subwayinfo[station].color;
        subway_color(g, curr_color); 
        g->fill_arc({xpos, ypos}, radius, 0, 360);
        //a white smaller circle inside
        g->set_color(255, 255, 255);
        g->fill_arc({xpos, ypos}, (radius/1.5), 0, 360);
        
        if (level >= 6){
            print_subway_station_name(station, g, xpos, ypos);
        }
    }
    
}

//Subway color for each subway lines
void subway_color (ezgl::renderer*& g, std::string curr_color){
    
    //when initialize (load toronto for the first time) this path would be empty
    //Exclusive premium color coding for our loving TORONTO!!! The same as official TTC colors.
    if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/toronto_canada.streets.bin" || G_DATABASE.current_map_path.size() == 0){
        if (curr_color == "yellow") g->set_color(ezgl::TTC_YELLOW);
        else if (curr_color == "green") g->set_color(ezgl::TTC_GREEN);
        else if (curr_color == "purple") g->set_color(ezgl::TTC_PURPLE);
    }
    else{ //well-structured hex color codes
        if (curr_color.size() > 0 && curr_color.at(0) == '#'){
            //they are hex numbers, need to have a prefix
            std::string red = "0x";
            std::string green = "0x";
            std::string blue = "0x";
            //parse the received string into sub-strings
            red.append(curr_color.substr (1,2));
            green.append(curr_color.substr (3,2));
            blue.append(curr_color.substr (5,2));
            //cast hexadecimal strings into uint_fast8_t
            uint_fast8_t red_RGB = std::stoul(red, NULL, 16);
            uint_fast8_t green_RGB = std::stoul(green, NULL, 16);
            uint_fast8_t blue_RGB = std::stoul(blue, NULL, 16);
            ezgl::color current_subway_color(red_RGB, green_RGB, blue_RGB);
            g->set_color(current_subway_color);
        }
        
        //unfortunately there is one subway line in Beijing that doesn't have proper hex codes, handle separately here
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/beijing_china.streets.bin" && curr_color == "007E84"){ 
            g->set_color(ezgl::BEIJING_DAXING);
        }
        
        //unfortunately some subway lines in Moscow don't have proper hex codes, handle separately here
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/moscow_russia.streets.bin"){ 
            if (curr_color == "orange") g->set_color(ezgl::ORANGE);
            else if (curr_color == "blue") g->set_color(ezgl::BLUE);
            else if (curr_color == "red") g->set_color(ezgl::RED);
            else if (curr_color == "darkgreen") g->set_color(ezgl::DARK_GREEN);
        }
        
        //unfortunately some subway lines in New Delhi don't have proper hex codes, handle separately here
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/new-delhi_india.streets.bin"){
            if (curr_color == "blue") g->set_color(ezgl::BLUE);
            else if (curr_color == "yellow") g->set_color(ezgl::YELLOW);
            else if (curr_color == "red") g->set_color(ezgl::RED);
            else if (curr_color == "orange") g->set_color(ezgl::ORANGE);
            else if (curr_color == "gray") g->set_color(ezgl::GREY_75);
            else if (curr_color == "pink") g->set_color(ezgl::PINK);
        }
        
        //unfortunately some subway lines in Tehran don't have proper hex codes, handle separately here
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/tehran_iran.streets.bin"){
            if (curr_color == "yellow") g->set_color(ezgl::YELLOW);
        }
        
        //unfortunately some subway lines in Tokyo don't have proper hex codes, handle separately here
        else if (G_DATABASE.current_map_path == "/cad2/ece297s/public/maps/tokyo_japan.streets.bin"){
            if (curr_color == "brown") g->set_color(ezgl::SADDLE_BROWN);
        }
        
        //subways that don't have a clear color assignment
        else {
            g->set_color(ezgl::RED);
        }
    }
    
}

//Print subway station name
void print_subway_station_name(int station_Index, ezgl::renderer *g, double x_pos, double y_pos){
    
    g->set_color(ezgl::GREY_75);
    g->set_text_rotation(TEXT_ROTATION_DEFAULT);
    g->set_font_size(14);
    
    //Transfer stations might be printed twice, thus check if the same station name has been already printed
    if (G_DATABASE.subwayStationPrintedDatabase.find(G_DATABASE.subwayinfo[station_Index].name) == G_DATABASE.subwayStationPrintedDatabase.end()){
        //if not, print this time, and add this station to the printed list
        g->draw_text({x_pos, y_pos}, G_DATABASE.subwayinfo[station_Index].name);
        G_DATABASE.subwayStationPrintedDatabase.insert(std::make_pair(G_DATABASE.subwayinfo[station_Index].name, station_Index));
    }
    
}

//Draw Bus from OSM
void draw_bus(ezgl::renderer *g, int level) {
    
    if (!TRANSIT_MODE) return;  //only transit mode
    if (level <= 6) return;  //only when level > 6
    
    ezgl::rectangle rec = g->get_visible_world();
    ezgl::rectangle screen = g->get_visible_screen();
    
    for (auto segment = 0; segment < G_DATABASE.buslineinfo.size(); segment++) {
        
        const OSMWay* curr_way = & G_DATABASE.buslineinfo[segment].station_line;
        std::vector<OSMID> osm_way_members = getWayMembers(curr_way);
        
        for (auto member = 0; member < osm_way_members.size() - 1; member ++) {
            //find two nodes, which are the endpoints of the current sub-segment member
            const OSMNode* node1 = G_DATABASE.nodeDatabase.find(osm_way_members[member])-> second;
            const OSMNode* node2 = G_DATABASE.nodeDatabase.find(osm_way_members[member + 1])->second;
            double x1 = x_from_lon(getNodeCoords(node1).lon());
            double y1 = y_from_lat(getNodeCoords(node1).lat());
            double x2 = x_from_lon(getNodeCoords(node2).lon());
            double y2 = y_from_lat(getNodeCoords(node2).lat());
            
            //draw bus lines
            g->set_color(ezgl::RED);
            if (crossScreen(rec, {x1, y1}, {x2, y2})) {
                g->set_line_width(0);
                g->draw_line({x1,y1}, {x2,y2});
            }
        } 
    }
    
    //icon for busstop
    ezgl::surface *busstop = ezgl::renderer::load_png("libstreetmap/resources/busstop.png");
    for (auto station = 0; station < G_DATABASE.businfo.size(); station++) {
        //the xy coodinates of station nodes
        double xpos = x_from_lon((G_DATABASE.businfo[station].station_node).coords().lon());
        double ypos = y_from_lat(G_DATABASE.businfo[station].station_node.coords().lat());
        
        //set the radius of "circle" representing station
        double radius;
        if (level <= 8) radius = 0.00012;
        else if (level <= 10) radius = 0.00006;
        else if (level <= 12) radius = 0.00003;
        else radius = 0.000015;
        //outer circle
        g->set_color(ezgl::RED);
        g->fill_arc({xpos, ypos}, radius, 0, 360);
        //inner white smaller circle
        g->set_color(255, 255, 255);
        g->fill_arc({xpos, ypos}, (radius/1.5), 0, 360);
        
        //draw icons of bus stops
        if (withinScreen(rec, xpos, ypos) && (level >= 9))  
            g->draw_surface(busstop, icon_shift(rec, screen, {xpos, ypos}));
    }
    
    ezgl::renderer::free_surface(busstop);
    
}

//Draw Bus from Libcurl
void draw_current_bus(ezgl::renderer *g, int level) {
    
    //first check map mode
    if (!TRANSIT_MODE) return;
    
    ezgl::rectangle rec = g->get_visible_world();   
    ezgl::rectangle screen = g->get_visible_screen();  
    ezgl::surface *bus = ezgl::renderer::load_png("libstreetmap/resources/bus.png");
    
    //current bus locations from libcurl, loop through buses
    for (auto curr_bus = 0; curr_bus < REALTIME_BUS.size(); curr_bus++) {
        //the curr_bus xy coordinates
        double x = x_from_lon(REALTIME_BUS[curr_bus].position.lon());
        double y = y_from_lat(REALTIME_BUS[curr_bus].position.lat());
        if (withinScreen(rec, x, y) && (level >= 9))  
            g->draw_surface(bus, icon_shift(rec, screen, {x, y}));
    }
    ezgl::renderer::free_surface(bus);
    
}

//Draw Road Closure from Libcurl
void draw_current_closure(ezgl::renderer *g, int level) {
    
    //first check map mode
    if (!DRIVE_MODE) return;
    
    ezgl::rectangle rec = g->get_visible_world();   
    ezgl::rectangle screen = g->get_visible_screen();  
    ezgl::surface *closure = ezgl::renderer::load_png("libstreetmap/resources/closedroad.png");

    //current road closure locations from libcurl, loop through closures
    for (auto curr_closure = 0; curr_closure < REALTIME_CLOSURE.size(); curr_closure++) {
        double x = x_from_lon(REALTIME_CLOSURE[curr_closure].position.lon());
        double y = y_from_lat(REALTIME_CLOSURE[curr_closure].position.lat());
        if (withinScreen(rec, x, y) && (level >= 6))  
            g->draw_surface(closure, icon_shift(rec, screen, {x, y}));
    }
    
    ezgl::renderer::free_surface(closure);
    
}

/*
 *  The following functions are some helper functions
 */ 

//check if the point is within the world of screen
bool withinScreen(ezgl::rectangle rec, double x, double y) {
    
    //range of current screen
    double xfirst = rec.m_first.x;
    double yfirst = rec.m_first.y;
    double xsecond = rec.m_second.x;
    double ysecond = rec.m_second.y;
    
    if (x < xfirst || y < yfirst || x > xsecond || y > ysecond) {
        return false;
    }
    return true;
    
}

//check if the segment is crossing the screen
bool crossScreen(ezgl::rectangle rec, XYcoord first, XYcoord second) {
    
    //y = ax + b
    if (first.x >= rec.m_first.x && first.x <= rec.m_second.x && 
            first.y >= rec.m_first.y && first.y <= rec.m_second.y) {
        return true;
    }
     if (second.x >= rec.m_first.x && second.x <= rec.m_second.x && 
            second.y >= rec.m_first.y && second.y <= rec.m_second.y) {
        return true;
    }
    
    //linear function
    double a = (second.y - first.y) / (second.x -first.x);
    double b = second.y - a * second.x;
    
    //location that cross the line of the screen boundary)
    double lefty, righty, topx, bottomx;
    lefty= a* rec.m_first.x + b;
    righty = a* rec.m_second.x + b;
    topx = (rec.m_second.y - b) / a;
    bottomx = (rec.m_first.y - b) / a;
    
    //make sure the location cross the segment of screen boundary
    if ( (rec.m_first.x >= first.x && rec.m_first.x <= second.x) ||
            (rec.m_first.x <= first.x && rec.m_first.x >= second.x) ) {
        if (lefty <= rec.m_second.y && lefty >= rec.m_first.y) {
            return true;
        }
    }
    
    if ( (rec.m_second.x >= first.x && rec.m_second.x <= second.x) ||
            (rec.m_second.x <= first.x && rec.m_second.x >= second.x) ) {
        if (righty <= rec.m_second.y && righty >= rec.m_first.y) {
            return true;
        }
    }
    
    if ( (rec.m_first.y >= first.y && rec.m_first.y <= second.y) ||
            (rec.m_first.y <= first.y && rec.m_first.y >= second.y) ) {
        if (topx <= rec.m_second.x && topx >= rec.m_first.x) {
            return true;
        }
    }
    
    if ( (rec.m_second.y >= first.y && rec.m_second.y <= second.y) ||
            (rec.m_second.y <= first.y && rec.m_second.y >= second.y) ) {
        if (bottomx <= rec.m_second.x && bottomx >= rec.m_first.x) {
            return true;
        }
    }
    
    return false;
    
}

//define zoom  levels of the screen
void zoom_levels(ezgl::renderer *g, int& level) {
    
    ezgl::rectangle rec = g->get_visible_world();
    
    double x_first = rec.m_first.x;
    double y_first = rec.m_first.y;
    double x_second = rec.m_second.x;
    double y_second = rec.m_second.y;
    
    double x_difference = x_second - x_first;
    double y_difference = y_second - y_first;
    double area = x_difference * y_difference;
    
    //use natural log of area to define zoom level
    level = (-1) * log(area);
}

//determine appropriate bounds for current map
void determine_bounds(){
    
    double max_lat = getIntersectionPosition(0).lat();
    double min_lat = max_lat;
    double max_lon = getIntersectionPosition(0).lon();
    double min_lon = max_lon;
    
    //traverse to update accurate max & min latlon values
    for (int id = 0; id < getNumIntersections(); id ++) {
        
        G_DATABASE.intersectionDatabase[id].position = getIntersectionPosition(id);
        G_DATABASE.intersectionDatabase[id].name = getIntersectionName(id);
        
        max_lat = std::max(max_lat, G_DATABASE.intersectionDatabase[id].position.lat() );
        min_lat = std::min(min_lat, G_DATABASE.intersectionDatabase[id].position.lat() );
        max_lon = std::max(max_lon, G_DATABASE.intersectionDatabase[id].position.lon() );
        min_lon = std::min(min_lon, G_DATABASE.intersectionDatabase[id].position.lon() );
    }
    
    G_DATABASE.max_lat = max_lat;
    G_DATABASE.min_lat = min_lat;
    G_DATABASE.max_lon = max_lon;
    G_DATABASE.min_lon = min_lon;
    
    G_DATABASE.avg_lat = (G_DATABASE.min_lat + G_DATABASE.max_lat) / 2;
    
    for (int id = 0; id < getNumIntersections(); id ++) {
        G_DATABASE.intersectionDatabase[id].x = x_from_lon(G_DATABASE.intersectionDatabase[id].position.lon() );
        G_DATABASE.intersectionDatabase[id].y = y_from_lat(G_DATABASE.intersectionDatabase[id].position.lat() );
    }
    
}


//Shift the png icons that we loaded
ezgl::point2d icon_shift(ezgl::rectangle rec, ezgl::rectangle screen, ezgl::point2d pos) {
    
    //shift the icon to to the correct location so that the "tail" of icon
    //is exactly at the location of POI
    double xpos = pos.x;
    double ypos = pos.y;
    
    double world_width = rec.m_second.x - rec.m_first.x;
    double screen_width = screen.m_second.x - screen.m_first.x;
    double ratio = world_width / screen_width;
    double shiftdistancex = ratio * ICON_WIDTH / 2;
    double shiftdistancey = ratio * ICON_HEIGHT;
    //return the calculated location
    return  {xpos - shiftdistancex, ypos + shiftdistancey};

}

//For the physical "find" button
std::vector<int> find_button_two_streets(std::string first_street_partial, std::string second_street_partial, ezgl::application *application){
    std::vector<int> first_street = find_street_ids_from_partial_street_name(first_street_partial);
    std::vector<int> second_street = find_street_ids_from_partial_street_name(second_street_partial);
    std::vector<int> result_intersection;
    
    bool terminate = false; //flag to indicate if continue
    
    if (first_street.size() != 1) { //not an acceptable input
        if (first_street.size() == 0) { //did not find any street
            //may need to extend this feature for error handling
            std::cout << "Sorry...No matching result for first street input:(" << std::endl;
            popup_error_window(application);
            terminate = true;
        }
        
        else if (first_street.size() > 1) { //found more than one street
            if (first_street_partial.length() < 5) {
                terminate = true;
                std::cout << "Sorry...First street input not specific enough, please type more characters:(" << std::endl;
                std::cout << "Suggestions: " << std::endl;
                for (auto current_street = first_street.begin(); current_street != first_street.end(); current_street++) {
                    std::cout << getStreetName(*current_street) << std::endl;
                }
            }
        }
        
    }
    
    if (second_street.size() != 1) { //not an acceptable input
        if (second_street.size() == 0) { //did not find any street
            //may need to extend this feature for error handling
            std::cout << "Sorry...No matching result for second street input:(" << std::endl;
            if (!terminate){
                popup_error_window(application);
            }
            terminate = true;
        } else if (second_street.size() > 1) { //found more than one street
            if (second_street_partial.length() < 5) {
                terminate = true;
                std::cout << "Sorry...Second street input not specific enough, please type more characters:(" << std::endl;
                std::cout << "Suggestions: " << std::endl;
                for (auto current_street = second_street.begin(); current_street != second_street.end(); current_street++) {
                    std::cout << getStreetName(*current_street) << std::endl;
                }
            }
        }
    }
    
    //not a successful find
    if (terminate){
        std::cout << "Find operation terminated..." << std::endl;
        return result_intersection;
    }
    
    else { //both inputs good, print detailed search result in command line
        for (auto current_first_street = first_street.begin(); current_first_street != first_street.end(); current_first_street++){
            for (auto current_second_street = second_street.begin(); current_second_street != second_street.end(); current_second_street++){
                std::cout << "Looking for intersections between ";
                std::cout << getStreetName(*current_first_street);
                std::cout << " and ";
                std::cout << getStreetName(*current_second_street) << std::endl;
                std::vector<int> current_result_intersection = find_intersections_of_two_streets(std::make_pair(*current_first_street, *current_second_street));
                result_intersection.insert(result_intersection.end(), current_result_intersection.begin(), current_result_intersection.end());
            }
        }
        if (result_intersection.size() == 0) { //no intersection found
            std::cout << "No intersection found"  << std::endl;
        }
        return result_intersection;
    }
    
}

//Find the closest starred location from the click position and calculate the distance
std::pair<int, double> find_closest_star_with_length(LatLon my_position){
    
    std::pair <int,double> star;
    star = std::make_pair(0, PLACEHOLDER); //in case there is no star yet, insert a placeholder to avoid seg fault, it 
     if (inRange(my_position.lat(), MINLAT, MAXLAT) && inRange(my_position.lon(), MINLON, MAXLON) && G_DATABASE.starDatabase.size() > 0){
         
        int star_id = 0;
    
        //calculate the distance between the click location and the first star
        LatLon first_star_location = LatLon(lat_from_y(G_DATABASE.starDatabase[0].y), lon_from_x(G_DATABASE.starDatabase[0].x));
        std::pair<LatLon, LatLon> star_distance_first (first_star_location,my_position);
        double length_first = find_distance_between_two_points(star_distance_first);

        //then find the closest intersection
        for (int i = 1; i < G_DATABASE.starDatabase.size(); i++){
            LatLon star_location = LatLon(lat_from_y(G_DATABASE.starDatabase[i].y), lon_from_x(G_DATABASE.starDatabase[i].x));
            std::pair<LatLon, LatLon> star_distance_loop (star_location,my_position);
            double length_loop = find_distance_between_two_points(star_distance_loop);
            //store the shorter star id
            if (length_loop < length_first){
                star_id = i;
                length_first = length_loop;
            }
        }

        //return the closest star's id
        star = std::make_pair(star_id, length_first);
        return star;
    }
    
     else{ //not in range
        std::cout << "Invalid Position" << std::endl;
        return star;
    }
    
}

/*
 *  GTK related functions
 */

/**
 * Function called before the activation of the application
 * Can be used to create additional buttons, initialize the status message,
 * or connect added widgets to their callback functions
 */
void initial_setup(ezgl::application *application, bool /*new_window*/)
{
    // Update the status bar message
    application->update_message("Welcome to Team19 Property Managing Map");

    /*
    *  Connect button to their callback functions
    */
    GObject *find_button_object = application->get_object("FindButton");
    g_signal_connect(find_button_object, "clicked", G_CALLBACK(find_button), application);
    
    GObject *refresh_button_object = application->get_object("Refresh");
    g_signal_connect(refresh_button_object, "clicked", G_CALLBACK(refresh_button), application);
    
    GObject *star_button_object = application->get_object("StarButton");
    g_signal_connect(star_button_object, "clicked", G_CALLBACK(star_button), application);
    
    GObject *UofT_button_object = application->get_object("UofTButton");
    g_signal_connect(UofT_button_object, "clicked", G_CALLBACK(UofT_button), application);

    GObject *about_button_object = application->get_object("About");
    g_signal_connect(about_button_object, "clicked", G_CALLBACK(about_button), application);
    
    GObject *home_object = application->get_object("Home");
    g_signal_connect(home_object, "clicked", G_CALLBACK(home_callback), application);

    GObject *switch_button = application->get_object("highlight_control");
    g_signal_connect(switch_button, "state-set", G_CALLBACK(switch_button_callback), application);
    
    /*
    *  Connect combobox to their callback functions
    */
    auto map_select_combobox = application->get_object("MapSelect");
    gtk_combo_box_set_active(GTK_COMBO_BOX(map_select_combobox), 0);
    g_signal_connect(map_select_combobox, "changed", G_CALLBACK(map_swicth_callback), application);
    
    auto mode_select_combobox = application->get_object("ModeSelect");
    gtk_combo_box_set_active(GTK_COMBO_BOX(mode_select_combobox), -1);
    g_signal_connect(mode_select_combobox, "changed", G_CALLBACK(mode_select_callback), application);
    
    auto saved_star_combobox = application->get_object("Saved");
    gtk_combo_box_set_active(GTK_COMBO_BOX(saved_star_combobox), -1);
    g_signal_connect(saved_star_combobox, "changed", G_CALLBACK(save_star_combobox), application);
   
    /*
    *  Connect text entry to their callback functions
    */
    GtkEntry *search_input_one = (GtkEntry*) application->get_object("SearchInputOne");
    GtkEntry *search_input_two = (GtkEntry*) application->get_object("SearchInputTwo");
    
    const gchar *placeholder_one = "Please type the name of the first street.";
    gtk_entry_set_placeholder_text (search_input_one, placeholder_one);
    
    g_signal_connect(search_input_one, "activate", G_CALLBACK(search_bar_callback), application);
    
    const gchar *placeholder_two =  "Please type the name of the second street.";
    gtk_entry_set_placeholder_text (search_input_two, placeholder_two);
    
    g_signal_connect(search_input_two, "activate", G_CALLBACK(search_bar_callback), application);
    
    street_name_completion(application);
    
}

/**
 * Function to handle keyboard press event
 * The name of the key pressed is returned (0-9, a-z, A-Z, Up, Down, Left, Right, Shift_R, Control_L, space, Tab, ...)
 * A pointer to the application and the entire GDK event are also returned
 */
void act_on_key_press(ezgl::application *application, GdkEventKey */*event*/, char *key_name)
{
    application->update_message("Key Pressed");
    street_name_completion(application);
    std::cout << key_name <<" key is pressed" << std::endl;
}

/**
 * Function to handle mouse press event
 * The current mouse position in the main canvas' world coordinate system is returned
 * A pointer to the application and the entire GDK event are also returned
 */
void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y)
{
    
    street_name_completion(application);
    LatLon mouse_location = LatLon(lat_from_y(y), lon_from_x(x));

    application->update_message("Mouse Clicked");


    if (event->button == 1) {
        
        //determine the shortest POI, intersection, and star. Only highlight the closest among these three
        std::pair<int, double> POI = find_closest_POI_with_length(mouse_location);
        std::pair<int, double> intersection = find_closest_intersection_with_length(mouse_location);
        std::pair<int, double> star = find_closest_star_with_length(mouse_location);
        
        if (POI.second < intersection.second && POI.second < star.second) { //POI is closer
            application->update_message(G_DATABASE.POI_Database[POI.first].name);

            if (G_DATABASE.POI_Database[POI.first].highlight) {
                G_DATABASE.POI_Database[POI.first].highlight = false;
            } else {
                G_DATABASE.POI_Database[POI.first].highlight = true;
                application->update_message(G_DATABASE.POI_Database[POI.first].name);
            }
        }
        
        else if (intersection.second < POI.second && intersection.second < star.second){ //intersection is closer
            G_DATABASE.intersectionDatabase[intersection.first].highlight = true;
            application->update_message(G_DATABASE.intersectionDatabase[intersection.first].name);
            popup_intersection_window(application, intersection.first);
        }
        
        else{ //star is closer
            application->update_message(G_DATABASE.starDatabase[star.first].name);
            
            if (G_DATABASE.starDatabase[star.first].highlight) {
                G_DATABASE.starDatabase[star.first].highlight = false;
            } else {
                G_DATABASE.starDatabase[star.first].highlight = true;
                application->update_message(G_DATABASE.starDatabase[star.first].name);
            }
        }

    } 
    
    else if (event->button == 2){
        
        //Unhighlight all highlighted object
        for (auto intersection_index = 0; 
            intersection_index < getNumIntersections();
            intersection_index ++) {
            G_DATABASE.intersectionDatabase[intersection_index].highlight = false;
        }
        for (auto POI_index = 0; 
            POI_index < getNumPointsOfInterest();
            POI_index ++) {
            G_DATABASE.POI_Database[POI_index].highlight = false;
        }
        for (auto star_index = 0; 
            star_index < G_DATABASE.starDatabase.size();
            star_index ++) {
            G_DATABASE.starDatabase[star_index].highlight = false;
        }
        
    }
    
    else if (event->button == 3){ 
        
        //add special type of star
        G_DATABASE.home_x = x;
        G_DATABASE.home_y = y;
        home_propmt_input(application);
        
    }

    //save this for future milestones if needed...
    if ((event->state & GDK_CONTROL_MASK) && (event->state & GDK_SHIFT_MASK))
        std::cout << "with control and shift pressed ";
    else if (event->state & GDK_CONTROL_MASK)
        std::cout << "with control pressed ";
    else if (event->state & GDK_SHIFT_MASK)
        std::cout << "with shift pressed ";

    std::cout << std::endl;
    G_DATABASE.mouseClick = std::make_pair(x, y); //update mouse location
    application->refresh_drawing();//force redraw
  
}

/**
 * A callback function to the find button
 */
void find_button(GtkWidget */*widget*/, ezgl::application *application)
{
    // Update the status bar message
    application->update_message("Initiating a find operation...Please use command line");
    std::string first_street;
    std::string second_street;

    //receive input from user
    std::cout<<"Please type the name of the first street..."<<std::endl;
    getline(std::cin,first_street);
    std::cout<<"Please type the name of the second street..."<<std::endl;
    getline(std::cin,second_street);

    //call helper function to determine the result of this find operation
    std::vector<int> result_intersection = find_button_two_streets(first_street, second_street, application);
    removeDuplicates(result_intersection);

    if (result_intersection.size() > 0) { //found at least one intersection
      auto current_intersection = result_intersection.begin();
      while (current_intersection != result_intersection.end()) {//print detailed result in command
          std::cout << "Intersection ID: "<<*current_intersection << " containing following streets: "<<std::endl;
          std::vector<std::string> street_name= find_street_names_of_intersection(*current_intersection);
          for (auto current_street_name = street_name.begin(); current_street_name!= street_name.end(); current_street_name++){
              std::cout << "      "<<*current_street_name<<std::endl;
          }
          G_DATABASE.intersectionDatabase[*current_intersection].highlight = true;
          current_intersection++;
      }
    }

    else { //find nothing
        return;
    }

    //if found any, auto-zoom to the first intersection that we found
    ezgl::canvas* cnv = application->get_canvas(application->get_main_canvas_id());
    int level;
    zoom_levels(application->get_renderer(), level);
    double xpos = G_DATABASE.intersectionDatabase[result_intersection[0]].x;
    double ypos = G_DATABASE.intersectionDatabase[result_intersection[0]].y;
    ezgl::zoom_to(cnv,{xpos, ypos}, level);

    // Redraw the main canvas
    application->refresh_drawing();
    
}


/**
 * A callback function to the star button
 */
void star_button(GtkWidget */*widget*/, ezgl::application *application)
{
  
    std::string type_input;
    std::string name_input;
    bool success = true; //if this add operation is successful

    //receive type input
    std::cout<<"Please select the type of this location (food/work/study/transit/general)..."<<std::endl;
    getline(std::cin,type_input);

    if (type_input != "food" && type_input != "work" && type_input != "study" && type_input != "transit" && type_input != "general"){ //invalid star type
        std::cout<<"Sorry...not a valid type"<<std::endl;
        std::cout<<"Did not add a star to this location"<<std::endl;
        application->update_message("Did not add a star to this location");
        success = false;
      }

    if (success) { //only when type input is valid
      std::cout << "Please enter the name of this location..." << std::endl;
      getline(std::cin,name_input);
      for (auto current_star = G_DATABASE.starDatabase.begin(); current_star != G_DATABASE.starDatabase.end(); current_star++) {
          if (success && current_star->name == name_input){ //if not success, no need to compare again
              success = false; //already has this name
          }
      }

      if (!success){ //invalid name
          std::cout << "Sorry...name already exists" << std::endl;
          std::cout<<"Did not add a star to this location"<<std::endl;
          application->update_message("Did not add a star to this location");
      }

    }

    if (success){ //only when type and name are valid
      star new_star;
      //initialize the star with known information
      new_star.type = type_input;
      new_star.name = name_input;
      new_star.x = G_DATABASE.mouseClick.first;
      new_star.y = G_DATABASE.mouseClick.second;
      G_DATABASE.starDatabase.push_back(new_star);

      //add the new star into drop menu for future quick access
      auto saved_star_combobox = application->get_object("Saved");
      const gchar* saved_name = name_input.c_str();
      gtk_combo_box_text_append_text((GtkComboBoxText*)saved_star_combobox, saved_name);
      //Update the status bar message
      std::cout<<"Added a star to this location"<<std::endl;
      application->update_message("Added a star to this location");
    }

    // Redraw the main canvas
    application->refresh_drawing();
  
}

//this was actually a test.. but we'd like to keep it as a bonus!
//you don't have to walk in heavy snow any more... just click this button :)
void UofT_button(GtkWidget */*widget*/, ezgl::application * application) {
    ezgl::canvas* cnv = application->get_canvas(application->get_main_canvas_id());
    int level;
    zoom_levels(application->get_renderer(), level);
    ezgl::zoom_to(cnv, {-57.4007,43.6611}, level); //uoft location
}

/**
 * A callback function to the about button
 */
void about_button(GtkWidget */*widget*/, ezgl::application * application) {
    
    GObject *window;
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *dialog;
    
    window = application ->get_object(application->get_main_window_id().c_str());
    dialog = gtk_dialog_new_with_buttons(
        "Instructions",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("OK"),
        GTK_RESPONSE_ACCEPT,
        ("CANCEL"),
        GTK_RESPONSE_REJECT,
        NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    label = gtk_label_new("\n  Here are some tips about this property managing map:  \n\n"
                           "  1. The two search bars on the top will allow you to search for an intersection between two street.  \n\n"
                           "  2. Our map provides you with three different modes.  \n\n"
            "    1) General Mode: No real time data displays.    \n"
            "    2) Transit Mode: if you are traveling through public transportation, this will be your choice.    \n"
            "    3) Drive Mode: if you are driving your favorite car, choose this one to start your trip.    \n\n"
            "  3. You can highlight any intersections or point of interests on our map using your cursor.    \n\n"
            "  4. The switch button allows you to unhighlight all highlighted objects in one instance.    \n\n"
            "  5. You can save any location you like by using the Add to My Favorite button.    \n\n"
            "  6. You can also save your home using the home button.    \n\n"
            "  7. You can access all saved starred location with the combobox at the bottom of the screen.    \n\n"
            "  8. Don't want to walk in heavy snow to campus? Now it's your chance! Try the UofT button!    \n\n"
            "  9. Want to see real time data? Click the refresh button to keep you updated with real time data.    \n\n"
            "  Want to travel to another city? Select your destination using combobox at the bottom right.    \n\n"
            "  Enjoy your trip :) ");
  
    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show_all(dialog);
    
    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL);

}

/**
 * A callback function to the refresh button (refresh real time data)
 */
void refresh_button(GtkWidget */*widget*/, ezgl::application *application){
    
    if (DRIVE_MODE) {
        refresh_libcurl_closure();
    }
    else if (TRANSIT_MODE) {
        refresh_libcurl_bus();
    }
    
    application->refresh_drawing();
    
}

/**
 * Pop up window for clicked intersection
 * Code accessed from lab handout
 */
void popup_intersection_window(ezgl::application *application, int current_intersection){
    
    GObject *window;
    GtkWidget *content_area;
    GtkWidget *label1;
    GtkWidget *label2;
    GtkWidget *dialog;
    
    window = application ->get_object(application->get_main_window_id().c_str());
    
    dialog = gtk_dialog_new_with_buttons(
        "Intersection Info",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("OK"),
        GTK_RESPONSE_ACCEPT,
        ("CANCEL"),
        GTK_RESPONSE_REJECT,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    std::cout << "Intersection ID: "<<current_intersection << " containing following streets: "<<std::endl;
    std::vector<std::string> street_name= find_street_names_of_intersection(current_intersection);
    for (auto current_street_name = street_name.begin(); current_street_name != street_name.end(); 
            current_street_name++ ) {
        std::cout << "      "<<*current_street_name<<std::endl;
    }
    
    const gchar *first_line = G_DATABASE.intersectionDatabase[current_intersection].name.c_str(); //convert to char* to fit popup windows
    label1 = gtk_label_new(first_line);
    label2 = gtk_label_new("Streets are printed in terminal window:)");
  
    gtk_container_add(GTK_CONTAINER(content_area), label1);
    gtk_container_add(GTK_CONTAINER(content_area), label2);
    gtk_widget_show_all(dialog);
    
    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL);
}

/**
 * Pop up window for when an invalid street name is given by user while finding intersection
 * Code accessed from lab handout
 */
void popup_error_window(ezgl::application *application){
    
    GObject *window;
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *dialog;
    
    //window layout
    window = application ->get_object(application->get_main_window_id().c_str());
    dialog = gtk_dialog_new_with_buttons(
        "Invalid street name",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("OK"),
        GTK_RESPONSE_ACCEPT,
        ("CANCEL"),
        GTK_RESPONSE_REJECT,
        NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    label = gtk_label_new("  Sorry... No matching result for street input :( \n See command line for detailed report...  ");
  
    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show_all(dialog);
    
    //link window to signal
    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL);
    
}

/**
 * Dialog response to receive user response
 * Code accessed from lab handout
 */
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer /*user_data*/){
    std::cout << "response is ";
    switch(response_id) {
        case GTK_RESPONSE_ACCEPT:
            std::cout << "GTK_RESPONSE_ACCEPT ";
            break;
        case GTK_RESPONSE_DELETE_EVENT:
            std::cout << "GTK_RESPONSE_DELETE_EVENT (i.e. ’X’ button) ";
            break;
        case GTK_RESPONSE_REJECT:
            std::cout << "GTK_RESPONSE_REJECT ";
            break;
        default:
            std::cout << "UNKNOWN ";
            break;
        
    }
    std::cout << "(" << response_id << ")\n";
    
    gtk_widget_destroy(GTK_WIDGET (dialog));
    
}

/**
 * Combobox callback for accessing clicked saved location
 */
gboolean save_star_combobox(GtkWidget *widget, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    gchar* id = gtk_combo_box_text_get_active_text((GtkComboBoxText*)widget);
    
    for (int star_id = 0; star_id < G_DATABASE.starDatabase.size(); star_id++){
        if (G_DATABASE.starDatabase[star_id].name == id){
            ezgl::canvas* cnv = application->get_canvas(application->get_main_canvas_id());
            int level;
            zoom_levels(application->get_renderer(), level);
            ezgl::zoom_to(cnv, {G_DATABASE.starDatabase[star_id].x,G_DATABASE.starDatabase[star_id].y}, level);
            return TRUE;
        }
    }
    
    return TRUE;

}

/**
 * Switch button callback for unhighlight all highlighted objects
 */
gboolean switch_button_callback(GtkWidget *, gboolean state, gpointer data)
{
    auto application = static_cast<ezgl::application *>(data);
    
    GtkSwitch* switch_highlight = (GtkSwitch*) application->get_object("highlight_control");
    
    //if switched
    if(state){
        for (auto intersection_index = 0;  
            intersection_index < getNumIntersections();
            intersection_index ++) {
            G_DATABASE.intersectionDatabase[intersection_index].highlight = false;
        }
        for (auto POI_index = 0; 
            POI_index < getNumPointsOfInterest();
            POI_index ++) {
            G_DATABASE.POI_Database[POI_index].highlight = false;
        }
        for (auto star_index = 0; 
            star_index < G_DATABASE.starDatabase.size();
            star_index ++) {
            G_DATABASE.starDatabase[star_index].highlight = false;
        }
        //restore the switch to OFF state
        gtk_switch_set_active (switch_highlight, false); 
        unhighlight_success(application);
    }
        
    application->refresh_drawing();
   
    return TRUE;
    
}

/**
 * Popup window informing user that unhighlight is done
 * Code accessed from Lab Handout
 */
void unhighlight_success(ezgl::application * /*application*/) {
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *dialog;
    
    dialog = gtk_dialog_new_with_buttons(
        "Unhighlight",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("OK"),
        GTK_RESPONSE_ACCEPT,
        NULL,
        GTK_RESPONSE_REJECT,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
       
    label = gtk_label_new("  Now all highlighted objects are unhighlighted.  ");
  
    gtk_container_add(GTK_CONTAINER(content_area), label);

    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL);
    
    gtk_widget_show_all(dialog);

}

/**
 * Search bar callback for finding the intersection of two streets
 */
gboolean search_bar_callback(GtkWidget */*widget*/, ezgl::application *application){

    GtkEntry *search_input_one = (GtkEntry*) application->get_object("SearchInputOne");
    GtkEntry *search_input_two = (GtkEntry*) application->get_object("SearchInputTwo");
        
    const char* first_street_input = gtk_entry_get_text(search_input_one);
    const char* second_street_input = gtk_entry_get_text(search_input_two);
    
    std::string first_street_name(first_street_input);
    std::string second_street_name(second_street_input);

    std::vector<int> result_intersection = find_button_two_streets(first_street_name, second_street_name, application);
    removeDuplicates(result_intersection);
    
    if (result_intersection.size() > 0) { //found at least one intersection
        
        auto current_intersection = result_intersection.begin();
        
        while (current_intersection != result_intersection.end()) {
        
            std::cout << "Intersection ID: "<<*current_intersection << " containing following streets: "<<std::endl;
            std::vector<std::string> street_name= find_street_names_of_intersection(*current_intersection);

            for (auto current_street_name = street_name.begin(); current_street_name!= street_name.end(); current_street_name++){
                std::cout << "      "<<*current_street_name<<std::endl;
            }

            G_DATABASE.intersectionDatabase[*current_intersection].highlight = true;
            current_intersection++;
        
        }
    
    }
    else{ 
        
        return TRUE;
        
    }
    
    //Zoom to the intersection of the two street
    ezgl::canvas* cnv = application->get_canvas(application->get_main_canvas_id());
    
    int level;
    zoom_levels(application->get_renderer(), level);
    double xpos = G_DATABASE.intersectionDatabase[result_intersection[0]].x;
    double ypos = G_DATABASE.intersectionDatabase[result_intersection[0]].y;
    ezgl::zoom_to(cnv, {xpos, ypos}, level);
    
    // Redraw the main canvas
    application->refresh_drawing();

    return TRUE;
    
}

/**
 * For dropdown menu for prompting possible street name when using the text entry
 * Learnt how to use GtkTreeIter, GtkListStore and GtkCompletion from GTK+ 3 Reference Manual
 */
void street_name_completion(ezgl::application *application){
    
    GtkTreeIter iter;
    GtkListStore *name_store = gtk_list_store_new(1, G_TYPE_STRING);

    for(auto current_street = G_DATABASE.street_names.begin(); current_street != G_DATABASE.street_names.end(); current_street++) {
        gtk_list_store_append(name_store, &iter);
        const gchar *current_street_name = current_street->first.c_str();
        gtk_list_store_set(name_store, &iter, 0, current_street_name, -1);
    }
    
    GtkEntry *search_input_one = (GtkEntry*) application->get_object("SearchInputOne");
 
    GtkEntry *search_input_two = (GtkEntry*) application->get_object("SearchInputTwo");
    
    GtkEntryCompletion *completion_one = gtk_entry_completion_new();
    gtk_entry_completion_set_model(completion_one, GTK_TREE_MODEL(name_store));
    gtk_entry_completion_set_text_column(completion_one, 0);

    GtkEntryCompletion *completion_two = gtk_entry_completion_new();
    gtk_entry_completion_set_model(completion_two, GTK_TREE_MODEL(name_store));
    gtk_entry_completion_set_text_column(completion_two, 0);
    
    gtk_entry_set_completion(GTK_ENTRY(search_input_one), completion_one);
    gtk_entry_set_completion(GTK_ENTRY(search_input_two), completion_two);

}

/**
 * Combobox for switching maps
 */
std::string previous_id = "Toronto";
gboolean map_swicth_callback(GtkWidget *widget, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    std::string combobox_id(gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget)));
    
    if (combobox_id == "-1"){
        return TRUE;
    }
    else{
        G_DATABASE.current_map_path = "/cad2/ece297s/public/maps/" + combobox_id + ".streets.bin";
    }
    
    std::cout << previous_id << std::endl;
    std::cout << combobox_id << std::endl;

    close_map();
    
    load_map(G_DATABASE.current_map_path);
    ezgl::canvas* cnv = application->get_canvas(application->get_main_canvas_id());

    ezgl::rectangle new_initial_world ({x_from_lon(G_DATABASE.min_lon), y_from_lat(G_DATABASE.min_lat)}, 
                                        {x_from_lon(G_DATABASE.max_lon),y_from_lat(G_DATABASE.max_lat)});
    
    std::cout << "new initial world " << x_from_lon(G_DATABASE.min_lon) << y_from_lat(G_DATABASE.min_lat) 
                           << x_from_lon(G_DATABASE.max_lon) << y_from_lat(G_DATABASE.max_lat) << std::endl;

    application->change_canvas_world_coordinates("MainCanvas", new_initial_world);
    
    cnv->redraw();
    application->refresh_drawing();
    previous_id = combobox_id; 
    
    return TRUE;
    
}

/**
 * Combobox for switching modes
 */
gboolean mode_select_callback(GtkWidget *widget, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    std::string combobox_id(gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget)));
    
    if (combobox_id == "-1"){
        return TRUE;
    }
    
    if (combobox_id == "DriveMode" ) {
        DRIVE_MODE = true;
        GENERAL_MODE = false;
        TRANSIT_MODE = false;
        refresh_libcurl_closure();
        application->refresh_drawing();
        std::cout << "drive mode" << std::endl;
    }
    else if (combobox_id == "TransitMode"){
        DRIVE_MODE = false;
        GENERAL_MODE = false;
        TRANSIT_MODE = true;
        refresh_libcurl_bus();
        application->refresh_drawing();
        std::cout << "transit mode" << std::endl;
    }
    else if (combobox_id == "GeneralMode"){
        DRIVE_MODE = false;
        GENERAL_MODE = true;
        TRANSIT_MODE = false;
        application->refresh_drawing();
        std::cout << "general mode" << std::endl;
    }
    
    return TRUE;
}

/**
 * Home button callback -> invoke popup window
 * Code accessed from lab handout
 */
void home_callback(GtkWidget */*widget*/, ezgl::application * /*application*/) {
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *dialog;
    
    dialog = gtk_dialog_new_with_buttons(
        "Add to My Home",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("OK"),
        GTK_RESPONSE_ACCEPT,
        ("CANCEL"),
        GTK_RESPONSE_REJECT,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
       
    label = gtk_label_new("  Please select the location you would like to save as your home  \n  on the map using the right click of your mouse.  ");
  
    gtk_container_add(GTK_CONTAINER(content_area), label);

    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL);
    
    gtk_widget_show_all(dialog);

}

/**
 * Invoke popup window when user right click a location on the map
 */
void home_propmt_input(ezgl::application *application){
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *dialog;
    
    dialog = gtk_dialog_new_with_buttons(
        "Add to My Home",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("CLOSE"),
        GTK_RESPONSE_ACCEPT,
        NULL,
        GTK_RESPONSE_REJECT,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
       
    label = gtk_label_new("\n  What would you like to store your home as  \n");
  
    gtk_container_add(GTK_CONTAINER(content_area), label);
        
    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL);
   
    GtkWidget *home_name_entry = gtk_entry_new(); 
    gtk_container_add(GTK_CONTAINER(content_area), home_name_entry); 	
    gtk_entry_set_max_length(GTK_ENTRY(home_name_entry), TEXT_LENGTH_MAX);  
    
    GtkWidget *hint_label = gtk_label_new("  Please press enter on your keyboard when you finish typing.  ");
    gtk_container_add(GTK_CONTAINER(content_area), hint_label);
    g_signal_connect(home_name_entry, "activate", G_CALLBACK(home_entry_callback), application);
    
    gtk_widget_show_all(dialog);
    
}

/**
 * Add home icon and store into star database
 */
void home_entry_callback(GtkWidget *widget, ezgl::application *application) {
    
    GtkEntry *home_name_input = (GtkEntry*) widget;
    const char* home_name_char = gtk_entry_get_text(home_name_input);
    std::string home_name(home_name_char);
    
    star new_star;
    new_star.type = "home";
    new_star.name = home_name;
    new_star.x = G_DATABASE.home_x;
    new_star.y = G_DATABASE.home_y;
    std::cout<<G_DATABASE.home_x<<std::endl<<G_DATABASE.home_y<<std::endl;
    G_DATABASE.starDatabase.push_back(new_star);
    auto saved_star_combobox = application->get_object("Saved");
    const gchar* saved_name = home_name.c_str();
    gtk_combo_box_text_append_text((GtkComboBoxText*)saved_star_combobox, saved_name);
    //Update the status bar message
    std::cout<<"Added as home to this location"<<std::endl;
    application->update_message("Added as home to this location");
    home_success_popup(application);
    std::cout << home_name <<std::endl;

}

/**
 * Pop up window for success
 */
void home_success_popup(ezgl::application *application) {
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *dialog;
    
    dialog = gtk_dialog_new_with_buttons(
        "Add to My Home",
        (GtkWindow*) window,
        GTK_DIALOG_MODAL,
        ("CLOSE"),
        GTK_RESPONSE_ACCEPT,
        NULL,
        GTK_RESPONSE_REJECT,
        NULL);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
       
    label = gtk_label_new("\n  You saved home location is now added on the map  \n"
                            "  you can now access it through the saved location combobox  \n");
  
    gtk_container_add(GTK_CONTAINER(content_area), label);

    g_signal_connect(
        GTK_DIALOG(dialog),
        "response",
        G_CALLBACK(on_dialog_response),
        NULL);
    
    gtk_widget_show_all(dialog);
    
    application->refresh_drawing();
    
}

//void draw_streets(ezgl::rectangle rec, ezgl::renderer *g, int level)
void draw_path_segments(ezgl::rectangle rec, ezgl::renderer* g, std::vector<StreetSegmentIndex> path_segments) {
    for (auto segment = 0; segment< path_segments.size(); segment++) {
        g->set_color(255, 0, 0, 100);
        g->set_line_width(3);
        InfoStreetSegment info = getInfoStreetSegment(path_segments[segment]);
        draw_segment_helper(rec, g, info, path_segments[segment]);
    }
}