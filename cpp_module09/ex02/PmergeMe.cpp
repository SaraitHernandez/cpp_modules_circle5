/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PmergeMe.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 11:30:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/07/12 11:30:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PmergeMe.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <climits>
#include <sys/time.h>

PmergeMe::PmergeMe() {}

PmergeMe::PmergeMe(const PmergeMe &other) : _vec(other._vec), _deq(other._deq) {}

PmergeMe &PmergeMe::operator=(const PmergeMe &other)
{
	if (this != &other)
	{
		_vec = other._vec;
		_deq = other._deq;
	}
	return *this;
}

PmergeMe::~PmergeMe() {}

// --------------------------------------------------------------------------
// Parsing
// --------------------------------------------------------------------------

static bool parseInt(const std::string &tok, int &out)
{
	if (tok.empty())
		return false;
	for (std::string::size_type i = 0; i < tok.size(); ++i)
		if (!std::isdigit(static_cast<unsigned char>(tok[i])))
			return false;
	// Reject overflow: use strtol and compare with INT_MAX.
	char *end = 0;
	long v = std::strtol(tok.c_str(), &end, 10);
	if (*end != '\0' || v < 0 || v > INT_MAX)
		return false;
	out = static_cast<int>(v);
	return true;
}

void PmergeMe::parse(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i)
	{
		std::istringstream iss(argv[i]);
		std::string tok;
		// Allow either separate arguments or a single quoted, space-separated list.
		while (iss >> tok)
		{
			int value;
			if (!parseInt(tok, value))
				throw std::runtime_error("Error");
			_vec.push_back(value);
			_deq.push_back(value);
		}
	}
	if (_vec.empty())
		throw std::runtime_error("Error");
}

// --------------------------------------------------------------------------
// Ford-Johnson merge-insert sort (templated, instantiated per container)
// --------------------------------------------------------------------------

// Jacobsthal numbers: 0, 1, 1, 3, 5, 11, 21, 43, 85, ...
static size_t jacobsthal(size_t n)
{
	if (n == 0)
		return 0;
	if (n == 1)
		return 1;
	size_t a = 0, b = 1;
	for (size_t i = 2; i <= n; ++i)
	{
		size_t c = b + 2 * a;
		a = b;
		b = c;
	}
	return b;
}

// Builds the order in which the "pend" (smaller) elements are inserted,
// following the Jacobsthal sequence, then appending any remaining index.
static std::vector<size_t> buildInsertionOrder(size_t m)
{
	std::vector<size_t> order;
	std::vector<bool> used(m, false);

	size_t k = 2;
	size_t prev = jacobsthal(1); // 1
	while (prev < m)
	{
		size_t cur = jacobsthal(k);
		if (cur > m)
			cur = m;
		for (size_t idx = cur; idx > prev; --idx)
		{
			size_t zero = idx - 1;
			if (zero < m && !used[zero])
			{
				order.push_back(zero);
				used[zero] = true;
			}
		}
		if (cur == m)
			break;
		prev = jacobsthal(k);
		++k;
	}
	// Safety net: append any index not yet covered, in ascending order.
	for (size_t i = 0; i < m; ++i)
		if (!used[i])
			order.push_back(i);
	return order;
}

// Binary insertion of 'val' into the sorted range chain[0..hi).
template <typename C>
static void binaryInsert(C &chain, int val, size_t hi)
{
	size_t lo = 0;
	while (lo < hi)
	{
		size_t mid = lo + (hi - lo) / 2;
		if (chain[mid] < val)
			lo = mid + 1;
		else
			hi = mid;
	}
	chain.insert(chain.begin() + lo, val);
}

template <typename C>
static void mergeInsertSort(C &a)
{
	size_t n = a.size();
	if (n < 2)
		return;

	// 1. Split into pairs (larger, smaller). Keep track of an odd straggler.
	bool odd = (n % 2 == 1);
	int straggler = 0;
	if (odd)
		straggler = a[n - 1];

	C larger;
	C smaller;
	for (size_t i = 0; i + 1 < n; i += 2)
	{
		if (a[i] < a[i + 1])
		{
			larger.push_back(a[i + 1]);
			smaller.push_back(a[i]);
		}
		else
		{
			larger.push_back(a[i]);
			smaller.push_back(a[i + 1]);
		}
	}

	// 2. Recursively sort the larger elements: this is the main chain seed.
	mergeInsertSort(larger);

	// 3. The sorted larger elements form the initial main chain.
	C chain = larger;

	// 4. Insert the smaller elements using binary insertion, following the
	//    Jacobsthal order. The very first smaller can go straight to the front.
	if (!smaller.empty())
	{
		binaryInsert(chain, smaller[0], chain.size());
		std::vector<size_t> order = buildInsertionOrder(smaller.size());
		for (size_t i = 0; i < order.size(); ++i)
		{
			size_t idx = order[i];
			if (idx == 0)
				continue; // already inserted
			binaryInsert(chain, smaller[idx], chain.size());
		}
	}

	// 5. Insert the straggler, if any.
	if (odd)
		binaryInsert(chain, straggler, chain.size());

	a = chain;
}

void PmergeMe::_sortVector()
{
	mergeInsertSort(_vec);
}

void PmergeMe::_sortDeque()
{
	mergeInsertSort(_deq);
}

// --------------------------------------------------------------------------
// Output / driver
// --------------------------------------------------------------------------

template <typename C>
static void printSequence(const std::string &label, const C &c)
{
	std::cout << label;
	for (typename C::const_iterator it = c.begin(); it != c.end(); ++it)
		std::cout << " " << *it;
	std::cout << std::endl;
}

static double nowMicroseconds()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return static_cast<double>(tv.tv_sec) * 1e6 + static_cast<double>(tv.tv_usec);
}

void PmergeMe::run()
{
	printSequence("Before:", _vec);

	double startV = nowMicroseconds();
	_sortVector();
	double endV = nowMicroseconds();

	double startD = nowMicroseconds();
	_sortDeque();
	double endD = nowMicroseconds();

	printSequence("After: ", _vec);

	std::cout << "Time to process a range of " << _vec.size()
			  << " elements with std::vector : " << (endV - startV)
			  << " us" << std::endl;
	std::cout << "Time to process a range of " << _deq.size()
			  << " elements with std::deque  : " << (endD - startD)
			  << " us" << std::endl;
}
