/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AForm.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 17:36:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/04/25 17:36:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AForm.hpp"
#include "Bureaucrat.hpp"

AForm::AForm() : _name("Default"), _signed(false), _signGrade(150), _execGrade(150)
{
}

AForm::AForm(const std::string &name, int signGrade, int execGrade)
	: _name(name), _signed(false), _signGrade(signGrade), _execGrade(execGrade)
{
	if (signGrade < 1 || execGrade < 1)
		throw GradeTooHighException();
	if (signGrade > 150 || execGrade > 150)
		throw GradeTooLowException();
}

AForm::AForm(const AForm &other)
	: _name(other._name),
	  _signed(other._signed),
	  _signGrade(other._signGrade),
	  _execGrade(other._execGrade)
{
}

AForm::~AForm()
{
}

AForm &AForm::operator=(const AForm &other)
{
	if (this != &other)
		_signed = other._signed;
	return *this;
}

const std::string &AForm::getName() const
{
	return _name;
}

bool AForm::getIsSigned() const
{
	return _signed;
}

int AForm::getSignGrade() const
{
	return _signGrade;
}

int AForm::getExecGrade() const
{
	return _execGrade;
}

void AForm::beSigned(const Bureaucrat &bureaucrat)
{
	if (bureaucrat.getGrade() > _signGrade)
		throw GradeTooLowException();
	_signed = true;
}

void AForm::execute(Bureaucrat const &executor) const
{
	if (!_signed)
		throw FormNotSignedException();
	if (executor.getGrade() > _execGrade)
		throw GradeTooLowException();
	executeAction();
}

const char *AForm::GradeTooHighException::what() const throw()
{
	return "grade is too high";
}

const char *AForm::GradeTooLowException::what() const throw()
{
	return "grade is too low";
}

const char *AForm::FormNotSignedException::what() const throw()
{
	return "form is not signed";
}

std::ostream &operator<<(std::ostream &os, const AForm &form)
{
	os << "Form " << form.getName()
	   << ", signed: " << (form.getIsSigned() ? "yes" : "no")
	   << ", sign grade: " << form.getSignGrade()
	   << ", exec grade: " << form.getExecGrade() << ".";
	return os;
}
