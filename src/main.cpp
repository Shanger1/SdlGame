#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <string>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
Uint32 startTime;
bool collision = false;
bool gotShot = false;
int main();

class LTexture
{
	public:
		LTexture();

		~LTexture();

		bool loadFromFile( std::string path );
		
		#ifdef _SDL_TTF_H
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		#endif

		void free();

		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		void setBlendMode( SDL_BlendMode blending );

		void setAlpha( Uint8 alpha );
		
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		int getWidth();
		int getHeight();

	private:
		SDL_Texture* mTexture;

		int mWidth;
		int mHeight;
};

class Enemy
{
    public:
		static const int ENEMY_WIDTH = 32;
		static const int ENEMY_HEIGHT = 48;

		static const int ENEMY_VEL = 1;

		Enemy();

		void move(SDL_Rect& obstacle);

		void render();

		int mPosX, mPosY;

		int mAcc, mVelAccelerated;
		
		SDL_Rect mCollider;

	private:
		int mVelX, mVelY;
};

class Soldier
{
    public:
		static const int SOLDIER_WIDTH = 32;
		static const int SOLDIER_HEIGHT = 48;

		static const int SOLDIER_VEL = 1;

		Soldier();

		void handleEvent( SDL_Event& e );
		void move( SDL_Rect& obstacle );
		void render();
		int mPosX, mPosY;

		SDL_Rect mCollider;

    private:
		int mVelX, mVelY;
};

class Bullet
{
    public:
		static const int BULLET_WIDTH = 28;
		static const int BULLET_HEIGHT = 20;

		static const int BULLET_VEL = 1;

		Bullet();

		void handleEvent( SDL_Event& e );

		void move(Soldier& soldier, Enemy& enemy);

		void render();

		int mPosX, mPosY;

		bool shot;
		
		SDL_Rect mCollider;

	private:
		int mVelX, mVelY;
};

Enemy::Enemy()
{
    mPosX = 20;
    mPosY = SCREEN_HEIGHT-ENEMY_HEIGHT;

	mCollider.w = ENEMY_WIDTH;
	mCollider.h = ENEMY_HEIGHT;

	mAcc = 0;
	mVelAccelerated = 0;

    mVelX = 0;
    mVelY = 0;
}

Soldier::Soldier()
{
    mPosX = 240;
    mPosY = SCREEN_HEIGHT-SOLDIER_HEIGHT;

	mCollider.w = SOLDIER_WIDTH;
	mCollider.h = SOLDIER_HEIGHT;

    mVelX = 0;
    mVelY = 0;
}

Bullet::Bullet()
{
    mPosX = 700;
    mPosY = 700;

	mCollider.w = BULLET_WIDTH;
	mCollider.h = BULLET_HEIGHT;

	shot = false;

    mVelX = 0;
    mVelY = 0;
}

class LoseScreen
{
public:	
	void handleEvent(SDL_Event& e, bool& collision);
	void render();
};

bool init();

bool loadMedia();

void close();

bool checkCollision( SDL_Rect a, SDL_Rect b );

Uint32 time();

SDL_Window* gWindow = NULL;

SDL_Renderer* gRenderer = NULL;

LTexture gSoldierTexture;
LTexture gEnemyTexture;
LTexture gLoseScreenTexture;
LTexture gBulletTexture;


LTexture::LTexture()
{
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	free();

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		SDL_FreeSurface( loadedSurface );
	}

	mTexture = newTexture;
	return mTexture != NULL;
}

void LTexture::free()
{
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

void Soldier::handleEvent( SDL_Event& e )
{
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        switch( e.key.keysym.sym )
        {
            case SDLK_LEFT: mVelX -= SOLDIER_VEL; break;
            case SDLK_RIGHT: mVelX += SOLDIER_VEL; break;
        }
    }
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        switch( e.key.keysym.sym )
        {
            case SDLK_LEFT: mVelX += SOLDIER_VEL; break;
            case SDLK_RIGHT: mVelX -= SOLDIER_VEL; break;
        }
    }
}

void Soldier::move( SDL_Rect& enemy )
{

    mPosX += mVelX;
	mCollider.x = mPosX;
	mCollider.y = mPosY;

    if( ( mPosX < 0 ) || ( mPosX + SOLDIER_WIDTH > SCREEN_WIDTH ))
    {
        mPosX -= mVelX;
		mCollider.x = mPosX;
    }else if(checkCollision( mCollider, enemy )){
		collision = true;
	}
}

void Soldier::render()
{
	gSoldierTexture.render( mPosX, mPosY );
}

