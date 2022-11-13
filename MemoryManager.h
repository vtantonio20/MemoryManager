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
#include <tuple>
#include <stdlib.h>
#include "Allocators.h"
using namespace std;

class MemoryManager{
public:
    MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator);
    ~MemoryManager();
    void initialize(size_t sizeInWords);
    void shutdown();
    void *allocate(size_t sizeInBytes);
    void *allocateWithVector(size_t sizeInBytes);
    void free(void *address);
    void setAllocator(std::function<int(int, void *)> allocator);
    int dumpMemoryMap(char* filename);
    int dumpMemoryMapOFStream(char *filename);
    void *getList();
    void *getBitmap();
    unsigned getWordSize();
    void *getMemoryStart();
    vector<pair<void*, int>> calculateHoles();
    unsigned getMemoryLimit();
    int calculateOffset(void* address);
    void allocateAt(int index);
    void printEverything(bool printOnlyWords=true);
private:

    map<void*, pair<bool,int>> bitMap; //key is address, value is pair <bool: 1 is process, 0 is whole and int which is index>
    vector<pair<void*, int>> processList; //pair <<adress, length of process;
    vector<pair<void*, int>> holesList; //pair<adress, length of hole>


    char* memBlock;
    unsigned wordSize;
    size_t sizeInWords;
    std::function<int(int, void *)> allocator;

};