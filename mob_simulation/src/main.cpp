#include "Instance.h"
#include "Mobility.h"
#include "Solution.h"
#include "Solver.h"

// #include "InitError.h"
#include "ArgPack.h"
#include "bossa_timer.h"

#include <assert.h>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <ctime>
#include <math.h>

using namespace std;
using namespace opt;

#define MAXRADIUS 500

namespace opt {

	// Mersenne Twister 19937 generator
	mt19937 generator;

} // namespace opt

double normalize(double v, double min, double max) {
	return (v - min)/(max - min);
}

pair<double, double> getRSRQ(double dist, double loss) {
	double maxRSRQ = 19.5;
	double minRSRQ = 3;
	double ret;

	// ret = (dist/MAXRADIUS)*(minRSRQ-maxRSRQ) + maxRSRQ;
	ret = (dist/MAXRADIUS)*(maxRSRQ-minRSRQ) + minRSRQ;
	ret *= loss;
	double normRet = normalize(ret, minRSRQ, maxRSRQ);

	// if(normRet > 1) {
	// 	normRet = 1;
	// }
	// else if(normRet < 0){
	// 	normRet = 0;
	// }
	return make_pair(ret, normRet);
}


void saveHandoverLog(vector<vector<double>> handMatrix, string fileName) {
	ofstream handFile;
    handFile.open(fileName, ios::out);
    assert(handFile.is_open());
	int enbs = handMatrix.at(0).size();
    string ch;

    for(int i = 0; i < enbs; i++) {
        ch = i == ( enbs - 1) ? '\n':',';
        handFile << "enb_" + to_string(i) << ch;
    }

    for(int i = 0; i <  enbs; i++) {
        for(int j = 0; j <  enbs; j++){
            ch = j == (enbs - 1) ? '\n':',';
            handFile << handMatrix[i][j] << ch;
        }
    }
    handFile.close();
}

int main(int argc, char *argv[]) {



	ArgPack single_ap(argc, argv);
	string idx = ArgPack::ap().inputName;
	string handoverFileName = "./LOG/handFreq"+idx+".csv";
	string enbPosFileName = "./enbsPosDir/enbPos"+idx+".csv";

	Instance instance("./instances/ds1.in");
	// instance.loadEnbsTransmissionRatio("eTransmissionRatio.csv");
	instance.setEnbMaxRadius(MAXRADIUS);

	Mobility mob;
	mob.loadEnbsPosition(enbPosFileName);
	mob.loadUesPosition();
	mob.nextUePosition(); // Set all the UE's at the initial position
	XYT uePos; XY enbPos;

	vector<int> previousAllocation =
		vector<int>(mob.getNumberOfUes(), 0);

	vector<vector<double>> distValues = 
		vector<vector<double>>(mob.getNumberOfEnbs(), vector<double>(mob.getNumberOfUes(), -1));

	vector<vector<double>> rsrqValues = 
		vector<vector<double>>(mob.getNumberOfEnbs(), vector<double>(mob.getNumberOfUes(), -1));

	vector<vector<double>> handValues = 
		vector<vector<double>>(mob.getNumberOfEnbs(), vector<double>(mob.getNumberOfEnbs(), 0));	
	
	double dist, loss;
	int currentEnb = -1, prevEnb = -1, handovers = 0;
	pair<double, double> rsrq;

	string ueLogFileName = "./LOG/ueLog"+idx+".csv";
    ofstream ueLogFile(ueLogFileName, ios::out);
    assert(ueLogFile.is_open());
    ueLogFile << "ue_id,ue_x,ue_y,alloc_enb,rsrq\n";
	cout << "enb_id,cost,alloc,rsrq,hand_count\n";

	int n = 0;
	while(!mob.isEndOfRoute()) {
		for(int ue = 0; ue < mob.getNumberOfUes(); ue++) {
			uePos = mob.getUePosition(ue);
			int currStep = uePos.second;
			if(currStep == -1) continue;
			int alloc = 0;
			for(int enb = 0; enb < mob.getNumberOfEnbs(); enb++) {
				enbPos = mob.getEnbPosition(enb);
				dist = mob.get_distance(uePos.first, enbPos);
				loss = 1 + (1 - instance.getEnbTransmissionRatio(enb));
				rsrq = getRSRQ(dist, loss);

				distValues[enb][ue] =
					normalize(dist, 0, instance.getEnbMaxRadius());

				rsrqValues[enb][ue] = rsrq.second;
				
				if(enb == currentEnb) {
					ueLogFile << mob.getUeID(ue) << "," << uePos.first.first << ","
						<< uePos.first.second << "," << currentEnb <<","<< -rsrq.first << endl;
					alloc = 1;
				}

				if(n != 0) {
					cout << enb << "," << instance.getUserCost(enb, ue) << "," << alloc
						<< "," << -rsrq.first <<","<< handovers << endl;		
				}
				alloc = 0;
			}
		}

		instance.setDistances(distValues);
		// instance.setRssi(rsrqValues);
		instance.setRsrq(rsrqValues);

		Solver solver(&instance);
		Solution solution = solver.solve();

		currentEnb = solution.getUserConnectionTable().at(0);

		if(currentEnb != prevEnb && prevEnb != -1) {
			handValues[currentEnb][prevEnb]++;
			handValues[prevEnb][currentEnb]++;
	
			if(n != 0) {
				handovers++;
			}
		}

		prevEnb = currentEnb;
		mob.nextUePosition();
		n++;
	}

	saveHandoverLog(handValues, handoverFileName);
} // int main(int argc, char *argv[])