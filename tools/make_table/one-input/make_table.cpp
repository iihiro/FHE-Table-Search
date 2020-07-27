//This code makes a demo table, not for real function
#include <iostream>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <random>
#include <vector>
#include <thread>
#include <future>
#include <chrono>

//number of columns is a half of slot count!
#define TABLE_SIZE_COL 819200
#define TABLE_SIZE_ROW 2

int main(void)
{
    std::vector<std::vector<long>> inputTable;
    std::vector<long> inputTable_row;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);

    for(int i=0 ; i<TABLE_SIZE_ROW ; i++){
        for(int j=0 ; j<TABLE_SIZE_COL ; j++){
            if (j < 36) {
                inputTable_row.push_back(j+1);
            } else {
                inputTable_row.push_back(100);
            }
        }
        inputTable.push_back(inputTable_row);
        inputTable_row.clear();
    }
    
    std::ofstream InputTable;
    InputTable.open("LUTin_for-one-input", std::ios::out);
    for(size_t i=0; i<inputTable.size(); ++i){
        for(size_t j=0; j<inputTable[0].size(); ++j){
            InputTable << inputTable[i][j] << ' ';
        }
        InputTable << std::endl;
    }
    InputTable.close();
    
    std::cout << "end" << std::endl;

    return 0;
}
