#include "Solution.h"


namespace opt
{
    Solution::Solution() {}

    Solution::Solution(const Instance *i) : i_(i) {
        int enbs = i_->enbs();
        int ues = i_->users();
        int dcs = i_->data_centers();

        resetInstance();

        loadUserStatistics();
        loadUserAndEnbsBdwTotalRequirements();
        loadCostPerUserList();

        for(int i = 0; i < ues; i++) { m_usersList.push_back(i); }
        for(int i = 0; i < enbs; i++) { m_eNBsList.push_back(i); }
    };

    void Solution::resetInstance() {
        m_enbUsedBandwidth = i_->enbUsedBandwidth();

        m_enbUserCount = vector<int>(i_->enbs(), 0);

        m_enbsCurrentCost = vector<vector<double>>(i_->enbs(), vector<double>(i_->enbs(), INT32_MAX));
        m_enbsDcCurrentCost = vector<vector<double>>(i_->enbs(), vector<double>(i_->data_centers(), INT32_MAX));
        m_uesCurrentCost = vector<vector<double>>(i_->enbs(), vector<double>(i_->users(), INT32_MAX));

        m_enbConnectionTable = vector<int>(i_->enbs(), -1);
        m_enbDcConnectionTable = vector<int>(i_->enbs(), -1);
        m_userConnectionTable = vector<int>(i_->users(), -1);
        m_userPreviousAllocSolution = vector<int>(i_->users(), -1);

        m_connectedUsersCounter = 0;

        m_noExistingSolution = false;
        m_solutionCost = -1;
        m_pertubationLimit = 1;
        m_pertubationAttemps = 50;
    }

    void Solution::loadCostPerUserList() {
        vector<costUEPairVEC> m_costPerUserList = vector<costUEPairVEC>(i_->enbs(), costUEPairVEC(i_->users(), make_pair(0,0)));
        for (int i = 0; i < i_->enbs(); i++) {
            for (int ue = 0; ue < i_->users(); ue++) {
			    double cost = getUserStartingCost(i, ue);
			    m_costPerUserList[i][ue] = make_pair(cost, ue);
            }
            sort(m_costPerUserList[i].begin(), m_costPerUserList[i].end());
        }
    }

    void Solution::loadUserAndEnbsBdwTotalRequirements() {
        m_totalMaxBdw = 0;

        for(int i = 0; i < i_->enbs(); i++) {
            m_totalMaxBdw += i_->enbMaxBandwidth(i);
            m_totalUserRequiredBdw = 0;
            for(int k = 0; k < i_->users(); k++ ){
                m_totalUserRequiredBdw += i_->userRequiredBandwidth(i, k);
            }
        }
    }

    void Solution::loadUserStatistics(){
        for(int k = 0; k < i_->users(); k++) {
            double min = INT64_MAX;
            double max = INT64_MIN;
            double avg = 0;
            for(int i = 0; i < i_->enbs(); i++) {
                double val = getUserStartingCost(i, k);
                if(val > max) {
                    max = val;
                }

                if(val < min) {
                    min = val;
                }
                avg += val;
            }
            avg /= i_->enbs();
            m_userCostStatistics.push_back( make_pair( make_pair(min, max), avg));
        }
    }

    void Solution::shuffleUsers() {
        random_shuffle ( m_usersList.begin(), m_usersList.end() );
    }

    void Solution::shuffleEnbs() {
        random_shuffle ( m_eNBsList.begin(), m_eNBsList.end() );
    }

    // Return the cost to connect user k to eNB_j
    double Solution::getUserCurrentCost(int j, int k) {
        return m_uesCurrentCost[j][k];
    }

    // Return the cost to connect user k to eNB_j
    double Solution::getUserStartingCost(int j, int k) {
        double cost = i_->rsrq(j, k) + i_->handover_i(j);
        return cost;
    }

	double Solution::getSolutionCost() {
		return m_solutionCost;
	}

    double Solution::getEnbUsedBandwidth(const int enb) {
        return m_enbUsedBandwidth[enb];
    }

	void Solution::setEnbUsedBandwidth(const int enb, double bdw) {
        m_enbUsedBandwidth[enb] = bdw;
    }

    double Solution::normalize(double val, double min, double max) {
        return (val - min)/(max - min);
    }

