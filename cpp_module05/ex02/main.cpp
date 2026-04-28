/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 17:36:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/04/25 17:36:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bureaucrat.hpp"
#include "PresidentialPardonForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "ShrubberyCreationForm.hpp"
#include <cstdlib>
#include <ctime>

int main()
{
	std::srand(std::time(NULL));

	Bureaucrat chief("Chief", 1);
	Bureaucrat worker("Worker", 140);

	ShrubberyCreationForm shrub("home");
	RobotomyRequestForm robot("Bender");
	PresidentialPardonForm pardon("Ford Prefect");

	std::cout << shrub << std::endl;
	std::cout << robot << std::endl;
	std::cout << pardon << std::endl;

	worker.signForm(shrub);
	worker.executeForm(shrub);

	worker.signForm(robot);
	worker.executeForm(robot);

	chief.signForm(robot);
	chief.executeForm(robot);

	chief.signForm(pardon);
	chief.executeForm(pardon);

	return 0;
}
