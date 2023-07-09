#include "scanline_triangulation.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <SFML/Graphics.hpp>

using namespace std;

auto GetPointsFromStdin() {
    std::vector<geometry::Vector2D> points;

    std::string line;
    std::ifstream in;
    in.open("1.txt");
    // Считывание пар точек до EOF
    double x, y;
    while (in >> x && (in >> y)) {
        points.emplace_back(x, y);
    }
    return points;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "SFML Rectangle Example"); // Создаем окно размером 1920x1080 пикселей с заголовком "SFML Rectangle Example"

    auto points = GetPointsFromStdin();
    const auto builder = geometry::DelaunayBuilder::Create(std::move(points));
    const auto& triangulation = builder->Get();
    

    while (window.isOpen()) // Главный цикл приложения
    {
        sf::Event event;
        while (window.pollEvent(event)) // Обработка событий
        {
            if (event.type == sf::Event::Closed) // Если событие - закрытие окна
                window.close(); // Закрываем окно
        }

        window.clear(); // Очищаем окно

        for (auto item : triangulation.points) {
            sf::Vertex point(sf::Vector2f(item.x, item.y), sf::Color::White);
            window.draw(&point, 1, sf::Points);
        }

        for (auto item : triangulation.graph) {
            if (item.first.v1 > -1 && item.first.v2 > -1) {
                sf::Vertex line[] =
                {
                 sf::Vertex(sf::Vector2f(triangulation.points[item.first.v1].x, triangulation.points[item.first.v1].y)),
                 sf::Vertex(sf::Vector2f(triangulation.points[item.first.v2].x,triangulation.points[item.first.v2].y))
                };

                window.draw(line, 2, sf::Lines);
            }

           
        }
        
        window.display(); // Отображаем содержимое окна
    }
   

    return 0;
}