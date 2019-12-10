#include "NavMeshEditorApp.h"

#include <iostream>


// Used to render tesselated polygons
#include <SFML/OpenGL.hpp>

using namespace std;

NavMeshEditorApp* NavMeshEditorApp::_instance = NULL;

NavMeshEditorApp::NavMeshEditorApp(): _width(800), _height(600), _App(sf::VideoMode(_width, _height, 32), "NavMesh Editor"), _polygonUserController(_height), _pNavMesh(NULL)
{
	_pNavMesh = new NavMesh();
}

NavMeshEditorApp::~NavMeshEditorApp()
{
	if( _pNavMesh != NULL )
		delete _pNavMesh;

	_walkableCuadrilaterals.clear();
}

NavMeshEditorApp& NavMeshEditorApp::getInstance()
{
	if( _instance != NULL )
		return *_instance;
	else
	{
		_instance = new NavMeshEditorApp();
		return *_instance;
	}
}

sf::RenderWindow& NavMeshEditorApp::getRenderer()
{
	if( _instance != NULL )
		return (_instance->_App);
	else
	{
		NavMeshEditorApp::_instance = new NavMeshEditorApp();
		return (_instance->_App);
	}
}

void NavMeshEditorApp::_inicializarCamara()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, _width, 0, _height, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
}

int NavMeshEditorApp::run()
{
	_inicializarCamara();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glShadeModel(GL_SMOOTH);
	
	/*
	GeneralPolygon* pPoly = new GeneralPolygon();

	pPoly->setPosition(sf::Vector2f(100.0f,100.0f));
	pPoly->setRotation(-45);
	pPoly->setSize(sf::Vector2f(100,100));
	pPoly->setColor(1.0f, 0.0f, 0.0f, 0.8f);

	pPoly->addEquilateralTriangle();
	
	pPoly->addPentagram(0.7f, 0.2f, sf::Vector2f(0.0f, -0.3f));

	pPoly->addLetterE(0.2f, 0.2f);

	pPoly->addPentagram(0.3f, 0.3f, sf::Vector2f(0.3f, 0.3f));

	pPoly->addPentagram(0.3f, 0.3f, sf::Vector2f(-0.3f, 0.3f));

	pPoly->addRectangle(0.8f, 0.4f, sf::Vector2f(0.0f, 0.8f));

	_walkableCuadrilaterals.push_back(pPoly);
	*/

	_curAppState = EditingWalkableRegion;

	sf::Event event;
		
	//Loop
	while (_App.isOpen())
	{
		while( _App.pollEvent(event) )
			_processEvent(event);
		
		_App.clear();

		_render();
		
		_App.display();
	}
	return EXIT_SUCCESS;
}

void NavMeshEditorApp::_processEvent(const sf::Event& event)
{
	_polygonUserController.processUserEvent(event);

	switch(event.type)
	{
		case sf::Event::KeyPressed:
		{
			switch(event.key.code)
			{
				case sf::Keyboard::A:
				{
					_changeEditorState(EditorState::EditingWalkableRegion);

					std::cerr<<"State: EditingWalkableRegion"<<std::endl;
				}
					break;

				case sf::Keyboard::S:
				{
					_changeEditorState(EditorState::EditingObstacleRegion);

					std::cerr<<"State: EditingObstacleRegion"<<std::endl;
				}
					break;

				case sf::Keyboard::Q:
				{
					// walkableRegions
					ClipperLib::Polygons walkableRegions;
					for(unsigned int i=0; i < _walkableCuadrilaterals.size() ; ++i)
					{
						ClipperLib::Polygons p = (ClipperLib::Polygons)*_walkableCuadrilaterals[i];
						for(unsigned int j=0; j < p.size() ; ++j)
							walkableRegions.push_back(p[j]);
					}

					std::cerr<<"Process Walkable Polygons"<<std::endl;

					// obstaclesRegions
					ClipperLib::Polygons obstacleRegions;
					for(unsigned int i=0; i < _obstacleCuadrilaterals.size() ; ++i)
					{
						ClipperLib::Polygons p = (ClipperLib::Polygons)*_obstacleCuadrilaterals[i];
						for(unsigned int j=0; j < p.size() ; ++j)
							obstacleRegions.push_back(p[j]);
					}

					std::cerr<<"Process Obstacles Polygons"<<std::endl;

					_pNavMesh->processNavigationMesh( walkableRegions, obstacleRegions );

					walkableRegions.clear();
					obstacleRegions.clear();

					_changeEditorState(EditorState::ViewingNavMeshResult);
				}
					break;

				case sf::Keyboard::Escape:
					_App.close();
					break;
			}
		}
			break;
				
		case sf::Event::Closed://Si se cierra la ventana
			_App.close();
			break;
	}
}

