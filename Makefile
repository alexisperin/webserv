# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: marvin <marvin@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/12 11:18:06 by aperin            #+#    #+#              #
#    Updated: 2023/06/11 13:54:52 by marvin           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= webserv

SRCS_DIR	= srcs
OBJSDIR		= objs
INCDIR		= includes

SRC_FILES	= main utils Webserv Server Location Cgi

SRCS		= $(addprefix ${SRCS_DIR}/, $(addsuffix .cpp, ${SRC_FILES}))
OBJS		= $(addprefix ${OBJSDIR}/, $(addsuffix .o, $(basename ${SRC_FILES})))
OBJS_DIR	= $(sort $(dir $(OBJS)))

# CGI_FILES	= cookie form quotes wiki
# CGIS		= $(addprefix resources/cgi/cgi_, $(addsuffix .cpp, ${CGI_FILES}))
# CGI_EXE		= $(addprefix resources/cgi/, $(addsuffix .cgi, ${CGI_FILES}))

# ===---===---===---===---===---===---===---===---===---===---===---===---

CC			= c++
SAN			= -fsanitize=address -g
CPPFLAGS	= -Wall -Wextra -Werror -std=c++98
INCS		= $(foreach d, $(INCDIR), -I$d)

# ===---===---===---===---===---===---===---===---===---===---===---===---

${OBJSDIR}/%.o: ${SRCS_DIR}/%.cpp
			${CC} $(SAN) ${CPPFLAGS} ${INCS} -c -o $@ $<

# resources/cgi/%.cgi: resources/cgi/cgi_%.cpp
# 			${CC} -c -o $@ $<

all:		${OBJSDIR} ${NAME}

$(OBJSDIR):
			@mkdir -p ${OBJSDIR}

${NAME}:	${OBJS}# ${CGI_EXE}
			${CC} ${SAN} ${CPPFLAGS} ${OBJS} -o ${NAME}

clean:
			rm -rf ${OBJSDIR}

fclean:		clean
			rm -f ${NAME}
			# rm -f ${CGI_EXE}

re:			fclean all

.PHONY:		all clean fclean re
