#pragma once

// para usar librerias dinamicas comentar el siguiente define
// y cambiar los nombres de las librerias contra la que linkeamos
#define SFML_STATIC

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <SFML/OpenGL.hpp>
#include <gl/glu.h> 

// used to creates the cast operator to ClipperLib::Polygons class for easy communication with clipper library
#include "..\Dependencies\clipper_ver5.1.6\clipper.hpp"

// used to tesselate the polygons for peeking and/or render
#include "..\Dependencies\polypartition\polypartition.h"

struct PolygonAdjacency
{
    PolygonAdjacency(TPPLPoly* _polygonA, TPPLPoly* _polygonB, unsigned int _polygonAEdge, unsigned int _polygonBEdge):
        polygonA(_polygonA), polygonB(_polygonB), polygonAEdge(_polygonAEdge), polygonBEdge(_polygonBEdge)
    {}

    PolygonAdjacency():
        polygonA(NULL), polygonB(NULL), polygonAEdge(0), polygonBEdge(0)
    {}

    unsigned int polygonAEdge;
    unsigned int polygonBEdge;

    TPPLPoly* polygonA;
    TPPLPoly* polygonB;
};


/// /brief A GeneralPolygon object could represent one polygon 2d or multiple polygons 2d, convex or concave and even with holes inside of them.
/// It is defined in base of the concept of contours that could be polygons of any type or holes.
/// It can render any of the polygons that can represent, tesselating the polygon/s to render with OpenGL.
class GeneralPolygon
{
    public:

        /// \brief Scale of Clipper. Needed because that library works only with integer for robustness so we have top convert from float to integer. Multiplies when casting to ClipperLib::Polygons and divides when creating GeneralPolygon from ClipperLib::Polygons
        static float clipperScaleCoeficient;

        ///////////////////////////////////////////////////////////////////////
        /// \brief The different render modes that GeneralPolygon can use.
        /// GLU_Tessalation uses GLU functions to tessalate and render 
        ///////////////////////////////////////////////////////////////////////
        enum RenderMethod {GLU_Tessalation, ConvexTessalation};

        ///////////////////////////////////////////////////////////////////////
        /// \brief Represent a Contour object of the general polygon
        ///////////////////////////////////////////////////////////////////////
        typedef std::vector<sf::Vector2f> Contour;

        operator ClipperLib::Polygons();

        operator list<TPPLPoly>();

