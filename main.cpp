// main.cpp
//

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>

//#include "ConcurrentStack.h"


unsigned char const g_nThreadCount = 8;


int main() {
	using namespace std::chrono_literals;

	//for (long long nRecordsPerThread = 1LL; nRecordsPerThread < 500LL; ++nRecordsPerThread)
	{
		size_t nRecordsPerThread = 5'000'000ULL;
		auto nStackSize = g_nThreadCount * nRecordsPerThread;
		//	ConcurrentStack<long long> oSUT;

		//	for (long long i = 0; i < nStackSize; ++i) {
		//		oSUT.push_front(i);
		//	}

		//	assert(oSUT.size() == nStackSize);

		std::vector<long long> oSUT;
		oSUT.reserve(nStackSize);
		for (size_t i = 0ULL; i < nStackSize; ++i) {
			oSUT.push_back(i);
		}

		std::vector<std::future<int>> vAllThreads;
		for (int nThreadNum = 0; nThreadNum < g_nThreadCount; ++nThreadNum) {
			//		vAllThreads.push_back(std::async(std::launch::async, [&oSUT, nThreadNum]() {
			//			std::ofstream oTargetFile("./popped_by_thread_" + std::to_string(nThreadNum) + ".txt");
			//			while (!oSUT.empty()) {
			//				auto nHeadVal = *oSUT.front();
			//				oSUT.pop_front();
			//				oTargetFile << nHeadVal << "\n";
			//			}
			//			return 0;
			//		}));
			vAllThreads.push_back(std::async(std::launch::async, [&oSUT, nRecordsPerThread, nStackSize, nThreadNum]() {
				std::ofstream oTargetFile("./thread_" + std::to_string(nThreadNum) + "_output.txt");
				size_t nLoopNum = 0ULL;
				size_t nTargetIndex = nThreadNum;
				while (nTargetIndex < nStackSize) {
					oTargetFile << oSUT[nTargetIndex] << "\n";
					nTargetIndex = (++nLoopNum) * g_nThreadCount + nThreadNum;
				}
				return 0;
			}));
		}

		while (true) {
			if (std::all_of(vAllThreads.cbegin(), vAllThreads.cend(), [](std::future<int> const &oOneThread) {
				auto status = oOneThread.wait_for(0s);
				return status == std::future_status::ready;
			})) {
				break;
			}
			std::this_thread::sleep_for(500ms);
		}
	}



	return 0;
}
