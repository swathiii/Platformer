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
const Vector2D BLOCK_AABB{ 27.f, 27.f };


enum HeroState
{
	STATE_IDLE,
	STATE_FALL,
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
	TYPE_BLOCK,   
	TYPE_GROUND,
	TYPE_COIN,
};

struct GameState
{

	int Gcollision = 0;
	bool floorcollision = false;
	bool platcollision = false;
	bool blockcollision = false;
	bool SpriteFaceLeft = false;
	bool SpriteStanding = false;

	const Vector2D gravity = { 0, 2.5f };
	const Vector2D jumpright = { 0.5f, 0 };
	const Vector2D jumpleft = { -0.5f, 0 };
	const Vector2D thrust = { 0, -3 };

	Vector2D camera_focus{ 0, 0 };
	int camera_cord_x = 0;
	int camera_cord_y = 0;

	HeroState herostate = STATE_IDLE;
};

GameState gamestate;

struct ObjectState
{
	int maxcoins = 6;
	Point2f coinpos{ -100, -100 };
	int CoinsCollected = 0; 
};

ObjectState objectstate;

//declarations
void map(); 
void blocks(); 
void createblocks(int posx, int posy); 

void platforms();

void createcoins(int posx, int posy, int count);   
void stats();

void UpdateHero();
void UpdateControls();
void Draw();
void groundcollision();
void UpdateCamera();
void Collision();
void BlockCollision(); 

void Jump();
void fall();
void Walk();
void coins();
void CollectCoins(); 

void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background4.png");

	Play::CreateGameObject(TYPE_HERO, { DISPLAY_WIDTH / 2, 100 }, 10, "Pink_Monster");

	map(); 
}

bool MainGameUpdate(float elapsedTime)
{
	UpdateCamera();

	UpdateHero();

	//UpdateControls(); 

	Draw();

	Collision();

	groundcollision();

	BlockCollision();

	CollectCoins(); 

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
	Play::CentreSpriteOrigin("Ground2");
	//aabb
	//GameObject& obj_ground = (Play::GetGameObjectByType(TYPE_GROUND));
	//Play::DrawRect(obj_ground.pos - GROUND_AABB, obj_ground.pos + GROUND_AABB, Play::cGreen); 
	//Play::SetDrawingSpace(Play::WORLD); 

	//draw obi
	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_HERO));
	Play::CentreMatchingSpriteOrigins("Pink_Monster");
	//aabb
	//GameObject& obj_hero = (Play::GetGameObjectByType(TYPE_HERO));
	//Play::DrawRect(obj_hero.pos - HERO_AABB, obj_hero.pos + HERO_AABB, Play::cGreen);

	blocks(); 
	
	platforms();

	coins();

	stats();

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
		obj_hero.pos = { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100 };
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
	else if (Play::KeyDown(VK_RIGHT))
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
	if (!gamestate.floorcollision && !gamestate.platcollision)
	{
		obj_hero.velocity = { 0, 3.98f };

	}
	
	//acceleration
	obj_hero.acceleration = Vector2D(0.f, 0.2f);

	obj_hero.pos += obj_hero.velocity;
	obj_hero.velocity += obj_hero.acceleration;
}

void Walk()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);

	if (Play::KeyDown(VK_RIGHT))
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
		gamestate.SpriteStanding = true;

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

	if (Play::KeyDown(VK_SPACE))
	{
		if (gamestate.SpriteStanding)
		{
			obj_hero.velocity += gamestate.thrust;
			obj_hero.velocity += gamestate.gravity;
			obj_hero.pos += obj_hero.velocity;

			if (!gamestate.SpriteFaceLeft)
			{
				Play::SetSprite(obj_hero, "Pink_Monster_Jump_8", 0.10f);

			}

			if (gamestate.SpriteFaceLeft)
			{
				Play::SetSprite(obj_hero, "Pink_Monster_Jump_Left_8", 0.10f);
			}

		}
		
		if (Play::KeyDown(VK_SPACE) && Play::KeyDown(VK_RIGHT) ) 
		{
			obj_hero.velocity += gamestate.thrust;
			obj_hero.velocity += gamestate.jumpright;
			obj_hero.velocity += gamestate.gravity;
			obj_hero.pos += obj_hero.velocity;
		}
		
		if (Play::KeyDown(VK_SPACE) && Play::KeyDown(VK_LEFT)) 
		{
			obj_hero.velocity += gamestate.thrust;
			obj_hero.velocity += gamestate.jumpleft;
			obj_hero.velocity += gamestate.gravity;
			obj_hero.pos += obj_hero.velocity;
		}
	}
}

