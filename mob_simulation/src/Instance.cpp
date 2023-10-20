#include "Instance.h"

using namespace std;

namespace opt
{
	Instance::~Instance() {
		
	}

	Instance::Instance(string file_name)
	{

		ifstream file(file_name);

		if(!file.is_open()) {
			// cout << color::fore::red;
			cout << "Could not open the instance file:\n";
			cout << file_name << endl; 
			// cout << color::console;
			exit(1);
		}

		// cout << "Reading the number of UEs, eNBs, data centers, dataset type, and rmax\n";
		file >> ue_ >> enb_ >> dc_ >> m_instanceType >> m_RMax;

		// cout << "Reading instance's RMax and base stations max capacity\n";
		enb_max_bandwidth_ = vector<double>(enb_);
		for(int i = 0; i < enb_; i++) {
			file >> enb_max_bandwidth_[i];
			// enb_max_bandwidth_[i] = 3000;
			// cout << enb_max_bandwidth_[i] << ' ';
		}
		// cout << m_totalMaxBdw << '\n';

		dist_ = vector<vector<double>>(enb_, vector<double>(ue_));

		m_prevDist = vector<vector<double>>(enb_, vector<double>(ue_));

		rssi_ = vector<vector<double>>(enb_, vector<double>(ue_));
		cost_ = vector<vector<double>>(enb_, vector<double>(dc_));
		ue_req_bandwidth_ = vector<vector<double>>(enb_, vector<double>(ue_));
		m_handoverFreqBetweenEnbs = vector<vector<double>>(enb_, vector<double>(enb_));

		m_uesPositions = vector<vector<XY>> (ue_, vector<XY>());
		m_ueIDList = vector<II>(ue_);

		enb_bandwidth_ = vector<double>(enb_);
		m_enbAvgHandoverFreq = vector<double>(enb_);
		m_enbsPosition = vector<XY>(enb_);
		m_eNBRadius = 300;

		// cout << "Reading users' bandwidth requirements\n";
		//file >> user_bdw_req >> enb_max_bdw;
		for(int i = 0; i < enb_; i++) {
			for(int k = 0; k < ue_; k++) {
				file >> ue_req_bandwidth_[i][k];
			}
		}
		// cout << m_totalUserRequiredBdw << '\n';

		/**
		 * 
		 * INPUT PARAMETERS (COST AND HANDOVER AVG)
		 * 
		 **/

		//Reading average handover frequency
		// cout << "Reading average handover frequency between eNBs\n";
		for (int i = 0; i < enb_; i++) {
			m_enbAvgHandoverFreq[i] = 0;
			for (int j = 0; j < enb_; j++) {
				file >> m_handoverFreqBetweenEnbs[i][j];
				m_enbAvgHandoverFreq[i] += m_handoverFreqBetweenEnbs[i][j];
			}
			m_enbAvgHandoverFreq[i] /= (enb_ - 1);
		}

		//Reading the available bandwidth in each eNB
		// cout << "Reading the bandwidth occupancy in each eNB\n";
		for (int i = 0; i < enb_; i++) {
			file >> enb_bandwidth_[i];
		}

		//Reading eNBs and data centers communication cost
		// cout << "Reading eNBs and data centers communication cost\n";
		for (int i = 0; i < enb_; i++) {
			for (int s = 0; s < dc_; s++) {
				file >> cost_[i][s];
			}
		}

		//Reading the distance between UEs and eNBs
		// cout << "Reading the distance between UEs and eNBs\n";
		for (int i = 0; i < enb_; i++) {
			for (int k = 0; k < ue_; k++) {
				file >> dist_[i][k];
				m_prevDist[i][k] = dist_[i][k];
			}
		}

		//Reading the RSSI value between users and eNBs
		// cout << "Reading the RSSI between UEs and eNBs\n";
		for (int i = 0; i < enb_; i++) {
			for (int k = 0; k < ue_; k++) {
				file >> rssi_[i][k];
			}
		}
		loadEnbsTransmissionRatio("eTransmissionRatio.csv");
	}

