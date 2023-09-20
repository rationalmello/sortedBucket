/**
 * @file sortedBucketVV.h
 * 
 * @author Gavin Dan (xfdan10@gmail.com)
 * @brief Implementation of Sorted Bucket container using std::vector of vectors
 * @version 1.1
 * @date 2023-09-19
 * 
 * 
 * Container provides sub-linear time lookup, distance, insertion, deletion.
 * Credit to Grant Jenks for his SortedContainers idea for Python (found at 
 * https://grantjenks.com/docs/sortedcontainers/sortedlist.html) which I
 * used as inspiration.
 * 
 * Time complexities:
 *      find:       O(log(sqrt(n)))
 *      distance:   O(sqrt(n))
 *      insert:     O(log(sqrt(n)))
 *      erase:      O(log(sqrt(n)))
 * 
 * 
 */

#ifndef UTIL_SORTED_BUCKET_VV_H
#define UTIL_SORTED_BUCKET_VV_H

/* 
    The default bucket size. Note this cannot be a template argument because it 
    would invalidate the move constructor taking a SortedBucketVV with a 
    different bucket size.
*/
#define DefaultSmallDensity (size_t(500))

#include <functional>
#include <math.h>

#define DEBUG
#ifdef DEBUG
#define SENTINEL_FLAG 99999 // value for sentinel, otherwise it uses default T().
#include <iostream>
#include <string>
#endif

template <typename T,
          typename Comp     = std::less<T>,
          typename Alloc    = std::allocator<T>>
class SortedBucketVV {
public:
    friend struct Iterator;
    struct Iterator {
        friend class SortedBucketVV;
        /* todo: might make this random access iterator in a future release */
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        /*  difference_type is included here for compliance with iterator_traits,
            but distance should be calculated from SortedBucketVV::distance()
            for runtime in O(log(n)) rather than O(n)   */
        using difference_type   = std::ptrdiff_t; 
        using pointer           = value_type*;
        using const_pointer     = value_type const*;
        using reference         = value_type&;
        using const_reference   = value_type const&;

        Iterator() noexcept {}

        Iterator(typename std::vector<std::vector<T>>::iterator targetBucket,
                 typename std::vector<T>::iterator targ) noexcept
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
            return (targetBucket        == other.targetBucket && 
                    targ                == other.targ);
        }

        inline bool operator <(const Iterator other) const noexcept {
            return targetBucket < other.targetBucket ||
                (targetBucket == other.targetBucket &&
                    targ < other.targ);
        }

        inline bool operator >(const Iterator other) const noexcept {
            return targetBucket > other.targetBucket ||
                (targetBucket == other.targetBucket &&
                    targ > other.targ);
        }

        inline bool operator <=(const Iterator other) const noexcept {
            return targetBucket < other.targetBucket ||
                   (targetBucket == other.targetBucket &&
                   targ <= other.targ);
        }

        inline bool operator >=(const Iterator other) const noexcept {
            return targetBucket > other.targetBucket ||
                   (targetBucket == other.targetBucket &&
                   targ >= other.targ);
        }

        inline bool operator !=(const Iterator other) const noexcept {
            return !(*this == other);
        }

        inline void operator =(const Iterator other) noexcept {
            targetBucket = other.targetBucket;
            targ = other.targ;
        }

        /*  Pre-increment. Calling this on SortedBucketVV::end() may segfault, 
            so requires checks like in STL containers */
        Iterator& operator ++() {
            ++targ;
            if (targ == targetBucket->end()) {
                ++targetBucket;
                targ = targetBucket->begin();
            }
            return *this;
        }

        /*  Post-increment. Calling this on SortedBucketVV::end() may segfault, 
            so requires checks like in STL containers */
        Iterator operator ++(int) {
            Iterator temp = *this;
            operator++();
            return temp;
        }

        /*  Pre-decrement. Calling this on SortedBucketVV::begin() may segfault, 
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

        /*  Post-decrement. Calling this on SortedBucketVV::begin() may segfault, 
            so requires checks like in STL containers */
        Iterator operator --(int) {
            Iterator temp = *this;
            operator--();
            return temp;
        }

