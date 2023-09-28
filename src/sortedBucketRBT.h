/**
 * @file sortedBucketRBT.h
 * 
 * @author Gavin Dan (xfdan10@gmail.com)
 * @brief Implementation of Sorted Bucket container using weighted red-black tree
 * @version 1.1
 * @date 2023-09-19
 * 
 * 
 * Container provides sub-linear time lookup, distance, insertion, deletion.
 * Note that although this container has theoretically decent time complexity,
 * the constant factors are high because each node stores more info than usual 
 * (weight, copies). Furthermore, expect poor cache performance because of 
 * underlying tree structure. Also possibly frequent garbage collection
 * 
 * Time complexities:
 *      find:               O(log(n))
 *      distance:           O(log(n))
 *      insert:             O(log(n))
 *      erase:              O(log(n))
 * 
 * 
 */

#ifndef UTIL_SORTED_BUCKET_RBT_H
#define UTIL_SORTED_BUCKET_RBT_H

#include <functional>

#define DEBUG
#ifdef DEBUG
#define SENTINEL_FLAG 99999 // value for sentinel, otherwise it uses default T().
#include <iostream>
#include <queue>
#include <string>
#endif

template <typename T,
          typename Comp     = std::less<T>,
          typename Equal    = std::equal_to<T>,
          typename Alloc    = std::allocator<T>>
class SortedBucketRBT {
public:
    struct Node {
        Node(const T& val, Node* par, unsigned char color, size_t copies = 1) noexcept
            : val(val)
            , par(par)
            , mass(copies)
            , copies(copies) 
            , color(color){}
            
        Node(T&& val, Node* par, unsigned char color, size_t copies = 1) noexcept
            : val(std::move(val))
            , par(par)
            , mass(copies)
            , copies(copies)
            , color(color){}
        
        Node*           par     {nullptr};
        Node*           left    {nullptr};
        Node*           right   {nullptr};
        int             mass    {1}; // mass of itself and all lower nodes
        int             copies  {1}; // allow multiset functionality while keeping BST rules
        unsigned char   color   {Red};
        T               val;
    };
    /*  steal Alloc's traits and rebind to allocate for node, which contains extra
        bookkeeping info in addition to the value_type, like in std::list. In certain
        cases, it may ruin alignment or the few bytes of extra data may mess with
        a very fine-funed custom allocator, but STL seems to tolerate this so I will */
    using AllocNode = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    
    friend struct Iterator;
    struct Iterator {
        friend class SortedBucketRBT;
        using Iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        /*  difference_type is included here for compliance with Iterator_traits,
            but distance should be calculated from SortedBucketRBT::distance()
            for runtime in O(log(n)) rather than O(n)   */
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type*;
        using const_pointer     = value_type const*;
        using reference         = value_type&;
        using const_reference   = value_type const&;

        Iterator() noexcept {}

        explicit Iterator(Node* node) noexcept : nodePtr(node) {}

        Iterator(const Iterator& other) noexcept
            : nodePtr(other.nodePtr) {}

        inline reference operator *() noexcept {
            return nodePtr->val;
        }

        inline const_reference operator *() const noexcept {
            return nodePtr->val;
        }

        inline pointer operator ->() {
            return &(nodePtr->val);
        }

        inline const_pointer operator ->() const {
            return &(nodePtr->val);
        }
        
        inline bool operator ==(const Iterator other) const noexcept {
            return (nodePtr == other.nodePtr);
        }

        inline bool operator !=(const Iterator other) const noexcept {
            return !(*this == other);
        }

        inline void operator =(const Iterator other) noexcept {
            nodePtr = other.nodePtr;
        }

        inline int copies() const noexcept {
            return nodePtr->copies;
        }

        /*  Pre-increment. Calling this on SortedBucketRBT::end() is UB, 
            so requires checks like in STL containers */
        Iterator& operator ++() {
            Node* node = nodePtr;
            if (node->right) {
                node = node->right;
                while (node->left) {
                    node = node->left;
                }
                nodePtr = node;
            }
            else {
                Node* temp = node->par;
                while (temp && temp->right == node) {
                    node = temp;
                    temp = temp->par;
                }
                nodePtr = temp;
            }
            return *this;
        }

        /*  Post-increment. Calling this on SortedBucketRBT::end() is UB, 
            so requires checks like in STL containers */
        Iterator operator ++(int) {
            Iterator temp = *this;
            operator++();
            return temp;
        }