	double Instance::getEnbTransmissionRatio(int enb) {
		return m_transmissionRatio.at(enb);
	}

	 // Return the cost to connect user k to eNB_j
    double Instance::getUserCost(int j, int k) {
		// double cost = rssi_[j][k] + m_enbAvgHandoverFreq[j];
		double cost = rssi_[j][k] + m_enbAvgHandoverFreq[j];
		return cost;
    }

	void Instance::loadEnbsTransmissionRatio(const string &fileName) {
		ifstream file(fileName);

		if(!file.is_open()) {
			cout << "Could not open the instance file:\n";
			cout << fileName << endl; 
			exit(1);
		}

		string line;
		getline(file, line); // Skip first line

		while (!file.eof()) {
			getline(file, line);
			if (line == "") { break; } //Reached end of file
			// cout << line << endl;
			m_transmissionRatio.push_back(atof(line.c_str()));
    	}
	}

	void Instance::setEnbMaxRadius(double radius) {
		if(radius > 0) {
			m_eNBRadius = radius;
		}
	}

	double Instance::getEnbMaxRadius() {
		return m_eNBRadius;
	}

	double Instance::distToRSRP(double dist) {
		if(dist >= m_eNBRadius) {
			return -140;
		}
		return -44 -96*dist/m_eNBRadius;
	}

	void Instance::setDistances(const vector<vector<double>> &dist) {
		m_prevDist = dist_;
		dist_ = dist;
	}

	void Instance::setRssi(const vector<vector<double>> &rssi) {
		rssi_ = rssi;
	}

	void Instance::setRsrq(const vector<vector<double>> &rsrq) {
		rsrq_ = rsrq;
	}

	void Instance::computeHandoverAvg(int enbID) {
		double sum = 0;
		for(int j = 0; j < enb_; j++) {
			if(j != enbID) {
				sum += m_handoverFreqBetweenEnbs[enbID][j];
			}
		}
		m_enbAvgHandoverFreq[enbID] = sum/(enb_ - 1);
	}

	void Instance::instanceUpdateData() {

	}

	void Instance::print_instance() {
		//Printing average handover frequency
		cout << "Printing average handover frequency between eNBs\n";
		for (int i = 0; i < enb_; i++) {			
			for (int j = 0; j < enb_; j++) {
				cout << m_handoverFreqBetweenEnbs[i][j] << ' ';
			}
			cout << "\n";
			
		}

		//Printing the available bandwidth in each eNB
		cout << "Printing the handover average in each eNB\n";
		for (int i = 0; i < enb_; i++) {
			cout << m_enbAvgHandoverFreq[i] << ' ';
		}
		cout << "\n";

		//Printing the available bandwidth in each eNB
		cout << "Printing the available bandwidth in each eNB\n";
		for (int i = 0; i < enb_; i++)
		{
			cout << enb_bandwidth_[i] << ' ';
		}
		cout << "\n";

		//Printing eNBs and data centers communication cost
		cout << "Printing eNBs and data centers communication cost\n";
		for (int i = 0; i < enb_; i++) {
			for (int s = 0; s < dc_; s++) {
				cout << cost_[i][s] << ' ';
			}
			cout << '\n';
		}

		//Printing the distance between UEs and eNBs
		cout << "Printing the distance between UEs and eNBs\n";
		for (int i = 0; i < enb_; i++) {
			for (int k = 0; k < ue_; k++) {
				cout << dist_[i][k] << ' ';
			}
			cout << '\n';
		}

		//Reading the RSSI value between users and eNBs
		cout << "Printing the RSSI between UEs and eNBs\n";
		for (int i = 0; i < enb_; i++) {
			for (int k = 0; k < ue_; k++) {
				cout << rssi_[i][k] << ' ';
			}
			cout << '\n';
		}
	}
} // namespace opt