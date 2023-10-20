#ifndef MOBILITY_H_
#define MOBILITY_H_

#include <map>
#include <iostream>
#include "Solution.h"
#include "Instance.h"

using namespace std;
using namespace opt;

typedef pair <XY, int> XYT;

class Mobility {

public:

    Mobility();

	vector<XY> loadPositions(const string &PosFile, bool isUePosition);

    // UE's methods
    int getUeID(int idx);
    void loadUesPosition();
	XYT getUePosition(int k);
	void setUePosition(int k, XY pos);
	void nextUePosition();
    int getNumberOfUes();

    // eNBs methods
	XY getEnbPosition(int i);
	void setEnbPosition(int i, XY pos);
	void loadEnbsPosition(const string &enbPosFile);
    int getNumberOfEnbs();

	// void loadMobilityModel(const string &mobilityFileName);
	// void mobilityModelUpdateUsersPosition();
	// void loadUeRoute(const string &routeFileName);

    bool isEndOfRoute();

	//Compute distance between to positions
	double get_distance(XY p1, XY p2);

private:
    int m_numberOfues;
    int m_numberOfEnbs;
    int m_currentTimeStep;
    bool m_endOfRoute;
    vector<int> m_ueIDList;
	vector<XY> m_enbsPosition;
    vector<XYT> m_ueCurrentPosition;
    vector<string> m_uesPositionList;
	map <int, deque<XYT>> m_uesPositions;

    void loadUesPositionListNames();
};


#endif // #ifndef MOBILITY_H_