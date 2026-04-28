/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 00:00:00 by sarherna            #+#    #+#             */
/*   Updated: 2026/04/25 00:00:00 by sarherna           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Identify.hpp"
#include "Base.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>

int main() {
	std::srand(static_cast<unsigned int>(std::time(0)));

	for (int i = 0; i < 6; ++i) {
		Base* p = generate();
		std::cout << "pointer: ";
		identify(p);
		std::cout << "reference: ";
		identify(*p);
		delete p;
	}
	return 0;
}
