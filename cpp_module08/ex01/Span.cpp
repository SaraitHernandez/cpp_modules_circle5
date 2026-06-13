/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Span.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 02:30:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/06/12 02:56:30 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Span.hpp"
#include <algorithm>

Span::Span(void) : _maxSize(0)
{
}

Span::Span(unsigned int n) : _maxSize(n)
{
	_numbers.reserve(n);
}

Span::Span(const Span& other) : _maxSize(other._maxSize), _numbers(other._numbers)
{
}

Span& Span::operator=(const Span& other)
{
	if (this != &other)
	{
		_maxSize = other._maxSize;
		_numbers = other._numbers;
	}
	return *this;
}

Span::~Span(void)
{
}

void Span::addNumber(int number)
{
	if (_numbers.size() >= _maxSize)
		throw FullSpanException();
	_numbers.push_back(number);
}

unsigned int Span::shortestSpan(void) const
{
	if (_numbers.size() < 2)
		throw NoSpanException();

	std::vector<int> sorted(_numbers);
	std::sort(sorted.begin(), sorted.end());

	unsigned int shortest = static_cast<unsigned int>(sorted[1] - sorted[0]);
	for (size_t i = 1; i + 1 < sorted.size(); ++i)
	{
		unsigned int diff = static_cast<unsigned int>(sorted[i + 1] - sorted[i]);
		if (diff < shortest)
			shortest = diff;
	}
	return shortest;
}

unsigned int Span::longestSpan(void) const
{
	if (_numbers.size() < 2)
		throw NoSpanException();

	std::vector<int>::const_iterator min = std::min_element(_numbers.begin(), _numbers.end());
	std::vector<int>::const_iterator max = std::max_element(_numbers.begin(), _numbers.end());
	return static_cast<unsigned int>(*max - *min);
}

const char* Span::FullSpanException::what(void) const throw()
{
	return "Span is full: cannot add more numbers";
}

const char* Span::NoSpanException::what(void) const throw()
{
	return "No span can be found: need at least two numbers";
}
