#include "Solver.h"

Solver::Solver(const Instance *i) {
    m_solution = Solution(i);
    m_timeLimit = 0.1;
    m_instance = i;
}

void Solver::setInstance(const Instance *i) {
    m_instance = i;
}

Solution Solver::solve() {
    if(m_instance == NULL) {
        cout << "Instance not initialized\n";
        return NULL;
    }

    BossaTimer input_timer;
    input_timer.start();

    m_solution.greedySolution();
    m_solution.vnd();

    Solution best = m_solution;
    Solution solPert = m_solution;
    int ctn = 0;
    
    while (input_timer.getTime() < m_timeLimit) {
        m_solution = solPert;
        for (int p = 0; p < 1; p++) {
            m_solution.pertubateUser();

        }
        m_solution.vnd();
        ctn++;

        if (m_solution.getSolutionCost() < solPert.getSolutionCost()) {
            solPert = m_solution;
            if (m_solution.getSolutionCost() < best.getSolutionCost()) {
                best = m_solution;
            }
            ctn = 0;
        }
        else if (ctn >= 2) {
            Solution s(m_instance);
            solPert = s;
            solPert.greedySolution();
            ctn = 0;
        }
    }
    m_solution = best;
    return best;
}

void Solver::generateUsersOutput() {
    int users = m_instance->users();
    ofstream outfile;
    string file_name = "./output/ue_data.csv";
    
    // if file does not exists, create a new one
    if(!ifstream(file_name).good()) {
        outfile.open(file_name, ios::out);
        assert(outfile.is_open());
        outfile << "ue_id,enb_alloc,h_i,d_ki,rssi,w_i,cost\n";
    }
    else {
        outfile.open(file_name, ios::out | ios::app);
        assert(outfile.is_open());
    }

    for(int k = 0; k < users; k++) {
        int enb = m_solution.getUserConnectionTable().at(k);
        outfile << k <<',' << enb << ',' << m_instance->handover_i(enb) <<',' 
            << m_instance->distance(enb, k) << ',' << m_instance->rssi(enb, k) <<','
            << m_solution.getEnbUsedBandwidth(enb) << ',' << m_solution.getUserCurrentCost(enb, k) << endl;
    }
    outfile.close();
}

void Solver::generateEnbsOutput() {
    int enbs = m_instance->enbs();
    ofstream outfile;
    string file_name = "./output/enb_data.csv";
    
    // if file does not exists, create a new one
    if(!ifstream(file_name).good()) {
        outfile.open(file_name, ios::out);
        assert(outfile.is_open());
        outfile << "enb_id,conn_users,h_i,w_i,cost\n";
    }
    else {
        outfile.open(file_name, ios::out | ios::app);
        assert(outfile.is_open());
    }

    for(int i = 0; i < enbs; i++) {
        int enb = m_solution.getUserConnectionTable().at(i);
        outfile << i <<',' << enb << ',' << m_instance->handover_i(enb) <<',' 
            << m_instance->distance(enb, i) << ',' << m_instance->rssi(enb, i) <<','
            << m_solution.getEnbUsedBandwidth(enb) << ',' << m_solution.getUserCurrentCost(enb, i) << endl;
    }
    outfile.close();
}