        /*  Pre-decrement. Calling this on SortedBucketRBT::begin() is UB, 
            so requires checks like in STL containers */
        Iterator& operator --() {
            Node* node = nodePtr;
            if (node->left) {
                node = node->left;
                while (node->right) {
                    node = node->right;
                }
                nodePtr = node;
            }
            else {
                Node* temp = node->par;
                while (temp && temp->left == node) {
                    node = temp;
                    temp = temp->par;
                }
                nodePtr = temp;
            }
            return *this;
        }

        /*  Post-decrement. Calling this on SortedBucketRBT::begin() is UB, 
            so requires checks like in STL containers */
        Iterator operator --(int) {
            Iterator temp = *this;
            operator--();
            return temp;
        }

    private:
        Node* nodePtr {nullptr};
    };

    enum Color {
        Red             = 0,
        Black           = 1,
        DoubleBlack     = 2
    };
    
    /* Default constructor */
    SortedBucketRBT() noexcept {init();}

    /* Range constructor */
    template <class InputIterator>
    SortedBucketRBT(InputIterator beginIt, InputIterator endIt) noexcept {
        init();
        for (InputIterator it = beginIt; it != endIt; ++it) {
            insert(*it);
        }
    }
    
    /* Default destructor */
    ~SortedBucketRBT() noexcept {
        destroy(root);
    }

    /* Size getter */
    inline size_t size() const noexcept {
        return sz;
    }

    /* Begin getter */
    inline Iterator begin() noexcept {
        return Iterator(leftmost);
    }

    /* End getter */
    inline Iterator end() noexcept {
        return Iterator(endSentinel);
    }

    /*  Front element access. Calling front() on an empty RBTree will cause
        segfault, just like with other STL containers */
    inline T& front() noexcept {
        return leftmost->val;
    }

    /*  Back element access. Calling back() on an empty RBTree will cause
        segfault, just like with other STL containers */
    inline T& back() noexcept {
        return (--Iterator(endSentinel))->val;
    }

    /*
        find() runs in O(log(n)) and returns an Iterator to the first instance of n.
    */
    Iterator find(const T& n) noexcept {
        return findWithDistance(n).first;
    }

    /*
        findWithDistance() runs in O(log(n)) time and returns a pair of: an Iterator 
        to the element, along with the index of its first occurrence, relative to 
        the smallest element in the tree (0-indexed). If the element was not 
        found, the first of the pair is the end() Iterator. 
    */
    std::pair<Iterator, int> findWithDistance(const T& n) noexcept {
        Node* node = root;
        int dist = 0;
        while (node) {
            if (node == endSentinel) {
                /* endSentinel guaranteed to not have right children */
                node = node->left;
            }
            else if (Equal{} (n, node->val)) {
                dist += (node->left) ? node->left->mass : 0;
                return std::make_pair(Iterator(node), dist);
            }
            else if (Comp{} (n, node->val)) {
                node = node->left;
            }
            else {
                dist += (node->left) ? node->left->mass : 0;
                dist += node->copies;
                node = node->right;
            }
        }
        return std::make_pair(Iterator(static_cast<Node*>(nullptr)), -1);
    }

    /*
        distance() runs in O(logn) time and returns the index of the first 
        occurrence of the element, 0-indexed. (Returns -1 if element not found)
    */
    int distance (const T& n) noexcept {
        return findWithDistance(n).second;
    }

    /*
        insert() runs in O(log(n)) time and returns an Iterator to the inserted
        element.
    */
    Iterator insert(const T& n, size_t copies = 1) noexcept {
        sz += copies;
        Node* node = root;
        while (node) {
            node->mass += copies;
            if (node == endSentinel) {
                if (!endSentinel->left) {
                    endSentinel->left = std::allocator_traits<AllocNode>::allocate(
                        allocNode, 1);
                    std::allocator_traits<AllocNode>::construct(allocNode, endSentinel->left,
                        n, endSentinel, Red, copies);
                    balanceDoubleRed(endSentinel->left);
                    return insertHelper(n, endSentinel->left);
                }
                node = endSentinel->left;
            }
            else if (Equal{} (n, node->val)) {
                node->copies += copies;
                return insertHelper(n, node);
            }
            else if (Comp{} (n, node->val)) {
                if (!node->left) {
                    node->left = std::allocator_traits<AllocNode>::allocate(
                        allocNode, 1, /* hint location */ node);
                    std::allocator_traits<AllocNode>::construct(allocNode, node->left, 
                        n, node, Red, copies);
                    balanceDoubleRed(node->left);
                    return insertHelper(n, node->left);
                }
                node = node->left;
            }
            else {
                if (!node->right) {
                    node->right = std::allocator_traits<AllocNode>::allocate(
                        allocNode, 1, /* hint location */ node);
                    std::allocator_traits<AllocNode>::construct(allocNode, node->right, 
                        n, node, Red, copies);
                    balanceDoubleRed(node->right);
                    return insertHelper(n, node->right);
                }
                node = node->right;
            }
        }
        return insertHelper(n, static_cast<Node*>(nullptr));
    }

