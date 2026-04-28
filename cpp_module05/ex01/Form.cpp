/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Form.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 17:27:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/04/25 17:27:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Form.hpp"
#include "Bureaucrat.hpp"

Form::Form() : _name("Default"), _signed(false), _signGrade(150), _execGrade(150)
{
}

Form::Form(const std::string &name, int signGrade, int execGrade)
	: _name(name), _signed(false), _signGrade(signGrade), _execGrade(execGrade)
{
	if (signGrade < 1 || execGrade < 1)
		throw GradeTooHighException();
	if (signGrade > 150 || execGrade > 150)
		throw GradeTooLowException();
}

Form::Form(const Form &other)
	: _name(other._name),
	  _signed(other._signed),
	  _signGrade(other._signGrade),
	  _execGrade(other._execGrade)
{
}

Form::~Form()
{
}

Form &Form::operator=(const Form &other)
{
	if (this != &other)
		_signed = other._signed;
	return *this;
}

const std::string &Form::getName() const
{
	return _name;
}

bool Form::getIsSigned() const
{
	return _signed;
}

int Form::getSignGrade() const
{
	return _signGrade;
}

int Form::getExecGrade() const
{
	return _execGrade;
}

void Form::beSigned(const Bureaucrat &bureaucrat)
{
	if (bureaucrat.getGrade() > _signGrade)
		throw GradeTooLowException();
	_signed = true;
}

const char *Form::GradeTooHighException::what() const throw()
{
	return "grade or exec is too high";
}

const char *Form::GradeTooLowException::what() const throw()
{
	return "grade or exec is too low";
}

std::ostream &operator<<(std::ostream &os, const Form &form)
{
	os << "Form " << form.getName()
	   << ", signed: " << (form.getIsSigned() ? "yes" : "no")
	   << ", sign grade: " << form.getSignGrade()
	   << ", exec grade: " << form.getExecGrade() << ".";
	return os;
}
