# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aperin <aperin@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/12 11:18:06 by aperin            #+#    #+#              #
#    Updated: 2023/05/17 13:14:16 by aperin           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= webserv

SRC_FILE	= main.cpp\
				Webserv.cpp
				
SRCS_DIR	= srcs
OBJSDIR		= objs
INCDIR		= includes

SRCS		= $(addprefix ${SRCS_DIR}/, ${SRC_FILE})
OBJS		= $(addprefix ${OBJSDIR}/, $(addsuffix .o, $(basename ${SRC_FILE})))
OBJS_DIR	= $(sort $(dir $(OBJS)))

CC			= c++
SAN			= -fsanitize=address -g
CPPFLAGS	= -Wall -Wextra -Werror -std=c++98
INCS		= $(foreach d, $(INCDIR), -I$d)

${OBJSDIR}/%.o: ${SRCS_DIR}/%.cpp
			@mkdir -p ${OBJSDIR} ${OBJS_DIR}
			${CC} $(SAN) ${CPPFLAGS} ${INCS} -c -o $@ $<

all:		${NAME}

${NAME}:	${OBJS}
			${CC} ${SAN} ${CPPFLAGS} ${OBJS} -o ${NAME}

clean:
			rm -rf ${OBJSDIR}

fclean:		clean
			rm -f ${NAME}

re:			fclean all

.PHONY:		all clean fclean re