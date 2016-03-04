/*****************************************************************************
 File:   ssat.cc
 Author: Adela Yang, Venecia Xu, Son Ngo
 Date:   February 2016
 
 Description:
 Implement a DPLL-based stochastic satisfiability (SSAT) solver to test the
 effectiveness of unit clause propagation, pure variable elimination, and
 three different splitting heuristics
 
 Running instructions:
 Run this program with two arguments on the command line
 the input file and the algorithm
 
 g++ -Wall -o ssat ssat.cc [file name]
 ./ssat [file name]
 
 Running on dover: g++ -std=c++11 -Wall -o ssat ssat.cc [file name]
 ./ssat [file name]
 
 
 NOTES FROM ADELA FOR SPLITTING HEURISTIC:
 https://goo.gl/Ax0bpc
 On page 549+, it lists 6 splitting heuristics and their performances
 
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

map <int, varInfo> variables; //starts index at 1
map <int, set<int> > clauses; //startes index at 0

bool UNSATclauseExists = false;
bool headHasValue = false;

/***************************************************************************/
/* functions */
double SOLVESSAT(const unsigned int &algorithm);
void readFile(string input);
void tokenize(string str, vector<string> &token_v);
pair<bool, int> isPureChoice(int variable);

int variableSignInClause(int clauseEntry, int variable);
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
 Returns:   nothing
 Description:
 *****************************************************************************/
int main(int argc, char* argv[]) {
    //open the file for reading
    string input = argv[1];
    readFile(input);

    string names[] = {"NAIVE", "UCPONLY", "PVEONLY", "UCPPVE", "RANDOMVAR", "MAXVAR", "MINCLAUSE", "MAXCLAUSE"};
    
    for(unsigned int i = NAIVE; i <= MAXCLAUSE; i++) {
        resetResult();
        runAndPrintResult(i, names[i]);
    }
    
    return 0;
}

/*****************************************************************************
 Function:  runAndPrintResult
 Inputs:    number and name of algorithm being run
 Returns:   nothing
 Description:   prints information to terminal and files
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
 Description:   the main algorithm implementation
 ***************************************************************************/
