#pragma once
#include <iostream>
#include <fstream>
#include <memory>
#include "logFile.h"
#include "Entity.h"
#include "ncurses.h"
#include <complex>
#include "Map.h"


Point::Point(int x, int y, char symbol)
        : _symbol(symbol), _x(x), _y(y),  _defaultSymbol(symbol)
{
    switch (symbol) {
        case '#':
            _type = Wall;
            break;
        case ' ':
            _type = Empty;
            break;
        case '@':
            _type = Attacker;
            break;
        case 'I':
            _type = Tower;
            break;
        case '<':
            _type = Entry;
            break;
        case '=':
            if(x == 1)
                _type = Exit;
            else
                _type = Entry;
            _type = Entry;
            break;
        case '>':
            _type = Exit;
            break;
        case 'o':
            _type = Water;
            break;
        case '0':
            _type = Water;
            break;
    }
    _defaultType = _type;
    _ident = -10;
}
// helper constructor for ForEachNeighbour function
Point::Point(int x, int y) {
    this->_x = x;
    this->_y = y;
    _type = Empty;
    _symbol = ' ';
    _ident = -10;

}

bool Point::operator<(const Point &rhs) const {
    return std::tie(_x, _y) < std::tie(rhs._x, rhs._y);
}

bool Point::operator!=(const Point &rhs) const {
    return _x != rhs._x || _y != rhs._y;
}

bool Point::operator==(const Point&rhs) const {
    return _x == rhs._x && _y == rhs._y;
}
//
//Point::Point(const Point & src) {
//    _x=src._x;
//    _y=src._y;
//    _type=src._type;
//    _defaultType=src._defaultType;
//    _symbol = src._symbol;
//    _defaultSymbol = src._defaultSymbol;
//    _IDent=src._IDent;
//}

//Point &Point::operator=(const Point &rhs);
//Point &Point::operator=(const Point &rhs);

Point::Point(int x, int y, PointType type)
: _x(x), _y(y), _type(type)
{
}



bool Map::updateMap( int x, int y, Entity * entity)
{
    if(y>m_map.size() || (y<0) || (x>m_map[0].size()) || (x<0))
    {
        throw(mapException("Out of bounds"));
    }
    auto prev = entity->getPosition();
    revertCell(prev.first, prev.second);
    char symbol = entity->getSymbol();
    if(entity->getType() == Point::Tower)
    {
        auto deltas = entity->getDeltas();
        for(auto [dx,dy] : deltas) {
            if(checkBounds(prev.first+dx,prev.second+dy)) {
                if (m_map[prev.second + dy][prev.first + dx]._type == Point::Empty || m_map[prev.second + dy][prev.first + dx]._type == Point::Attacker) {
                    removeFromRadius(prev.first + dx, prev.second+dy, entity);
                }
            }
        }
        for(auto [dx,dy] : deltas) {
            if(checkBounds(x+dx,y+dy)) {
                if (m_map[y + dy][x + dx]._type == Point::Empty || m_map[y + dy][x + dx]._type == Point::Attacker) {
                    addToRadius(x + dx, y, entity);
                }
            }
        }
    }
    entity->move(x,y);
    m_map[y][x]._type = entity->getType();
    m_map[y][x]._symbol = symbol;
    m_map[y][x]._ident = entity->getID();
    mvwprintw(m_game_window,y+1,x+1,"%c",symbol);
    return true;
}

bool Map::setEntity(int x, int y, Entity * entity){
    if(y>m_map.size() || (y<0) || (x>m_map[0].size()) || (x<0))
    {
        throw(mapException("Out of bounds"));
    }
    m_map[y][x]._type = entity->getType();
    m_map[y][x]._symbol = entity->getSymbol();
    m_map[y][x]._ident = entity->getID();
    if(entity->getType() == Point::Tower) {
        int radius = entity->getRadius();
        entity->setRadius(abs(radius - 2)); // temporarily setting radius to be little smaller
        entity->calculateDeltas();     // so that fast attackers have to go in their range
        entity->setRadius(radius);
        for(auto [dx,dy] : entity->getDeltas()) {
            if(checkBounds(x+dx,y+dy)) {
                if (getType({x+dx,y+dy}) == Point::Empty || getType({x+dx,y+dy}) == Point::Attacker) {
                    addToRadius(x + dx, y + dy, entity);
                }
            }
        }
        entity->calculateDeltas();
    }
    entity->move(x,y);
    mvwprintw(m_game_window,y+1,x+1,"%c",entity->getSymbol());
    return true;
}

bool Map::loadNextMap(int level){
    return   readMap(level);
}

void Map::refreshWindow(){
    if(m_game_window != nullptr)
        wrefresh(m_game_window);
}


