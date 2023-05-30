# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/12 11:18:06 by aperin            #+#    #+#              #
#    Updated: 2023/05/30 11:01:12 by yhuberla         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= webserv

SRCS_DIR	= srcs
OBJSDIR		= objs
INCDIR		= includes

SRC_FILE	= main utils Webserv Server Location

SRCS		= $(addprefix ${SRCS_DIR}/, $(addsuffix .cpp, ${SRC_FILE}))
OBJS		= $(addprefix ${OBJSDIR}/, $(addsuffix .o, $(basename ${SRC_FILE})))
OBJS_DIR	= $(sort $(dir $(OBJS)))

# ===---===---===---===---===---===---===---===---===---===---===---===---

CC			= c++
SAN			= -fsanitize=address -g
CPPFLAGS	= -Wall -Wextra -Werror -std=c++98
INCS		= $(foreach d, $(INCDIR), -I$d)

# ===---===---===---===---===---===---===---===---===---===---===---===---

${OBJSDIR}/%.o: ${SRCS_DIR}/%.cpp
			${CC} $(SAN) ${CPPFLAGS} ${INCS} -c -o $@ $<

all:		${OBJSDIR} ${NAME}

$(OBJSDIR):
			@mkdir -p ${OBJSDIR}

${NAME}:	${OBJS}
			${CC} ${SAN} ${CPPFLAGS} ${OBJS} -o ${NAME}

clean:
			rm -rf ${OBJSDIR}

fclean:		clean
			rm -f ${NAME}

re:			fclean all

.PHONY:		all clean fclean re
