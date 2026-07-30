/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PmergeMe.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 11:30:00 by sarherna          #+#    #+#             */
/*   Updated: 2026/07/12 11:30:00 by sarherna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PMERGEME_HPP
# define PMERGEME_HPP

# include <string>
# include <vector>
# include <deque>

class PmergeMe
{
	private:
		std::vector<int> _vec;
		std::deque<int>  _deq;

		// Ford-Johnson (merge-insert) sort implemented for each container.
		void _sortVector();
		void _sortDeque();

	public:
		PmergeMe();
		PmergeMe(const PmergeMe &other);
		PmergeMe &operator=(const PmergeMe &other);
		~PmergeMe();

		// Parses argv (positive integers) into both containers.
		// Throws std::exception on any invalid token.
		void parse(int argc, char **argv);

		// Runs the whole program: prints before/after and both timings.
		void run();
};

#endif