        GeneralPolygon(const sf::Vector2i& position, const sf::Vector2f& size = sf::Vector2f(1.0f, 1.0f), float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

        GeneralPolygon(const sf::Vector2f& position = sf::Vector2f(0.0f, 0.0f), const sf::Vector2f& size = sf::Vector2f(1.0f, 1.0f), float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

        GeneralPolygon(const ClipperLib::Polygons& Polygons);

        GeneralPolygon(list<TPPLPoly>& Polygons);

        ~GeneralPolygon();

        ///////////////////////////////////////////////////////////////////////
        /// \brief Adds new contour to the Polygon. A contour can be or a polygon or a hole
        /// in a general polygon. Uses the winding rule of Even-Odd to identify actual polygons and holes. 
        /// Odds  are polygons and Evens are holes
        /// \param contour A contour object that is defined with points of the contour
        ///////////////////////////////////////////////////////////////////////
        void addContour(const Contour& contour);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Set the render mode. By default uses GLU_Tessalation
        /// \param mode The render mode
        ///////////////////////////////////////////////////////////////////////
        void setRenderMode(RenderMethod mode);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Renders with OpenGL the polygon. IMPORTANT: this implementation is not optimized
        ///////////////////////////////////////////////////////////////////////
        void render();

        ///////////////////////////////////////////////////////////////////////
        /// \brief Renders with OpenGL the polygon tesselation in convex polygons used internally to peek the polygon
        ///////////////////////////////////////////////////////////////////////
        void renderConvexTesselation();

        /////////////////////////////////////////////////////
        /// \brief Renders with OpenGL the polygons adyacency
        /////////////////////////////////////////////////////
        void renderConvexTesselationAdjacency();

        ///////////////////////////////////////////////////////////////////////
        /// \brief Moves the polygon
        /// \param movement The movement of the polygon
        ///////////////////////////////////////////////////////////////////////
        void move(const sf::Vector2f& movement);
        void move(const sf::Vector2i& movement);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Sets the position of the polygon
        /// \param position The position of the polygon
        ///////////////////////////////////////////////////////////////////////
        void setPosition(const sf::Vector2f& position);
        void setPosition(const sf::Vector2i& position);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Gets the position of the object
        /// \return the position of the object
        ///////////////////////////////////////////////////////////////////////
        sf::Vector2f getPosition();

        ///////////////////////////////////////////////////////////////////////
        /// \brief Rotetes the polygon. The rotation is counter-clockwise
        /// \param degree The degrees of rotation of the polygon
        ///////////////////////////////////////////////////////////////////////
        void rotate(float degree);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Set the angle of rotation of the polygon. The rotation is counter-clockwise
        /// \param degree The degrees of rotation of the polygon
        ///////////////////////////////////////////////////////////////////////
        void setRotation(float degree);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Gets the angle of rotation of the object in degrees
        /// \return the angle of rotation of the object
        ///////////////////////////////////////////////////////////////////////
        float getRotation();

        ///////////////////////////////////////////////////////////////////////
        /// \brief Set the size or scale of the polygon. If the scale is (1,1) then the 
        /// size of the polygon is determined directly with the coordinates of the contour points
        /// \param size The scale of the polygon
        ///////////////////////////////////////////////////////////////////////
        void setSize(const sf::Vector2f& size);
        void setSize(const sf::Vector2i& size);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Gets the size or scale of the directly
        /// \return the scale of the polygon
        ///////////////////////////////////////////////////////////////////////
        const sf::Vector2f& getSize();

        ///////////////////////////////////////////////////////////////////////
        /// \brief Set the color to render the polygon. All the polygon will have the same color
        /// \param r The red component of the color
        /// \param g The green component of the color
        /// \param b The blue component of the color
        /// \param a The alpha component of the color
        ///////////////////////////////////////////////////////////////////////
        void setColor(float r, float g, float b, float a);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Get the color in witch is rendered all the polygons
        /// \param r The red component of the color
        /// \param g The green component of the color
        /// \param b The blue component of the color
        /// \param a The alpha component of the color
        ///////////////////////////////////////////////////////////////////////
        void getColor(float& r, float& g, float& b, float& a);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Gets all the conexions of tha adyacent polygon in ther 
        /// general polygon.
        /// \return the conexions on the general polygon
        ///////////////////////////////////////////////////////////////////////
        std::vector<PolygonAdjacency> GetAllConexions();

        bool peek(const sf::Vector2f& click);
        bool peek(const sf::Vector2i& click);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Adds a contour to the general polygon with equilateral triangular shape. The added
        /// polygon is normalized around the origin by default
        /// \param scaleX The scale in x coordinate of the normalized triangle
        /// \param scaleY The scale in y coordinate of the normalized triangle
        /// \param displace The displacement from the origin of the triangular shape
        ///////////////////////////////////////////////////////////////////////
        void addEquilateralTriangle(float scaleX = 1.0f, float scaleY = 1.0f, const sf::Vector2f& displace = sf::Vector2f(0.0f, 0.0f) );

        ///////////////////////////////////////////////////////////////////////
        /// \brief Adds a contour to the general polygon with rectangular shape. The added
        /// polygon is normalized around the origin by default
        /// \param scaleX The scale in x coordinate of the normalized rectangle
        /// \param scaleY The scale in y coordinate of the normalized rectangle
        /// \param displace The displacement from the origin of the rectangle shape
        ///////////////////////////////////////////////////////////////////////
        void addRectangle(float scaleX = 1.0f, float scaleY = 1.0f, const sf::Vector2f& displace = sf::Vector2f(0.0f, 0.0f));

        ///////////////////////////////////////////////////////////////////////
        /// \brief Adds a contour to the general polygon with pentagram shape. The added
        /// polygon is normalized around the origin by default
        /// \param scaleX The scale in x coordinate of the normalized pentagram
        /// \param scaleY The scale in y coordinate of the normalized pentagram
        /// \param displace The displacement from the origin of the pentagram shape
        ///////////////////////////////////////////////////////////////////////
        void addPentagram(float scaleX = 1.0f, float scaleY = 1.0f, const sf::Vector2f& displace = sf::Vector2f(0.0f, 0.0f));

        ///////////////////////////////////////////////////////////////////////
        /// \brief Adds a contour to the general polygon with the leter E shape. The added
        /// polygon is normalized around the origin by default
        /// \param scaleX The scale in x coordinate of the normalized E
        /// \param scaleY The scale in y coordinate of the normalized E
        /// \param displace The displacement from the origin of the E shape
        ///////////////////////////////////////////////////////////////////////
        void addLetterE(float scaleX = 1.0f, float scaleY = 1.0f, const sf::Vector2f& displace = sf::Vector2f(0.0f, 0.0f));

    private:

        /// \brief The current render mode set
        RenderMethod _renderMode;

        /// \brief Creates and adds to the variable _combineNewVertexData a new point added in the tesselation (combined) of the polygon added (combined)
        GLdouble* _newCombineVertexData(GLdouble x, GLdouble y);

        /// \brief Clear the points of the polygon added (combined) after the tesselation to render
        void _clearCombineVertexData();

        /// \brief Maintains the added (combined) points of the polygon after the tesselation to render
        std::vector<GLdouble*> _combineNewVertexData;

        /// \brief GLU tesselator object. Only created if render at least once
        GLUtesselator* _gluTesselator;

        /// \brief The position of the polygon
        sf::Vector2f _position;

        /// \brief The rotation of the polygon
        float _rotation;

        /// \brief The scale or size of the polygon
        sf::Vector2f _size;

        /// \brief The color to render the polygon
        float _r, _g, _b, _a;

        /// \brief The winding rule of the polygon render
        GLdouble _windingRule;

        /// \brief Maintains the contours of the general polygon
        std::vector<Contour> _contours;

        /// \brief Updates the _optimalConvexTesselation convex polygon tessaletion array. Should be called always after adding a new contour, ie with addContour, or peeking will no longer work
        void _updateOptimalConvexTesselation();

        /// \brief Convex polygon tessaletion array
        list<TPPLPoly> _optimalConvexTesselation;

        ///////////////////////////////////////////////////////////////////////
        /// \brief Gets the conexions between two polygons of the general polygon.
        /// \param A The first polygon
        /// \param B The second polygon
        /// \return the conexions
        ///////////////////////////////////////////////////////////////////////
        std::vector<PolygonAdjacency> _getConexions(TPPLPoly& A, TPPLPoly& B);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Implementation of the Even-Odd rule algorithm for testing if a point
        /// is inside a polygon. Particularly this implementation only return a boolean saying if it is inside the polygon
        /// \param testPoint The test point from where we will test if it is inside the polygon
        /// \param polygon The polygon to take into account
        /// \return true if testPoint is inside the polygon, false if it is outside
        ///////////////////////////////////////////////////////////////////////
        bool _evenOddRuleAlgorithm(const sf::Vector2f& testPoint, const Contour & polygon);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Implementation of the Even-Odd rule algorithm for testing if a point
        /// is inside a polygon when using Even-Odd filling rule. Particularly this implementation only
        /// return a boolean saying if it is inside the polygon. This method could be used to initialize correctly 
        /// the contours as holes or polygons and to test if apoint is inside a polygon
        /// \param testPoint The test point from where we will test if it is inside the polygon
        /// \param arrayToTest The array of polygons to take into account
        /// \return true if testPoint is inside the polygon, false if it is outside (really outside or in a hole)
        ///////////////////////////////////////////////////////////////////////
        bool _evenOddRuleAlgorithm(const sf::Vector2f& testPoint, const std::vector<Contour> & arrayToTest);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Implementation of the Even-Odd rule algorithm for testing if a point
        /// is inside a polygon when using Even-Odd filling rule. Particularly this implementation only
        /// return a boolean saying if it is inside the polygon. This method could be used to initialize correctly 
        /// the contours as holes or polygons and to test if apoint is inside a polygon.
        /// This method test with all the contours of the GeneralPolygon
        /// \param testPoint The test point from where we will test if it is inside the polygon
        /// \return true if testPoint is inside the polygon, false if it is outside (really outside or in a hole)
        ///////////////////////////////////////////////////////////////////////
        bool _evenOddRuleAlgorithm(const sf::Vector2f& testPoint);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Transform a point from local polygon space to the world space determined with the polygon position, rotation and scale
        /// \param localPosition The position in local space to transform
        /// \return the point after transformation
        ///////////////////////////////////////////////////////////////////////
        sf::Vector2f _transformFromLocalToWorld(const sf::Vector2f& localPosition);

        ///////////////////////////////////////////////////////////////////////
        /// \brief Transform a point from world space to the local polygon space determined with the polygon position, rotation and scale
        /// \param worldPosition The position in world space to transform
        /// \return the point after transformation
        ///////////////////////////////////////////////////////////////////////
        sf::Vector2f _transformFromWorldToLocal(const sf::Vector2f& worldPosition);

        ///////////////////////////////////////////////////////////////////////
        /// \brief The winding order of a polygon or contour
        ///////////////////////////////////////////////////////////////////////
        enum PolygonWindingOrder {Winding_None, Winding_CCW, Winding_CW};

        ///////////////////////////////////////////////////////////////////////
        /// \brief Tells if a polygon is in CW winding order, CCW or none
        /// \param polygon A polygon defined by points
        /// \return PolygonWindingOrder type telling the winding order
        ///////////////////////////////////////////////////////////////////////
        PolygonWindingOrder _getWinding(const Contour& polygon);
};