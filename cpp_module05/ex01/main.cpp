/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 17:27:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/04/25 17:27:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bureaucrat.hpp"
#include "Form.hpp"

int main()
{
	try
	{
		Bureaucrat chief("Chief", 10);
		Bureaucrat intern("Intern", 150);
		Form leaveRequest("LeaveRequest", 20, 50);

		std::cout << chief << std::endl;
		std::cout << intern << std::endl;
		std::cout << leaveRequest << std::endl;

		intern.signForm(leaveRequest);
		chief.signForm(leaveRequest);
		std::cout << leaveRequest << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}

	try
	{
		Form invalid("Invalid", 80, 151);
		std::cout << invalid << std::endl;
	}
	catch (std::exception &e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
