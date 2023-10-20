/*
 * ArgPack.cpp
 *
 *  Created on: 14/09/2015
 *      Author: bruno
 */

#include "ArgPack.h"
#include "InitError.h"

#include <string>
#include <cstring>
#include <unistd.h> // for getopt
#include <iostream>

//extern int optind;

using namespace std;

namespace opt
{

	ArgPack *ArgPack::def_ap_ = 0;

	ArgPack::ArgPack(int argc, char *const argv[])
	{
		filesPath = "./instances/";
		inputName = "";
		iterations = 100;
		repetitions = 30;
		time_limit = 0.5;
		rand_seed = 1;
		assert(!def_ap_);
		def_ap_ = this;
		program_name = argv[0];
		numberOfDatasets = 1;
		singleDataset = false;
		getParameters(argc, argv);
	}

	void ArgPack::getParameters(int argc, char *const argv[])
	{
		string usage = string("Usage: ") + program_name + " [options] <file>\n" +
					   "Compile time: " + __DATE__ + " " + __TIME__ + "\n" +
					   "	-h			: Help\n" +
					   "	-t			: Time limit\n" +
					   "	-n			: Number of datasets\n" +
					   "	-d			: Datasets path [default: " + filesPath + "]\n" +
					   "	-i			: The name of the instance\n" +
					   "	-r			: Number of repetitions per dataset [default: " + to_string(repetitions) + "] \n" +
					   "	-s<random seed>		: Random seed [default: " + to_string(rand_seed) + "]\n";

		string help = "Use -h for more information\n";

		if (argc == 1)
		{
			cout << "Executing program with default values\n";
			return;
		}
		else if (argc % 2 == 0) {
			const char ch = argv[1][1];
			if(ch == 'h') {
				cout << usage;
				exit(0);
			}
			else {
				throw InitError("Invalid command or number of parameters, something is missing!\n" + help);
			}
		}
		else
		{
			int i = 1;
			do
			{
				//Ignore the '-' character at position [0]
				const char ch = argv[i][1];
				switch (ch)
				{
				case 'd':
					filesPath = argv[i+1];
					break;
				case 'n':
					numberOfDatasets = atoi(argv[i+1]);
					break;
				case 'i':
					inputName = argv[i+1];
					singleDataset = true;
					break;
				case 'r':
					repetitions = atoi(argv[i+1]);
					break;
				case 's':
					rand_seed = atoi(argv[i+1]);
					break;
				case 't':
					time_limit = atof(argv[i+1]);
					break;
				default:
					throw InitError("\nInvalid input.\n" + usage);
					break;
				}
				i += 2;
			} while (i < argc);
			return;
		}
	}

} // namespace opt