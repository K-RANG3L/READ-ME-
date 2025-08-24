#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <limits>

using namespace std;


//Utility helpers 

static inline string ltrim(string s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));
    return s;
}

static inline string rtrim(string s) {
    s.erase(find_if(s.rbegin(), s.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

static inline string trim(string s) { return rtrim(ltrim(s)); }

// Upper-case copy 
static inline string upper(string s) {
    transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return s;
}

// Split a line by commas 
static vector<string> splitCSV(const string& line) {
    vector<string> out;
    string token;
    stringstream ss(line);
    while (getline(ss, token, ',')) {
        out.push_back(trim(token));
    }
    return out;
}

//Data model

struct Course {
    string number;               
    string title;                
    vector<string> prerequisites;
};

struct Node {
    Course course;
    Node* left = nullptr;
    Node* right = nullptr;
    Node() = default;
    Node(const Course& c) : course(c) {}
};

// Binary Search Tree keyed by Course.number
class CourseBST {
public:
    CourseBST() : root(nullptr), count(0) {}
    ~CourseBST() { destroy(root); }

    // Insert (replaces title/prereqs if the key already exists)
    void insert(const Course& c) {
        root = insertRec(root, c);
    }

    // Find by course number (case-insensitive)
    const Course* find(const string& key) const {
        string k = upper(key);
        Node* cur = root;
        while (cur) {
            string here = upper(cur->course.number);
            if (k == here) return &cur->course;
            if (k < here) cur = cur->left;
            else          cur = cur->right;
        }
        return nullptr;
    }

    // In-order traversal: alphabetical by course number
    void printInOrder() const {
        if (!root) {
            cout << "No courses loaded. Choose option 1 to load the data.\n";
            return;
        }
        inOrderRec(root);
    }

    size_t size() const { return count; }

    // Clear the tree (for re-loading)
    void clear() {
        destroy(root);
        root = nullptr;
        count = 0;
    }

private:
    Node* root;
    size_t count;

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    Node* insertRec(Node* n, const Course& c) {
        if (!n) {
            ++count;
            return new Node(c);
        }
        string key = upper(c.number);
        string here = upper(n->course.number);
        if (key < here) {
            n->left = insertRec(n->left, c);
        }
        else if (key > here) {
            n->right = insertRec(n->right, c);
        }
        else {
            n->course = c;
        }
        return n;
    }

    static void inOrderRec(Node* n) {
        if (!n) return;
        inOrderRec(n->left);
        cout << n->course.number << ", " << n->course.title << '\n';
        inOrderRec(n->right);
    }
};

//File loading

// Reads CSV into the BST; returns number of courses loaded.
size_t loadCoursesFromCSV(const string& filename, CourseBST& bst) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "Error: could not open file '" << filename
            << "'. Check the path or Working Directory.\n";
        return 0;
    }

    bst.clear();

    string line;
    size_t lineNo = 0;
    size_t loaded = 0;

    while (getline(fin, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty()) continue;

        auto cols = splitCSV(line);
        if (cols.size() < 2) {
            cerr << "Warning: line " << lineNo
                << " has fewer than 2 columns. Skipping.\n";
            continue;
        }

        Course c;
        c.number = cols[0];
        c.title = cols[1];
        for (size_t i = 2; i < cols.size(); ++i) {
            if (!cols[i].empty()) c.prerequisites.push_back(cols[i]);
        }

        bst.insert(c);
        ++loaded;
    }

    cout << "Loaded " << loaded << " course"
        << (loaded == 1 ? "" : "s") << " from '" << filename << "'.\n";
    return loaded;
}

void printCourseDetails(const Course& c) {
    cout << c.number << ", " << c.title << '\n';
    cout << "Prerequisites: ";
    if (c.prerequisites.empty()) {
        cout << "None\n";
    }
    else {
        for (size_t i = 0; i < c.prerequisites.size(); ++i) {
            cout << c.prerequisites[i];
            if (i + 1 < c.prerequisites.size()) cout << ", ";
        }
        cout << '\n';
    }
}

//Menu UI

void showMenu() {
    cout << "\nWelcome to the course planner.\n\n";
    cout << "  1. Load Data Structure.\n";
    cout << "  2. Print Course List.\n";
    cout << "  3. Print Course.\n";
    cout << "  9. Exit.\n\n";
    cout << "What would you like to do? ";
}

// Safe integer input (rejects non-numeric)
int readMenuChoice() {
    int choice;
    while (true) {
        if (cin >> choice) return choice;
        cout << "Please enter a valid numeric option: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    CourseBST bst;
    bool running = true;

    while (running) {
        showMenu();
        int choice = readMenuChoice();
        cout << '\n';

        switch (choice) {
        case 1: {
            cout << "Enter the CSV filename (ex: CS300_ABCU_Advising_Program_Input.csv): ";
            string filename;
            cin >> filename;  
            loadCoursesFromCSV(filename, bst);
            break;
        }
        case 2: {
            cout << "Here is a sample schedule:\n";
            bst.printInOrder();
            break;
        }
        case 3: {
            if (bst.size() == 0) {
                cout << "No courses loaded. Choose option 1 first.\n";
                break;
            }
            cout << "What course do you want to know about? ";
            string key;
            cin >> key; 
            if (const Course* c = bst.find(key)) {
                printCourseDetails(*c);
            }
            else {
                cout << "Course '" << key << "' not found.\n";
            }
            break;
        }
        case 9:
            running = false;
            cout << "Thank you for using the course planner!\n";
            break;
        default:
            cout << "Please enter a valid option.\n";
        }
    }
    return 0;
}
