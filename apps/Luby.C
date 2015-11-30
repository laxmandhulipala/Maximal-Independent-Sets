#include "ligra.h"
#include "math.h"
#include <stdlib.h> 

// seq version of luby
template <class vertex>
int runLubyIteration(graph<vertex>& G, bool* inMis, bool* active, int* priorities) {
  const intE n = G.n;

  // all active v pick a priority
  // non-det: det version uses sequential for
  {parallel_for(long i=0;i<n;i++) if (active[i]) priorities[i] = rand();}
//  for (int i = 0; i < n; i++) {
//    if (active[i]) {
//      priorities[i] = rand();
//    }
//  }

  // loop over active v. Check if ngh is in MIS or 
  {parallel_for (int i = 0; i < n; i++) {
    if (active[i]) {
      intE outDeg = G.V[i].getOutDegree();
      bool stillIn = true;
      for (int j = 0; j < outDeg; j++) {
        intE ngh = G.V[i].getOutNeighbor(j);
        if (inMis[ngh]) {
          active[i] = false;
          inMis[i] = false;
          stillIn = false;
          break;
        }
        if (active[ngh] && priorities[ngh] >= priorities[i]) {
          stillIn = false;
        }
      }
      if (stillIn) {
        inMis[i] = true;
        active[i] = false;
      }
    }
  }}

  int numActive = 0;
  for (int i = 0; i < n; i++) {
    if (active[i]) {
      numActive++;
    }
  }

  return numActive;
}

template<class vertex>
bool checkMis(graph<vertex>& G, bool* inMis) {
  const intE n = G.n;
  bool correct = true;
  for (int i = 0; i < n; i++) {
    intE outDeg = G.V[i].getOutDegree();
    int numConflict = 0;
    bool iInMis = inMis[i];
    for (int j = 0; j < outDeg; j++) {
      // really should write iterator but v lazy
      intE ngh = G.V[i].getOutNeighbor(j);
      if (iInMis && inMis[ngh]) {
        numConflict++;
      }
    }
    if (numConflict > 0) {
      printf("numConflict is %d\n", numConflict);
      correct = false;
    }
  }
  return correct;
}

// ignore using functional-primitives just to make MIS 
// computation as explicit as possible
template <class vertex>
void Compute(graph<vertex>& G, commandLine P) {
  srand(4);
  const intE n = G.n;

  bool* inMis = newA(bool, n);
  bool* active = newA(bool, n);
  int* priorities = newA(int, n);
  {parallel_for(long i=0;i<n;i++) {
      inMis[i] = 0;
      active[i] = 1;
  }}

  int numActive = n;

  int round = 0;
  while (numActive > 0) {
    printf("round = %d, numActive = %d\n", round, numActive);
    numActive = runLubyIteration(G, inMis, active, priorities);
    round++;
  }
  printf("Finished after %d rounds\n", round);

  int numInMis = 0;
  for (int i = 0; i < n; i++) {
    if (inMis[i]) {
      numInMis++;
    }
  }
  bool correct = checkMis(G, inMis);
  printf("numInMis = %d\n", numInMis);
  printf("correct = %d\n", correct);

  free(inMis);
  free(active);
  free(priorities);
}