double SOLVESSAT(const unsigned int &algorithm){
    
    //base cases
    if(clauses.empty()){
        return SUCCESS;
    }
    
    if (UNSATclauseExists || variables.empty() == true) {
        return FAILURE;
    }
    
    pair<bool, int> result;
    varInfo savedInfo;
    map<int, set <int> > savedSATClauses;
    vector<int> savedFalseLiteralClause;
    map<int, double> savedInactiveVariables;
    
    int v = 0;
    int value = 0;
    
    //BEGIN UNIT CLAUSES PROPAGATION
    if(algorithm == UCPONLY || algorithm >= UCPPVE){
        for(map<int, set<int> >::iterator it = clauses.begin(); it != clauses.end(); it++){
            
            if(it->second.size() == UNIT_SIZE){
                
                ++numUCP;
                
                value = POSITIVE;
                set<int>::iterator se = it->second.begin();
                v = *se;
                
                if(v < 0){
                    value = NEGATIVE;
                    v *= NEGATIVE;
                }
                
                savedInfo.quantifier = variables[v].quantifier;
                savedInfo.clauseMembers = variables[v].clauseMembers;
                
                updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
                
                double probSAT = SOLVESSAT(algorithm);
                
                undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
                
                if(variables[v].quantifier == CHOICE_VALUE){
                    return probSAT;
                }
                
                if(value == NEGATIVE){
                    return probSAT * (1 - variables[v].quantifier);
                }
                
                return probSAT * variables[v].quantifier;
            }
        }
    }
    //END UNIT CLAUSES PROPAGATION
    
    //BEGIN PURE CHOICE ELIMINATION
    if(algorithm >= PVEONLY){
        for (map<int, varInfo>::iterator it = variables.begin(); it != variables.end(); it++){
            
            result = isPureChoice(it->first);
            v = it->first;
            
            if(result.first == false) {
                continue;
            }
            else {
                
                ++numPVE;
                
                savedInfo.quantifier = variables[v].quantifier;
                savedInfo.clauseMembers = variables[v].clauseMembers;
                value = result.second;
                
                updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
                
                double probSSAT = SOLVESSAT(algorithm);
                
                undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
                
                return probSSAT;
            }
        }
    }
    //END PURE CHOICE ELIMINATION
    
    //BEGIN VARIABLE SPLITS
    if(algorithm <= UCPPVE){
        v = unassigned_var();
    } else {
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
    
    ++numVS;

    //try setting v to FALSE

    value = NEGATIVE;
    
    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
    double probSATWithFalse = SOLVESSAT(algorithm);
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
    //end setting v to FALSE
    
    //try setting v to TRUE
    
    value = POSITIVE;
    
    savedInfo.quantifier = variables[v].quantifier;
    savedInfo.clauseMembers = variables[v].clauseMembers;
    
    updateClausesAndVariables(v, value, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
    double probSATWithTrue = SOLVESSAT(algorithm);
    
    undoChanges(v, value, &savedInfo, &savedSATClauses, &savedFalseLiteralClause, &savedInactiveVariables);
    
    //end setting v to TRUE
    
    if(variables[v].quantifier == CHOICE_VALUE){
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
    varInfo* info = &(variables[variable]);
    map<int, int>* clauseSet = &((*info).clauseMembers);
    
    for(map<int, int>::iterator it = (*clauseSet).begin(); it != (*clauseSet).end(); it++) {
        int clauseEntry = it->first;
        int clauseEntryValue = it->second;
        
        //remove clauses that have the true-value variables
        if (clauseEntryValue == value){
            
            //save true clause for undochanges
            (*savedSATClausesPtr).insert(pair<int, set<int> >(clauseEntry, clauses[clauseEntry]));
            
            //going through the satisfied clause to update its variables' clauseMembers
            for (set<int>::iterator iter = clauses[clauseEntry].begin(); iter != clauses[clauseEntry].end(); iter++){
                
                int removedVar = abs(*iter);
                
                //skip the being examined variable because we will delete them from
                //variables map later
                if (removedVar == variable){
                    continue;
                }
                
                //if variable we need to update is still active (i.e in the variables map) then remove the
                //clauseEntry from that variable
                if (variables.find(removedVar) != variables.end()){
                    variables[removedVar].clauseMembers.erase(clauseEntry);
                    
                    //if the variable after update is in no active clause then remove that variable from variables map
                    if (variables[removedVar].clauseMembers.empty() == true){
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
            (*savedFalseLiteralClausePtr).push_back(clauseEntry);
            clauses[clauseEntry].erase(NEGATIVE * value * variable);
            
            if (clauses[clauseEntry].empty() == true){
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
    
    //put back the assigned variable to variables list
    for (map<int, double>:: iterator it = (*savedInactiveVariables).begin(); it != (*savedInactiveVariables).end();){
        variables[it->first].quantifier = it->second;
        (*savedInactiveVariables).erase(it++);
    }
    
    variables[variable].quantifier = (*savedInfo).quantifier;
    variables[variable].clauseMembers = (*savedInfo).clauseMembers;
    
    //put back SAT clauses to clauses list
    for (map<int, set<int> >::iterator it = (*savedSATClausesPtr).begin(); it != (*savedSATClausesPtr).end(); it++){
        clauses[it->first] = it->second;
        
        //put back the satisfied clauses to their corresponding variables
        for (set<int>::iterator iter = (it->second).begin(); iter != (it->second).end(); iter++){
            int savedVariable = abs(*iter);
            
            if (*iter > 0){
                (variables[savedVariable].clauseMembers).insert(pair<int, int>(it->first, POSITIVE));
            }
            else {
                (variables[savedVariable].clauseMembers).insert(pair<int, int>(it->first, NEGATIVE));
            }
        }
    }
    
    //put back FALSE literals to their original clauses
    for (vector<int>::iterator it = (*savedFalseLiteralClausePtr).begin(); it != (*savedFalseLiteralClausePtr).end(); it++){
        
        if ((variables[variable].clauseMembers)[*it] > 0){
            clauses[*it].insert(variable);
        }
        else {
            clauses[*it].insert(NEGATIVE * variable);
        }
    }
    
    UNSATclauseExists = false;
}

/***************************************************************************/
/* SPLITTING HEURISTICS */

/***************************************************************************
 Function:  randomSH
 Inputs:    none
 Returns:   variable
 Description: picks a random variable from the block
 ***************************************************************************/
int randomSH(){
    if (variables.empty() == true) {
        return INVALID;
    }
    
    vector <int> temp = helperSH();
    
    srand(time(NULL));
    int randNum = rand() % (temp.size());
    
    return temp[randNum];
}

/***************************************************************************
 Function:  maximumSH
 Inputs:    none
 Returns:   variable
 Description:   picks the variable that appears in the most clauses
 ***************************************************************************/
int maximumSH(){
    if (variables.empty() == true) {
        return INVALID;
    }
    
    vector <int> temp = helperSH();
    
    int max = INT_MIN;
    int maxIndex;
    vector <int>::iterator it;
    for(it = temp.begin(); it!=temp.end(); it++){
        if ((int)(variables[(*it)].clauseMembers).size() > max){
            max = (variables[(*it)].clauseMembers).size();
            maxIndex = (*it);
        }
    }
    
    return maxIndex;
}

/***************************************************************************
 Function:  maxClause
 Inputs:    none
 Returns:   variable
 Description:   picks the variable that appears in the most clauses
 ***************************************************************************/
 int maxClause(){
    vector<int> activeBlock = helperSH();

    unsigned int maxSize = 0;
    unsigned int maxClause = 0;
    unsigned int maxVar = 1;

    for (unsigned int i = 0; i < activeBlock.size(); i++){

        unsigned int tempClauseEntry = largestClause(activeBlock[i]);

        unsigned int tempSize = (clauses[tempClauseEntry]).size();

        if (maxSize < tempSize){
            maxSize = tempSize;
            maxClause = tempClauseEntry;
            maxVar = activeBlock[i];
        }

    }

    if (clauses[maxClause].empty() == true){
        return INVALID;
    }

    return maxVar;
 }

 /***************************************************************************
 Function:  minClause
 Inputs:    none
 Returns:   variable
 Description:   picks the variable that appears in the most clauses
 ***************************************************************************/
 int minClause(){
    vector<int> activeBlock = helperSH();

    unsigned int minSize = INT_MAX;
    unsigned int minClause = INT_MAX;
    unsigned int minVar = 1;

    for (unsigned int i = 0; i < activeBlock.size(); i++){

        unsigned int tempClauseEntry = smallestClause(activeBlock[i]);

        unsigned int tempSize = (clauses[tempClauseEntry]).size();

        if (minSize > tempSize){
            minSize = tempSize;
            minClause = tempClauseEntry;
            minVar = activeBlock[i];
        }

    }

    if (clauses[minClause].empty() == true){
        return INVALID;
    }

    return minVar;
 }
 
 /***************************************************************************
 Function:  helperSH
 Inputs:    none
 Returns:   vector of same block variables
 Description: determines the variables of the same block
 ***************************************************************************/
vector<int> helperSH(){
    vector <int> temp;
    double previous;
    bool started = false;
    
    map<int, varInfo>::iterator it;
    for(it = variables.begin(); it!=variables.end(); ++it){
        if(!started){
            started = true;
            previous = (it->second).quantifier;
            temp.push_back(it->first);
        }
        else{
            if(previous * (it->second).quantifier < 0){
                break;
            }
            else{
                temp.push_back(it->first);
                previous = (it->second).quantifier;
            }
        }
    }
    
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
    
    for (map<int, int>::iterator it = (*clauseInfo).begin(); it != (*clauseInfo).end(); it++){
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
 Function:  variableSignInClause
 Inputs:    variable and clause
 Returns:   int (positive or negative)
 Description:   indicates the sign of a variable within a given clause
 ***************************************************************************/
int variableSignInClause(int clauseEntry, int variable) {
    return variables[variable].clauseMembers[clauseEntry];
}

/***************************************************************************
 Function:  unassigned_var
 Inputs:    none
 Returns:   variable
 Description:   return the next variable in the same block with no assigned value
 ***************************************************************************/
int unassigned_var(){
    if (variables.empty() == true) {
        return INVALID;
    }
    return variables.begin()->first;
}

/***************************************************************************/
/* PRINTING FUNCTIONS */

/***************************************************************************
 Function:  printClauses
 Inputs:    none
 Returns:   none
 Description:   prints the clauses
 ***************************************************************************/
void printClauses() {
    cout << "printing clauses " << endl;
    map<int, set <int> >::iterator it;
    for(it = clauses.begin(); it!=clauses.end(); ++it){
        cout << it->first << ":";
        set<int>::iterator iter;
        
        for(iter = (it->second).begin(); iter!=(it->second).end();++iter){
            cout << " " << (*iter);
        }
        cout << endl;
    }
    cout << "\n";
}

/***************************************************************************
 Function:  printVariables
 Inputs:    none
 Returns:   none
 Description:   prints the variables
 ***************************************************************************/
void printVariables() {
    
    map<int, varInfo>::iterator it;
    
    cout << "printing variable quantifiers " << endl;
    for(it = variables.begin(); it != variables.end(); it++){
        cout << it->first << " => " << (it->second).quantifier << endl;
    }
    cout << "\n";
    
    cout << "printing varInfo" << endl;
    for (it = variables.begin(); it != variables.end(); it++){
        map<int, int>::iterator itClause = (it->second).clauseMembers.begin();
        cout << "Clause Set of variable " << it->first << endl;
        for (; itClause != (it->second).clauseMembers.end(); itClause++){
            cout << itClause->first << " =>"<< itClause->second << endl;
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
    if(!inFile.is_open()){
        cout << "File is not valid" << endl;
        exit(1);
    }
    
    i = 4;
    while(i >= 0) {
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
    while(i > 0 ){
        getline(inFile, sTemp);
        vSTemp.clear();
        tokenize(sTemp, vSTemp);
        
        varInfo var;
        for(unsigned int b = 0; b <= vSTemp.size(); b++){
            if(b%2 == 1) {
                var.quantifier = stod(vSTemp.at(b));
                variables.insert(pair<int, varInfo>(count, var));
            }
        }
        
        count++;
        i--;
    }
    
    getline(inFile, sTemp);
    getline(inFile, sTemp);
    
    for(i = 0; i < numClauses; i++){
        vSTemp.clear();
        getline(inFile, sTemp);
        tokenize(sTemp, vSTemp);
        vSTemp.pop_back();
        
        for(unsigned int j = 0; j < vSTemp.size(); j++){
            int num = stoi(vSTemp[j].c_str());
            vITemp.insert(num);
            
            if(num > 0){
                ((variables[abs(num)]).clauseMembers).insert(pair<int, int>(i, POSITIVE));
            }
            else{
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
 Returns:   vector
 Description:
 splits a string by whitespace
 ***************************************************************************/
void tokenize(string str, vector<string> &token_v){
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

/***************************************************************************
 Function:  resetResult
 Inputs:    none
 Returns:   none
 Description:   reset all counts for numUCP, numPVE and numVS
 ***************************************************************************/
void resetResult(){
    numUCP = 0;
    numPVE = 0;
    numVS = 0;
}

/***************************************************************************
 Function:  largestClause
 Inputs:    variable
 Returns:   clause
 Description:
     returns the clause that has the most variables within a variable's 
     clauseSet
 ***************************************************************************/
int largestClause(int variable){

    unsigned int maxSize = 0;
    unsigned int maxClause = 0;

    for (map<int, int>::iterator it = variables[variable].clauseMembers.begin(); it != variables[variable].clauseMembers.end(); it++){
        if (maxSize < clauses[it->first].size()){
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
     returns the clause that has the most variables within a variable's 
     clauseSet
 ***************************************************************************/
int smallestClause(int variable){

    unsigned int minSize = INT_MAX;
    unsigned int minClause = INT_MAX;

    for (map<int, int>::iterator it = variables[variable].clauseMembers.begin(); it != variables[variable].clauseMembers.end(); it++){
        if (minSize > clauses[it->first].size()){
            minSize = clauses[it->first].size();
            minClause = it->first;
        }
    }

    return minClause;
}


/****** END OF FILE **********************************************************/