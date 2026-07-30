/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RPN.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 11:15:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/07/12 11:15:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RPN_HPP
# define RPN_HPP

# include <string>
# include <stack>
# include <exception>

class RPN
{
	private:
		std::stack<int> _stack;

		bool isOperator(char c) const;
		int applyOperator(char op, int a, int b);

	public:
		RPN();
		RPN(const RPN &other);
		RPN &operator=(const RPN &other);
		~RPN();

		// Evaluates the given Reverse Polish Notation expression.
		int evaluate(const std::string &expr);

		class RPNError : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};
};

#endif
