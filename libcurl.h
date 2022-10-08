/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   libcurl.h
 * Author: hanyi13
 *
 * Created on February 26, 2020, 2:03 AM
 */

#ifndef LIBCURL_H
#define LIBCURL_H


#include <string.h>
#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include "LatLon.h"

using boost::property_tree::ptree;
using boost::property_tree::read_json;

struct extBus {
    std::string route_name;
    LatLon position;
    int id;
};

struct extClosure {
    std::string id;
    std::string name;
    LatLon position;
    std::string description;
};

typedef struct MyCustomStruct {
    char *url = nullptr;
    unsigned int size = 0;
    char *response = nullptr;
} MyCustomStruct;

extern std::vector<extBus> REALTIME_BUS;
extern std::vector<extClosure> REALTIME_CLOSURE;

void bus_location_info();
void closure_info();
void LoadTTCVehicleInfo(ptree &ptRoot);
void LoadClosureInfo(ptree &ptRoot);
void refresh_libcurl_bus();
void refresh_libcurl_closure();

#endif /* LIBCURL_H */

