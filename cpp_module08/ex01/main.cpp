/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 04:41:16 by sarherna          #+#    #+#             */
/*   Updated: 2026/06/12 04:56:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Span.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>

int main(void)
{
	// Example from the subject
	{
		Span sp = Span(5);
		sp.addNumber(6);
		sp.addNumber(3);
		sp.addNumber(17);
		sp.addNumber(9);
		sp.addNumber(11);
		std::cout << sp.shortestSpan() << std::endl;
		std::cout << sp.longestSpan() << std::endl;
	}

	std::cout << "----------------------------------------" << std::endl;

	// Exception: adding too many numbers
	{
		Span sp(2);
		sp.addNumber(1);
		sp.addNumber(2);
		try
		{
			sp.addNumber(3);
		}
		catch (const std::exception& e)
		{
			std::cout << "Caught: " << e.what() << std::endl;
		}
	}

	// Exception: not enough numbers for a span
	{
		Span sp(5);
		sp.addNumber(42);
		try
		{
			sp.shortestSpan();
		}
		catch (const std::exception& e)
		{
			std::cout << "Caught: " << e.what() << std::endl;
		}
	}

	std::cout << "----------------------------------------" << std::endl;

	// Filling with a range of iterators
	{
		std::vector<int> data;
		data.push_back(5);
		data.push_back(3);
		data.push_back(15);
		data.push_back(8);
		data.push_back(1);

		Span sp(data.size());
		sp.addRange(data.begin(), data.end());
		std::cout << "Range shortestSpan: " << sp.shortestSpan() << std::endl;
		std::cout << "Range longestSpan:  " << sp.longestSpan() << std::endl;
	}

	std::cout << "----------------------------------------" << std::endl;

	// Stress test with at least 10,000 numbers
	{
		const unsigned int N = 20000;
		Span sp(N);
		std::vector<int> big;
		big.reserve(N);
		for (unsigned int i = 0; i < N; ++i)
			big.push_back(std::rand());
		sp.addRange(big.begin(), big.end());
		std::cout << "Stress test with " << N << " numbers:" << std::endl;
		std::cout << "shortestSpan: " << sp.shortestSpan() << std::endl;
		std::cout << "longestSpan:  " << sp.longestSpan() << std::endl;
	}

	return 0;
}
