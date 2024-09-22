#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <sstream>
#include <algorithm>

using namespace std;

typedef set<int> Itemset;

vector<Itemset> generateCandidates(const vector<Itemset>& frequentItemsets, int k) {
    vector<Itemset> candidates;
    for (size_t i = 0; i < frequentItemsets.size(); ++i) {
        for (size_t j = i + 1; j < frequentItemsets.size(); ++j) {
            Itemset candidate(frequentItemsets[i]);
            candidate.insert(frequentItemsets[j].begin(), frequentItemsets[j].end());
            if (candidate.size() == k) {
                candidates.push_back(candidate);
            }
        }
    }
    return candidates;
}

map<Itemset, int> countSupport(const vector<Itemset>& transactions, const vector<Itemset>& candidates, int start, int end) {
    map<Itemset, int> itemsetCounts;
    for (int i = start; i < end; ++i) {
        for (const auto& candidate : candidates) {
            if (includes(transactions[i].begin(), transactions[i].end(), candidate.begin(), candidate.end())) {
                itemsetCounts[candidate]++;
            }
        }
    }
    return itemsetCounts;
}

vector<Itemset> filterBySupport(const map<Itemset, int>& candidateCounts, int minSupport) {
    vector<Itemset> frequentItemsets;
    for (const auto& candidateCount : candidateCounts) {
        if (candidateCount.second >= minSupport) {
            frequentItemsets.push_back(candidateCount.first);
        }
    }
    return frequentItemsets;
}

void DIC(const vector<Itemset>& transactions, int globalMinSupport, int M, const string& outputFilename) {
    ofstream outputFile(outputFilename);

    // Initialize pass and itemset tracking
    vector<Itemset> frequentItemsets;

    // First pass, start with frequent 1-itemsets
    map<int, int> itemCounts;
    int numTransactions = transactions.size();
    int intervalSize = numTransactions / M;

    for (int interval = 0; interval < M; ++interval) {
        int start = interval * intervalSize;
        int end = (interval == M - 1) ? numTransactions : (interval + 1) * intervalSize;

        // Count support for 1-itemsets
        for (int i = start; i < end; ++i) {
            for (int item : transactions[i]) {
                itemCounts[item]++;
            }
        }

        // Filter 1-itemsets by support
        for (const auto& itemCount : itemCounts) {
            if (itemCount.second >= globalMinSupport) {
                frequentItemsets.push_back({ itemCount.first });
            }
        }
    }

    // Output frequent 1-itemsets to file
    outputFile << "Frequent 1-itemsets:\n";
    for (const auto& itemset : frequentItemsets) {
        for (int item : itemset) {
            outputFile << item << " ";
        }
        outputFile << endl;
    }

    // Dynamic itemset introduction for k > 1
    int k = 2;
    while (!frequentItemsets.empty()) {
        vector<Itemset> candidates = generateCandidates(frequentItemsets, k);
        map<Itemset, int> candidateCounts;

        for (int interval = 0; interval < M; ++interval) {
            int start = interval * intervalSize;
            int end = (interval == M - 1) ? numTransactions : (interval + 1) * intervalSize;

            // Count support for candidates in the current interval
            map<Itemset, int> intervalCounts = countSupport(transactions, candidates, start, end);

            // Accumulate candidate counts across intervals
            for (const auto& intervalCount : intervalCounts) {
                candidateCounts[intervalCount.first] += intervalCount.second;
            }

            // Filter candidates by support at each interval dynamically
            frequentItemsets = filterBySupport(candidateCounts, globalMinSupport);

            // Output frequent k-itemsets if found
            if (!frequentItemsets.empty()) {
                outputFile << "\nFrequent " << k << "-itemsets (after interval " << interval + 1 << "):\n";
                for (const auto& itemset : frequentItemsets) {
                    for (int item : itemset) {
                        outputFile << item << " ";
                    }
                    outputFile << endl;
                }
            }
        }

        k++;
    }

    outputFile.close();
    cout << "Results saved to " << outputFilename << endl;
}

// Function to read transactions from a text file
vector<Itemset> readTransactions(const string& filename) {
    ifstream file(filename);
    vector<Itemset> transactions;
    string line;

    if (file.is_open()) {
        while (getline(file, line)) {
            stringstream ss(line);
            Itemset transaction;
            int item;
            while (ss >> item) {
                transaction.insert(item);
            }
            transactions.push_back(transaction);
        }
        file.close();
    }
    else {
        cerr << "Unable to open file" << endl;
    }

    return transactions;
}

int main() {
    string inputFilename = "td.txt";
    string outputFilename = "output.txt";
    int globalMinSupport = 10; 
    int M = 3;  // Number of intervals

    vector<Itemset> transactions = readTransactions(inputFilename);

    DIC(transactions, globalMinSupport, M, outputFilename);

    return 0;
}
