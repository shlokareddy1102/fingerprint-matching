#  Fingerprint-Based Criminal Identification System

A C++ project that simulates a lightweight, logic-driven biometric system for matching crime scene fingerprints with a criminal database. This system implements **Minutiae-Based Fingerprint Analysis** along with advanced matching strategies like **Zonal Matching** and **Graph-Based Relational Matching**, entirely using core C++ with STL.

>  Developed as an academic project to explore biometric matching techniques using data structures and algorithms.

---

##  OBJECTIVE

To develop a command-line tool that enables:
- Storage and retrieval of fingerprint data
- Matching fingerprints with high confidence
- Detection of criminal networks and accomplices
- All using **efficient C++ logic** and **file handling**, without external libraries

---

##   Matching Techniques Explained

### 1. 🗺️ Zonal Matching
- The fingerprint is logically **divided into zones** (like a grid).
- Each zone holds its own **minutiae features** (endpoints, bifurcations).
- Matching is performed **zone-by-zone**, increasing robustness for partial/unclear prints.
- **Similarity score** is calculated by comparing minutiae counts and types per zone.

> This simulates how real-world biometric systems handle noisy or partial fingerprint inputs.

### 2. 🌐 Graph-Based Relational Matching
- Each fingerprint is modeled as a **graph**, where:
  - **Nodes** = minutiae points
  - **Edges** = spatial/geometric relationships
- Matching involves comparing **graph structures** (e.g., relative angles, distances)
- This helps detect **unique structural patterns**, not just point-to-point similarity

> This mirrors complex matching done in AI/ML-powered biometric engines.

---

##  SYSTEM FEATURES

-  **Secure login system** with login attempt tracking
-  **Fingerprint matching** using:
  - Zonal Matching
  - Graph-Based Matching
-  **Confidence scoring** for match accuracy
-  **Search history tracking**
-  **Criminal network exploration** through accomplice graph traversal
- **Add/View criminals** with details & fingerprint data
-  All data stored in a **text file-based mini-database**

---

##  Technologies Used

| Tool/Concept          | Purpose                          |
|-----------------------|----------------------------------|
| **C++ (STL)**          | Core logic & data structures     |
| **File Handling**      | Data storage & retrieval         |
| `map`, `vector`, `set` | Minutiae representation          |
| **Graph Algorithms**   | Relational fingerprint matching  |
| **Authentication Logic** | Login and session management |

---
## 📁 File Structure

| File Name            | Purpose                                |
|----------------------|----------------------------------------|
| `main.cpp`              | Main source code implementing logic   |
| `criminal_database.txt` | Stores criminal fingerprint data     |
| `credentials.txt`       | Login details                        |
| `logs.txt`              | Login logs                          |
| `search_history.txt`    | Previous search history             |

---
## 🖥️ How to Run

1. Clone the repo (or copy the project folder):
   ```bash
   git clone https://github.com/sanjanareddy19/Fingerprint-Matching.git
   cd fingerprint_project
2. Compile the project:

 ```bash
 g++ main.cpp functions.cpp -o fingerprint
 ```
3. Run :
```bash
  ./fingerprint
 ```



