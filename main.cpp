// main.cpp
//

#include <algorithm>
#include <chrono>
#include <fstream>
#include <future>
#include <string>
#include <thread>
#include <vector>

#include "ConcurrentStack.h"


int const g_nThreadCount = 8;
long long const g_nRecordsPerThread = 10'000LL;


int main() {
	long long nStackSize = g_nThreadCount * g_nRecordsPerThread;
	ConcurrentStack<long long> oSUT;

	for (long long i = 0; i < nStackSize; ++i) {
		oSUT.push_front(i);
	}

	assert(oSUT.size() == nStackSize);

	std::vector<std::future<int>> vAllThreads;

	for (int nThreadNum = 0; nThreadNum < g_nThreadCount; ++nThreadNum) {
		vAllThreads.push_back(std::async(std::launch::async, [&oSUT, nThreadNum]() {
			std::ofstream oTargetFile("./popped_by_thread_" + std::to_string(nThreadNum) + ".txt");
			while (oSUT.front().get() != nullptr) {
				auto nHeadVal = *oSUT.front();
				oSUT.pop_front();
				oTargetFile << nHeadVal << "\n";
			}
			return 0;
		}));
	}

	while (true) {
		if (std::all_of(vAllThreads.cbegin(), vAllThreads.cend(), [](std::future<int> const &oOneThread) {
			using namespace std::chrono_literals;
			auto status = oOneThread.wait_for(0s);
			return status == std::future_status::ready;
		})) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	return 0;
}
