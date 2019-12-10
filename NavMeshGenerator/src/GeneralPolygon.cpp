#include "GeneralPolygon.h"

#include <iostream>

#define SMALL_NUM 0.0000000001f
#define ADJACENT_DISTANCE 0.1f

float GeneralPolygon::clipperScaleCoeficient = 10.0f;


GeneralPolygon::GeneralPolygon(const sf::Vector2f& position, const sf::Vector2f& size, float r, float g, float b, float a)
    : _position(position), _rotation(0.0f), _size(size), _gluTesselator(NULL), _r(r), _g(g), _b(b), _a(a), _renderMode(RenderMethod::GLU_Tessalation)
{}

GeneralPolygon::GeneralPolygon(const sf::Vector2i& position, const sf::Vector2f& size, float r, float g, float b, float a)
    : _position(position.x, position.y), _rotation(0.0f), _size(size), _gluTesselator(NULL), _r(r), _g(g), _b(b), _a(a), _renderMode(RenderMethod::GLU_Tessalation)
{}

GeneralPolygon::GeneralPolygon(const ClipperLib::Polygons& Polygons): 
    _position(0.0f, 0.0f), _rotation(0.0f), _size(1.0f, 1.0f), _gluTesselator(NULL), _renderMode(RenderMethod::GLU_Tessalation)
{
    _contours.resize(Polygons.size());
    for(unsigned int c=0; c < _contours.size() ; ++c)
    {
        const ClipperLib::Polygon &polygon = Polygons[c];
        Contour &contour = _contours[c];

        contour.resize(polygon.size());

        for(unsigned int v=0; v < polygon.size() ; ++v)
            contour[v] = sf::Vector2f(polygon[v].X / clipperScaleCoeficient, polygon[v].Y / clipperScaleCoeficient);

        PolygonWindingOrder winding = _getWinding(contour);

        if( winding == PolygonWindingOrder::Winding_CW )
            std::reverse(contour.begin(), contour.end());
        
    }

    _updateOptimalConvexTesselation();
}

GeneralPolygon::GeneralPolygon(list<TPPLPoly>& Polygons): 
    _position(0.0f, 0.0f), _rotation(0.0f), _size(1.0f, 1.0f), _gluTesselator(NULL), _renderMode(RenderMethod::GLU_Tessalation)
{
    _contours.resize(Polygons.size());
    int c=0;

    list<TPPLPoly>::iterator it = Polygons.begin();

    while( it != Polygons.end() )
    {
        TPPLPoly &polygon = *it;
        Contour &contour = _contours[c];

        contour.resize(polygon.GetNumPoints());

        for(int v=0; v < polygon.GetNumPoints() ; ++v)
            contour[v] = sf::Vector2f(polygon[v].x, polygon[v].y);

        ++c;
        ++it;
    }
    
    _updateOptimalConvexTesselation();
}

GeneralPolygon::~GeneralPolygon()
{
    if( _gluTesselator != NULL )
        gluDeleteTess(_gluTesselator);

    for(unsigned int c=0; c < _contours.size() ; ++c)
        _contours[c].clear();
    _contours.clear();

    _clearCombineVertexData();

    _optimalConvexTesselation.clear();
}

GeneralPolygon::operator ClipperLib::Polygons()
{
    ClipperLib::Polygons polys(_contours.size());
    for(unsigned int c=0; c < _contours.size() ; ++c)
    {
        const Contour &contour = _contours[c];
        ClipperLib::Polygon poly(contour.size());

        for(unsigned int v=0; v < contour.size() ; ++v)
        {
            sf::Vector2f worldPos = _transformFromLocalToWorld(contour[v]);
            poly[v] = ClipperLib::IntPoint(worldPos.x * clipperScaleCoeficient, worldPos.y * clipperScaleCoeficient);
        }

        polys[c] = poly;
    }
    return polys;
}

