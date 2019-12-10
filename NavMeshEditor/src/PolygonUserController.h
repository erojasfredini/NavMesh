#pragma once

// para usar librerias dinamicas comentar el siguiente define
// y cambiar los nombres de las librerias contra la que linkeamos
#define SFML_STATIC

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "GeneralPolygon.h"

class PolygonUserController
{
    private:

        enum ElementType {Rectangle, ConcavePolygon, ConvexPolygon};

        ElementType _curElementState;

        bool _enabled;

        enum ControllerState {Idle, Creating, Editing};

        ControllerState _curEditorState;

        ControllerState _prevEditorState;

        unsigned int _windowHeight;

        bool _movingPolygon;
        
        bool _rotatingPolygon;
        
        bool _scalingPolygon;

        sf::Vector2i _mousePressPoint;

        sf::Vector2i _prevMousePosition;

        sf::Vector2i _curMousePosition;

        GeneralPolygon* _selectedPolygon;

        void _rectangleElementProcessEvent(const sf::Event& event);

        void _concaveElementProcessEvent(const sf::Event& event);

        void _convexElementProcessEvent(const sf::Event& event);

        void _changeControllerState(ControllerState state);

    public:

        PolygonUserController(unsigned int windowHeight);

        void setEnabled(bool state);

        void processUserEvent(const sf::Event& event);

        void setElementType(ElementType type);

        ElementType getElementType();
};