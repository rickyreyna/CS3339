/******************************
 * CacheStats.cpp submitted by: Ricardo Reyna rjr110
 * CS 3339 - Spring 2019
 * Project 4 Branch Predictor
 * Copyright 2019, all rights reserved
 * Updated by Lee B. Hinkle based on prior work by Martin Burtscher and Molly O'Neil
 ******************************/
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "CacheStats.h"
using namespace std;

CacheStats::CacheStats() {

  cout << "Cache Config: ";

  if(!CACHE_EN)
  {
    cout << "cache disabled" << endl;
  }
  else
  {
    cout << (SETS * WAYS * BLOCKSIZE) << " B (";
    cout << BLOCKSIZE << " bytes/block, " << SETS << " sets, " << WAYS << " ways)" << endl;
    cout << "  Latencies: Lookup = " << LOOKUP_LATENCY << " cycles, ";
    cout << "Read = " << READ_LATENCY << " cycles, ";
    cout << "Write = " << WRITE_LATENCY << " cycles" << endl;
  }

  loads = 0;
  stores = 0;
  load_misses = 0;
  store_misses = 0;
  writebacks = 0;

  /* TODO: your code here */
  for(int i = 0; i < SETS; ++i)  /// Sets cache
  {
      round_robin[i] = 0;
      for(int j = 0; j < WAYS; ++j)
      {
        modify_bit[i][j] = false;
        valid_bit[i][j] = false;
        tags[i][j] = 0;
      }
  }
}

int CacheStats::access(uint32_t addr, ACCESS_TYPE type) {
  if(!CACHE_EN) { // cache is off
    return (type == LOAD) ? READ_LATENCY : WRITE_LATENCY;
  }

  index = (addr >> 5) & 7; /// CURRENT #  OF SET
  tag = addr >> 8;
  stalls = 0;

  if(type == LOAD)
  {
      loads++;
      for(int i = 0; i < WAYS; ++i)
      {
         if(valid_bit[index][i] && (tags[index][i] == tag))
         {
             return stalls;
         }
      }

      load_misses++;

      way = round_robin[index];
      tags[index][way] = tag;
      valid_bit[index][way] = true;
      round_robin[index] = (round_robin[index] + 1) % WAYS;
      stalls += READ_LATENCY;

      if(modify_bit[index][way] == true)
      {
          stalls += WRITE_LATENCY;
          writebacks++;
      }

      modify_bit[index][way] = false;
      return stalls;
  }

  else if(type == STORE)
  {
      stores++;
      for(int i = 0; i < WAYS; ++i)
      {
          if(valid_bit[index][i] && (tags[index][i] == tag))
          {
              modify_bit[index][i] = true;
              return stalls;
          }
      }

      store_misses++;

      way = round_robin[index];
      tags[index][way] = tag;
      valid_bit[index][way] = true;
      round_robin[index] = (round_robin[index] + 1) % WAYS;
      stalls += READ_LATENCY;

      if(modify_bit[index][way] == true)
      {
        stalls += WRITE_LATENCY;
        writebacks++;
      }

      modify_bit[index][way] = true;
      return stalls;
  }
}

void CacheStats::printFinalStats() {
  /* TODO: your code here (don't forget to drain the cache of writebacks) */
  for(int i = 0; i < SETS; ++i)
  {
      for(int j = 0; i < WAYS; ++i)
      {
          if(modify_bit[i][j])
          {
              writebacks++;
          }
      }
  }

  int accesses = loads + stores;
  int misses = load_misses + store_misses;

  cout << "Accesses: " << accesses << endl;
  cout << "  Loads: " << loads << endl;
  cout << "  Stores: " << stores << endl;
  cout << "Misses: " << misses << endl;
  cout << "  Load misses: " << load_misses << endl;
  cout << "  Store misses: " << store_misses << endl;
  cout << "Writebacks: " << writebacks << endl;
  cout << "Hit Ratio: " << fixed << setprecision(1) << 100.0 * (accesses - misses) / accesses;
  cout << "%" << endl;
}
