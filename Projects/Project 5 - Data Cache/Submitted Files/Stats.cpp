/******************************
 * Name:  Ricardo Reyna - rjr110
 * CS 3339 - Spring 2019
 ******************************/
#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;
  stalls = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  for(int i = IF1; i < PIPESTAGES; i++){
    resultReg[i] = -1;
  }
}

void Stats::clock() {
  cycles++;

  // advance all pipeline flip flops
  for(int i = WB; i > IF1; i--) {
    resultReg[i] = resultReg[i-1];
  }
  // inject a no-op into IF1
  resultReg[IF1] = -1;
}

void Stats::registerSrc(int r) {
  /*  if(r == 0)
    {
        return;
    }

    for(int i = EXE1; i < WB; i++)
    {
        if(resultReg[i] == r)
        {
            bubble(i);
        }
    }*/

  if (r == 0)
  {
    return;
  }

  int bubblesInsert = 0;

  for( int i = EXE1; i < WB; i++)
  {
     if(resultReg[i] == r)
     {
        bubblesInsert = WB - i;
        break;
     }
  }

  for(int i = 0; i < bubblesInsert; i++)
  {
    bubble();
  }


}

void Stats::registerDest(int r) {
    if(r == 0)
    {
        return;
    }

    resultReg[ID] = r;
}

void Stats::flush(int count) { // count == how many ops to flush
    for(int i = 0; i < count; i++)
    {
        flushes++;
        clock();
    }
}

void Stats::Stalls(int count) {            /// Inserted for P5
    for(int i = 0; i < count; i++)
    {
        stalls++;
        cycles++;
    }
}

void Stats::bubble(/*int duplicateRegister*/) {
   /* for(int i = WB; i > duplicateRegister; i--)
    {
        resultReg[i] = resultReg[i-1];
    }
    resultReg[duplicateRegister] = -1;
    bubbles++;
    cycles++;*/

  /*  for(int i = duplicateRegister; i < WB; i++)
    {
        resultReg[i+1] = resultReg[i];
        resultReg[i] = -1;
        bubbles++;
        cycles++;
    }                         */

  cycles++;
  bubbles++;

  for(int i = WB; i > EXE1; i--)
  {
    resultReg[i] = resultReg[i-1];
  }

  resultReg[EXE1] = -1;


}
