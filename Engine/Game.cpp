/******************************************************************************************
*	Chili DirectX Framework Version 16.10.01											  *
*	Game.cpp																			  *
*	Copyright 2016 PlanetChili.net <http://www.planetchili.net>							  *
*																						  *
*	This file is part of The Chili DirectX Framework.									  *
*																						  *
*	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
******************************************************************************************/
#include "MainWindow.h"
#include "Game.h"
#include "Box.h"
#include <algorithm>
#include <sstream>
#include <typeinfo>
#include <functional>
#include <iterator>

Game::Game( MainWindow& wnd )
	:
	wnd( wnd ),
	gfx( wnd ),
	world( { 0.0f,-0.5f } ),
	pepe( gfx )
{
	pepe.effect.vs.cam.SetPos( { 0.0,0.0f } );
	pepe.effect.vs.cam.SetZoom( 1.0f / boundarySize );

	std::generate_n( std::back_inserter( boxPtrs ),nBoxes,[this]() {
		return Box::Spawn( boxSize,bounds,world,rng );
	} );

	class Listener : public b2ContactListener
	{
	public:
		void BeginContact( b2Contact* contact ) override
		{
			const b2Body* bodyPtrs[] = { contact->GetFixtureA()->GetBody(),contact->GetFixtureB()->GetBody() };
			if( bodyPtrs[0]->GetType() == b2BodyType::b2_dynamicBody &&
				bodyPtrs[1]->GetType() == b2BodyType::b2_dynamicBody )
			{
				Box* boxPtrs[] = {
					reinterpret_cast<Box*>(bodyPtrs[0]->GetUserData()),
					reinterpret_cast<Box*>(bodyPtrs[1]->GetUserData())
				};
				auto& tid0 = typeid(boxPtrs[0]->GetColorTrait());
				auto& tid1 = typeid(boxPtrs[1]->GetColorTrait());

				std::stringstream msg;
				msg << "Collision between " << tid0.name() << " and " << tid1.name() << std::endl;
				OutputDebugStringA( msg.str().c_str() );

				if (boxPtrs[0]->GetColorTrait() == boxPtrs[1]->GetColorTrait())
				{
					boxPtrs[0]->SetShouldDestroy();
					boxPtrs[1]->SetShouldDestroy();
				}

			}
		}
	};
	static Listener mrLister;
	world.SetContactListener( &mrLister );
}

void Game::Go()
{
	gfx.BeginFrame();
	UpdateModel();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::UpdateModel()
{
	const float dt = ft.Mark();
	world.Step( dt,8,3 );
	SplitBox();
	DestroyBox();
}

void Game::ComposeFrame()
{
	for( const auto& p : boxPtrs )
	{
		p->Draw( pepe );
	}
}

void Game::DestroyBox()
{
	const auto new_end = std::remove_if( boxPtrs.begin(), boxPtrs.end(),
		[]( const std::unique_ptr<Box>& pBox )
		{
			return pBox->GetShouldDestroy();
		}
	);
	boxPtrs.erase( new_end, boxPtrs.end() );
}

void Game::SplitBox()
{
	Vec2 pos;
	Vec2 linVel;
	Color color;
	float angVel;
	float size;
	float angle;

	auto i = std::find_if( boxPtrs.begin(), boxPtrs.end(),
		[&]( const std::unique_ptr<Box>& pBox )
		{
			bool flag = false;

			if ( pBox->GetShouldSplit() )
			{
				flag = true;

				pos = pBox->GetPosition();
				linVel = pBox->GetVelocity();
				color = pBox->GetColorTrait().GetColor();
				angVel = pBox->GetAngularVelocity();
				size = pBox->GetSize() / 2.0f;
				angle = pBox->GetAngle();
			}
			return flag;
		}
	);
	if ( i != boxPtrs.end() && (size >= 0.1f) )
	{
		boxPtrs.erase( i );
		boxPtrs.emplace_back( std::make_unique<Box>( Box::MakeColorTrait( color ), world, Vec2( pos.x - size, pos.y + size ), size, angle, linVel, angVel ) );
		boxPtrs.emplace_back( std::make_unique<Box>( Box::MakeColorTrait( color ), world, Vec2( pos.x + size, pos.y + size ), size, angle, linVel, angVel ) );
		boxPtrs.emplace_back( std::make_unique<Box>( Box::MakeColorTrait( color ), world, Vec2( pos.x + size, pos.y - size ), size, angle, linVel, angVel ) );
		boxPtrs.emplace_back( std::make_unique<Box>( Box::MakeColorTrait( color ), world, Vec2( pos.x - size, pos.y - size ), size, angle, linVel, angVel ) );
	}
}