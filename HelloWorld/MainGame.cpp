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
	STATE_DEAD,

	STATE_PLAY,
};

enum GameObjectType
{
	TYPE_HERO,
	TYPE_OWL,
	TYPE_THIEF,

	TYPE_DIALOGUE,

	TYPE_PLATFORM,
	TYPE_BLOCK,   
	TYPE_GROUND,
	TYPE_COIN,
	TYPE_THORN,
};

struct GameState
{
	int health = 50; 

	bool gameOver = false; 

	int Gcollision = 0;
	bool floorcollision = false;
	bool platcollision = false;
	bool blockcollision = false;
	bool SpriteFaceLeft = false;
	bool SpriteStanding = false;
	bool SpriteHurt = false; 
	int SpriteHit = 0; 

	const Vector2D gravity = { 0, 2.5f };
	const Vector2D fall = { 0, 0.5f };  
	const Vector2D jumpright = { 0.3f, 0 };
	const Vector2D jumpleft = { -0.3f, 0 };
	const Vector2D thrust = { 0, -3 };

	double Gtime = 0.0; 
	double Jtime = 0.0; 

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
	bool Coinsgiven = false; 
	int dialogueClosed = 0; 

	int bookmark = 0; 
};

ObjectState objectstate;

//declarations

void map(); 
void blocks(); 
void createblocks(int posx, int posy); 
void coins();
void CollectCoins();
void thorns();    
void thorncollision(); 

void platforms();

void createcoins(int posx, int posy, int count);   
void stats();

void UpdateHero();
void UpdateOwl();  
void UpdateThief(); 

void UpdateDialogue();   
void Dialogue(); 

void UpdateControls(); 
void Draw();

void groundcollision();
void UpdateCamera();
void Collision();
void BlockCollision(); 
void obj_GroundCollision(GameObjectType TYPE);   

void Jump(); 
void fall();
void Walk();

void gameOver(); 

void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{

	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background4.png");

	Play::CreateGameObject(TYPE_HERO, { DISPLAY_WIDTH / 2, 100 }, 10, "Pink_Monster");
	Play::CreateGameObject(TYPE_OWL, { -250, DISPLAY_HEIGHT - 50 }, 40, "Owlet_Monster_Idle_4");
	Play::CreateGameObject(TYPE_THIEF, { 610, DISPLAY_HEIGHT - 50 }, 40, "Dude_Monster_Idle_4"); 


	map();    
} 

bool MainGameUpdate(float elapsedTime)
{
	gamestate.Gtime += elapsedTime;  

	if (Play::KeyDown(VK_SPACE))
	{
		gamestate.Jtime += elapsedTime; 
	}
	else
	{
		gamestate.Jtime = 0.0; 
	}

	UpdateDialogue();   
	//Dialogue(); 

	UpdateCamera();

	UpdateHero();

	UpdateOwl(); 

	UpdateThief();  


	Draw();

	obj_GroundCollision(TYPE_THIEF); 

	obj_GroundCollision(TYPE_OWL);  


	Collision();

	groundcollision();

	BlockCollision();


	CollectCoins(); 

	thorncollision();   

	return Play::KeyDown(VK_ESCAPE);

}

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK; //indicated program ginished without any errors
}

void UpdateDialogue()
{
	for (int d : Play::CollectGameObjectIDsByType(TYPE_DIALOGUE))
	{
		Play::DrawObjectRotated(Play::GetGameObject(d));
	}

	Dialogue(); 
}

