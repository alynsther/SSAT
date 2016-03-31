/*****************************************************************************
 File:   ssat.cc
 Author: Adela Yang, Venecia Xu, Son Ngo
 Date:   February 2016
 
 Description:
 Implement a DPLL-based stochastic satisfiability (SSAT) solver to test the
 effectiveness of unit clause propagation, pure variable elimination, and
 three different student-devised splitting heuristics compare to the naive 
 algorithm.
 
 Running instructions:
 Run this program with two arguments on the command line
 the input file and the algorithm
 
 g++ -Wall -o ssat ssat.cc
 ./ssat [file name]
 
 Running on dover: 
 g++ -std=c++11 -Wall -o ssat ssat.cc
 ./ssat [file name]
  
 ******************************************************************************/

/*****************************************************************************/
/* include files */
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <map>
#include <set>
#include <ctime>
#include <climits>

using namespace std;

/***************************************************************************/
/* constants */
static const double CHOICE_VALUE = -1.0;
static const int POSITIVE = 1;
static const int NEGATIVE = -1;
static const int INVALID = 0;
static const int UNIT_SIZE = 1;
static const int FAILURE = 0.0;
static const int SUCCESS = 1.0;

static const unsigned int NAIVE = 0;
static const unsigned int UCPONLY = 1;
static const unsigned int PVEONLY = 2;
static const unsigned int UCPPVE = 3;
static const unsigned int RANDOMVAR = 4;
static const unsigned int MAXVAR = 5;
static const unsigned int MINCLAUSE = 6;
static const unsigned int MAXCLAUSE = 7;
static const unsigned int PERCENTAGE = 100;

/***************************************************************************/
/* structs */
typedef struct varInfo{
    double quantifier;              // choice/chance (probability if chance)
    map<int, int> clauseMembers;    // key: index of the clauses the var is in
                                    // value: POS/NEG in its respective clauses
} varInfo;

/***************************************************************************/
/* globals variables */
int maximumClauseLength;
int minimumClauseLength;
double averageClauseLength;
int seed;
int numVars;
int numClauses;
int numUCP;
int numPVE;
int numVS;
double percentageVariableSplits;

map <int, varInfo> variables; //starts indexing at 1
map <int, set<int> > clauses; //starts indexing at 0

bool UNSATclauseExists = false;  //indicate existence of unsatisfiable clause

/***************************************************************************/
/* functions prototypes */
double SOLVESSAT(const unsigned int &algorithm);
void readFile(string input);
void tokenize(string str, vector<string> &token_v);
pair<bool, int> isPureChoice(int variable);

void updateClausesAndVariables(int variable, int value,
                               map<int, set<int> >* savedSATClausesPtr,
                               vector<int>* savedFalseLiteralClausePtr,
                               map<int, double>* savedInactiveVariables);
void assign(int variable, int value);
void undoChanges(int variable, int value, varInfo* savedInfo,
                 map<int, set<int> >* savedSATClausesPtr,
                 vector<int>* savedFalseLiteralClausePtr,
                 map<int, double>* savedInactiveVariables);
int unassigned_var();
void printVariables();
void printClauses();
int randomSH();
int maximumSH();
vector<int> helperSH();
void runAndPrintResult(int num, string name);
void resetResult();
int maxClause();
int minClause();
int largestClause(int variable);
int smallestClause(int variable);

/*****************************************************************************
 Function:  main
 Inputs:    argv
 Returns:   success/failure
 Description:
        give a test case (.ssat), it runs the solver with 7 required algorithms
        and prints out the result for recording
 *****************************************************************************/
int main(int argc, char* argv[]) {

    //open the file for reading
    string input = argv[1];
    readFile(input);

    string names[] = {"NAIVE", "UCPONLY", "PVEONLY", "UCPPVE", "RANDOMVAR", "MAXVAR", "MINCLAUSE", "MAXCLAUSE"};
    
    //run and print results of all algorithms, one at a time
    for (unsigned int i = NAIVE; i <= MAXCLAUSE; i++) {
        resetResult();
        runAndPrintResult(i, names[i]);
    }
    
    return 0;
}

