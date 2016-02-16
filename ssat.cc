/*****************************************************************************
 File:   ssat.cc
 Author: Adela Yang, Venecia Xu, Son Ngo
 Date:   February 2016

 Description:
 Implement a DPLL-based stochastic satisfiability (SSAT) solver to test the
    effectiveness of unit clause propagation, pure variable elimination, and 
    three different splitting heuristics
 
 Notes: 
 Run this program with 2 arguments on the command line
 
 Running instructions:
 Run this program with two arugments on the command line 
    the input file
    the splitting heuristic

 Data Structures:

******************************************************************************/

/*****************************************************************************/
/* include files */
#include <iostream>

/***************************************************************************/
/* globals variables */

//probably will not use these vars...
string command;
static const int maximumClauseLength;
static const int minimumClauseLength;
static const double averageClauseLength;
static const int seed;

static const int numVars;
static const int numClauses;

vector <double> variables;
vector <vector <int>> clauses;

static const vector <vector <int>> solution;
static const double successProbability;
static const double solutionTime

using namespace std;

/***************************************************************************/
/* functions */

/*****************************************************************************
 Function: main
 Inputs:   argv
 Returns:  nothing
 Description:
 *****************************************************************************/
int main(int argc, char* argv[]) {
    
    /* open the file for reading */
    string input  = "small1.ssat"
    
	return 0;
} /* end Main */

/***************************************************************************
 Function: readFile
 Inputs:   file name
 Returns:  nothing
 Description:
 ***************************************************************************/
void readFile(string input) {
    string temp;
    ifstream inFile;
    
    inFile.open(input.c_str());
    
    /* file does not exist, then do nothing */
    if(!infile.is_open()){
        cout << "File is not valid" << endl;
        return 0;
    }
    
    while(!infile.eof()){			// store file names in a vector
        getline(infile, tempS);
        fileNames.push_back(tempS);
    }
    
    /* close the file */
    infile.close();
    

}


/***************************************************************************
 Function:
 Inputs:
 Returns:
 Description:
 ***************************************************************************/


/****** END OF FILE **********************************************************/
