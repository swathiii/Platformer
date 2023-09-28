#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h" 

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 750;
int DISPLAY_SCALE = 1;

const Vector2D HERO_DEFAULT_VELOCITY(0.f, 1.f); 

const Vector2D GROUND_AABB{ 750.0f, 40.f }; 
const Vector2D PLATFORM_AABB{ 120.0f, 30.0f };  
const Vector2D HERO_AABB{ 10.0f, 20.0f };


enum HeroState
{
	STATE_IDLE, 
	STATE_FALL,
	STATE_PLAY,  ///////// 
	STATE_JUMP,
	STATE_LAND,  
	STATE_WALK,
	STATE_CLIMB,
	STATE_DEAD,
};

enum GameObjectType
{
	TYPE_HERO,
	TYPE_PLATFORM,
	TYPE_GROUND,

};

struct GameState
{
	
	int Gcollision = 0; 
	bool floorcollision = false;
	bool platcollision = false; 
	bool SpriteFaceLeft = false; 

	const Vector2D gravity = { 0, -5 };
	const Vector2D moveforward = { 1, 0 }; 
	const Vector2D thrust = { 0, 3 };

	
	HeroState herostate = STATE_IDLE; 
};

GameState gamestate; 

//declarations
void platforms(); 
void stats(); 

void UpdateHero();
void UpdateControls();  
void Draw(); 
void groundcollision(); 
void UpdateCamera(); 
void Collision(); 

void Jump(); 
void fall();  
void Walk(); 

void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background4.png"); 

	Play::CreateGameObject(TYPE_HERO, { DISPLAY_WIDTH / 2, 100 }, 10, "Pink_Monster"); 

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
	UpdateCamera();  

	UpdateHero();

	//UpdateControls(); 

	Draw(); 

	Collision();  

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
	//Play::SetDrawingSpace(Play::SCREEN); 
	Play::DrawObject(Play::GetGameObjectByType(TYPE_GROUND)); 
	Play::CentreSpriteOrigin("Ground"); 
		//aabb
	GameObject& obj_ground = (Play::GetGameObjectByType(TYPE_GROUND));
	Play::DrawRect(obj_ground.pos - GROUND_AABB, obj_ground.pos + GROUND_AABB, Play::cGreen); 
	//Play::SetDrawingSpace(Play::WORLD); 


	//draw obi
	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_HERO)); 
	Play::CentreMatchingSpriteOrigins("Pink_Monster");
		//aabb
	GameObject& obj_hero = (Play::GetGameObjectByType(TYPE_HERO));
	Play::DrawRect(obj_hero.pos - HERO_AABB, obj_hero.pos + HERO_AABB, Play::cGreen);     

	platforms(); 

	stats(); 

	Play::SetDrawingSpace(Play::SCREEN);
	Play::DrawDebugText({  DISPLAY_WIDTH / 2, 100 }, "hello", Play::cWhite); 
	Play::SetDrawingSpace(Play::WORLD);

	Play::PresentDrawingBuffer();  
}

void platforms()
{	
	Play::CentreSpriteOrigin("Platform");

	//draw platforms
	for (int i : Play::CollectGameObjectIDsByType(TYPE_PLATFORM))  
	{
		Play::DrawObject(Play::GetGameObject(i));
	}
	//aabb
	for (int i : Play::CollectGameObjectIDsByType(TYPE_PLATFORM))
	{
		Play::DrawRect(Play::GetGameObject(i).pos - PLATFORM_AABB, Play::GetGameObject(i).pos + PLATFORM_AABB, Play::cGreen);
	}
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
			gamestate.herostate = STATE_FALL;  
		}
		break; 

	case STATE_FALL:
		fall(); 

		if (gamestate.floorcollision || gamestate.platcollision)
		{
			gamestate.herostate = STATE_WALK;
		}
		break; 

	case STATE_WALK:
		Walk(); 
		if (Play::KeyDown(VK_SPACE))
		{
			gamestate.herostate = STATE_JUMP; 
		}
		break;  

	case STATE_JUMP:
		Jump();
		if (gamestate.floorcollision || gamestate.platcollision)
		{
			gamestate.herostate = STATE_WALK;
		}
		break;  

	case STATE_LAND:    
		Collision(); 
		groundcollision(); 
		break; 

	//case STATE_PLAY:
	//	UpdateControls(); 
	//	//if (obj_hero.pos.y > Ymin_surface) 
	//	//{
	//	//	gamestate.herostate = STATE_LAND;  
	//	//}
	//	break;  

	}

}


void UpdateControls()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	
	//setting default velocity for the hero
	obj_hero.velocity = { 0, 1.f };

	//acceleration
	obj_hero.acceleration = Vector2D(0.f, 0.1f); 

	obj_hero.pos += obj_hero.velocity; 
	obj_hero.velocity += obj_hero.acceleration;  

	if (Play::KeyPressed(VK_SPACE))
	{
		Jump();
	}
	else if (Play::KeyDown(VK_RIGHT) )
	{
		Walk();  
	}
	else if (Play::KeyDown(VK_LEFT)) 
	{ 
		Walk();  
	}
	else if (Play::KeyDown(VK_DOWN))
	{
		obj_hero.velocity = { 0, 7 }; 
	}
	else if (Play::KeyDown(VK_F1))
	{
		obj_hero.pos = Vector2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 3);
		//obj_agent.velocity = AGENT_VELOCITY_DEFAULT;
	}
	else 
	{
		if (!gamestate.SpriteFaceLeft)
		{
			Play::SetSprite(obj_hero, "Pink_Monster_Idle_4", 0.05f);

		}

		if (gamestate.SpriteFaceLeft)
		{
			Play::SetSprite(obj_hero, "Pink_Monster_Idle_Left_4", 0.05f);
		}
	}
	Play::UpdateGameObject(obj_hero); 
}