    Iterator insert(T&& n, size_t copies = 1) noexcept {
        sz += copies;
        Node* node = root;
        while (node) {
            node->mass += copies;
            if (node == endSentinel) {
                if (!endSentinel->left) {
                    endSentinel->left = std::allocator_traits<AllocNode>::allocate(
                        allocNode, 1);
                    std::allocator_traits<AllocNode>::construct(allocNode, endSentinel->left,
                        std::move(n), endSentinel, Red, copies);
                    balanceDoubleRed(endSentinel->left);
                    return insertHelper(n, endSentinel->left);
                }
                node = endSentinel->left;
            }
            else if (Equal{} (n, node->val)) {
                node->copies += copies;
                return insertHelper(n, node);
            }
            else if (Comp{} (n, node->val)) {
                if (!node->left) {
                    node->left = std::allocator_traits<AllocNode>::allocate(
                        allocNode, 1, /* hint location */ node);
                    std::allocator_traits<AllocNode>::construct(allocNode, node->left, 
                        std::move(n), node, Red, copies);
                    balanceDoubleRed(node->left);
                    return insertHelper(n, node->left);
                }
                node = node->left;
            }
            else {
                if (!node->right) {
                    node->right = std::allocator_traits<AllocNode>::allocate(
                        allocNode, 1, /* hint location */ node);
                    std::allocator_traits<AllocNode>::construct(allocNode, node->right, 
                        std::move(n), node, Red, copies);
                    balanceDoubleRed(node->right);
                    return insertHelper(n, node->right);
                }
                node = node->right;
            }
        }
        return insertHelper(n, static_cast<Node*>(nullptr));
    }

    template <class InputIterator>
    void insert(InputIterator beginIt, InputIterator endIt, size_t copies) noexcept {
        /*  If allowing "copies" arg to be default-parameterized, then our template 
            deduction can fail because calling insert(4, 3) will call treat type int
            as type InputIterator rather than T (on GCC 13.1) */
        for (InputIterator it = beginIt; it != endIt; ++it) {
            insert(*it, copies);
        }
    }

    /*
        erase runs in O(logn) erases a single instance of the element and 
        returns how many instances of the element were erased (1 if successful,
        0 if not found).
    */
    int erase(const T& n) noexcept {
        auto [it, pos] = findWithDistance(n);
        Node* node = it.nodePtr;
        if (!node) {
            return 0;
        }
        if (node->copies > 1) {
            updateMass(node, -1);
            --sz;
            return 1;
        }
        else {
            return eraseAll(node);
        }
    }

    /*
        eraseAll runs in O(logn) and erases all instances of the element. 
        It returns how many instances of the element were erased.
    */
    int eraseAll(const T& n) noexcept {
        auto [it, pos] = findWithDistance(n);
        Node* node = it.nodePtr;
        if (!node) {
            return 0;
        }
        return eraseAll(node);
    }

