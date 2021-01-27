/******************************
 * Name:  Ricardo Reyna - rjr110
 * CS 3339 - Spring 2019
 ******************************/
#ifndef __STATS_H
#define __STATS_H
#include <iostream>
#include "Debug.h"
using namespace std;

enum PIPESTAGE { IF1 = 0, IF2 = 1, ID = 2, EXE1 = 3, EXE2 = 4, MEM1 = 5,
                 MEM2 = 6, WB = 7, PIPESTAGES = 8 };

class Stats {
  private:
    long long cycles;
    int flushes;
    int bubbles;
    int stalls; /// Inserted for P5

    int memops;
    int branches;
    int taken;

    // This represents the pipeline
    int resultReg[PIPESTAGES];

  public:
    Stats();

    void clock();

    void flush(int count);
    void Stalls(int count); /// Inserted for P5

    void registerSrc(int r);
    void registerDest(int r);

    void countMemOp() { memops++; }
    void countBranch() { branches++; }
    void countTaken() { taken++; }

    // getters
    long long getCycles() { return cycles; }
    int getFlushes() { return flushes; }
    int getBubbles() { return bubbles; }
    int getStalls() { return stalls; }
    int getMemOps() { return memops; }
    int getBranches() { return branches; }
    int getTaken() { return taken; }

  private:
    void bubble(/*int r*/);
};

#endif
