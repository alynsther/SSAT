

//  Stephen Majercik
//  26 September 2002
//  edited 1 March 2016

//  Compile with:  g++ ssat-generator.cc -o ssat-generator


// Includes
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <map>

using namespace std;

//  Defines
#define LPC  20     		// Max literals per clause.
#define VARS 500	        // Max variables (minus 1).
#define EC   5000	        // Max Total clauses.

#define MAX_LINE_CHARS 500      // maximum characters in file line
#define MAX_NAME_LENGTH 50      // maximum variable name length

#define NUM_BASIC_ARGS 6

//  Constants
#define UNASSIGNED -1           // var has no value yet
#define CHOICEVAR -1.0          // indicates that var is a choice var
#define NOTNEGATED 1            // var in clause is not negated
#define NEGATED 0               // var in clause is negated
#define TRUE 1                  // var in clause is not negated
#define FALSE 0                 // var in clause is negated
#define NOVAR -1	        // no such variable:  signals error
#define SATISFIED 1	        // clause is satisfied
#define NEITHER_SAT_UNSAT 0     // clause is neither satisfied or unsat
#define UNSATISFIED -1          // clause is unsatisfied
#define EOL 0                   // value that signals end of a clause
#define UNDERFLOW_FACTOR 1e37   // takes care of underflow problem
#define CASEINC 1.0             // increment to distinguish no case from
// case with 0.0 prob in map
#define DEBUG 0                 // to display *lots* of debugging information
#define CHOICE 10
#define CHANCE 11

#define VARWIDTH 5
#define VALWIDTH 12

// Command line arguments
long numvars;         // file containing the SSAT encoding
long numclauses;         // low threshold success probability (minimum acceptable)
long max_clause_length;
long min_clause_length;

// Other global variables
long num_tree_nodes = 0;
long form[EC][LPC];	          // the formula... which vars in which clauses.
long sgn[EC][LPC];	          // sign of variable (0 = negated, 1 = not negated)
long lic[EC];		          // number of literals in clause
long assgn[VARS];	          // holds current truth assignment
long activelits[EC];          // number of active literals in each clause
long satisflits[EC];          // number of satisfying literals in each clause
long varstats[VARS][2];       // holds stats about variable distribution in active clauses
long num_clauses_sat = 0;     // how many clauses are satisfied
long num_clauses_unsat = 0;   // how many clauses are unsatisfied
double chancevarprob[VARS+1]; // probability of corresponding variable (-1 if choice var)
long ***vc;                   // structure holding clause lists for each literal

// function to read SSAT file
double make_ssat(long seeed);
bool alreadyin(long c, long numlit, long newlit);
void print_ssat_numbers();

// functions to solve SSAT problem
double solve_ssat(long node);
long set_stats();
void initassgn(long *a);

void print_clause(long c);

long floorlg (long n);

// main function