/*****************************************************************************
 Function:  runAndPrintResult
 Inputs:    algorithm number and name of algorithm being run
 Returns:   nothing
 Description:   prints results of the corresponding algorithm to output window
 *****************************************************************************/
void runAndPrintResult(int num, string name) {

    double start, end, solutionTime;
    double allPossibleSplits = pow(2,numVars) - 1;
    
    start = clock();
    cout << "====================================================================" << endl;
    cout << "RESULT OF SOLVESSAT - " + name + ": " << SOLVESSAT(num) << endl;
    cout << "NUM OF UCP: " << numUCP << endl;
    cout << "NUM OF PVE: " << numPVE << endl;
    cout << "NUM OF VS: " << numVS << endl;
    cout << "PERCENTAGE OF VS: " << PERCENTAGE * (double)numVS/allPossibleSplits << endl;
    end = clock();
    solutionTime = double(end-start)/CLOCKS_PER_SEC;
    cout << "SOLUTION TIME: " << solutionTime << endl;
    cout << "====================================================================" << endl;
}

/***************************************************************************
 Function:  SOLVESSAT
 Inputs:    identifier of algorithm being run
 Returns:   double
 Description:   the main ssat algorithm implementation
 ***************************************************************************/
double SOLVESSAT(const unsigned int &algorithm){
    
    //returns success if all clauses have been satisfied
    if (clauses.empty()) {
        return SUCCESS;
    }
    
    //returns failure if there is at least one unsatisfiable clause OR there are 
    //no more active variables while there are still active clauses
    if (UNSATclauseExists || variables.empty() == true) {
        return FAILURE;
    }
    
    //[START] setting up the structures needed to save information that are being updated

    //struct to store the information of the variables: its quantifier and its clauseMember map
    // being considered
    varInfo savedInfo;

    //map of all clauses that are satisfied under the assignment
    map<int, set <int> > savedSATClauses;

    //vector of clause numbers whose currently assigned variable appear false
    vector<int> savedFalseLiteralClause;

    //map of all variables that become inactive after their clauseMember map is updated
    //after the assignment
    map<int, double> savedInactiveVariables;

    //[END] setting up
    
    //v - variable being considered
    int v = 0;

    //value - value to be assigned to v
    int value = 0;
    
    //BEGIN UNIT CLAUSES PROPAGATION
    if (algorithm == UCPONLY || algorithm >= UCPPVE) {
        for (map<int, set<int> >::iterator it = clauses.begin(); it != clauses.end(); it++) {
            
            if (it->second.size() == UNIT_SIZE) {
                
                //updating total number of UCP
                ++numUCP;       
                
                //assigning the required value for v
                value = POSITIVE;
                set<int>::iterator se = it->second.begin();
                v = *se;
                
                //make sure that the value matches with its quantifier
                if (v < 0) {
                    value = NEGATIVE;
                    v *= NEGATIVE;
                }
                
                //save the info of assigned variable v
                savedInfo.quantifier = variables[v].quantifier;
                savedInfo.clauseMembers = variables[v].clauseMembers;
                
                //the below part resemebles the algorithm distributed by professor Majercik
                updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
                
                double probSAT = SOLVESSAT(algorithm);
                
                undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
                
                if (variables[v].quantifier == CHOICE_VALUE) {
                    return probSAT;
                }
                
                if (value == NEGATIVE) {
                    return probSAT * (1 - variables[v].quantifier);
                }
                
                return probSAT * variables[v].quantifier;
            }
        }
    }
    //END UNIT CLAUSES PROPAGATION
    
    //BEGIN PURE CHOICE ELIMINATION

    //stores the return from isPureChoice function
    pair<bool, int> result;     
    
    if (algorithm >= PVEONLY) {

        //going through every key in the variables map to find the first pure choice variable
        for (map<int, varInfo>::iterator it = variables.begin(); it != variables.end(); it++){
            
            result = isPureChoice(it->first);

            //if there is no pure choice variable then continue
            if (result.first == false) {
                continue;
            }

            //store the being considered variable in variable v
            v = it->first;
            
            //found a pure choice variable, execute PVE and return computed probability, ending the above 
            // for-loop
            else {
                
                //updating total number of PVE
                ++numPVE;       
                
                //save the info of assigned variable v
                savedInfo.quantifier = variables[v].quantifier;
                savedInfo.clauseMembers = variables[v].clauseMembers;

                //set the value of v
                value = result.second;
                
                //the below part resemebles the algorithm distributed by professor Majercik
                updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
                
                double probSSAT = SOLVESSAT(algorithm);
                
                undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
                
                return probSSAT;
            }
        }
    }
    //END PURE CHOICE ELIMINATION
    
    //BEGIN VARIABLE SPLITS

    //choosing the right heuristic to run based on the "algorithm" variable
    if (algorithm <= UCPPVE) {
        v = unassigned_var();
    } 
    else {
        switch (algorithm){
            case RANDOMVAR:
                v = randomSH();
                break;
            case MAXVAR:
                v = maximumSH();
                break;
            case MINCLAUSE:
                v = minClause();
                break;
            default:
                v = maxClause();
                break;
        }
    }
    
    if (v == INVALID) {
        cout << "The variable is invalid" << endl;
        return FAILURE;
    }
    
    //updating total number of variable splits (VS)
    ++numVS;        

    //[BEGIN] try setting v to FALSE
    value = NEGATIVE;
    
    //save info of assigned variable v
    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    //the below part resemebles the algorithm distributed by professor Majercik
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
    double probSATWithFalse = SOLVESSAT(algorithm);
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
    //[END] try setting v to FALSE
    
    //[BEGIN] try setting v to TRUE
    value = POSITIVE;
    
    //save info of assigned variable v
    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    //the below part resemebles the algorithm distributed by professor Majercik
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
    double probSATWithTrue = SOLVESSAT(algorithm);
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
    //[END] try setting v to TRUE
    
    //the below part resemebles the algorithm distributed by professor Majercik
    if (variables[v].quantifier == CHOICE_VALUE) {
        return max(probSATWithFalse, probSATWithTrue);
    }
    
    return probSATWithFalse * (1 - variables[v].quantifier) + probSATWithTrue * variables[v].quantifier;
    
    //END VARIABLE SPLITS
}

