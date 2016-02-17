#include "ligra.h"

template<class vertex>
bool checkMis(graph<vertex>& G, bool* inMis) {
  const intE n = G.n;
  bool correct = true;
  for (int i = 0; i < n; i++) {
    intE outDeg = G.V[i].getOutDegree();
    int numConflict = 0;
    bool iInMis = inMis[i];
    for (int j = 0; j < outDeg; j++) {
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

struct Luby_P1 {
  int* priorities;
  bool* active;
  bool* inMis;
  Luby_P1(int* _priorities, bool* _active, bool* _inMis) : 
    priorities(_priorities), active(_active), inMis(_inMis) {}
  inline bool update (uintE s, uintE d) { 
    if (priorities[s] < priorities[d]) {
      inMis[s] = false;
    }
    return 0; 
  }
  inline bool updateAtomic (uintE s, uintE d) {
    if (priorities[s] > priorities[d]) {
      (CAS(&inMis[s],true,false));
    }
    return 0;
  }
  inline bool cond (uintE d) { return (active[d] == true); } 
};

struct Luby_P2 {
  bool* active;
  bool* inMis;
  Luby_P2(bool* _active, bool* _inMis) : 
    active(_active), inMis(_inMis) {}
  inline bool update (uintE s, uintE d) {
    active[s] = false;
    return 0;
  }
  inline bool updateAtomic (uintE s, uintE d) { 
    (CAS(&active[s], true, false));
    return 0;
  }
  inline bool cond (uintE d) { return (inMis[d] == true); } 
};

struct Luby_Vertex_F {
  int* priorities;
  bool* inMis;
  Luby_Vertex_F(int* _priorities, bool* _inMis) : 
    priorities(_priorities), inMis(_inMis) {} 
  inline bool operator ()  (uintE i) {
    priorities[i] = rand(); 
    inMis[i] = true;
    return 1;
  }
};

struct Luby_Filter {
  bool* active;
  bool* inMis;
  Luby_Filter(bool* _active, bool* _inMis) : 
    active(_active), inMis(_inMis)  {}
  inline bool operator () (uintE i) {
    return (active[i] && !inMis[i]);
  }
};

template <class vertex>
void Compute(graph<vertex>& GA, commandLine P) {
  const intE n = GA.n;
  bool checkCorrectness = P.getOptionValue("-checkCorrectness");

  bool* inMis = newA(bool, n);
  bool* active = newA(bool, n);
  bool* frontier_data = newA(bool, n);
  int* priorities = newA(int, n);
  {parallel_for(long i=0;i<n;i++) {
    inMis[i] = 0;
    active[i] = 1;
    frontier_data[i] = 1;
  }}

  int round = 0;
  vertexSubset Frontier(n, frontier_data);
  while (!Frontier.isEmpty()) {
    // assign priorities. set all active vertices to be in MIS
    vertexMap(Frontier, Luby_Vertex_F(priorities, inMis));
    // compare priorities. unset from MIS if p(u) > p(v) for any (u,v)
    edgeMap(GA, Frontier, Luby_P1(priorities, active, inMis), GA.m/20);
    // make vertex inactive if for any (u,v) v \in MIS
    edgeMap(GA, Frontier, Luby_P2(active, inMis), GA.m/20);
    vertexSubset output = vertexFilter(Frontier, Luby_Filter(active, inMis));
    Frontier.del();
    Frontier = output;
    round++;
  }

  if (checkCorrectness) {
    cout << "Mis correctness : " << checkMis(GA, inMis) << endl;
  }

  free(inMis);
  free(active);
  free(priorities);
  Frontier.del();
}
