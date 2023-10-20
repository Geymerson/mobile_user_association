#include <iostream>
#include <unistd.h>
#include "Allocator.h"
#include <algorithm>
#include <iterator> 
#include <vector>
#include <deque>

using namespace std;

int main(int argc, char *argv[]) {

    string idx;
    if(argc == 2) {
        idx = argv[1];
    }

    string enbPosFileName = "./enbsPosDir/enbPos"+idx+".csv";
    string ueLogFileName = "./LOG/ueLog"+idx+".csv";
    string handoverFileName = "./LOG/handFreq"+idx+".csv";

    int handovers = -1;
    bool isFinalStop = false;
    Allocator allocator(enbPosFileName);

    allocator.readSplitPoints("./routes/splits.csv");
    allocator.loadRouteList("./routes/full_route_0_m");

    allocator.loadEnbTranmissionRatio("eTransmissionRatio.csv");
    allocator.setEnbsRadius(500);
    XY ueStartingPos = allocator.getRoute().at(0);
    allocator.setUePosition(ueStartingPos);
    allocator.setUeID(0);

    ofstream ueLogFile(ueLogFileName, ios::out);
    assert(ueLogFile.is_open());
    ueLogFile << "ue_id,ue_x,ue_y,alloc_enb,rsrq\n";
        
    cout << "enb_id,score,alloc,rsrq,hand_ctn\n";

    while(!isFinalStop) {

        allocator.computeEnbsMultiRouteScore();
        int currEnb = allocator.getCurrentConnectionID();
        int bestEnbID = allocator.getBestScore().first;

        if(bestEnbID != currEnb) {
            allocator.connectToEnb(bestEnbID);
            handovers++;
        }

        int alloc = 0;
        for(int enb = 0; enb < allocator.getNumberOfEnbs(); enb++) {
            XY enbPos = allocator.getEnbPosition(enb);
            XY uePos = allocator.getUePosition();
            if(allocator.getCurrentConnectionID() == enb) {
                ueLogFile << allocator.getUeID() << "," << uePos.first << "," << uePos.second
                    << "," << enb <<  "," << allocator.getRSRQ(uePos, enbPos, enb).first << endl; 
                alloc = 1;
            }
            
            cout << enb << "," << allocator.getEnbScore(enb) << "," << alloc 
                << "," << allocator.getRSRQ(uePos, enbPos, enb).first << "," << handovers <<endl;
            alloc = 0;
        }

        isFinalStop = allocator.isFinalDestination();
        allocator.nextUePosition();
    }

    allocator.saveHandoverLog(handoverFileName);
    ueLogFile.close();
    return 0;
}