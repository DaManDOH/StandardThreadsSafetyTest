// STSTMain.cpp
//

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <fstream>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

//#include "ConcurrentStack.h"


unsigned char const g_nThreadCount = 8;


template<typename T>
class StdVecWrapper : public std::vector<T> {
private:
	std::mutex m_oMutex;
	//std::ofstream m_fExecution;

public:
	//StdVecWrapper()
	//	: std::vector<T>::vector()
	//	, m_fExecution("./execution_timeline.txt")
	//{}

	//~StdVecWrapper() {
	//	std::ofstream fDestructor("./vector_destroyed.txt");
	//	fDestructor << "Vector destroyed." << std::endl;
	//	std::vector<T>::~vector();
	//}

	//bool empty(std::string const &thread_and_loop_num) {
	//	using namespace std;
	//	bool bRetval;
	//	{
	//		m_fExecution << (thread_and_loop_num + " e v\n");
	//		lock_guard<mutex> oLock(m_oMutex);
	//		m_fExecution << (thread_and_loop_num + " e <<\n");
	//		bRetval = vector<T>::empty();
	//		m_fExecution << (thread_and_loop_num + " e ^\n");
	//	}
	//	return bRetval;
	//}

	T& back() = delete;
	void pop_back() = delete;

	//T fetch_and_pop_back(std::string const &thread_and_loop_num) {
	T fetch_and_pop_back() {
		using namespace std;
		T oRetval;
		{
			//m_fExecution << (thread_and_loop_num + " p v\n");
			lock_guard<mutex> oLock(m_oMutex);
			//m_fExecution << (thread_and_loop_num + " p <<\n");
			oRetval = vector<T>::back();
			vector<T>::pop_back();
			//m_fExecution << (thread_and_loop_num + " p ^\n");
		}
		return oRetval;
	}
};


int main() {
	using namespace std::chrono_literals;

	//for (long long nRecordsPerThread = 1LL; nRecordsPerThread < 500LL; ++nRecordsPerThread)
	{
		size_t nRecordsPerThread = 5'000'000ULL;
		auto nStackSize = g_nThreadCount * nRecordsPerThread;
		std::atomic_llong nSum = 0LL;

		auto pSUT = std::make_shared<StdVecWrapper<long long>>();
		{
			auto &oSUT = *pSUT;
			oSUT.reserve(nStackSize);
			for (long long i = 0LL; i < (long long)nStackSize; ++i) {
				oSUT.push_back(i + 1);
			}
		}

		std::vector<std::future<int>> vAllThreads;
		for (int nThreadNum = 0; nThreadNum < g_nThreadCount; ++nThreadNum) {
			vAllThreads.push_back(std::async(std::launch::async, [pSUT, nRecordsPerThread, nStackSize, nThreadNum, &nSum]() {
				//std::ofstream oTargetFile("./thread_" + std::to_string(nThreadNum) + "_output.txt");
				//auto sThreadNum = std::to_string(nThreadNum);
				size_t nLoopNum = 0;
				//auto sThreadAndLoopNum = sThreadNum + " L" + std::to_string(nLoopNum);
				//while (!pSUT->empty()) {
				//while (!pSUT->empty(sThreadAndLoopNum)) {
				while (nLoopNum < nRecordsPerThread) {
					//auto value = pSUT->fetch_and_pop_back(sThreadAndLoopNum);
					auto value = pSUT->fetch_and_pop_back();
					//oTargetFile << value << "\n";
					nSum += value;
					++nLoopNum;
					//sThreadAndLoopNum = sThreadNum + " L" + std::to_string(nLoopNum);
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
			std::this_thread::sleep_for(1s);
		}

		assert(pSUT->empty());
	}

	return 0;
}
