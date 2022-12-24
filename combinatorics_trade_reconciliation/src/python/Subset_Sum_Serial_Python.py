# Required Libraries
from itertools import combinations
import pandas as pd
import time
start = time.time()

larget_trades = pd.read_csv("./largest_trades_sorted.csv")
#larget_trades = pd.read_csv("./largest_trades.csv")
print(f"Number of trades in file: {len(larget_trades)}, Unique:{larget_trades.Amount.nunique()}")

# Serial code to find combinations
target_low = -1     # min range
target_high = 1     # max range

count_combo_found = 0       # keep count of total combinations found
unique_set = set()          # keep unique digits found in previous combinations
count_unique_combo = 0      # keep count of total unique combinations

trades= larget_trades['Amount'].tolist()
for k in range(2, len(trades), 1):
    for combo in combinations(trades,k):
        if target_low <=sum(combo)<= target_high:
            #print(combo, sum(combo))
            count_combo_found += 1
            # insert the fist combination values in empty set
            if count_combo_found == 1:
                for item in combo:
                    unique_set.add(item)
            # check if other combinations have overlapping digits with previous combinations
            combo_set = set(combo)
            if len(unique_set.intersection(combo_set)) == 0 or count_combo_found == 1:
                count_unique_combo += 1
                for item in combo:
                    unique_set.add(item)
                print(f"Unique {count_unique_combo}: {combo},sum = {round(sum(combo),4)}", flush =True)
                print(f"Total combinations until now: {count_combo_found}, Time Elapsed: {time.time()-start}")	

print(f"Total combinations: {count_combo_found}\n",
      f"Total unique combinations:{count_unique_combo}\n",
      f"Total digits covered in unique combinations:{len(unique_set)}", flush=True)