/***************************************************************************
 Function:  updateClausesAndVariables
 Inputs:    structures keeping track of changing data
 Returns:   none
 Description:   remove all satisfactory clauses after a given assignment
                update the clauseMembers of affected variables
 ***************************************************************************/
void updateClausesAndVariables(int variable, int value,
                               map<int, set<int> >* savedSATClausesPtr,
                               vector<int>* savedFalseLiteralClausePtr,
                               map<int, double>* savedInactiveVariables) {

    //create pointer to varInfo struct of assigned variable
    varInfo* info = &(variables[variable]);

    //create pointer to clauseMember map (in varInfo struct) of assigned variable
    map<int, int>* clauseSet = &((*info).clauseMembers);
    
    //going through every entry in clauseSet to udpate based on the variable's assignment
    for (map<int, int>::iterator it = (*clauseSet).begin(); it != (*clauseSet).end(); it++) {

        //storing intermediate value to variables
        int clauseEntry = it->first;
        int clauseEntryValue = it->second;
        
        //remove clauses that have the true-value variables
        if (clauseEntryValue == value) {
            
            //save clauses that have been satisfied for undoChanges
            (*savedSATClausesPtr).insert(pair<int, set<int> >(clauseEntry, clauses[clauseEntry]));
            
            //going through every literal in the satisfied clause to update its variables' clauseMembers
            for (set<int>::iterator iter = clauses[clauseEntry].begin(); iter != clauses[clauseEntry].end(); iter++) {
                
                //keys in variables map are always positive
                int removedVar = abs(*iter);
                
                //skip the being examined variable because we will delete them from
                //variables map later
                if (removedVar == variable) {
                    continue;
                }
                
                //if removedVar is active (i.e in the variables map) then remove the
                //clauseEntry from removedVar's clauseMember map. If removedVar is not active then continue.
                if (variables.find(removedVar) != variables.end()) {
                    variables[removedVar].clauseMembers.erase(clauseEntry);
                    
                    //if removedVar's clauseMember map has no keys then erase removedVar from variables map 
                    if (variables[removedVar].clauseMembers.empty() == true) {

                        //save before erasing for undoChanges
                        (*savedInactiveVariables).insert(pair<int, double>(removedVar, variables[removedVar].quantifier));

                        variables.erase(removedVar);
                    }
                }
                
            }
            
            //clauseEntry becomes inactive, removed from clauses
            clauses.erase(clauseEntry);
        }
        
        //remove the falsely assigned variable in the clauses
        else {

            //save before erasing for undoChanges
            (*savedFalseLiteralClausePtr).push_back(clauseEntry);

            //making sure the removed value match the key's set's entry
            clauses[clauseEntry].erase(NEGATIVE * value * variable);
            
            //problem unsolvable if there is at least one unsatisfiable clause
            if (clauses[clauseEntry].empty() == true) {
                UNSATclauseExists = true;
            }
        }
    }
    
    //variable becomes inactive, removed from variables map
    variables.erase(variable);
}


