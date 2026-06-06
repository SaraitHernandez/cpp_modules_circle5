/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   iter.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 08:45:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/06/05 11:45:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ITER_HPP
#define ITER_HPP

#include <cstddef>

template <typename T, typename F>
void iter(T *array, const size_t length, F f)
{
	for (size_t i = 0; i < length; ++i)
		f(array[i]);
}

template <typename T, typename F>
void iter(const T *array, const size_t length, F f)
{
	for (size_t i = 0; i < length; ++i)
		f(array[i]);
}

#endif
