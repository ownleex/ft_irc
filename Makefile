# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ayarmaya <ayarmaya@student.42nice.fr>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/22 09:30:05 by cparodi           #+#    #+#              #
#    Updated: 2025/08/16 01:37:27 by ayarmaya         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Program Executable
EXE = ircsrv
BOTEXE = channelbot

# Directories
SRCDIR = srcs/
INCDIR = includes/
OBJDIR = obj/

# Files
SRC =		main.cpp \
			Client.cpp \
			Server.cpp \
			Channel.cpp \
			CommandHandler_Core.cpp \
			CommandHandler_Channel.cpp \
			CommandHandler_Auth.cpp \
			CommandHandler_Messages.cpp \
			CommandHandler_Utils.cpp

BOT_SRC = 	srcs/Bot.cpp

# Headers
HEADERS =	Client.hpp \
			Server.hpp \
			CommandHandler.hpp \
			Channel.hpp

SOURCES =		$(addprefix $(SRCDIR), $(SRC))
OBJECTS =		$(patsubst $(SRCDIR)%.cpp, $(OBJDIR)%.o, $(SOURCES))
DEPENDS =		$(OBJECTS:.o=.d)

BOT_OBJECTS = $(patsubst $(SRCDIR)%.cpp, $(OBJDIR)%.o, $(BOT_SRC))

# Variables
CC		= c++
CFLAGS	= -std=c++98 -Wall -Werror -Wextra -MMD -MP -I$(INCDIR)
RM		= rm -f

# Loading Bar
RESET		= \e[0m
UP			= \e[A

WHITE		= \e[0;1;97m
_WHITE		= \e[1;4;97m
RED			= \e[0;1;31m
GREEN		= \e[0;1;32m
_GREEN		= \e[1;4;32m

FILE_COUNT	= 0
FILE_TOTAL	= 10
BAR_SIZE	= ${shell expr 100 \* ${FILE_COUNT} / ${FILE_TOTAL}}
BAR_LOAD	= ${shell expr 23 \* ${FILE_COUNT} / ${FILE_TOTAL}}
BAR_REST	= ${shell expr 23 - ${BAR_LOAD}}

# Makefile
all:		${EXE} ${BOTEXE}

${EXE}:		${OBJECTS}
		@${CC} ${CFLAGS} ${OBJECTS} -o ${EXE}
		@echo "\n\n${GREEN}[✓] - ${_GREEN}ft_irc${GREEN} Successfully Compiled!${RESET}"

${BOTEXE}: ${BOT_OBJECTS}
		@${CC} ${CFLAGS} ${BOT_OBJECTS} -o ${BOTEXE}
		@echo "\n\n${GREEN}[✓] - ${_GREEN}ft_irc${GREEN} BOT Successfully Compiled!${RESET}"

${OBJDIR}:
		@mkdir -p ${OBJDIR}

$(OBJDIR)%.o:	$(SRCDIR)%.cpp | ${OBJDIR}
		@${eval FILE_COUNT = ${shell expr ${FILE_COUNT} + 1}}
		@${CC} ${CFLAGS} -c $< -o $@
		@echo "${WHITE}[!] - Compilation loading...${RESET}"
		@printf "${WHITE}[${GREEN}%-.${BAR_LOAD}s${RED}%-.${BAR_REST}s${WHITE}] [%d/%d (%d%%)]${RESET}" "#######################" "#######################" ${FILE_COUNT} ${FILE_TOTAL} ${BAR_SIZE}
		@echo ""
		@echo "${UP}${UP}${UP}"

clean:
		@${RM} -r ${OBJDIR}
		@echo "${WHITE}[!] - ${_WHITE}ft_irc${WHITE} Successfully Cleaned!${RESET}"

fclean: 	clean
		@${RM} ${EXE} ${BOTEXE}

re:			fclean all

-include $(DEPENDS)

.PHONY: all clean fclean re