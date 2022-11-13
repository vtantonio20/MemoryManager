#include <cmath>
#include "MemoryManager.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>

//constructor
MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator){
    this->wordSize= wordSize;
    this->allocator= allocator;
}

//destructor
MemoryManager::~MemoryManager(){
    if(!bitMap.empty()){
        shutdown();
    }
}
void MemoryManager::shutdown() {
    delete[] memBlock;
    bitMap.clear();
    holesList.clear();
    processList.clear();
}
void MemoryManager::initialize(size_t sizeInWords){
    //cout << sizeInWords <<endl;
    this->sizeInWords = sizeInWords;
    if(sizeInWords < 65536){
        //instantiate block of requestedSize -> (sizeInWords *wordSize) = Max Memory Block Size
        // memBlock = (void*)malloc(sizeInWords*wordSize *sizeof(char));
        //cout << sizeof(uint8_t)<<endl;
        memBlock = new char[sizeInWords*wordSize];
        for(int i=0; i < sizeInWords*wordSize; i++){
            bitMap.insert({&memBlock[0]+i, make_pair(false, i)});
        }
    }
}


void* MemoryManager::allocate(size_t sizeInWords){
    //cout << "\tAllocating " << sizeInWords << " words" <<endl;
    if(holesList.empty() && sizeInWords > getMemoryLimit()) return nullptr;
    if(holesList.empty() && !processList.empty()) return nullptr;

    uint16_t* list = static_cast<uint16_t*>(getList());

    int offset = !processList.empty() ? allocator((uint16_t)(sizeInWords/getWordSize()), list) : 0;
    if(offset != -1) {
        offset*=(int)getWordSize();
        int i=0;
        auto it = bitMap.begin();
        for(it = bitMap.begin(); it != bitMap.end(); ++it){
            //if between offset and offset+size
            if(i >= offset+sizeInWords) break;
            if(i >= offset) it->second.first =1;
            if(i == offset) processList.emplace_back(it->first, sizeInWords);
            i++;
        }
        holesList = calculateHoles();
        delete [] list;

        return (void*)processList.back().first;
    }
    cout << "DOES NOT FIT IN BITMAP. FREE SOME DATA. BITMAP REMAINS UNCHANGED" <<endl;
    delete [] list;
    return nullptr;
}

