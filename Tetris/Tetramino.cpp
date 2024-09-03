#include "Tetris.h"

void Tetris::Tetramino::init_sounds()
{
	std::vector<const char*> sounds_paths{
		"resources/fall.wav",
		"resources/rotate.wav",
		"resources/move.wav" };

	std::ranges::for_each(sounds_paths, [&](const char*& path) {
		sounds.push_back(sEngine->addSoundSourceFromFile(path));
		}
	);

	sounds[ROTATE]->setDefaultVolume(0.15f);
	sounds[FALL]->setDefaultVolume(0.15f);
}

void Tetris::Tetramino::init_shader()
{
	this->shad = std::make_unique<Shader>("tetris_shad.vert", "tetramino_shad.frag");
}

Tetris::Tetramino::Tetramino(Field& fd, TetraminoPrototype const& prot, GLfloat dl)
	: field(fd)
{
	this->init_sounds();
	this->update(prot, dl);
}

Tetris::Tetramino::~Tetramino()
{
	this->sEngine->drop();
}

void Tetris::Tetramino::update(TetraminoPrototype const& shell, GLfloat dl)
{
	color = shell.color;
	this->delay = dl;
	isPlaced = false;

	//Init shape coordinates
	auto const& shape = shell.shape;
	for (uint16_t i = 0; i < 4; ++i)
	{
		tiles_pos[i].i = shape[i] / 2;
		tiles_pos[i].j = 4 + shape[i] % 2;
	}

	//Check if the tetramino can be added to the field
	if (!this->canMoveTowards({ 0,0 }))
	{
		//If cannot then lift up the one by 1
		this->advancePos({ -1, 0 });
	}

	shadow = getBottom();
}

void Tetris::Tetramino::drawTile(Tile const& tile, glm::vec2 const& position) const
{
	shad->use();

	glm::mat4 model(1.f);
	model = glm::translate(model, convert(glm::vec3(position + TILE_SIDE / 2.f, 0.f)));
	model = glm::scale(model, glm::vec3(SCALE * glm::vec2(TILE_SIDE * 2 / WIDTH, TILE_SIDE * 2 / HEIGHT), 1.f));
	shad->setUniform("model", model);
	shad->setUniform("tileColor", tile.color);
	shad->setUniform("texture1", TILES);
	shad->setUniform("transparency", tile.transparency);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Tetris::Tetramino::process_input(MovingType type)
{
	using enum MovingType;
	switch (type)
	{
	case FAST:
		this->fast();
		break;

	case SLOW:
		this->slow();
		break;

	case LEFT:
		this->moveLeft();
		break;

	case RIGHT:
		this->moveRight();
		break;

	case ROTATE:
		this->rotate();
		break;

	case FALL:
		this->fall();
		break;
	}
}

void Tetris::Tetramino::advancePos(Pos const& direction)
{
	std::ranges::for_each(tiles_pos, [&](Pos& pos) {
		pos.i += direction.i;
		pos.j += direction.j;
		}
	);
}

bool Tetris::Tetramino::move(Pos const& direction)
{
	if (canMoveTowards(direction))
	{
		advancePos(direction);
		return true;
	}
	return false;
}

void Tetris::Tetramino::moveLeft()
{
	if (this->move({ 0, -1 }))
	{
		updateShadow();
		sEngine->play2D(sounds[MOVE]);
	}
}

void Tetris::Tetramino::moveRight()
{
	if (this->move({ 0, 1 }))
	{
		updateShadow();
		sEngine->play2D(sounds[MOVE]);
	}
}

void Tetris::Tetramino::moveDown()
{
	fallTimer.stop();
	if(fallTimer.getElapsedTime() > delay)
	{
		fallTimer.start();
		if (!move({ 1, 0 })) //If cannot move down further
		{
			//Set flag that tetramino is placed
			isPlaced = true;
		}
	}
}

void Tetris::Tetramino::rotate()
{
	//Temp tetramino's tiles coordinates
	auto t_tile_coords = tiles_pos;

	//Center of rotation
	Pos const& p = tiles_pos[1];
	for (auto& tile : t_tile_coords) {
		auto tile_i = tile.i;

		tile.i = p.i - (tile.j - p.j);
		tile.j = p.j + (tile_i - p.i);
		if (!tileIsAllowed(tile))
			return;
	}

	tiles_pos = t_tile_coords;
	updateShadow();
	sEngine->play2D(sounds[ROTATE]);
}

void Tetris::Tetramino::fall()
{
	//Update the position to the bottom one
	this->tiles_pos = this->shadow;
	isPlaced = true;

	//Play fall sound
	sEngine->play2D(sounds[FALL]);

	//Reset timer
	fallTimer.start();
}

void Tetris::Tetramino::fast()
{
	this->delay = 0.05f;
}

void Tetris::Tetramino::slow()
{
	this->delay = 0.7f;
}

void Tetris::Tetramino::updateShadow()
{
	this->shadow = this->getBottom();
}

void Tetris::Tetramino::removeFromField() const
{
	this->fillWith(NONE);
}

//Return false if there was an error with adding tetramino at the field
bool Tetris::Tetramino::addToField() const
{
	//Add only if there is a place for it
	if (!canMoveTowards({ 0,0 }))
		return false ;

	this->fillWith(this->color);
	return true;
}

//Fill the tetramino with the given color
void Tetris::Tetramino::fillWith(TileColor col) const
{
	for (uint16_t i = 0; i < 4; ++i)
	{
		auto const& tetr_tile = tiles_pos[i];
		auto const& shadow_tile = shadow[i];

		//Fill the shadow's tiles only when they would not overlap the main ones
		if(!std::ranges::contains(tiles_pos, shadow_tile))
			field[shadow_tile.i][shadow_tile.j] = { col, 0.5f };
		field[tetr_tile.i][tetr_tile.j] = { col, 0.f };
	}
}

bool Tetris::Tetramino::tileIsAllowed(Pos const& pos) const
{
	return pos.j >= 0 && pos.j < GRID_NUMBER_J && pos.i < GRID_NUMBER_I &&
		field[pos.i][pos.j].color == NONE;
}

bool Tetris::Tetramino::canMoveTowards(Pos const& direction) const
{
	return std::ranges::all_of(tiles_pos, [&](Pos const& pos) {
		return tileIsAllowed({ pos.i + direction.i, pos.j + direction.j });
		}
	);
}

std::array<Tetris::Tetramino::Pos, 4> Tetris::Tetramino::getBottom() const
{
	auto copy_tiles_pos = tiles_pos;

	auto canMoveDown = [&]() {
		return std::ranges::all_of(copy_tiles_pos, [&](Pos const& pos) {
			return pos.i + 1 < GRID_NUMBER_I && field[pos.i + 1LL][pos.j].color == NONE;
			});
		};
	while (canMoveDown())
		std::ranges::for_each(copy_tiles_pos, [](Pos& pos) { ++pos.i; });

	return copy_tiles_pos;
}
