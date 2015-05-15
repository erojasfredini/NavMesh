#pragma once

#include <List>

// para usar librerias dinamicas comentar el siguiente define
// y cambiar los nombres de las librerias contra la que linkeamos
#define SFML_STATIC

#include <SFML\\System.hpp>
#include <SFML\\Window.hpp>
#include <SFML\\Graphics.hpp>
#include <SFML\\Audio.hpp>

#include "NavMesh.h"

//#include "WalkableCuadrilateral.h"
#include "GeneralPolygon.h"

#include "PolygonUserController.h"

class NavMeshEditorApp
{
	private:

		enum EditorState {EditingWalkableRegion, EditingObstacleRegion, ViewingNavMeshResult};

		EditorState _curAppState;

		static NavMeshEditorApp* _instance;

		NavMeshEditorApp();

		int _width;

		int _height;

		sf::RenderWindow _App;

		std::vector<GeneralPolygon*> _walkableCuadrilaterals;

		std::vector<GeneralPolygon*> _obstacleCuadrilaterals;

		NavMesh* _pNavMesh;

		void _processEvent(const sf::Event& event);

		void _inicializarCamara();

		void _render();

		// Renders the NavMesh current Solution
		// If only the Union of the Walkable regions are processed then only that will be rendered
		// If the Intersection of the Walkable and Obstacle regions are processed then that will be rendered
		// If the final NavMesh tesselation is processed then that final NavMesh will be rendered
		void _drawNavMeshSolution(const ClipperLib::Polygons& solution);

		void _processNavMesh();

		PolygonUserController _polygonUserController;

		void _changeEditorState(EditorState state);

	public:

		static NavMeshEditorApp& getInstance();

		static sf::RenderWindow& getRenderer();

		~NavMeshEditorApp();

		int run();

		GeneralPolygon* peek(const sf::Vector2f& click);

		GeneralPolygon* peek(const sf::Vector2i& click);

		void addPolygon(GeneralPolygon& polygon);
};