    int eraseAll(Node* node) noexcept {
        if (!node) {
            return 0;
        }
        Node* par = node->par;
        int ct = node->copies;

        if (leftmost != endSentinel && node->val == leftmost->val) {
            leftmost = (++Iterator(leftmost)).nodePtr;
        }

        //  node is a leaf. 
        if (!node->left && !node->right) {
            if (node == root) {
                root = nullptr;
            }
            else {
                updateMass(par, 0 - ct);
                bool nodeOnLeft = (par->left == node);
                if (nodeOnLeft) {
                    par->left = nullptr;
                }
                else {
                    par->right = nullptr;
                }
                if (node->color == Black) {
                    bool nodeRemoved = false;
                    while (!nodeRemoved) {
                        /* If node is a black leaf, then a sibling must exist due
                            to length property for sibling's branch. This might
                            produce a "DoubleBlack null node" which is not an 
                            actual node, so we cannot use balanceDoubleBlack(node).
                            Instead, handle the scenario right here.
                        */
                        Node* sibling = (nodeOnLeft) ? par->right : par->left;

                        if (sibling->color == Red) {
                            /* sibling must have exactly two black children to
                                satisfy initial length property */
                            sibling->color = Black;
                            par->color = Red;
                            if (nodeOnLeft) {
                                leftRotate(par);
                            }
                            else {
                                rightRotate(par);
                            }
                            // Repeat loop brings us to black sibling case
                        }
                        else {
                            // black sibling has no children.
                            if (!sibling->left && !sibling->right) {
                                par->color += sibling->color;
                                sibling->color = Red;
                                balanceDoubleBlack(par);
                                nodeRemoved = true;
                            }
                            /* black sibling has an in-line child along rotation
                                (lhs), which must be red, other child irrelevant */
                            else if (!nodeOnLeft && sibling->left) {
                                sibling->color = par->color;
                                sibling->left->color = Black;
                                par->color = Black;
                                rightRotate(par);
                                nodeRemoved = true;
                            }
                            /* black sibling has an in-line child along rotation
                                (rhs), which must be red, other child irrelevant */
                            else if (nodeOnLeft && sibling->right) {
                                sibling->color = par->color;
                                sibling->right->color = Black;
                                par->color = Black;
                                leftRotate(par);
                                nodeRemoved = true;
                            }
                            /* black sibling has out-of-line child (lhs), which
                                must be red, create an in-line red child from it */
                            else if (nodeOnLeft && sibling->left) {
                                sibling->left->color = Black;
                                sibling->color = Red;
                                rightRotate(sibling);
                                // continue loop, now sibling has in-line child
                            }
                            /* black sibling has out-of-line child (rhs), which 
                                must be red, create an in-line red child from it */
                            else {
                                sibling->right->color = Black;
                                sibling->color = Red;
                                leftRotate(sibling);
                                // continue loop, now sibling has in-line child
                            }
                        }
                    }
                }
            }
            sz -= ct;
            std::allocator_traits<AllocNode>::destroy(allocNode, node);
        }

        // node has only a left child
        else if (node->left && !node->right) {
            node->left->par = node->par;
            if (node == root) {
                root = node->left;
                root->color = Black;
            }
            else {
                node->left->color += node->color;
                if (par->left == node) {
                    par->left = node->left;
                }
                else {
                    par->right = node->left;
                }
            }
            updateMass(par, 0 - ct);
            balanceDoubleBlack(node->left);
            sz -= ct;
            std::allocator_traits<AllocNode>::destroy(allocNode, node);
        }

        // node has only a right child
        else if (!node->left && node->right) {
            node->right->par = node->par;
            if (node == root) {
                root = node->right;
                root->color = Black;
            }
            else {
                node->right->color += node->color;
                if (par->left == node) {
                    par->left = node->right;
                }
                else {
                    par->right = node->right;
                }
            }
            updateMass(par, 0 - ct);
            balanceDoubleBlack(node->right);
            sz -= ct;
            std::allocator_traits<AllocNode>::destroy(allocNode, node);
        }

        // node has two children
        else {
            /*  Normal BST deletion, find next in-order successor.
                Succ now takes the place of node.
                We must physically rearrange (not simply swap contents of) successor
                node, so that Iterators and pointers become invalid after we erase
                that node, rather than cause confusion when contents of an
                apparently valid Iterator suddenly change.
            */
            Node* succ = node->right;
            while (succ->left) {
                succ = succ->left;
            }
            Node* succPar = succ->par;

            if (node->right != succ) {
                this->swap(succ, node, false);
            }
            else {
                this->swap(succ, node, true);
            }
            eraseAll(node);
        }
        return ct;
    }

#ifdef DEBUG
    void print(const std::string& name = "SortedBucketRBT") const {
        std::cout << "Printing " << name << " with size = " << sz << std::endl;
        std::cout << "===========================================" << std::endl;
        typedef std::pair<const Node*, bool> p;
        std::queue<p> q;
        if (!root) {
            std::cout << "red black tree is empty" << std::endl;
            return;
        }
        if (root) { //else {
            std::cout << "root is " << root->val << ", mass = " << root->mass << 
            ", copies = " << root->copies << ", color is " <<
            (root->color == Black ? "Black" : "Red") << std::endl;
        }
        q.emplace(static_cast<Node*>(root->left), true);
        q.emplace(static_cast<Node*>(root->right), false);

        while (!q.empty()) {
            const auto [node, newLayer] = q.front();
            q.pop();
            if (newLayer) {} //std::cout << std::endl;
            if (!node) {
                if (newLayer && !q.empty()) { q.emplace(
                    static_cast<const Node*>(nullptr), true);
                }
                continue;
            }
            else {
                std::cout << "child of " << node->par->val << " is " << 
                node->val << ", mass = " << node->mass << ", copies = " <<
                node->copies << ", color is ";
                if (node->color == Red) {
                    std::cout << "Red";
                }
                else if (node->color == Black) {
                    std::cout << "Black";
                }
                else {
                    std::cout << "DoubleBlack";
                }
                std::cout << std::endl;
            }
            q.emplace(static_cast<const Node*>(node->left), newLayer);
            //cout << " pushed " << ((node->left) ? " val " : "null") << 
            //(node->left ? node->left->val : 0) << endl;
            q.emplace(static_cast<const Node*>(node->right), false);
            //cout << " pushed " << ((node->right) ? " val " : "null") << 
            //(node->right ? node->right->val : 0) << endl;
        }
        std::cout << std::endl;
    }
#endif

private:
    inline void init() {
        endSentinel = std::allocator_traits<AllocNode>::allocate(
            allocNode, 1);
        std::allocator_traits<AllocNode>::construct(allocNode, endSentinel,
            T(
            #ifdef DEBUG 
                SENTINEL_FLAG // if no flag, we use T() default constructor.
            #endif
            ), nullptr, Black, 0);
        /*  Must explicitly set internal data when using traits::allocate and construct
            combo, since class initializers not called this way. */
        endSentinel->left = nullptr;
        endSentinel->right = nullptr;
        endSentinel->mass = 0;
        root = endSentinel;
        leftmost = endSentinel;
    }

