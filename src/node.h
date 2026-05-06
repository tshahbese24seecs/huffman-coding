#ifndef NODE_H
#define NODE_H

/**
 * @brief Represents a node in the Huffman tree.
 *
 * Leaf nodes store a character and its frequency.
 * Internal nodes store the combined frequency of their children
 * and use '\0' as a sentinel value.
 */
struct Node
{
    char value;
    int freq;
    Node *left;
    Node *right;

    Node(char data, int freq)
        : value(data), freq(freq), left(nullptr), right(nullptr) {}

    Node(char data, int freq, Node *left, Node *right)
        : value(data), freq(freq), left(left), right(right) {}
};

/**
 * @brief Comparator for the min-heap used in Huffman tree construction.
 * Orders nodes by ascending frequency so the two smallest are always on top.
 */
struct compare_nodes
{
    bool operator()(Node *const &n1, Node *const &n2)
    {
        return n1->freq > n2->freq;
    }
};

#endif // NODE_H