void Collision()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	gamestate.platcollision = false;

	gamestate.floorcollision = false;

	for (int id_platform : vPlatforms)
	{
		GameObject& obj_platform = Play::GetGameObject(id_platform);

		if (obj_hero.pos.y + HERO_AABB.y > obj_platform.pos.y - PLATFORM_AABB.y
			&& obj_hero.pos.y - HERO_AABB.y < obj_platform.pos.y + PLATFORM_AABB.y)
		{
			if (obj_hero.pos.x + HERO_AABB.x > obj_platform.pos.x - PLATFORM_AABB.x
				&& obj_hero.pos.x - HERO_AABB.x < obj_platform.pos.x + PLATFORM_AABB.x)
			{
				//gamestate.Gcollision += 1;
				gamestate.platcollision = true;
				//gamestate.platcollision += 1;

				gamestate.floorcollision = true;
				//gamestate.floorcollision += 1;

				obj_hero.velocity = { 0, 0 };
				obj_hero.pos.y = (obj_platform.pos.y - PLATFORM_AABB.y - 20);


				float minx = obj_platform.pos.x - (PLATFORM_AABB.x / 2);
				float maxx = obj_platform.pos.x + (PLATFORM_AABB.x / 2); 
				float miny = obj_platform.pos.y - (PLATFORM_AABB.y / 2); 
				float maxy = obj_platform.pos.y + (PLATFORM_AABB.y / 2); 

				//to detect collisions on the sides and bottom of the platform to fall
				if ((obj_hero.oldPos.x < minx && obj_hero.oldPos.y > miny && obj_hero.oldPos.y < maxy)
					|| (obj_hero.oldPos.x > maxx && obj_hero.oldPos.y < maxy && obj_hero.oldPos.y > miny)
					|| (obj_hero.oldPos.y > maxy) )
				{
					obj_hero.pos = obj_hero.oldPos;
					obj_hero.velocity = gamestate.gravity;
					obj_hero.pos += obj_hero.velocity;

				}

			}
		}
	}
}

void BlockCollision()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	std::vector<int> vBlocks = Play::CollectGameObjectIDsByType(TYPE_BLOCK);

	for (int id_blocks : vBlocks) 
	{
		GameObject& obj_block = Play::GetGameObject(id_blocks);

		if (obj_hero.pos.y + HERO_AABB.y > obj_block.pos.y - BLOCK_AABB.y
			&& obj_hero.pos.y - HERO_AABB.y < obj_block.pos.y + BLOCK_AABB.y)
		{
			if (obj_hero.pos.x + HERO_AABB.x > obj_block.pos.x - BLOCK_AABB.x
				&& obj_hero.pos.x - HERO_AABB.x < obj_block.pos.x + BLOCK_AABB.x)
			{
				//gamestate.Gcollision += 1;
				gamestate.blockcollision = true;
				//gamestate.platcollision += 1;

				gamestate.floorcollision = true;
				//gamestate.floorcollision += 1;

				obj_hero.velocity = { 0, 0 };
				obj_hero.pos.y = (obj_block.pos.y - BLOCK_AABB.y - 20); 


				float minx = obj_block.pos.x - (BLOCK_AABB.x / 2);
				float maxx = obj_block.pos.x + (BLOCK_AABB.x / 2);
				float miny = obj_block.pos.y - (BLOCK_AABB.y / 2);
				float maxy = obj_block.pos.y + (BLOCK_AABB.y / 2);

				//to detect collisions on the sides and bottom of the platform to fall
				if ((obj_hero.oldPos.x < minx && obj_hero.oldPos.y > miny && obj_hero.oldPos.y < maxy)
					|| (obj_hero.oldPos.x > maxx && obj_hero.oldPos.y < maxy && obj_hero.oldPos.y > miny)
					|| (obj_hero.oldPos.y > maxy))
				{
					obj_hero.pos = obj_hero.oldPos;
					obj_hero.velocity = gamestate.gravity;
					obj_hero.pos += obj_hero.velocity;

				}

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

	Vector2f temp = obj_hero.pos - gamestate.camera_focus;
	gamestate.camera_focus += temp / 2;

	//bottom
	if (gamestate.camera_focus.y > 390)
	{
		gamestate.camera_focus.y = 390;
	}

	//left
	if (gamestate.camera_focus.x < -100)
	{
		gamestate.camera_focus.x = -100; 
	}

	//right
	if (gamestate.camera_focus.x > 3500)
	{
		gamestate.camera_focus.x = 3500;
	}

	Play::SetCameraPosition((gamestate.camera_focus - Vector2f(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2)));
}

void coins()
{
	for (int c : Play::CollectGameObjectIDsByType(TYPE_COIN))
	{
		Play::DrawObjectRotated(Play::GetGameObject(c));
		
		GameObject& obj_coin = Play::GetGameObject(c);
		//Play::DrawCircle({ obj_coin.pos }, 10, Play::cOrange);
		Play::SetSprite(obj_coin, "coin_gold_8", 0.05f);   
	}
}

void createcoins(int posx, int posy, int count)  
{
	for (int c = 1; c < count + 1 ; c++) 
	{
		Play::CreateGameObject(TYPE_COIN, { posx, posy }, 10, "coin_gold_8");

		posx += 50;

		Play::CentreSpriteOrigin("coin_gold");

	}
}

void CollectCoins()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);         
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);
	for (int id_coins : vCoins)
	{
		GameObject& obj_coin = Play::GetGameObject(id_coins); 
		if (Play::IsColliding(obj_hero, obj_coin))
		{
			objectstate.CoinsCollected += 1; 
			Play::DestroyGameObject(id_coins); 
		}
	}
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
	//for (int i : Play::CollectGameObjectIDsByType(TYPE_PLATFORM))
	//{
	//	Play::DrawRect(Play::GetGameObject(i).pos - PLATFORM_AABB, Play::GetGameObject(i).pos + PLATFORM_AABB, Play::cGreen);
	//}
}

