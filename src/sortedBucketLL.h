/**
 * @file sortedBucketLL.h
 * 
 * @author Gavin Dan (xfdan10@gmail.com)
 * @brief Implementation of Sorted Bucket container using std::list of lists
 * @version 1.2
 * @date 2023-11-03
 * 
 * 
 * Container provides sub-linear time lookup, distance, insertion, deletion.
 * Expect worse performance than the vector of vectors implementation because
 * of garbage collection and worse cache performance.
 * 
 * Time complexities:
 *      find:       O(sqrt(n))
 *      distance:   O(sqrt(n))
 *      insert:     O(sqrt(n))
 *      erase:      O(sqrt(n))
 *
 * 
 */

#ifndef UTIL_SORTED_BUCKET_LL_H
#define UTIL_SORTED_BUCKET_LL_H

/* 
    The default bucket size. Note this cannot be a template argument because it 
    would invalidate the move constructor taking a SortedBucketLL with a 
    different bucket size.
*/
#define DefaultSmallDensity (size_t(500))

#include <cassert>
#include <functional>
#include <list>
#include <math.h>

//#define NDEBUG
#ifndef NDEBUG
/* magic value for sentinel, otherwise uses T{} */
#define SENTINEL_FLAG 0xBEEF
#include <iostream>
#include <string>
#endif // ifndef NDEBUG

template <typename T,
          typename Comp     = std::less<T>,
          typename Alloc    = std::allocator<T>>
class SortedBucketLL {
public:
    friend struct Iterator;
    struct Iterator {
        friend class SortedBucketLL;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        /*  difference_type is included here for compliance with iterator_traits,
            but distance should be calculated from SortedBucketLL::distance()
            for runtime in O(log(n)) rather than O(n)   */
        using difference_type   = std::ptrdiff_t; 
        using pointer           = value_type*;
        using const_pointer     = value_type const*;
        using reference         = value_type&;
        using const_reference   = value_type const&;

        Iterator() noexcept {}

        Iterator(typename std::list<std::list<T>>::iterator targetBucket,
                 typename std::list<T>::iterator targ) noexcept
            : targetBucket(targetBucket)
            , targ(targ) {}

        Iterator(const Iterator& other) noexcept
            : targetBucket(other.targetBucket)
            , targ(other.targ) {}

        inline reference operator *() noexcept {
            return *targ;
        }

        inline const_reference operator *() const noexcept {
            return *targ;
        }

        inline pointer operator ->() {
            return std::addressof(*targ);
        }

        inline const_pointer operator ->() const {
            return std::addressof(*targ);
        }
        
        inline bool operator ==(const Iterator other) const noexcept {
            return (targetBucket    == other.targetBucket && 
                    targ            == other.targ);
        }

        inline bool operator !=(const Iterator other) const noexcept {
            return !(*this == other);
        }

        inline void operator =(const Iterator other) noexcept {
            targetBucket = other.targetBucket;
            targ = other.targ;
        }

        /*  Pre-increment. Calling this on SortedBucketLL::end() may segfault, 
            so requires checks like in STL containers */
        Iterator& operator ++() {
            ++targ;
            if (targ == targetBucket->end()) {
                ++targetBucket;
                targ = targetBucket->begin();
            }
            return *this;
        }

        /*  Post-increment. Calling this on SortedBucketLL::end() may segfault, 
            so requires checks like in STL containers */
        Iterator operator ++(int) {
            Iterator temp = *this;
            operator++();
            return temp;
        }

        /*  Pre-decrement. Calling this on SortedBucketLL::begin() may segfault, 
            so requires checks like in STL containers */
        Iterator& operator --() {
            if (targ == targetBucket->begin()) {
                --targetBucket;
                targ = std::prev(targetBucket->end());
            }
            else {
                --targ;
            }
            return *this;
        }

        /*  Post-decrement. Calling this on SortedBucketLL::begin() may segfault, 
            so requires checks like in STL containers */
        Iterator operator --(int) {
            Iterator temp = *this;
            operator--();
            return temp;
        }

    private:
        typename std::list<std::list<value_type>>::iterator     targetBucket
            {typename std::list<std::list<value_type>>::iterator(nullptr)};
        typename std::list<value_type>::iterator                targ
            {typename std::list<value_type>::iterator           (nullptr)};
    };

    /* Default constructor */
    SortedBucketLL() noexcept {
        init();
    }

    /* Copy constructor */
    explicit SortedBucketLL(const SortedBucketLL<T, Comp>& old) noexcept {
        buckets = old.buckets;
        sz = old.sz;
        capacity = old.capacity;
        bucketDensity = old.bucketDensity;
        /* populate sentinel */
        init();
    }

