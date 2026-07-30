# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/12 11:30:00 by sarherna          #+#    #+#              #
#    Updated: 2026/07/12 11:30:00 by sarherna         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= PmergeMe
CXX		= c++
CXXFLAGS= -Wall -Wextra -Werror -std=c++98
RM		= rm -f

SRCS	= main.cpp PmergeMe.cpp
OBJS	= $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp PmergeMe.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
