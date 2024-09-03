#include "Tetris.h"

///////////////// Static members initialization ///////////////////////
void Tetris::keyboard_callback(GLFWwindow*, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		tetris.isGame = false;
		glfwSetWindowShouldClose(tetris.window, true);
		return;
	}
	if (!tetris.isGame)
		return;

	using enum Tetris::Tetramino::MovingType;

	static std::vector<Tetris::Tetramino::MovingType> moves;
	if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		moves.push_back(LEFT);

	if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		moves.push_back(RIGHT);

	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		moves.push_back(FAST);

	if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
		moves.push_back(SLOW);

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		moves.push_back(ROTATE);

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		moves.push_back(FALL);

	for (auto move : moves)
		tetris.tetramino.process_input(move);
	moves.clear();
}

glm::vec3 Tetris::convert(glm::vec3 const& vec)
{
	glm::vec3 result{};
	// Normalize window coordinates to [0, 1]
	glm::vec2 normalized = glm::vec2(vec.x / WIDTH, vec.y / HEIGHT) * SCALE;

	// Convert to OpenGL coordinates [-1, 1]
	result = glm::vec3(
		normalized.x * 2.f - 1.f,
		1.f - normalized.y * 2.f, // Inver
		0.f);

	return result;
}

///////////////// Private member methods /////////////////////

GLFWimage Tetris::load_icon() const
{
	GLFWimage icon{};
	stbi_set_flip_vertically_on_load(true);
	icon.pixels = stbi_load("resources/Icon.png", &icon.width, &icon.height, nullptr, 4);
	if (!icon.pixels)
	{
		std::cerr << "\n\nICON IMAGE WAS NOT LOADED\n\n";
		return GLFWimage{};
	}

	return icon;
}

//Inits
void Tetris::init_window()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	this->window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyboard_callback);
	GLFWimage icon = load_icon();
	glfwSetWindowIcon(window, 1, &icon);
	stbi_image_free(icon.pixels);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		system("pause");
		exit(1);
	}
}

void Tetris::init_buffers() const
{
	constexpr GLfloat vertices[] =
	{
		//Triangles				//Textures		//Tiles texture
		-0.5f, -0.5f, 0.f,		0.f, 0.f,		0.f, 0.f,
		-0.5f, 0.5f, 0.f,		0.f, 1.f,		0.f, 1.f,
		0.5f, 0.5f, 0.f,		1.f, 1.f,		0.125f, 1.f,

		-0.5f, -0.5f, 0.f,		0.f, 0.f,		0.f, 0.f,
		0.5f, -0.5f, 0.f,		1.f, 0.f,		0.125f, 0.f,
		0.5f, 0.5f, 0.f,		1.f, 1.f,		0.125f, 1.f
	};

	GLuint VBO;
	GLuint VAO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
}

void Tetris::init_textures() const
{
	std::vector<const char*> textures_paths{
		"resources/tiles.png", 
		"resources/frame.png", 
		"resources/background.png" };

	std::vector<GLuint> textures(textures_paths.size());
	glGenTextures((GLsizei)textures_paths.size(), textures.data());

	GLint width;
	GLint height;
	GLint nrChannels;
	stbi_set_flip_vertically_on_load(true);
	for (GLenum i = 0; i < textures_paths.size(); ++i)
	{
		//Enable transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		auto data = stbi_load(textures_paths[i], &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format = GL_RGBA;
			if (nrChannels == 3)
				format = GL_RGB;

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data);
		}
		else std::cerr << "Failed to load texture" << std::endl;
	}
}

void Tetris::init_shader()
{
	this->shad = std::make_unique<Shader>("tetris_shad.vert", "tetris_shad.frag");
}

void Tetris::init_sounds()
{
	if (!sEngine)
	{
		std::cerr << "Failed to initialize sound engine" << std::endl;
		system("pause");
		exit(1);
	}
	
	std::vector<const char*> sounds_paths{
		"resources/soundtrack.mp3",
		"resources/line_clear.wav",
		"resources/end_game.wav" 
	};

	sounds.reserve(sounds_paths.size());
	std::ranges::for_each(sounds_paths, [&](const char*& path) {
		sounds.push_back(sEngine->addSoundSourceFromFile(path));
		}
	);

	sounds[LINE_CLEAR]->setDefaultVolume(0.6f);
	sounds[GAME_OVER]->setDefaultVolume(0.15f);
}

void Tetris::init_tetraminos()
{
	next_tetraminos.reserve(3);

	//Push 3 tetraminos to the vector
	for(size_t i = 0; i < 3; ++i)
		next_tetraminos.push_back(TetraminoPrototype{});
}

