#include <iostream>
#include <unordered_map>
#include <queue>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
using namespace std;

struct Node
{
    char value;
    int freq;
    Node *left;
    Node *right;

    Node(char data, int freq)
    {
        value = data;
        this->freq = freq;
    }

    Node(char data, int freq, Node *left, Node *right)
    {
        this->value = data;
        this->freq = freq;
        this->left = left;
        this->right = right;
    }
};

struct compare_nodes
{
    bool operator()(Node *const &n1, Node *const &n2)
    {
        return n1->freq > n2->freq;
    }
};

string take_input()
{
    int oper;
    string input;
    filesystem::path root_dir = "../";

    do
    {
        cout << "Select the operation: \n1.Add file path (1)\n2.Type text (2)\n3.Exit (-1)\n";
        cin >> oper;

        switch (oper)
        {
        case 1:
        {
            cout << "Add file path: ";
            cin >> input;

            filesystem::path file_path = root_dir / input;
            string file_text;
            try
            {
                ifstream file(file_path);

                if (!file.is_open())
                {
                    cerr << "Unable to open file at path " << file_path << endl;
                }

                stringstream buffer;
                buffer << file.rdbuf();
                file_text = buffer.str();

                file.close();
                return file_text;
            }
            catch (const exception &e)
            {
                cerr << e.what() << '\n';
                break;
            }

            break;
        }

        case 2:
        {
            cout << "Enter text: ";
            cin.ignore();
            getline(cin, input);
            return input;
        }

        case -1:
            return "";

        default:
            cout << "Not a valid operation\n";
        }
    } while (oper != -1);
    return "";
}

unordered_map<char, int> calculate_freq(string text)
{

    unordered_map<char, int> frequency;

    for (auto ch : text)
    {
        auto find = frequency.find(ch);

        if (find != frequency.end())
        {
            find->second++;
        }
        else
        {
            frequency.insert({ch, 1});
        }
    }
    return frequency;
}

Node *generate_huffman_tree(unordered_map<char, int> freq)
{

    priority_queue<Node *, vector<Node *>, compare_nodes> min_heap;

    for (auto it = freq.begin(); it != freq.end(); it++)
    {
        min_heap.push(new Node(it->first, it->second));
    }

    while (min_heap.size() > 1)
    {
        Node *left_node = min_heap.top();
        min_heap.pop();

        Node *right_node = min_heap.top();
        min_heap.pop();

        Node *intermediate = new Node('\0', left_node->freq + right_node->freq, left_node, right_node);
        min_heap.push(intermediate);
    }

    Node *tree_root = min_heap.top();
    return tree_root;
}

void print_tree(Node *root, string indent = "") {
    if (root == nullptr) {
        return;
    }

    // Using your original logic: checking for the dummy character
    if (root->value != '\0') {
        // It's a Leaf! Print the character and STOP.
        cout << indent << "└── '" << root->value << "': " << root->freq << "\n";
        return; // Crucial: Prevents recursing into uninitialized/garbage children
    } 
    else {
        // It's an Internal Node
        cout << indent << "├── [Internal]: " << root->freq << "\n";
        
        // Only recurse if we know it's an internal node
        print_tree(root->left, indent + "    ");
        print_tree(root->right, indent + "    ");
    }
}



int main()
{

    string text = take_input();
    cout << "Given text: " << text << "\n\n";

    unordered_map<char, int> freq = calculate_freq(text);

    for (auto it = freq.begin(); it != freq.end(); it++)
    {
        cout << it->first << ": " << it->second << endl;
    }

    Node *root = generate_huffman_tree(freq);
    cout << endl;
    print_tree(root);

    return 0;
}