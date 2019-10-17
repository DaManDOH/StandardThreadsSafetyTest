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
	class Node {
	public:
		Node() = default;
		~Node() = default;

		T t;
		std::shared_ptr<Node> pNext;
	};

	std::shared_ptr<Node> pHead;
	std::atomic_ullong nNodeCount = 0ULL;

	ConcurrentStack(ConcurrentStack &) = delete;
	void operator=(ConcurrentStack &) = delete;

public:
	ConcurrentStack() = default;
	~ConcurrentStack() = default;

	class Reference {
	private:
		std::shared_ptr<Node> p;

	public:
		Reference(std::shared_ptr<Node> p_)
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
	};

	bool empty() const {
		return pHead == nullptr;
	}

	size_t size() const {
		return nNodeCount;
	}

	auto find(T t) const {
		assert(pHead != nullptr);
		auto p = std::atomic_load(&pHead);
		while (p && p->t != t) {
			p = p->pNext;
		}
		return Reference(std::move(p));
	}

	auto front() const {
		return Reference(std::atomic_load(&pHead));
	}

	void push_front(T t) {
		auto p = std::make_shared<Node>();
		p->t = t;
		p->pNext = std::atomic_load(&pHead);
		while (!std::atomic_compare_exchange_weak(&pHead, &p->pNext, p)) {}
		++nNodeCount;
	}

	void pop_front() {
		auto p = std::atomic_load(&pHead);
		while (p && !std::atomic_compare_exchange_weak(&pHead, &p, p->pNext)) {}
		--nNodeCount;
	}
};