void NavMeshEditorApp::_render()
{
	switch(_curAppState)
	{
		case EditorState::EditingWalkableRegion:
		case EditorState::EditingObstacleRegion:
		{
			for(unsigned int i=0;i < _walkableCuadrilaterals.size(); ++i )
				_walkableCuadrilaterals[i]->render();

			for(unsigned int i=0;i < _obstacleCuadrilaterals.size(); ++i )
				_obstacleCuadrilaterals[i]->render();
		}
			break;
		case EditorState::ViewingNavMeshResult:
		{
			if( _pNavMesh != NULL )
				_pNavMesh->renderNavigationMesh(1.0f,1.0f,0.0f,0.6f);
		}
			break;
	}
}

GeneralPolygon* NavMeshEditorApp::peek(const sf::Vector2f& click)
{
	switch( _curAppState )
	{
		case EditorState::EditingWalkableRegion:
		{
			for(unsigned int i=0; i < _walkableCuadrilaterals.size() ; ++i)
				if( _walkableCuadrilaterals[i]->peek(click) )
					return _walkableCuadrilaterals[i];
		}
			break;

		case EditorState::EditingObstacleRegion:
		{
			for(unsigned int i=0; i < _obstacleCuadrilaterals.size() ; ++i)
				if( _obstacleCuadrilaterals[i]->peek(click) )
					return _obstacleCuadrilaterals[i];
		}
			break;
	}
	//if( _navMesh != NULL && _navMesh->peek(click) )
	//	return _navMesh;

	return NULL;
}

GeneralPolygon* NavMeshEditorApp::peek(const sf::Vector2i& click)
{
	return peek(sf::Vector2f(click.x, click.y));
}

void NavMeshEditorApp::addPolygon(GeneralPolygon& polygon)
{
	switch( _curAppState )
	{
		case EditorState::EditingWalkableRegion:
			polygon.setColor(0.0f, 1.0f, 0.0f, 1.0f);
			_walkableCuadrilaterals.push_back(&polygon);
		break;

		case EditorState::EditingObstacleRegion:
			polygon.setColor(1.0f, 0.0f, 0.0f, 1.0f);
			_obstacleCuadrilaterals.push_back(&polygon);
		break;
	}
}

void NavMeshEditorApp::_changeEditorState(EditorState state)
{
	_curAppState = state;

	switch(_curAppState)
	{
		case EditorState::EditingWalkableRegion:
		{
			_polygonUserController.setEnabled(true);

			for(unsigned int i=0; i < _walkableCuadrilaterals.size() ; ++i)
			{
				float r,g,b,a;
				_walkableCuadrilaterals[i]->getColor(r,g,b,a);
				_walkableCuadrilaterals[i]->setColor(r,g,b,1.0f);
			}

			for(unsigned int i=0; i < _obstacleCuadrilaterals.size() ; ++i)
			{
				float r,g,b,a;
				_obstacleCuadrilaterals[i]->getColor(r,g,b,a);
				_obstacleCuadrilaterals[i]->setColor(r,g,b,0.6f);
			}
		}
			break;
		case EditorState::EditingObstacleRegion:
		{
			_polygonUserController.setEnabled(true);

			for(unsigned int i=0; i < _obstacleCuadrilaterals.size() ; ++i)
			{
				float r,g,b,a;
				_obstacleCuadrilaterals[i]->getColor(r,g,b,a);
				_obstacleCuadrilaterals[i]->setColor(r,g,b,1.0f);
			}

			for(unsigned int i=0; i < _walkableCuadrilaterals.size() ; ++i)
			{
				float r,g,b,a;
				_walkableCuadrilaterals[i]->getColor(r,g,b,a);
				_walkableCuadrilaterals[i]->setColor(r,g,b,0.6f);
			}
		}
			break;
		case EditorState::ViewingNavMeshResult:
		{
			_polygonUserController.setEnabled(false);
		}
			break;
	}
}