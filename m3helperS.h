/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m3helperS.h
 * Author: wushiyin
 *
 * Created on March 11, 2020, 11:43 PM
 */

#ifndef M3HELPERS_H
#define M3HELPERS_H

class frontier{
public:
    IntersectionIndex nodeID;
    
    frontier(){
        nodeID = -1;
    }
    
    frontier(IntersectionIndex id){
        nodeID = id;
    }
};

struct node{
    StreetSegmentIndex segment_ID;
    IntersectionIndex parent_ID;
    double travel_time;
};

std::vector<node> nodeList;

class comparator{
public:
    bool operator()(const frontier first, const frontier second){
        return nodeList[first.nodeID].travel_time > nodeList[second.nodeID].travel_time;
    }
};

double A_start_weight(IntersectionIndex current, IntersectionIndex destination);
//double average_speed_limit(IntersectionIndex current, IntersectionIndex destination);
#endif /* M3HELPERS_H */