    private:
        typename std::vector<std::vector<value_type>>::iterator     targetBucket
            {typename std::vector<std::vector<value_type>>::iterator(nullptr)};
        typename std::vector<value_type>::iterator                  targ
            {typename std::vector<value_type>::iterator             (nullptr)};
    };

    /* Default constructor */
    SortedBucketVV() noexcept {
        init();
    }

    /* Copy constructor */
    explicit SortedBucketVV(const SortedBucketVV<T, Comp>& old) noexcept {
        init();
        buckets = old.buckets;
        sz = old.sz;
        capacity = old.capacity;
        bucketDensity = old.bucketDensity;
        /* populate sentinel */
        init();
    }

    /* Move constructor */
    explicit SortedBucketVV(SortedBucketVV<T, Comp>&& old) noexcept {
        buckets.swap(old.buckets);
        sz = old.sz;
        capacity = old.capacity;
        bucketDensity = old.bucketDensity;
    }

    /* Capacity constructor */
    explicit SortedBucketVV(size_t cap) noexcept 
        : capacity(cap)
        , bucketDensity(std::max(DefaultSmallDensity, 
                        static_cast<size_t>(std::sqrt(cap)))) {
        init();
    }
    
    /* Range constructor */
    template<class InputIterator>
    SortedBucketVV(InputIterator beginIt, InputIterator endIt, size_t cap = 25000) noexcept 
        : capacity(cap)
        , bucketDensity(std::max(DefaultSmallDensity, 
                                 static_cast<size_t>(std::sqrt(cap)))) {
        init();
        for (InputIterator it = beginIt; it != endIt; ++it) {
            insert(*it);
        }
    }
    
    /* Default destructor */
    ~SortedBucketVV() noexcept {}

    /* Size getter */
    size_t size() const noexcept {
        return sz;
    }

    /* Density getter */
    size_t getDensity() const noexcept {
        return bucketDensity;
    }

    /* Begin getter */
    inline Iterator begin() noexcept {
        typename std::vector<std::vector<T>>::iterator targetBucket = buckets.begin();
        return Iterator(targetBucket, targetBucket->begin());
    }

    /* End getter */
    inline Iterator end() noexcept {
        return Iterator(std::prev(buckets.end()), endSentinel);
    }
    
    /*  Front element access. Calling front() on an empty SortedBucketVV will cause
        segfault, just like with other STL containers */
    inline T& front() noexcept {
        return buckets.front().front();
    }

    /*  Back element access. Calling back() on an empty SortedBucketVV will cause
        segfault, just like with other STL containers */
    inline T& back() noexcept {
        *std::prev(buckets.back().end());
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
                if (!balance(b)) {
                    ++b;
                }
            }
        }
    }

    /* 
        lowerBound() runs in O(log(sqrt(n))) time and returns the first iterator 
        which satisfies: (element < n) or Comp{}(element, n) is false).
    */
    Iterator lowerBound(const T& n) {
        /*  Sentinel is last item of last bucket, so need to exclude from search */
        int dist = 0;
        typename std::vector<std::vector<T>>::iterator targetBucket = buckets.begin();
        typename std::vector<std::vector<T>>::iterator sentinelBucket = 
            std::prev(buckets.end());
        if (buckets.size() > 1) {
            targetBucket = std::lower_bound(buckets.begin(), sentinelBucket, n,
                [&](const std::vector<T>& bucket, const T& n) 
                { return bucket.empty() || Comp{} (bucket.back(), n); });
        }
        // Find insertion point within targetBucket
        typename std::vector<T>::iterator end = targetBucket->end();
        if (std::next(targetBucket) == buckets.end()) {
            /* If here, then we are in sentinel bucket. Exclude sentinel from search */
            --end;
        }
        typename std::vector<T>::iterator targ = 
            std::lower_bound(targetBucket->begin(), end, n,
            [&](const T& elem, const T& n) { return Comp{} (elem, n); });

        if (targ == targetBucket->end()) { 
            // point to beginning of next bucket rather than end of this bucket.
            ++targetBucket;
            targ = targetBucket->begin();
        }
        return Iterator(targetBucket, targ);
    }
    
    /* 
        upperBound() runs in O(log(sqrt(n))) time and returns the first iterator which
        satisfies: (n < element) or Comp{}(n, element) is true)
    */
    Iterator upperBound(const T& n) {
        /*  Sentinel is last item of last bucket, so need to exclude from search */
        int dist = 0;
        typename std::vector<std::vector<T>>::iterator targetBucket = buckets.begin();
        typename std::vector<std::vector<T>>::iterator sentinelBucket = 
            std::prev(buckets.end());
        if (buckets.size() > 1) {
            targetBucket = std::upper_bound(buckets.begin(), buckets.end(), n,
                [&](const T& n, const std::vector<T>& bucket) 
                { return bucket.empty() || Comp{} (n, bucket.back()); });
        }
        // Find insertion point within targetBucket
        typename std::vector<T>::iterator end = targetBucket->end();
        if (std::next(targetBucket) == buckets.end()) {
            /* If here, then we are in sentinel bucket. Exclude sentinel from search */
            --end;
        }
        typename std::vector<T>::iterator targ = 
            std::upper_bound(targetBucket->begin(), end, n,
            [&](const T& n, const T& elem) { return Comp{} (n, elem); });

        if (targ == targetBucket->end()) { 
            // point to beginning of next bucket rather than end of this bucket.
            ++targetBucket;
            targ = targetBucket->begin();
        }
        return Iterator(targetBucket, targ);
    }

    /*
        find() runs in O(log(sqrt(n))) and returns an iterator to the first 
        instance of n.
    */
    Iterator find(const T& n) {
        auto [targetBucket, targ] = lowerBound(n);
        if ((targetBucket == std::prev(buckets.end()) && targ == endSentinel) || 
            *targ != n) {
            return this->end();
        }
        return Iterator(targetBucket, targ);
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
        typename std::vector<std::vector<T>>::iterator targetBucket = buckets.begin();
        typename std::vector<std::vector<T>>::iterator sentinelBucket = 
                std::prev(buckets.end());
        if (buckets.size() > 1) {
            while (targetBucket != sentinelBucket && 
                   Comp{} (targetBucket->back(), n)) {
                dist += targetBucket->size();
                ++targetBucket;
            }
            if (Comp{} (targetBucket->back(), n)) {
                targetBucket = sentinelBucket;
            }
        }
        // Find insertion point within targetBucket
        typename std::vector<T>::iterator targ = targetBucket->begin();
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
        /*  Only for insertion: the rebalance may invalidate all buckets::iterator
            and bucket::iterator if allocation occurs due to buckets vector 
            resizing. We store dists then regenerate iterators after balancing */
        size_t bucketDist = std::distance(buckets.begin(), targetBucket);
        size_t targDist = std::distance(targetBucket->begin(), targ);
        targetBucket->emplace(targ, n);
        endSentinel = std::prev(buckets.back().end());
        
        size_t origSize = targetBucket->size();
        bool shiftRight = balance(targetBucket,
                                  std::next(targetBucket->begin(), targDist),
                                  true);
        
        targetBucket = std::next(buckets.begin(), bucketDist + shiftRight);
        targDist -= (shiftRight && origSize > 2*bucketDensity) ? bucketDensity : 0;
        targ = std::next(targetBucket->begin(), targDist);
        ++sz;
        return Iterator(targetBucket, targ);
    }

    Iterator insert(T&& n) {
        auto [targetBucket, targ] = upperBound(n);
        /*  Only for insertion: the rebalance may invalidate all buckets::iterator
            and bucket::iterator if allocation occurs due to buckets vector 
            resizing. We store dists then regenerate iterators after balancing */
        size_t bucketDist = std::distance(buckets.begin(), targetBucket);
        size_t targDist = std::distance(targetBucket->begin(), targ);
        targetBucket->emplace(targ, n);
        endSentinel = std::prev(buckets.back().end());

        size_t origSize = targetBucket->size();
        bool shiftRight = balance(targetBucket,
                                  std::next(targetBucket->begin(), targDist),
                                  true);
        
        targetBucket = std::next(buckets.begin(), bucketDist + shiftRight);
        targDist -= (shiftRight && origSize > 2*bucketDensity) ? bucketDensity : 0;
        targ = std::next(targetBucket->begin(), targDist);
        ++sz;
        return Iterator(targetBucket, targ);
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
        int ct = 0;
        typename std::vector<std::vector<T>>::iterator thisBucket = targetBucket;
        typename std::vector<std::vector<T>>::iterator sentinelBucket = 
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

#ifdef DEBUG
    /*
        forceDensity() forcibly changes the bucket density since in normal usage,
        we cannot go below the DefaultSmallDensity (500). This is used in demo 
        to force balancing for small number of elements.
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
            buckets.emplace_back(std::vector<T>());
            buckets.front().reserve(2*bucketDensity + 4);
            buckets.front().emplace_back(T(
            # ifdef DEBUG
                SENTINEL_FLAG // if no flag, we use T() default constructor.
            #endif
            ));
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
    bool balance(typename std::vector<std::vector<T>>::iterator targetBucket,
                 typename std::vector<T>::iterator targ = typename std::vector<T>::iterator(),
                 bool targSupplied = false) {
        if (targetBucket == buckets.end()) {
            return false;
        }
        bool shiftRight = false;
        typename std::vector<std::vector<T>>::iterator right = std::next(targetBucket);
        while (right != buckets.end() && right->empty()) {
            right = buckets.erase(right); 
        }
        // right points to next available bucket
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
        endSentinel = std::prev(buckets.back().end());
        return shiftRight;
    }

    // Private members
    size_t                              sz              {0};
    size_t                              capacity        {0};
    size_t                              bucketDensity   {DefaultSmallDensity};
    std::vector<std::vector<T, Alloc>>  buckets;
    typename std::vector<T>::iterator   endSentinel;
};

#endif // UTIL_SORTED_BUCKET_VV_H