void Dialogue() // solve: multiple dialogues are created 
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	GameObject& obj_owl = Play::GetGameObjectByType(TYPE_OWL);
	GameObject& obj_thief = Play::GetGameObjectByType(TYPE_THIEF);   
	bool boxRetrieved = false; 

	if (&Play::GetGameObjectByType(TYPE_DIALOGUE) != &Play::noObject)
	{
		switch(objectstate.dialogueClosed)
		{
		case 0:
			if (!Play::IsColliding(obj_hero, obj_owl) && !Play::IsColliding(obj_hero, obj_thief)) 
			{
				Play::DestroyGameObjectsByType(TYPE_DIALOGUE);
			}
			break;

		case 1:
			if (Play::IsColliding(obj_hero, obj_owl))
			{
				Play::DestroyGameObjectsByType(TYPE_DIALOGUE);
				objectstate.dialogueClosed = 0;  
			}
			break; 
		}
	}
	else
	{


		switch (objectstate.bookmark)
		{
		case 0:
			if (objectstate.CoinsCollected == 0 && Play::IsColliding(obj_hero, obj_owl))
			{
				Play::CreateGameObject(TYPE_DIALOGUE, { -630, 550 }, 10, "greet_1");
			}
			
			if (objectstate.CoinsCollected == 1)
			{
				objectstate.bookmark = 1; 
			}

			break; 

		case 1: 

			if (objectstate.CoinsCollected == 1 && !Play::IsColliding(obj_hero, obj_owl))
			{
				objectstate.dialogueClosed = 1;
				Play::CreateGameObject(TYPE_DIALOGUE, { -630, 610 }, 10, "greet_excl_1");
			}
			if (Play::IsColliding(obj_hero, obj_owl))
			{
				objectstate.bookmark = 5;
			}


			break; 

		case 5:  

			if (objectstate.CoinsCollected >= 1 && Play::IsColliding(obj_hero, obj_owl))
			{
				Play::CreateGameObject(TYPE_DIALOGUE, { -630, 600 }, 10, "greet_2_1");
			}

			if (Play::IsColliding(obj_hero, obj_thief) && objectstate.CoinsCollected <= 6 )
			{
				objectstate.bookmark = 10;
			}

			if (Play::IsColliding(obj_hero, obj_thief) && objectstate.CoinsCollected > 6)
			{
				objectstate.bookmark = 15;
			}

			break; 

		case 10:

			if (Play::IsColliding(obj_hero, obj_thief) && objectstate.CoinsCollected <= 6)
			{
				Play::CreateGameObject(TYPE_DIALOGUE, { obj_thief.pos.x , 450 }, 10, "greet_3_1");  
			}

			if (Play::IsColliding(obj_hero, obj_owl) && objectstate.CoinsCollected < 11 )
			{
				objectstate.bookmark = 5; 
			}

			if (Play::IsColliding(obj_hero, obj_thief) && objectstate.CoinsCollected > 6)
			{
				objectstate.bookmark = 15;
			}

			break; 

		case 15:

			if (Play::IsColliding(obj_hero, obj_thief) && objectstate.CoinsCollected > 6 && objectstate.CoinsCollected < 11)
			{
				Play::CreateGameObject(TYPE_DIALOGUE, { obj_thief.pos.x + 50, 600 }, 10, "greet_4_1");
			}

			if (Play::IsColliding(obj_hero, obj_owl) && objectstate.CoinsCollected < 11)
			{
				objectstate.bookmark = 5;
			}

			if (Play::IsColliding(obj_hero, obj_thief) && objectstate.CoinsCollected == 11)
			{
				objectstate.bookmark = 20;
			}
			break; 

		case 20:

			if ((!objectstate.Coinsgiven && objectstate.CoinsCollected == 11 && Play::IsColliding(obj_hero, obj_thief))) //arbitrary values for testing -- coinscollected 
			{
				Play::CreateGameObject(TYPE_DIALOGUE, { obj_hero.pos.x - 150, obj_hero.pos.y - 50 }, 100, "greet_5_1");
				objectstate.Coinsgiven = true;
			}
			else if (objectstate.Coinsgiven && objectstate.CoinsCollected == 11 && Play::IsColliding(obj_hero, obj_thief))	//arbitrary values for testing -- coinscollected 
			{
				Play::CreateGameObject(TYPE_DIALOGUE, { obj_thief.pos.x - 100 , obj_thief.pos.y - 150 }, 100, "greet_6_1");
			}

			if (Play::IsColliding(obj_hero, obj_owl))
			{
				objectstate.bookmark = 25;
			}
			break; 

		case 25: 

			if (Play::IsColliding(obj_hero, obj_owl)) 
			{
				Play::CreateGameObject(TYPE_DIALOGUE, { obj_owl.pos.x + 40, obj_owl.pos.y - 180 }, 100, "greet_7_1"); 
			}

			if (Play::IsColliding(obj_hero, obj_thief))
			{
				Play::CreateGameObject(TYPE_DIALOGUE, { obj_thief.pos.x + 20 , obj_thief.pos.y - 150 }, 100, "greet_8_1");
			}

			break;  

		}
	}

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

	//draw owl
	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_OWL));
	Play::CentreMatchingSpriteOrigins("Owlet_Monster"); 

	//draw thief
	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_THIEF));
	Play::CentreMatchingSpriteOrigins("Dude_Monster_Idle_4"); 

	thorns(); 

	blocks(); 
	
	platforms();

	coins();

	stats();

	//draw dialogues
	for (int d : Play::CollectGameObjectIDsByType(TYPE_DIALOGUE))
	{
		Play::DrawObject(Play::GetGameObject(d));
		Play::CentreMatchingSpriteOrigins("greet");
	}

	Play::PresentDrawingBuffer();
}

                                             
void UpdateHero()
{
	GameObject& obj_ground = Play::GetGameObjectByType(TYPE_GROUND);
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	obj_hero.scale = 1.8f;

	//setting bounds-restricted movement for obi - sides
	if (obj_hero.pos.x < -720 || obj_hero.pos.x > 3120)
	{
		obj_hero.pos.x = std::clamp(obj_hero.pos.x, -720.f, 3120.f);    
	}


	switch (gamestate.herostate)
	{
	case STATE_IDLE:
		Play::DrawFontText("64px", "PRESS ENTER TO START", { -450, DISPLAY_HEIGHT - 500 }, Play::CENTRE);
		Play::PresentDrawingBuffer();

		Play::SetSprite(obj_hero, "Pink_Monster", 0.05f);
		obj_hero.pos = { -600, DISPLAY_HEIGHT - 100 };
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

		if (gamestate.health < 1)
		{
			gamestate.herostate = STATE_DEAD; 
		}

		break;

	case STATE_JUMP:

		Jump();  

		Collision(); 

		groundcollision(); 

		BlockCollision(); 

		if (gamestate.floorcollision || gamestate.platcollision)
		{
			gamestate.herostate = STATE_WALK;
		}
		break;

	case STATE_LAND:
		Collision();
		groundcollision();
		break;


	case STATE_DEAD: 

		Play::SetSprite(obj_hero, "Pink_Monster_Death_8", 0.05f);

		if (Play::IsAnimationComplete(obj_hero))
		{
			gameOver(); 

		}

		if (Play::KeyDown(VK_SPACE))
		{
			gamestate.herostate = STATE_IDLE;
		}
		break; 
	}

}