    /* Move constructor */
    explicit SortedBucketLL(SortedBucketLL<T, Comp>&& old) noexcept {
        buckets.swap(old.buckets);
        sz = old.sz;
        capacity = old.capacity;
        bucketDensity = old.bucketDensity;
    }

    /* Capacity constructor */
    explicit SortedBucketLL(size_t cap) noexcept 
        : capacity(cap)
        , bucketDensity(std::max(DefaultSmallDensity, 
                                 static_cast<size_t>(std::sqrt(cap)))) {
        init();
    }

    /* Range constructor */
    template<class InputIterator>
    SortedBucketLL(InputIterator beginIt, InputIterator endIt, size_t cap = 25000) noexcept 
        : capacity(cap)
        , bucketDensity(std::max(DefaultSmallDensity, 
                                 static_cast<size_t>(std::sqrt(cap)))) {
        init();
        for (InputIterator it = beginIt; it != endIt; ++it) {
            insert(*it);
        }
    }
    
    /* Default destructor */
    ~SortedBucketLL() noexcept {}

    /* Size getter */
    inline size_t size() const noexcept {
        return sz;
    }

    /* Density getter */
    inline size_t getDensity() const noexcept {
        return bucketDensity;
    }

    /* Begin getter */
    inline Iterator begin() noexcept {
        typename std::list<std::list<T>>::iterator targetBucket = buckets.begin();
        return Iterator(targetBucket, targetBucket->begin());
    }

    /* End getter */
    inline Iterator end() noexcept {
        return Iterator(std::prev(buckets.end()), endSentinel);
    }

    /*  Front element access. Calling front() on an empty SortedBucketLL will cause
        segfault, just like with other STL containers */
    inline T& front() noexcept {
        return buckets.front().front();
    }

    /*  Back element access. Calling back() on an empty SortedBucketLL will cause
        segfault, just like with other STL containers */
    inline T& back() noexcept {
        return *std::prev(buckets.back().end());
    }

    /*
        changeCapacity() makes the container aware of the intended capacity 
        and rebalances all buckets. Because this implementation uses list nodes,
        it does not invalidate element iterators, but may invalidate target
        iterators.
    */
    void changeCapacity(size_t cap) {
        bucketDensity = std::max(DefaultSmallDensity,
                                static_cast<size_t>(std::sqrt(cap)));
        if (sz > 0) {
            typename std::list<std::list<T, Alloc>>::iterator b = buckets.begin();
            while (b != buckets.end()) {
                /*  Guaranteed no empty buckets if sz > 0, so track the first elem
                    of each bucket. For list implementation, iterators remain 
                    valid after balancing */
                assert(!b->empty());
                balance(b);
                ++b;
            }
        }
    }

    /* 
        lowerBound() runs in O(sqrt(n)) time and returns the first iterator 
        which satisfies: (element < n) or Comp{}(element, n) is false).
    */
    Iterator lowerBound(const T& n) {
        /*  Sentinel is last item of last bucket, so need to exclude from search */
        typename std::list<std::list<T>>::iterator targetBucket = buckets.begin();
        typename std::list<std::list<T>>::iterator sentinelBucket = 
            std::prev(buckets.end());
        if (buckets.size() > 1) {
            while (targetBucket != sentinelBucket && Comp{} (targetBucket->back(), n)) {
                ++targetBucket;
            }
            if (Comp{} (targetBucket->back(), n)) {
                targetBucket = sentinelBucket;
            }
        }
        // Find insertion point within targetBucket
        typename std::list<T>::iterator targ = targetBucket->begin();
        while (targ != targetBucket->end() &&
               (targetBucket != sentinelBucket || targ != endSentinel) && 
               Comp{} (*targ, n)) {
            ++targ;
        }
        if (targ == targetBucket->end()) {
            /*  Point to beginning of next bucket rather than end of this bucket.
                If targ was already the sentinel, we cannot arrive here. */
            ++targetBucket;
            targ = targetBucket->begin();
        }
        return Iterator(targetBucket, targ);
    }

