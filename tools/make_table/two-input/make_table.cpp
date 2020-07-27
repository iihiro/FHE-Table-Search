#include <iostream>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <random>
#include <vector>
#include <thread>
#include <future>
#include <ctime>
#include <ratio>
#include <math.h>
#include <cstddef>
#include <iomanip>
#include <string>

#define NUM 4096

int main(void)
{
  std::vector<std::vector<long>> inputT;
  std::vector<long> inputT_row, outputT;

  for (long i=0; i<2; ++i) {
      for (long j=0; j<NUM; ++j) {
          if (j<36) {
              inputT_row.push_back(j+1);
          } else {
              inputT_row.push_back(100);
          }
      }
      inputT.push_back(inputT_row);
      inputT_row.clear();
  }

  long k=0, d=0;
  for (long i=0; i<NUM; ++i) {
      for(long j=0; j<NUM; ++j) {
          ++d;
          if (i<36 && j<36) {
              outputT.push_back(inputT[0][i] + inputT[1][j]);
              ++k;
          } else {
              outputT.push_back(1000);
          }
      }
  }
  std::cout<<"all output number(not100):"<<k<<",sum num:"<<d<<std::endl;
  
  std::ofstream InputTable;
  InputTable.open("LUTin_for-two-input", std::ios::out);
  InputTable << NUM << std::endl;
  for (size_t i=0; i<inputT.size(); ++i) {
      for(size_t j=0; j<inputT[0].size(); ++j) {
          InputTable << inputT[i][j] << ' ';
      }
      InputTable << std::endl;
  }
  InputTable.close();

  std::ofstream OutputTable;
  OutputTable.open("LUTout_for-two-input", std::ios::out);
  OutputTable << (NUM * NUM) << std::endl;
  for (size_t i=0; i<outputT.size(); ++i){
      OutputTable << outputT[i] << ' ';
  }
  OutputTable << std::endl;
  OutputTable.close();

  std::cout << "end" << std::endl;
  return 0;
}
