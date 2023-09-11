/**
 * @file sortedBucketVV.h
 * 
 * @author Gavin Dan (xfdan10@gmail.com)
 * @brief Implementation of Sorted Bucket container using std::vector of vectors
 * @version 1.0
 * @date 2023-08-27
 * 
 * 
 * Container provides sub-linear time lookup, distance, insertion, deletion.
 * Credit to Grant Jenks for his SortedContainers idea for Python (found at 
 * https://grantjenks.com/docs/sortedcontainers/sortedlist.html) which I
 * used as an inspiration.
 * 
 * Time complexities:
 *      find:       O(sqrt(n))
 *      distance:   O(sqrt(n))
 *      insert:     O(sqrt(n))
 *      erase:      O(sqrt(n))
 * 
 * @todo bidirectional iterator interface
 */

#ifndef UTIL_SORTED_BUCKET_VV_H
#define UTIL_SORTED_BUCKET_VV_H

/* 
    The default bucket size. Note this cannot be a template argument because it 
    would invalidate the move constructor taking a SortedBucketLL with a 
    different bucket size.
*/
#define DefaultSmallDensity (size_t(500))

#include <functional>
#include <math.h>

#define DEBUG
#ifdef DEBUG
#include <iostream>
#include <string>
#endif

template <typename T,
          typename Comp     = std::less<T>,
          typename Alloc    = std::allocator<T>>
class SortedBucketVV {
public:
    SortedBucketVV() {
        init();
    }

    SortedBucketVV(const SortedBucketVV<T, Comp>& old) {
        init();
        buckets = old.buckets;
        sz = old.sz;
        capacity = old.capacity;
        bucketDensity = old.bucketDensity;
    }

    SortedBucketVV(SortedBucketVV<T, Comp>&& old) noexcept {
        buckets.swap(old.buckets);
        sz = old.sz;
        capacity = old.capacity;
        bucketDensity = old.bucketDensity;
    }

    explicit SortedBucketVV(size_t cap)
        : capacity(cap)
        , bucketDensity(std::max(DefaultSmallDensity, 
                        static_cast<size_t>(std::sqrt(cap)))) {init();}
    
    template<class InputIterator>
    SortedBucketVV(InputIterator beginIt, InputIterator endIt, size_t cap = 25000)
        : capacity(cap)
        , bucketDensity(std::max(DefaultSmallDensity, 
                                 static_cast<size_t>(std::sqrt(cap)))) {
        init();
        for (InputIterator it = beginIt; it != endIt; ++it) {
            insert(*it);
        }
    }
    
    size_t size() const {
        return sz;
    }

    size_t getDensity() const {
        return bucketDensity;
    }

    /*
        changeCapacity() makes the container aware of the intended capacity 
        and rebalances all buckets. This is equivalent to a rehash and potentially
        invalidates all iterators.
    */
    void changeCapacity(size_t cap) {
        bucketDensity = std::max(DefaultSmallDensity, 
                                 static_cast<size_t>(std::sqrt(cap)));
        if (sz > 0) {
            typename std::vector<std::vector<T>>::iterator b = buckets.begin();
            while (b < buckets.end()) {
                /*  Guaranteed no empty buckets if sz > 0, so track the first elem
                    of each bucket. For vector implementation, if we shift the 
                    first elem to the right, then the current b iterator may
                    point to the next bucket if the old one was undersized and 
                    got merged (since vector iterators are equivalent to indexes).
                    If so, we would stay in the same iterator */
                if (!balance(b))
                    ++b;
            }
        }
    }

    /*
        distance() runs in O(sqrt(n)) time and returns the supposed index 
        (from 0) of the first occurence of n inside the sortedBucket.
        If n is not present then it returns -1.
    */
    int distance(const T& n) const {
        // Edge case: one empty bucket can exist if no elements in container
        if (buckets.empty() || buckets.front().empty()) {
            return 0;
        }
        int ct = 0;
        typename std::vector<std::vector<T>>::const_iterator targetBucket = buckets.cbegin();      
        while (targetBucket != buckets.cend() && Comp{} (targetBucket->back(), n)) {
            ct += targetBucket->size();
            ++targetBucket;
        }
        if (targetBucket == buckets.cend()) {
            return -1;
        }
        typename std::vector<T>::const_iterator targ = 
            std::lower_bound(targetBucket->begin(), targetBucket->end(), n,
            [&](const T& elem, const T& n) { return Comp{} (elem, n); } );
        ct += std::distance(targetBucket->begin(), targ);
        return (targ == targetBucket->cend() || *targ != n) ? -1 : ct;
    }

