#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <queue>
#include <functional>
#include <map>
#include <vector>
#include <list>
#include <algorithm>    // std::min_element, std::max_element
#include "Allocators.h"
using namespace std;

int worstFit(int sizeInWords, void* list){

    uint16_t* holesList = static_cast<uint16_t*>(list);

    uint16_t numOfHoles = *holesList++;

    uint16_t maxHole= holesList[1];
    uint16_t offset = holesList[0];
    for(int i=1; i < numOfHoles*2; i+=2){
        if(maxHole < sizeInWords){
            if(holesList[i] >= sizeInWords){
                maxHole = holesList[i];
                offset = holesList[i-1];
            }
        }else if(holesList[i] >= sizeInWords && holesList[i] > maxHole){
            maxHole = holesList[i];
            offset = holesList[i-1];
        }
    }
    return maxHole >= sizeInWords ? offset : -1;
}

int bestFit(int sizeInWords, void* list){

    uint16_t* holesList = static_cast<uint16_t*>(list);

    uint16_t numOfHoles = *holesList++;

    uint16_t minHole= holesList[1];
    uint16_t offset = holesList[0];
    for(int i=1; i < numOfHoles*2; i+=2){
        if(minHole < sizeInWords){
            if(holesList[i] >= sizeInWords){
                minHole = holesList[i];
                offset = holesList[i-1];
            }
        }else if(holesList[i] >= sizeInWords && holesList[i] < minHole){
            minHole = holesList[i];
            offset = holesList[i-1];
        }
    }
    return minHole >= sizeInWords ? offset : -1;
}
pair<void*, int> worstFitHelperVector(vector<pair<void*, int>>holesList, int sizeOfBlk) {
    if(holesList.empty()) return make_pair(nullptr, 0);
    pair<void*,int> maxHole = holesList.front();
    for(auto &it: holesList){
        if(it.second >= sizeOfBlk && it.second > maxHole.second){
            maxHole = it;
        }
    }
    if(maxHole.second < sizeOfBlk) return make_pair(nullptr, -1);
    else return maxHole;
}
pair<void*, int> bestFitHelperVector(vector<pair<void*, int>>holesList, int sizeOfBlk){
    if(holesList.empty()) return make_pair(nullptr, 0);
    pair<void*, int> minHole = holesList.front();
    for(auto &it: holesList){
        //NEED THE MINIMUM HOLE LARGER THAN N
        if(it.second >= sizeOfBlk && it.second < minHole.second){
            minHole = it;
        }
    }
    if(minHole.second < sizeOfBlk) return make_pair(nullptr, -1);
    else return minHole;
}
int bestFitVector(int sizeInWords, void *list){
    pair<map<void*, pair<bool,int>>,vector<pair<void*, int>>> lmao = *(pair<map<void*, pair<bool,int>>,vector<pair<void*, int>>> *) list;
    map<void*, pair<bool,int>> bitMap = lmao.first;
    vector<pair<void*, int>> holesList = lmao.second;
    pair<void*,int> minHole = bestFitHelperVector(holesList, sizeInWords);
    if(minHole.second == 0) return 0;
    else if (minHole.second == -1) return -1;
    else return bitMap.at(minHole.first).second;
}

int worstFitVector(int sizeInWords, void* list){
    pair<map<void*, pair<bool,int>>,vector<pair<void*, int>>> lmao = *(pair<map<void*, pair<bool,int>>,vector<pair<void*, int>>> *) list;
    map<void*, pair<bool,int>> bitMap = lmao.first;
    vector<pair<void*, int>> holesList = lmao.second;
    pair<void*,int> maxHole = worstFitHelperVector(holesList, sizeInWords);
    if(maxHole.second == 0) return 0;
    else if (maxHole.second == -1) return -1;
    else return bitMap.at(maxHole.first).second;
}

