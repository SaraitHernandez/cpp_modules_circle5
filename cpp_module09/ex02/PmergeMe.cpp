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
#include <algorithm>
#include <cstdlib>
#include <cctype>
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

// The sort runs on indices, so a pair stays linked even when values repeat.
template <typename C> struct IndexContainer;
template <> struct IndexContainer<std::vector<int> > { typedef std::vector<size_t> type; };
template <> struct IndexContainer<std::deque<int> >  { typedef std::deque<size_t> type; };

// Jacobsthal numbers: 0, 1, 1, 3, 5, 11, 21, 43, 85, ...
static size_t jacobsthal(size_t n)
{
	if (n < 2)
		return n;
	size_t a = 0, b = 1;
	for (size_t i = 2; i <= n; ++i)
	{
		size_t c = b + 2 * a;
		a = b;
		b = c;
	}
	return b;
}

// Binary insertion of 'element' into the sorted range chain[0, last).
template <typename IC, typename C>
static void binaryInsert(IC &chain, const C &values, size_t element, size_t last)
{
	size_t first = 0;
	while (first < last)
	{
		size_t mid = first + (last - first) / 2;
		if (values[chain[mid]] < values[element])
			first = mid + 1;
		else
			last = mid;
	}
	chain.insert(chain.begin() + first, element);
}

template <typename IC, typename C>
static void fordJohnson(IC &idx, const C &values)
{
	size_t n = idx.size();
	if (n < 2)
		return;

	bool odd = (n % 2 == 1);
	size_t leftover = 0;
	if (odd)
		leftover = idx[n - 1];

	IC mains;
	IC partnerOf(values.size(), 0);
	for (size_t i = 0; i + 1 < n; i += 2)
	{
		size_t small = idx[i];
		size_t big = idx[i + 1];
		if (values[big] < values[small])
			std::swap(small, big);
		mains.push_back(big);
		partnerOf[big] = small;
	}

	fordJohnson(mains, values);

	size_t m = mains.size();
	IC chain;
	chain.push_back(partnerOf[mains[0]]);
	for (size_t k = 0; k < m; ++k)
		chain.push_back(mains[k]);

	size_t pend = odd ? m + 1 : m; // the odd element is one more pend element
	size_t prevEnd = 1;            // b1 is already in place
	size_t r = 3;                  // J(3) = 3 closes the first group after b1

	while (prevEnd < pend)
	{
		size_t currEnd = jacobsthal(r);
		if (currEnd > pend)
			currEnd = pend;
		size_t reach = currEnd + prevEnd - 1;
		if (reach > chain.size())
			reach = chain.size();

		for (size_t position = currEnd; position > prevEnd; --position)
		{
			size_t k = position - 1; // positions start at 1, indices at 0
			size_t element = (k < m) ? partnerOf[mains[k]] : leftover;
			binaryInsert(chain, values, element, reach);
		}
		prevEnd = currEnd;
		++r;
	}

	idx = chain;
}

template <typename C>
static void mergeInsertSort(C &a)
{
	size_t n = a.size();
	if (n < 2)
		return;

	typename IndexContainer<C>::type idx;
	for (size_t i = 0; i < n; ++i)
		idx.push_back(i);

	fordJohnson(idx, a);

	C sorted;
	for (size_t i = 0; i < n; ++i)
		sorted.push_back(a[idx[i]]);
	a = sorted;
}

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
	mergeInsertSort(_vec);
	double endV = nowMicroseconds();

	double startD = nowMicroseconds();
	mergeInsertSort(_deq);
	double endD = nowMicroseconds();

	printSequence("After: ", _vec);

	std::cout << "Time to process a range of " << _vec.size()
			  << " elements with std::vector : " << (endV - startV)
			  << " us" << std::endl;
	std::cout << "Time to process a range of " << _deq.size()
			  << " elements with std::deque  : " << (endD - startD)
			  << " us" << std::endl;
}
