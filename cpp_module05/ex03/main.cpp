/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 17:43:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/04/25 17:43:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bureaucrat.hpp"
#include "Intern.hpp"
#include <cstdlib>
#include <ctime>

int main()
{
	std::srand(std::time(NULL));

	Intern intern;
	Bureaucrat chief("Chief", 1);
	Bureaucrat low("Low", 150);

	AForm *rrf = intern.makeForm("robotomy request", "Bender");
	AForm *scf = intern.makeForm("shrubbery creation", "home");
	AForm *ppf = intern.makeForm("presidential pardon", "Arthur Dent");
	AForm *bad = intern.makeForm("coffee request", "Office");

	if (rrf)
	{
		low.signForm(*rrf);
		chief.signForm(*rrf);
		chief.executeForm(*rrf);
		delete rrf;
	}
	if (scf)
	{
		chief.signForm(*scf);
		chief.executeForm(*scf);
		delete scf;
	}
	if (ppf)
	{
		chief.signForm(*ppf);
		chief.executeForm(*ppf);
		delete ppf;
	}
	if (bad)
		delete bad;

	return 0;
}
