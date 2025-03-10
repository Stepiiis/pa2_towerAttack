#pragma once
#include "constants.h"
#include "Map.h"
#include "ncurses.h"
#include "HelpStructs.h"
#include <iostream>
#include <memory>
#include <set>

class Map;

class Entity {
public:
    /** virtual destructor because Entity is abstract*/
    virtual ~Entity() = default;

    /** Entity constructur which assigns shared variables provided by derived classes.
     * @param posX position x to be spawned at
     * @param posY position y to be spawned at
     * @param hp amount of HP to be spawned with
     * @param dmg amount of damage that an attacker gives
     * @param symbol symbol to be printed on the map when needed
     * @param radius radius in which other entities can be attacked
     * @param attackSpeed speed at which an attack can be given to other entities (lower is faster)
     * @param map pointer to a shared entity of Map class.
     * @param attackerID id of attacker which will be set onto the map for others to see
     * */
    Entity(int posX, int posY,int hp, int dmg, char symbol, int radius, int attackSpeed, Map * map,int attackerID);
    /** Moves the entity to the given position on screen, actual showing on screen is handled by Map class.
     * @param x position x to move to
     * @param y position y to move to
     */
    bool move(int x, int y);

    /** @returns If the entity is alive, it will take damage and return true, otherwise it will return false and set status alive to false*/
    bool takeDamage(int damage);

    /** Removes the entity from the map and sets alive to false @returns true if destroyed correctly, otherwise false. */
    bool destroy();

    /** sets m_radius to different value specified by parameter given. @param radius - new radius*/
    void setRadius(int radius);

    /** getter for radius @returns m_radius */
    int getRadius()const;

    /** draws the entity on the map at current coordinates of entity. */
    void draw();

    /** @returns m_damage - amount of damage that it can give to other attackers*/
    int getDamage() const;

    /** @returns m_hp - current amount of HP */
    int getHP() const;

    /** @returns id of entity on which it is currently focusing and wants to attack*/
    int getCurrentFocus() const;

    /** @returns symbol which will be printed onto the map*/
    char getSymbol() const;

    /** @returns true if it is focused on any entity and if attackSpeed allows it. Otherwise false.  */
    bool canAttack() ;

    /** @returns type of entity (Attacker/Tower). */
    virtual Point::PointType getType() = 0;

    /** checks radius of entity for enemies.
     * @returns true if an entitty was found in the perimeter. otherwise returns false. */
    bool checkRadius(Point::PointType type); // checks radius of the entity and returns true if agiven entity type is in the radius

    /** Calculates deltas of coordinates for given radius of reach. Has to be done on instantiation and can be recalculated at any time.\n
     * radius 1 deltas\n
     *  x\n
     * xox\n
     *  x\n
     * radius 2 deltas\n
     *   x\n
     *  xxx\n
     * xxoxx\n
     *  xxx\n
     *   x\n
     * radius 3 deltas\n
     *    x\n
     *   xxx\n
     *  xxxxx\n
     * xxxoxxx\n
     *  xxxxx\n
     *   xxx\n
     *    x\n
     */
    void calculateDeltas();

    /** @returns set of pairs of coordinates representing deltas for current reach*/
    [[nodiscard]] const std::set<std::pair<int,int>> &getDeltas() const;

    /** @returns typeName. Used as polymorphic function for saving entity into a savefile. */
    virtual std::string getTypeName() const = 0;  // returns name of type, used for saves

    /** @returns ID which corresponds to the entity. */
    int getID()const;

    /** @returns pair of positions at which the entity currently stands on.*/
    [[nodiscard]] std::pair<int,int> getPosition()const;

    bool alive = true;
protected:
    int m_x;       // souřadnice _x

    int m_y;       // -//- y

    char m_symbol{};    // symbol identifikující typ entity

    int m_id;    // identifikator dane postavy

    int m_hp;      // současné zdraví

    Map* m_sharedMap;   // pointer na sdilenou mapu.
                        // Mapa vždy přežije všechny entity.
    std::set<std::pair<int,int>> m_deltas;
    int m_currentFocusID=-10;// ID of an entity that we are currently attacking
    bool m_hasFocus = false;

    int m_damage;

    int m_attackSpeed = 1;
    int m_attackClock = 0;

    int m_radius{};
};