    /*  insertHelper catches all return paths from insert(), and injects a check 
        where leftmost is replaced if we inserted a new smallest element. */
    inline Iterator insertHelper(const T& n, Node* node) noexcept {
        if (!node) {
            return Iterator(node);
        }
        if (leftmost == endSentinel || n < leftmost->val) {
            leftmost = node;
        }
        return Iterator(node);
    };

    /*  
        swap() takes two nodes and swaps their positions in the tree, rather than
        contents. This means that any Iterators will stay valid in pointing to
        the correct contents of the nodes. Ensuring that the new node positions
        meet BST ordering is the caller's responsibility. This function is used
        only when deleting a node and replacing it with its next in-order successor.
    */
    void swap(Node* a, Node* b, bool isImmediateRight) noexcept {
        if (!a || !b) {
            return;
        }
        Node* aPar = a->par, *bPar = b->par;

        if (!aPar) {
            root = b;
        }
        else {
            if (aPar->left == a) {
                aPar->left = b;
            }
            else {
                aPar->right = b;
            }
        }
        if (!bPar) {
            root = a;
        }
        else {
            if (bPar->left == b) {
                bPar->left = a;
            }
            else {
                bPar->right = a;
            }
        }
        std::swap(a->left, b->left);
        std::swap(a->color, b->color);
        if (a->left) {
            a->left->par = a;
        }
        if (b->left) {
            b->left->par = b;
        }
        if (isImmediateRight) {
            // a (successor) is the immediate right of b (node).
            b->right = a->right;
            a->right = b;
            a->par = b->par;
            b->par = a;
        }
        else {
            std::swap(a->par, b->par);
            std::swap(a->right, b->right);
        }
        if (a->right) {
            a->right->par = a;
        }
        if (b->right) {
            b->right->par = b;
        }
        // Swap only the part of mass that's from its children, not itself!
        int aMassBelow = a->mass - a->copies, bMassBelow = b->mass - b->copies;
        a->mass = bMassBelow + a->copies;
        b->mass = aMassBelow + b->copies;
    }
    
