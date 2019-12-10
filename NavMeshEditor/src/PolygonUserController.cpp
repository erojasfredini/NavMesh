#include "PolygonUserController.h"

#include "NavMeshEditorApp.h"

#include <iostream>

PolygonUserController::PolygonUserController(unsigned int windowHeight): 
    _movingPolygon(false), _rotatingPolygon(false), _scalingPolygon(false), _selectedPolygon(NULL), _windowHeight(windowHeight), _curEditorState(Idle), _prevEditorState(Idle), _curElementState(Rectangle), _enabled(true)
{}

void PolygonUserController::processUserEvent(const sf::Event& event)
{
    if( !_enabled )
        return;

    switch(_curElementState)
    {
        case ElementType::Rectangle:
            _rectangleElementProcessEvent(event);
            break;

        case ElementType::ConcavePolygon:
            _concaveElementProcessEvent(event);
            break;

        case ElementType::ConvexPolygon:
            _convexElementProcessEvent(event);
            break;
    }

}

void PolygonUserController::setEnabled(bool state)
{
    _enabled = state;
}

void PolygonUserController::_rectangleElementProcessEvent(const sf::Event& event)
{
    switch(event.type)
    {
        case sf::Event::EventType::MouseMoved:
        {
            _prevMousePosition = _curMousePosition;
            _curMousePosition = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
            sf::Vector2i displacement = _curMousePosition - _prevMousePosition;
            displacement.y *= -1; 

            switch(_curEditorState)
            {
                case ControllerState::Editing:
                {
                    if( _movingPolygon )
                    {
                        _selectedPolygon->move(displacement);
                    }

                    if( _rotatingPolygon )
                    {
                        auto Vector2i2Vector2f = [](const sf::Vector2i& input)->sf::Vector2f {return sf::Vector2f(input.x, input.y);};
                        auto dot = [](const sf::Vector2f& a, const sf::Vector2f& b)->float {return (a.x*b.x+a.y*b.y);};
                        sf::Vector2f displacementF = Vector2i2Vector2f( displacement );
                        float rotation = dot(displacementF, sf::Vector2f(1.0f, 0.0f));

                        _selectedPolygon->rotate(rotation);
                    }

                    if( _scalingPolygon )
                    {
                        auto Vector2i2Vector2f = [](const sf::Vector2i& input)->sf::Vector2f {return sf::Vector2f(input.x, input.y);};
                        auto dot = [](const sf::Vector2f& a, const sf::Vector2f& b)->float {return (a.x*b.x+a.y*b.y);};
                        sf::Vector2f displacementF = Vector2i2Vector2f( _mousePressPoint - _curMousePosition );
                        float scale = abs( dot(displacementF, sf::Vector2f(1.0f, 0.0f)) );

                        _selectedPolygon->setSize(sf::Vector2f(scale, scale));
                    }
                }
                    break;

                case ControllerState::Creating:
                {
                    auto Vector2i2Vector2f = [](const sf::Vector2i& input)->sf::Vector2f {return sf::Vector2f(input.x, input.y);};
                    auto dot = [](const sf::Vector2f& a, const sf::Vector2f& b)->float {return (a.x*b.x+a.y*b.y);};
                    sf::Vector2f displacementF = Vector2i2Vector2f( _mousePressPoint - _curMousePosition );
                    float scale = abs( dot(displacementF, sf::Vector2f(1.0f, 0.0f)) );
                    
                    _selectedPolygon->setSize( sf::Vector2f(scale, scale) );
                }
                    break;
            }

        }
            break;

        case sf::Event::EventType::MouseButtonPressed:
        {
            _mousePressPoint = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
            _mousePressPoint.y = _windowHeight - _mousePressPoint.y;
            GeneralPolygon* pPoly = NULL;
            if( ( pPoly = NavMeshEditorApp::getInstance().peek(_mousePressPoint) ) != NULL )
            {
                _changeControllerState(ControllerState::Editing);
                _selectedPolygon = pPoly;

                //std::cerr<< "PEEK!!! position: x="<<_mousePressPoint.x<<"   "<<"y="<<_mousePressPoint.y<<std::endl;

                if( event.mouseButton.button == sf::Mouse::Button::Left )
                {
                    _movingPolygon = true;
                }

                if( event.mouseButton.button == sf::Mouse::Button::Right )
                {
                    _rotatingPolygon = true;
                }

                if( event.mouseButton.button == sf::Mouse::Button::Middle )
                {
                    _scalingPolygon = true;
                }
            }else
            {
                _changeControllerState(ControllerState::Creating);
                _selectedPolygon = new GeneralPolygon(_mousePressPoint, sf::Vector2f(1.0f,1.0f));
                _selectedPolygon->addRectangle(1, 1);

                NavMeshEditorApp::getInstance().addPolygon(*_selectedPolygon);
            }
        }
            break;

        case sf::Event::EventType::MouseButtonReleased:
        {
            _movingPolygon = false;
            _rotatingPolygon = false;
            _scalingPolygon = false;

            _selectedPolygon = NULL;

            _changeControllerState(ControllerState::Idle);
        }
            break;
    }
}

void PolygonUserController::_concaveElementProcessEvent(const sf::Event& event)
{}

void PolygonUserController::_convexElementProcessEvent(const sf::Event& event)
{}

void PolygonUserController::_changeControllerState(ControllerState state)
{
    _prevEditorState = _curEditorState;
    _curEditorState = state;

    // new state callbacks
    switch( _curEditorState )
    {
        case ControllerState::Idle:

            break;

        case ControllerState::Creating:

            break;

        case ControllerState::Editing:

            break;
    }

    // previous state callbacks
    switch( _prevEditorState )
    {
        case ControllerState::Idle:

            break;

        case ControllerState::Creating:

            break;

        case ControllerState::Editing:

            break;
    }
}

void PolygonUserController::setElementType(ElementType type)
{
    _curElementState = type;
}

PolygonUserController::ElementType PolygonUserController::getElementType()
{
    return _curElementState;
}