    /* 
        upperBound() runs in O(sqrt(n)) time and returns the first iterator which
        satisfies: (n < element) or Comp{}(n, element) is true).
    */
    Iterator upperBound(const T& n) {
        /*  Sentinel is last item of last bucket, so need to exclude from search */
        typename std::list<std::list<T>>::iterator targetBucket = buckets.begin();
        typename std::list<std::list<T>>::iterator sentinelBucket = 
            std::prev(buckets.end());
        if (buckets.size() > 1) {
            while (targetBucket != sentinelBucket && 
                !Comp{} (n, targetBucket->back())) {
                ++targetBucket;
            }
            if (!Comp{} (n, targetBucket->back())) {
                targetBucket = sentinelBucket;
            }
        }
        // Find insertion point within targetBucket
        typename std::list<T>::iterator targ = targetBucket->begin();
        while (targ != targetBucket->end() && 
               (targetBucket != sentinelBucket || targ != endSentinel) &&
               !Comp{} (n, *targ)) {
            ++targ;
        }
        if (targ == targetBucket->end()) {
            /*  Point to beginning of next bucket rather than end of this bucket.
                If targ was already the sentinel, we cannot arrive here. */
            ++targetBucket;
            targ = targetBucket->begin();
        }
        return Iterator(targetBucket, targ);
    }

    /*
        find() runs in O(sqrt(n)) and returns an iterator to the first 
        instance of n.
    */
    Iterator find(const T& n) {
        return findWithDistance(n).first;
    }
    
    /*
        distance() runs in O(sqrt(n)) time and returns the supposed index 
        (from 0) of the first occurence of n inside the sortedBucket.
        If n is not present then it returns -1.
    */
    int distance(const T& n) {
        return findWithDistance(n).second;
    }

    /*
        findWithDistance() runs in O(sqrt(n)) time and returns a pair of: an 
        Iterator to the element, along with the index of its first occurrence. 
        If the element was not found, the pair consists of the end() Iterator 
        and a distance of -1.
    */
    std::pair<Iterator, int> findWithDistance(const T& n) noexcept {
        /*  Sentinel is last item of last bucket, so need to exclude from search */
        int dist = 0;
        typename std::list<std::list<T>>::iterator targetBucket = buckets.begin();
        typename std::list<std::list<T>>::iterator sentinelBucket = 
            std::prev(buckets.end());
        if (buckets.size() > 1) {
            while (targetBucket != sentinelBucket && Comp{} (targetBucket->back(), n)) {
                dist += targetBucket->size();
                ++targetBucket;
            }
            if (Comp{} (targetBucket->back(), n)) {
                targetBucket = sentinelBucket;
            }
        }
        // Find insertion point within targetBucket
        typename std::list<T>::iterator targ = targetBucket->begin();
        while (targ != targetBucket->end() &&
               (targetBucket != sentinelBucket || targ != endSentinel) && 
               Comp{} (*targ, n)) {
            ++dist;
            ++targ;
        }
        if (targ == targetBucket->end()) {
            /*  Point to beginning of next bucket rather than end of this bucket.
                If targ was already the sentinel, we cannot arrive here. */
            ++targetBucket;
            targ = targetBucket->begin();
        }
        return ((targetBucket == sentinelBucket && targ == endSentinel) || 
            *targ != n)
            ? std::make_pair(this->end(), -1)
            : std::make_pair(Iterator(targetBucket, targ), dist);
    }

    /* 
        insert() runs in O(sqrt(n)). It preserves stable sorting order (by
        calling upperBound()) and returns an iterator to the inserted element.
    */
    Iterator insert(const T& n) {
        auto [targetBucket, targ] = upperBound(n);
        targetBucket->emplace(targ, n);
        --targ; // so we point to newly inserted element. Only for LL, not VV
        typename std::list<std::list<T>>::iterator outBucket = 
            balance(targetBucket, targ, true) ? std::next(targetBucket) : targetBucket;
        ++sz;
        return Iterator(outBucket, targ);
    }

    Iterator insert(T&& n) {
        auto [targetBucket, targ] = upperBound(n);
        targetBucket->emplace(targ, std::forward<T>(n));
        --targ; // so we point to newly inserted element. Only for LL, not VV
        typename std::list<std::list<T>>::iterator outBucket = 
            balance(targetBucket, targ, true) ? std::next(targetBucket) : targetBucket;
        ++sz;
        return Iterator(outBucket, targ);
    }

    /*
        erase() runs in (O(sqrtn)) and erases a single instance of the element. 
        It returns how many instances of the element were erased (1 or 0)
    */
    int erase(const T& n) {
        auto [targetBucket, targ] = find(n);
        if (targetBucket == std::prev(buckets.end()) && targ == endSentinel) {
            return 0;
        }
        targetBucket->erase(targ);
        balance(targetBucket);
        --sz;
        return 1;
    }

