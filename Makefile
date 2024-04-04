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

SRC			=		main.c	\
					validate_params.c	\
					socket_initialization.c	\
					options/validate_options.c	\
					utils/print_params.c	\
					resolve_host.c


OBJ			=		$(addprefix $(BUILD_DIR), $(SRC:.c=.o))

DEPS		=		$(OBJ:.o=.d)

#######################
#	FLAGS
#######################

CFLAGS		=		-Wall -Werror -Wextra

IFLAGS		=		-I $(INC_DIR)

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

################
#	EXECUTABLES
################

-include			$(DEPS)

$(NAME):			$(OBJ)
					$(CC) $(CFLAGS) $(IFLAGS) $^ -o $@

##################
#	OBJECTS FILES
##################

$(BUILD_DIR)%.o:	$(SRC_DIR)%.c
					mkdir -p $(shell dirname $@)
					$(CC) $(CFLAGS) $(DFLAGS) $(IFLAGS) -c $< -o $@