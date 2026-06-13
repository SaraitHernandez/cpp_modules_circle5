/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MutantStack.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 12:20:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/06/13 12:42:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MUTANTSTACK_HPP
# define MUTANTSTACK_HPP

# include <stack>
# include <deque>

template <typename T, typename Container = std::deque<T> >
class MutantStack : public std::stack<T, Container>
{
	public:
		MutantStack(void) {}
		MutantStack(const MutantStack& other) : std::stack<T, Container>(other) {}
		MutantStack& operator=(const MutantStack& other)
		{
			std::stack<T, Container>::operator=(other);
			return *this;
		}
		~MutantStack(void) {}

		typedef typename Container::iterator				iterator;
		typedef typename Container::const_iterator			const_iterator;
		typedef typename Container::reverse_iterator		reverse_iterator;
		typedef typename Container::const_reverse_iterator	const_reverse_iterator;

		// std::stack stores its underlying container in the protected member 'c'.
		iterator begin(void) { return this->c.begin(); }
		iterator end(void) { return this->c.end(); }

		const_iterator begin(void) const { return this->c.begin(); }
		const_iterator end(void) const { return this->c.end(); }

		reverse_iterator rbegin(void) { return this->c.rbegin(); }
		reverse_iterator rend(void) { return this->c.rend(); }

		const_reverse_iterator rbegin(void) const { return this->c.rbegin(); }
		const_reverse_iterator rend(void) const { return this->c.rend(); }
};

#endif
