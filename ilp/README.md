# ILP User Association Model

Source code for an Integer Linear Programming model that associates mobile network users to base stations.

Example of input instances can be found in the **/intances/** folder.

To execute this model, you will need CPLEX Studio 12.10.

### Basic Commands

Compile the source code
> make

Run the model
> ./ilp_model

Clean the directory
> make clean

Results will be outputed to the **/output/** folder.

The allocation.csv file indicates which user UE (columns) is connected to which base station (rows).

The result.csv file contains the CPU time need to solve the problem and the optimal solution value.