int main(int argc, char *argv[]) {

	// make sure all arguments are present
	if (argc < NUM_BASIC_ARGS) {
		cerr << "ssat-generator numvars numclauses max-clause-length min-clause-length varorder probs seed" << endl;
		exit(-1);
	}

	// process command-line arguments
	numvars = atoi(argv[1]);
	if (numvars > VARS) {
		cerr << "maximum number of variables > " << VARS << endl;
		exit(-1);
	}

	numclauses = atoi(argv[2]);
	if (numvars > EC) {
		cerr << "maximum number of clauses > " << EC << endl;
		exit(-1);
	}

	max_clause_length = atoi(argv[3]);
	if (max_clause_length > LPC) {
		cerr << "maximum clause length > " << LPC << endl;
		exit(-1);
	}

	min_clause_length = atoi(argv[4]);
	if (min_clause_length < 1) {
		cerr << "minimum clause length < 1" << endl;
		exit(-1);
	}

	if (strlen(argv[5]) != numvars) {
		cerr << "varorder string contains wrong number of variables" << endl;
		exit(-1);
	}

	long chance_argindex = NUM_BASIC_ARGS - 1;
	chancevarprob[0] = -99;
	char *varorder = argv[5];
	int choice_num = 1;
	int chance_num = 1;
	for (long v = 1 ; v <= numvars ; v++) {
		if (index(varorder, 'E') == varorder) {
			chancevarprob[v] = CHOICEVAR;
			++choice_num;
		}
		else if (index(varorder, 'R') == varorder) {
			++chance_argindex;
			if (chance_argindex >= argc-1) {  // minus 1 because of seed
				cerr << "not enough probabilities for chance variables in string" << endl;
				exit(-1);
			}
			chancevarprob[v] = atof(argv[chance_argindex]);
			++chance_num;
		}
		else
			break;
		++varorder;
	}

	long seed = atoi(argv[argc-1]);

	// generate the SSAT formula
	double average_clause_length = make_ssat(seed);

	// print out encoding statistics
	cout << endl;
	cout << ";  command               = ";
	for (int a = 0 ; a < argc ; a++)
		cout << argv[a] << " ";
	cout << endl;
	cout << ";  number of variables   = " << numvars << endl;
	cout << ";  number of clauses     = " << numclauses << endl;
	cout << ";  maximum clause length = " << max_clause_length << endl;
	cout << ";  minimum clause length = " << min_clause_length << endl;
	cout << ";  average clause length = " << average_clause_length << endl;
	cout << ";  seed                  = " << seed << endl << endl;
	cout << "v " << numvars << endl;
	cout << "c " << numclauses << endl << endl;

	print_ssat_numbers();

	// allocate memory for tree arrays
	num_tree_nodes = (long) pow(2.0, (double) numvars) - 1;

	// initialize the current assignment to UNASSIGNED
	initassgn(assgn);

	// set the stats initially
	set_stats();

	// start the clock
	clock_t tv_start = clock();

	// solve the problem
	double optimal_prob = solve_ssat(1);

	// stop the clock
	clock_t tv_end = clock();

	cout << endl << "Success Probability:  " << optimal_prob/UNDERFLOW_FACTOR << endl;

	// print out solution time
	double time_total = (tv_end - tv_start)/(double)CLOCKS_PER_SEC;
	cout << "Solution Time (CPU secs):  " << time_total << endl << endl;

}




double make_ssat(long seed) {


	// need vc structure for quick reference re: which clauses contain a given lit
	if ((vc = (long ***) malloc((numvars + 1) * sizeof (long **))) == NULL) {
		cerr << "Error: insufficient memory to allocate vs array" << endl;
		exit(-1);
	}

	// a list for clauses that v appears negated in and a list for clauses
	// that v appears not negated in
	for (long v = 1 ; v <= numvars ; ++v)
		if ((vc[v] = (long **) malloc(2 * sizeof(long *))) == NULL) {
			cerr << "Error: insufficient memory to allocate vc array" << endl;
			exit(-1);
		}

	// for now, just allocate space to count number of clauses that
	// each literal is in
	for (long v = 1 ; v <= numvars ; ++v) {
		for (long s = 0 ; s <= 1 ; ++s)
			if ((vc[v][s] = (long *) malloc(sizeof(long)))  == NULL) {
				cerr << "Error: insufficient memory to allocate vc array" << endl;
				exit(-1);
			}
	}

	// initialize vc structure
	for (long v = 1 ; v <= numvars ; ++v)
		for (long s = 0 ; s <= 1 ; ++s)
			vc[v][s][0] = 0;


	long total_lits = 0;

	srand(seed);

	for (long c = 0 ; c < numclauses ; c++) {
		long clength = (rand() % (max_clause_length - min_clause_length + 1)) + min_clause_length;
		lic[c] = clength;
		total_lits += clength;
		for(long l = 0 ; l < clength ; ++l) {
			long newlit = (rand() % numvars) + 1;
			while (alreadyin(c, l, newlit))
				newlit = (rand() % numvars) + 1;
			if (rand() % 2 == 0) {
				form[c][l] = newlit;
				sgn[c][l] = NOTNEGATED;
				long index = ++vc[newlit][TRUE][0];
				vc[newlit][TRUE][index] = c;
			}
			else {
				form[c][l] = newlit;
				sgn[c][l] = NEGATED;
				long index = ++vc[newlit][FALSE][0];
				vc[newlit][FALSE][index] = c;
			}
		}
	}

	return (double) total_lits / (double) numclauses;


}


bool alreadyin(long c, long numlit, long newlit) {

	for (long l = 0 ; l < numlit ; ++l)
		if (form[c][l] == newlit)
			return true;

	return false;
}




