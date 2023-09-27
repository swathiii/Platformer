#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h" 

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 750;
int DISPLAY_SCALE = 1;

const Vector2D HERO_DEFAULT_VELOCITY(0.f, 1.f); 

const Vector2D GROUND_AABB{ 1500.0f, 37.f }; 
const Vector2D PLATFORM_AABB{ 40.0f, 40.0f };  

enum HeroState
{
	STATE_IDLE, 
	STATE_PLAY,
	STATE_STAND, 
	STATE_JUMP,
	STATE_RUN,
	STATE_FLY,
	STATE_WALK,
	STATE_DEAD,
};

struct GameState
{
	
	int Gcollision = 0; 
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
	Play::MoveSpriteOrigin("Pink_Monster_Idle_4",16, 32); 
	
	//creating the ground
	Play::CreateGameObject(TYPE_GROUND, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT }, 20, "Ground");

	//creating platforms
	const int spacing{ 500 }; 
	for (int i = 0; i < 5; i++)
	{
		Play::CreateGameObject(TYPE_PLATFORM, { spacing * i, DISPLAY_HEIGHT - 400 }, 20, "Platform");
	}

}

bool MainGameUpdate(float elapsedTime)
{
	UpdateHero();

	UpdateControls(); 

	Draw(); 

	groundcollision(); 

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

	//draw ground
	Play::DrawObject(Play::GetGameObjectByType(TYPE_GROUND)); 
	Play::CentreSpriteOrigin("Ground"); 
	GameObject& obj_ground = (Play::GetGameObjectByType(TYPE_GROUND));
	Play::DrawRect(obj_ground.pos - GROUND_AABB, obj_ground.pos + GROUND_AABB, Play::cGreen); 

	//draw obi
	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_HERO)); 

	Play::DrawFontText("64px", "Collision: " + std::to_string(gamestate.Gcollision), Point2D(50, 600), Play::LEFT);

	Play::PresentDrawingBuffer();  
}

void UpdateHero()
{
	GameObject& obj_ground = Play::GetGameObjectByType(TYPE_GROUND); 
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	obj_hero.scale = 1.5f; 
	float Ymin_surface = obj_ground.pos.y - (GROUND_AABB.y / 2);   

	switch (gamestate.herostate)
	{
	case STATE_IDLE:
		Play::DrawFontText("64px", "PRESS ENTER TO START", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 300 }, Play::CENTRE);
		Play::PresentDrawingBuffer(); 
		Play::SetSprite(obj_hero, "Pink_Monster", 0.05f); 
		obj_hero.pos = { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 };
		obj_hero.velocity = { 0, 0 }; 
		
		if (Play::KeyDown(VK_RETURN))
		{
			gamestate.herostate = STATE_PLAY; 
		}
		break; 

	case STATE_PLAY:

		UpdateControls(); 
		if (obj_hero.pos.y > Ymin_surface) 
		{
			gamestate.herostate = STATE_STAND;  
		}
		break;    

	case STATE_STAND:  
		groundcollision(); 
		break; 

	}
}

void UpdateControls()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	Play::SetSprite(obj_hero, "Pink_Monster_Idle_4", 0.05f);  
	
	//setting default velocity for the hero
	obj_hero.velocity = { 0, 0.33f };

	//acceleration
	obj_hero.acceleration = Vector2D(0.f, 0.01f); 

	obj_hero.pos += obj_hero.velocity; 
	obj_hero.velocity += obj_hero.acceleration;  

	 
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
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	GameObject& obj_ground = Play::GetGameObjectByType(TYPE_GROUND);
	float Ymin_surface = obj_ground.pos.y - (GROUND_AABB.y / 2); 

	if (obj_hero.pos.y > Ymin_surface) 
	{
		obj_hero.velocity = { 0, 0 }; 
		obj_hero.pos.y = (obj_ground.pos.y - (GROUND_AABB.y / 2)) - 0;

	}

	//obj_hero.pos.y = (obj_ground.pos.y - (GROUND_AABB.y / 2)) - 20; 

	Play::UpdateGameObject(obj_hero); 
}