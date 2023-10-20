#include "Allocator.h"

Allocator::Allocator(const string &enbPosFileName) {

	loadEnbPositions(enbPosFileName);
	m_numberOfEnbs = m_eNBsPositions.size();
	m_eNBscore = vector<double> (m_numberOfEnbs, 0);
	m_distance = vector<DD> (m_numberOfEnbs, make_pair(INT16_MAX, INT16_MAX));
	m_RSRQ = vector<DD> (m_numberOfEnbs, make_pair(INT16_MAX, INT16_MAX));
	m_handoverFrequency = vector< vector<int>>(m_numberOfEnbs, vector<int>(m_numberOfEnbs, 0));
	m_eNbRadius = 500;
	m_enbConnectionID = -1;
	m_previousEnbConnectionID = -1;
	m_transmitingRatio = vector<double>(m_numberOfEnbs, 1);
	m_currentStep = -1;
	m_splitPointIdx = 0;
	m_splitPoints = vector<SPLITS>();
}

int Allocator::getHandoverFrequency(int enb_i, int enb_j) {
	if(enb_i >= 0 && enb_i < m_numberOfEnbs && enb_j >= 0 && enb_j < m_numberOfEnbs) {
		return m_handoverFrequency[enb_i][enb_j];
	}
	cout << "Invalid base station ID\n";
	return -1;
}

void Allocator::readSplitPoints(const string &splitsFileName) {
	ifstream inputFile;
    inputFile.open(splitsFileName, ios::out);
    assert(inputFile.is_open());

	string timestamp, routes;
	string skipper;
	getline(inputFile, skipper); //Ignore column names

	vector<int> routeIdx;
	while (!inputFile.eof()) {
		getline(inputFile, timestamp,',');
		if(timestamp == "") { break; } //Reached end of file
		getline(inputFile, routes);
		// cout << timestamp << " " << routes << endl;

		//The the IDs of the routes involved in the split
		for(int i = 0; i < routes.size(); i++) {
			if(routes.at(i) != '-') {
				int idx = routes.at(i) - 48;
				routeIdx.push_back(idx);
				// cout << "route " << idx << " is part of a split" << endl;
			}
		}
		m_splitPoints.push_back(make_pair(atoi(timestamp.c_str()), routeIdx));
	}
	// cout << "Number of splits " << m_splitPoints.size() << endl;
	inputFile.close();
}

void Allocator::saveHandoverLog(const string &fileName){
	ofstream handFile;
    handFile.open(fileName, ios::out);
    assert(handFile.is_open());

    string ch;
    for(int i = 0; i < m_numberOfEnbs; i++) {
        ch = i == ( m_numberOfEnbs - 1) ? '\n':',';
        handFile << "enb_" + to_string(i) << ch;
    }

    for(int i = 0; i <  m_numberOfEnbs; i++) {
        for(int j = 0; j <  m_numberOfEnbs; j++){
            ch = j == (m_numberOfEnbs - 1) ? '\n':',';
            handFile << getHandoverFrequency(i, j) << ch;
        }
    }
    handFile.close();
}

void Allocator::connectToEnb(int enb) {
	if(enb >= 0 && enb < m_numberOfEnbs) {
		m_previousEnbConnectionID = m_enbConnectionID;
		m_enbConnectionID = enb;

		if(m_previousEnbConnectionID != -1 && m_previousEnbConnectionID != enb) {
			m_handoverFrequency[m_previousEnbConnectionID][m_enbConnectionID] += 1;
			m_handoverFrequency[m_enbConnectionID][m_previousEnbConnectionID] += 1;
		}
		return;
	}
	cout << "Error: invalid eNB" << endl;
}

void Allocator::disconnectFromEnb() {
	m_previousEnbConnectionID = m_enbConnectionID;
	m_enbConnectionID = -1;
}

int Allocator::getCurrentConnectionID() {
	return m_enbConnectionID;
}

int Allocator::getPreviousConnectionID() {
	return m_previousEnbConnectionID;
}

int Allocator::getNumberOfEnbs() {
	return m_numberOfEnbs;
}

void Allocator::setEnbPosition(int enb, XY pos) {
	m_eNBsPositions.at(enb) = pos;
}

XY Allocator::getEnbPosition(int enb) {
	return m_eNBsPositions.at(enb);
}

void Allocator::loadEnbPositions(const string &enbPosFileName) {
	m_eNBsPositions = loadPositions(enbPosFileName);
}

int Allocator::getUeID(){
	return m_ueID;
}

void Allocator::setUeID(int id) {
	m_ueID = id;
}

XY Allocator::getUePosition() {
	return m_uePosition;
}