    /* 
        lowerBound() runs in O(sqrt(n)) time and returns a pair of non-const
        iterators, describing the bucket and the element (Where the element is
        the first that satisfies: (element < n) or Comp{}(element, n) is false)
        
        If the element is past the entire range, the first iterator in the pair 
        will be buckets.end() and the second iterator is invalid.
    */
    std::pair<typename std::vector<std::vector<T>>::iterator, 
              typename std::vector<T>::iterator> lowerBound(const T& n) {
        /* Edge case: when container is empty, there is one bucket which is empty,
            so instead of returning a valid targetBucket as first iterator, return 
            buckets.end() */
        if (buckets.empty() || buckets.front().empty()) {
            return make_pair(buckets.end(), typename std::vector<T>::iterator());
        }
        // Search tail of buckets
        typename std::vector<std::vector<T>>::iterator targetBucket = 
            std::lower_bound(buckets.begin(), buckets.end(), n,
            [&](const std::vector<T>& bucket, const T& n) 
                { return bucket.empty() || Comp{} (bucket.back(), n); } );
        if (targetBucket == buckets.end()) {
            return make_pair(buckets.end(), typename std::vector<T>::iterator());
        }
        // Find insertion point within targetBucket
        typename std::vector<T>::iterator targ = 
            std::lower_bound(targetBucket->begin(), targetBucket->end(), n,
            [&](const T& elem, const T& n) { return Comp{} (elem, n); } );

        if (targ == targetBucket->end()) { 
            // point to beginning of next bucket rather than end of this bucket.
            ++targetBucket;
            targ = (targetBucket == buckets.end())
                ? typename std::vector<T>::iterator()
                : targetBucket->begin();
        }
        return make_pair(targetBucket, targ);
    }
    
    /* 
        upperBound() runs in O(sqrt(n)) time and returns a pair of non-const
        iterators, describing the bucket and the element (Where the element is
        the first that satisfies: (n < element) or Comp{}(n, element) is true)
        
        If the element is past the entire range, the first iterator in the pair 
        will be buckets.end() and the second iterator is invalid.
    */
    std::pair<typename std::vector<std::vector<T>>::iterator, 
              typename std::vector<T>::iterator> upperBound(const T& n) {
        /* Edge case: when container is empty, there is one bucket which is empty,
            so instead of returning a valid targetBucket as first iterator, return 
            buckets.end() */
        if (buckets.empty() || buckets.front().empty()) {
            return make_pair(buckets.end(), typename std::vector<T>::iterator());
        }
        // Search tail of buckets
        typename std::vector<std::vector<T>>::iterator targetBucket =
            std::upper_bound(buckets.begin(), buckets.end(), n,
            [&](const T& n, const std::vector<T>& bucket)
                { return bucket.empty() || Comp{} (n, bucket.back()); } );

        if (targetBucket == buckets.end()) {
            return make_pair(buckets.end(), typename std::vector<T>::iterator());
        }
        // Find insertion point within targetBucket
        typename std::vector<T>::iterator targ = 
            std::upper_bound(targetBucket->begin(), targetBucket->end(), n,
            [&](const T& n, const T& elem) { return Comp{} (n, elem); } );

        if (targ == targetBucket->end()) {
            // point to beginning of next bucket rather than end of this bucket.
            ++targetBucket;
            targ = (targetBucket == buckets.end())
                ? typename std::vector<T>::iterator()
                : targetBucket->begin();
        }
        return make_pair(targetBucket, targ);
    }

    /*
        find() returns a pair of iterators to the first instance of n,
        describing the bucket and the element. If the element does not exist, 
        the first iterator in the pair will be buckets.end() and the second 
        iterator is invalid.
    */
    std::pair<typename std::vector<std::vector<T>>::iterator, 
              typename std::vector<T>::iterator> find(const T& n) {
        auto [targetBucket, targ] = lowerBound(n);
        if (targetBucket == buckets.end() || *targ != n) {
            return make_pair(buckets.end(), typename std::vector<T>::iterator());
        }
        return make_pair(targetBucket, targ);
    }

