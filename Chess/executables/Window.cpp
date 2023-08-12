#pragma once

#include "../headers/Window.h"

void Window::draw(const sf::Sprite& sprite)
{
    m_window.draw(sprite);
}

void Window::draw(const sf::RectangleShape& shape)
{
    m_window.draw(shape);
}

sf::RenderWindow& Window::getWindow()
{
    return m_window;
}

void Window::close()
{
    m_window.close();
}

void Window::clear()
{
    m_window.clear();
}

void Window::display()
{
    m_window.display();
}

bool Window::isOpen() const
{
    return m_window.isOpen();
}

bool Window::checkIfWasClosed(sf::Event& e)
{
    if (e.type == sf::Event::Closed ||
        e.type == sf::Event::KeyPressed
        && e.key.code == sf::Keyboard::Escape)
    {
        m_window.close();
        return true;
    }

    return false;
}

bool Window::waitEvent(sf::Event& e)
{
    return m_window.waitEvent(e);
}