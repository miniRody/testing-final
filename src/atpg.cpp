/**********************************************************************/
/*           Constructors of the classes defined in atpg.h            */
/*           ATPG top-level functions                                 */
/*           Author: Bing-Chen (Benson) Wu                            */
/*           last update : 01/21/2018                                 */
/**********************************************************************/

#include "atpg.h"

void ATPG::test() {
    string vec;
    int current_detect_num = 0;
    int total_detect_num = 0;
    int total_no_of_backtracks = 0;  // accumulative number of backtracks
    int current_backtracks = 0;
    int no_of_aborted_faults = 0;
    int no_of_redundant_faults = 0;
    int no_of_calls = 0;

    fptr fault_under_test = flist_undetect.front();

    /* stuck-at fault sim mode */
    if (fsim_only) {
        fault_simulate_vectors(total_detect_num);
        in_vector_no += vectors.size();
        display_undetect();
        fprintf(stdout, "\n");
        return;
    }// if fsim only

    /* transition fault sim mode */
    if (tdfsim_only) {
        transition_delay_fault_simulation(total_detect_num);
        in_vector_no += vectors.size();
        display_undetect();

        printf("\n# Result:\n");
        printf("-----------------------\n");
        printf("# total transition delay faults: %d\n", num_of_tdf_fault);
        printf("# total detected faults: %d\n", total_detect_num);
        printf("# fault coverage: %lf %%\n", (double) total_detect_num / (double) num_of_tdf_fault * 100);
        return;
    }// if fsim only

    vector<string> test_patterns;
    vector<fptr> fault_list(flist_undetect.begin(), flist_undetect.end());
    int num_undetected = fault_list.size();
    int detected_fnum, gen_patterns = 0;
    bool verbose = false;

    for (fptr fault : fault_list) {
        if (fault->detect == TRUE)
            continue;
        total_attempt_num = detected_num - fault->detected_time;
        switch (podem(fault, test_patterns)) {
            case TRUE:
                for (string &vec : test_patterns) {
                    // TODO: Dynamic compression
                    rand_fill_unknown(vec);  // random fill unknown values
                    tdfault_sim_a_vector(vec, detected_fnum);
                    printf("T'%s'\n", vec.c_str());
                }
                gen_patterns += test_patterns.size();
                break;
            case FALSE:
                fault->detect = FALSE;
                if (verbose)
                    printf("#Undetectable fault!\n");
                break;
            case MAYBE:
                if (verbose)
                    printf("#Cannot find solution within time limit\n");
                break;
        }
        // cout << --num_undetected << " faults remaining\n";
    }

    display_undetect();
    fprintf(stdout, "\n");
    fprintf(stdout, "#number of generated test patterns = %d\n", gen_patterns);
    fprintf(stdout, "#number of aborted faults = %d\n", no_of_aborted_faults);
    fprintf(stdout, "#number of redundant faults = %d\n", no_of_redundant_faults);
    fprintf(stdout, "#number of calling podem1 = %d\n", no_of_calls);
}/* end of test */


/* constructor of ATPG */
ATPG::ATPG() {
    /* orginally assigned in tpgmain.c */
    this->backtrack_limit = 50;     /* default value */
    this->total_attempt_num = 1;    /* default value */
    this->fsim_only = false;        /* flag to indicate fault simulation only */
    this->tdfsim_only = false;      /* flag to indicate tdfault simulation only */
    this->compress_test = false;    /* flag to indicate whether to compress test */

    /* orginally assigned in input.c */
    this->debug = 0;                /* != 0 if debugging;  this is a switch of debug mode */
    this->lineno = 0;               /* current line number */
    this->targc = 0;                /* number of args on current command line */
    this->file_no = 0;              /* number of current file */

    /* orginally assigned in init_flist.c */
    this->num_of_gate_fault = 0; // totle number of faults in the whole circuit

    /* orginally assigned in test.c */
    this->in_vector_no = 0;         /* number of test vectors generated */
}

/* constructor of WIRE */
ATPG::WIRE::WIRE() {
    this->value = 0;
    this->level = 0;
    this->wire_value1 = 0;
    this->wire_value2 = 0;
    this->wlist_index = 0;
}

/* constructor of NODE */
ATPG::NODE::NODE() {
    this->type = 0;
    this->marked = false;
}

/* constructor of FAULT */
ATPG::FAULT::FAULT() {
    this->node = nullptr;
    this->io = 0;
    this->index = 0;
    this->fault_type = 0;
    this->detect = 0;
    this->test_tried = false;
    this->eqv_fault_num = 0;
    this->to_swlist = 0;
    this->fault_no = 0;
}

