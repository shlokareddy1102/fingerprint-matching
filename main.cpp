#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <climits>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <limits>
using namespace std;

// ================== COLOR CODING ==================
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

// ================== STRUCTURES ==================
struct Minutiae {
    int x, y, angle;
    char type; // 'R' for Ridge ending, 'B' for Bifurcation
    double orientation;
};

struct Criminal {
    int id;
    string name;
    vector<Minutiae> fingerprint;
    vector<int> accomplices;
};

struct Zone {
    vector<Minutiae> minutiae;
    double avgOrientation;
    int x_center, y_center;
};

// ================== GLOBAL VARIABLES ==================
map<int, Criminal> criminalDB;
unordered_map<int, vector<int>> accompliceGraph;
const int ZONE_SIZE = 100;
const int MAX_ZONES = 10;

string databaseFile = "project\\criminal_database.txt";
string credentialsFile = "project\\credentials.txt";
string logFile = "project\\logs.txt";
string historyFile = "project\\search_history.txt";

// ================== UTILITY FUNCTIONS ==================
void printHeader(const string& title) {
    cout << COLOR_BOLD << COLOR_CYAN << "\n=== " << title << " ===" << COLOR_RESET << "\n";
}

void printSuccess(const string& message) {
    cout << COLOR_GREEN << "[✓] " << message << COLOR_RESET << "\n";
}

void printWarning(const string& message) {
    cout << COLOR_YELLOW << "[!] " << message << COLOR_RESET << "\n";
}

void printError(const string& message) {
    cout << COLOR_RED << "[✗] " << message << COLOR_RESET << "\n";
}

void printInfo(const string& message) {
    cout << COLOR_BLUE << "[i] " << message << COLOR_RESET << "\n";
}

void logAction(const string& action) {
    ofstream fout(logFile, ios::app);
    time_t now = time(0);
    fout << ctime(&now) << ": " << action << "\n";
    fout.close();
}

void addToSearchHistory(const string& entry) {
    ofstream fout(historyFile, ios::app);
    time_t now = time(0);
    fout << ctime(&now) << ": " << entry << "\n";
    fout.close();
}

void viewSearchHistory() {
    ifstream fin(historyFile);
    string line;
    printHeader("SEARCH HISTORY");
    while (getline(fin, line)) {
        cout << line << "\n";
    }
    fin.close();
}

// ================== AUTHENTICATION ==================
bool authenticate() {
    string user, pass, line;
    int attempts = 0;

    while (attempts < 3) {
        cout << COLOR_BOLD << "Enter username: " << COLOR_RESET;
        cin >> user;
        cout << COLOR_BOLD << "Enter password: " << COLOR_RESET;
        cin >> pass;

        ifstream fin(credentialsFile);
        bool success = false;
        while (getline(fin, line)) {
            stringstream ss(line);
            string u, p;
            ss >> u >> p;
            if (u == user && p == pass) {
                success = true;
                break;
            }
        }
        fin.close();

        if (success) {
            printSuccess("Login successful!");
            logAction("Login successful for user: " + user);
            return true;
        } else {
            printError("Invalid credentials. Try again.");
            logAction("Failed login attempt for user: " + user);
            attempts++;
        }
    }
    printError("Too many failed attempts. Exiting.");
    return false;
}

// ================== DATABASE MANAGEMENT ==================
void saveCriminalDB() {
    ofstream fout(databaseFile);
    for (auto it = criminalDB.begin(); it != criminalDB.end(); ++it) {
        int id = it->first;
        Criminal& crim = it->second;
        fout << id << "|" << crim.name;
        for (auto& m : crim.fingerprint)
            fout << "|" << m.x << "|" << m.y << "|" << m.angle << "|" << m.type << "|" << m.orientation;
        fout << "|AC";
        for (int ac : crim.accomplices)
            fout << "|" << ac;
        fout << "\n";
    }
    fout.close();
}

