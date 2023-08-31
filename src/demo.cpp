/**
 * @file demo.cpp
 * 
 * @author Gavin Dan (xfdan10@gmail.com)
 * @brief Demo for functionality of Sorted Containers
 * @version 1.0
 * @date 2023-08-27
 * 
 * 
 * Demonstrating the usage of the different implementations of Sorted Containers.
 * Prints out contents after operations.
 * 
 * Ensure DEBUG is defined in all implementation headers to compile print functs.
 * 
 */

#include "sortedBucketRBT.h"
#include "sortedBucketLL.h"
#include "sortedBucketVV.h"

#define DEMO_RBT    // Prints out RBT implemenation demo
#define DEMO_LL     // Prints out LL implemenation demo
#define DEMO_VV     // Prints out VV implemenation demo

#ifdef DEBUG
using namespace std; // only for the demo, sue me :)

int main() {
    vector<int> v1 {1, 2, 3};
    vector<int> v2 {4, 5, 6, 7, 8};
    vector<int> v3;
    for (int i = 0; i < 20; ++i)
        v3.emplace_back(i*2);

#ifdef DEMO_RBT // Red-Black Tree implementation
    cout << "After range constructing SortedBucketRBT: " << endl;
    SortedBucketRBT<int> rbt(v3.begin(), v3.end());
    rbt.print();

    cout << "After erasing some nodes: " << endl;
    rbt.erase(24);
    rbt.erase(26);
    rbt.erase(28);
    rbt.print();

    cout << "After erasing the root: " << endl;
    rbt.erase(14);
    rbt.print();

    cout << "After inserting some nodes: " << endl;
    rbt.insert(v1.begin(), v1.end(), 3);
    rbt.insert(16, 3);
    rbt.insert(17);
    rbt.insert(18);
    rbt.print();

    for (int n:v1) {
        cout << "idx of first occurence of " << n << " is " << rbt.distance(n) 
        << endl;
    }
    cout << "idx of first occurence of " << 4 << " is " << rbt.distance(4) 
    << endl;
    cout << "idx of first occurence of " << 6 << " is " << rbt.distance(6) 
    << endl;
    cout << endl;
#endif // DEMO_RBT


#ifdef DEMO_LL // List of lists implementation
    cout << "After range constructing SortedBucketLL: " << endl;
    SortedBucketLL<int> ll(v3.begin(), v3.end());
    ll.print();
    SortedBucketLL<int> ll2(move(ll));

    cout << "After changing density of move-constructed SortedBucketLL2: " << endl;
    ll2.forceDensity(4); // force a small density to observe balancing
    ll2.print("SortedBucketLL Number 2");
    ll2.erase(12);
    ll2.print("SortedBucketLL Number 2");

    int llDist1 = ll2.distance(10), llDist2 = ll2.distance(11);
    if (llDist1 != -1) 
        cout << "the sorted index of 10 is " << llDist1 << endl;
    else 
        cout << "10 is not present" << endl;
    if (llDist2 != -1) 
        cout << "the sorted index of 11 is " << llDist2 << endl;
    else 
        cout << "11 is not present" << endl;

    auto [llBucket, llTarg] = ll2.find(15);
    if (llBucket == ll2.getBucketsEnd())
    /*  Bandaid fix to access end of private buckets until iterator 
        interface is made */
        cout << "15 is not present" << endl;
    else 
        cout << "15 found with val " << *llTarg << endl;
    cout << endl;
#endif // DEMO_LL


#ifdef DEMO_VV // Vector of vectors implementation
    cout << "After range constructing SortedBucketVV: " << endl;
    SortedBucketVV<int, less<int>> vv(v3.begin(), v3.end());
    vv.insert(12); 
    /* duplicate element 12 is distinct and separate. In another version 
    I can match RBT implementation and store copies of each value in a
    single element */
    vv.print();

    cout << "After resizing avg bucket density to 3: " << endl;
    vv.forceDensity(3);
    vv.print();

    cout << "After inserting into largest bucket " << endl;
    vv.insert(50);
    vv.insert(55);
    vv.print();

    cout << "After erasing some elements" << endl;
    vv.erase(6);
    vv.erase(8);
    vv.eraseAll(12);
    vv.erase(14);
    vv.print();
    
    /*  Note that bucket 1 contains only one element, which is expected because
        we resize if a bucket contains strictly less than floor(density/2). */

    int vvDist1 = vv.distance(19);
    int vvDist2 = vv.distance(20);
    if (vvDist1 != -1) 
        cout << "the sorted index of 19 is " << vvDist1 << endl;
    else 
        cout << "19 is not present" << endl;
    if (vvDist2 != -1) 
        cout << "the sorted index of 20 is " << vvDist2 << endl;
    else 
        cout << "20 is not present" << endl;
    cout << endl;

    auto [vvBucket, vvTarg] = vv.find(30);
    if (vvBucket == vv.getBucketsEnd()) 
    /*  Bandaid fix to access end of private buckets until iterator 
        interface is made */
        cout << "30 is not present" << endl;
    else 
        cout << "30 found with val " << *vvTarg << endl;
    cout << endl;

#endif // DEMO_VV

    return 0;
}
#endif // DEBUG