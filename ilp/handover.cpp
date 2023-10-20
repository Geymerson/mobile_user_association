#include <ilcplex/ilocplex.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <string>
#include <math.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cassert>

// Earth Radius for haversines formula
#define E_RADIUS 6371
#define REPEAT 30
#define DATASETS 6

ILOSTLBEGIN 

// This function converts decimal degrees to radians
double deg2rad(double deg) {
  return (deg * M_PI / 180);
}

//  This function converts radians to decimal degrees
double rad2deg(double rad) {
  return (rad * 180 / M_PI);
}

double denormalize(double val, double min, double max) {
	return val*(max-min) + min;
}

// Calculates the distance between two points from latitude and longitude
// For mobility s
double distance(double lat1d, double lon1d, double lat2d, double lon2d) {
	double lat1r, lon1r, lat2r, lon2r, u, v;
	lat1r = deg2rad(lat1d);
	lon1r = deg2rad(lon1d);
	lat2r = deg2rad(lat2d);
	lon2r = deg2rad(lon2d);
	u = sin((lat2r - lat1r)/2);
	v = sin((lon2r - lon1r)/2);
	return 2.0 * E_RADIUS * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}

//Define Matrix of variable numbers
typedef IloArray<IloNumVarArray> NumVarMatrix;

//Define Matrix of numbers
typedef IloArray<IloNumArray> NumMatrix;

//Define pair of doubles
typedef pair<double, double> LL;