    /* 
        insert() preserves stable sorting order (calls upperBound) and returns a 
        pair of non-const iterators describing the bucket and the element. Note
        that subsequent insertions may invalidate this iterator pair (just like
        the underlying container, vector).
    */
    std::pair<typename std::vector<std::vector<T>>::iterator,
              typename std::vector<T>::iterator> insert(const T& n) {
        auto [targetBucket, targ] = upperBound(n);
        if (targetBucket == buckets.end()) {
            --targetBucket;
            targ = targetBucket->end();
        }
        /*  Only for insertion: the rebalance may invalidate all buckets::iterator
            and bucket::iterator if allocation occurs due to buckets vector 
            resizing. We store dists then regenerate iterators after balancing */
        size_t bucketDist = std::distance(buckets.begin(), targetBucket);
        size_t targDist = std::distance(targetBucket->begin(), targ);
        targetBucket->emplace(targ, n);
        
        size_t origSize = targetBucket->size();
        bool shiftRight = balance(targetBucket,
                                  std::next(targetBucket->begin(), targDist),
                                  true);
        
        targetBucket = std::next(buckets.begin(), bucketDist + shiftRight);
        targDist -= (shiftRight && origSize > 2*bucketDensity) ? bucketDensity : 0;
        targ = std::next(targetBucket->begin(), targDist);
        ++sz;
        return make_pair(targetBucket, targ);
    }

    std::pair<typename std::vector<std::vector<T>>::iterator,
              typename std::vector<T>::iterator> insert(T&& n) {
        auto [targetBucket, targ] = upperBound(n);
        if (targetBucket == buckets.end()) {
            --targetBucket;
            targ = targetBucket->end();
        }
        /*  Only for insertion: the rebalance may invalidate all buckets::iterator
            and bucket::iterator if allocation occurs due to buckets vector 
            resizing. We store dists then regenerate iterators after balancing */
        size_t bucketDist = std::distance(buckets.begin(), targetBucket);
        size_t targDist = std::distance(targetBucket->begin(), targ);
        targetBucket->emplace(targ, n);
        
        size_t origSize = targetBucket->size();
        bool shiftRight = balance(targetBucket,
                                  std::next(targetBucket->begin(), targDist),
                                  true);
        
        targetBucket = std::next(buckets.begin(), bucketDist + shiftRight);
        targDist -= (shiftRight && origSize > 2*bucketDensity) ? bucketDensity : 0;
        targ = std::next(targetBucket->begin(), targDist);
        ++sz;
        return make_pair(targetBucket, targ);
    }

    /*
        erase() runs in (O(sqrtn)) and deletes a single instance of the element. 
        It returns how many instances of the element were deleted (1 or 0)
    */
    int erase (const T& n) {
        auto [targetBucket, targ] = lowerBound(n);
        if (targetBucket == buckets.end() || targ == targetBucket->end()) {
            return 0;
        }
        targetBucket->erase(targ);
        balance(targetBucket);
        --sz;
        return 1;
    }

    /*
        eraseAll() runs in (O(sqrtn + #erased_elements)) and deletes all instances 
        of the element. It returns how many instances of the element were deleted.
    */
    int eraseAll (const T& n) {
        auto [targetBucket, targ] = lowerBound(n);
        if (targetBucket == buckets.end()) {
            return 0;
        }
        // targ guaranteed to not point to end of targetBucket if valid targetBucket.
        int ct = 0;
        typename std::vector<std::vector<T>>::iterator thisBucket = targetBucket;
        while (thisBucket != buckets.end() && *targ == n) {
            ++ct;
            targ = thisBucket->erase(targ);
            // now targ points right after erased element
            if (targ == thisBucket->end()) {
                targ = (++thisBucket)->begin();
            }
        }
        balance(targetBucket);
        balance(thisBucket);
        sz -= ct;
        return ct;
    }

    /*
        getBucketsEnd() returns a const iterator to the end of buckets. This is 
        a bandaid fix until I implement an actual iterator interface. Used to 
        check if the return iterator of find, insert, etc. is valid since 
        buckets is a private member.
    */
    typename std::vector<std::vector<T>>::const_iterator getBucketsEnd() const {
        return buckets.end();
    }

#ifdef DEBUG
    /*
        Forcibly change the bucket density since in normal usage we cannot go
        below the DefaultSmallDensity (500). This is used in debugging to force
        balancing for small number of elements.
    */
    void forceDensity(size_t density) {
        bucketDensity = density;
        if (sz > 0) {
            /* For vectors, rebalancing may invalidate iterator, so use idx */
            size_t bucketDist = 0;
            while (bucketDist < buckets.size()) {
                /*  Guaranteed no empty buckets if sz > 0, so track the first elem
                    of each bucket. For vector implementation, if we shift the 
                    first elem to the right, then the current b iterator may
                    point to the next bucket if the old one was undersized and 
                    got merged (since vector iterators are equivalent to indexes).
                    If so, we would stay in the same iterator */
                typename std::vector<std::vector<T>>::iterator b = 
                    std::next(buckets.begin(), bucketDist);
                if (!balance(b)) {
                    ++bucketDist;
                }
            }
        }
    }

