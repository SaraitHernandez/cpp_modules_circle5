/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 05:05:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/06/06 11:45:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Array.hpp"
#include <iostream>
#include <string>

int main(void)
{
	std::cout << "--- empty array ---" << std::endl;
	Array<int> empty;
	std::cout << "size: " << empty.size() << std::endl;

	std::cout << "--- array of 5 ints (default initialized) ---" << std::endl;
	Array<int> numbers(5);
	std::cout << "size: " << numbers.size() << std::endl;
	for (unsigned int i = 0; i < numbers.size(); ++i)
	{
		numbers[i] = static_cast<int>(i * 10);
		std::cout << "numbers[" << i << "] = " << numbers[i] << std::endl;
	}

	std::cout << "--- deep copy ---" << std::endl;
	Array<int> copy(numbers);
	copy[0] = 999;
	std::cout << "original[0] = " << numbers[0]
			  << " | copy[0] = " << copy[0] << std::endl;

	std::cout << "--- assignment ---" << std::endl;
	Array<int> assigned;
	assigned = numbers;
	assigned[1] = 777;
	std::cout << "original[1] = " << numbers[1]
			  << " | assigned[1] = " << assigned[1] << std::endl;

	std::cout << "--- out of bounds access ---" << std::endl;
	try
	{
		std::cout << numbers[42] << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cout << "Exception caught: " << e.what() << std::endl;
	}

	std::cout << "--- string array ---" << std::endl;
	Array<std::string> words(3);
	words[0] = "hello";
	words[1] = "templates";
	words[2] = "world";
	for (unsigned int i = 0; i < words.size(); ++i)
		std::cout << "words[" << i << "] = " << words[i] << std::endl;

	return (0);
}