void Allocator::setUePosition(XY pos) {
	m_uePosition = pos;
	// Update distance and RSRQ value for each eNB
	updateVariableValues();
}

void Allocator::nextUePosition() {
	m_ueCurrentPath.pop_front();
	setUePosition(m_ueCurrentPath.front());
	m_currentStep++;
}

void Allocator::updateVariableValues() {
	XY uePos = getUePosition();
	for(int enb = 0; enb < m_numberOfEnbs; enb++) {
		double dist = getDistance(uePos, m_eNBsPositions.at(enb)).second;
		m_distance[enb].second = m_distance[enb].first;
		m_distance[enb].first = dist;
		double rsrq = getRSRQ(uePos, m_eNBsPositions.at(enb), enb).second;
		m_RSRQ[enb].second = m_RSRQ[enb].first;
		m_RSRQ[enb].first = rsrq;
	}
}

DD Allocator::getDistance(XY p1, XY p2) {
	double dx = p1.first-p2.first;
	double dy = p1.second-p2.second;
	double dist =  sqrt(dx*dx + dy*dy);
	double normalized = dist > m_eNbRadius ? 1: dist/m_eNbRadius;
	return make_pair(dist, normalized);
}

DD Allocator::getRSRQ(XY p1, XY p2, int enbID) {

	double maxRSRQ = 19.5;
	double minRSRQ = 3;
	double loss = 1 + (1 - m_transmitingRatio.at(enbID));
	double dist = getDistance(p1, p2).first;

	// double RSRQ = (-16.5*dist)/m_eNbRadius - 3;
	double RSRQ = (dist/m_eNbRadius)*(maxRSRQ-minRSRQ) + minRSRQ;
	RSRQ *= loss;

	// double normalized = (RSRQ + 19.5)/(16.5);
	double normalized = (RSRQ - maxRSRQ)/(minRSRQ - maxRSRQ);
	
	if(normalized > 1) {
		normalized = 1;
	}
	
	return make_pair(-RSRQ, normalized);
}

int Allocator::getDirectionFactor(int enb) {
	double deltaDist = m_distance[enb].second - m_distance[enb].first;
	double deltaRSRQ = m_RSRQ[enb].second - m_RSRQ[enb].first;

	if(deltaDist > 0 && deltaRSRQ > 0) {
		return 1;
	}
	return 0;
}

double Allocator::getDistRatio(int enb) {
	double midPsum = 0;
	double idealPsum = 0.1;
	XY enbPos = m_eNBsPositions[enb];

	for(int p = 0; p < m_middlePoints.size(); p++) {
		midPsum += getDistance(m_middlePoints.at(p), enbPos).second; //Not normalized distance
		idealPsum += getDistance(m_middlePoints.at(p), m_idealPoint).second;
	}

	return midPsum/idealPsum;
}

void Allocator::loadEnbTranmissionRatio(const string &fileName){
	ifstream inputFile(fileName);

	if (!inputFile.is_open()) {
		cout << "Could not open the instance file:\n";
		cout << fileName << endl;
		exit(1);
	}

	string t_ratio;
	string skipper;
	getline(inputFile, skipper); //Ignore column names

	int enb = 0;
	while (!inputFile.eof()) {
		getline(inputFile, t_ratio);
		if(t_ratio == "") { break; } //Reached end of file
		m_transmitingRatio.at(enb) = atof(t_ratio.c_str());
		enb++;
	}
	
	inputFile.close();
}

vector<XY> Allocator::loadPositions(const string &posFileName){
	ifstream posFile(posFileName);
	vector<XY> positionList;

	if (!posFile.is_open()) {
		cout << "Could not open the instance file:\n";
		cout << posFileName << endl;
		exit(1);
	}

	int ue_id;
	double px, py;
	string id, x, y;
	string skipper;
	vector<int> updateList;
	getline(posFile, skipper); //Ignore column names

	while (!posFile.eof()) {
		getline(posFile, id, ',');
		if(id == "") { break; } //Reached end of file
		ue_id = atoi(id.c_str());
		getline(posFile, x, ',');
		getline(posFile, y);
		px = atof(x.c_str()); 
		py = atof(y.c_str());
		positionList.push_back(make_pair(px, py));
	}

	posFile.close();
	return positionList;
}

void Allocator::loadRoute(string routeFileName) {
	m_route = loadPositions(routeFileName);
	generatePathFromRoute();
}

