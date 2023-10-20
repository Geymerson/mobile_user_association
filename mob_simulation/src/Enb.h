#ifndef ENB_H_
#define ENB_H_

#include <vector>
#include <string>
#include <assert.h>
#include <algorithm>

using namespace std;

class Enb {

public:
	Enb(uint32_t id, uint32_t maxBandwidth);

	// return the number of users
	int numberOfconnectedUsers() const { return m_numberOfConnectedUser; }

	double getAvailableBandwidth();
	double getHandoverAverage();
	int getConnectedDC_ID();
	
private:

	vector<int> m_connectedUsersID; // A list containning the IDs of the connected users
	vector<int> m_users;

	double m_availableBandwidth;
	double m_handoverAverage;

	int m_connectedDcId;
	int m_maxBandwidth;
	int m_numberOfConnectedUser; // number of users
	int m_id;					 // number of enbs

	// @return: true if the eNB is connected to a data center, false otherwise	
	bool m_isConnected;


}; // class Enb


#endif // #ifndef INSTANCE_H_