/***************************************************************************
 Function:  undoChanges
 Inputs:    structures keeping track of changing data
 Returns:   none
 Description:   undo changes made before call to SOLVESSAT
 ***************************************************************************/
void undoChanges(int variable, int value, varInfo* savedInfo,
                 map<int, set<int> >* savedSATClausesPtr,
                 vector<int>* savedFalseLiteralClausePtr,
                 map<int, double>* savedInactiveVariables){
    
    //put back all inactive variables after the update to variables list
    for (map<int, double>:: iterator it = (*savedInactiveVariables).begin(); it != (*savedInactiveVariables).end();) {
        variables[it->first].quantifier = it->second;
        (*savedInactiveVariables).erase(it++);
    }
    
    //restore info of assigned variable
    variables[variable].quantifier = (*savedInfo).quantifier;
    variables[variable].clauseMembers = (*savedInfo).clauseMembers;
    
    //put back satisfied clauses to clauses list, going through every clause in the savedSATClauses map
    for (map<int, set<int> >::iterator it = (*savedSATClausesPtr).begin(); it != (*savedSATClausesPtr).end(); it++) {
        clauses[it->first] = it->second;
        
        //going through every literals in the satisfied clause to undo changes made to their clauseMembers maps
        for (set<int>::iterator iter = (it->second).begin(); iter != (it->second).end(); iter++) {

            //key of variables map is positive
            int savedVariable = abs(*iter);
            
            //put back removed clauses to the corresponding variable's clauseMembers map. We need to make sure
            //that the value is matched with their original sign.
            if (*iter > 0) {
                (variables[savedVariable].clauseMembers).insert(pair<int, int>(it->first, POSITIVE));
            }
            else {
                (variables[savedVariable].clauseMembers).insert(pair<int, int>(it->first, NEGATIVE));
            }
        }
    }
     
    //going through every clause that the assigned variable appears falsely and put back that variable to the 
    // right clause in clauses map
    for (vector<int>::iterator it = (*savedFalseLiteralClausePtr).begin(); it != (*savedFalseLiteralClausePtr).end(); it++) {
        
        //put back the variable to its original clause, with the right sign and value
        if ((variables[variable].clauseMembers)[*it] > 0) {
            clauses[*it].insert(variable);
        }
        else {
            clauses[*it].insert(NEGATIVE * variable);
        }
    }
    
    //reset the existence of unsatisfiable clause to be false
    UNSATclauseExists = false;
}

/***************************************************************************/
/* SPLITTING HEURISTICS */

/***************************************************************************
 Function:  randomSH
 Inputs:    none
 Returns:   variable
 Description: picks a random variable from the currently active block
 ***************************************************************************/