//New
void Allocator::loadRouteList(string baseRouteFileName) {
	
	for(int i = 0; i < 5; i++) {
		string routeFileName = baseRouteFileName + to_string(i) + ".csv";
		m_routes.push_back(loadPositions(routeFileName));
		// cout << "route m_" << i << " size " << m_routes.at(i).size() << endl;
	}
	m_route = m_routes.at(0);
	generatePathFromRoute();

	// for(int i = 0; i < 300; i++) {
	// 	XY m0CurrP = m_routes.at(0).at(i);
	// 	XY m1CurrP = m_routes.at(1).at(i);
	// 	XY m2CurrP = m_routes.at(2).at(i);

	// 	if(m0CurrP.first != m1CurrP.first && m0CurrP.second != m1CurrP.second) {
	// 		// cout << i << " " << m0CurrP.first << endl;
	// 	}
	// }
}

void Allocator::generatePathFromRoute() {
	m_ueCurrentPath.clear();
	for(int i = 0; i < m_route.size(); i++) {
		m_ueCurrentPath.push_back(m_route.at(i));
	}
}

void Allocator::computeEnbsMultiRouteScore() {
	// cout << "here "<< m_splitPointIdx << " " << m_splitPoints.size()<< "\n";
	int splitPoint;

	if(m_splitPointIdx < m_splitPoints.size()) {
		splitPoint = m_splitPoints.at(m_splitPointIdx).first;
		// cout << "Split point idx " << m_splitPointIdx << " current step is " << m_currentStep <<" and m_splitPointIdx " << splitPoint << endl;
	}
	// cout << "And here\n";

	if(m_currentStep != splitPoint) {
		findRouteMiddlePoints();
	}
	else {
		// cout << "Split point\n";
		// cout << "Computing split at " << m_currentStep << " current step is " << m_currentStep << endl;
		m_idealPoint = findMultiRouteIdealPoint();
		// cout << "After multiroute computation\n";
	}

	m_bestScore.first = -1;
	m_bestScore.second = 0;

	for(int enb = 0; enb < m_numberOfEnbs; enb++) {
		XY enbPos = m_eNBsPositions.at(enb);
		double distRatio = 1/getDistRatio(enb) > 1 ? 1 : 1/getDistRatio(enb); 
		m_eNBscore[enb] = distRatio + getDirectionFactor(enb) + getRSRQ(getUePosition(), enbPos, enb).second;
		if(m_eNBscore[enb] > m_bestScore.second){
			m_bestScore.first = enb;
			m_bestScore.second = m_eNBscore[enb];
		}
	}
}

void Allocator::computeEnbsScore() {

	findRouteMiddlePoints();
	
	m_bestScore.first = -1;
	m_bestScore.second = 0;

	for(int enb = 0; enb < m_numberOfEnbs; enb++) {
		XY enbPos = m_eNBsPositions.at(enb);
		double distRatio = 1/getDistRatio(enb) > 1 ? 1 : 1/getDistRatio(enb); 
		m_eNBscore[enb] = distRatio + getDirectionFactor(enb) + getRSRQ(getUePosition(), enbPos, enb).second;
		if(m_eNBscore[enb] > m_bestScore.second){
			m_bestScore.first = enb;
			m_bestScore.second = m_eNBscore[enb];
		}
	}
}

double Allocator::getEnbScore(int enb) {
	return m_eNBscore.at(enb);
}

DD Allocator::getBestScore() {
	return m_bestScore;
}

bool Allocator::isMultiRoutePoint() {
	if (m_currentStep == 0) {
		return true;
	}
	return false;
}

vector<XY> Allocator::getRoute() {
	return m_route;
}

void Allocator::setRoute(const vector<XY> &route) {
	m_route = route;
	generatePathFromRoute();
}

vector<XY> Allocator::getRouteMiddlePoints() {
	return m_middlePoints;
}

void Allocator::findIdealPoint() {
	double idealX = 0, idealY = 0;
	for (int i = 0; i < m_middlePoints.size(); i++){
		idealX += m_middlePoints.at(i).first;
		idealY += m_middlePoints.at(i).second;
	}
	idealX /= m_middlePoints.size();
	idealY /= m_middlePoints.size();
	m_idealPoint = make_pair(idealX, idealY);
	// cout << "Single route ideal point " << m_idealPoint.first << " "
	// 		<< m_idealPoint.second << endl;
	return;
}

XY Allocator::getIdealPoint() {
	return m_idealPoint;
}

vector<XY> Allocator::getRouteAt(int idx) {
	if(!m_routes.empty() && idx < m_routes.size()) {
		return m_routes.at(idx);
	}
}