GeneralPolygon::operator list<TPPLPoly>()
{
    
    auto isPolygonOutside = [&](const Contour &referencePolygon, const Contour &poly)
    {
        for(unsigned int p=0; p < referencePolygon.size() ; ++p)
            if( !_evenOddRuleAlgorithm( referencePolygon[p], poly) )
                return true;
        return false;
    };

    list<TPPLPoly> polys;
    for(unsigned int c=0; c < _contours.size() ; ++c)
    {
        const Contour &contour = _contours[c];
        TPPLPoly poly;
        poly.Init(contour.size());

        for(unsigned int v=0; v < contour.size() ; ++v)
        {
            TPPLPoint point;
            point.x = contour[v].x;
            point.y = contour[v].y;
            poly[v] = point;
        }

        // BE CAREFULL!!! not really correct because the polygon could be concave then
        // the center of mass could not be inside the polygon
        sf::Vector2f centerOfMass(0.0f, 0.0f);
        for(unsigned int i=0; i < contour.size() ; ++i)
            centerOfMass += contour[i];
        centerOfMass *= (1.0f/contour.size());

        std::vector<Contour> outsideContours;
        for(unsigned int i=0; i < _contours.size() ; ++i)
        {
            if( i == c )
            {
                outsideContours.push_back(_contours[i]);
                continue;
            }
            if( isPolygonOutside(_contours[i], contour) )
                outsideContours.push_back(_contours[i]);
        }

        // first test if is a hole or not
        if( !_evenOddRuleAlgorithm( centerOfMass, outsideContours ) )
            poly.SetHole(true);
        else
            poly.SetHole(false);

        // if it is a hole then it must be in CW order to the algorithm to recognize
        // else must be in CCW
        if( poly.IsHole() )
            poly.SetOrientation(TPPL_CW);// Hole orientation (needed in the algorithm)
        else
            poly.SetOrientation(TPPL_CCW);// Not a hole orientation (needed in the algorithm)

        polys.push_back(poly);
    }
    return polys;
}

void GeneralPolygon::setRenderMode(RenderMethod mode)
{
    _renderMode = mode;
}

void GeneralPolygon::addContour(const Contour& contour)
{
    _contours.push_back(contour);

    // updates the tesselation of convex polygons so we can make peek of the polygons
    _updateOptimalConvexTesselation();
}

GLdouble* GeneralPolygon::_newCombineVertexData(GLdouble x, GLdouble y)
{
  GLdouble *vert = new GLdouble[3];
  vert[0] = x;
  vert[1] = y;
  vert[2] = 0;
  _combineNewVertexData.push_back(vert);
  return vert;
}

void GeneralPolygon::_clearCombineVertexData()
{
    for (std::vector<GLdouble*>::size_type i = 0; i < _combineNewVertexData.size(); ++i)
        delete[] _combineNewVertexData[i];
    _combineNewVertexData.clear(); 
}