int randomSH(){

    // if there are no variables to choose from, return
    if (variables.empty() == true) {
        return INVALID;
    }
    
    // gets the vector of variables of the current block
    vector <int> temp = helperSH();

    
    srand(time(NULL));
    
    // choose a random variable of the block
    int randNum = rand() % (temp.size());

    return temp[randNum];
}

/***************************************************************************
 Function:  maximumSH
 Inputs:    none
 Returns:   int (variable)
 Description:   picks the variable that appears in the most clauses
 ***************************************************************************/
int maximumSH() {

    // if there are no variables to choose from, return
    if (variables.empty() == true) {
        return INVALID;
    }
    
    // get the vector of variables of the current block
    vector <int> temp = helperSH();
    
    int max = INT_MIN;
    int maxIndex;
    vector <int>::iterator it;

    // go through the block and update the variable that appears in the most clauses
    for (it = temp.begin(); it!=temp.end(); it++) {
        if ((int)(variables[(*it)].clauseMembers).size() > max) {
            max = (variables[(*it)].clauseMembers).size();
            maxIndex = (*it);
        }
    }
    
    return maxIndex;
}

/***************************************************************************
 Function:  maxClause
 Inputs:    none
 Returns:   int (variable)
 Description:   picks the variable that belongs to the biggest active clause     
 ***************************************************************************/
 int maxClause(){

    //get the variables in the current block
    vector<int> activeBlock = helperSH();

    unsigned int maxSize = 0;       //size of the largest clause found so far
    unsigned int maxClause = 0;     //index of the largest clause
    unsigned int maxVar = 1;        //the variable associated with the largest clause

     
    //for each variable in the active block
    for (unsigned int i = 0; i < activeBlock.size(); i++) {

        //find the largest clause the current variable is a part of
        unsigned int tempClauseEntry = largestClause(activeBlock[i]);
        unsigned int tempSize = (clauses[tempClauseEntry]).size();

        //if the clause is larger than the currently saved one, update
        if (maxSize < tempSize) {
            maxSize = tempSize;
            maxClause = tempClauseEntry;
            maxVar = activeBlock[i];
        }

    }

    if (clauses[maxClause].empty() == true) {
        return INVALID;
    }

    return maxVar;
 }

 /***************************************************************************
 Function:  minClause
 Inputs:    none
 Returns:   int (variable)
 Description:   picks the variable that belongs to the smallest active clause
 ***************************************************************************/
 int minClause() {

    //get the variables in the current block
    vector<int> activeBlock = helperSH();

    unsigned int minSize = INT_MAX;         //smallest clause size found so far
    unsigned int minClause = INT_MAX;       //index of smallest clause
    unsigned int minVar = 1;                //the variable associated with the current smallest clause

    //for each variable in the active block
    for (unsigned int i = 0; i < activeBlock.size(); i++) {

        //find the smallest clause the current variable is a part of
        unsigned int tempClauseEntry = smallestClause(activeBlock[i]);
        unsigned int tempSize = (clauses[tempClauseEntry]).size();

        //if the clause is smaller than the current saved one, update
        if (minSize > tempSize) {
            minSize = tempSize;
            minClause = tempClauseEntry;
            minVar = activeBlock[i];
        }

    }

    if (clauses[minClause].empty() == true) {
        return INVALID;
    }

    return minVar;
 }
 
 /***************************************************************************
 Function:  helperSH
 Inputs:    none
 Returns:   vector representing the currently active block
 Description: determines the variables of the same block and put all into
                one vector
 ***************************************************************************/
vector<int> helperSH() {

    vector <int> temp;
    double previous;
    bool started = false;
    
    map<int, varInfo>::iterator it;
    //iterates through variables map
    for (it = variables.begin(); it!=variables.end(); ++it) { 
        // gets the first variable and its quantifier of the map
        if (!started) {
            started = true;
            previous = (it->second).quantifier;
            temp.push_back(it->first);
        }
        // otherwise, it checks if the other quantifiers are part of the same block
        else {
            if (previous * (it->second).quantifier < 0) {
                // different block then return
                break;
            }
            else {
                temp.push_back(it->first);
                previous = (it->second).quantifier;
            }
        }
    }
    
    // returns a vector of the variables
    return temp;
}
         