    double Solution::denormalize(double val, double min, double max) {
        return val*(max-min) + min;
    }

    bool Solution::isValidSolution(){
        return allUsersConnected() && validEnbsCapacity();
    }

    bool Solution::validEnbsCapacity() {
        int enbs = i_->enbs();
        for(int j = 0; j < enbs; j++) {
            double enbCurrentBdw = 0;
            int connected_users = 0;
            for(int k = 0; k < i_->users(); k++) {
                if(m_userConnectionTable[k] == j) {
                    enbCurrentBdw += i_->userRequiredBandwidth(j, k);
                    connected_users++;
                }
            }

            if(enbCurrentBdw > i_->enbMaxBandwidth(j)) {
                return false;
            }
        }
        return true;
    }

    bool Solution::allUsersConnected() {
        for(int k = 0; k < i_->users(); k++) {
            if(m_userConnectionTable[k] == -1) {
                return false;
            }
        }
        return true;
    }

    vector<int> Solution::getUserConnectionTable() {
        return m_userConnectionTable;
    }

    bool Solution::isBandwidthAvailable(const int enb, const int user) {
        double enbUsedbdw = getEnbUsedBandwidth(enb);
        double userReqBdw = normalize(i_->userRequiredBandwidth(enb, user), 0, i_->enbMaxBandwidth(enb));
        if(enbUsedbdw + userReqBdw > 1) {
            return false;
        }
        return true;
    }

    //Check if there is enough bandwidth to disconnect ue_v and connect ue_v
    bool Solution::isBandwidthAvailableSP(const int enb, const int ue_u, const int ue_v) {
        double enbUsedbdw = getEnbUsedBandwidth(enb);
        double ue_u_ReqBdw = normalize(i_->userRequiredBandwidth(enb, ue_u), 0, i_->enbMaxBandwidth(enb));
        double ue_v_ReqBdw = normalize(i_->userRequiredBandwidth(enb, ue_v), 0, i_->enbMaxBandwidth(enb));

        if(enbUsedbdw - ue_u_ReqBdw + ue_v_ReqBdw > 1) {
            return false;
        }
        return true;
    }

    // Allocate user k in eNB_i 
	bool Solution::connectUserToEnb(const int i, const int k) {
		if(i < 0 || k < 0) {
			return false;
		}
        m_enbUsedBandwidth[i] += normalize(i_->userRequiredBandwidth(i, k), 0, i_->enbMaxBandwidth(i));
        m_userConnectionTable[k] = i;
        m_enbUserCount[i]++;
        m_connectedUsersCounter++;
        return true;
	}

	// Remove user k from eNB_i 
	bool Solution::disconnectUserFromEnb(const int i, const int k) {
		if(i < 0 || k < 0) {
			return false;
		}		
        double userReqBdw = normalize(i_->userRequiredBandwidth(i, k), 0, i_->enbMaxBandwidth(i));
        m_userConnectionTable[k] = -1;
        m_enbUsedBandwidth[i] -= userReqBdw;
        m_enbUserCount[i]--;
        m_connectedUsersCounter--;
        return true;
	}

    void Solution::disconnectAllUsersFromEnb() {
        for(int k = 0; k < i_->users(); k++) {
            int enb = m_userConnectionTable[k];
            if(enb != -1) {
                disconnectUserFromEnb(enb, k);
            }
        }
        computeSolutionCost();
    }

    // Connect eNB_i to data center s
	void Solution::connectEnbToDc(const int i, const int s) {
		if(i < 0 || s < 0) {
			return;
		}
        m_enbDcConnectionTable[i] = s;
        m_enbsDcCurrentCost[i][s] = i_->cost(i, s);
	}

	// Disconnect eNB_i from data center s 
	void Solution::disconnectEnbFromDc(const int i, const int s) {
		if(i < 0 || s < 0) {
			return;
		}		
        m_enbDcConnectionTable[i] = -1;
        m_enbsDcCurrentCost[i][s] = INT64_MAX;
	}

    // Connect eNB_j to eNB_i 
	void Solution::connectEnbs(const int i, const int j) {
		if(i < 0 || j < 0) {
			return;
		}
		m_enbConnectionTable[i] = j;
        m_enbConnectionTable[j] = i;
        m_enbsCurrentCost[i][j] = i_->handover_avg(i, j);
        m_enbsCurrentCost[j][i] = i_->handover_avg(i, j);
	}

