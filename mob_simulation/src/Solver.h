#ifndef SOLVER_H_
#define SOLVER_H_

#include <iostream>
#include "bossa_timer.h"

#include "Solution.h"
#include "Instance.h"

using namespace std;
using namespace opt;

class Solver {

public:

    Solver(const Instance *i);
    Solution solve();
    void setInstance(const Instance *i);
    
    void generateUsersOutput();
    void generateEnbsOutput();

private:
    Solution m_solution;
    const Instance *m_instance;
    int m_timeLimit;
};

#endif // #ifndef SOLVER_H_