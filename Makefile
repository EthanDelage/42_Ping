NAME		=		ft_ping

#######################
#	DIR
#######################

SRC_DIR		=		src/

INC_DIR		=		include/

BUILD_DIR	=		.build/

#######################
#	FILES
#######################

SRC			=		main.c \
					options.c \
					common.c \
					rtt.c \
					ping.c \
					socket.c \
					icmp.c \
					print.c


OBJ			=		$(addprefix $(BUILD_DIR), $(SRC:.c=.o))

DEPS		=		$(OBJ:.o=.d)

#######################
#	FLAGS
#######################

CFLAGS		=		-Wall -Werror -Wextra

IFLAGS		=		-I $(INC_DIR)

LFLAGS		=		-lm

DFLAGS		=		-MMD -MP

#######################
#	RULES
#######################

############
#	GENERAL
############

.PHONY:				all
all:				$(NAME)

.PHONY:				run
run:				all
					./$(NAME)

.PHONY:				clean
clean:
					$(RM) $(OBJ) $(DEPS)

.PHONY:				fclean
fclean:
					$(RM) $(OBJ) $(DEPS) $(NAME)

.PHONY:				re
re:					fclean
					$(MAKE)

.PHONY:				rerun
rerun:				re
					$(MAKE) run

.PHONY:				format
format:
					find . -name "*.c" -o -name "*.h" | xargs clang-format -i

################
#	EXECUTABLES
################

-include			$(DEPS)

$(NAME):			$(OBJ)
					$(CC) $(CFLAGS) $(IFLAGS) $^ $(LFLAGS) -o $@

##################
#	OBJECTS FILES
##################

$(BUILD_DIR)%.o:	$(SRC_DIR)%.c
					mkdir -p $(shell dirname $@)
					$(CC) $(CFLAGS) $(DFLAGS) $(IFLAGS) -c $< -o $@