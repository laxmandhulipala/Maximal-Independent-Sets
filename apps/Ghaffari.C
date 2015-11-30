#include "ligra.h"
#include "math.h"
#include <stdlib.h> 
#include <algorithm>
#include <functional>
#include <vector>

#define DEGREE_THRESH 2
#define MAX_PROB -1

template <class vertex>
double getEffectiveDegree(graph<vertex>& G, int* desires, bool* active, int i) {
  intE outDeg = G.V[i].getOutDegree();
  double effDeg = 0;
  for (int j = 0; j < outDeg; j++) {
    intE ngh = G.V[i].getOutNeighbor(j);
    if (active[ngh]) {
      effDeg += pow(2, desires[ngh]);
    }
  }
  return effDeg;
}

// assumption: i is still active and needs to be updated
template <class vertex>
void updateEffectiveDegree(graph<vertex>& G, int* prev, int* next, bool* active, int i) {
  double effDeg = getEffectiveDegree(G, prev, active, i);
  int ourDesire = prev[i];
  int nextDesire;
  if (effDeg >= DEGREE_THRESH) {
    nextDesire = ourDesire - 1;
  } else {
    nextDesire = std::min(ourDesire + 1, MAX_PROB);
  }
  next[i] = nextDesire;
}

template <class vertex>
void runGhaffariIteration1(graph<vertex>& G, bool* commit, bool* active, 
                           int* desires, int* nextDesires) {
  const intE n = G.n;

  {parallel_for (int i = 0; i < n; i++) { commit[i] = false; }}

  {parallel_for (int i = 0; i < n; i++) {
    // for all active vertices, if active then try to add self to MIS. Then
    // update probabilities
    if (active[i]) {

      double ourProb = pow(2, desires[i]);
      double randDub = ((double)rand()/(double)RAND_MAX);
      if (randDub <= ourProb) {
        // vtx wants to try and get into this round
        commit[i] = true;
      }
      updateEffectiveDegree(G, desires, nextDesires, active, i);
    }
  }}
}


template <class vertex>
int runGhaffariIteration2(graph<vertex>& G, bool* commit, bool* active, bool* inMis) {
  const intE n = G.n;

  {parallel_for (int i = 0; i < n; i++) {
    // for all active vertices, if active then try to add self to MIS. Then
    // update probabilities
    if (commit[i]) {
      // iterate thru ngh, make sure 
      intE outDeg = G.V[i].getOutDegree();
      bool stillIn = true;
      for (int j = 0; j < outDeg; j++) {
        intE ngh = G.V[i].getOutNeighbor(j);
        if (inMis[ngh]) {
          // ngh made it, we're out. We can also take ourselves out of commit, but
          // that breaks determinism a bit. 
          active[i] = false;
          stillIn = false;
        }
        if (commit[ngh]) {
          // ngh also tried to commit. Back off
          stillIn = false;
        }
      }

      if (stillIn) {
        active[i] = false;
        inMis[i] = true;
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

template <class vertex>
void Compute(graph<vertex>& G, commandLine P) {
  srand(4);
  const intE n = G.n;

  bool* inMis = newA(bool, n);
  bool* commit = newA(bool, n);
  bool* active = newA(bool, n);
  int* desires = newA(int, n);
  int* nextDesires = newA(int, n);

  double init = pow(2, -1);
  {parallel_for(long i=0;i<n;i++) {
      inMis[i] = 0;
      active[i] = 1;
      desires[i] = init;
  }}
  printf("init is %lf\n", init);

  int numActive = n;

  int round = 0;
  while (numActive > 0) {
    printf("round = %d, numActive = %d\n", round, numActive);
    runGhaffariIteration1(G, commit, active, desires, nextDesires);
    numActive = runGhaffariIteration2(G, commit, active, inMis);
    swap(desires, nextDesires);
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
}