// loads map from assets files and sets up the map into a 2D array
bool Map::readMap(int level) {
    m_map.clear();
    std::string mapLocation = "assets/maps/map" + std::to_string(level) + ".txt";
    std::ifstream mapFile(mapLocation, std::ios::in);
    if(!mapFile.is_open()) {
        std::cout << "Could not open map file :" << mapLocation << std::endl;
        try {
            sendToLogFile(10, "Map::readMap: Could not open map file: " + mapLocation, "Map.cpp");
        } catch (logException &e) {
            std::cout << e.what() << std::endl;
        }
        return false;
    }
    std::string line;
    std::vector <Point> mapLines;
    auto it = m_map.begin();
    int y = 0;
    while(getline(mapFile, line)) {
        if(line.length() > 0) {
            for(size_t x = 0; x<=line.length(); x++) {
                mapLines.emplace_back(x,y,line[x]);
            }
            m_map.push_back(mapLines);
            mapLines.clear();
            y++;
        }
    }
//    std::string mapa;
//    printMap(mapa);
//    std::cout << mapa<< std::endl;
    mapFile.close();
    sendToLogFile(0, "Map::readMap: Map loaded successfully", "Map.cpp");
    return true;
}


// Prints the map to the screen and also checks for inputs and exits from the map
void Map::printMap(){
    m_entries.clear();
    for( auto& line : m_map) {
        for ( auto & point: line) {
            mvwprintw(m_game_window, point._y + 1, point._x + 1, "%c", point._symbol);
            if(point._x == 0 || point._x == 1){
                if(point._symbol == '<'){
                    m_exit = point;
                    point._type = Point::Exit;
                    point._defaultType = Point::Exit;
                }else if(point._symbol == '='){
                    point._type = Point::Exit;
                    point._defaultType = Point::Exit;
                }
            }
            else if(point._x == line.back()._x - 3 || point._x == line.back()._x - 2){
                if(point._symbol == '='){
                    m_entries.emplace_back(point._x, point._y, point._symbol);
                    point._type = Point::Entry;
                    point._defaultType = Point::Entry;
                }else if(point._symbol == '<'){
                    point._type = Point::Entry;
                    point._defaultType = Point::Entry;
                }
            }
        }
    }
    wrefresh(m_game_window);
}
bool Map::getLaneByID(int id, Point & spawnLane)const{
    if(m_entries.size()<id || id<0)
        return false;
    spawnLane = m_entries.at(id-1);
    spawnLane._x = spawnLane._x - 1;
    return true;
}

void Map::highlightLane(int lanenr){
    auto style = A_STANDOUT;
    for(int i = 0; i<m_entries.size(); i++){
        if(i==lanenr){
            wattron(m_game_window, style);
            mvwprintw(m_game_window, m_entries[i]._y + 1, m_entries[i]._x + 0, "%c", '<');
            mvwprintw(m_game_window, m_entries[i]._y + 1, m_entries[i]._x + 1, "%c", '=');
            mvwprintw(m_game_window, m_entries[i]._y + 1, m_entries[i]._x + 2, "%d", i + 1);
            wattroff(m_game_window, style);
            wrefresh(m_game_window);
        }else{
            mvwprintw(m_game_window, m_entries[i]._y + 1, m_entries[i]._x + 0, "%c", '<');
            mvwprintw(m_game_window, m_entries[i]._y + 1, m_entries[i]._x + 1, "%c", '=');
            mvwprintw(m_game_window, m_entries[i]._y + 1, m_entries[i]._x + 2, "%d", i + 1);
            wrefresh(m_game_window);
        }
    }
}


bool Map::checkNeighbours(int x, int y){
    if(x > 0 && x < MAP_WIDTH - 1 && y > 0 && y < MAP_HEIGHT - 1){
        if(m_map[y][x - 1]._type == Point::Empty && m_map[y][x + 1]._type == Point::Empty && m_map[y - 1][x]._type == Point::Empty && m_map[y + 1][x]._type == Point::Empty){
            if(m_map[y-1][x - 1]._type == Point::Empty && m_map[y - 1][x + 1]._type == Point::Empty && m_map[y - 1][x + 1]._type == Point::Empty && m_map[y + 1][x - 1]._type == Point::Empty){
                return true;
            }
        }
    }
    return false;
}

void Map::setWindow(WINDOW *win) {
    m_game_window=win;
}

void Map::forEachNeighborImpl(const Point &p, const Map::Callback& fun) {
    size_t x, y;
    for (auto [xd, yd] : { std::pair<int, int>{-1,0}, {0,-1}, {1, 0}, {0, 1} }) {
        x = p._x + xd;
        y = p._y + yd;
        if (y < 0 || y >= m_map.size()) continue;
        auto& row = m_map[y];
        if (x < 0 || x >= row.size()) continue;
        fun(m_map[y][x]);
    }
}

Point::PointType Map::getType(const Point& p) {
    return m_map[p._y][p._x]._type;
}

Point Map::getMapExit() {
    return m_exit;
}