int main(int argc, char **argv) {
	int m = 0;
	while(m < DATASETS) {

		// Create the environment
		IloEnv env;
		try {
			/**
			 * 					### MODEL ###
			 *	User Association for Mobile Networks Base Stations
			* 
			**/
			IloModel model(env, "User Association for Mobile Networks Base Stations");

			// Initiate cplex objetc
			IloCplex cplex(model);

			cplex.setParam(IloCplex::Param::Threads, 1);

			// RNG seed
			srand (time(NULL));

			ifstream file("./instances/ds"+to_string(m+1)+".in", std::ios::in);
			//check to see that the file was opened correctly:
			if (!file.is_open()) {
				std::cerr << "There was a problem opening the input file: "<< "../instances/ds"+to_string(m+1)+".in" <<"\n" ;
				exit(1);//exit or do additional error checking
			}

			// Reading number of users and eNBs
			int ues, enbs, dcs, ds_type;
			double r_max;
			file >> ues >> enbs >> dcs >> ds_type >> r_max;

			//Max bandwidth provided by eNBi
			vector<double> enbs_bdw(enbs);
			for(int i = 0; i < enbs; i++) {
				file >> enbs_bdw[i];
			}

			// Reading bandwidth requirements for users and eNBs.
			vector<vector<double>> users_bdw_requirements(enbs, vector<double>(ues));
			for(int i = 0; i < enbs; i++) {
				for(int k = 0; k < ues; k++) {
					file >> users_bdw_requirements[i][k];
				}
			}

			/**
			 * 
			 * INPUT PARAMETERS READINGS (COST, HANDOVER AVG, LAT and LONG)
			 * 
			 **/

			//Average handover frequency between eNBi and eNBj
			vector<vector<double>> handover_avg(enbs, vector<double>(enbs));

			//Handove average for each eNBj
			vector<double> hj(enbs);

			//The available bandwith in the eNB i
			vector<double> Wi(enbs);

			//The RSSI value between user k and enbs i
			vector<vector<double>> rsrq_ki(enbs, vector<double>(ues));

			//The distance value between user k and enbs i
			vector<vector<double>> d_ki(enbs, vector<double>(ues));

			//The communication cost between eNB_i and data center s
			vector<vector<double>> c_is(enbs, vector<double>(dcs));

			// Reading average handover frequency matrix
			for (int i = 0; i < enbs; i++) {
				hj[i] = 0;
				for (int j = 0; j < enbs; j++) {
					file >> handover_avg[i][j];
					hj[i] += handover_avg[i][j];				
				}
				hj[i] /= (enbs - 1);
			}

			// Reading the bandwidth occupancy in each eNB
			for (int i = 0; i < enbs; i++) {			
				file >> Wi[i];
			}

			// Reading eNBs and data centers communication cost
			// Variable not used for this model
			for (int i = 0; i < enbs; i++) {			
				for (int s = 0; s < dcs; s++) {
					file >> c_is[i][s];
				}		
			}	

			// Reading distance matrix
			for (int i = 0; i < enbs; i++) {			
				for (int k = 0; k < ues; k++) {
					file >> d_ki[i][k];
				}		
			}
	
			// Reading RSSI OR RSRQ matrix
			for (int i = 0; i < enbs; i++) {			
				for (int k = 0; k < ues; k++) {
					file >> rsrq_ki[i][k];
				}
			}
			file.close();

			/** ###	DECISION VARIABLES ### **/
			// Bki = ue_k connected to eNB_i
			NumVarMatrix Bki(env, enbs);
			for (int i = 0; i < enbs; i++) {
				Bki[i] = IloNumVarArray(env, ues, 0, 1, ILOINT);
			}

			/** #### Users model ### **/
			IloExpr users(env);
			for(int i = 0; i < enbs; i++) {
				for(int k = 0; k < ues; k++) {
					users += (Bki[i][k] * (rsrq_ki[i][k] + hj[i]));
				}
			}

			//Every user must be connected to one tower only
			for(int i = 0; i < ues; i++) {
				IloExpr exp(env);			
				for(int j = 0; j < enbs; j++) {
					exp += Bki[j][i];											
				}
				model.add(exp == 1);
				exp.end();
			}

			// enbs must have enough bandwidth to serve the users
			for(int i = 0; i < enbs; i++) {
				IloExpr exp(env);
				for(int j = 0; j < ues; j++) {
					exp += Bki[i][j] * users_bdw_requirements[i][j];				
				}
				model.add(exp + denormalize(Wi[i], 0, enbs_bdw[i]) <= enbs_bdw[i]);
				exp.end();
			}

			// Save to file, uncomment if needed
			ofstream outfile;
			string name = "_ues"+to_string(ues)+"enbs"+to_string(enbs);
			outfile.open("./output/ds"+to_string(m+1)+"_result.csv", ios::out | ios::trunc);
			assert(outfile.isOpen());
			outfile << "value,exe_time,ues,enbs\n";

			for (int j = 0; j < REPEAT; j++) {

				IloObjective obj1 = IloMinimize(env, users);
				model.add(obj1);

				clock_t c_start = clock(); //CPU TIME
				
				if (cplex.solve()) { //Solve and save to file at output/ds<j>.csv
					outfile << cplex.getObjValue() << ',' << (double) (clock() - c_start)/CLOCKS_PER_SEC
							<< ',' << ues << ',' << enbs;
					if(j != REPEAT - 1) {
						outfile << '\n';
					}
				}
				
				if(j == 0) {
					NumMatrix solBki(env, enbs);
					for (int i = 0; i < enbs; i++) {
						solBki[i] = IloNumArray(env, ues, 0, 1, ILOINT);
						cplex.getValues(solBki[i], Bki[i]);
					}
					
					ofstream userAllocFile;
					userAllocFile.open("./output/ds"+to_string(m+1)+"_allocation"+".csv", ios::out | ios::trunc);
					assert(userAllocFile.isOpen());
					
					for(int k = 0; k < ues; k++) {
						if(k != ues -1){
							userAllocFile << "UE_" + to_string(k) << ',';
						}
						else {
							userAllocFile << "UE_" + to_string(k) << '\n';
						}
					}
					
					for(int i = 0; i < enbs; i++) {
						for(int k = 0; k < ues; k++) {
							if(k != ues -1) {
								userAllocFile << solBki[i][k] << ',';
							}
							else {
								userAllocFile << solBki[i][k] << '\n';
							}
						}
					}
				}
				model.remove(obj1);					
			}

			outfile.close();
			users.end();
		}
		catch (const IloException &e)
		{
			cerr << "Exception caught: " << e << endl;
		}
		catch (...)
		{
			cerr << "Unknown exception caught!" << endl;
		}

		env.end();
		m++;
	}
	return 0;
}
