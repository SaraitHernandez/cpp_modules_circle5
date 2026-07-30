/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 11:05:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/07/12 11:05:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BitcoinExchange.hpp"
#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Error: could not open file." << std::endl;
		return 1;
	}

	BitcoinExchange btc;
	try
	{
		btc.loadDatabase("data.csv");
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	btc.processInput(argv[1]);
	return 0;
}
