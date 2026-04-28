/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Form.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 17:27:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/04/25 17:27:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FORM_HPP
#define FORM_HPP

#include <exception>
#include <iostream>
#include <string>

class Bureaucrat;

class Form
{
  private:
	const std::string	_name;
	bool				_signed;
	const int			_signGrade;
	const int			_execGrade;

  public:
	Form();
	Form(const std::string &name, int signGrade, int execGrade);
	Form(const Form &other);
	~Form();

	Form &operator=(const Form &other);

	const std::string	&getName() const;
	bool				getIsSigned() const;
	int					getSignGrade() const;
	int					getExecGrade() const;

	void	beSigned(const Bureaucrat &bureaucrat);

	class GradeTooHighException : public std::exception
	{
	  public:
		virtual const char *what() const throw();
	};

	class GradeTooLowException : public std::exception
	{
	  public:
		virtual const char *what() const throw();
	};
};

std::ostream &operator<<(std::ostream &os, const Form &form);

#endif
