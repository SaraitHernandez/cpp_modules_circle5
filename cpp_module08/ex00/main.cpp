/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 12:01:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/06/12 12:30:01 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "easyfind.hpp"
#include <iostream>
#include <vector>
#include <list>

int main(void)
{
	std::vector<int> vec;
	for (int i = 0; i < 10; ++i)
		vec.push_back(i * 2);

	// Found case
	try
	{
		std::vector<int>::iterator it = easyfind(vec, 8);
		std::cout << "Found value 8 at index "
			<< (it - vec.begin()) << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	// Not found case
	try
	{
		easyfind(vec, 7);
		std::cout << "Value 7 found" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	// First occurrence with a std::list containing duplicates
	std::list<int> lst;
	lst.push_back(42);
	lst.push_back(21);
	lst.push_back(42);
	try
	{
		std::list<int>::iterator it = easyfind(lst, 42);
		std::cout << "First occurrence of 42 found: " << *it << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	return 0;
}