    /*
        eraseAll() runs in (O(sqrtn + #erased_elements)) and erases all instances 
        of the element. It returns how many instances of the element were erased.
    */
    int eraseAll(const T& n) {
        auto [targetBucket, targ] = find(n);
        if (targetBucket == std::prev(buckets.end()) && targ == endSentinel) {
            return 0;
        }
        // targ guaranteed to not point to end of targetBucket if valid targetBucket.
        assert(targ != targetBucket->end());
        int ct = 0;
        typename std::list<std::list<T>>::iterator thisBucket = targetBucket;
        typename std::list<std::list<T>>::iterator sentinelBucket = 
            std::prev(buckets.end());
        while ((thisBucket != sentinelBucket || targ != endSentinel) && *targ == n) {
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
    
#ifndef NDEBUG
    /*
        forceDensity() forcibly changes the bucket density since in normal usage,
        we cannot go below the DefaultSmallDensity (500). This is used in demo 
        to force balancing for small number of elements.
    */
    void forceDensity(size_t density) {
        bucketDensity = density;
        if (sz > 0) {
            typename std::list<std::list<T>>::iterator b = buckets.begin();
            while (b != buckets.end()) {
                /*  Guaranteed no empty buckets if sz > 0, so track the first elem
                    of each bucket. For list implementation, iterators remain 
                    valid after balancing */
                assert(!b->empty());
                balance(b);
                ++b;
            }
        }
    }

    /*
        Prints entire contents. For debugging
    */
    void print(const std::string& name = "SortedBucketLL") const {
        std::cout << "Printing " << name << std::endl;
        std::cout << "    with size = " << sz << 
            " and density = " << bucketDensity << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << "Total buckets " << buckets.size() << std::endl;
        int b = 0;
        for (auto bucket = buckets.begin(); bucket != buckets.end(); ++bucket) {
            std::cout << "bucket " << b++ << " contains: " << std::endl;
            for (auto it = bucket->begin(); it != bucket->end(); ++it) {
                if (bucket == std::prev(buckets.end()) && it == endSentinel) {
                    std::cout << " sent ";
                }
                else {
                    std::cout << "  " << *it;
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
#endif

private:
    inline void init() {
        if (buckets.empty()) {
            buckets.emplace_back(std::list<T>());
            buckets.front().emplace_back(T{
            # ifndef NDEBUG
                SENTINEL_FLAG // if no flag, we use T{} constructor.
            #endif
            });
        }
        endSentinel = std::prev(buckets.back().end());
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
    bool balance(typename std::list<std::list<T>>::iterator targetBucket,
                 typename std::list<T>::iterator targ = typename std::list<T>::iterator(),
                 bool targSupplied = false) {
        if (targetBucket == buckets.end()) {
            return false;
        }
        bool shiftRight = false;
        typename std::list<std::list<T>>::iterator right = std::next(targetBucket);
        while (right != buckets.end() && right->empty()) {
            right = buckets.erase(right); 
        }
        // right points to next available bucket
        if (targetBucket->size() > bucketDensity * 2) {
            /* Create a bucket right of targetBucket and dump half of the 
                oversized targetBucket there */
            shiftRight = true;
            typename std::list<T>::iterator mid = targetBucket->begin();
            for (int i = 0; i < bucketDensity; ++i) {
                // if we encounter targ in first half, it remains in same bucket
                shiftRight &= !(targSupplied && mid == targ);
                ++mid;
            }
            typename std::list<std::list<T>>::iterator next = 
                buckets.emplace(std::next(targetBucket), std::list<T, Alloc>());
            next->splice(next->begin(), *targetBucket, mid, targetBucket->end());
        }
        else if (targetBucket->size() < bucketDensity / 2) {
            // last bucket is permitted to be undersized
            if (std::next(targetBucket) == buckets.end()) {
                return false;
            }
            typename std::list<std::list<T>>::iterator next = std::next(targetBucket);
            /* If dumping to the right would cause overflow, append some of right
                into targetBucket */
            if (targetBucket->size() + next->size() > bucketDensity * 2) {
                int desired = (next->size() - targetBucket->size()) / 2;
                targetBucket->splice(targetBucket->end(), *next,
                    next->begin(), std::next(next->begin(), desired));
            }
            // Otherwise prepend all to the right
            else {
                shiftRight = true;
                next->splice(next->begin(), *targetBucket);
                buckets.erase(targetBucket);
            }
        }
        endSentinel = std::prev(buckets.back().end());
        return shiftRight;
    }

    // Private members
    size_t                              sz              {0};
    size_t                              capacity        {0};
    size_t                              bucketDensity   {DefaultSmallDensity};
    std::list<std::list<T, Alloc>>      buckets;
    typename std::list<T>::iterator     endSentinel;
};

#endif // UTIL_SORTED_BUCKET_LL_H