void loadCriminalDB() {
    criminalDB.clear();
    ifstream fin(databaseFile);
    string line;
    
    while (getline(fin, line)) {
        stringstream ss(line);
        Criminal c;
        string token;
        
        getline(ss, token, '|');
        try { c.id = stoi(token); } 
        catch (...) { continue; }
        
        getline(ss, c.name, '|');
        
        while (getline(ss, token, '|')) {
            if (token == "AC") break;
            
            try {
                Minutiae m;
                m.x = stoi(token);
                getline(ss, token, '|'); m.y = stoi(token);
                getline(ss, token, '|'); m.angle = stoi(token);
                getline(ss, token, '|'); m.type = token[0];
                getline(ss, token, '|'); m.orientation = stod(token);
                c.fingerprint.push_back(m);
            } catch (...) { break; }
        }
        
        while (getline(ss, token, '|')) {
            try { c.accomplices.push_back(stoi(token)); } 
            catch (...) { break; }
        }
        
        criminalDB[c.id] = c;
    }
    fin.close();
}

// ================== NETWORK VISUALIZATION ==================
void buildAccompliceGraph() {
    accompliceGraph.clear();
     for (auto it = criminalDB.begin(); it != criminalDB.end(); ++it) {
        int id = it->first;
        Criminal& crim = it->second;
        for (int accompliceId : crim.accomplices) {
            accompliceGraph[id].push_back(accompliceId);
            if (accompliceGraph.find(accompliceId) == accompliceGraph.end()) {
                accompliceGraph[accompliceId] = vector<int>();
            }
        }
    }
}

void showAccompliceNetwork(int id) {
    if (criminalDB.find(id) == criminalDB.end()) {
        printError("Criminal not found in database!");
        return;
    }

    unordered_set<int> visited;
    queue<int> q;
    q.push(id);
    visited.insert(id);

    printHeader("FULL CRIMINAL NETWORK FOR #" + to_string(id) + " (" + criminalDB[id].name + ")");
    cout << COLOR_MAGENTA << "Network shows all direct and indirect connections\n";
    cout << "Format: [ID] Name (Connection Degree)\n" << COLOR_RESET;

    unordered_map<int, int> degrees;
    degrees[id] = 0;

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        cout << "\n" << COLOR_BOLD << "[" << current << "] " << criminalDB[current].name 
             << COLOR_RESET << " (Degree: " << degrees[current] << ") connected to:\n";

        if (accompliceGraph.find(current) != accompliceGraph.end()) {
            for (int neighbor : accompliceGraph[current]) {
                cout << "  -> [" << neighbor << "] " << criminalDB[neighbor].name;
                if (degrees.find(neighbor) == degrees.end()) {
                    degrees[neighbor] = degrees[current] + 1;
                }
                cout << " (Degree: " << degrees[neighbor] << ")\n";
                
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }
    }

    cout << "\n" << COLOR_YELLOW << "Network Summary:\n";
    cout << "- Total criminals in network: " << visited.size() << "\n";
    cout << "- Maximum degree of separation: " << max_element(degrees.begin(), degrees.end(),
        [](const pair<int, int>& a, const pair<int, int>& b) { return a.second < b.second; })->second << "\n";
    cout << COLOR_RESET;
}

void viewAccomplicesAdjacencyList() {
    int id;
    cout << "Enter criminal ID: ";
    cin >> id;

    if (criminalDB.find(id) == criminalDB.end()) {
        printError("Invalid criminal ID.");
        return;
    }

    printHeader("ADJACENCY LIST FOR CRIMINAL #" + to_string(id) + " (" + criminalDB[id].name + ")");
    
    if (accompliceGraph.find(id) != accompliceGraph.end()) {
        for (int accompliceId : accompliceGraph[id]) {
            cout << "[" << id << "] -> [" << accompliceId << "]";
            if (criminalDB.find(accompliceId) != criminalDB.end()) {
                cout << " (" << criminalDB[accompliceId].name << ")";
            }
            cout << endl;
        }
    } else {
        printWarning("No connections found for this criminal.");
    }
}

// ================== MATCHING ALGORITHMS ==================
double compareGraphBasedMatching(const vector<Minutiae>& fp1, const vector<Minutiae>& fp2) {
    int matches = 0;
    for (auto& m1 : fp1) {
        for (auto& m2 : fp2) {
            if (m1.type != m2.type) continue;
            double dist = hypot(m1.x - m2.x, m1.y - m2.y);
            double angleDiff = min(abs(m1.angle - m2.angle), 360 - abs(m1.angle - m2.angle));
            if (dist <= 10 && angleDiff <= 20) matches++;
        }
    }
    return 1.0 - (double)matches / max(fp1.size(), fp2.size());
}

