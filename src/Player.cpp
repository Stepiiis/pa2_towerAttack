#include "Player.h"



Player::Player(Map *map, defEntity def) {
    m_def = std::move(def);
    m_map = map;
    m_spawnLane._x = -10;
    m_spawnLane._y = -10;
    m_attackerID = 0;
}


void Player::setLane(int lane) {
    if (!m_map->getLaneByID(lane+1, m_spawnLane)) {
        m_spawnLane._x = -10;
        m_spawnLane._y = -10;
    }else{
        m_map->highlightLane(lane);
    }
}



void Player::setCoins(int coins) {
    m_coins = coins;
}

int Player::getCoins() const {
    return m_coins;
}


std::string Player::spawnAttacker() {
    std::pair<std::string, Point> attacker = m_attackersQueue.front();
    auto point = attacker.second;
    if (point._x == -10)
        return {};
    if (m_map->getType(point) != Point::Entry)
        throw logException("Player::spawnAttacker: point is not an entry");

    switch (m_def[attacker.first]["symbol"]) { // chooses on the basis of different ascii values
        case 36: // ascii value of $
            m_attackers.emplace(m_attackerID,
                                std::make_unique<basicAttacker>(attacker.second._x, attacker.second._y, m_def, m_map,
                                                                m_attackerID));
            break;
        case 37: // %
            m_attackers.emplace(m_attackerID,
                                std::make_unique<fastAttacker>(attacker.second._x, attacker.second._y, m_def, m_map,
                                                               m_attackerID));
            break;
        case 64: // @
            m_attackers.emplace(m_attackerID,
                                std::make_unique<chargerAttacker>(attacker.second._x, attacker.second._y, m_def, m_map,
                                                                  m_attackerID));
            break;
        default:
            throw logException("Player::spawnAttacker: unknown attacker type");
    }
    if (!m_attackers[m_attackerID]->findShortestPath()) {
        m_attackers.erase(m_attackerID);
        m_coins += m_def[attacker.first]["price"];
        m_attackersQueue.pop_front();
        return "Attacker in queue would not be able to reach end.";
    }
    m_attackerID++;
    m_attackersQueue.pop_front();
    return {};
}

void Player::setAttackerType(int type) {
    switch (type) {
        case 0:
            m_attackerType = BASICA;
            break;
        case 1:
            m_attackerType = CHARGERA;
            break;
        case 2:
            m_attackerType = FASTA;
            break;
        default:
            m_attackerType = BASICA; // redundant
            break;
    }
}

bool Player::addAttackerToQueue() {
    if (m_def[m_attackerType]["price"] > m_coins)
        return false;
    m_coins -= m_def[m_attackerType]["price"];
    m_attackersQueue.emplace_back(m_attackerType, m_spawnLane);
    return true;
}

void Player::moveAttackers() {
    std::vector<int> toDelete;
    for (auto &[id, attacker]: m_attackers) {
        if (attacker->hasSlowEffect())
            attacker->setSlowerMovement();
        else {
            attacker->setNormalMovement();
        }
        if (attacker->findShortestPath(Point::Exit)) {
            if (!attacker->moveOnPath()) {
                if (attacker->getNextPoint()._type == Point::Exit) {
                    m_attackersFinished++;
                    toDelete.push_back(id);
                    attacker->destroy();
                }
            }
        }
    }
    for (auto &id: toDelete) {
        m_attackers.erase(id);
    }
}

bool Player::emptyAttackers() {
    return m_attackers.empty();
}

bool Player::emptyAttackerQueue() {
    return m_attackersQueue.empty();
}

void Player::clearAttackers() {
    for (auto &attacker: m_attackers) {
        attacker.second->destroy();
    }
    m_attackers.clear();
    m_attackersQueue.clear();
}

int Player::getFinished() const {
    return m_attackersFinished;
}

const std::map<int, std::unique_ptr<Attacker> > &Player::getAttackers() const {
    return m_attackers;
}

std::vector<std::pair<int, int>> Player::getTowersToAttack() const {
    std::vector<std::pair<int, int> > towers;
    for (auto &attacker: m_attackers) {
        if (attacker.second->checkRadius(Point::Tower)) {
            if (attacker.second->canAttack())
                towers.emplace_back(attacker.second->getCurrentFocus(), attacker.second->getDamage());
        }
    }
    return towers;
}

bool Player::createNewAttacker(int type, int x, int y, int hp, CEffects effects) {
    switch (type) {
        case 0:
            m_attackers.emplace(m_attackerID, std::make_unique<basicAttacker>(x, y, m_def, m_map, m_attackerID, hp));
            break;
        case 1:
            m_attackers.emplace(m_attackerID, std::make_unique<fastAttacker>(x, y, m_def, m_map, m_attackerID, hp));
            break;
        case 2:
            m_attackers.emplace(m_attackerID, std::make_unique<chargerAttacker>(x, y, m_def, m_map, m_attackerID, hp));
            break;
    }
    m_attackerID++;
    return true;
}

void Player::printAttackers() {
    for (const auto &attacker: m_attackers) {
        if (attacker.second->getHP() > 0)
            attacker.second->draw();
    }
}

void Player::setFinished(int nr) {
    m_attackersFinished = nr;
}

void Player::setAttackersQueue(std::deque<std::pair<std::string, Point> > &queue) {
    m_attackersQueue.clear();
    m_attackersQueue = queue;
}

size_t Player::getAttackerCount() {
    return m_attackers.size();
}

int Player::damageAttackers(std::vector<std::tuple<int, int, CEffects> > &attackers) {
    std::vector<int> toRemove;
    for (const auto &[id, dmg, effects]: attackers) {
        if (m_attackers.count(id)) {
            if (!m_attackers[id]->takeDamage(dmg))
                toRemove.emplace_back(id);
            else
                m_attackers[id]->addEffects(effects);
        }
    }
    for (int index: toRemove) {
        if (m_attackers.count(index)) {
            m_attackers[index]->destroy();
            m_attackers.erase(index);
        }
    }

    return toRemove.size();
}

const std::deque<std::pair<std::string, Point>> & Player::getAttackersQueue() {
    return m_attackersQueue;
}