void gameOver()
{
	gamestate.gameOver = true; 

	Play::DestroyGameObjectsByType(TYPE_HERO); 
	Play::DestroyGameObjectsByType(TYPE_THIEF);
	Play::DestroyGameObjectsByType(TYPE_OWL);
	
	Play::DestroyGameObjectsByType(TYPE_PLATFORM); 
	Play::DestroyGameObjectsByType(TYPE_COIN);
	Play::DestroyGameObjectsByType(TYPE_BLOCK); 
	Play::DestroyGameObjectsByType(TYPE_THORN);

}

void UpdateOwl()
{
	GameObject& obj_owl = Play::GetGameObjectByType(TYPE_OWL);
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	obj_owl.scale = 2.5f; 

	switch (gamestate.herostate)
	{
	case STATE_IDLE: 
		Play::SetSprite(obj_owl, "Owlet_Monster", 0.05f);
		obj_owl.pos = { -650, DISPLAY_HEIGHT - 70 };
		obj_owl.velocity = { 0, 0 };
		if (Play::KeyDown(VK_RETURN))
		{
			gamestate.herostate = STATE_PLAY; 
		}
		break;

	case STATE_PLAY:
		Play::SetSprite(obj_hero, "Owlet_Monster_Idle_4", 0.10f); 
		break;
	}
}