//Drawings
void Tetris::drawBackground() const
{
	shad->use();

	glm::mat4 model(1.f);
	model = glm::scale(model, glm::vec3(SCALE, 1.f));

	shad->setUniform("model", model);
	shad->setUniform("texture1", BACKGROUND);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Tetris::drawFrame() const
{
	shad->use();

	glm::mat4 model(1.f);
	model = glm::scale(model, glm::vec3(SCALE, 1.f));

	shad->setUniform("model", model);
	shad->setUniform("texture1", FRAME);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}   

void Tetris::drawField() const
{
	bool tetramino_was_drawn = tetramino.addToField();
	for (GLuint i = 0; i < GRID_NUMBER_I; ++i)
		for (GLuint j = 0; j < GRID_NUMBER_J; ++j)
			if (auto const& tile = this->field[i][j]; tile.color != NONE)
				tetramino.drawTile(tile, { 28 + (GLfloat)j * Tetramino::TILE_SIDE,
					31 + (GLfloat)i * Tetramino::TILE_SIDE });

	//Do not remove anything if tetramino wasn't added to the field
	if(tetramino_was_drawn)
		this->tetramino.removeFromField();
}

void Tetris::drawNextTetraminos() const
{
	std::ranges::for_each(next_tetraminos, [this, i = 1](TetraminoPrototype const& tetr) mutable {
		std::ranges::for_each(tetr.shape, [&](GLuint shape_ind)  {
			tetramino.drawTile({ tetr.color, 0.f }, {
				250.f + Tetramino::TILE_SIDE * (shape_ind % 2),
				90.f * i + Tetramino::TILE_SIDE * (shape_ind / 2) });
			}
		);
		++i;
		}
	);
}

void Tetris::render() const
{
	this->drawBackground();
	this->drawField();
	this->drawFrame();
	this->drawNextTetraminos();
}

void Tetris::updateTetramino()
{
	//Move tetramino down
	tetramino.moveDown();

	//If one is placed and it's not a game over
	if (tetramino.isPlaced && isGame)
	{
		//Leave it at its place
		tetramino.addToField();
		
		//Check lines to clear
		clearLines();

		//And update
		updateNextTetraminos();
	}
	//...
	//Do nothing if either tetramino is not placed or it's a game over
}

//Updates the current tetramino and the next tetraminos shown on the screen
void Tetris::updateNextTetraminos()
{
	tetramino.update(next_tetraminos.front(), tetramino.delay);
	std::move(next_tetraminos.begin() + 1, next_tetraminos.end(), next_tetraminos.begin());
	next_tetraminos.back() = TetraminoPrototype{};
}

void Tetris::clearLines()
{
	//If tetramino is not placed at the field then no need to check for lines to clear
	if (!tetramino.isPlaced)
		return;

	bool line_cleared = false;
	for (auto row = field.begin() + 3; row != field.end(); ++row)
		if (std::ranges::all_of(*row, [](Tile tile) {return tile.color != NONE; }))
		{	
			std::ranges::move(field.begin(), row, field.begin() + 1);
			std::ranges::fill(field.front(), Tile{ NONE, 0.f });
			line_cleared = true;
		}

	if (line_cleared)
	{
		tetramino.updateShadow();
		
		//Stop line-clearing sound if it's currently playing
		if(sEngine->isCurrentlyPlaying(sounds[LINE_CLEAR]))
			sEngine->stopAllSoundsOfSoundSource(sounds[LINE_CLEAR]);
		//And then play it
		playSound(LINE_CLEAR);
	}
}

void Tetris::playSound(SoundType type)
{
	sEngine->play2D(sounds[type]);
}

//Checks whether a tetramino was placed out of the upper bounds of the field
bool Tetris::checkForGameOver()
{
	isGame = std::ranges::all_of(tetramino.tiles_pos, [](Tetramino::Pos const& tile_pos) {
		return tile_pos.i >= 0;
		}
	);

	if(!isGame)
	{
		playSound(GAME_OVER);
		std::cout << "\n\nEND GAME!!!\n\n";
		return true;
	}
	return false;
}

Tetris::Tetris()
{
	this->init_window();
	this->init_buffers();
	this->init_textures();
	this->init_shader();
	this->init_sounds();
	this->init_tetraminos();
	this->tetramino.init_shader();
}

Tetris::~Tetris()
{
	glfwTerminate();
	this->sEngine->drop();
}

void Tetris::game()
{
	sEngine->play2D(sounds[SOUNDTRACK], true);
	while (!glfwWindowShouldClose(window))
	{
		this->render();

		//Stop updating objects if game is over
		if(isGame)
		{
			deltaTime.stop();
			
			this->updateTetramino();
			this->checkForGameOver();

			deltaTime.start();
		}

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
}