void createplatforms(int posx, int posy)
{
	std::vector<int> vPlatform = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	for (int id_platform : vPlatform)
	{

		Play::CreateGameObject(TYPE_PLATFORM, { posx, posy }, 10, "Platform");
		GameObject& obj_platform = Play::GetGameObject(id_platform); 

	}
}

void blocks()
{
	Play::CentreSpriteOrigin("Block");

	for (int b : Play::CollectGameObjectIDsByType(TYPE_BLOCK)) 
	{
		Play::DrawObject(Play::GetGameObject(b)); 
	}
	//aabb
	//for (int b : Play::CollectGameObjectIDsByType(TYPE_BLOCK)) 
	//{
	//	Play::DrawRect(Play::GetGameObject(b).pos - BLOCK_AABB, Play::GetGameObject(b).pos + BLOCK_AABB, Play::cRed);
	//}

}

void createblocks(int posx, int posy)
{
	std::vector<int> vBlocks = Play::CollectGameObjectIDsByType(TYPE_BLOCK);
	for (int id_blocks : vBlocks)
	{

		Play::CreateGameObject(TYPE_BLOCK, { posx, posy }, 10, "Block"); 
		GameObject& obj_blocks = Play::GetGameObject(id_blocks);  

	}
}

void stats()
{

	Point2D mcoord = Play::GetMousePos();
	int mouse_x = mcoord.x; 
	int mouse_y = mcoord.y; 

	//coords to place objects at
	Play::SetDrawingSpace(Play::SCREEN);
	Play::DrawFontText("64px", "X: " + std::to_string(mouse_x), Point2D(50, 450), Play::LEFT);
	Play::DrawFontText("64px", "Y: " + std::to_string(mouse_y), Point2D(50, 400), Play::LEFT); 

	Play::DrawFontText("64px", "Coins: " + std::to_string(objectstate.CoinsCollected), Point2D(50, 70), Play::LEFT);
	//Play::DrawFontText("64px", "Collision: " + std::to_string(gamestate.Gcollision), Point2D(50, 600), Play::LEFT);

	//Play::SetDrawingSpace(Play::SCREEN);
	//Play::DrawFontText("64px", "Camera Coord X: " + std::to_string(gamestate.camera_cord_x), Point2D(50, 450), Play::LEFT);
	//Play::DrawFontText("64px", "Camera Coord Y: " + std::to_string(gamestate.camera_cord_y), Point2D(50, 400), Play::LEFT);
	//Play::SetDrawingSpace(Play::WORLD);
	
	//ground collision 
	//if (gamestate.floorcollision)
	//{
	//	Play::DrawFontText("64px", "landing: " + std::to_string(gamestate.floorcollision), Point2D(50, 550), Play::LEFT);
	//}
	//else
	//{
	//	Play::DrawFontText("64px", "landing: " + std::to_string(gamestate.floorcollision), Point2D(50, 550), Play::LEFT);
	//}

	////platform collision 
	//if (gamestate.floorcollision)
	//{
	//	Play::DrawFontText("64px", "Planding: " + std::to_string(gamestate.platcollision), Point2D(50, 500), Play::LEFT);
	//}
	//else
	//{
	//	Play::DrawFontText("64px", "Planding: " + std::to_string(gamestate.platcollision), Point2D(50, 500), Play::LEFT);
	//}

	Play::SetDrawingSpace(Play::WORLD);

	//Play::SetDrawingSpace(Play::SCREEN);
	//Play::SetDrawingSpace(Play::WORLD);

}

void map()
{


	//creating platforms
	const int spacing{ 500 };
	for (int i = 0; i < 15; i++)
	{
		Play::CreateGameObject(TYPE_PLATFORM, { spacing * i, 0 }, 20, "Platform");
	}


	//creating coins
	for (int c = 0; c < 15; c++)
	{
		createcoins(spacing * c, -50, 1); 
	}

	createcoins(1280, 565, 5);
	createcoins(1920, 565, 5);


	//creating blocks
	Play::CreateGameObject(TYPE_BLOCK, { -100, 800 }, 20, "Block"); 






	///////////////////    //////////////////     /////////////////////     //////////////////////////////							/////////////////////////				////////////////////////////			//////////////////////				////////////

																											createblocks(750, 100);
																						
																						 createblocks(650, 200);

							createcoins(250, 300, 1);		createcoins(350, 250, 5);
							createblocks(250, 350);			createplatforms(450, 300); 

						createcoins(150, 400, 1);			
						createblocks(150, 450);				    

				createcoins(50, 500, 1);			
				createblocks(50, 550);				

		createcoins(-50, 600, 1);
		createblocks(-50, 650); 
	
 
	
//creating the ground/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Play::CreateGameObject(TYPE_GROUND, { 1890, DISPLAY_HEIGHT }, 20, "Ground2"); 
}
