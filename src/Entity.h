#pragma once
#include "constants.h"
#include "Map.h"
#include <iostream>
#include <memory>
#include <set>
#include "ncurses.h"

class Map;

class Entity {
public:
    virtual ~Entity() = default;
    Entity(int posX, int posY, defEntity def, Map * map,int attackerID);

    // moves the entity to the given position on screen, actual showing on screen is handled by Map class
    bool move(int x, int y);

    // if the entity is alive, it will take damage and return true, otherwise it will return false
    bool takeDamage(int damage);
    // gives damage to targeted entity and if the entity dies it returns false, otherwise it returns true.
    // if the entity dies it will be removed from the map
    bool giveDamage(int damage, Entity* target);

    // removes the entity from the map and sets alive to false
    bool destroy();
    void attack();
    void update();


    void draw();
    int getDamage() const;
    int getHP() const;
    size_t getCurrentFocus();
    char getSymbol() const;
    virtual Point::PointType getType() = 0;
    bool checkRadius(Point::PointType type);
    void calculateDeltas();
    size_t getID()const;
    [[nodiscard]] std::pair<int,int> getPosition()const;

    bool printEntity(int x, int y);
    bool alive = true;
protected:
    int m_x;       // souřadnice _x

    int m_y;       // -//- y

    char m_symbol{};    // symbol identifikující typ entity

    size_t m_id;    // identifikator dane postavy

    int m_hp;      // současné zdraví

    int m_maxhp;   // maximální zdraví

    Map* m_sharedMap;   // pointer na sdilenou mapu.
                        // Mapa vždy přežije všechny entity.
    std::set<std::pair<int,int>> m_deltas;
    size_t m_currentFocusID=-10;// ID of an entity that we are currently attacking
    bool hasFocus = false;
    defEntity m_def;

    int m_damage;

    int m_speed{};

    int m_radius{};
};