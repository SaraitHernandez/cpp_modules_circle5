/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 02:10:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/06/05 05:45:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "iter.hpp"
#include <iostream>
#include <string>

template <typename T>
void print(const T &value)
{
	std::cout << value << std::endl;
}

template <typename T>
void increment(T &value)
{
	value += 1;
}

int main(void)
{
	int numbers[] = {1, 2, 3, 4, 5};
	size_t intLen = sizeof(numbers) / sizeof(numbers[0]);

	std::cout << "--- int array (read) ---" << std::endl;
	iter(numbers, intLen, print<int>);

	std::cout << "--- int array (increment) ---" << std::endl;
	iter(numbers, intLen, increment<int>);
	iter(numbers, intLen, print<int>);

	std::string words[] = {"hello", "world", "templates"};
	size_t strLen = sizeof(words) / sizeof(words[0]);

	std::cout << "--- string array (read) ---" << std::endl;
	iter(words, strLen, print<std::string>);

	std::cout << "--- const array (read) ---" << std::endl;
	const int constNumbers[] = {10, 20, 30};
	iter(constNumbers, 3, print<int>);

	return (0);
}
