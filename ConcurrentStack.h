// ConcurrentStack.h
//

/*
Based on: https://www.modernescpp.com/index.php/atomic-smart-pointers
*/

#pragma once

#include <atomic>
#include <cassert>
#include <memory>

template<typename T>
class ConcurrentStack {

private:
	struct Node {
		T t;
		std::shared_ptr<Node> pNext;
	};

	std::shared_ptr<Node> pHead;

	ConcurrentStack(ConcurrentStack &) = delete;
	void operator=(ConcurrentStack &) = delete;

public:
	ConcurrentStack() = default;
	~ConcurrentStack() = default;

	class reference {
	private:
		std::shared_ptr<Node> p;

	public:
		reference(std::shared_ptr<Node> p_)
			: p{ p_ }
		{ }

		T& operator* () {
			assert(p != nullptr);
			return p->t;
		}

		T* operator->() {
			assert(p != nullptr);
			return &p->t;
		}

		Node const * get() const {
			return p.get();
		}
	};

	bool empty() const {
		return pHead == nullptr;
	}

	size_t size() const {
		auto p = std::atomic_load(&pHead);
		size_t nRetVal = 0ULL;
		while (p) {
			++nRetVal;
			p = p->pNext;
		}
		return nRetVal;
	}

	auto find(T t) const {
		assert(pHead != nullptr);
		auto p = std::atomic_load(&pHead);
		while (p && p->t != t) {
			p = p->pNext;
		}
		return reference(std::move(p));
	}

	auto front() const {
		return reference(std::atomic_load(&pHead));
	}

	void push_front(T t) {
		auto p = std::make_shared<Node>();
		p->t = t;
		p->pNext = std::atomic_load(&pHead);
		while (!std::atomic_compare_exchange_weak(&pHead, &p->pNext, p)) {}
	}

	void pop_front() {
		auto p = std::atomic_load(&pHead);
		while (p && !std::atomic_compare_exchange_weak(&pHead, &p, p->pNext)) {}
	}
};
