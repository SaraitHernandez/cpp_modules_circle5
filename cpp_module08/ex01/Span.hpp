/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Span.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 01:03:30 by sarherna          #+#    #+#             */
/*   Updated: 2026/06/12 01:20:12 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SPAN_HPP
# define SPAN_HPP

# include <vector>
# include <stdexcept>
# include <iterator>

class Span
{
	private:
		unsigned int		_maxSize;
		std::vector<int>	_numbers;

	public:
		Span(void);
		Span(unsigned int n);
		Span(const Span& other);
		Span& operator=(const Span& other);
		~Span(void);

		void			addNumber(int number);
		unsigned int	shortestSpan(void) const;
		unsigned int	longestSpan(void) const;

		template <typename Iterator>
		void addRange(Iterator begin, Iterator end)
		{
			if (_numbers.size() + static_cast<unsigned int>(std::distance(begin, end)) > _maxSize)
				throw std::out_of_range("Span is full: range too large");
			_numbers.insert(_numbers.end(), begin, end);
		}

		// Custom exceptions
		class FullSpanException : public std::exception
		{
			public:
				virtual const char* what(void) const throw();
		};

		class NoSpanException : public std::exception
		{
			public:
				virtual const char* what(void) const throw();
		};
};

#endif
