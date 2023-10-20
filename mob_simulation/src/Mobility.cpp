#include "Mobility.h"

Mobility::Mobility() {
    loadUesPositionListNames();
    m_ueCurrentPosition = vector<XYT> (m_numberOfues, make_pair(make_pair(-1, -1), -1));
    m_currentTimeStep = 0;
}

void Mobility::loadUesPositionListNames() {
    string fileName = "uesRouteList.csv";
    ifstream routesFile = ifstream(fileName);

    if (!routesFile.is_open()) {
        cout << "Could not open route list name file:\n";
        cout << fileName << endl;
        exit(1);
    }

    m_numberOfues = 0;
    string routeName;
    while (!routesFile.eof()) {
        getline(routesFile, routeName);
        if (routeName == "") {
            break;
        }
        // cout << routeName << endl;
        m_uesPositionList.push_back(routeName);
    }
    m_numberOfues = m_uesPositionList.size();
    // cout << m_numberOfues << endl;
}

bool Mobility::isEndOfRoute() {
    return m_endOfRoute;
}

void Mobility::nextUePosition() {
    m_endOfRoute = true;
    for(int k = 0; k < m_numberOfues; k++) {
        m_ueCurrentPosition.at(k).second = -1;
        int ueID = getUeID(k);
        if(m_uesPositions.at(ueID).empty()) { continue; }
        m_endOfRoute = false;
        XYT p = m_uesPositions.at(ueID).front();
        if(p.second == m_currentTimeStep) {
            m_ueCurrentPosition.at(k) = p;
            m_uesPositions.at(ueID).pop_front();
        }
    }
    m_currentTimeStep++;
}

int Mobility::getNumberOfUes() {
    return m_numberOfues;
}

int Mobility::getNumberOfEnbs(){
    return m_numberOfEnbs;
}

void Mobility::loadUesPosition() {
    for (int i = 0; i < m_uesPositionList.size(); i++) {
        string ueFileName = "./routes/"+m_uesPositionList.at(i);
        loadPositions(ueFileName, true);
    }
}

void Mobility::setEnbPosition(int i, XY pos) {
    m_enbsPosition.at(i) = pos;
}

XY Mobility::getEnbPosition(int i) {
    return m_enbsPosition.at(i);
}

XYT Mobility::getUePosition(int k) {
    int id = m_ueIDList.at(k);
    return m_ueCurrentPosition.at(k);
	return m_uesPositions.at(id).at(0);
}

// void Mobility::setUePosition(int k, XY pos) {
// 	m_uesPosition[k] = pos;
// }

void Mobility::loadEnbsPosition(const string &posFileName) {
	m_enbsPosition = loadPositions(posFileName, false);
    m_numberOfEnbs = m_enbsPosition.size();
}

vector<XY> Mobility::loadPositions(const string &posFileName, bool isUePosition) {
    ifstream posFile(posFileName);

    if (!posFile.is_open()) {
        cout << "Could not open the file:\n";
        cout << posFileName << endl;
        exit(1);
    }

    int ue_id, timeStep;
    double px, py;
    string id, x, y, step;
    string skipper;
    getline(posFile, skipper); //Ignore column names

    vector<XY> positionList;
    deque<XYT> uePositionList;
    while (!posFile.eof()) {
        getline(posFile, id, ',');
        if (id == "") { break; } //Reached end of file

        ue_id = atoi(id.c_str());
        getline(posFile, x, ',');
        px = atof(x.c_str());

        if (isUePosition) {
            getline(posFile, y, ',');
            py = atof(y.c_str());
            getline(posFile, step);
            timeStep = atoi(step.c_str());
            XY p = make_pair(px, py);
            uePositionList.push_back(make_pair(p, timeStep));
        }
        else {
            getline(posFile, y);
            py = atof(y.c_str());
            positionList.push_back(make_pair(px, py));
        }
    }


    if (isUePosition) {
        m_ueIDList.push_back(ue_id);
        m_uesPositions.insert(make_pair(ue_id, uePositionList));
    //     cout <<"Node ID " << ue_id << endl;
    //     // cout << uePositionList.size() << endl;
    }
    posFile.close();
    return positionList;
}

int Mobility::getUeID(int idx) {
    return m_ueIDList.at(idx);
}

double Mobility::get_distance(XY p1, XY p2) {
	double deltaX = (p1.first - p2.first);
	double deltaY = (p1.second - p2.second);
	deltaX = pow(deltaX, 2);
	deltaY = pow(deltaY, 2);
	return sqrt(deltaX+deltaY);
}

// void Instance::setAllEnbsPosition(vector<XY> pos) {
// 	m_enbsPosition = pos;
// }