bool Map::updateCell(int x, int y, Point::PointType type, const char symbol) {
    if (x < 0 || x >= (int)m_map[y].size()) throw mapException("Invalid _x coordinate: updateCell");
    if (y < 0 || y >= (int)m_map.size()) throw mapException("Invalid y coordinate: updateCell");
    m_map[y][x]._type = type;
    m_map[y][x]._symbol = symbol;
    m_map[y][x]._ident = -10;
    mvwprintw(m_game_window,y+1,x+1,"%c",symbol);
    return true;
}
// throws mapException if coordinate is out of bounds of map
bool Map::revertCell(int x, int y) {
    if (x < 0 || x >= (int)m_map[y].size()) throw mapException("Invalid _x coordinate: updateCell");
    if (y < 0 || y >= (int)m_map.size()) throw mapException("Invalid y coordinate: updateCell");
    m_map[y][x]._type = m_map[y][x]._defaultType;
    m_map[y][x]._symbol = m_map[y][x]._defaultSymbol;
    m_map[y][x]._ident = -10;
    mvwprintw(m_game_window,y+1,x+1,"%c",m_map[y][x]._defaultSymbol);
    return true;
}

void Map::clearMap() {
//    for(auto &row : m_map) {
//       row.clear();
//    }
    m_map.clear();
}

int Map::getMapWidth() {
    int width = 0;
    for(auto &row : m_map) {
        if(row.size() > width) {
            width = (int)row.size();
        }
    }
    return width;
}
//I tried implementing the bresenham algoroithm but i dont understand it fully so tried my own version
//bool Map::checkClearSight(std::pair<int,int>& p1, std::pair<int,int>& p2) {
//    int x1 = p1.first;
//    int y1 = p1.second;
//    int x2 = p2.first;
//    int y2 = p2.second;
//
//    int dx = abs(x2 - x1); // x vzdalenost dvou bodu
//    int dy = abs(y2 - y1); // y vzdalenost dvou bodu
//    int sx = x1 < x2 ? 1 : -1; // smer x
//    int sy = y1 < y2 ? 1 : -1; // smer y
//    int err = dx - dy; // err threshold
//    while (true) {
//        if (x1 == x2 && y1 == y2) { // došli jsme do konce v pohodě
//            return true;
//        }
//        if(m_map[y1][x1]._type != Point::Empty) { // v cestě je překážka
//            return false;
//        }
//        int e2 = 2 * err;
//        if (e2 > -dy) {
//            err -= dy;
//            x1 += sx;
//        }
//        if (e2 < dx) {
//            err += dx;
//            y1 += sy;
//        }
//    }
//}



bool Map::checkClearSight(const std::pair<int,int>& p1, const std::pair<int,int>& p2) const{

    int x1 = p1.first; // aktualni pozice "cesty"
    int y1 = p1.second; // aktualni pozice "cesty"
    int x2 = p2.first;
    int y2 = p2.second;
    int dx; // x vzdalenost dvou bodu
    int dy; // y vzdalenost dvou bodu
    double incl;
    while(true) {
        if(x1 == x2 && y1 == y2) { // došli jsme do konce v pohodě
            return true;
        }
        if(m_map[y1][x1]._type == Point::Wall) { // v cestě je překážka
            return false;
        }
        dx = abs(x2 - x1); // x vzdalenost dvou bodu
        dy = abs(y2 - y1); // y vzdalenost dvou bodu

        int sx = x1 < x2 ? 1 : -1; // smer x
        int sy = y1 < y2 ? 1 : -1; // smer y

        if (dy == 0) {
            x1 += sx;
            continue;
        } else if (dx == 0) {
            y1 += sy;
            continue;
        }

        incl = (atan2(dy, dx) * 180) / PI; // úhel svíraný mezi bodem a p2
        bool posuny = incl > 45; // posun x
        bool posunx = incl < 45;
//        bool half = (incl - 45) <= EPSILON ;

        x1 += posunx ? sx : 0;
        y1 += posuny ? sy : 0;

    }
    return false;
}

void Map::removeFromRadius(int x, int y, Entity *entity) {
    if(m_map[y][x]._inRadiusOf.count(entity->getType()) == 1) {
        if(m_map[y][x]._inRadiusOf[entity->getType()].count(entity->getID()) == 1) {
            m_map[y][x]._inRadiusOf[entity->getType()].erase(entity->getID());
            if(m_map[y][x]._inRadiusOf[entity->getType()].empty()) {
                m_map[y][x]._inRadiusOf.erase(entity->getType());
            }
        }
    }
}

void Map::addToRadius(int x, int y, Entity * entity) {
if(m_map[y][x]._inRadiusOf.count(entity->getType()) == 0) {
        std::map<int,bool> temp;
        temp.insert(std::pair<int,bool>(entity->getID(),true));
        m_map[y][x]._inRadiusOf.emplace(entity->getType(),temp);
}else{
        m_map[y][x]._inRadiusOf[entity->getType()].insert(std::pair<int,bool>(entity->getID(),true));
    }
}

bool Map::checkBounds(int x, int y) {
    return x >= 0 && x < getMapWidth() && y >= 0 && y < m_map.size();
}

const Point &Map::getExit() const {
    return m_exit;
}