	// Disconnect eNB_j from eNB_i 
	void Solution::disconnectEnbs(const int i, const int j) {
		if(i < 0 || j < 0) {
			return;
		}		
        m_enbConnectionTable[i] = -1;
        m_enbConnectionTable[j] = -1;
        m_enbsCurrentCost[i][j] = INT64_MAX;
        m_enbsCurrentCost[j][i] = INT64_MAX;
	}

    void Solution::greedySolution() {
        double min = 0;
        int enb = 0; 
        // int dc = -1;

        //Connect users to the best eNBs
        shuffleUsers();
        shuffleEnbs();

        for (int k = 0; k < i_->users(); k++) {
            // enb = -1;
            min = INT64_MAX;
            int ue_k = m_usersList.at(k);
 
            for (int j = 0; j < i_->enbs(); j++) {
                int enb_j = m_eNBsList.at(j);
                double ue_cost = getUserStartingCost(enb_j, ue_k);

                if (ue_cost < min) {
                    if(isBandwidthAvailable(enb_j, ue_k)) {
                        min = ue_cost;
                        enb = enb_j;
                    }
                }
            }

            //Disconnect the user from the current eNB
            if(m_userConnectionTable[ue_k] >= 0) {
                disconnectUserFromEnb(m_userConnectionTable[ue_k], ue_k);
            }

            //Found a enb to connect
            if(enb != -1) {
                connectUserToEnb(enb, ue_k);
                m_uesCurrentCost[enb][ue_k] = min;
            }
        }

        if(m_connectedUsersCounter == 0) {
            m_noExistingSolution = true;
            return;
        }
        
        computeSolutionCost();
    }
    
    // Best improvement insert user local search
    bool Solution::bestImprovementInsertUser() {
        int enb_ = i_->enbs();
        int users_ = i_->users(); 
        int best_i = -1;
        int best_j = -1;
        int best_k = -1;
        double best_gain = 0;
        double currentCost = 0;
        for (int u = 0; u < users_; u++) {
            int currentEnb = m_userConnectionTable[u];
            if(currentEnb == -1) continue; /**TODO: Fix the case where the user is not connected**/  
            for(int j = 0; j < enb_; j++) {
                if(j == currentEnb || !isBandwidthAvailable(j, u)) continue;
                double gain = m_uesCurrentCost[currentEnb][u] - getUserStartingCost(j, u);
                if(gain > best_gain) {
                    best_gain = gain;
                    best_i = currentEnb;
                    best_j = j;
                    best_k = u;
                    currentCost = getUserStartingCost(j, u);
                    goto first;
                }
            }
        }

        first:
        if (best_gain > 0) {
            disconnectUserFromEnb(best_i, best_k);
            connectUserToEnb(best_j, best_k);
             m_uesCurrentCost[best_j][best_k] = currentCost;
             m_userConnectionTable[best_k] = best_j;
            computeSolutionCost();
            return true;
        }
        return false;
    }

    void Solution::forceSwap() {

        uniform_int_distribution<int> distUE(0, i_->users() - 1);
        int ue_u = distUE(generator);
        int ue_v = distUE(generator);

        int u_enb = m_userConnectionTable[ue_u];
        int v_enb = m_userConnectionTable[ue_v];

        do {
            ue_v = distUE(generator);
            v_enb = m_userConnectionTable[ue_v];
        } while (ue_u == ue_v || u_enb == v_enb || v_enb == m_userPreviousAllocSolution[ue_u]);

        disconnectUserFromEnb(u_enb, ue_u);
        connectUserToEnb(v_enb, ue_u);
        m_uesCurrentCost[v_enb][ue_u] = getUserStartingCost(v_enb, ue_u);
        m_userPreviousAllocSolution[ue_u] = u_enb;

        // disconnect user v from eNB_j and inserts
        // it in eNB_i
        disconnectUserFromEnb(v_enb, ue_v);
        connectUserToEnb(u_enb, ue_v);
        m_uesCurrentCost[u_enb][ue_v] = getUserStartingCost(u_enb, ue_v);
        m_userPreviousAllocSolution[ue_v] = v_enb;
        computeSolutionCost();
    }

