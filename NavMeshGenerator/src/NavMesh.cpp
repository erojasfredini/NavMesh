#include "NavMesh.h"


#include <iostream>
#include "GeneralPolygon.h"

/*
////////////////////////////////////////////////////////////
///				ConvexPolygon
////////////////////////////////////////////////////////////

bool NavMesh::NavigationGraph::ConvexPolygon::peek(const sf::Vector2f& peekPoint)
{
    sf::Vector2f P1, P2, P1_P2, P1_peekPoint;
    float cross;
    for(unsigned int i=1; i < (points.size()+1) ; ++i)
    {
        P1 = points[i-1];
        P2 = points[i%points.size()];

        P1_P2 = P2 - P1;
        P1_peekPoint = peekPoint - P1;

        cross = P1_P2.x * P1_peekPoint.y - P1_P2.y * P1_peekPoint.x;

        if( cross < 0.0f )
            return false;
    }
    return true;
}

////////////////////////////////////////////////////////////
///				Conexion
////////////////////////////////////////////////////////////

NavMesh::NavigationGraph::Conexion::Conexion(const ConvexPolygon& _destination, unsigned int _originEdgeConexionIndex, unsigned int _destinationEdgeConexionIndex): destination(_destination), originEdgeConexionIndex(_originEdgeConexionIndex), destinationEdgeConexionIndex(_destinationEdgeConexionIndex)
{}

////////////////////////////////////////////////////////////
///				NavigationGraph
////////////////////////////////////////////////////////////

std::vector<NavMesh::NavigationGraph::Conexion> NavMesh::NavigationGraph::_getConexions(const ConvexPolygon& A, const ConvexPolygon& B)
{
    auto areEdgesParallel = [](const sf::Vector2f& e1, const sf::Vector2f& e2) -> bool
    {

    };

    auto distanceEdges = [](const sf::Vector2f& e1, const sf::Vector2f& e2) -> bool
    {

    };

    auto getEdgePointsIndexs = [](unsigned int edgeIndex) -> std::pair<unsigned int, unsigned int>
    {
        return std::pair<unsigned int,unsigned int>(edgeIndex, edgeIndex+1);
    };

    auto testIfCloseEdgePoints = [](std::pair<unsigned int, unsigned int> edgePoints1, std::pair<unsigned int, unsigned int> edgePoints2) -> bool
    {
        
    };

    std::vector<Conexion> conexions;

    for(unsigned int eA=0; eA < A.edges.size() ; ++eA )
        for(unsigned int eB=0; eA < B.edges.size() ; ++eB )
        {
            if( areEdgesParallel(A.edges[eA], B.edges[eA]) )
                if( distanceEdges(A.edges[eA], B.edges[eA]) < 0.1f )
                    if( testIfCloseEdgePoints(getEdgePointsIndexs(eA), getEdgePointsIndexs(eB) )
                        conexions.push_back( Conexion(A, ) );
        }
}

void NavMesh::NavigationGraph::addConexion(const ConvexPolygon& origin, const ConvexPolygon& destination)
{
    _graphNodeConexions[origin].insert(destination);
}

const std::set<NavMesh::NavigationGraph::ConvexPolygon>& NavMesh::NavigationGraph::getConexions(const ConvexPolygon& node)
{
    return _graphNodeConexions[node];
}

*/
////////////////////////////////////////////////////////////
///				NavMesh
////////////////////////////////////////////////////////////


TPPLPoly TPPLPoly_To_Polygon(const ClipperLib::Polygon& B)
{
    TPPLPoly poly;
    poly.Init(B.size());
    for(unsigned int i=0; i < B.size() ; ++i)
    {
        poly[i].x = B[i].X;
        poly[i].y = B[i].Y;
    }
    return poly;
}

std::list<TPPLPoly> fromClipperFormat2PolyPartitionFormat(const ClipperLib::Polygons& clipperPoly)
{
    std::list<TPPLPoly> polypartitionPoly;
    for(unsigned int i=0; i < clipperPoly.size() ; ++i)
        polypartitionPoly.push_back( TPPLPoly_To_Polygon(clipperPoly[i]) );

    return polypartitionPoly;
}

NavMesh::NavMesh(): pNavigationMesh(NULL)
{}

ClipperLib::Polygons NavMesh::_processWalkableObstacleRegions(const ClipperLib::Polygons& walkable, const ClipperLib::Polygons& obstacles)
{
    ClipperLib::Clipper clipper;

    ClipperLib::Polygons solution;

    clipper.AddPolygons(walkable, ClipperLib::ptSubject );

    clipper.AddPolygons(obstacles, ClipperLib::ptClip);

    clipper.Execute(ClipperLib::ctDifference, solution, ClipperLib::PolyFillType::pftNonZero, ClipperLib::PolyFillType::pftNonZero );

    return solution;
}

list<TPPLPoly> NavMesh::_processNavigationConvexTesselation(list<TPPLPoly> &polygons)
{
    TPPLPartition partition;

    list<TPPLPoly> navigationMeshPolygons;

    // convex tesselation with minimal convex elements
    int res = partition.ConvexPartition_HM(&polygons, &navigationMeshPolygons);
    
    if( res == 0 )
        std::cerr<< "NavMesh::processNavigationTesselation -> Error in the ConvexPartition_HM method"<<std::endl;

    return navigationMeshPolygons;
}

void NavMesh::processNavigationMesh(const ClipperLib::Polygons& walkable, const ClipperLib::Polygons& obstacles)
{
    ClipperLib::Polygons navMeshPolygon_ClipperFormat = _processWalkableObstacleRegions(walkable, obstacles);

    // GeneralPolygon already tesselates the polygon so we only need the output from Clipper
    // This GeneralPolygon is only for rendering
    if( pNavigationMesh != NULL )
        delete pNavigationMesh;

    pNavigationMesh = new GeneralPolygon(navMeshPolygon_ClipperFormat);

    //list<TPPLPoly> navMeshPolygon_PolyPartitionFormat = fromClipperFormat2PolyPartitionFormat(navMeshPolygon_ClipperFormat);
    GeneralPolygon P(navMeshPolygon_ClipperFormat);
    list<TPPLPoly> navMeshPolygon_PolyPartitionFormat = (list<TPPLPoly>)P;

    list<TPPLPoly> navMeshTesselation_PolyPartitionFormat = _processNavigationConvexTesselation(navMeshPolygon_PolyPartitionFormat);
}

void NavMesh::renderNavigationMesh(float r, float g, float b, float a)
{
    if( pNavigationMesh != NULL )
    {
        pNavigationMesh->setColor(r, g, b, a);

        pNavigationMesh->render();
        pNavigationMesh->renderConvexTesselation();
        pNavigationMesh->renderConvexTesselationAdjacency();
    }
}

bool NavMesh::queryPath()
{
    return true;
}