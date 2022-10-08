/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "libcurl.h"
#include "LatLon.h"
#include <iostream>
#include <string.h>
#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <string>

using namespace std;
using boost::property_tree::ptree;
using boost::property_tree::read_json;

/* buffer holds the data received from curl_easy_perform()
 * size is always 1
 * nmemb is the number of bytes in buffer
 * userp is a pointer to user data (i.e. myStruct from main)
 *
 * Should return same value as nmemb, else it will signal an error to libcurl
 * and curl_easy_perform() will return an error (CURLE_WRITE_ERROR). This is
 * useful if you want to signal an error has occured during processing.
 */
static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    if (buffer && nmemb && userp) {
        MyCustomStruct *pMyStruct = (MyCustomStruct *)userp;

        // Writes to struct passed in from main
        if (pMyStruct->response == nullptr) {
            // Case when first time write_data() is invoked
            pMyStruct->response = new char[nmemb + 1];
            strncpy(pMyStruct->response, (char *)buffer, nmemb);
        }
        else {
            // Case when second or subsequent time write_data() is invoked
            char *oldResp = pMyStruct->response;

            pMyStruct->response = new char[pMyStruct->size + nmemb + 1];

            // Copy old data
            strncpy(pMyStruct->response, oldResp, pMyStruct->size);

            // Append new data
            strncpy(pMyStruct->response + pMyStruct->size, (char *)buffer, nmemb);

            delete []oldResp;
        }

        pMyStruct->size += nmemb;
        pMyStruct->response[pMyStruct->size] = '\0';
    }
    size = size; //prevent unused variable
    return nmemb;
}

/* Boost uses the following JSON to property tree mapping rules:
 *   1) JSON objects are mapped to nodes. Each property is a child node.
 *   2) JSON arrays are mapped to nodes.
 *      Each element is a child node with an empty name. If a node has both
 *      named and unnamed child nodes, it cannot be mapped to a JSON representation.
 *   3) JSON values are mapped to nodes containing the value.
 *      However, all type information is lost; numbers, as well as the literals
 *      "null", "true" and "false" are simply mapped to their string form.
 *   4) Property tree nodes containing both child nodes and data cannot be mapped.
 */
void LoadTTCVehicleInfo(ptree &ptRoot) {
    string busName;
    int busID = 0;
    double longitude = 0, latitude = 0;

    BOOST_FOREACH(ptree::value_type &featVal, ptRoot.get_child("features")) {
        // "features" maps to a JSON array, so each child should have no name
        if ( !featVal.first.empty() )
            throw "\"features\" child node has a name";

        busName = featVal.second.get<string>("properties.route_name");
        busID = featVal.second.get<int>("properties.vehicle_id");

        // Get GPS coordinates (stored as JSON array of 2 values)
        // Sanity checks: Only 2 values
        ptree coordinates = featVal.second.get_child("geometry.coordinates");
        if (coordinates.size() != 2)
            throw "Coordinates node does not contain 2 items";
        
        longitude = coordinates.front().second.get_value<double>();
        latitude = coordinates.back().second.get_value<double>();
        
        extBus info;
        info.route_name = busName;
        info.id = busID;
        LatLon bus_position(latitude, longitude);
        info.position = bus_position;
        //load to global realtime closure data
        REALTIME_BUS.push_back(info);
    }

    return;
}

