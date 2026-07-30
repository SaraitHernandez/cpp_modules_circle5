/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BitcoinExchange.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 11:05:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/07/12 11:05:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BitcoinExchange.hpp"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <stdexcept>

BitcoinExchange::BitcoinExchange() {}

BitcoinExchange::BitcoinExchange(const BitcoinExchange &other) : _db(other._db) {}

BitcoinExchange &BitcoinExchange::operator=(const BitcoinExchange &other)
{
	if (this != &other)
		_db = other._db;
	return *this;
}

BitcoinExchange::~BitcoinExchange() {}

const char *BitcoinExchange::DatabaseError::what() const throw()
{
	return "could not open database file.";
}

// Trim surrounding whitespace of a string.
static std::string trim(const std::string &s)
{
	std::string::size_type start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	std::string::size_type end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

// Checks whether the string is only digits.
static bool isAllDigits(const std::string &s)
{
	if (s.empty())
		return false;
	for (std::string::size_type i = 0; i < s.size(); ++i)
		if (!std::isdigit(static_cast<unsigned char>(s[i])))
			return false;
	return true;
}

bool isValidDate(const std::string &date)
{
	// Expected format: YYYY-MM-DD
	if (date.size() != 10 || date[4] != '-' || date[7] != '-')
		return false;

	std::string y = date.substr(0, 4);
	std::string m = date.substr(5, 2);
	std::string d = date.substr(8, 2);

	if (!isAllDigits(y) || !isAllDigits(m) || !isAllDigits(d))
		return false;

	int year = std::atoi(y.c_str());
	int month = std::atoi(m.c_str());
	int day = std::atoi(d.c_str());

	if (month < 1 || month > 12 || day < 1 || day > 31)
		return false;

	int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	if (month == 2 && leap)
	{
		if (day > 29)
			return false;
	}
	else if (day > mdays[month - 1])
		return false;
	return true;
}

bool parseValue(const std::string &str, float &out, std::string &err)
{
	std::string s = trim(str);
	if (s.empty())
	{
		err = "bad input";
		return false;
	}

	// Validate the numeric form manually (float or positive integer).
	char *end = 0;
	double val = std::strtod(s.c_str(), &end);
	if (end == s.c_str() || *end != '\0')
	{
		err = "bad input";
		return false;
	}

	if (val < 0)
	{
		err = "not a positive number.";
		return false;
	}
	if (val > 1000)
	{
		err = "too large a number.";
		return false;
	}
	out = static_cast<float>(val);
	return true;
}

void BitcoinExchange::loadDatabase(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw DatabaseError();

	std::string line;
	std::getline(file, line); // header line "date,exchange_rate"
	while (std::getline(file, line))
	{
		if (trim(line).empty())
			continue;
		std::string::size_type comma = line.find(',');
		if (comma == std::string::npos)
			continue;
		std::string date = trim(line.substr(0, comma));
		std::string rateStr = trim(line.substr(comma + 1));
		if (date.empty() || rateStr.empty())
			continue;
		float rate = static_cast<float>(std::atof(rateStr.c_str()));
		_db[date] = rate;
	}
	file.close();
	if (_db.empty())
		throw DatabaseError();
}

float BitcoinExchange::getRate(const std::string &date) const
{
	std::map<std::string, float>::const_iterator it = _db.lower_bound(date);

	// Exact match found.
	if (it != _db.end() && it->first == date)
		return it->second;
	// No date lower or equal exists in the database.
	if (it == _db.begin())
		throw std::runtime_error("no earlier date in database.");
	// Use the closest lower date.
	--it;
	return it->second;
}

void BitcoinExchange::processInput(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Error: could not open file." << std::endl;
		return;
	}

	std::string line;
	std::getline(file, line); // header "date | value"
	// Tolerate the case where the first line is not the header.
	if (trim(line) != "date | value")
		file.seekg(0);

	while (std::getline(file, line))
	{
		if (trim(line).empty())
			continue;

		std::string::size_type bar = line.find('|');
		if (bar == std::string::npos)
		{
			std::cout << "Error: bad input => " << trim(line) << std::endl;
			continue;
		}

		std::string date = trim(line.substr(0, bar));
		std::string valStr = line.substr(bar + 1);

		if (!isValidDate(date))
		{
			std::cout << "Error: bad input => " << date << std::endl;
			continue;
		}

		float value;
		std::string err;
		if (!parseValue(valStr, value, err))
		{
			if (err == "bad input")
				std::cout << "Error: bad input => " << trim(valStr) << std::endl;
			else
				std::cout << "Error: " << err << std::endl;
			continue;
		}

		try
		{
			float rate = getRate(date);
			std::cout << date << " => " << value << " = " << (value * rate) << std::endl;
		}
		catch (std::exception &e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}
	}
	file.close();
}
