#pragma once

#include "../Dependencies/clipper_ver5.1.6/clipper.hpp"
#include "../Dependencies/polypartition/polypartition.h"

#include "GeneralPolygon.h"
/*
#include <vector>
#include <map>
#include <set>

#include <SFML\\System.hpp>
#include <SFML\\Window.hpp>
#include <SFML\\Graphics.hpp>
#include <SFML\\Audio.hpp>
*/

class NavMesh
{
    private:

        GeneralPolygon* pNavigationMesh;

        /*
        class NavigationGraph
        {
            private:

                struct ConvexPolygon
                {
                        std::vector<sf::Vector2f> points;

                        std::vector<sf::Vector2f> edges;

                        bool peek(const sf::Vector2f& peekPoint);					
                };

                struct Conexion
                {
                    unsigned int originEdgeConexionIndex;
                    unsigned int destinationEdgeConexionIndex;
                    ConvexPolygon destination;

                    Conexion(const ConvexPolygon& _destination, unsigned int _originEdgeConexionIndex, unsigned int _destinationEdgeConexionIndex);
                };

                std::vector<Conexion> _getConexions(const ConvexPolygon& A, const ConvexPolygon& B);

                std::map< ConvexPolygon , std::set<Conexion> > _graphNodeConexions;

            public:

                void addConexion(const ConvexPolygon& origin, const ConvexPolygon& destination);

                //const std::set<Conexion>& getConexions(const ConvexPolygon& node);

        };
        */

        ClipperLib::Polygons _processWalkableObstacleRegions(const ClipperLib::Polygons& walkable, const ClipperLib::Polygons& obstacle);

        list<TPPLPoly> _processNavigationConvexTesselation(list<TPPLPoly> &polygons);

    public:

        NavMesh();

        void processNavigationMesh(const ClipperLib::Polygons& walkable, const ClipperLib::Polygons& obstacles);

        void renderNavigationMesh(float r, float g, float b, float a);

        bool queryPath();

};