    /*
        Prints entire contents. For debugging
    */
    void print(const std::string& name = "SortedBucketVV") const {
        std::cout << "Printing " << name << std::endl;
        std::cout << "    with size = " << sz << 
            " and density = " << bucketDensity << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << "Total buckets " << buckets.size() << std::endl;
        int b = 0;
        for (const std::vector<T>& bucket:buckets) {
            std::cout << "bucket " << b++ << " contains: " << std::endl;
            for (const T elem:bucket) std::cout << "  " << elem;
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
#endif

private:
    void init () {
        if (buckets.empty()) {
            buckets.emplace_back(std::vector<T>());
            buckets.front().reserve(2*bucketDensity + 4);
        }
    }

    /* 
        balance() runs in O(1) time and balances the bucket sizes, then
        returns whether or not the target element was shifted to the bucket 
        to its right after the balancing.
        This first removes any empty buckets to its right.
        Then it checks if targetBucket is too large or too small. 
        If so, we redistribute the contents among other buckets to maintain 
        approximately sqrt(n) operations for this container.
    */
    bool balance(typename std::vector<std::vector<T>>::iterator targetBucket,
                 typename std::vector<T>::iterator targ = typename std::vector<T>::iterator(),
                 bool targSupplied = false) {
        if (targetBucket >= buckets.end()) {
            return false;
        }
        bool shiftRight = false;
        typename std::vector<std::vector<T>>::iterator right = std::next(targetBucket);
        while (right != buckets.end() && right->empty()) {
            right = buckets.erase(right); // now right points the next available bucket
        }
        if (targetBucket->size() > bucketDensity * 2) {
            /* Create a bucket right of targetBucket and dump half of the
                oversized targetBucket there */
            shiftRight = targSupplied && 
                         (std::distance(targetBucket->begin(), targ) >= bucketDensity);
            typename std::vector<std::vector<T>>::iterator next = 
                buckets.emplace(std::next(targetBucket), std::vector<T, Alloc>());
            next->reserve(2*bucketDensity + 4);
            /* targetBucket may be invalidated if a vector reallocation occurred 
                on buckets, but the return iterator next is guaranteed to be 
                post-reallocation valid, so regenerate targetBucket from it. */
            targetBucket = std::prev(next);
            next->insert(next->begin(), std::next(targetBucket->begin(), bucketDensity), 
                         targetBucket->end());
            targetBucket->erase(std::next(targetBucket->begin(), bucketDensity),
                                targetBucket->end());
        }
        else if (targetBucket->size() < bucketDensity / 2) {
            // last bucket is permitted to be undersized
            if (std::next(targetBucket) == buckets.end()) {
                return false;
            }
            typename std::vector<std::vector<T>>::iterator next = std::next(targetBucket);
            /* If dumping to the right would cause overflow, append some of right
                into targetBucket */
            if (targetBucket->size() + next->size() > bucketDensity * 2) {
                targetBucket->reserve(2*bucketDensity + 4);
                int desired = (next->size() - targetBucket->size()) / 2;
                targetBucket->insert(targetBucket->end(), next->begin(), 
                                     std::next(next->begin(), desired));
                next->erase(next->begin(), std::next(next->begin(), desired));
            }
            // Otherwise prepend all to the right
            else {
                shiftRight = true;
                next->reserve(2*bucketDensity + 4);
                next->insert(next->begin(), targetBucket->begin(), targetBucket->end());
                buckets.erase(targetBucket);
            }
        }
        return shiftRight;
    }

    // Private members
    std::vector<std::vector<T, Alloc>> buckets;
    size_t sz {0};
    size_t capacity {0};
    size_t bucketDensity {DefaultSmallDensity};
};

#endif // UTIL_SORTED_BUCKET_VV_H