     bool Solution::bestImprovementSwapUser() {
        int users_ = i_->users();
        for(int ue_u = 0; ue_u < users_; ue_u++) {
            for(int ue_v = 0; ue_v < users_; ue_v++) {
                if(ue_u == ue_v) continue;

                int u_enb = m_userConnectionTable[ue_u];
                int v_enb = m_userConnectionTable[ue_v];

                //Check for user disconnection
                if(u_enb == -1 || v_enb == -1) {
                   return false;
                }

                if(!isBandwidthAvailableSP(u_enb, ue_u, ue_v) || !isBandwidthAvailableSP(v_enb, ue_v, ue_u)) {
                    // cout << "Insufficient bandwidth\n";
                    continue;
                }

                 //Do not swap back to a previous state
                if(v_enb == m_userPreviousAllocSolution[ue_u] || u_enb == m_userPreviousAllocSolution[ue_v]) {
                    // cout << "Skipping swap to avoid same previous state\n";
                    continue;
                }

                double curr_cost = m_uesCurrentCost[u_enb][ue_u] + m_uesCurrentCost[v_enb][ue_v];
                double swap_cost = getUserStartingCost(v_enb, ue_u) + getUserStartingCost(u_enb, ue_v);
                double gain = curr_cost - swap_cost;

                if (gain > 0) {

                    disconnectUserFromEnb(u_enb, ue_u);
                    connectUserToEnb(v_enb, ue_u);
                    m_uesCurrentCost[v_enb][ue_u] = getUserStartingCost(v_enb, ue_u);
                    m_userPreviousAllocSolution[ue_u] = u_enb;

                    // disconnect user v from eNB_j and inserts
                    // it in eNB_i
                    disconnectUserFromEnb(v_enb, ue_v);
                    connectUserToEnb(u_enb, ue_v);
                    m_uesCurrentCost[u_enb][ue_v] = getUserStartingCost(u_enb, ue_v);
                    m_userPreviousAllocSolution[ue_v] = v_enb;
                    computeSolutionCost();
                    // m_pertubationAttemps = 50;
                    return true;
                }
            }
            // m_userSwapQueue.pop();
        }
        return false;
    }

    bool Solution::bestImprovementInsertEnb() {
        int dcs_ = i_->data_centers();
        int enbs_ = i_->enbs();
        int best_dc_s = -1;
        int best_enb_i = -1;
        double best_gain = 0;

        for (int i = 0; i < enbs_; i++) {
            int currentDc = m_enbDcConnectionTable[i];

            for(int s = 0; s < dcs_; s++) {
                if(s == currentDc) continue;
                double gain = m_enbsDcCurrentCost[i][currentDc] - i_->cost(i, s);
                if (gain > best_gain) {
                    best_gain = gain;
                    best_dc_s = s;
                    best_enb_i = i;
                }
            }
        }

        if (best_gain > 0) {
            //disconnect eNB_i from the current data center and
            //inserts it into data center s
            disconnectEnbFromDc(best_enb_i, m_enbDcConnectionTable[best_enb_i]);
            connectEnbToDc(best_enb_i, best_dc_s);
            computeSolutionCost();
            return true;
        }
        return false;
    }

    bool Solution::bestImprovementSwapEnb() {
        int enbs_ = i_->enbs();
        int best_dc_s = -1;
        int best_dc_l = -1;
        int best_enb_i = -1;
        int best_enb_j = -1;
        float best_gain = 0;

        for (int i = 0; i < enbs_; i++) {
            for(int j = 0; j < enbs_; j++) {
                if(i == j) continue;
                int i_dc = m_enbDcConnectionTable[i];
                int j_dc = m_enbDcConnectionTable[j];
                float curr_cost = m_enbsDcCurrentCost[i][i_dc] + m_enbsDcCurrentCost[j][j_dc];
                float swap_cost = i_->cost(i, j_dc) + i_->cost(j, i_dc);
                float gain = curr_cost - swap_cost;
                if (gain > best_gain) {
                    best_gain = gain;
                    best_enb_i = i;
                    best_enb_j = j;
                    best_dc_s = i_dc;
                    best_dc_l = j_dc;
                }
            }
        }

        if (best_gain > 0) {
            // disconnect eNB_i from data center s and inserts
            // eNB_j in data center s
            disconnectEnbFromDc(best_enb_i, m_enbDcConnectionTable[best_enb_i]);
            connectEnbToDc(best_enb_j, best_dc_s);

            // disconnect eNB_j from data center l and inserts
            // eNB_i in data center l
            disconnectEnbFromDc(best_enb_j, m_enbDcConnectionTable[best_enb_j]);
            connectEnbToDc(best_enb_i, best_dc_l);
            computeSolutionCost();
            return true;
        }
        return false;
    }


