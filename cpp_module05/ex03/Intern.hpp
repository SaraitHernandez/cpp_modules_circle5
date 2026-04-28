/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Intern.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 17:43:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/04/25 17:43:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INTERN_HPP
#define INTERN_HPP

#include "AForm.hpp"
#include <string>

class Intern
{
  private:
	AForm *createShrubbery(const std::string &target) const;
	AForm *createRobotomy(const std::string &target) const;
	AForm *createPardon(const std::string &target) const;

  public:
	Intern();
	Intern(const Intern &other);
	~Intern();

	Intern &operator=(const Intern &other);

	AForm *makeForm(const std::string &formName, const std::string &target) const;
};

#endif
