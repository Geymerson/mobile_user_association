A Python script to generate instance inputs to benchmark the ILP and the ILS-VND models

Instance generation follows the GAP generation scheme proposed on the paper: **A genetic algorithm for the generalised assignment problem** (https://doi.org/10.1016/S0305-0548(96)00032-9)

To generate problem instances, choose the instance type [1 to 5] and run the script. The generated dataset is saved in the "instances" directory, and the file is named "dsX.in," where "X" is an index specific to each dataset generated using the script.

> python generate.py

---

General ds.in file structure:

The first line of the file contains general information about the dataset, including:

**First line**: Number of Users (ues); Number of eNBs (enbs); Number of Data Centers (dcs); Type of Instance (tpe, from 0 to 5); Maximum Required Bandwidth (RMax)**

**Second line**: contains the maximum available bandwidth for each eNB. The bandwidth values for each eNB are separated by spaces.

**The next N=enbs lines (NxK Matrix)**: contain the bandwidth requirements of users for each eNB. There is one line for each eNB. Each line lists the bandwidth requirements for individual users, separated by spaces.

**Subsequent N=enbs lines (NxN Matrix)**: contain information about the handover frequency between eNBs. Each line corresponds to a specific eNB, and it lists the handover frequencies to other eNBs. The values are separated by spaces.

**The next line**: includes the current available bandwidth for each eNB, which is represented as the "Wi" value for each eNB. These values are separated by spaces.

**The following N=enbs lines (NxS Matrix)**: contain communication costs between eNBs and data centers. Each line corresponds to a specific eNB and lists the communication costs to different data centers. The values are separated by spaces.

**The following N=enbs lines (NxK Matrix)**: provide information about the distances between eNBs and users. Each line corresponds to a specific eNB and lists the distances to different users. The values are separated by spaces.

**The following N=enbs lines (NxK Matrix)**: contain the RSRQ (Reference Signal Received Quality) values between eNBs and users. Each line corresponds to a specific eNB and lists the RSRQ values for different users. The values are separated by spaces.