XY Allocator::findMultiRouteIdealPoint() {
	double sumX = 0, sumY = 0;
	XY idealPoint;

	vector<int> routesIndexes = m_splitPoints.at(m_splitPointIdx).second;

	for(int i = 0; i < routesIndexes.size(); i++) {
		int r = routesIndexes.at(i);
		// cout << r <<"\nBlah 01\n";
		vector<XY> route = getRouteAt(r);
		// cout << "Blah 02\n";
		XY mid = findRouteMiddlePoint(route);
		sumX += mid.first;
		sumY += mid.second;
	}

	idealPoint.first = sumX/m_currentNumberOfPossibleRoutes;
	idealPoint.second = sumY/m_currentNumberOfPossibleRoutes;
	m_splitPointIdx++;
	return idealPoint;
}

XY Allocator::findRouteMiddlePoint(vector<XY> route) {

	int lowerIdx = 0;
	double midX, midY;
	int numberOfPoints = 0;
	XY middlePoint;

	if(route.empty()) {
		cout << "No route found\n";
		middlePoint.first = -1;
		middlePoint.second = -1;
		return middlePoint;
	}
	else if(route.size() <= 3) {
		int upperIdx = route.size() - 1;
		middlePoint.first = (route.at(lowerIdx).first + route.at(upperIdx).first)/2;
		middlePoint.second = (route.at(lowerIdx).second + route.at(upperIdx).second)/2; 
	}
	else {

		for(int i = 0; i < route.size() - 2; i++) {
			if(!areCollinear(route.at(i), route.at(i+1), route.at(i+2))) {
				midX = (route.at(lowerIdx).first + route.at(i).first)/2;
				midY = (route.at(lowerIdx).second + route.at(i).second)/2;
				middlePoint.first += midX;
				middlePoint.second += midY;
				lowerIdx = i+1;
				numberOfPoints++;
			}
		}
		
		middlePoint.first += (route.at(lowerIdx).first + route.at(route.size() -1).first)/2;
		middlePoint.second += (route.at(lowerIdx).second + route.at(route.size() -1).second)/2;

	}//End if-else

	middlePoint.first = middlePoint.first/(numberOfPoints+1);
	middlePoint.second = middlePoint.second/(numberOfPoints+1);
	return middlePoint;
}

void Allocator::findRouteMiddlePoints() {

	int lowerIdx = 0;
	double midX, midY;
	m_middlePoints.clear();
	if(m_ueCurrentPath.empty()) {
		cout << "No route found\n";
		return;
	}
	else if(m_ueCurrentPath.size() <= 3) {
		int upperIdx = m_ueCurrentPath.size() - 1;
		midX = (m_ueCurrentPath.at(lowerIdx).first + m_ueCurrentPath.at(upperIdx).first)/2;
		midY = (m_ueCurrentPath.at(lowerIdx).second + m_ueCurrentPath.at(upperIdx).second)/2;
		m_middlePoints.push_back(make_pair(midX, midY));
	}
	else {

		for(int i = 0; i < m_ueCurrentPath.size() - 2; i++) {
			if(!areCollinear(m_ueCurrentPath.at(i), m_ueCurrentPath.at(i+1), m_ueCurrentPath.at(i+2))) {
				midX = (m_ueCurrentPath.at(lowerIdx).first + m_ueCurrentPath.at(i).first)/2;
				midY = (m_ueCurrentPath.at(lowerIdx).second + m_ueCurrentPath.at(i).second)/2;
				m_middlePoints.push_back(make_pair(midX, midY));
				lowerIdx = i+1;
			}
		}
		
		midX = (m_ueCurrentPath.at(lowerIdx).first + m_ueCurrentPath.at(m_ueCurrentPath.size() -1).first)/2;
		midY = (m_ueCurrentPath.at(lowerIdx).second + m_ueCurrentPath.at(m_ueCurrentPath.size() -1).second)/2;
		m_middlePoints.push_back(make_pair(midX, midY));
	}
	findIdealPoint();
	return;
}

bool Allocator::isFinalDestination() {
	int lastIdx = m_route.size() - 1;
	if (m_uePosition.first != m_route.at(lastIdx).first || m_uePosition.second != m_route.at(lastIdx).second) {
		return false;
	}
	return true;
}

bool Allocator::areCollinear(XY p1, XY p2, XY p3){
	
	//Checks if three points are part of the same line segment
	double a = p1.first * (p2.second - p3.second) + p2.first * (p3.second - p1.second) + p3.first * (p1.second - p2.second);

   	if (a == 0) {
	   return true;
   	}
   return false;
}

void Allocator::setEnbsRadius(double enbRadius) {
	if(enbRadius > 0) {
		m_eNbRadius = enbRadius;
		return;
	}
	cout << "The radius must be a positive value\n";
}

double Allocator::getEnbsRadius() {
	return m_eNbRadius;
}