void fall()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	obj_hero.velocity = { 0, 3.98f };

	//acceleration
	obj_hero.acceleration = Vector2D(0.f, 0.1f);

	obj_hero.pos += obj_hero.velocity;
	obj_hero.velocity += obj_hero.acceleration;
} 

void Walk()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO); 
	 
	 if (Play::KeyDown(VK_RIGHT) )
	 {
		gamestate.SpriteFaceLeft = false;
		obj_hero.velocity = { 3, 0 }; 
		obj_hero.pos += obj_hero.velocity; 
		Play::SetSprite(obj_hero, "Pink_Monster_Walk_Right_6", 0.10f);  
	 }
	else if (Play::KeyDown(VK_LEFT)) 
	{ 
		gamestate.SpriteFaceLeft = true; 
		obj_hero.velocity = { -3, 0 };
		obj_hero.pos += obj_hero.velocity; 
		Play::SetSprite(obj_hero, "Pink_Monster_Walk_Left_6", 0.10f); 
	}
	else
	 {
		 if (!gamestate.SpriteFaceLeft)
		 {
			 Play::SetSprite(obj_hero, "Pink_Monster_Idle_4", 0.05f);

		 }

		 if (gamestate.SpriteFaceLeft)
		 {
			 Play::SetSprite(obj_hero, "Pink_Monster_Idle_Left_4", 0.05f);
		 }
	 }

}

void Jump()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);


	//obj_hero.velocity += obj_hero.acceleration;
	if (Play::KeyDown(VK_SPACE))
	{
		obj_hero.velocity += gamestate.thrust + gamestate.moveforward +  gamestate.gravity;  
		obj_hero.pos += obj_hero.velocity; 
	}

	Play::SetSprite(obj_hero, "Pink_Monster_Jump_8", 0.05f);
}

void Collision()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO); 
	std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	gamestate.platcollision = false;

	gamestate.floorcollision = false; 

	//point vs aabb collision
	for (int id_platform : vPlatforms)
	{
		GameObject& obj_platform = Play::GetGameObject(id_platform);

		if (obj_hero.pos.y + HERO_AABB.y > obj_platform.pos.y - PLATFORM_AABB.y
			&& obj_hero.pos.y - HERO_AABB.y < obj_platform.pos.y + PLATFORM_AABB.y)
		{
			if (obj_hero.pos.x + HERO_AABB.x > obj_platform.pos.x - PLATFORM_AABB.x
				&& obj_hero.pos.x - HERO_AABB.x < obj_platform.pos.x + PLATFORM_AABB.x)
			{
				gamestate.Gcollision += 1;
				gamestate.platcollision = true;
				gamestate.platcollision += 1;

				gamestate.floorcollision = true;
				gamestate.floorcollision += 1;

				obj_hero.velocity = { 0, 0 };
				obj_hero.pos.y = (obj_platform.pos.y - PLATFORM_AABB.y - 20);   
				
			}
		}

	}
}

void groundcollision()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	GameObject& obj_ground = Play::GetGameObjectByType(TYPE_GROUND);
	gamestate.floorcollision = false;

	if (obj_hero.pos.y + HERO_AABB.y > obj_ground.pos.y - GROUND_AABB.y
		&& obj_hero.pos.y - HERO_AABB.y < obj_ground.pos.y + GROUND_AABB.y)
	{
		gamestate.floorcollision = true;
		gamestate.floorcollision += 1;

		obj_hero.velocity = { 0, 0 };
		obj_hero.pos.y = (obj_ground.pos.y - PLATFORM_AABB.y - 30);
	}

	Play::UpdateGameObject(obj_hero); 
}

void UpdateCamera()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);  

	if (obj_hero.pos.y > 450)
	{
		Point2f cam_pos = Play::GetCameraPosition();
		Play::SetCameraPosition(cam_pos);  
	}
	else
	{
		Play::SetCameraPosition((obj_hero.pos - Vector2f(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2))); 
	}
}

void stats()
{
	Play::DrawFontText("64px", "Collision: " + std::to_string(gamestate.Gcollision), Point2D(50, 600), Play::LEFT); 

	if (gamestate.floorcollision)
	{
		Play::DrawFontText("64px", "landing: " + std::to_string(gamestate.floorcollision), Point2D(50, 550), Play::LEFT);
	}
	else
	{
		Play::DrawFontText("64px", "landing: " + std::to_string(gamestate.floorcollision), Point2D(50, 550), Play::LEFT);
	}


	if (gamestate.floorcollision)
	{
		Play::DrawFontText("64px", "Planding: " + std::to_string(gamestate.platcollision), Point2D(50, 500), Play::LEFT);
	}
	else
	{
		Play::DrawFontText("64px", "Planding: " + std::to_string(gamestate.platcollision), Point2D(50, 500), Play::LEFT);
	}
}