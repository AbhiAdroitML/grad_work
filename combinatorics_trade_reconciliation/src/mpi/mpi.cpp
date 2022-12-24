#include <iostream>
#include <vector>
#include "rapidcsv.h"
#include <cstdio>
#include<stdio.h>
#include <set>
#include <experimental/iterator>
#include <mpi.h>
#include <numeric>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

// Combination Function `k`
// Since Itertools like library was not available in C++, used this function to generate combinations
// Source:https://www.techiedelight.com/find-distinct-combinations-given-length-2/
void getCombinations(vector <float>const &trades, int i,int k,set<vector < float>>&subset,vector<float> &out)
{
// empty set
if (trades.size()== 0) {
return;
}

// base case: combination size is `k`
if (k == 0) {
subset.insert(out);
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

// Function to print vectors
template<typename T>
void comboPrint(vector < T >
const &input)
{
cout <<fixed;
cout << "[";
copy(begin(input),
        end(input),
        experimental::make_ostream_joiner(cout, ", "));
cout << "]\n";
}

int main() {
    // Library to read CSV file
    // Source: https://github.com/d99kris/rapidcsv
    rapidcsv::Document doc("./largest_trades.csv");

    std::vector<float> col = doc.GetColumn<float>("Amount");
    std::vector<float> Amount(col.size());

    for (int i = 0; i < col.size(); i++) {
        Amount[i] = col.at(i);
    }

    // Run this J times to reduce the number of elements to search
        // Initialize MPI.

        MPI_Init(nullptr, nullptr);

        int my_rank = -1;
        int n_ranks = 0;

        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
    // PARAMETER TO TUNE	
    for (int j = 0; j < 10000; j++) {

        // Distribution of elements among the ranks
        int M = Amount.size() / n_ranks;  // number of elements per rank
        int r = Amount.size() % n_ranks;  // number of elements remaining

        // Each rank is given one M elements to find the subset sum
        int start_vec = my_rank * M;
        int end_vec = start_vec + M;

        // Passing the remaining elements to the last rank
        int is_last_rank = my_rank + 1;
        if (is_last_rank == n_ranks) {
            end_vec += r;
        }

        vector<float> amount_myrank;                       //* Amount assigned to local rank
        //printf("my_rank=%d, start=%d, end=%d \n", my_rank, start_vec, end_vec);
        for (int i = start_vec; i < end_vec; i++) {
            amount_myrank.push_back(Amount[i]);            //* Local distribution
        }

        //printf("Iteration=%d, rank=%d, Amount file size=%d\n", j, my_rank, amount_myrank.size());
 
        vector<float> value_found;                         //* Store unique elements which are found in a matching combo
        vector<int> count_of_values_found(1);           //* Number of unique elements found in matching combo
        vector<int> count_of_values_found_all(n_ranks); //* Number of unique elements shared with all ranks
        vector<int> displacement(n_ranks);              //* Displacement vector used in gatherv
        int combo_total = 0;                               //* Keep track of total combinations found by the rank
        int combo_unique = 0;                              //* Keep track of only unique combinations - no overlapping numbers
        for (int k = 2; k < M; k++) {                      //* Search in space of k = 2 to number of elements in amount vector (PARAMETER TO TUNE)

            // set to store all combinations
            set<vector < float>>subset;

            // vector to store a combination
            vector<float> out;
            //printf("Iteration=%d, k=%d, Amount file size=%d\n", j, k, amount_myrank.size());
            getCombinations(amount_myrank, 0, k, subset, out);

            int combo_found = 0; // keep track of combo found by rank and selected k
            // check subset for sum in range -1 and 1
            for (auto const &vec: subset) {

                // Step 1: find the sum of subset
                double sum_of_elems = 0;
                sum_of_elems = std::accumulate(vec.begin(), vec.end(), 0.0);

                // Step 2: if sum is in range then
                if (sum_of_elems >= -1.0 && sum_of_elems <= 1) {

                    //printf("Combo in range found by rank=%d, k=%d", my_rank, k);  //* uncomment to see the detailed results
                    //comboPrint(vec);
                    
		            combo_found += 1;
                    combo_total += 1;
                    // Step 3: add the elements in another new vector
                    // - this vector will keep track of all new elements that become part of a
                    // combination which is in range of -1 and 1
                    // Example: [ -2, 3] , [-4 3] then unique elements are [-2 and 3] , the second combination
                    // will be ignored because 3 is already there in first combination

                    int counter = 0; //* keep track of unique elements

                    // Step 4: we check if the element was used in any previous combination or
                    // not (assuming no duplicate numbers)
                    for (auto &n: vec) {

                        if (std::find(value_found.begin(), value_found.end(), n) != value_found.end()) {
                            counter = 0;   //* element found in vector
                            break;
                        }
                        else {
                            counter = 1;   //* element not found in vector
                        }
                    }
                    // if the combination is unique i.e. no element in combo used previously,
                    // we add it in the new vector
                    if (counter == 1) {
                        combo_unique += 1;
                        printf("Unique Combo in range found by iteration=%d, rank=%d, k=%d", j, my_rank, k);
                        comboPrint(vec);
                        for (auto &n: vec) {
                            value_found.push_back(n);
                            count_of_values_found[0] += 1;
                        }
                    }

                }

            }

             // only print where combination is found to do downstream analysis on this
            if (combo_found >= 1) {
                printf("Total combinations found by rank=%d, k=%d: %d\n", my_rank, k, combo_found);
            }
        }

        // prints all possible combinations in range (overlaps included)
        if (combo_total >=1){
            printf("Total combinations found by rank=%d, iteration=%d: %d\n", my_rank, j, combo_total);
        }

        // Gathering the count of unique values in each rank
        MPI_Allgather(count_of_values_found.data(), 1, MPI_INT,
                      count_of_values_found_all.data(), 1, MPI_INT, MPI_COMM_WORLD);

        for (int i = 1; i < n_ranks; i++) {
            displacement[i] = displacement[i - 1] + count_of_values_found_all[i - 1];
        }

        int sum_of_elems_values = std::accumulate(count_of_values_found_all.begin(),
                                                  count_of_values_found_all.end(),
                                                  0); //* Total number of unique values (across all ranks)

        std::vector<float> values_root(sum_of_elems_values);  //* Total unique values from all ranks

        // Gathering the unique values at each rank
        MPI_Allgatherv(value_found.data(),
                    count_of_values_found[0],
                    MPI_FLOAT,
                    values_root.data(),
                    count_of_values_found_all.data(),
                    displacement.data(),
                    MPI_FLOAT, MPI_COMM_WORLD);

        // all unique combination found in the iteration
        int total_combo_unique = 0;
        MPI_Reduce(&combo_unique, &total_combo_unique, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // Delete the previously found values from the Amount vector

        vector<float> result;
        std::sort(Amount.begin(), Amount.end());
        std::sort(values_root.begin(), values_root.end());

        set_difference(Amount.begin(), Amount.end(), values_root.begin(), values_root.end(),
                           inserter(result, end(result)));

	unsigned seed = std::chrono::system_clock::now()
            .time_since_epoch()
            .count();

        std::shuffle(result.begin(), result.end(), std::default_random_engine(seed));

        // Replace Amount vector with new short vector of amounts after removing unique values already found
        // in matching combination
        Amount.clear();
        for (int i = 0; i< result.size(); i++){
                Amount.push_back(result[i]);
        }
        if (my_rank == 0) {
		printf("Iteration=%d - unique combinations found=%d - New File size=%d\n", j, total_combo_unique, Amount.size());
        }

    }
    MPI_Finalize();
    return 0;
}
