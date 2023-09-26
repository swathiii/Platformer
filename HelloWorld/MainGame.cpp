#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h" 

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 750;
int DISPLAY_SCALE = 1;

const Vector2D HERO_DEFAULT_VELOCITY(5.0f, -1.f); 

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
	TYPE_PLATFORM,
	TYPE_GROUND,

};

void UpdateHero();
void UpdateControls();  
void Draw(); 
void groundcollision(); 

void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background4.png"); 

	Play::CreateGameObject(TYPE_HERO, { DISPLAY_WIDTH / 2, 500 }, 10, "Pink_Monster"); 
	Play::MoveSpriteOrigin("Pink_Monster", 16, 32); 
	//creating the ground
	Play::CreateGameObject(TYPE_GROUND, { 0, DISPLAY_HEIGHT - 50 }, 20, "Ground");  

	//creating platforms
	const int spacing{ 500 }; 
	for (int i = 0; i < 5; i++)
	{
		Play::CreateGameObject(TYPE_PLATFORM, { spacing * i, DISPLAY_HEIGHT - 400 }, 20, "Platform");
	}

	//setting default velocity for the hero
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	obj_hero.velocity = HERO_DEFAULT_VELOCITY;

	//acceleration
	obj_hero.acceleration = Vector2D(0.f, 0.50f);

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
	
	for (int i : Play::CollectGameObjectIDsByType(TYPE_PLATFORM))
	{
		Play::DrawObject(Play::GetGameObject(i)); 
	}

	Play::DrawObject(Play::GetGameObjectByType(TYPE_GROUND)); 

	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_HERO)); 

	Play::PresentDrawingBuffer();  
}

void UpdateHero()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	obj_hero.scale = 1.5f; 
	
	obj_hero.pos += obj_hero.velocity; 
	obj_hero.velocity += obj_hero.acceleration; 


	switch (gamestate.herostate)
	{
	case STATE_IDLE:
		obj_hero.velocity = { 0, 0 };
		Play::SetSprite(obj_hero, "Pink_Monster", 0.05f);
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
	obj_hero.velocity = { 0,0 }; 
	Play::SetSprite(obj_hero, "Pink_Monster", 0.05f);
	 
	if (Play::KeyDown(VK_RIGHT))
	{
		obj_hero.velocity = { 7, 0 }; 
		Play::SetSprite(obj_hero, "Pink_Monster", 0.12f); 
	}
	if (Play::KeyDown(VK_LEFT)) 
	{ 
		obj_hero.velocity = { -7, 0 }; 
		Play::SetSprite(obj_hero, "Pink_Monster", 0.7f);

	}
	if (Play::KeyDown(VK_DOWN))
	{
		obj_hero.velocity = { 0, 7 }; 
	}
	if (Play::KeyDown(VK_SPACE))
	{
		obj_hero.velocity = { 2, -4 };
		Play::SetSprite(obj_hero, "Pink_Monster", 0.7f); 
	}
	
	Play::UpdateGameObject(obj_hero); 
}

void groundcollision()
{

}