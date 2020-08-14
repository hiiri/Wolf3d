# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sadawi <sadawi@student.hive.fi>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2020/04/10 19:26:38 by sadawi            #+#    #+#              #
#    Updated: 2020/08/14 15:33:01 by sadawi           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = wolf3d
CFILES = main.c handle_map.c
SRCS = $(addprefix srcs/, $(CFILES))
OBJS = $(addprefix objs/, $(notdir $(SRCS:.c=.o)))
INCLUDES = -I libft/includes -I includes \
-I ~/.brew/Cellar/sdl2/2.0.12_1/include/SDL2/ \
-I ~/.brew/Cellar/sdl2_image/2.0.5/include/SDL2 \
-I ~/.brew/Cellar/sdl2_ttf/2.0.15/include/SDL2
FLAGS = -Wall -Wextra -Werror -O3
RUN_LIB = make --no-print-directory -C libft/
LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf \
-L ~/.brew/Cellar/sdl2/2.0.12_1/lib/ \
-L ~/.brew/Cellar/sdl2_image/2.0.5/lib/ \
-L ~/.brew/Cellar/sdl2_ttf/2.0.15/lib/

all:
	@$(RUN_LIB)
	@make --no-print-director $(NAME)

$(NAME): $(SRCS) libft/
	@rm -rf objs
	@echo Compiling $(NAME)...
	@gcc $(FLAGS) $(INCLUDES) -c $(SRCS)
	@mkdir objs
	@mv $(notdir $(SRCS:.c=.o)) objs
	@gcc $(FLAGS) $(INCLUDES) -o $(NAME) $(OBJS) $(LIBS) libft/libft.a
	@echo $(NAME) compiled succesfully!

lib:
	@$(RUN_LIB)

noflags:
	@rm -rf objs
	@echo Compiling $(NAME) without flags...
	@gcc $(INCLUDES) -c $(SRCS)
	@mkdir objs
	@mv $(notdir $(SRCS:.c=.o)) objs
	@gcc $(INCLUDES) -o $(NAME) $(OBJS) libft/libft.a $(LIBS)
	@echo $(NAME) compiled without flags succesfully!

clean:
	@/bin/rm -f $(OBJS)
	@rm -rf objs
	@make -C libft/ clean
	@echo Clean successful!

clean_binary:
	@/bin/rm -f $(OBJS)
	@rm -rf objs
	@echo Clean successful!

fclean: clean_binary
	@/bin/rm -f $(NAME)
	@make -C libft/ fclean
	@echo Clean successful!

re: fclean all
