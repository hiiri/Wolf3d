/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sadawi <sadawi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/08/12 19:47:21 by sadawi            #+#    #+#             */
/*   Updated: 2020/08/14 13:51:09 by sadawi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "wolf3d.h"

void	close_sdl(t_sdl *sdl)
{
	SDL_DestroyRenderer(sdl->renderer);
	sdl->renderer = NULL;
	SDL_FreeSurface(sdl->image);
	sdl->image = NULL;
	SDL_DestroyWindow(sdl->window);
	sdl->window = NULL;
	free(sdl);
	IMG_Quit();
	SDL_Quit();
	exit(0);
}

void	handle_error_sdl(char *message)
{
	ft_printf("SDL Error: %s SDL_ERROR %s\n", message, SDL_GetError());
	exit(0);
}

void	handle_error(char *message)
{
	ft_printf("Error: %s\n", message);
	exit(0);
}

t_sdl	*init(void)
{
	t_sdl *sdl;

	if (!(sdl = (t_sdl*)ft_memalloc(sizeof(t_sdl))))
		handle_error("Malloc failed");
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		handle_error_sdl("SDL could not initialize!");
	sdl->window = SDL_CreateWindow(PROGRAM_TITLE, 100, 100,
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!sdl->window)
		handle_error_sdl("Window could not be created!");
	if (!(sdl->renderer = SDL_CreateRenderer(sdl->window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)))
		handle_error_sdl("Renderer could not be created!");
	sdl->img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(sdl->img_flags) & sdl->img_flags))
		handle_error_sdl("SDL image could not initialize!");
	sdl->screen = SDL_GetWindowSurface(sdl->window);
	return (sdl);
}



void		set_spawn_point(t_player *player, t_map *map)
{
	int	col;
	int	row;

	col = 0;
	row = 0;
	while (row < map->rows)
	{
		col = 0;
		while (col < map->cols)
		{

			if (map->map[row][col] == PLAYER_SPAWN_POINT)
			{
				player->spawn_x = (double)col;
				player->spawn_y = (double)row;
				return;
			}
			col++;
		}
		row++;
	}
	handle_error("No spawn point found in map.");
}

t_player	*init_player(t_map *map)
{
	t_player *player;

	if (!(player = (t_player*)ft_memalloc(sizeof(*player))))
		handle_error("Malloc failed");
	set_spawn_point(player, map);
	player->posX = player->spawn_x;
	player->posY = player->spawn_y;
	ft_printf("%f %f\n", player->spawn_x, player->spawn_y);
	player->dirX = -1;
	player->dirY = 0;
	player->planeX = 0;
	player->planeY = 0.66;
	return (player);
}

void	draw_vertical_line(t_sdl *sdl, int x, int y[2], int color)
{
	SDL_SetRenderDrawColor(sdl->renderer, color / 256 / 256 % 256, color / 256 % 256, color % 256, 0xFF);
	// while (y[0] < y[1])
	// 	SDL_RenderDrawPoint(sdl->renderer, x, y[0]++);
	SDL_RenderDrawLine(sdl->renderer, x, y[0], x, y[1]);
	//write pixels to buffer then save them all to renderer at the same tiime
}

void	draw_map(t_sdl *sdl)
{
	for(int x = 0; x < SCREEN_WIDTH; x++)
    {
      //calculate ray position and direction
      double cameraX = 2 * x / (double)SCREEN_WIDTH - 1; //x-coordinate in camera space
      double rayDirX = sdl->player->dirX + sdl->player->planeX * cameraX;
      double rayDirY = sdl->player->dirY + sdl->player->planeY * cameraX;
      //which box of the map we're in
      int mapX = (int)sdl->player->posX;
      int mapY = (int)sdl->player->posY;

      //length of ray from current position to next x or y-side
      double sideDistX;
      double sideDistY;

       //length of ray from one x or y-side to next x or y-side
      double deltaDistX = fabs(1 / rayDirX);
      double deltaDistY = fabs(1 / rayDirY);
      double perpWallDist;

      //what direction to step in x or y-direction (either +1 or -1)
      int stepX;
      int stepY;

      int hit = 0; //was there a wall hit?
      int side; //was a NS or a EW wall hit?
      //calculate step and initial sideDist
      if(rayDirX < 0)
      {
        stepX = -STEP_DIST_X;
        sideDistX = (sdl->player->posX - mapX) * deltaDistX;
      }
      else
      {
        stepX = STEP_DIST_X;
        sideDistX = (mapX + 1.0 - sdl->player->posX) * deltaDistX;
      }
      if(rayDirY < 0)
      {
        stepY = -STEP_DIST_Y;
        sideDistY = (sdl->player->posY - mapY) * deltaDistY;
      }
      else
      {
        stepY = STEP_DIST_Y;
        sideDistY = (mapY + 1.0 - sdl->player->posY) * deltaDistY;
      }
      //perform DDA
      while (hit == 0)
      {
        //jump to next map square, OR in x-direction, OR in y-direction
        if(sideDistX < sideDistY)
        {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        }
        else
        {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if(sdl->map->map[mapX][mapY] > 0) hit = 1;
      }
      //Calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
      if(side == 0) perpWallDist = (mapX - sdl->player->posX + (1 - stepX) / 2) / rayDirX;
      else          perpWallDist = (mapY - sdl->player->posY + (1 - stepY) / 2) / rayDirY;

      //Calculate height of line to draw on screen
      int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

      //calculate lowest and highest pixel to fill in current stripe
      int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
      if(drawStart < 0)drawStart = 0;
      int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
      if(drawEnd >= SCREEN_HEIGHT)drawEnd = SCREEN_HEIGHT - 1;

      //choose wall color
    int color;

	color = 0xFFFF;
      switch(sdl->map->map[mapX][mapY])
      {
        case 1:  color = 0xFF0000;    break; //red
        case 2:  color = 0x00FF00;  break; //green
        case 3:  color = 0x0000FF;   break; //blue
        case 4:  color = 0xFFFFFF;  break; //white
        default: color = 0x00FFFF; break; //yellow
      }

      //give x and y sides different brightness
	  if (side == 0)
	  	if (mapX - sdl->player->posX > 0)
		  	color = color / 4;
	if (side == 1)
	  	if (mapY - sdl->player->posY > 0)
		  	color = color / 5;
		if(side == 1) {color = color / 2;}

      //draw the pixels of the stripe as a vertical line
      //verLine(x, drawStart, drawEnd, color);
	  draw_vertical_line(sdl, x, (int[2]){drawStart, drawEnd}, color);
    }
}

t_texture	*create_texture(void)
{
	t_texture *texture;

	if (!(texture = (t_texture*)ft_memalloc(sizeof(t_texture))))
		handle_error("Malloc failed.");
	texture->texture = NULL;
	texture->width = 0;
	texture->height = 0;
	texture->next = NULL;
	return (texture);
}

void	texture_load_from_file(t_sdl *sdl, t_texture **texture, char *path)
{
	SDL_Surface *loaded_surface;
	SDL_Texture	*new_texture;

	*texture = create_texture();
	if (!(loaded_surface = IMG_Load(path)))
		handle_error_sdl("Unable to load image!");
	if (!(new_texture = SDL_CreateTextureFromSurface(sdl->renderer, loaded_surface)))
		handle_error_sdl("Unable to create texture from surface!");
	(*texture)->texture = new_texture;
	(*texture)->width = SCREEN_WIDTH;
	(*texture)->height = SCREEN_HEIGHT;
	SDL_FreeSurface(loaded_surface);
}

void	render_texture(t_sdl *sdl, t_texture *texture, int x, int y)
{
	SDL_Rect render_quad;

	render_quad = (SDL_Rect){x, y, texture->width, texture->height};
	SDL_RenderCopy(sdl->renderer, texture->texture, NULL, &render_quad);
}

int		main(int argc, char **argv)
{
	t_sdl *sdl;
	double moveSpeed = 0.1;
	double rotSpeed = 0.05;
	int up = 0;
	int down = 0;
	int right = 0;
	int left = 0;

	sdl = init();
	handle_arguments(sdl, argc, argv);
	sdl->player = init_player(sdl->map);
	print_map(sdl->map);
	texture_load_from_file(sdl, &sdl->textures, "alex.jpeg");
	while (1)
	{
		if (SDL_PollEvent(&sdl->e))
		{
			if (sdl->e.type == SDL_QUIT)
				close_sdl(sdl);
			else if (sdl->e.type == SDL_KEYDOWN)
			{
				if (sdl->e.key.keysym.sym == SDLK_ESCAPE)
					break ;
				if (sdl->e.key.keysym.sym == SDLK_RIGHT)
					right = 1;
				if (sdl->e.key.keysym.sym == SDLK_LEFT)
					left = 1;
				if (sdl->e.key.keysym.sym == SDLK_UP)
					up = 1;
				if (sdl->e.key.keysym.sym == SDLK_DOWN)
					down = 1;
			}
			else if (sdl->e.type == SDL_KEYUP)
			{
				if (sdl->e.key.keysym.sym == SDLK_RIGHT)
					right = 0;
				if (sdl->e.key.keysym.sym == SDLK_LEFT)
					left = 0;
				if (sdl->e.key.keysym.sym == SDLK_UP)
					up = 0;
				if (sdl->e.key.keysym.sym == SDLK_DOWN)
					down = 0;
			}
		}
		if (right)
		{
			double oldDirX = sdl->player->dirX;
			sdl->player->dirX = sdl->player->dirX * cos(-rotSpeed) - sdl->player->dirY * sin(-rotSpeed);
			sdl->player->dirY = oldDirX * sin(-rotSpeed) + sdl->player->dirY * cos(-rotSpeed);
			double oldPlaneX = sdl->player->planeX;
			sdl->player->planeX = sdl->player->planeX * cos(-rotSpeed) - sdl->player->planeY * sin(-rotSpeed);
			sdl->player->planeY = oldPlaneX * sin(-rotSpeed) + sdl->player->planeY * cos(-rotSpeed);
		}
		if (left)
		{
			double oldDirX = sdl->player->dirX;
			sdl->player->dirX = sdl->player->dirX * cos(rotSpeed) - sdl->player->dirY * sin(rotSpeed);
			sdl->player->dirY = oldDirX * sin(rotSpeed) + sdl->player->dirY * cos(rotSpeed);
			double oldPlaneX = sdl->player->planeX;
			sdl->player->planeX = sdl->player->planeX * cos(rotSpeed) - sdl->player->planeY * sin(rotSpeed);
			sdl->player->planeY = oldPlaneX * sin(rotSpeed) + sdl->player->planeY * cos(rotSpeed);
		}
		if (up)
		{
			if (sdl->map->map[(int)(sdl->player->posX + sdl->player->dirX * moveSpeed)][(int)(sdl->player->posY)] == 0)
				sdl->player->posX += sdl->player->dirX * moveSpeed;
			if (sdl->map->map[(int)(sdl->player->posX)][(int)(sdl->player->posY + sdl->player->dirY * moveSpeed)] == 0)
				sdl->player->posY += sdl->player->dirY * moveSpeed;
		}
		if (down)
		{
			if (sdl->map->map[(int)(sdl->player->posX - sdl->player->dirX * moveSpeed)][(int)(sdl->player->posY)] == 0)
				sdl->player->posX -= sdl->player->dirX * moveSpeed;
			if (sdl->map->map[(int)(sdl->player->posX)][(int)(sdl->player->posY - sdl->player->dirY * moveSpeed)] == 0)
				sdl->player->posY -= sdl->player->dirY * moveSpeed;
		}
		draw_map(sdl);
		SDL_SetRenderDrawColor(sdl->renderer, 0x77, 0x77, 0x77, 0xFF);
		SDL_RenderClear(sdl->renderer);
		render_texture(sdl, sdl->textures, 0, 0);
		render_texture(sdl, sdl->textures, 0, SCREEN_HEIGHT / 2);
		draw_map(sdl);
		SDL_RenderPresent(sdl->renderer);
	}
	close_sdl(sdl);
}