    /* 
        leftRotate() runs in O(1) time and left rotates the tree while rebalancing
        mass. It preserves colors, so maintaining valid BST structure is the 
        responsibility of the caller.
    */
    void leftRotate(Node* upperPivot) noexcept {
        if (!upperPivot) {
            return;
        }
        Node* child = upperPivot->right;
        if (!child) {
            return;
        }
        if (root == upperPivot) {
            root = child;
        }
        else if (upperPivot->par->left == upperPivot) {
            upperPivot->par->left = child;
        }
        else {
            upperPivot->par->right = child;
        }
        Node* cl = child->left;
        child->par = upperPivot->par;
        child->left = upperPivot;
        upperPivot->par = child;
        if (cl) {
            cl->par = upperPivot;
        }
        upperPivot->right = cl;
        
        upperPivot->mass = upperPivot->copies;
        upperPivot->mass += (upperPivot->left) ? upperPivot->left->mass : 0;
        upperPivot->mass += (cl) ? cl->mass : 0;
        child->mass = child->copies + upperPivot->mass;
        child->mass += (child->right) ? child->right->mass : 0;
    }

    /* 
        rightRotate() runs in O(1) time and right rotates the tree while rebalancing
        mass. It preserves colors, so maintaining valid BST structure is the
        responsibility of the caller.
    */
    void rightRotate(Node* upperPivot) noexcept {
        if (!upperPivot) {
            return;
        }
        Node* child = upperPivot->left;
        if (!child) {
            return;
        }
        if (root == upperPivot) {
            root = child;
        }
        else if (upperPivot->par->left == upperPivot) {
            upperPivot->par->left = child;
        }
        else {
            upperPivot->par->right = child;
        }
        Node* cr = child->right;
        child->par = upperPivot->par;
        child->right = upperPivot;
        upperPivot->par = child;
        if (cr) {
            cr->par = upperPivot;
        }
        upperPivot->left = cr;

        child->mass = upperPivot->copies;
        upperPivot->mass = 1;
        upperPivot->mass += (cr) ? cr->mass : 0;
        upperPivot->mass += (upperPivot->right) ? upperPivot->right->mass : 0;
        child->mass = child->copies + upperPivot->mass;
        child->mass += (child->left) ? child->left->mass : 0;
    }

    /* 
        updateMass runs in O(logn) time and propogates a mass change of (ct) up
        the tree to the root, starting from node.
    */
    inline void updateMass(Node* node, int ct) noexcept {
        while (node) {
            node->mass += ct;
            node = node->par;
        }
    }

    /*
        balanceDoubleRed() is only called following certain insert operations.
        It checks the "problem child" node for double red together with its
        parent, and fixes tree if needed.
    */
    void balanceDoubleRed(Node* child) {
        while (child) {
            if (child->color == Black) {
                return;
            }
            Node* par = child->par;
            if (!par || par->color == Black) {
                root->color = Black;
                return;
            }
            Node* grandpar = par->par;
            if (!grandpar) {
                root->color = Black;
                return;
            }
            bool parOnLeft = (grandpar->left == par);

            /* red problem child is along axis of rotation (lhs) */
            if (parOnLeft && child == par->left) {
                child->color = Black;
                rightRotate(grandpar);
                /* continue loop for par again. */
                child = par;
            }
            /* red problem child is along axis of rotation (rhs) */
            else if (!parOnLeft && child == par->right) {
                child->color = Black;
                leftRotate(grandpar);
                /* continue loop for par again. */
                child = par;
            }
            /* red problem child is out-of-line (lhs) */
            else if (parOnLeft && child == par->right) {
                if (grandpar == root) {
                    root = child;
                }
                else {
                    if (grandpar->par->left == grandpar) {
                        grandpar->par->left = child;
                    }
                    else {
                        grandpar->par->right = child;
                    }
                }
                grandpar->left = child->right;
                if (child->right) {
                    child->right->par = grandpar;
                }
                par->right = child->left;
                if (child->left) {
                    child->left->par = par;
                }
                child->par = grandpar->par;
                child->left = par;
                child->right = grandpar;
                grandpar->par = child;
                par->par = child;
                par->color = Black;
                grandpar->mass = grandpar->copies;
                grandpar->mass += (grandpar->left) ? grandpar->left->mass : 0;
                grandpar->mass += (grandpar->right) ? grandpar->right->mass : 0;
                par->mass = par->copies;
                par->mass += (par->left) ? par->left->mass : 0;
                par->mass += (par->right) ? par->right->mass : 0;
                child->mass = child->copies + grandpar->mass + par->mass;
                /* continue loop for child again. */
            }
            /* red problem child is out-of-line (rhs) */
            else {
                if (grandpar == root) {
                    root = child;
                }
                else {
                    if (grandpar->par->left == grandpar) {
                        grandpar->par->left = child;
                    }
                    else {
                        grandpar->par->right = child;
                    }
                }
                grandpar->right = child->left;
                if (child->left) {
                    child->left->par = grandpar;
                }
                par->left = child->right;
                if (child->right) {
                    child->right->par = par;
                }
                child->par = grandpar->par;
                child->left = grandpar;
                child->right = par;
                grandpar->par = child;
                par->par = child;
                par->color = Black;
                grandpar->mass = grandpar->copies;
                grandpar->mass += (grandpar->left) ? grandpar->left->mass : 0;
                grandpar->mass += (grandpar->right) ? grandpar->right->mass : 0;
                par->mass = par->copies;
                par->mass += (par->left) ? par->left->mass : 0;
                par->mass += (par->right) ? par->right->mass : 0;
                child->mass = child->copies + grandpar->mass + par->mass;
                /* continue loop for child again. */
            }
        }
        root->color = Black;
    }