vector<Zone> createZones(const vector<Minutiae>& fingerprint) {
    vector<Zone> zones;
    int min_x = INT_MAX, max_x = INT_MIN;
    int min_y = INT_MAX, max_y = INT_MIN;

    for (const auto& m : fingerprint) {
        min_x = min(min_x, m.x);
        max_x = max(max_x, m.x);
        min_y = min(min_y, m.y);
        max_y = max(max_y, m.y);
    }

    for (int y = min_y; y < max_y; y += ZONE_SIZE) {
        for (int x = min_x; x < max_x; x += ZONE_SIZE) {
            Zone zone;
            zone.x_center = x + ZONE_SIZE/2;
            zone.y_center = y + ZONE_SIZE/2;
            double orientationSum = 0;
            int count = 0;

            for (const auto& m : fingerprint) {
                if (m.x >= x && m.x < x + ZONE_SIZE && 
                    m.y >= y && m.y < y + ZONE_SIZE) {
                    zone.minutiae.push_back(m);
                    orientationSum += m.orientation;
                    count++;
                }
            }

            if (count > 0) {
                zone.avgOrientation = orientationSum / count;
                zones.push_back(zone);
            }
        }
    }

    sort(zones.begin(), zones.end(), [](const Zone& a, const Zone& b) {
        return a.minutiae.size() > b.minutiae.size();
    });

    if (zones.size() > MAX_ZONES) zones.resize(MAX_ZONES);
    return zones;
}

double compareZonalMatching(const vector<Minutiae>& fp1, const vector<Minutiae>& fp2) {
    auto zones1 = createZones(fp1);
    auto zones2 = createZones(fp2);
    double totalScore = 0;
    int comparedZones = 0;

    for (const auto& zone1 : zones1) {
        for (const auto& zone2 : zones2) {
            double dist = hypot(zone1.x_center - zone2.x_center, 
                               zone1.y_center - zone2.y_center);
            if (dist > ZONE_SIZE * 1.5) continue;
            
            double orientationDiff = min(
                abs(zone1.avgOrientation - zone2.avgOrientation),
                360 - abs(zone1.avgOrientation - zone2.avgOrientation)
            );
            
            double countRatio = min(
                zone1.minutiae.size() / (double)zone2.minutiae.size(),
                zone2.minutiae.size() / (double)zone1.minutiae.size()
            );
            
            double zoneScore = (1 - orientationDiff/180.0) * countRatio;
            totalScore += zoneScore;
            comparedZones++;
            break;
        }
    }
    
    return comparedZones > 0 ? (1.0 - totalScore/comparedZones) : 1.0;
}

// ================== CORE FUNCTIONS ==================
void addCriminal() {
    printHeader("ADD NEW CRIMINAL RECORD");
    Criminal c;
    
    cout << "Enter ID: ";
    while (!(cin >> c.id) || criminalDB.find(c.id) != criminalDB.end()) {
        printError("Invalid ID or ID already exists!");
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter ID: ";
    }
    
    cout << "Enter name: ";
    cin.ignore();
    getline(cin, c.name);
    
    cout << "Enter number of minutiae points: ";
    int n;
    while (!(cin >> n) || n <= 0) {
        printError("Please enter a positive number!");
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter number of minutiae points: ";
    }
    
    for (int i = 0; i < n; ++i) {
        Minutiae m;
        cout << "\nMinutiae Point #" << i + 1 << ":\n";
        cout << "X coordinate: ";
        cin >> m.x;
        cout << "Y coordinate: ";
        cin >> m.y;
        cout << "Angle (0-359 degrees): ";
        cin >> m.angle;
        cout << "Type (R for Ridge ending, B for Bifurcation): ";
        cin >> m.type;
        cout << "Orientation (0.0-1.0): ";
        cin >> m.orientation;
        c.fingerprint.push_back(m);
    }
    
    cout << "Number of accomplices: ";
    int a;
    cin >> a;
    if (a > 0) {
        cout << "Enter accomplice IDs (space separated): ";
        for (int i = 0; i < a; ++i) {
            int aid;
            cin >> aid;
            c.accomplices.push_back(aid);
        }
    }
    
    criminalDB[c.id] = c;
    saveCriminalDB();
    buildAccompliceGraph();
    printSuccess("Criminal record added successfully!");
    logAction("Added criminal ID: " + to_string(c.id));
}

