#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h" 

int DISPLAY_WIDTH = 1400;
int DISPLAY_HEIGHT = 761;
int DISPLAY_SCALE = 1;

enum HeroState
{
	STATE_IDLE, 
	STATE_PLAY,
	STATE_JUMP,
	STATE_RUN,
	STATE_FLY,
	STATE_WALK,
	STATE_DEAD,
};

struct GameState
{
	HeroState herostate = STATE_IDLE; 
};

GameState gamestate; 

enum GameObjectType
{
	TYPE_HERO,

};

void UpdateHero();
void UpdateControls();  
void Draw(); 

void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\sky_background_mountains.png"); 

	Play::CreateGameObject(TYPE_HERO, { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 2 }, 10, "standing_idle_15"); 

}

bool MainGameUpdate(float elapsedTime)
{
	UpdateHero();
	UpdateControls();  
	Draw(); 

	return Play::KeyDown(VK_ESCAPE); 

}

int MainGameExit(void)
{
	Play::DestroyManager(); 
	return PLAY_OK; //indicated program ginished without any errors
}

void Draw()
{
	Play::ClearDrawingBuffer(Play::cWhite);

	Play::DrawBackground();

	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_HERO)); 


	Play::PresentDrawingBuffer();  
}

void UpdateHero()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	obj_hero.scale = 0.25f; 

	switch (gamestate.herostate)
	{
	case STATE_IDLE:
		obj_hero.velocity = { 0, 0 };
		Play::SetSprite(obj_hero, "standing_idle", 0.05f);
		Play::PresentDrawingBuffer();
		if (Play::KeyDown(VK_RETURN))
		{
			gamestate.herostate = STATE_PLAY; 
		}
		break; 

	case STATE_PLAY:
		UpdateControls();
		break;

	}
}

void UpdateControls()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	Play::SetSprite(obj_hero, "standing_idle", 0.05f);
	 
	if (Play::KeyDown(VK_RIGHT))
	{
		obj_hero.pos.x += 7;
		Play::SetSprite(obj_hero, "hero_walking_5x3", 0.7f);
	}
	if (Play::KeyDown(VK_LEFT)) 
	{ 
		obj_hero.pos.x -=  7; 
		Play::SetSprite(obj_hero, "hero_walking_left_5x3", 0.7f);

	}


	Play::UpdateGameObject(obj_hero); 
}
