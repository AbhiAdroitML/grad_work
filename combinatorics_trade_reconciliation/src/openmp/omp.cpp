#include <iostream>
#include <vector>
#include "rapidcsv.h"
#include <cstdio>
#include<bits/stdc++.h>
#include<stdio.h>
#include <chrono>
#include <set>
#include <experimental/iterator>
#include <omp.h>
#include <numeric>
#include <random>
#include <algorithm>

using namespace std;

// Combination Function `k`
// Since Itertools like library was not available in C++, used this function to generate combinations
// Source:https://www.techiedelight.com/find-distinct-combinations-given-length-2/
void getCombinations(vector <float>const &trades, int i, int k,
                     set<vector < float>>&subset,vector<float> &out) {
// empty set
if (trades.size()== 0) {
return;
}

// base case: combination size is `k`
if (k == 0) {subset.insert(out);
return;
}

// return if no more elements are left
if (i == trades.size()) {
return;
}

// include the current element in the current combination and recur
out.push_back(trades[i]);
getCombinations(trades, i+ 1, k - 1, subset, out);

// exclude the current element from the current combination
out.pop_back();        // backtrack

// exclude the current element from the current combination and recur
getCombinations(trades, i+ 1, k, subset, out);
}

// Function to print combination vectors
template<typename T>
void comboPrint(vector < T >
const &input) {
cout <<fixed;
cout << "Combo Found [";
copy(begin(input),
        end(input),
        experimental::make_ostream_joiner(cout, ", ")
);
cout << "]\n";
}


int main() {
    // Library to read CSV file
    // Source: https://github.com/d99kris/rapidcsv

    rapidcsv::Document doc("./largest_trades.csv");
    std::vector<float> col = doc.GetColumn<float>("Amount");
    std::cout << "Read " << col.at(10) << " values." << std::endl;
    std::vector<float> Amount(col.size());

    for (int i = 0; i < col.size(); i++) {
        Amount[i] = col.at(i);
    }

    int combo_total = 0;                  //* Counter to keep track of total combinations found by each thread
    vector<float> value_found;            //* Saves all unique digits found in combinations
    int th_id;                            //* Thread ID

#pragma omp parallel private(th_id) shared(value_found)
    {
        //omp_lock_t lck;
        //omp_init_lock(&lck);

#pragma omp for nowait
        for (int k = 2; k < 24; k++) {
            th_id = omp_get_thread_num();
            int combo_found = 0;
            int unique_combo = 0;
            printf("k = %d assigned to thread_id = %d\n", k, th_id);

            // split the file and iterate randomly 10000 times
            for (int j = 0; j < 10000; j++) {
                vector<float> Amount_chunk;
                unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                std::shuffle(Amount.begin(), Amount.end(), std::default_random_engine(seed));
                // Take only 100 random trades 
                for (int i = 0; i < 100; i++) {
                    Amount_chunk.push_back(Amount[i]);
                }

                set<vector < float>>subset;     //* Vector set to store all combinations
                vector<float> out;              //* Vector to store a combination
                getCombinations(Amount_chunk, 0, k, subset, out);

                // check subset for sum in range -1 and 1
                for (auto const &vec: subset) {

                    double sum_of_elems = 0;
                    for (auto &n: vec) {
                        sum_of_elems += n;
                    }
                    if (sum_of_elems >= -1.0 && sum_of_elems <= 1) {
                        combo_found += 1;
                        int counter = 0;       //* keep track of unique elements

                        // Check if the element was used in any previous combination or
                        // not (assuming no duplicate numbers)
#pragma omp critical
                        {
                            for (auto &n: vec) {

                                if (std::find(value_found.begin(), value_found.end(), n) != value_found.end()) {
                                    counter = 0;   // element found in vector
                                    break;
                                } else {
                                    counter = 1;   // element not found in vector
                                }
                            }
                            // if the combination is unique i.e. no element in combo used previously,
                            // we add it in the value found vector
                            if (counter == 1) {
                                unique_combo += 1;
                                sum_of_elems = 0;
                                // only print unique combination by each thread

                                comboPrint(vec);
                                printf("sum of elems %f \n", sum_of_elems);

                                for (auto &n: vec) {
                                    value_found.push_back(n);
                                    Amount.erase(std::remove(Amount.begin(), Amount.end(), n), Amount.end());
                                    printf("Dropped 1 element from amount vector.New Size : %d\n", Amount.size());
                                }
                            }
                        }

                    }
                }
                printf("Iteration=%d, Thread ID=%d, k=%d, Total combinations=%d, Unique combinations=%d\n", j, th_id, k,
                       combo_found, unique_combo);
            }
            printf("Final counts: Thread ID=%d, k=%d, Total combinations=%d, Unique combinations=%d\n", th_id, k,
                   combo_found, unique_combo);
        }

    }

    return 0;
}  

