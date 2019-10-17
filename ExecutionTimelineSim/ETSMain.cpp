// ETSMain.cpp
//

#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

ostream & operator<<(ostream &out, deque<int> const &rhs) {
	if (!rhs.empty()) {
		for (auto iter = rhs.cbegin(); iter != rhs.cend() - 1; ++iter) {
			out << *iter << ", ";
		}
		out << *(rhs.cend() - 1);
	}
	return out;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		return -1;
	}

	string sFilepath(argv[1]);
	if (!filesystem::is_regular_file(sFilepath)) {
		return -2;
	}

	vector<string> vFileLines;
	{
		ifstream fExecutionTimeline(sFilepath);
		if (!fExecutionTimeline.is_open()) {
			return -3;
		}

		string sOneLine;
		while (getline(fExecutionTimeline, sOneLine)) {
			vFileLines.push_back(sOneLine);
		}
	}

	deque<int> qWaitingThreads;
	for (auto oOneLine : vFileLines) {
		int nEventThreadNum;
		string sLoopNum, sFunction, sStage;
		istringstream oParser(oOneLine);
		oParser >> nEventThreadNum >> sLoopNum >> sFunction >> sStage;
		if (sStage == "v") {
			qWaitingThreads.push_back(nEventThreadNum);
			cout << "Thread " << nEventThreadNum << " awaiting lock in ";
			if (sFunction == "e") {
				cout << "bool empty()";
			}
			else if (sFunction == "p") {
				cout << "void fetch_and_pop_back()";
			}
			else {
				cout << "unknown";
			}
			cout << " function; ";
		}
		else if (sStage == "<<") {
			if (nEventThreadNum == qWaitingThreads.front()) {
				qWaitingThreads.pop_front();
			}
			else {
				cout << "** WARNING: Unexpected thread gained lock! ** ";

				/*
				Only remove the first instance.
				
				No thread should be in the waiting "queue" more than once, but
				if it does, the deque will grow uncontrollably & indicate a
				threading problem.
				*/
				bool bPreviouslyFound = false;
				qWaitingThreads.erase(
					remove_if(
						qWaitingThreads.begin(),
						qWaitingThreads.end(),
						[nEventThreadNum, &bPreviouslyFound] (int one_waiting_thread_num) {

					bool bRetval = false;
					if (nEventThreadNum == one_waiting_thread_num && !bPreviouslyFound) {
						bRetval = true;
						bPreviouslyFound = true;
					}
					return bRetval;
				}),
					qWaitingThreads.end());
			}

			cout << "Lock now owned by thread " << nEventThreadNum << " in ";
			if (sFunction == "e") {
				cout << "bool empty()";
			}
			else if (sFunction == "p") {
				cout << "void fetch_and_pop_back()";
			}
			else {
				cout << "unknown";
			}
			cout << " function; ";
		}
		else if (sStage == "^") {
			cout << "Released lock by thread " << nEventThreadNum << " in ";
			if (sFunction == "e") {
				cout << "bool empty()";
			}
			else if (sFunction == "p") {
				cout << "void fetch_and_pop_back()";
			}
			else {
				cout << "unknown";
			}
			cout << " function; ";
		}
		else {
			cerr << "Unrecognized stage; ";
		}
		cout << "waiting: " << qWaitingThreads << "\n";
	}

	return 0;
}