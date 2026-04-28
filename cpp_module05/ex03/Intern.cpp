/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Intern.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 17:43:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/04/25 17:43:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Intern.hpp"
#include "PresidentialPardonForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "ShrubberyCreationForm.hpp"
#include <iostream>

Intern::Intern()
{
}

Intern::Intern(const Intern &other)
{
	(void)other;
}

Intern::~Intern()
{
}

Intern &Intern::operator=(const Intern &other)
{
	(void)other;
	return *this;
}

AForm *Intern::createShrubbery(const std::string &target) const
{
	return new ShrubberyCreationForm(target);
}

AForm *Intern::createRobotomy(const std::string &target) const
{
	return new RobotomyRequestForm(target);
}

AForm *Intern::createPardon(const std::string &target) const
{
	return new PresidentialPardonForm(target);
}

AForm *Intern::makeForm(const std::string &formName, const std::string &target) const
{
	std::string names[3] = {"shrubbery creation", "robotomy request", "presidential pardon"};
	AForm *(Intern::*creators[3])(const std::string &) const = {
		&Intern::createShrubbery,
		&Intern::createRobotomy,
		&Intern::createPardon};

	for (int i = 0; i < 3; ++i)
	{
		if (formName == names[i])
		{
			std::cout << "Intern creates " << formName << std::endl;
			return (this->*creators[i])(target);
		}
	}
	std::cout << "Intern couldn't create form: unknown form name \"" << formName << "\"."
			  << std::endl;
	return NULL;
}