void MemoryManager::free(void *address){
    int length;
    for(auto it = processList.begin(); it != processList.end(); it++){
        if(it->first == address) {
            length = it->second;
            processList.erase(it);
            break;
        }
    }
    for(auto it = bitMap.begin(); it != bitMap.end(); it++) {
        if (it->first == address) {
            while (length != 0) {
                it->second.first = 0;
                it++;
                length--;
            }
            return;
        }
    }
    holesList = calculateHoles();
}
int MemoryManager::calculateOffset(void* address){
    return bitMap.at(address).second;
}
void* MemoryManager::getList(){
    holesList=calculateHoles();
//    if(holesList.empty() && !processList.empty()) return 0;
    if(!holesList.empty()){
        holesList = calculateHoles();
        uint16_t size= 1 + holesList.size()*2;
        uint16_t * list = new uint16_t[size];                              //******************************************************ASK ABOUT THIS
        list[0]= holesList.size();
        int j=0;
        for(uint16_t i=1; i < size; i++){
            if(i%2 == 1){
                //adding a offset
                //offset can be found in bitMap value.second
                //pass in the address for the given hole
                list[i] = calculateOffset(holesList[j].first)/getWordSize();
            }else{
                //adding holeSize
                //wholeSize can be found in holeList second val
                list[i] = holesList[j].second/getWordSize();
                j++;
            }
        }
        return list;
    }
    return nullptr;
}
void MemoryManager::setAllocator(std::function<int(int, void *)> allocator){
    holesList = calculateHoles();
    this->allocator = allocator;
}
int MemoryManager::dumpMemoryMap(char* filename){
    uint16_t* list = static_cast<uint16_t*>(getList());
    uint16_t* listEntryPoint = list;

    uint16_t listLength = *list++;
    uint16_t bytesPerEntry = 2;
    uint16_t entriesInList = listLength * bytesPerEntry;


    string s;
    for(uint16_t i = 0; i < entriesInList; ++i) {
        if(i%2==0) s += "[" + to_string(list[i]) + ", ";
        else if(i != entriesInList-1) s += to_string(list[i]) + "] - ";
        else s += to_string(list[i]) + "]";
    }

    delete [] listEntryPoint;

    int file_descriptor = open(filename, O_WRONLY | O_CREAT | S_IRWXU | S_IWUSR);

    if(file_descriptor == -1) {
        // print which type of error have in a code
        printf("Error Number % d\n", errno);
        // print program detail "Success or failure"
        perror("Program");
        return -1;
    }else{
        printf("Success in opening fd\n");

    }
    if(write(file_descriptor, &s[0], s.size()) == -1) {
        printf("Error in writing to fd\n");
        return -1;
    }
    if(close(file_descriptor) ==-1) {
        printf("Error in closing fd\n");

        return -1;
    }
    return 0;
}
int MemoryManager::dumpMemoryMapOFStream(char *filename) {
    string file = string(filename);
    std::ofstream outfile (file);

    uint16_t* list = static_cast<uint16_t*>(getList());
    uint16_t* listEntryPoint = list;

    uint16_t listLength = *list++;
    //cout << listLength <<endl;
    uint16_t bytesPerEntry = 2;
    uint16_t entriesInList = listLength * bytesPerEntry;


    string s;
    for(uint16_t i = 0; i < entriesInList; ++i) {
        if(i%2==0) s += "[" + to_string(list[i]) + ", ";
        else if(i != entriesInList-1) s += to_string(list[i]) + "] - ";
        else s += to_string(list[i]) + "]";
    }
    //cout << s <<endl;



    outfile << s << std::endl;

    outfile.close();
    delete [] listEntryPoint;

    return 1;
}



void* MemoryManager::getBitmap() {


    int i=0;
    vector<uint16_t> bitMapWW;
    bitMapWW.push_back(bitMap.begin()->second.first);
    //cout <<  bitMap.begin()->second.first << " ";
    for (auto b = bitMap.begin(); b != bitMap.end(); b++, i++){
        if(i==wordSize) {
            bitMapWW.push_back(b->second.first);
            i = 0;
        }
    }

    uint16_t size = ceil((double)bitMapWW.size()/8);
    uint8_t s1 = static_cast<uint8_t>(size & 0x00FF);
    uint8_t s2 = static_cast<uint8_t>((size & 0xFF00) >> 8);

    uint8_t * bm = new uint8_t[size+2];
    bm[0] = (uint16_t)s1;
    bm[1] = (uint16_t)s2;

    //cout << size<< endl;
    vector<string> binaryBytes;
    for(int j = 0; j < bitMapWW.size(); j++){
        if(j%8==0 && j!=0){
            string s = to_string(bitMapWW[j-1]) + to_string(bitMapWW[j-2]) + to_string(bitMapWW[j-3]) + to_string(bitMapWW[j-4]) + to_string(bitMapWW[j-5]) +to_string(bitMapWW[j-6]) + to_string(bitMapWW[j-7]) + to_string(bitMapWW[j-8]);
            binaryBytes.push_back(s);
            //cout << s << "\t";
            //cout << bitMapWW[j-8] << " " <<bitMapWW[j-7] << " " << bitMapWW[j-6] << " "
            //      << bitMapWW[j-5] << " " <<bitMapWW[j-4] << " " << bitMapWW[j-3] << " "
            //    << bitMapWW[j-2] << " " <<bitMapWW[j-1] << " \t";
            //cout << j <<endl;
            if(j+8 > bitMapWW.size()){
                int i=0;
                string s2;
                while(j< bitMapWW.size()){
                    s2.insert(0,to_string(bitMapWW[j]));
                    //cout << bitMapWW[j] << " ";
                    i++; j++;
                }
                while(i < 8){
                    s2.insert(0,to_string(0));
                    //cout << 0 <<" ";
                    i++;
                }
                binaryBytes.push_back(s2);
                //cout<< s2<<endl;
                break;
            }
        }
    }
    if(binaryBytes.size()== size-1){
        binaryBytes.emplace_back("00000000");
    }
    int n=0;
    for(int i=0; i < size; i++){
        bm[i+2] =strtol(binaryBytes[n++].c_str(), nullptr, 2);;
    }
    return bm;
}