void UpdateThief( )
{
	GameObject& obj_thief = Play::GetGameObjectByType(TYPE_THIEF); 
	obj_thief.scale = 2.5f;

	switch (gamestate.herostate)
	{
	case STATE_IDLE:
		Play::SetSprite(obj_thief, "Dude_Monster_Idle_4", 0.05f);
		obj_thief.pos = { 920, DISPLAY_HEIGHT - 70 };
		obj_thief.velocity = { 0, 0 };
		if (Play::KeyDown(VK_RETURN))
		{
			gamestate.herostate = STATE_PLAY;
		}
		break;

	case STATE_PLAY:
		Play::SetSprite(obj_thief, "Dude_Monster_Idle_4", 0.10f);
		break;
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

	if (Play::KeyDown(VK_SPACE) && gamestate.Jtime < 0.4) 
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
			obj_hero.velocity += gamestate.thrust * 0.9 ;
			obj_hero.velocity += gamestate.jumpright;
			obj_hero.velocity += gamestate.gravity;
			obj_hero.pos += obj_hero.velocity;
		}
		
		if (Play::KeyDown(VK_SPACE) && Play::KeyDown(VK_LEFT)) 
		{
			obj_hero.velocity += gamestate.thrust * 0.9 ;
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

void obj_GroundCollision(GameObjectType TYPE)  
{
	GameObject& obj_owl = Play::GetGameObjectByType(TYPE);   
	GameObject& obj_ground = Play::GetGameObjectByType(TYPE_GROUND);
	gamestate.floorcollision = false;

	if (obj_owl.pos.y + HERO_AABB.y > obj_ground.pos.y - GROUND_AABB.y
		&& obj_owl.pos.y - HERO_AABB.y < obj_ground.pos.y + GROUND_AABB.y)
	{
		gamestate.floorcollision = true;
		gamestate.floorcollision += 1;

		obj_owl.velocity = { 0, 0 };
		obj_owl.pos.y = (obj_ground.pos.y - PLATFORM_AABB.y - 30);
	}

	Play::UpdateGameObject(obj_owl); 
}



void UpdateCamera() //set top, set boundaries
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
	if (gamestate.camera_focus.x > 2500)
	{
		gamestate.camera_focus.x = 2500;
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
	//std::vector<int> vPlatform = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	//for (int id_platform : vPlatform)
	//{
		Play::CreateGameObject(TYPE_PLATFORM, { posx, posy }, 10, "Platform");
	//	GameObject& obj_platform = Play::GetGameObject(id_platform); 

	//}
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
	//std::vector<int> vBlocks = Play::CollectGameObjectIDsByType(TYPE_BLOCK);
	//for (int id_blocks : vBlocks)
	//{

		Play::CreateGameObject(TYPE_BLOCK, { posx, posy }, 10, "Block"); 
		//GameObject& obj_blocks = Play::GetGameObject(id_blocks);  

	//}
}


void thorns()
{
	Play::CentreSpriteOrigin("Thorn");

	for (int t : Play::CollectGameObjectIDsByType(TYPE_THORN))
	{
		Play::DrawObject(Play::GetGameObject(t)); 
		GameObject& obj_thorn = Play::GetGameObject(t); 

		//Play::DrawCircle({ obj_thorn.pos }, 35, Play::cGreen);  
		Play::SetSprite(obj_thorn, "Thorn", 0.05f); 
	}
}

void createthorns(int posx, int posy, int count) 
{
	//for (int t = 1; t < count + 1; t++)
	//{
		Play::CreateGameObject(TYPE_THORN, { posx, posy }, 35, "Thorn");

		posx += 50;

	//}
}

void thorncollision()
{
	GameObject& obj_hero = Play::GetGameObjectByType(TYPE_HERO);
	std::vector<int> vThorns = Play::CollectGameObjectIDsByType(TYPE_THORN); 

	for (int id_thorns : vThorns)
	{
		GameObject& obj_thorns = Play::GetGameObject(id_thorns); 
		if (Play::IsColliding(obj_hero, obj_thorns))
		{
			gamestate.SpriteHurt = true;   
			gamestate.SpriteHit += 1; 
			gamestate.health -= 1; 

			Play::SetSprite(obj_hero, "Pink_Monster_Hurt_4", 0.30f); 

			if (!gamestate.SpriteFaceLeft)
			{
				obj_hero.pos.x += -20;
			}
			else
			{
				obj_hero.pos.x += 20; 
			}
		}
	}
}



void stats()
{

	Point2D mcoord = Play::GetMousePos();
	int mouse_x = mcoord.x;  
 	int mouse_y = mcoord.y; 

	
	Play::SetDrawingSpace(Play::SCREEN);

	//------------------   coords to place objects at
	//Play::DrawFontText("64px", "X: " + std::to_string(mouse_x), Point2D(50, 450), Play::LEFT);
	//Play::DrawFontText("64px", "Y: " + std::to_string(mouse_y), Point2D(50, 400), Play::LEFT); 

	Play::DrawFontText("64px", "Coins: " + std::to_string(objectstate.CoinsCollected), Point2D(50, 70), Play::LEFT);
	Play::DrawFontText("64px", "Hits: " + std::to_string(gamestate.SpriteHit), Point2D(50, 150), Play::LEFT);
	Play::DrawFontText("64px", "Health: " + std::to_string(gamestate.health), Point2D(50, 200), Play::LEFT);

	if (gamestate.gameOver)
	{
		Play::DrawFontText("132px", "GAME OVER", Point2D(640, 375), Play::CENTRE);
		Play::DrawFontText("64px", "(The forest is no easy feat, press space to try again)", Point2D(640, 500), Play::CENTRE); 
	}
	//------------------ timer
	//Play::DrawFontText("64px", "GameTime: " + std::to_string(gamestate.Gtime), Point2D(50, 270), Play::LEFT);
	//Play::DrawFontText("64px", "JumpTime: " + std::to_string(gamestate.Jtime), Point2D(50, 310), Play::LEFT);

	//Play::DrawFontText("64px", "Collision: " + std::to_string(gamestate.Gcolli  sion), Point2D(50, 600), Play::LEFT);

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
	Play::CreateGameObject(TYPE_PLATFORM, { 0, 0 }, 20, "Platform"); 
	//creating platforms
	const int spacing{ 500 };
	for (int i = 0; i < 10; i++)
	{
		Play::CreateGameObject(TYPE_PLATFORM, { spacing * i, 0 }, 20, "Platform"); 
	}

	////creating coins
	for (int c = 0; c < 15; c++)
	{
		createcoins(spacing * c, -50, 1); 
	}

	createcoins(-250, 680, 1); 
	createcoins(680, 675, 5);
	createcoins(1280, 675, 5);
	createcoins(1920, 675, 5);
	createcoins(2760, 675, 5);

	// first thorn 
	createthorns(DISPLAY_WIDTH/2, DISPLAY_HEIGHT - 50, 1);    

	//creating blocks
	Play::CreateGameObject(TYPE_BLOCK, { -100, 800 }, 20, "Block"); 

/// WORLD 1 // Code aligned for visualisation - don't judge :P

									     ///////////////                             //////////////////			////////////////////			///////////////////			///////////////////				////////////////////////			/////////////////////			////////////////////////////			//////////////////////////			///////////////////////////		 /////////////////////////////
															createblocks(120, 80);
				/*	createblocks(100, 100); */
										createblocks(250, 180); 

					createblocks(100, 280);																	createcoins(850, 250, 5);																																																					createcoins(2000, 250 , 4);
																													createplatforms(950, 300);								createblocks(1200, 300);																					createblocks(1800, 350);								createplatforms(2050, 300);
																																	  createthorns(1050, 350, 1);										createcoins(1350, 350, 3);																						createthorns(1955, 350, 1);
										createblocks(250, 380);			/*createblocks(690, 380);*/				createcoins(850, 380, 4); createthorns(1050, 380, 1);				createthorns(1300, 380, 1);					createthorns(1500, 380, 1);			createblocks(1650, 380);								createthorns(1955, 380, 1);		createcoins(2000, 380, 4);			createblocks(2300, 380);
																																	  createthorns(1050, 400, 1);									createplatforms(1400, 400);																							createthorns(1955, 400, 1);																	createblocks(2400, 480);
				createblocks(100, 480);																			createplatforms(950, 450);																																																		createplatforms(2050, 450);																	createblocks(2500, 580);
																																																																																																																	createblocks(2600, 670);																																																																																																										
																																		createthorns(1050, 500, 1);
								createcoins(250, 500, 5);				createcoins(650, 400, 1);										createthorns(1050, 540, 1);
								createplatforms(350, 550);				createblocks(650, 470);											createthorns(1050, 570, 1);
																																		createthorns(1050, 600, 1);
																																		createthorns(1050, 640, 1);
		createblocks(100, 650);																											createthorns(1050, 670, 1);
																																		createthorns(1050, 700, 1);
//creating the ground//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Play::CreateGameObject(TYPE_GROUND, { 1890, DISPLAY_HEIGHT }, 20, "Ground2");

}
