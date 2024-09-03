#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>
#include <print>
#include <random>
#include <ranges>
#include <thread>
#include <chrono>
#include <irrKlang.h>

#include "stb_image.h"
#include "Shader.hpp"
#include "Timer.hpp"

class Tetris
{
public: 
	//Window settings
	GLFWwindow* window;
	static constexpr const char* TITLE = "Tetris";
	static constexpr glm::vec2 SCALE = glm::vec2(2.f);
	static constexpr GLsizei WIDTH = (GLsizei)(320 * SCALE.x);
	static constexpr GLsizei HEIGHT = (GLsizei)(480 * SCALE.y);
	static constexpr GLuint GRID_NUMBER_J = 10;
	static constexpr GLuint GRID_NUMBER_I = 20; 
	
private:
	//Enumerations
	enum Textures
	{
		TILES,
		FRAME,
		BACKGROUND
	};

	enum TileColor : GLuint
	{
		NONE,
		PURPLE,
		RED,
		GREEN,
		YELLOW,
		SKY,
		ORANGE,
		BLUE,
		COLORS_END
	};

	enum SoundType
	{
		SOUNDTRACK,
		LINE_CLEAR,
		GAME_OVER
	};

	//Describes tile settings at field
	struct Tile{
		TileColor color;
		GLfloat transparency;
	};

	//Displays information about the next tetraminos
	struct TetraminoPrototype {
		TileColor color = (TileColor)((rd() % (TileColor::COLORS_END - 1)) + 1);
		std::array<GLuint, 4> shape = (Tetramino::SHAPES[rd() % Tetramino::SHAPES.size()]);
	};

	using Field = std::array<std::array<Tile, GRID_NUMBER_J>, GRID_NUMBER_I>;
	struct Tetramino
	{
		static constexpr std::array<std::array<GLuint, 4>, 7> SHAPES{ {
				{ 0, 2, 4, 6 },		//I
				{ 0, 2, 4, 5 },		//L
				{ 1, 3, 4, 5 },		//J
				{ 1, 2, 3, 5 },		//T
				{ 0, 1, 2, 3 },		//O
				{ 1, 2, 3, 4 },		//S
				{ 0, 2, 3, 5 } 		//Z
			} };
		static constexpr GLfloat TILE_SIDE = 18;

		enum class MovingType
		{
			LEFT,
			RIGHT,
			FAST,
			SLOW,
			ROTATE,
			FALL
		};

		enum SoundType {
			FALL,
			ROTATE,
			MOVE
		};

		struct Pos {
			GLint i;
			GLint j;

			bool operator==(Pos const& other) const = default;
		};//Represent position as 'i', 'j' indices
		
		//Properties
		Tetris::Field& field;
		TileColor color;
		std::array<Pos, 4> tiles_pos;
		std::array<Pos, 4> shadow;
		GLfloat delay;
		Timer fallTimer;
		std::unique_ptr<Shader> shad;
		bool isPlaced;

		//Sound
		std::vector<irrklang::ISoundSource*> sounds;
		irrklang::ISoundEngine* sEngine{ irrklang::createIrrKlangDevice() };

		//Inititalization member functions
		void init_sounds();
		void init_shader();
		explicit Tetramino(Field& fd, TetraminoPrototype const&, GLfloat delay = 0.7f);
		~Tetramino();
		void update(TetraminoPrototype const&, GLfloat delay);

		//Moving
		void process_input(MovingType);
		void advancePos(Pos const&);
		bool move(Pos const&);
		void moveLeft();
		void moveRight();
		void moveDown();
		void rotate();
		void fall();
		void fast();
		void slow();
		void updateShadow();

		//Drawing
		void drawTile(Tile const&, glm::vec2 const& pos) const;
		void removeFromField() const;
		bool addToField() const;

		//Help const functions
		void fillWith(TileColor col) const;
		bool tileIsAllowed(Pos const&) const;
		bool canMoveTowards(Pos const&) const;
		std::array<Pos, 4> getBottom() const;
	};

	//Engine objects
	Field field{ NONE };
	Tetramino tetramino{ field, TetraminoPrototype{} };
	std::vector<TetraminoPrototype> next_tetraminos;
	Timer deltaTime{};
	bool isGame = true;
	static inline std::mt19937 rd{ std::random_device{}() };

	//Sounds
	irrklang::ISoundEngine* sEngine{ irrklang::createIrrKlangDevice() };
	std::vector<irrklang::ISoundSource*> sounds;

	//OpenGL data
	std::unique_ptr<Shader> shad;

	//Static functions
	static void keyboard_callback(GLFWwindow*, int key, int scancode, int action, int mods);
	static glm::vec3 convert(glm::vec3 const& vec);

	//Initialization member functions
	GLFWimage load_icon() const;
	void init_window();
	void init_buffers() const;
	void init_textures() const;
	void init_shader();
	void init_sounds();
	void init_tetraminos();
	
	//Drawable member functions
	void drawBackground() const;
	void drawFrame() const;
	void drawField() const;
	void drawNextTetraminos() const;
	void render() const;

	//Core member functions
	void updateTetramino();
	void updateNextTetraminos();
	void clearLines();
	void playSound(SoundType);
	bool checkForGameOver();

public:
	Tetris();
	~Tetris();
	void game();
};
inline Tetris tetris{};