/***************************************************************************/
/* UTILITY FUNCTIONS */

/***************************************************************************
 Function:  isPureChoice
 Inputs:    variable
 Returns:   True if the variable is a pure choice variable
 Description:   checks if variable is a pure choice variable
 ***************************************************************************/
pair<bool, int> isPureChoice(int variable) {

    varInfo info = variables[variable];
    if (info.quantifier != CHOICE_VALUE) {
        return pair<bool, int>(false, INVALID);
    }
    
    map<int, int>* clauseInfo = &(info.clauseMembers);
    int status = POSITIVE;
    bool signSwitch = false;
    
    for (map<int, int>::iterator it = (*clauseInfo).begin(); it != (*clauseInfo).end(); it++) {
        if (!signSwitch) {
            status = it->second;
            signSwitch = true;
        }
        else {
            if (status != it->second) {
                return pair<bool, int>(false, INVALID);
            }
        }
    }
    
    return pair<bool, int>(true, status);
}

/***************************************************************************
 Function:  unassigned_var
 Inputs:    none
 Returns:   variable
 Description:   return the next variable in the same block with no assigned value
 ***************************************************************************/
int unassigned_var() {

    if (variables.empty() == true) {
        return INVALID;
    }
    return variables.begin()->first;
}

/***************************************************************************
 Function:  resetResult
 Inputs:    none
 Returns:   none
 Description:   reset all counts for numUCP, numPVE and numVS
 ***************************************************************************/
void resetResult() {

    numUCP = 0;
    numPVE = 0;
    numVS = 0;
}

/***************************************************************************
 Function:  largestClause
 Inputs:    variable
 Returns:   clause
 Description:
        returns the clause that has the most variables within a variable's clauseSet
 ***************************************************************************/
int largestClause(int variable) {

    unsigned int maxSize = 0;
    unsigned int maxClause = 0;

    //for every clause the variable is a part of, check if it is larger than the current saved max clause and update if so
    for (map<int, int>::iterator it = variables[variable].clauseMembers.begin(); it != variables[variable].clauseMembers.end(); it++) {
        
        if (maxSize < clauses[it->first].size()) {
            maxSize = clauses[it->first].size();
            maxClause = it->first;
        }
    }

    return maxClause;
}

/***************************************************************************
 Function:  smallestClause
 Inputs:    variable
 Returns:   clause
 Description:
        returns the clause that has the least variables within a variable's clauseSet
 ***************************************************************************/
int smallestClause(int variable) {

    unsigned int minSize = INT_MAX;
    unsigned int minClause = INT_MAX;

    //for every clause the variable is a part of, check if it is smaller than the current saved min clause and update if so
    for (map<int, int>::iterator it = variables[variable].clauseMembers.begin(); it != variables[variable].clauseMembers.end(); it++) {
        
        if (minSize > clauses[it->first].size()) {
            minSize = clauses[it->first].size();
            minClause = it->first;
        }
    }

    return minClause;
}

/***************************************************************************/
/* PRINTING FUNCTIONS */

/***************************************************************************
 Function:  printClauses
 Inputs:    none
 Returns:   none
 Description:   prints the currently active clauses
 ***************************************************************************/
void printClauses() {

    cout << "printing clauses " << endl;
    map<int, set <int> >::iterator it;
    for (it = clauses.begin(); it!=clauses.end(); ++it) {
        cout << it->first << ":";
        set<int>::iterator iter;
        
        for (iter = (it->second).begin(); iter!=(it->second).end();++iter) {
            cout << " " << (*iter);
        }
        cout << endl;
    }
    cout << endl;
}

/***************************************************************************
 Function:  printVariables
 Inputs:    none
 Returns:   none
 Description:   prints the currently active variables
 ***************************************************************************/
