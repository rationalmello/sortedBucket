/**
 * @file parity.cpp
 * 
 * @author Gavin Dan (xfdan10@gmail.com)
 * @brief Tests for sortedBucket implementations for correctness
 * @version 1.1
 * @date 2023-11-03
 * 
 * 
 * Inputting large dataset (10^6 integers) and checking for correctness
 * with std::vector as a guarantee
 */


#include <cassert>
#include <iostream>
#include <random>
#include "sortedBucketRBT.h"
#include "sortedBucketLL.h"
#include "sortedBucketVV.h"

/* Number of operations for test. Recommended 10^4 in Debug or 10^5 in Release,
    otherwise it uses too much memory and page faults take a lot of time.
    Also, ops above 190,000 cause ASan container overflow error and breaks test
    on my machine. */
constexpr size_t ops = 190000;

/* Mersenne Twister random number gen */
static std::mt19937_64 random{uint64_t(rand())};


using std::cout;
using std::endl;
using std::vector;

int main() {
    cout << "Starting tests " << endl;
    SortedBucketRBT<int> rbt;
    SortedBucketVV<int> vv;
    SortedBucketLL<int> ll;
    vector<int> in, out;
    in.reserve(ops);
    out.reserve(ops);

    /* Populate sortedBuckets with random numbers */
    for (size_t i = 0; i < ops; ++i) {
        int rand = random();
        in.emplace_back(rand);
        rbt.insert(rand);
        vv.insert(rand);
        ll.insert(rand);
    }
    cout << "Random numbers inserted " << endl;

    std::sort(in.begin(), in.end());

    /* Test RBT */
    cout << "Entering test for RBT" << endl;
    int last = 0;
    for (auto it = rbt.begin(); it != rbt.end(); ++it) {
        out.emplace_back(*it);
    }
    for (int i = 0; i < in.size(); ++i) {
        if (in[i] != out[i]) {
            cout << "Mismatched RBT at index " << i << ", actual val " << in[i] 
            << " but claimed " << out[i] << endl;
        }
        if (in[i] != last) {
            /* Check distance, first of every group of identical ints has valid distance */
            if (i != rbt.findWithDistance(in[i]).second) {
                cout << "Mismatched RBT at index " << i << ", actual dist " << i
                << " but claimed " << rbt.findWithDistance(in[i]).second << endl;
            }
        }
        last = in[i];
    }
    cout << "Done test for RBT insertion" << endl;

    out.clear();
    last = 0;
    /* Test VV */
    cout << "Entering test for VV" << endl;
    for (auto it = vv.begin(); it != vv.end(); ++it) {
        out.emplace_back(*it);
    }
    for (int i = 0; i < in.size(); ++i) {
        if (in[i] != out[i]) {
            cout << "Mismatched VV at index " << i << ", actual val " << in[i] 
            << " but claimed " << out[i] << endl;
        }
        if (in[i] != last) {
            /* Check distance, first of every group of identical ints has valid distance */
            if (i != vv.findWithDistance(in[i]).second) {
                cout << "Mismatched VV at index " << i << ", actual dist " << i
                << " but claimed " << vv.findWithDistance(in[i]).second << endl;
            }
        }
        last = in[i];
    }
    cout << "Done test for VV insertion" << endl;

    out.clear();
    last = 0;
    /* Test LL */
    cout << "Entering test for LL" << endl;
    for (auto it = ll.begin(); it != ll.end(); ++it) {
        out.emplace_back(*it);
    }
    for (int i = 0; i < in.size(); ++i) {
        if (in[i] != out[i]) {
            cout << "Mismatched LL at index " << i << ", actual val " << in[i] 
            << " but claimed " << out[i] << endl;
        }
        if (in[i] != last) {
            /* Check distance, first of every group of identical ints has valid distance */
            if (i != ll.findWithDistance(in[i]).second) {
                cout << "Mismatched LL at index " << i << ", actual dist " << i
                << " but claimed " << ll.findWithDistance(in[i]).second << endl;
            }
        }
        last = in[i];
    }
    cout << "Done test for LL insertion" << endl;


    cout << "Done all tests" << endl;
    return 0;
}