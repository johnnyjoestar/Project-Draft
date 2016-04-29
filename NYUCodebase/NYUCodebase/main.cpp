/*
RYAN GOUDJIL
PROJECT GAME DESIGN
Idea: Procedurally generated level where the tiles are either red or blue, blue being death if the player touches
there's a floor and the ceiling the player is always moving rightward, like "the impossible game"
the player can "flip" which is done by [at the moment] light tap on the f-key
it flips the player gravity
There are enemies that have AI to jump at the player, which can be dodged by jumping

*/

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

//Global Variables
//matrix
Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

//textures
GLuint exampleTexture;
GLuint playerTexture;

//program
ShaderProgram* program;
SDL_Event event;

//update vars
float lastFrameTicks = 0.0f;
float ticks = 0.0f;
float angle = 0.0f;
float elapsed = 0.0f;
bool done = false;


class Tile{
public:
	float x_velocity;
	float y_velocity;
	float x_acceleration;
	float y_acceleration;
	float x_position;
	float y_position;
	float fric;
	float height, width;

	GLuint textureID;

	bool isStatic;

	Tile();
	Tile(GLuint tex, float x, float y) :textureID(tex), x_position(x), y_position(y), height(1.0f), width(1.0f){	}

	void ProcessEvents(){}

	void move(float accel){
		return;
	}

	void Draw(ShaderProgram* program){
		program->setModelMatrix(modelMatrix);
		program->setProjectionMatrix(projectionMatrix);
		program->setViewMatrix(viewMatrix);

		modelMatrix.identity();
		modelMatrix.Translate(x_position, y_position, 0.0f);
		program->setModelMatrix(modelMatrix);
		glBindTexture(GL_TEXTURE_2D, textureID);

		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);

		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}

	void Update(float elapsed){
		return;
	}
};
//Classes
class Entity{
public:
	float x_velocity;
	float y_velocity;
	float x_acceleration;
	float y_acceleration;
	float x_position;
	float y_position;
	float fric;
	float height, width;
	float gravity;
	
	bool isFlipped;
	

	GLuint textureID;
	Entity();
	Entity(GLuint tex) :textureID(tex), x_position(0.0f), y_position(0.0f),
		x_velocity(-.0f), y_velocity(0.0f), x_acceleration(0.0f), y_acceleration(0.0f), fric(6.0f), isFlipped(false), height(1.0f), width(1.0f), gravity(5.0f){};

	void ProcessEvents(){}

	void move(float accel){
		x_acceleration = accel;
	}

	void flip(){
		isFlipped = !isFlipped;
		gravity *= -1;
	}

	void Draw(ShaderProgram* program){
		program->setModelMatrix(modelMatrix);
		program->setProjectionMatrix(projectionMatrix);
		program->setViewMatrix(viewMatrix);

		modelMatrix.identity();
		if (isFlipped) modelMatrix.Scale(1.0f, -1.0f, 0.0f);
		modelMatrix.Translate(x_position,y_position,0.0f);
		program->setModelMatrix(modelMatrix);
		glBindTexture(GL_TEXTURE_2D, textureID);

		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);

		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}

	void Colliding(Tile* other){
		float penetration = 0;
		if (((y_position - (height / 2.0f)) > (other->y_position + (other->height / 2.0f)) ||  // checking the player's bottom colliding 
			(y_position + (height / 2.0f)) < (other->y_position - (other->height / 2.0f)) ||
			(x_position - (width / 2.0f)) > (other->x_position + (other->width / 2.0f)) ||
			(x_position + (width / 2.0f)) < (other->x_position - (other->width / 2.0f)))
			== false){
			penetration = fabs(y_position - other->y_position - (height / 2) - (other->height / 2));
			y_position += penetration + 0.0001f;
		}
		else if (((y_position - (height / 2.0f)) < (other->y_position + (other->height / 2.0f)) ||  // checking the player's top colliding 
			(y_position + (height / 2.0f)) > (other->y_position - (other->height / 2.0f)) ||
			(x_position - (width / 2.0f)) > (other->x_position + (other->width / 2.0f)) ||
			(x_position + (width / 2.0f)) < (other->x_position - (other->width / 2.0f)))
			== false){
			penetration = fabs(y_position - other->y_position - (height / 2) - (other->height / 2));
			y_position -= penetration + 0.0001f;
		}
	}

	void Update(float elapsed){
		x_position += x_velocity * elapsed;
		y_position += y_velocity * elapsed;
		x_velocity += x_acceleration * elapsed;
		y_velocity += (y_acceleration - gravity) * elapsed;
		if (x_velocity > 0.0f) x_velocity -= fric * elapsed;
		else if (x_velocity < 0.0f) x_velocity += fric * elapsed;
	}
};


class Player{};
// Planning

class Level{};

class Enemy{};




//vector<Entity*> entities;
vector<Tile*> tiles;
Entity* player;

//Functions
GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	SDL_FreeSurface(surface);
	return textureID;
}

void SetUp(){
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

		glViewport(0,0,640,360);
		program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
		playerTexture = LoadTexture(RESOURCE_FOLDER"player.png");
		exampleTexture = LoadTexture(RESOURCE_FOLDER"example.png");
		player = new Entity(playerTexture);
		tiles.push_back(new Tile(exampleTexture, -2.0f, -2.0f));
		tiles.push_back(new Tile(exampleTexture, -1.0f, -2.0f));
		tiles.push_back(new Tile(exampleTexture, 0.0f, -2.0f));
		tiles.push_back(new Tile(exampleTexture, 1.0f, -2.0f));
		tiles.push_back(new Tile(exampleTexture, -2.0f, 2.0f));
		tiles.push_back(new Tile(exampleTexture, -1.0f, 2.0f));
		tiles.push_back(new Tile(exampleTexture, 0.0f, 2.0f));
		tiles.push_back(new Tile(exampleTexture, 1.0f, 2.0f));

		
		projectionMatrix.setOrthoProjection(-5.3333f, 5.3333f, -3.0f, 3.0f, -1.0f, 1.0f);
		glUseProgram(program->programID);
		

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		
}

void Update(float elasped){
	player->Update(elapsed);
	for (int i = 0; i < tiles.size(); i++) player->Colliding(tiles[i]);

}

void ProcessEvents(){
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN){
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
				angle = 0.0;
			}
		}
	}

	ticks = (float)SDL_GetTicks() / 1000.0F;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	for (int i = 0; i < tiles.size(); i++) player->Colliding(tiles[i]);

	if (keys[SDL_SCANCODE_LEFT]){
		player->move(-30.0f);
	}
	else if (keys[SDL_SCANCODE_RIGHT]){
		player->move(30.0f);
	}
	else if (keys[SDL_SCANCODE_SPACE]){
		player->y_velocity = 5;
	}
	else if (keys[SDL_SCANCODE_F]){
		player->flip();
	}
	else player->move(0);
	Update(elapsed);
}
	
void Render(ShaderProgram* program){
	for (size_t i = 0; i < tiles.size(); i++) tiles[i]->Draw(program);
	player->Draw(program);
	SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[])
{	
	SetUp();
	while (!done) {
		ProcessEvents();
		Render(program);	
	}
	SDL_Quit();
	return 0;
}
