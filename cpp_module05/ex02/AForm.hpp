/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AForm.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 17:36:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/04/25 17:36:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AFORM_HPP
#define AFORM_HPP

#include <exception>
#include <iostream>
#include <string>

class Bureaucrat;

class AForm
{
  private:
	const std::string	_name;
	bool				_signed;
	const int			_signGrade;
	const int			_execGrade;

  protected:
	virtual void executeAction() const = 0;

  public:
	AForm();
	AForm(const std::string &name, int signGrade, int execGrade);
	AForm(const AForm &other);
	virtual ~AForm();

	AForm &operator=(const AForm &other);

	const std::string	&getName() const;
	bool				getIsSigned() const;
	int					getSignGrade() const;
	int					getExecGrade() const;

	void	beSigned(const Bureaucrat &bureaucrat);
	void	execute(Bureaucrat const &executor) const;

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

	class FormNotSignedException : public std::exception
	{
	  public:
		virtual const char *what() const throw();
	};
};

std::ostream &operator<<(std::ostream &os, const AForm &form);

#endif
