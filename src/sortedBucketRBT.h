/**
 * @file sortedBucketRBT.h
 * 
 * @author Gavin Dan (xfdan10@gmail.com)
 * @brief Implementation of Sorted Bucket container using weighted red-black tree
 * @version 1.0
 * @date 2023-08-27
 * 
 * 
 * Container provides sub-linear time lookup, distance, insertion, deletion.
 * Note that although this container has theoretically decent time complexity,
 * the constant factors are high because each node stores more info than usual 
 * (weight, copies). Furthermore, expect poor cache performance because of 
 * underlying tree structure. Also possibly frequent garbage collection
 * 
 * Time complexities:
 *      find:       O(log(n))
 *      distance:   O(log(n))
 *      insert:     O(log(n))
 *      erase:      O(log(n))
 * 
 * @todo bidirectional iterator interface
 */

#ifndef UTIL_SORTED_BUCKET_RBT_H
#define UTIL_SORTED_BUCKET_RBT_H

#include <functional>

#define DEBUG
#ifdef DEBUG
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
        Node(const T& val, Node* par, unsigned char color, size_t copies = 1) 
            : val(val)
            , par(par)
            , color(color)
            , copies(copies) {}
            
        Node(T&& val, Node* par, unsigned char color, size_t copies = 1) noexcept 
            : val(val)
            , par(par)
            , color(color)
            , copies(copies) {}
        
        Node* par {nullptr};
        Node* left {nullptr};
        Node* right {nullptr};
        int mass {1};   // mass of itself and all lower nodes
        int copies {1}; // allow multiset functionality while keeping BST rules
        unsigned char color {Red};
        T val;
    };

    enum Color {
        Red             = 0,
        Black           = 1,
        DoubleBlack     = 2
    };
    
    SortedBucketRBT() {}

    template <class InputIterator>
    SortedBucketRBT(InputIterator beginIt, InputIterator endIt) {
        for (InputIterator it = beginIt; it != endIt; ++it) {
            insert(*it);
        }
    }
    
    ~SortedBucketRBT() {
        destroy(root);
    }

    size_t size() const {
        return sz;
    }

    /*
        find() runs in O(logn) time and returns a pair containing an iterator to 
        the element, along with the index of its first occurrence, relative to 
        the smallest element in the tree (0-indexed). If the element was not 
        found, it returns a null iterator. 
    */
    std::pair<Node*, int> find(const T& n) {
        Node* node = root;
        int dist = 0;
        while (node) {
            if (Equal{} (n, node->val)) {
                dist += (node->left) ? node->left->mass : 0;
                return std::make_pair(node, dist);
            }
            if (Comp{} (n, node->val))
                node = node->left;
            else {
                dist += (node->left) ? node->left->mass : 0;
                dist += node->copies;
                node = node->right;
            }
        }
        return std::make_pair(static_cast<Node*>(nullptr), -1);
    }

    /*
        distance() runs in O(logn) time and returns the index of the first 
        occurrence of the element, 0-indexed. (Returns -1 if element not found)
    */
    int distance (const T& n) {
        return find(n).second;
    }

    /*
        insert() runs in O(logn) time and returns a pair containing an iterator 
        to the element, along with its index relative to the smallest element 
        in the tree (0-indexed).
    */
    std::pair<Node*, int> insert(const T& n, size_t copies = 1) {
        sz += copies;
        int dist = 0;
        if (!root) {
            root = new Node(n, nullptr, Black, copies);
            return std::make_pair(root, 0);
        }
        Node* node = root;
        while (node) {
            node->mass += copies;
            if (Equal{} (n, node->val)) {
                node->copies += copies;
                return std::make_pair(node, dist);
            }
            if (Comp{} (n, node->val)) {
                if (!node->left) {
                    node->left = new Node(n, node, Red, copies);
                    balanceDoubleRed(node->left);
                    return std::make_pair(node->left, dist);
                }
                node = node->left;
            }
            else {
                dist += (node->left) ? node->left->mass : 0;
                dist += node->copies;
                if (!node->right) {
                    node->right = new Node(n, node, Red, copies);
                    balanceDoubleRed(node->right);
                    return std::make_pair(node->right, dist);
                }
                node = node->right;
            }
        }
        return std::make_pair(static_cast<Node*>(nullptr), -1);
    }

    std::pair<Node*, int> insert(T&& n, size_t copies = 1) {
        sz += copies;
        int dist = 0;
        if (!root) {
            root = new Node(n, nullptr, Black, copies);
            return std::make_pair(root, 0);
        }
        Node* node = root;
        while (node) {
            node->mass += copies;
            if (Equal{} (n, node->val)) {
                node->copies += copies;
                return std::make_pair(node, dist);
            }
            if (Comp{} (n, node->val)) {
                if (!node->left) {
                    node->left = new Node(n, node, Red, copies);
                    balanceDoubleRed(node->left);
                    return std::make_pair(node->left, dist);
                }
                node = node->left;
            }
            else {
                dist += (node->left) ? node->left->mass : 0;
                dist += node->copies;
                if (!node->right) {
                    node->right = new Node(n, node, Red, copies);
                    balanceDoubleRed(node->right);
                    return std::make_pair(node->right, dist);
                }
                node = node->right;
            }
        }
        return std::make_pair(static_cast<Node*>(nullptr), -1);
    }

    template <class InputIterator>
    void insert(InputIterator beginIt, InputIterator endIt, size_t copies) {
        /*  If allowing "copies" arg to be default-parameterized, then our template 
            deduction can fail because calling insert(4, 3) will call treat type int
            as type InputIterator rather than T (on GCC 13.1) */
        for (InputIterator it = beginIt; it != endIt; ++it) {
            insert(*it, copies);
        }
    }

    /*
        erase runs in O(logn) deletes a single instance of the element and 
        returns how many instances of the element were deleted (1 if successful,
        0 if not found).
    */
    int erase(const T& n) {
        auto [node, pos] = find(n);
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
        eraseAll runs in O(logn) and deletes all instances of the element. 
        It returns how many instances of the element were deleted.
    */
    int eraseAll(const T& n) {
        auto [node, pos] = find(n);
        if (!node) {
            return 0;
        }
        return eraseAll(node);
    }

    int eraseAll(Node* node) {
        if (!node) {
            return 0;
        }
        Node* par = node->par;
        int ct = node->copies;

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
            delete node;
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
            delete node;
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
            delete node;
        }

        // node has two children
        else {
            /*  Normal BST deletion, find next in-order successor.
                Succ now takes the place of node.
                We must physically rearrange (not simply swap contents of) successor
                node, so that iterators and pointers become invalid after we erase
                that node, rather than cause confusion when contents of an
                apparently valid iterator suddenly change.
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
        else {
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
    /* swap() takes two nodes and swaps their positions in the tree, rather than
        contents. This means that any iterators will stay valid in pointing to
        the correct contents of the nodes. Ensuring that the new node positions
        meet BST ordering is the caller's responsibility. This function is used
        only when deleting a node and replacing it with its next in-order successor.
    */
    void swap(Node* a, Node* b, bool isImmediateRight) {
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
    void leftRotate(Node* upperPivot) {
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
    void rightRotate(Node* upperPivot) {
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
    void updateMass(Node* node, int ct) {
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
            Node* uncle = (parOnLeft) ? grandpar->right : grandpar->left;
            if (!uncle || uncle->color == Black) { // null uncle counts as black
                grandpar->color = Red;
                par->color = Black;
                if (parOnLeft) {
                    rightRotate(grandpar);
                }
                else {
                    leftRotate(grandpar);
                }
                return;
            }
            else {
                par->color = Black;
                uncle->color = Black;
                grandpar->color = Red;
                child = grandpar; // continue loop for grandpar
            }
        }
        root->color = Black;
    }

    /* 
        balanceDoubleBlack() is only called following certain erase operations.
        It checks the "problem child" node for being DoubleBlack and fixes tree
        if needed.
    */
    void balanceDoubleBlack(Node* child) {
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
        Destructor helper function.
    */
    void destroy(Node* node) {
        if (node) {
            Node* left = node->left, *right = node->right;
            delete node;
            destroy(left);
            destroy(right);
        }
    }

    // Private members
    Node* root {nullptr};
    size_t sz {0};
};

#endif // UTIL_SORTED_BUCKET_RBT_H