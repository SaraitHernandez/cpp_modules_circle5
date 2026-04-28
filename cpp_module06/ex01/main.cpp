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

#include "Data.hpp"
#include "Serializer.hpp"
#include <cassert>
#include <iostream>

int main() {
	Data data;
	data.id = 42;
	data.label = "sarherna";

	Data* original = &data;
	uintptr_t raw = Serializer::serialize(original);
	std::cout << raw << std::endl;
	std::cout << &data << std::endl;
	std::cout << original << std::endl;
	Data* restored = Serializer::deserialize(raw);
	std::cout << raw << std::endl;
	std::cout << restored << std::endl;
	assert(restored == original);
	std::cout << "Pointer round-trip OK" << std::endl;
	std::cout << "id: " << restored->id << std::endl;
	std::cout << "label: " << restored->label << std::endl;
	return 0;
}
