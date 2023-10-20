#ifndef INSTANCE_H_
#define INSTANCE_H_

#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <random>

using namespace std;

typedef pair<double, double> XY;
typedef pair<int,int> II;

namespace opt {

extern mt19937 generator;

class Instance {

public:

	Instance(string file_name);

	~Instance();

	// return the number of users
	int users() const { return ue_; }

	// return the number of vertices
	int data_centers() const { return dc_; }
	
	// return the number of vertices
	int enbs() const { return enb_; }
	
	// return the distance between eNB_i and UE_k
	double distance(const int i, const int k)  const { return dist_[i][k]; }
	double prevDistance(const int i, const int k)  const { return m_prevDist[i][k]; }
	void setDistances(const vector<vector<double>> &dist);
	
	// return the rssi between eNB_i and UE_k
	double rssi(const int i, const int k)  const { return rssi_[i][k]; }
	void setRssi(const vector<vector<double>> &rssi);

	// return the rssi between eNB_i and UE_k
	double rsrq(const int i, const int k)  const { return rsrq_[i][k]; }
	void setRsrq(const vector<vector<double>> &rsrq);
	
	// return the communication cost between eNB_i and data center s
	double cost(const int i, const int s)  const { return cost_[i][s]; }

	// return the handover frequency between eNB_i and eNB_j
	double handover_avg(const int i, const int j)  const { return m_handoverFreqBetweenEnbs[i][j]; }
	
	// return the average handover of eNB_i
	double handover_i(const int i)  const { return m_enbAvgHandoverFreq[i]; }
	
	// return the currently available bandwidth in eNB_i
	double enbUsedBandwidth(const int i)  const { return enb_bandwidth_[i]; }

	vector<double> enbUsedBandwidth() const { return enb_bandwidth_; }

	// return the currently available bandwidth in eNB_i
	double enbMaxBandwidth(const int i)  const { return enb_max_bandwidth_[i]; }

	// return the minimum bandwidth required by user k
	double userRequiredBandwidth(const int i, const int k)  const { return ue_req_bandwidth_[i][k]; }

	int instanceType() const {return m_instanceType; }

	double getUserCost(int j, int k);

	// Print print instance
	void print_instance();

	//The max connection radius provided by the eNB in meters
	void setEnbMaxRadius(double radius);
	double getEnbMaxRadius();
	double getEnbTransmissionRatio(int enb);

	//Return the RSRP value in dBm for at the distance dist (in meters)
	double distToRSRP(double dist);

	void instanceUpdateData();
	void computeHandoverAvg(int enbID);

private:

	vector<double> m_enbAvgHandoverFreq; // Handover average of base stations
	vector<double> enb_bandwidth_; // Current bandwidth available in each base station
	vector<double> enb_max_bandwidth_; // Max capacity bandwidth of each base station
	vector< vector<double> > ue_req_bandwidth_; // Minimum bandwidth requirement of each user

	vector< vector<double> > m_handoverFreqBetweenEnbs; // A matrix of average handover between two enbs
	vector< vector<double> > cost_; // A matrix of costs between enbs and data centers
	vector< vector<double> > rssi_; // A matrix of rssi between users and enbs
	vector< vector<double> > rsrq_; // A matrix of rssi between users and enbs
	vector< vector<double> > dist_; // A matrix of distance between users and enb

	vector< vector<double> > m_prevDist; // A matrix of previous distance between users and enb

	vector<vector<XY>> m_uesPositions;
	vector<XY> m_enbsPosition;
	vector<string> m_uesPositionList;
	vector<II> m_ueIDList; // A list containing the UE ID and its initial timestep

	double m_RMax;
	int m_instanceType;
	int ue_; // number of users
	int enb_; // number of enbs	
	int dc_; // number of data centers
	int user_bdw_req; //User's bandwidth requirements
	double enb_max_bdw; //eNB's bandwidth max capacity

	vector<double> m_transmissionRatio;

	double m_eNBRadius;

	void loadEnbsTransmissionRatio(const string &fileName);
}; // class Instance

} // namespace opt

#endif // #ifndef INSTANCE_H_