void viewCriminal(int id) {
    if (criminalDB.find(id) == criminalDB.end()) {
        printError("Criminal not found!");
        return;
    }
    
    Criminal& c = criminalDB[id];
    printHeader("CRIMINAL DETAILS #" + to_string(id));
    cout << COLOR_BOLD << "Name: " << COLOR_RESET << c.name << "\n";
    cout << COLOR_BOLD << "Fingerprint Points: " << COLOR_RESET << c.fingerprint.size() << "\n";
    
    if (!c.accomplices.empty()) {
        cout << COLOR_BOLD << "Accomplices: " << COLOR_RESET;
        for (int aid : c.accomplices) {
            cout << aid << " ";
        }
        cout << "\n";
    } else {
        cout << COLOR_BOLD << "Accomplices: " << COLOR_RESET << "None\n";
    }
    
    logAction("Viewed criminal ID: " + to_string(id));
    addToSearchHistory("Viewed Criminal ID: " + to_string(id));
}

void matchFingerprint() {
    printHeader("FINGERPRINT MATCHING SYSTEM");
    cout << COLOR_BLUE << "=== (Ridge & Bifurcation Analysis) ===\n" << COLOR_RESET;
    
    // Collect fingerprint sample
    cout << "\nEnter number of minutiae points: ";
    int n;
    while (!(cin >> n) || n <= 0) {
        printError("Please enter a positive number!");
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter number of minutiae points: ";
    }

    vector<Minutiae> testPrint(n);
    for (int i = 0; i < n; ++i) {
        cout << "\nPoint " << i + 1 << " details:\n";
        cout << "Type (R for Ridge, B for Bifurcation): ";
        cin >> testPrint[i].type;
        cout << "Coordinates (x y): ";
        cin >> testPrint[i].x >> testPrint[i].y;
        cout << "Angle (0-359 degrees): ";
        cin >> testPrint[i].angle;
        if (testPrint[i].type == 'R') {
            cout << "Ridge count from reference: ";
            cin >> testPrint[i].orientation;
        }
    }

    // Add method selection back in
    cout << "\n" << COLOR_BOLD << "Select matching method:" << COLOR_RESET << "\n";
    cout << "1. Graph-based matching (precise point comparison)\n";
    cout << "2. Zonal-based matching (regional characteristics)\n";
    cout << "Your choice (1-2): ";
    
    int method;
    while (!(cin >> method) || (method != 1 && method != 2)) {
        printError("Please enter 1 or 2!");
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Your choice (1-2): ";
    }

    // Perform matching
    printInfo("Analyzing fingerprint...");
    
    double bestScore = (method == 1) ? 1e9 : 1.0; // Lower is better for graph, higher for zonal
    int bestID = -1;
    Criminal* bestMatch = nullptr;
    int ridgeMatches = 0, bifurcationMatches = 0;

    for (auto it = criminalDB.begin(); it != criminalDB.end(); ++it) {
        int id = it->first;
        Criminal& crim = it->second;
        double score;
        int currentRidgeMatches = 0;
        int currentBifurcationMatches = 0;
        
        if (method == 1) {
            // Graph-based matching
            int matches = 0;
            for (auto& m1 : testPrint) {
                for (auto& m2 : crim.fingerprint) {
                    if (m1.type != m2.type) continue;
                    double dist = hypot(m1.x - m2.x, m1.y - m2.y);
                    double angleDiff = min(abs(m1.angle - m2.angle), 360 - abs(m1.angle - m2.angle));
                    if (dist <= 10 && angleDiff <= 20) {
                        matches++;
                        if (m1.type == 'R') currentRidgeMatches++;
                        else currentBifurcationMatches++;
                        break; // Match found for this test point
                    }
                }
            }
            score = 1.0 - (double)matches / max(testPrint.size(), crim.fingerprint.size());
        } else {
            // Zonal-based matching
            score = compareZonalMatching(testPrint, crim.fingerprint);
        }
        
        if ((method == 1 && score < bestScore) || (method == 2 && score < bestScore)) {
            bestScore = score;
            bestID = id;
            bestMatch = &crim;
            ridgeMatches = currentRidgeMatches;
            bifurcationMatches = currentBifurcationMatches;
        }
    }

    // Display results
    printHeader("FINGERPRINT MATCH RESULT");
    
    if (bestID == -1) {
        printError("No matching fingerprint found in database!");
    } else {
        double confidence = 100 * (1.0 - bestScore);
        
        // Main match result
        cout << "+---------------------------------------+\n";
        cout << "| " << COLOR_BOLD << "Match: Criminal #" << bestID << " (" << bestMatch->name << ")" << COLOR_RESET << " |\n";
        cout << "| " << COLOR_BOLD << "Confidence: " << fixed << setprecision(2) 
             << ((confidence > 99.995) ? 100.00 : confidence) << "%" << COLOR_RESET << " |\n";
        cout << "+---------------------------------------+\n\n";

        // Detailed analysis
        printHeader("DETAILED ANALYSIS");
        cout << "+---------------------------------------+\n";
        cout << "| " << COLOR_BOLD << "Ridge endings matched: " << ridgeMatches << COLOR_RESET << " |\n";
        cout << "| " << COLOR_BOLD << "Bifurcations matched: " << bifurcationMatches << COLOR_RESET << " |\n";
        cout << "| " << COLOR_BOLD << "Total minutiae points: " << testPrint.size() << COLOR_RESET << " |\n";
        cout << "+---------------------------------------+\n";
        
        // Show ridge counts if available
        bool hasRidgeCounts = false;
        for (auto& m : testPrint) {
            if (m.type == 'R' && m.orientation != 0) {
                hasRidgeCounts = true;
                break;
            }
        }
        
        if (hasRidgeCounts) {
            cout << "\n" << COLOR_BLUE << "Ridge Counts from Reference:\n";
            for (int i = 0; i < testPrint.size(); i++) {
                if (testPrint[i].type == 'R') {
                    cout << "- Point " << i+1 << ": " << testPrint[i].orientation << "\n";
                }
            }
            cout << COLOR_RESET;
        }

        // Show network option
        cout << "\n" << COLOR_BOLD << "View accomplice network for this criminal? (y/n): " << COLOR_RESET;
        char choice;
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            showAccompliceNetwork(bestID);
        }
        
        logAction("Matched fingerprint with criminal ID: " + to_string(bestID));
        addToSearchHistory("Fingerprint match with ID: " + to_string(bestID));
    }
}
// ================== MAIN FUNCTION ==================
int main() {
    // Create required files if they don't exist
    ofstream{databaseFile, ios::app};
    ofstream{credentialsFile, ios::app};
    ofstream{logFile, ios::app};
    ofstream{historyFile, ios::app};

    if (!authenticate()) return 0;
    
    loadCriminalDB();
    buildAccompliceGraph();

    printHeader("FINGERPRINT IDENTIFICATION SYSTEM");
    cout << COLOR_GREEN << "System initialized successfully!\n";
    cout << "- Loaded " << criminalDB.size() << " criminal records\n";
    cout << "- Detected " << accompliceGraph.size() << " network connections\n" << COLOR_RESET;

    while (true) {
        cout << COLOR_BOLD << COLOR_CYAN << "\n=== MAIN MENU ===" << COLOR_RESET << "\n";
        cout << "1. Add new criminal record\n";
        cout << "2. View criminal details\n";
        cout << "3. Match fingerprint\n";
        cout << "4. View adjacency list\n";
        cout << "5. View full network\n";
        cout << "6. View search history\n";
        cout << "7. Exit system\n";
        cout << COLOR_BOLD << "Enter your choice (1-7): " << COLOR_RESET;

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: addCriminal(); break;
            case 2: {
                int id;
                cout << "Enter criminal ID: ";
                cin >> id;
                viewCriminal(id);
                break;
            }
            case 3: matchFingerprint(); break;
            case 4: viewAccomplicesAdjacencyList(); break;
            case 5: {
                int id;
                cout << "Enter criminal ID to view full network: ";
                cin >> id;
                showAccompliceNetwork(id);
                break;
            }
            case 6: viewSearchHistory(); break;
            case 7: {
                printSuccess("Thank you for using the system. Goodbye!");
                return 0;
            }
            default: printError("Invalid choice! Please try again.");
        }
    }
}