vector<pair<void*,int>> MemoryManager::calculateHoles(){
    auto it = bitMap.begin();
    vector<pair<void*,int>> holes;
    bool inWhole = !it->second.first;
    int count=0;
    for(it; it != bitMap.end(); it++, count++){
        //edge case for beginning
        if(inWhole && it == bitMap.begin()){
            holes.emplace_back(it->first, count);
        }//edge case for ending
        else if(inWhole && next(it) == bitMap.end() && it->second.first == 0){
            holes.back().second = count +1;
        }
        else if(!inWhole){
            if(it->second.first == 0){
                holes.emplace_back(it->first, count);
                inWhole= true;
                count=0;
            }
        } else {
            if(it->second.first == 1){
                inWhole=false;
                holes.back().second = count;
                count=0;
            }
        }
    }
    return holes;
}
void MemoryManager::printEverything(bool printOnlyWords){
    holesList = calculateHoles();
    cout << "BitMap:"<<endl;
    int i=0;
    for(auto &it : bitMap){
        if(i%getWordSize() ==0 && printOnlyWords){
            cout << "\t\tAddress: " << it.first << "\tType: " << it.second.first <<endl;
        }else if(!printOnlyWords){
            cout << "\t\tAddress: " << it.first << "\tType: " << it.second.first << "\t" << i <<endl;
        }
        i++;
    }
}
unsigned MemoryManager::getWordSize(){
    return wordSize;
}
void* MemoryManager::getMemoryStart(){
    return  &memBlock[0];
}
unsigned MemoryManager::getMemoryLimit(){
    return wordSize*sizeInWords;
}
void MemoryManager::allocateAt(int index){
    cout << "\tAllocating at " << index <<endl;
    for(int i=0; i < wordSize; i++){
        bitMap.at(&memBlock + sizeof(char)*index + sizeof(char)*i).first = true;
    }
    processList.emplace_back(make_pair(&memBlock + sizeof(char)*index, wordSize));
    holesList= calculateHoles();
}

void* MemoryManager::allocateWithVector(size_t sizeInWords){
    //cout << "\tAllocating " << sizeInWords << " words" <<endl;
    if(holesList.empty() && sizeInWords > getMemoryLimit()) return nullptr;
    pair<map<void*, pair<bool,int>>,vector<pair<void*, int>>> data = make_pair(bitMap, holesList);



    //int offset = allocator((int)sizeInWords, getList());
    int offset = allocator((uint16_t)sizeInWords, &data);  //HAVE TO USE GETLIST

    if(offset != -1) {
        int i=0;
        auto it = bitMap.begin();
        for(it = bitMap.begin(); it != bitMap.end(); ++it){
            //if between offset and offset+size
            if(i >= offset+sizeInWords) break;
            if(i >= offset) it->second.first =1;
            if(i == offset) processList.emplace_back(it->first, sizeInWords);
            i++;
        }
        holesList = calculateHoles();
        return processList.back().first;
    }
    cout << "DOES NOT FIT IN BITMAP. FREE SOME DATA. BITMAP REMAINS UNCHANGED" <<endl;
    return nullptr;
}