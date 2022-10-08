/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m2draw.h
 * Author: hanyi13
 *
 * Created on February 14, 2020, 4:17 PM
 */

#ifndef M2DRAW_H
#define M2DRAW_H

#include <chrono>
#include <thread>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "m1helper.h"
#include "m1.h"
#include <string>

constexpr double RADIAN_TO_DEGREE = 1 / DEGREE_TO_RADIAN;
int constexpr TEXT_ROTATION_DEFAULT = 0;
constexpr double INTERSECTION_RADIUS = 0.00003;
double constexpr LAKE_ONTARIO_X = -57.2449;
double constexpr LAKE_ONTARIO_Y = 43.5774;
int constexpr TEXT_BOUND_X_DEFAULT = 100;
int constexpr TEXT_BOUND_Y_DEFAULT = 100;
int constexpr TEXT_BOUND_Y_SMALL_LEVEL = 50;
constexpr double PLACEHOLDER = 999999; //a trivial large number
int constexpr TEXT_LENGTH_MAX = 100;
double constexpr LARGER_FEATURE = 300000;
 
//for city names display
double constexpr ICELAND_X = -7.5;
double constexpr ICELAND_Y = 64.8;
double constexpr SHENZHEN_X = 105.567;
double constexpr SHENZHEN_Y = 22.678;
double constexpr TOKYO_X = 113.4;
double constexpr TOKYO_Y = 35.8;
double constexpr RIO_X = -39.85;
double constexpr RIO_Y = -22.9;
double constexpr TORONTO_X = -57.48;
double constexpr TORONTO_Y = 43.72;
double constexpr BEIJING_X = 89.27;
double constexpr BEIJING_Y = 39.9;
double constexpr CAIRO_X = 27.09;
double constexpr CAIRO_Y = 30.04;
double constexpr HAMILTON_X = -58.16;
double constexpr HAMILTON_Y = 43.21;
double constexpr SYDNEY_X = 125.5;
double constexpr SYDNEY_Y = -33.9;
double constexpr SAINT_HELENA_X = -5.48;
double constexpr SAINT_HELENA_Y = -15.97;
double constexpr MACAU_X = 104.7;
double constexpr MACAU_Y = 22.2;
double constexpr HONGKONG_X = 105.6;
double constexpr HONGKONG_Y = 22.4;
double constexpr GUANGZHOU_X = 104.7;
double constexpr GUANGZHOU_Y = 23.0;
double constexpr LONDON_X = 0;
double constexpr LONDON_Y = 51.5;
double constexpr MOSCOW_X = 21.2;
double constexpr MOSCOW_Y = 55.75;
double constexpr NEW_DELHI_X = 67.85;
double constexpr NEW_DELHI_Y = 28.6;
double constexpr NEW_YORK_X = -56;
double constexpr NEW_YORK_Y = 40.75;
double constexpr SINGAPORE_X = 103.75;
double constexpr SINGAPORE_Y = 1.35;

//Width and Height of picture of our icons
double constexpr ICON_WIDTH = 32;
double constexpr ICON_HEIGHT = 37;


//draw and print function
void draw_map();
void print_map_name(ezgl::renderer *g, int level);
void draw_main_canvas(ezgl::renderer *g);
void draw_main_canvas_blank(ezgl::renderer *g, ezgl::rectangle rec); 
void draw_streets(ezgl::rectangle rec, ezgl::renderer *g, int level);
void draw_segment_helper(ezgl::rectangle rec, ezgl::renderer *g, InfoStreetSegment& info, int seg); 
void draw_segment_name(ezgl::rectangle rec, ezgl::renderer *g, InfoStreetSegment& info, int seg, int level);
void draw_segment_name_helper(ezgl::renderer *g, InfoStreetSegment& info, double del_x, double del_y, 
                                    double x_mid, double y_mid, double text_degree, double text_x_bound, double text_y_bound, int level);
void draw_major_street_names(ezgl::rectangle rec, ezgl::renderer *g, int level);

void draw_POI(ezgl::renderer *g, int level);
void print_POI_name(int POI_Index, ezgl::renderer *g, double x_pos, double y_pos);
void highlight_intersections(ezgl::rectangle rec, ezgl::renderer *g);
void draw_star(ezgl::renderer *g, int level);
void print_star_name(int POI_Index, ezgl::renderer *g, double x_pos, double y_pos);
void draw_features (ezgl::rectangle rec, ezgl::renderer *g, int level);
void feature_color (ezgl::renderer *&g, FeatureType type);
void print_feature_name(ezgl::renderer *g, int level);
void print_feature_name_helper(ezgl::renderer *g, int feature_index, int totalFeaturePoint);


void draw_subways(ezgl::renderer *g, int level); 
void subway_color (ezgl::renderer*& g, std::string curr_color);
void print_subway_station_name(int station_Index, ezgl::renderer *g, double x_pos, double y_pos);
void draw_bus(ezgl::renderer *g, int level); 
void draw_current_bus(ezgl::renderer *g, int level); 
void draw_current_closure(ezgl::renderer *g, int level);  

//helper functions
bool withinScreen(ezgl::rectangle rec, double x, double y); 
bool crossScreen(ezgl::rectangle rec, XYcoord first, XYcoord second); 
void zoom_levels(ezgl::renderer *g, int& level); 
ezgl::point2d icon_shift(ezgl::rectangle rec, ezgl::rectangle screen, ezgl::point2d pos); 
bool rightOneWay(double del_x, double del_y);
void determine_bounds();

std::vector<int> find_button_two_streets(std::string, std::string, ezgl::application *application);
std::pair<int, double> find_closest_star_with_length(LatLon my_position);
struct latlonBounds {
    double max_lat;
    double min_lat;
    double max_lon;
    double min_lon;
};

//GTK related
void initial_setup(ezgl::application *application, bool new_window);

// Callback functions for event handling
void act_on_mouse_press(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name);

//buttons
void find_button(GtkWidget *widget, ezgl::application *application);
void star_button(GtkWidget *widget, ezgl::application *application);
void UofT_button(GtkWidget */*widget*/, ezgl::application* application);
void about_button(GtkWidget *widget, ezgl::application * application);
void refresh_button(GtkWidget */*widget*/, ezgl::application *application);
void home_callback(GtkWidget */*widget*/, ezgl::application * /*application*/);

//windows
void home_propmt_input(ezgl::application * /*application*/);
void home_success_popup(ezgl::application * /*application*/);
void popup_intersection_window(ezgl::application *application, int current_intersection);
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void popup_error_window(ezgl::application *application);
void unhighlight_success(ezgl::application * /*application*/);

//combobox
gboolean save_star_combobox(GtkWidget *widget, gpointer data);

//other callback
gboolean switch_button_callback(GtkWidget *, gboolean state, gpointer data);
gboolean search_bar_callback(GtkWidget *widget, ezgl::application *application);
gboolean map_swicth_callback(GtkWidget *widget, gpointer data);
gboolean mode_select_callback(GtkWidget *widget, gpointer data);
void street_name_completion(ezgl::application *application);
void home_entry_callback(GtkWidget *widget, ezgl::application * /*application*/);


void draw_path_segments(ezgl::rectangle rec, ezgl::renderer* g, std::vector<StreetSegmentIndex> path_segments);
#endif /* M2DRAW_H */

