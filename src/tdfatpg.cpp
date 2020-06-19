#include "atpg.h"
#include <assert.h>
#include <limits.h>
#include <random>
#include <queue>
#include <stack>
#include <array>
#include <set>
#include <unordered_set>

#define CONFLICT 2
#define BACKTRACK_LIMIT 100

/* Assign values to PI such that object_wire has value of object_level */
bool ATPG::achieve_objective(const wptr object_wire, const int object_level) {
    vector<wptr> PI_wires;
    stack<wptr> decision_state; // Try 0 first, and then 1
    wptr decide_wire;
    int backtracks = 0;

    switch (backward_imply(object_wire, object_level)) {
        case TRUE:
            sim();
            break;
        case FALSE:
            break;
        case CONFLICT:
            return false;
    }
    if (object_wire->value == U)
        get_wire_support(object_wire, PI_wires);
    else
        return object_wire->value == object_level;
    do {
        sim();
        /* Go to next state */
        if (object_wire->value == U) { // Make decision
            decide_wire = PI_wires[decision_state.size()];
            decide_wire->value = 0;
            decide_wire->set_changed();
            decision_state.push(decide_wire);
        }
        else if (object_wire->value != object_level) {
            while (!decision_state.empty() && decision_state.top()->value == 1) {
                decision_state.top()->value = U;
                decision_state.top()->set_changed();
                decision_state.pop();
            }
            if (!decision_state.empty()) {
                decision_state.top()->value = 1;
                decision_state.top()->set_changed();
            }
            backtracks++;
        }
        else
            return true;
    } while (!decision_state.empty() && backtracks < BACKTRACK_LIMIT);
    return false;
}

 /* Assign value to PI such that the transition fault is activated.
  * Return the test pattern in string format. */
string ATPG::find_V1_pattern(fptr td_fault) {
    wptr faulty_wire = sort_wlist[td_fault->to_swlist];
    int shifted_in_bit;
    string ret_val;

    ckt_snapshot(0);

    for (int i = 0; i < cktin.size(); ++i) { // Convert D/D_bar to good value
        if (cktin[i]->value == D)
            cktin[i]->value = 1;
        else if (cktin[i]->value == D_bar)
            cktin[i]->value = 0;
    }
    // For PIs: reverse shift scan chain regs value
    // For internal wires: set value to unknown
    // Useful fact: cktin[0] is the entry point of scan chain
    shifted_in_bit = cktin[0]->value;
    for (int i = 0; i < cktin.size(); ++i) {
        if (i != cktin.size() - 1)
            cktin[i]->value = cktin[i+1]->value;
        else
            cktin[i]->value = U;
        cktin[i]->set_changed();
    }
    for (int i = 0; i < sort_wlist.size(); ++i) {
        if (i >= cktin.size()) {
            sort_wlist[i]->value = U;
            sort_wlist[i]->remove_changed();
        }
    }
    sim();
    if (td_fault->fault_type == STR && faulty_wire->value == TRUE ||
        td_fault->fault_type == STF && faulty_wire->value == FALSE)
        ret_val = "";
    else {
        ret_val = "";
        if (achieve_objective(faulty_wire, td_fault->fault_type)) {
            for (int i = 0; i < cktin.size(); ++i)
                ret_val += itoc(cktin[i]->value);
            ret_val += itoc(shifted_in_bit);
        }
    }
    ckt_snapshot(1);
    return ret_val;
}

/* mode=0: backup the status of the circuit
 * mode=1: restore circuit status */
void ATPG::ckt_snapshot(int mode) {
    static pair<int, bool> *backup = nullptr;

    if (backup == nullptr)
        backup = new pair<int, bool>[sort_wlist.size()];

    if (mode == 0) {
        for (int i = 0; i < sort_wlist.size(); i++) {
            backup[i].first = sort_wlist[i]->value;
            backup[i].second = sort_wlist[i]->is_changed();
        }
    }
    else if(mode == 1) {
        for (int i = 0; i < sort_wlist.size(); i++) {
            sort_wlist[i]->value = backup[i].first;
            if (backup[i].second)
                sort_wlist[i]->set_changed();
            else
                sort_wlist[i]->remove_changed();
        }
    }
}

/* Collect PIs which have a path with all unknown values to the given wire */
void ATPG::get_wire_support(wptr ckt_wire, vector<wptr> &supp_wires) {
    unordered_set<wptr> visited_wire;
    queue<wptr> visit_queue;

    supp_wires.clear();
    if (ckt_wire->value == U)
        visit_queue.push(ckt_wire);
    while (!visit_queue.empty()) {
        wptr ckt_wire = visit_queue.front();

        if (ckt_wire->is_input())
            supp_wires.push_back(ckt_wire);
        else {
            for (wptr w : ckt_wire->inode.front()->iwire) {
                if (w->value == U && visited_wire.find(w) == visited_wire.end()) {
                    visit_queue.push(w);
                    visited_wire.insert(w);
                }
            }
        }
        visit_queue.pop();
    }
}

void ATPG::rand_fill_unknown(string& pattern) {
    int pos, num_U_value = 0;
    unordered_set<string> cand_pattern;
    mt19937 mt(87);
    uniform_int_distribution<int> dist(INT_MIN, INT_MAX);
    
    for (char bit : pattern)
        if (bit == '2')
            num_U_value++;
    if (num_U_value == 0)
        return;
    // Derive the patterns we would like to simulate
    if (num_U_value > 7) {
        while (cand_pattern.size() < 200) {
            string unfilled = pattern;
            for (auto str_iter = unfilled.begin(); str_iter != unfilled.end(); ) {
                int random_num = dist(mt);
                for (int i = 31; i >= 0; i--) {
                    // Let the iterator shift to next unknown value
                    while (str_iter != unfilled.end() && *str_iter != '2')
                        str_iter++;
                    if (str_iter != unfilled.end())
                        *str_iter = ((random_num >> i) & 1)? '1':'0';
                    else
                        break;
                }
            }
            cand_pattern.insert(unfilled);
        }
    }
    else {
        list<string> before_fill(1, pattern);
        for (int i = 0; i < num_U_value; i++) {
            list<string> after_fill;
            for (string patt : before_fill) {
                for (pos = 0; pos < patt.length(); pos++)
                    if (patt[pos] == '2')
                        break;
                patt[pos] = '0';
                after_fill.push_back(patt);
                patt[pos] = '1';
                after_fill.push_back(patt);
            }
            before_fill.clear();
            before_fill.splice(before_fill.begin(), after_fill);
        }
        for (string const &filled : before_fill)
            cand_pattern.insert(filled);
    }

    int detect_num, max_detect_num = 0;
    for (string vec : cand_pattern) {
        tdfault_sim_a_vector(vec, detect_num, 1);
        if (detect_num > max_detect_num) {
            max_detect_num = detect_num;
            pattern = vec;
        }
    }
}
