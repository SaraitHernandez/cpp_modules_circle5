/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BitcoinExchange.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 11:05:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/07/12 11:05:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BITCOINEXCHANGE_HPP
# define BITCOINEXCHANGE_HPP

# include <string>
# include <map>

class BitcoinExchange
{
	private:
		std::map<std::string, float> _db;

	public:
		BitcoinExchange();
		BitcoinExchange(const BitcoinExchange &other);
		BitcoinExchange &operator=(const BitcoinExchange &other);
		~BitcoinExchange();

		// Loads the price database (csv "date,exchange_rate").
		void loadDatabase(const std::string &filename);
		// Reads the input file and prints the computed values line by line.
		void processInput(const std::string &filename);

		// Returns the exchange rate for the given date, using the closest
		// lower date when the exact date is not present.
		float getRate(const std::string &date) const;

		class DatabaseError : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};
};

// Helpers (also used by main / process logic).
bool isValidDate(const std::string &date);
bool parseValue(const std::string &str, float &out, std::string &err);

#endif