//load live location of TTC vehicles, only works for Toronto
void bus_location_info() {
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK) {
        cout << "ERROR: Unable to initialize libcurl" << endl;
        cout << curl_easy_strerror(res) << endl;
        return;
    }

    CURL *curlHandle = curl_easy_init();
    if ( !curlHandle ) {
        cout << "ERROR: Unable to get easy handle" << endl;
        return;
    } else {
        char errbuf[CURL_ERROR_SIZE] = {0};
        MyCustomStruct myStruct;
        char targetURL[] = "http://portal.cvst.ca/api/0.1/ttc/geojson";

        res = curl_easy_setopt(curlHandle, CURLOPT_URL, targetURL);
        if (res == CURLE_OK)
            res = curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, errbuf);
        if (res == CURLE_OK)
            res = curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, write_data);
        if (res == CURLE_OK)
            res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &myStruct);

        myStruct.url = targetURL;

        if (res != CURLE_OK) {
            cout << "ERROR: Unable to set libcurl option" << endl;
            cout << curl_easy_strerror(res) << endl;
        } else {
            res = curl_easy_perform(curlHandle);
        }

        cout << endl << endl;
        if (res == CURLE_OK) {
            // Create an empty proper tree
            ptree ptRoot;
            istringstream issJsonData(myStruct.response);
            read_json(issJsonData, ptRoot);

            try {
                LoadTTCVehicleInfo(ptRoot);
            } catch (const char *errMsg) {
                cout << "ERROR: Unable to fully parse the TTC JSON data" << endl;
                cout << "Thrown message: " << errMsg << endl;
            }

        } else {
            cout << "ERROR: res == " << res << endl;
            cout << errbuf << endl;
        }

        if (myStruct.response)
            delete []myStruct.response;

        curl_easy_cleanup(curlHandle);
        curlHandle = nullptr;
    }

    curl_global_cleanup();

}

void LoadClosureInfo(ptree &ptRoot) {
    string name, description, id;
    double longitude = 0, latitude = 0;
    BOOST_FOREACH(ptree::value_type &featVal, ptRoot.get_child("Closure")) {
        // "closure" maps to a JSON array, so each child should have no name
        if ( !featVal.first.empty() ) 
            throw "\"features\" child node has a name";
        longitude = featVal.second.get<double>("longitude");
        latitude = featVal.second.get<double>("latitude");
        LatLon position(latitude, longitude);
        //get desired information
        id = featVal.second.get<string> ("id");
	name = featVal.second.get<string>("name");
        description = featVal.second.get<string>("description");
        extClosure info;
        info.id = id;
        info.name = name;
        info.position = position;
        info.description = description;
        //load to global realtime closure data
        REALTIME_CLOSURE.push_back(info);
    }
}

//access live Toronto road closure data
void closure_info() {
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK) {
        return;
    }
    CURL *curlHandle = curl_easy_init();
    if ( !curlHandle ) {
        return;
    } else {
        char errbuf[CURL_ERROR_SIZE] = {0};
        MyCustomStruct myStruct;
        //char targetURL[] = "http://app.toronto.ca/opendata/cart/road_restrictions.json";
        char targetURL[] = "https://app.toronto.ca/opendata/cart/road_restrictions.json?v=1.0";

        res = curl_easy_setopt(curlHandle, CURLOPT_URL, targetURL);
        if (res == CURLE_OK)
            res = curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, errbuf);
        if (res == CURLE_OK)
            res = curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, write_data);
        if (res == CURLE_OK)
            res = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &myStruct);

        myStruct.url = targetURL;
        
        if (res != CURLE_OK) { } 
        else {
            res = curl_easy_perform(curlHandle);
        }
        
        if (res == CURLE_OK) {
            // Create an empty proper tree
            ptree ptRoot;
            istringstream issJsonData(myStruct.response);
            read_json(issJsonData, ptRoot);
            try {
                LoadClosureInfo(ptRoot);
            } catch (const char *errMsg) { }
        } 

        if (myStruct.response)
            delete []myStruct.response;

        curl_easy_cleanup(curlHandle);
        curlHandle = nullptr;
    }

    curl_global_cleanup();

    return;
}

//when user clicks on refresh button, update with server
void refresh_libcurl_bus() {
    //clear the existing global variable
    REALTIME_BUS.clear();
    
    //load again
    bus_location_info();

}

//when user clicks on refresh button, update with server
void refresh_libcurl_closure() {
    //clear the existing global variable
    REALTIME_CLOSURE.clear();
    
    //load again
    closure_info();
}