void printVariables() {
    
    map<int, varInfo>::iterator it;
    
    cout << "printing variable quantifiers " << endl;
    for(it = variables.begin(); it != variables.end(); it++){
        cout << it->first << " => " << (it->second).quantifier << endl;
    }
    cout << endl;
    
    cout << "printing varInfo" << endl;
    for (it = variables.begin(); it != variables.end(); it++) {
        map<int, int>::iterator itClause = (it->second).clauseMembers.begin();
        cout << "Clause Set of variable " << it->first << endl;
        for (; itClause != (it->second).clauseMembers.end(); itClause++) {
            cout << itClause->first << " => "<< itClause->second << endl;
        }
        cout << endl;
    }
}

/***************************************************************************/
/* READING FILE FUNCTIONS */

/***************************************************************************
 Function:  readFile
 Inputs:    file name
 Returns:   none
 Description:
            reads the content of file and stores the information as global
            variables
 ***************************************************************************/
void readFile(string input) {

    string sTemp;
    ifstream inFile;
    vector<string> vSTemp;
    set<int> vITemp;
    int i;
    
    inFile.open(input.c_str());
    
    // file does not exist, then do nothing
    if (!inFile.is_open()) {
        cout << "File is not valid" << endl;
        exit(1);
    }
    
    // skips the initial 4 lines
    i = 4;
    while (i >= 0) {
        getline(inFile, sTemp);
        i--;
    }
    
    //the comments
    tokenize(sTemp, vSTemp);
    maximumClauseLength = stoi(vSTemp.back());
    
    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    minimumClauseLength = stoi(vSTemp.back());
    
    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    averageClauseLength = stod(vSTemp.back());
    
    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    seed = stoi(vSTemp.back());
    
    getline(inFile, sTemp);
    
    //v
    getline(inFile, sTemp);
    tokenize(sTemp, vSTemp);
    numVars = stoi(vSTemp.back());
    
    //c
    getline(inFile, sTemp);
    vSTemp.clear();
    tokenize(sTemp, vSTemp);
    numClauses = stoi(vSTemp.back());
    
    getline(inFile, sTemp);
    getline(inFile, sTemp);
    
    //variables
    i = numVars;
    unsigned int count = 1;
    while (i > 0) {
        getline(inFile, sTemp);
        vSTemp.clear();
        tokenize(sTemp, vSTemp);
        
        varInfo var;
        for (unsigned int b = 0; b <= vSTemp.size(); b++) {
            if (b%2 == 1) {
                var.quantifier = stod(vSTemp.at(b));
                variables.insert(pair<int, varInfo>(count, var));
            }
        }
        
        count++;
        i--;
    }
    
    getline(inFile, sTemp);
    getline(inFile, sTemp);
    
    for (i = 0; i < numClauses; i++){
        vSTemp.clear();
        getline(inFile, sTemp);
        tokenize(sTemp, vSTemp);
        vSTemp.pop_back();
        
        for (unsigned int j = 0; j < vSTemp.size(); j++) {
            int num = stoi(vSTemp[j].c_str());
            vITemp.insert(num);
            
            if (num > 0) {
                ((variables[abs(num)]).clauseMembers).insert(pair<int, int>(i, POSITIVE));
            }
            else {
                ((variables[abs(num)]).clauseMembers).insert(pair<int, int>(i, NEGATIVE));
            }
        }
        
        clauses.insert(pair<int, set<int> >(i,vITemp));
        vITemp.clear();
    }
    
    inFile.close();
    cout << "File read successfully" << endl;
}

/***************************************************************************
 Function:  tokenize
 Inputs:    string and vector
 Returns:   none (but the second parameter is modified)
 Description: splits a string by whitespace
 ***************************************************************************/
void tokenize(string str, vector<string> &token_v) {

    size_t start = str.find_first_not_of(' '), end=start;
    
    while(start != string::npos) {
        // Find next occurence of delimiter
        end = str.find(' ', start);
        // Push back the token found into vector
        token_v.push_back(str.substr(start, end-start));
        // Skip all occurences of the delimiter to find new start
        start = str.find_first_not_of(' ', end);
    }
}

/****** END OF FILE **********************************************************/