void GeneralPolygon::render()
{
    //http://www.flipcode.com/archives/Polygon_Tessellation_In_OpenGL.shtml

    glPushMatrix();
    
    glTranslatef(_position.x, _position.y, 0.0f);
    glRotatef(_rotation,0.0f, 0.0f, 1.0f);
    glScalef(_size.x, _size.y, 1.0f);

    glColor4f( _r, _g, _b, _a);

    switch( _renderMode )
    {
        case RenderMethod::GLU_Tessalation:
        {

            // first time rendering? then create the GLUTesselate object
            if( _gluTesselator == NULL )
            {
                // helper lambda function that creates a vertex for OpenGL
                auto combineTesselate = [&](GLdouble coords[3], GLdouble *data[4], GLfloat weight[4], GLdouble **dataOut ) {GLdouble *vert = _newCombineVertexData(coords[0], coords[1]); *dataOut = vert;};
                auto errorTesselate = [] (GLenum errorCode) { std::cerr<< (char *)gluErrorString(errorCode); };

                _gluTesselator = gluNewTess();

                // defines all the callback to render the polygons
                gluTessCallback(_gluTesselator, GLU_TESS_BEGIN, (void (CALLBACK*)()) glBegin );
                gluTessCallback(_gluTesselator, GLU_TESS_VERTEX, (void (CALLBACK*)()) glVertex3dv );
                gluTessCallback(_gluTesselator, GLU_TESS_END, (void (CALLBACK*)()) glEnd );
                gluTessCallback(_gluTesselator, GLU_TESS_COMBINE, (void (CALLBACK*)()) &combineTesselate );
                gluTessCallback(_gluTesselator, GLU_TESS_ERROR, (void (CALLBACK*)()) &errorTesselate );

                // tells to all polygons that the normal is in +z direction
                gluTessNormal(_gluTesselator, 0.0, 0.0, 1.0);
                gluTessProperty(_gluTesselator, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
                gluTessProperty(_gluTesselator, GLU_TESS_BOUNDARY_ONLY, GL_FALSE);
            }	
            
            gluTessBeginPolygon(_gluTesselator, NULL); 
            for(unsigned int i = 0; i < _contours.size(); ++i)
            {
                const Contour& contour = _contours[i];
                gluTessBeginContour(_gluTesselator);

                for(unsigned int j = 0; j < contour.size(); ++j)
                {
                    GLdouble *vert = _newCombineVertexData((GLdouble)contour[j].x, (GLdouble)contour[j].y);
                    gluTessVertex(_gluTesselator, vert, vert); 
                }
                gluTessEndContour(_gluTesselator); 
            }

            // here at the TessEnd is where GLU makes its magic and tesselate our stuff
            gluTessEndPolygon(_gluTesselator);

            _clearCombineVertexData();
        }
        break;
        
        case RenderMethod::ConvexTessalation:
        {
            list<TPPLPoly>::iterator it = _optimalConvexTesselation.begin();

            while( it != _optimalConvexTesselation.end() )
            {
                glBegin(GL_POLYGON);// render convex polygon only

                TPPLPoly &poly = *it;

                for(unsigned int p=0; p < poly.GetNumPoints() ; ++p)
                    glVertex3f(poly[p].x, poly[p].y, 0);

                if( poly.GetNumPoints() > 0 )
                    glVertex3f(poly[0].x, poly[0].y, 0);
        
                glEnd();

                ++it;
            }
        }
        break;

        default:
            std::cerr<<"GeneralPolygon::render -> Render Method set not valid"<<std::endl;
        break;
    }

    glPopMatrix();
}

void GeneralPolygon::renderConvexTesselation()
{
    // first render convex polygon transparently
    glColor4f( 1.0f, 1.0f, 0.0f, 0.3f);

    glPushMatrix();
    
    glTranslatef(_position.x, _position.y, 0.0f);
    glRotatef(_rotation,0.0f, 0.0f, 1.0f);
    glScalef(_size.x, _size.y, 1.0f);

    list<TPPLPoly>::iterator it = _optimalConvexTesselation.begin();

    while( it != _optimalConvexTesselation.end() )
    {
        glBegin(GL_POLYGON);// render convex polygon only

        TPPLPoly &poly = *it;

        for(unsigned int p=0; p < poly.GetNumPoints() ; ++p)
            glVertex3f(poly[p].x, poly[p].y, 0);

        if( poly.GetNumPoints() > 0 )
            glVertex3f(poly[0].x, poly[0].y, 0);
        
        glEnd();

        ++it;
    }

    glColor4f( 1.0f, 1.0f, 0.0f, 1.0f);

    glLineStipple(1, 0x3F07);
    glEnable(GL_LINE_STIPPLE);

    it = _optimalConvexTesselation.begin();

    // second render the boundary lines
    while( it != _optimalConvexTesselation.end() )
    {
        glBegin(GL_LINE_STRIP);

        TPPLPoly &poly = *it;

        for(unsigned int p=0; p < poly.GetNumPoints() ; ++p)
            glVertex3f(poly[p].x, poly[p].y, 0);

        if( poly.GetNumPoints() > 0 )
            glVertex3f(poly[0].x, poly[0].y, 0);
        
        glEnd();

        ++it;
    }

    glDisable(GL_LINE_STIPPLE);

    glPopMatrix();
}

std::vector<PolygonAdjacency> GeneralPolygon::GetAllConexions()
{
    std::vector<PolygonAdjacency> conexions;

    list<TPPLPoly>::iterator it1 = _optimalConvexTesselation.begin();

    while( it1 != _optimalConvexTesselation.end() )
    {
        list<TPPLPoly>::iterator it2 = _optimalConvexTesselation.begin();

        while( it2 != _optimalConvexTesselation.end() )
        {
            if( it1 != it2 )
            {
                std::vector<PolygonAdjacency> moreConexions = _getConexions(*it1, *it2);
                std::copy( moreConexions.begin(), moreConexions.end(), std::inserter(conexions, conexions.end()));
            }
            ++it2;
        }
        ++it1;
    }

    return conexions;
}

void GeneralPolygon::renderConvexTesselationAdjacency()
{
    std::vector<PolygonAdjacency> conexions = GetAllConexions();

    glPushMatrix();
    
    glTranslatef(_position.x, _position.y, 0.0f);
    glRotatef(_rotation,0.0f, 0.0f, 1.0f);
    glScalef(_size.x, _size.y, 1.0f);

    glColor4f( 0.0f, 0.0f, 1.0f, 1.0f);

    glLineStipple(1, 0x3F07);
    glEnable(GL_LINE_STIPPLE);

    auto getPolygonCenter = [] (TPPLPoly &A) -> sf::Vector2f
    {
        sf::Vector2f center(0.0f, 0.0f);
        for(long i=0; i < A.GetNumPoints() ; ++i)
            center += sf::Vector2f(A[i].x, A[i].y);
        center *= (1.0f/A.GetNumPoints());
        return center;
    };

    for(unsigned int i=0; i < conexions.size() ; ++i)
    {
            
        glBegin(GL_LINE_STRIP);

        PolygonAdjacency &adjacency = conexions[i];

        //TPPLPoint P1 = adjacency.polygonA->GetPoint(adjacency.polygonAEdge);
        //TPPLPoint P2 = adjacency.polygonA->GetPoint((adjacency.polygonAEdge+1)%adjacency.polygonA->GetNumPoints());

        sf::Vector2f centerA = getPolygonCenter(*adjacency.polygonA);
        sf::Vector2f centerB = getPolygonCenter(*adjacency.polygonB);

        glVertex3f(centerA.x, centerA.y, 0);

        glVertex3f(centerB.x, centerB.y, 0);
        
        glEnd();

    }

    glDisable(GL_LINE_STIPPLE);

    glPopMatrix();
}

void GeneralPolygon::move(const sf::Vector2f& movement)
{
    _position += movement;
}

void GeneralPolygon::move(const sf::Vector2i& movement)
{
    move(sf::Vector2f(movement.x, movement.y));
}

void GeneralPolygon::setPosition(const sf::Vector2f& position)
{
    _position = position;
}

void GeneralPolygon::setPosition(const sf::Vector2i& position)
{
    setPosition(sf::Vector2f(position.x, position.y));
}

sf::Vector2f GeneralPolygon::getPosition()
{
    return _position;
}

void GeneralPolygon::rotate(float degree)
{
    _rotation += degree;
}

void GeneralPolygon::setRotation(float degree)
{
    _rotation = degree;
}

float GeneralPolygon::getRotation()
{
    return _rotation;
}

const sf::Vector2f& GeneralPolygon::getSize()
{
    return _size;
}

void GeneralPolygon::setColor(float r, float g, float b, float a)
{
    _r = r;
    _g = g;
    _b = b;
    _a = a;
}

void GeneralPolygon::getColor(float& r, float& g, float& b, float& a)
{
    r = _r;
    g = _g;
    b = _b;
    a = _a;
}

void GeneralPolygon::setSize(const sf::Vector2f& size)
{
    _size = size;
}

void GeneralPolygon::setSize(const sf::Vector2i& size)
{
    setSize(sf::Vector2f(size.x, size.y));
}

bool GeneralPolygon::peek(const sf::Vector2f& click)
{
    // the best way to test point inside concave polygon
    return _evenOddRuleAlgorithm( _transformFromWorldToLocal(click) );

    // Dummy implementation that test inside the convex polygons using
    // the cross product to test if a point is inside the convex polygon (only work on convex polygons)
    /*

    auto TPPLPoint2Vector2f = [](const TPPLPoint& a) -> sf::Vector2f
    {
        return sf::Vector2f(a.x, a.y);
    };

    list<TPPLPoly>::iterator it = _optimalConvexTesselation.begin();

    while( it != _optimalConvexTesselation.end() )
    {
        TPPLPoly &poly = *it;

        bool polygonPeeked = true;
        sf::Vector2f P1, P2, P1_P2, P1_click, position;
        float cross;
        for(unsigned int i=1; i < (poly.GetNumPoints()+1) ; ++i)
        {
            P1 = _transformFromLocalToWorld( TPPLPoint2Vector2f(poly[i-1]) );
            P2 = _transformFromLocalToWorld( TPPLPoint2Vector2f(poly[i%poly.GetNumPoints()]) );

            P1_P2 = P2 - P1;
            P1_click = click - P1;

            cross = P1_P2.x * P1_click.y - P1_P2.y * P1_click.x;

            if( cross < 0.0f )
            {
                polygonPeeked = false;
                break;
            }
        }


        if( polygonPeeked )
            return true;

        ++it;
    }

    return false;
    */
}

bool GeneralPolygon::peek(const sf::Vector2i& click)
{
    return this->peek( sf::Vector2f(click.x, click.y) );
}

void GeneralPolygon::addEquilateralTriangle(float scaleX, float scaleY, const sf::Vector2f& displace)
{
    GeneralPolygon::Contour tri;

    sf::Vector2f scale(scaleX, scaleY);

    auto dot = [](const sf::Vector2f& a, const sf::Vector2f& b)-> sf::Vector2f {return sf::Vector2f(a.x*b.x, a.y*b.y);};
    
    tri.push_back( dot( sf::Vector2f(0.0f,0.5f) + displace, scale) );
    tri.push_back( dot( sf::Vector2f(-0.5f,-0.5f) + displace, scale) );
    tri.push_back( dot( sf::Vector2f(0.5f,-0.5f) + displace, scale) );

    addContour(tri);
}

void GeneralPolygon::addRectangle(float scaleX, float scaleY, const sf::Vector2f& displace)
{
    GeneralPolygon::Contour rect;

    sf::Vector2f scale(scaleX, scaleY);

    auto dot = [](const sf::Vector2f& a, const sf::Vector2f& b)-> sf::Vector2f {return sf::Vector2f(a.x*b.x, a.y*b.y);};
    
    rect.push_back( dot( sf::Vector2f(0.5f,0.5f), scale) + displace);
    rect.push_back( dot( sf::Vector2f(-0.5f,0.5f), scale) + displace );
    rect.push_back( dot( sf::Vector2f(-0.5f,-0.5f), scale) + displace );
    rect.push_back( dot( sf::Vector2f(0.5f,-0.5f), scale) + displace );

    addContour(rect);
}

void GeneralPolygon::addPentagram(float scaleX, float scaleY, const sf::Vector2f& displace)
{
    GeneralPolygon::Contour pentagram;

    sf::Vector2f scale(scaleX, scaleY);

    auto dot = [](const sf::Vector2f& a, const sf::Vector2f& b)-> sf::Vector2f {return sf::Vector2f(a.x*b.x, a.y*b.y);};

    float PI = atan(1)*4;
    float degreeToRad = PI/180.0f;
    
    pentagram.push_back( dot( sf::Vector2f(0.5f*cos(18*degreeToRad), 0.5f*sin(18*degreeToRad)), scale) + displace );
    pentagram.push_back( dot( sf::Vector2f(0.0f, 0.5f), scale) + displace );
    pentagram.push_back( dot( sf::Vector2f(0.5f*cos(162*degreeToRad), 0.5f*sin(162*degreeToRad)), scale) + displace );
    pentagram.push_back( dot( sf::Vector2f(0.5f*cos(234*degreeToRad), 0.5f*sin(234*degreeToRad)), scale) + displace );
    pentagram.push_back( dot( sf::Vector2f(0.5f*cos(306*degreeToRad), 0.5f*sin(306*degreeToRad)), scale) + displace );

    addContour(pentagram);
}

void GeneralPolygon::addLetterE(float scaleX, float scaleY, const sf::Vector2f& displace)
{
    GeneralPolygon::Contour E;

    sf::Vector2f scale(scaleX, scaleY);

    auto dot = [](const sf::Vector2f& a, const sf::Vector2f& b)-> sf::Vector2f {return sf::Vector2f(a.x*b.x, a.y*b.y);};

    E.push_back( dot( sf::Vector2f(0.5f,0.5f), scale) + displace );
    E.push_back( dot( sf::Vector2f(-0.5f,0.5f), scale ) + displace );
    E.push_back( dot( sf::Vector2f(-0.5f,-0.5f), scale) + displace );
    E.push_back( dot( sf::Vector2f(0.5f,-0.5f), scale) + displace );
    E.push_back( dot( sf::Vector2f(0.5f,-0.3f), scale) + displace );
    E.push_back( dot( sf::Vector2f(-0.1f,-0.3f), scale) + displace );
    E.push_back( dot( sf::Vector2f(-0.1f,-0.1f), scale) + displace );
    E.push_back( dot( sf::Vector2f(0.5f,-0.1f), scale) + displace );
    E.push_back( dot( sf::Vector2f(0.5f,0.1f), scale) + displace );
    E.push_back( dot( sf::Vector2f(-0.1f,0.1f), scale) + displace );
    E.push_back( dot( sf::Vector2f(-0.1f,0.3f), scale) + displace );
    E.push_back( dot( sf::Vector2f(0.5f,0.3f), scale) + displace );

    addContour(E);
}

void GeneralPolygon::_updateOptimalConvexTesselation()
{
    TPPLPartition partition;
    
    _optimalConvexTesselation.clear();

    list<TPPLPoly> polys = (list<TPPLPoly>)*this;
    
    // convex tesselation with minimal convex elements
    int res = partition.ConvexPartition_HM(&polys, &_optimalConvexTesselation);

    // _optimalConvexTesselation will be in local space

    if( res == 0 )
        std::cerr<< "GeneralPolygon::_updateOptimalConvexTesselation -> Error in the ConvexPartition_HM method"<<std::endl;

    polys.clear();
}

bool GeneralPolygon::_evenOddRuleAlgorithm(const sf::Vector2f& testPoint, const Contour & polygon)
{
    //http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html

    int i, j;
    bool inside = false;

    for (i = 0, j = polygon.size()-1; i < polygon.size(); j = i++)
    {
        const sf::Vector2f &pointI = polygon[i];
        const sf::Vector2f &pointJ = polygon[j];

        if ( ((pointI.y>testPoint.y) != (pointJ.y>testPoint.y)) &&
            (testPoint.x < (pointJ.x-pointI.x) * (testPoint.y-pointI.y) / (pointJ.y-pointI.y) + pointI.x) )
            inside = !inside;
    }

    return inside;
}

bool GeneralPolygon::_evenOddRuleAlgorithm(const sf::Vector2f& testPoint, const std::vector<Contour> & arrayToTest)
{
    //http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html

    int i, j;
    bool inside = false;

    for(int c=0; c < arrayToTest.size() ; ++c)
    {
        const Contour &polygon = arrayToTest[c];

        for (i = 0, j = polygon.size()-1; i < polygon.size(); j = i++)
        {
            const sf::Vector2f &pointI = polygon[i];
            const sf::Vector2f &pointJ = polygon[j];

            if ( ((pointI.y>testPoint.y) != (pointJ.y>testPoint.y)) &&
                (testPoint.x < (pointJ.x-pointI.x) * (testPoint.y-pointI.y) / (pointJ.y-pointI.y) + pointI.x) )
                inside = !inside;
        }

    }

    return inside;
}

bool GeneralPolygon::_evenOddRuleAlgorithm(const sf::Vector2f& testPoint)
{
    return _evenOddRuleAlgorithm(testPoint, _contours);
}

sf::Vector2f GeneralPolygon::_transformFromLocalToWorld(const sf::Vector2f& localPosition)
{
    static float PI = atan(1)*4;
    static float degreeToRad = PI/180.0f;

    sf::Vector2f worldPosition;
    // scale
    float x = localPosition.x * _size.x;
    float y = localPosition.y * _size.y;

    // rotation
    float rotationRad = _rotation * degreeToRad;
    worldPosition.x = x * cos(rotationRad) - y * sin(rotationRad);
    worldPosition.y = x * sin(rotationRad) + y * cos(rotationRad);

    // translation
    worldPosition.x += _position.x;
    worldPosition.y += _position.y;
    return worldPosition;
}

sf::Vector2f GeneralPolygon::_transformFromWorldToLocal(const sf::Vector2f& worldPosition)
{
    static float PI = atan(1)*4;
    static float degreeToRad = PI/180.0f;

    sf::Vector2f localPosition;

    // translation
    float x = worldPosition.x - _position.x;
    float y = worldPosition.y - _position.y;

    // rotation
    float rotationRad = (-_rotation) * degreeToRad;
    localPosition.x = x * cos(rotationRad) - y * sin(rotationRad);
    localPosition.y = x * sin(rotationRad) + y * cos(rotationRad);

    // scale
    localPosition.x = localPosition.x / _size.x;
    localPosition.y = localPosition.y / _size.y;
    
    return localPosition;
}

GeneralPolygon::PolygonWindingOrder GeneralPolygon::_getWinding(const Contour& polygon)
{
    // search for a convex vertex, ie a vertex that with his neighbors
    // form a triangle inside the polygon always. They could be any extreme vertex
    // Here we search for the vertex with min X
    // Returns the index of the convex vertex. If no vertex returns -1
    auto searchConvexVertexIndex = [](const Contour& polygon) -> int
    {
        if( polygon.size() == 0 )
            return -1;

        int minX_VertexIndex = 0;
        for(int v=1; v < polygon.size() ; ++v)
            if( polygon[v].x < polygon[minX_VertexIndex].x )
                minX_VertexIndex = v;
        return minX_VertexIndex;
    };

    int convexVertexIndex = searchConvexVertexIndex(polygon);

    if( convexVertexIndex != -1 )
    {
        const sf::Vector2f &P_2 = polygon[convexVertexIndex];
        const sf::Vector2f &P_1 = polygon[((convexVertexIndex-1) < 0)? (polygon.size()-1): (convexVertexIndex-1)];
        const sf::Vector2f &P_3 = polygon[(convexVertexIndex+1)%polygon.size()];

        float cross;
        sf::Vector2f P1_P2 = P_2 - P_1;
        sf::Vector2f P2_P3 = P_3 - P_2;

        cross = P1_P2.x * P2_P3.y - P1_P2.y * P2_P3.x;

        if( cross < 0.0f )
            return PolygonWindingOrder::Winding_CW;

        if( cross > 0.0f )
            return PolygonWindingOrder::Winding_CCW;
    }

    return PolygonWindingOrder::Winding_None;
}

std::vector<sf::Vector2f> _getPoints(TPPLPoly& A)
{
    std::vector<sf::Vector2f> points;
    for(unsigned int i=0; i < A.GetNumPoints() ; ++i)
    {
        sf::Vector2f P( A[i].x, A[i].y);

        points.push_back( P );
    }
    return points;
}

std::vector<sf::Vector2f> _getEdges(TPPLPoly& A)
{
    std::vector<sf::Vector2f> edges;
    for(unsigned int i=0; i < (A.GetNumPoints()+1) ; ++i)
    {
        sf::Vector2f P1( A[i].x, A[i].y);
        sf::Vector2f P2( A[(i+1)%A.GetNumPoints()].x, A[(i+1)%A.GetNumPoints()].y );

        edges.push_back( P2 - P1 );
    }
    return edges;
}

std::pair<sf::Vector2f, sf::Vector2f> _getEdgePoints(unsigned int edgeIndex, std::vector<sf::Vector2f> &points)
{
    return std::pair<sf::Vector2f, sf::Vector2f>(points[edgeIndex], points[(edgeIndex+1)%points.size()]);
}

std::vector<PolygonAdjacency> GeneralPolygon::_getConexions(TPPLPoly& A, TPPLPoly& B)
{
    std::vector<sf::Vector2f> pointsA = _getPoints(A);
    std::vector<sf::Vector2f> edgesA = _getPoints(A);

    std::vector<sf::Vector2f> pointsB = _getPoints(B);
    std::vector<sf::Vector2f> edgesB = _getPoints(B);

    auto dot = [&](const sf::Vector2f& a, const sf::Vector2f& b) -> float
    {
        return (a.x*b.x + a.y*b.y);
    };
    auto norm = [&](const sf::Vector2f& a) -> float
    {
        return sqrt(a.x*a.x + a.y*a.y);
    };
    auto dist = [&](const sf::Vector2f& a, const sf::Vector2f& b) -> float
    {
        float aux1 = a.x-b.x;
        float aux2 = a.y-b.y;
        return sqrt(aux1*aux1 + aux2*aux2);
    };
    auto normalized = [&](const sf::Vector2f& a) -> sf::Vector2f
    {
        float mag = norm(a);
        return (a/mag);
    };

    auto areEdgesParallel = [&](const sf::Vector2f& e1, const sf::Vector2f& e2) -> bool
    {
        if( abs( dot(normalized(e1),normalized(e2)) ) > 0.9f )
            return true;
    };

    auto minDistanceEdges = [&](const std::pair<sf::Vector2f, sf::Vector2f> &S1, const std::pair<sf::Vector2f, sf::Vector2f> &S2) -> float
    {
        // http://geomalgorithms.com/a07-_distance.html

        sf::Vector2f   u = S1.second - S1.first;
        sf::Vector2f   v = S2.second - S2.first;
        sf::Vector2f   w = S1.first - S2.first;
        float    a = dot(u,u);         // always >= 0
        float    b = dot(u,v);
        float    c = dot(v,v);         // always >= 0
        float    d = dot(u,w);
        float    e = dot(v,w);
        float    D = a*c - b*b;        // always >= 0
        float    sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
        float    tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

        // compute the line parameters of the two closest points
        if (D < SMALL_NUM)
        {					// the lines are almost parallel
            sN = 0.0;       // force using point P0 on segment S1
            sD = 1.0;       // to prevent possible division by 0.0 later
            tN = e;
            tD = c;
        }
        else // get the closest points on the infinite lines
        {                 
            sN = (b*e - c*d);
            tN = (a*e - b*d);
            if (sN < 0.0) // sc < 0 => the s=0 edge is visible
            {        
                sN = 0.0;
                tN = e;
                tD = c;
            }
            else if (sN > sD) // sc > 1  => the s=1 edge is visible
            {  
                sN = sD;
                tN = e + b;
                tD = c;
            }
        }

        if (tN < 0.0)
        {            // tc < 0 => the t=0 edge is visible
            tN = 0.0;
            // recompute sc for this edge
            if (-d < 0.0)
                sN = 0.0;
            else if (-d > a)
                sN = sD;
            else
            {
                sN = -d;
                sD = a;
            }
        }
        else if (tN > tD)
        {      // tc > 1  => the t=1 edge is visible
            tN = tD;
            // recompute sc for this edge
            if ((-d + b) < 0.0)
                sN = 0;
            else if ((-d + b) > a)
                sN = sD;
            else
            {
                sN = (-d +  b);
                sD = a;
            }
        }
        // finally do the division to get sc and tc
        sc = (abs(sN) < SMALL_NUM ? 0.0 : sN / sD);
        tc = (abs(tN) < SMALL_NUM ? 0.0 : tN / tD);

        // get the difference of the two closest points
        sf::Vector2f   dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)

        return norm(dP);   // return the closest distance
    };

    auto testCloseEdgePoints = [&](std::pair<sf::Vector2f, sf::Vector2f> &edgePoints1, std::pair<sf::Vector2f, sf::Vector2f> &edgePoints2, float minDist) -> bool
    {
        if( ( dist(edgePoints1.first, edgePoints2.first) < minDist ) && ( dist(edgePoints1.second, edgePoints2.second) < minDist ) )
            return true;
        if( ( dist(edgePoints1.second, edgePoints2.first) < minDist ) && ( dist(edgePoints1.first, edgePoints2.second) < minDist ) )
            return true;
        return false;
    };

    std::vector<PolygonAdjacency> conexions;


    // here the actual work
    for(unsigned int eA=0; eA < edgesA.size() ; ++eA )
        for(unsigned int eB=0; eB < edgesB.size() ; ++eB )
        {
            if( areEdgesParallel(edgesA[eA], edgesB[eB]) )
                if( minDistanceEdges(_getEdgePoints(eA, pointsA), _getEdgePoints(eB, pointsB)) < ADJACENT_DISTANCE )
                    if( testCloseEdgePoints(_getEdgePoints(eA, pointsA), _getEdgePoints(eB, pointsB), ADJACENT_DISTANCE ) )
                        conexions.push_back( PolygonAdjacency(&A, &B, eA, eB) );
                        
        }

    return conexions;
}