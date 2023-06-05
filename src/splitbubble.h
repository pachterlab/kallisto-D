#ifndef SPLITBUBBLE_H
#define SPLITBUBBLE_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

namespace splitbubble {

  /* Function: compact_rules
   * 
   * Creates compaction "rules" from a list of sets of numbers.
   * The universe is consecutive numbers starting from 0 to some number N.
   * The rules are used downstream in the function: compact(), and in recapitulating the original (uncompressed) values.
   * This is an online phase where we can keep feeding in input to refine the rules.
   * The sets produced by this algorithm are referred to as "bubbles".
   * 
   * set_list: A vector of sets (each set containing numerical values) that we're going to compact
   * invbubble: An inverted index mapping each item (indices) to a bubble ID (values)
   * bubbles: Maps each bubble ID (indices) to the actual bubble/set (values)
   * verbose: If true, output status after each iteration to stdout
   * 
   * Note: invbubble and bubbles need not be empty; they can be updated from a previous iteration of the algorithm
   */
  template <typename T>
  void compact_rules(const std::vector<T>& set_list, std::vector<size_t>& invbubble, std::vector<T>& bubbles, bool verbose = false) {
    for (size_t curr_set_i = 0; curr_set_i < set_list.size(); curr_set_i++) { 
      const auto &curr_set = set_list[curr_set_i]; // curr_set = current set we're examining
      std::unordered_map<size_t, T> new_spaces;
      std::unordered_set<size_t> empty_bubbles_list;
      for (auto i : curr_set) { // Go through all items in current set
        size_t new_space_identifier;
        if (i < invbubble.size() && invbubble[i] != -1) { // A bubble containing item i already exists
          // Get bubble ID associated with item i
          auto bubble_i = invbubble[i];
          // Get actual bubble/set associated with that bubble ID
          auto& bubble = bubbles[bubble_i];
          // Remove item i from bubble
          bubble.remove(i);
          // ... and prepare to place i into a new space (with space identifier being the bubble ID)
          new_space_identifier = bubble_i;
          if (bubble.isEmpty()) { // Uh oh, we have an empty bubble
            empty_bubbles_list.insert(bubble_i); // We'll deal with empty bubbles later
          }
        } else { // A bubble containing item i does NOT already exist
          // Prepare to add item i to a new space
          new_space_identifier = bubbles.size(); // Space identifier being a bubble ID that doesn't already exist
        }
        new_spaces[new_space_identifier].add(i); // Add item i to the designated new space
      }
      // Now, aliquot all the new space contents among the empty bubbles or into new bubbles
      for (auto &it : new_spaces) {
        auto& bubble_items = it.second;
        size_t bubble_i; // The bubble ID that we're gonna place into
        if (empty_bubbles_list.size() != 0) { // We have some empty bubbles
          bubble_i = *(empty_bubbles_list.begin()); // Bubble ID of an empty bubble
          empty_bubbles_list.erase(bubble_i); // Bubble will no longer empty since we're gonna fill it up
        } else { // No more empty bubbles, so create a new bubble
          bubble_i = bubbles.size();
          bubbles.resize(bubbles.size()+1);
        }
        bubbles[bubble_i] |= bubble_items; // Add items from a new space into the bubble
        bubbles[bubble_i].runOptimize(); // Save space
        bubbles[bubble_i].shrinkToFit(); // Save additional space
        for (auto i : bubble_items) { // Iterate through items of the current new space
          invbubble.resize(i >= invbubble.size() ? (i+1) : invbubble.size(), -1); // Make sure inverted map is big enough to accomodate everything
          invbubble[i] = bubble_i; // Update bubble ID for all items in the current new space
        }
      }
      if (verbose) {
        std::cout << "Iteration: " << std::to_string(curr_set_i) << "\n";
        for (size_t i = 0; i < bubbles.size(); i++) {
          std::cout << "Bubble " << std::to_string(i) << ":";
          for (auto y : bubbles[i]) {
            std::cout << " " << y;
          }
          std::cout << "\n";
        }
        std::cout << std::endl;
      }
    }
  }

  /* 
   * Function: getNum
   * 
   * Returns the total number of elements across all sets
   */
  template <typename T>
  size_t getNum(const std::vector<T>& set_list) {
    size_t num_elems = 0;
    for (auto x : set_list) {
      num_elems += x.cardinality();
    }
    return num_elems;
  }

  /*
   * Function: compact
   * 
   * Uses the "rules" returned by function compact_rules to do the compaction.
   * Afterwards, the numbers within each set in set_list are numbers 
   * corresponding to bubbles and the contents of bubbles (which contain the 
   * original numbers), can be "uncompacted" by examining the bubbles from 
   * compact_rules (e.g. bubbles[n] yields the set of the original numbers 
   * that have been compacted in the bubble with bubble id: n).
   * 
   * set_list: The list of sets that will be compacted -- this should be the 
   * same set supplied to compact_rules. Note that set_list will be modified in-place.
   * invbubble: The "rules" (the inverted index) returned by compact_rules
   * verbose: If true, outputs the compaction results at each step
   * 
   */
  template <typename T>
  void compact(std::vector<T>& set_list, std::vector<size_t>& invbubble, bool verbose = false) {
    for (size_t curr_set_i = 0; curr_set_i < set_list.size(); curr_set_i++) {
      T compacted_set;
      for (auto i : set_list[curr_set_i]) { // Go through all items in current bubble
        compacted_set.add(invbubble[i]);
      }
      compacted_set.shrinkToFit(); // Save space
      compacted_set.runOptimize(); // Save additional space
      set_list[curr_set_i] = std::move(compacted_set);
      if (verbose) {
        std::cout << "Set " << std::to_string(curr_set_i) << ":";
        for (auto y : set_list[curr_set_i]) {
          std::cout << " " << y;
        }
        std::cout << std::endl;
      }
    }
  }
}


#endif // SPLITBUBBLE_H