    /* 
        balanceDoubleBlack() is only called following certain erase operations.
        It checks the "problem child" node for being DoubleBlack and fixes tree
        if needed.
    */
    void balanceDoubleBlack(Node* child) noexcept {
        while (child) {
            if (child->color != DoubleBlack) {
                return;
            }
            if (child == root) {
                child->color = Black;
                return;
            }
            Node* par = child->par;
            bool childOnLeft = (par->left == child);
            Node* sibling = (childOnLeft) ? par->right : par->left;
            // impossible to have null sibling if valid RB tree.

            // red sibling must have black children, then also par must be black.
            if (sibling->color == Red) {
                sibling->color = Black;
                par->color = Red;
                if (childOnLeft) {
                    leftRotate(par);
                }
                else {
                    rightRotate(par);
                }
                /* now our hierarchy looks like: Black at top, then Red (with 
                    only Black sub-children), then our original DoubleBlack child.
                    Repeat the loop again */
            }
            else {
                // black sibling has only black children (null children are black)
                if ((!sibling->left || sibling->left->color == Black) &&
                    (!sibling->right || sibling->right->color == Black)) {
                    child->color = Black;
                    sibling->color = Red;
                    par->color += Black;
                    child = par; // repeat loop for par, possibly now DoubleBlack
                }
                /* black sibling has red child along line of rotation 
                    (lhs for right rot). Other child color irrelevant */
                else if (!childOnLeft && sibling->left && sibling->left->color == Red) {
                    child->color = Black;
                    sibling->color = par->color;
                    sibling->left->color = Black;
                    par->color = Black;
                    rightRotate(par);
                }
                /* black sibling has red child along line of rotation 
                    (rhs for left rot). Other child color irrelevant */
                else if (childOnLeft && sibling->right && sibling->right->color == Red) {
                    child->color = Black;
                    sibling->color = par->color;
                    sibling->right->color = Black;
                    par->color = Black;
                    leftRotate(par);
                }
                /* black sibling has out-of-line red child (lhs), which we can 
                    create an in-line child from */
                else if (!childOnLeft && sibling->right && sibling->right->color == Red) {
                    sibling->color = Red;
                    sibling->right->color = Black;
                    leftRotate(sibling);
                }
                /* black sibling has out-of-line red child (rhs), which we can
                    create an in-line child from */
                else {
                    sibling->color = Red;
                    sibling->left->color = Black;
                    rightRotate(sibling);
                }
            }
        }
    }

    /* 
        Destructor recursive helper.
    */
    void destroy(Node* node) noexcept {
        if (node) {
            std::allocator_traits<AllocNode>::destroy(allocNode, node->left);
            std::allocator_traits<AllocNode>::destroy(allocNode, node->right);
            std::allocator_traits<AllocNode>::destroy(allocNode, node);
        }
    }

    // Private members
    AllocNode   allocNode;      // instance of Node template allocator
    size_t      sz              {0};
    Node*       root            {nullptr};
    Node*       leftmost        {nullptr};
    Node*       endSentinel     {nullptr};
};

#endif // UTIL_SORTED_BUCKET_RBT_H