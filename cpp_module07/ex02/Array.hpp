/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Array.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 09:15:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/06/06 11:45:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <cstddef>
#include <exception>

template <typename T>
class Array
{
private:
	T				*_data;
	unsigned int	_size;

public:
	Array(void);
	Array(unsigned int n);
	Array(const Array &other);
	Array &operator=(const Array &other);
	~Array(void);

	T &operator[](unsigned int index);
	const T &operator[](unsigned int index) const;

	unsigned int size(void) const;

	class OutOfBoundsException : public std::exception
	{
	public:
		virtual const char *what(void) const throw();
	};
};

#include "Array.tpp"

#endif
