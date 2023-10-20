#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

#include <queue> 
#include <vector>
#include <string>
#include <math.h>
#include <fstream>
#include <numeric>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <string.h>

#define PI 3.14159265

using namespace std;

typedef pair<double, double> XY;
typedef pair<double, double> DD;
typedef pair<int, vector<int>> SPLITS;
// typedef vector<vector<XY>> ROUTELIST;

class Allocator {

public:

	Allocator(const string &enbPosFileName);

    //Return the middle point of a route
	vector<XY> getRouteMiddlePoints();
	XY getIdealPoint();

	void setUePosition(XY pos);
	XY getUePosition();
	void nextUePosition();
	int getUeID();
	void setUeID(int id);

	//Reads a file containing a set of XY coordinates
	void loadRoute(string routeFileName);
	void loadRouteList(string baseRouteFileName);
	void setRoute(const vector<XY> &route);
	vector<XY> getRoute();

	void generatePathFromRoute();

	vector<XY> loadPositions(const string &posFileName);
	void loadEnbPositions(const string &posFileName);
	void loadEnbTranmissionRatio(const string &fileName);
	
	//Set an individual eNB position
	void setEnbPosition(int enb, XY pos);
	XY getEnbPosition(int enb);
	int getNumberOfEnbs();

	//Check if the points are collinear
	bool areCollinear(XY p1, XY p2, XY p3);

	// Checks is the user reached its final destination
	bool isFinalDestination();

	// Calculates the distance between p1 and p2
	DD getDistance(XY p1, XY p2);

	// Calculates the RSRQ between p1 and p2
	// DD getRSRQ(XY p1, XY p2);
	DD getRSRQ(XY p1, XY p2, int enbID);

	void updateVariableValues();

	void computeEnbsMultiRouteScore();
	void computeEnbsScore();// Compute the allocation score of each eNB
	int getDirectionFactor(int enb);
	double getDistRatio(int enb);
	double getEnbScore(int enb); // Get the score of an individual eNB
	DD getBestScore();

	void connectToEnb(int enb);
	void disconnectFromEnb();
	int getCurrentConnectionID();
	int getPreviousConnectionID();

	int getHandoverFrequency(int enb_i, int enb_j);

	void saveHandoverLog(const string &fileName);

	void setEnbsRadius(double enbRadius);
	double getEnbsRadius();

	//Finds the middle point of the roue
	vector<XY> getRouteAt(int idx);
	XY findRouteMiddlePoint(vector<XY> route);
	XY findMultiRouteIdealPoint();

	bool isMultiRoutePoint();
	void readSplitPoints(const string &basename);
	
private:
	int m_numberOfEnbs; // The number of base stations in the network
	double m_eNbRadius; // eNB max signal reach in meters
	int m_enbConnectionID; // The number of the eNB in which the user is connected
	int m_previousEnbConnectionID; // The ID of the previously connected eNB
	int m_ueID; // The current user's ID
	int m_currentNumberOfPossibleRoutes;
	int m_currentStep;
	int m_splitPointIdx;
	vector<vector<int>> m_handoverFrequency; //The handover frequency between the eNBs
	XY m_uePosition; // UE current position
	XY m_idealPoint; // The ideal point for the current route
	vector<XY> m_middlePoints; //The list of all the middle points of the possiblle routes to the UE's destination
	vector<XY> m_route; // A sequence of XY coordinates describing a route
	vector<vector<XY>> m_routes;
	vector<SPLITS> m_splitPoints;
	// vector<XY> m_altRoute1;
	// vector<XY> m_amtRoute2;
	deque<XY> m_ueCurrentPath; //The user's current position (at m_ueCurrentPath[0]) and the remaining path until final destination
	vector<XY> m_eNBsPositions; // A list of eNBs positions
	vector<DD> m_distance; //m_distance.first = current distance; m_distance.second = previous distance
	vector<DD> m_RSRQ; //m_RSRQ.first = current RSRQ; m_RSRQ.second = previous RSRQ
	vector<double> m_transmitingRatio; // The percentual RSRQ value witch the user will be able to receive
	vector<double> m_eNBscore; // The rank of the best eNBs for a UE to be allocated
	DD m_bestScore;

	void findRouteMiddlePoints();
	void findIdealPoint();
}; // Class Allocator


#endif // #ifndef INSTANCE_H_