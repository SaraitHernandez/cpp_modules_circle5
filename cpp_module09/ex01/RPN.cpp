/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RPN.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 11:15:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/07/12 11:15:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RPN.hpp"
#include <cctype>

RPN::RPN() {}

RPN::RPN(const RPN &other) : _stack(other._stack) {}

RPN &RPN::operator=(const RPN &other)
{
	if (this != &other)
		_stack = other._stack;
	return *this;
}

RPN::~RPN() {}

const char *RPN::RPNError::what() const throw()
{
	return "Error";
}

bool RPN::isOperator(char c) const
{
	return (c == '+' || c == '-' || c == '*' || c == '/');
}

int RPN::applyOperator(char op, int a, int b)
{
	switch (op)
	{
		case '+':
			return a + b;
		case '-':
			return a - b;
		case '*':
			return a * b;
		case '/':
			if (b == 0)
				throw RPNError();
			return a / b;
	}
	throw RPNError();
}

int RPN::evaluate(const std::string &expr)
{
	for (std::string::size_type i = 0; i < expr.size(); ++i)
	{
		char c = expr[i];

		if (std::isspace(static_cast<unsigned char>(c)))
			continue;
		else if (std::isdigit(static_cast<unsigned char>(c)))
		{
			// Numbers passed as arguments are always less than 10.
			_stack.push(c - '0');
		}
		else if (isOperator(c))
		{
			if (_stack.size() < 2)
				throw RPNError();
			int b = _stack.top();
			_stack.pop();
			int a = _stack.top();
			_stack.pop();
			_stack.push(applyOperator(c, a, b));
		}
		else
			throw RPNError(); // Any unexpected token.
	}

	if (_stack.size() != 1)
		throw RPNError();
	return _stack.top();
}