// solves the SSAT problem
//
double solve_ssat(long node) {

	set_stats();

	if (num_clauses_sat == numclauses) {
		return UNDERFLOW_FACTOR;
	}
	if (num_clauses_unsat > 0) {
		return 0.0;
	}


	long v = floorlg(node) + 1;

	assgn[v] = FALSE;
	double falseval = solve_ssat(node*2);

	assgn[v] = TRUE;
	double trueval = solve_ssat((node*2)+1);

	assgn[v] = UNASSIGNED;

	if (chancevarprob[v] == CHOICEVAR) {
		if (falseval > trueval)
			return falseval;

		return trueval;
	}

	else {
		return falseval * (1.0 - chancevarprob[v]) + trueval * chancevarprob[v];
	}

}





// If a variable appears with the right sgn, clause is true (1).
// Otherwise, value is number of active variables left (negated).
// A subsumed clause ought to be given value (1) automatically.
//
// varstats[v][i] is number of clauses containing var with sgn i.  Note
// that no statistics are kept on variables in inactive clauses.  This
// is because these literal instances have no effect on the truth value
// of the assignment.  This makes it possible for a variable to be
// irrelevant even though it appears in the formula, as long as it
// appears only in clauses whose truth value is known.
//
long set_stats() {
	long SAT_status = SATISFIED;
	num_clauses_sat = num_clauses_unsat = 0;

	for (long v = 1; v <= numvars; v++)
		varstats[v][NEGATED] = varstats[v][NOTNEGATED] = 0;

	for (long c = 0; c < numclauses; c++) {
		activelits[c]= satisflits[c] = 0;
		long actlits = 0;
		long satlits = 0;
		for (long l = 0; l < lic[c]; l++) {
			if (assgn[form[c][l]] == sgn[c][l])
				++satlits;
			else if (assgn[form[c][l]] == UNASSIGNED) {
				++actlits;
			}
		}

		activelits[c] = actlits;
		satisflits[c] = satlits;

		if (satlits > 0)
			++num_clauses_sat;
		else {
			if (actlits == 0)
				++num_clauses_unsat;

			for (long l = 0; l < lic[c]; l++) {
				long v = form[c][l];
				if (assgn[v] == UNASSIGNED)
					varstats[v][sgn[c][l]]++;
			}

			if ((SAT_status != UNSATISFIED) && (actlits > 0))
				SAT_status = NEITHER_SAT_UNSAT;
			else
				SAT_status = UNSATISFIED;
		}

	}

	return SAT_status;

}




void print_ssat_numbers() {

	cout << "variables" << endl;
	//	cout << "# ---------" << endl;
	for (long v = 1 ; v <= numvars ; v++) {
		cout << setw(VARWIDTH) << v;
		if (chancevarprob[v] == CHOICEVAR)
			cout << "   -1.0";
		else
			cout << "   " << chancevarprob[v];
		cout << endl;
	}

	cout << endl << "clauses" << endl;
	//	cout << "# -------" << endl;
	for (long c = 0 ; c < numclauses ; c++) {
		//    cout << setw(VARWIDTH) << c+1 << " (";
		for (long l = 0 ; l < lic[c] ; l++) {
			if (sgn[c][l] == NOTNEGATED)
				cout << setw(VARWIDTH) << form[c][l];
			else
				cout << setw(VARWIDTH) << -form[c][l];
		}
		//    cout << "  )";
		cout << setw(VARWIDTH) << 0;
		cout << endl;
	}

}


void print_clause(long c) {

	cout << "( ";
	for (long l = 0; l < lic[c]; l++) {
		if (sgn[c][l] == NEGATED)
			cout << "-";
		//    cout << names[v] << "[";
		if (assgn[form[c][l]] == UNASSIGNED)
			cout << "U] ";
		else if (assgn[form[c][l]] == TRUE)
			cout << "T] ";
		else if (assgn[form[c][l]] == FALSE)
			cout << "F] ";
		else
			cout << "!!] ";
	}
	cout << ")" << endl;
}



// initializes an assignment array to all UNASSIGNED
//
void initassgn(long *a) {
	for (long v = 1; v <= numvars; v++)
		a[v] = UNASSIGNED;
}



// computes the floor of the log base 2 of n
//
// [used to return log(n)/log(2), but taking the floor of this
// was giving me a strange result:  when n was 8, the floor of
// the log base 2 of n was giving me 2.]
//
long floorlg (long n) {

	long flrlg2 = 0;
	while (n > 1) {
		n /= 2;
		++flrlg2;
	}

	return flrlg2;

}