    bool Solution::vnd() {
        int k = 1;
        bool flag = false;
        bool improvement = false;
        while (k < 3) {
            switch (k) {
            case 1:
                flag = bestImprovementInsertUser();
                break;
            case 2:
                flag = bestImprovementSwapUser();
                break;
            // case 3:
            //     flag = bestImprovementInsertEnb();
            //     break;
            // case 4:
            //     flag = bestImprovementSwapEnb();
            //     break;
            }

            if (flag) {
                k = 1;
                improvement = true;
            }
            else {
                k++;
            }
        }
        return improvement;
    }

    void Solution::pertubateUser() {
        if(pertubateUserSwap()) {
            return;
        }
        pertubateUserInsert();
    }

    bool Solution::pertubateUserSwap() {
        int ues = i_->users();
        uniform_int_distribution<int> distUE(0, ues - 1);

        int ue_u = distUE(generator);
        int ue_v = distUE(generator);

        int u_enb = m_userConnectionTable[ue_u];
        int v_enb = m_userConnectionTable[ue_v];

        ue_v = distUE(generator);
        v_enb = m_userConnectionTable[ue_v];
        if(ue_u == ue_v || u_enb == v_enb || v_enb == m_userPreviousAllocSolution[ue_u]) {
            return false;
        }

        if(!isBandwidthAvailableSP(u_enb, ue_u, ue_v) || !isBandwidthAvailableSP(v_enb, ue_v, ue_u)) {
            return false;
        }

        disconnectUserFromEnb(u_enb, ue_u);
        connectUserToEnb(v_enb, ue_u);
        m_uesCurrentCost[v_enb][ue_u] = getUserStartingCost(v_enb, ue_u);
        m_userPreviousAllocSolution[ue_u] = u_enb;

        // disconnect user v from eNB_j and inserts
        // it in eNB_i
        disconnectUserFromEnb(v_enb, ue_v);
        connectUserToEnb(u_enb, ue_v);
        m_uesCurrentCost[u_enb][ue_v] = getUserStartingCost(u_enb, ue_v);
        m_userPreviousAllocSolution[ue_v] = v_enb;
        computeSolutionCost();
        return true;
    }

    bool Solution::isCostAcceptable(const int enb, const int ue) {
        double cost = getUserStartingCost(enb, ue);
        double avg =  m_userCostStatistics.at(ue).second; //GRASP AVG
        if(cost <= avg *  m_pertubationAttemps/100) {
            return true;
        }
        return false;
    }

    bool Solution::pertubateUserInsert() {
        int enbs = i_->enbs();
        uniform_int_distribution<int> distEnbs(0, enbs - 1);

        int enb_j = 0;
        int enb_i = distEnbs(generator);
                
        do {
            enb_j = distEnbs(generator);
        } while (enb_i == enb_j);

        int users_on_i = m_enbUserCount[enb_i];

        if(users_on_i == 0){
            return false;
        }

        uniform_int_distribution<int> distUE_u(0, users_on_i - 1);
        int ue_u = distUE_u(generator);

        if(!isBandwidthAvailable(enb_j, ue_u)) {
            return false;
        }

        disconnectUserFromEnb(enb_i, ue_u);
        connectUserToEnb(enb_j, ue_u);
        m_uesCurrentCost[enb_j][ue_u] = getUserStartingCost(enb_j, ue_u);
        m_userPreviousAllocSolution[ue_u] = enb_i;

        //Compute solution's new cost
        computeSolutionCost();
        return true;
    }

    void Solution::computeSolutionCost() {
        // Calculate the cost to allocate the users in eNBs
        if(!isValidSolution()){
            return;
        }

        double ueCost = 0;
        for(int u = 0; u < i_->users(); u++) {
                if(m_userConnectionTable[u] != -1) {

                    ueCost += m_uesCurrentCost[m_userConnectionTable[u]][u];
                }
        }
        
        m_solutionCost = ueCost;
    }

} // namespace opt