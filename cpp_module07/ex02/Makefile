# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sarherna <sarherna@student.42.fr>         +#+   +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/06 09:00:00 by sarherna          #+#    #+#              #
#    Updated: 2026/06/06 11:45:00 by sarherna         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= array
CXX		= c++
CXXFLAGS= -Wall -Wextra -Werror -std=c++98
RM		= rm -f

SRCS	= main.cpp
OBJS	= $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