void Enemy::move( SDL_Rect& soldier )
{
	mAcc += 1;
		if(mAcc == 20 || mAcc == 30 || mAcc == 40 || mAcc == 50)
		{
			mVelAccelerated = mAcc/15;
		}


	mCollider.x = mPosX;
	mCollider.y = mPosY;

    if(checkCollision(mCollider, soldier))
    {
		mPosX = 700;
		mPosY = 700;
		collision = true;
    }else{
		if (mAcc % 4 == 1) {
			mPosX += 0.1 + mVelAccelerated;
		}
		mCollider.x = mPosX;
	}

}

void Enemy::render()
{
	gEnemyTexture.render( mPosX, mPosY );
}

void Bullet::handleEvent( SDL_Event& e )
{
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        switch( e.key.keysym.sym )
        {
			case SDLK_SPACE: shot = true; break;
        }
    }
}

void Bullet::move( Soldier& soldier, Enemy& enemy)
{
	if (shot) {
		mPosX = soldier.mPosX-10;
		mPosY = soldier.mPosY+10;
		mCollider.x = mPosX;
		mCollider.y = mPosY;
		shot = !shot;
	}

    mPosX -= 1;
	mCollider.x = mPosX;

	if(checkCollision(mCollider, enemy.mCollider))
    {
		gotShot = true;
		gEnemyTexture.free();
    }
}

void Bullet::render()
{
	gBulletTexture.render( mPosX, mPosY );
}


void LoseScreen::handleEvent( SDL_Event& e, bool& collision )
{
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_1:
			collision = false;
			main();
			break;
		default:
			break;
		}
	}
}

void LoseScreen::render() {
	gLoseScreenTexture.render(0, 0);
}

bool init()
{
	bool success = true;

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		gWindow = SDL_CreateWindow( "SDL Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	startTime = SDL_GetTicks();

	return success;
}

bool loadMedia()
{
	bool success = true;

	//zrodlo: http://pixeljoint.com/pixelart/1350.htm
	if( !gSoldierTexture.loadFromFile( "soldier.gif" ) )
	{
		printf( "Failed to load soldier texture!\n" );
		success = false;
	}
	//zrodlo: http://pixeljoint.com/pixelart/1084.htm
	if( !gEnemyTexture.loadFromFile( "enemy.gif" ) )
	{
		printf( "Failed to load enemy texture!\n" );
		success = false;
	}
	//zrodlo: https://thenewinquiry.com/born-to-lose/
	if (!gLoseScreenTexture.loadFromFile("loseScreen.png"))
	{
		printf("Failed to load loseScreen texture!\n");
		success = false;
	}
	//zrodlo: http://pixeljoint.com/pixelart/85269.htm
	if (!gBulletTexture.loadFromFile("bullet.gif"))
	{
		printf("Failed to load bullet texture!\n");
		success = false;
	}

	return success;
}

void close()
{
	gSoldierTexture.free();
	gEnemyTexture.free();
	gLoseScreenTexture.free();
	gBulletTexture.free();


	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

bool checkCollision( SDL_Rect a, SDL_Rect b )
{
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;

    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;

    if( bottomA <= topB )
    {
        return false;
    }

    if( topA >= bottomB )
    {
        return false;
    }

    if( rightA <= leftB )
    {
        return false;
    }

    if( leftA >= rightB )
    {
        return false;
    }

    return true;
}

int main()
{
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{	
			bool quit = false;

			SDL_Event e;

			collision = false;


			Soldier soldier;
			Enemy enemy;
			Bullet bullet;
			LoseScreen loseScreen;
			
			while( !quit )
			{
				if(collision == true){
					while (SDL_PollEvent(&e) != 0)
					{
						loseScreen.handleEvent(e, collision);
					}
					loseScreen.render();
				}
				else if(collision == false){
					while( SDL_PollEvent( &e ) != 0 )
					{
						if( e.type == SDL_QUIT )
						{
							quit = true;
						}

						soldier.handleEvent( e);
						bullet.handleEvent( e);
					}

					soldier.move( enemy.mCollider );
					enemy.move( soldier.mCollider );
					bullet.move( soldier, enemy );

					SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
					SDL_RenderClear( gRenderer );
				
					soldier.render();
					enemy.render();	
					if(gotShot == true){
						enemy.mPosX = 700;
						enemy.mPosY = 700;
						gotShot = false;
						
					}
					bullet.render();			
				}
				SDL_RenderPresent( gRenderer );
				SDL_Delay( 2 );
			}
		}
	}
	close();
	return 0;
}
