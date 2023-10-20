#ifndef SOLUTION_H_
#define SOLUTION_H_

#include "Instance.h"
#include <assert.h>
#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <queue>
#include <algorithm>

using namespace std;

typedef vector<pair<double,int>> costUEPairVEC;

namespace opt
{

extern mt19937 generator;

class Solution
{

public:

	Solution();
	Solution(const Instance *g);

	/**
	 * @brief Get the Enb Used Bandwidth value
	 * 
	 * @param enb 
	 * @return double 
	 */
	double getEnbUsedBandwidth(const int enb);

	/**
	 * @brief Set the Enb Used Bandwidth
	 * 
	 * @param enb 
	 * @param bdw 
	 */
	void setEnbUsedBandwidth(const int enb, double bdw);

	bool bestImprovementInsertUser();
	bool bestImprovementSwapUser();
	bool bestImprovementInsertEnb();
	bool bestImprovementSwapEnb();

	void greedySolution();
	void pertubateDc();

	void pertubateUser();
	bool pertubateUserInsert();
	bool pertubateUserSwap();

	void computeSolutionCost();

	void printUserAlloc();
	void printEnbAlloc();
	void printDcAlloc();

	vector<int> getUserConnectionTable();

	bool vnd();

	double getSolutionCost();
	double getUserCurrentCost(const int enb, const int ue);

	bool isSolutionImpossible();

	// Solution verification functions
	bool allUsersConnected();
	bool validEnbsCapacity();
	bool isValidSolution();

	void disconnectAllUsersFromEnb();

	void resetInstance();

	void forceSwap();

	int m_pertubationLimit;
	int m_pertubationAttemps;

	vector<int> m_userPreviousAllocSolution;

	// double getUserCurrentCost(int j, int k);

	void loadUserAndEnbsBdwTotalRequirements();

private:

	// problem instance
	const Instance *i_;

	// The currently used bandwith of eNB_i
	vector<double> m_enbUsedBandwidth;

	vector<vector<double>> m_uesCurrentCost;
	vector<vector<double>> m_enbsCurrentCost;
	vector<vector<double>> m_enbsDcCurrentCost;

	vector<int> m_userConnectionTable;
	vector<int> m_enbConnectionTable;
	vector<int> m_enbDcConnectionTable;

	vector<int> m_enbUserCount;

	double m_totalMaxBdw;
	double m_totalUserRequiredBdw;

	// vector<vector<int>> m_swapTable;

	vector<int> m_usersList;

	vector<int> m_eNBsList;
	vector<costUEPairVEC> m_costPerUserList;

	queue <int> m_userSwapQueue;

	vector< pair<pair<double, double>, double>> m_userCostStatistics; 

	uint16_t m_connectedUsersCounter;

	double m_solutionCost;

	bool m_noExistingSolution;

	void loadUserStatistics();

	void loadCostPerUserList();

	// Return the cost of user k connected to eNB_j
	double getUserStartingCost(int j, int k);

	double normalize(double val, double min, double max);

	double denormalize(double val, double min, double max);

	bool isBandwidthAvailable(const int enb, const int user);

	bool isBandwidthAvailableSP(const int enb, const int ue_u, const int ue_v);

	bool connectUserToEnb(const int i, const int k);

	bool disconnectUserFromEnb(const int i, const int k);
	
	void connectEnbToDc(const int i, const int s);

	void disconnectEnbFromDc(const int i, const int s);

	void connectEnbs(const int i, const int j);
 
	void disconnectEnbs(const int i, const int j);

	bool isCostAcceptable(const int enb, const int ue);

	bool checkDataStructure();

	void shuffleUsers();

	void shuffleEnbs();

	vector<int, int> getRandomEnbPair();

}; // class Solution	
} // namespace opt

#endif // #ifndef SOLUTION_H_