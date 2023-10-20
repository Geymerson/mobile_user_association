# Source Code for the User Association using the ILS-VND meta-heuristic

### Basic Commands

Compile the source code
> make

Run the model 

(make sure the script is executable with the command **sudo chmod +x ./script.sh**)
> ./script.sh

Clean the directory
> make clean

### Output description

Results will be outputed to the **/LOG/** folder.
--- 
**The full_route_0.csv contains the results for each simulation regarding all the base stations for each simulation step.**

**enb_id**: the base station ID;

**score**: indicates how good is the eNb for association operation (higher is better);

**alloc**: indicates if the user is connected to the current base station;

**rsrq**: The RSRQ for the current eNB;

**hand_ctn**: How many handovers the user has performed so far.

--- 

**The handFreq1.csv file contains the handover counts between base stations. This file is a NxN matrix in which rows and columns represent the ID of each base station**

--- 

**The ueLog1.csv file shows in which base station the user was connected according to its position.**

**ue_id**: user ID;

**ue_x**: current x position;

**ue_y**: current y position;

**alloc_enb**: The base station in which the user is connected;

**rsrq**: The RSRQ at the connected base station.