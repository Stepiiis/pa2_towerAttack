
#include "Game.h"


CGame::~CGame() {
    delete m_player;
    delete m_tower_manager;
    delwin(m_game_window); // zavreni okna s hrou
    endwin(); // dealoc pameti a zavreni ncurses
    exit_curses(0);
}

bool CGame::start() {
    drawAttackerDefs();
    if (resume())
        return true;
    return false;
}

void CGame::initializeWindow() {
    refresh();
    keypad(m_game_window, TRUE);
    box(m_game_window, 0, 0);
    wrefresh(m_game_window);
}

bool CGame::resume() {
    int input;
    auto startTime = std::chrono::system_clock::now();
    auto currentTime = std::chrono::system_clock::now();
    auto lastTime = std::chrono::system_clock::now(); //std::chrono::time_point<std::chrono::system_clock>
    std::string errorMessage;
    int lastErrorLen{};
    m_player->setLane(0);
    highlightAttacker(0);
    m_player->setAttackerType(0);
    while (true) {
        this->drawCurrentMoney();
        if (!errorMessage.empty()) {
            lastErrorLen = (int) errorMessage.length();
            this->drawErrorMessage(errorMessage);
        } else
            this->cleanErrorMessage(lastErrorLen);
        wtimeout(m_game_window, 35);
        input = wgetch(m_game_window);
        if (input == 27) {
            return false;
        }
//        if (input != ERR)
//            mvprintw(0, 0, "%c", input);
        if (input == 'q' || input == 'Q') {
            if (pauseMenu()) {
                touchwin(m_game_window);
                continue;
            } else
                return false;
        }

        if (input >= '0' && input <= '9')// CHOICES OF INPUT LANES, can be 0-9 (currently using 3)
            m_player->setLane(input - 49);
        if (input == 'a' || input == 'A') // choices of attacker to spawn
        {
            highlightAttacker(0);
            m_player->setAttackerType(0);
        }
        if (input == 's' || input == 'S') {
            highlightAttacker(1);
            m_player->setAttackerType(1);
        }
        if (input == 'd' || input == 'D') {
            highlightAttacker(2);
            m_player->setAttackerType(2);
        }
        if (input == ' ') {
            if (!m_player->addAttackerToQueue())
                errorMessage = "Not enough money  to spawn this     attacker";
        }
        currentTime = std::chrono::system_clock::now();
        auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
        auto modulTime = deltaTime % 500;
        if ((modulTime < 100 && modulTime > 0) && currentTime != lastTime) {
            lastTime = currentTime;
            // perform attacks from attackers (we will make it easier for the player)
            // perform attacks from towers
            performAttacks();

            if (!m_player->emptyAttackerQueue()) {
                errorMessage = m_player->spawnAttacker();
            }
            // first figure out the route for attackers
            // then add them to ncurses window buffer
            if (!m_player->emptyAttackers())
                m_player->moveAttackers();

        }
        wrefresh(m_game_window);
        if (m_tower_manager->getTowerCount() == 0 && m_player->getAttackerCount() == 0)
            break;
        if (m_player->getAttackerCount() == 0 && m_player->getCoins() < 100)
            return false;
    }
    return true; // GAME WON
}


bool CGame::loadMap(int level) {
    if (m_gameMap.readMap(level)) {
        return true;
    }
    return false;
}

void CGame::drawTowers() {
    if (m_tower_manager->getTowerCount() != 0) {
        m_tower_manager->clearTowers();
    }
    m_tower_manager->createTowers();
    m_tower_manager->printTowers();
    wrefresh(m_game_window);
}

bool CGame::save(std::string saveName) {
    if (saveName.empty())
        saveName = CMenu::saveMenu();
    if (saveName.empty())
        return false;
    std::stringstream ss;
    ss << "assets/saves/" << saveName << ".txt";
    std::ofstream saveFile;
    saveFile.open(ss.str(), std::ofstream::out | std::ofstream::trunc);
    if (!saveFile.is_open())
        return false;
    if (saveFile.good() == 0)
        return false;
    saveFile << "map " << 1 << std::endl;
    saveFile << "difficulty " << m_difficulty << std::endl;
    saveFile << "tsdestroyed " << m_towers_destroyed << std::endl;
    saveFile << "asfinished " << m_player->getFinished() << std::endl;
    saveFile << "money " << m_player->getCoins() << std::endl;
    saveFile << std::endl;
    saveFile << "# attackers" << std::endl;
    for (const auto &attacker: m_player->getAttackers()) {
        auto point = attacker.second->getPosition();
        saveFile << "type " << attacker.second->getTypeName() << std::endl
                 << "x " << point.first << std::endl
                 << "y " << point.second << std::endl
                 << "hp " << attacker.second->getHP() << std::endl
                 << "slw " << attacker.second->getEffects().m_slowEffect << std::endl;
    }
    saveFile << "# towers" << std::endl;
    for (const auto &attacker: m_tower_manager->getTowers()) {
        auto point = attacker.second->getPosition();
        saveFile << "type " << attacker.second->getTypeName() << std::endl
                 << "x " << point.first << std::endl
                 << "y " << point.second << std::endl
                 << "hp " << attacker.second->getHP() << std::endl;
    }
    saveFile << "# queue" << std::endl;
    for (const auto &attacker: m_player->getAttackersQueue()) {
        auto point = attacker.second;
        saveFile << "name " << attacker.first << std::endl
                 << "x " << point._x << std::endl
                 << "y " << point._y << std::endl;
    }
    saveFile << "# end" << std::endl;
    saveFile << std::flush;
    saveFile.close();
    return true;
}

// loads game from file defined in the parameter and starts the game
bool CGame::loadFromSave(const std::string &nameOfSave) {
    // delete all towers and players
    if (m_player != nullptr)
        m_player->clearAttackers();
    if (m_tower_manager != nullptr)
        m_tower_manager->clearTowers();
    // delete the old map and load the map definition provided in the save file
    m_gameMap.clearMap();
    std::stringstream savepath;
    savepath << "assets/saves/" << nameOfSave;
    std::ifstream savefile(savepath.str(), std::ios::in);
    std::string fileLine;
    std::map<std::string, std::vector<std::map<std::string, std::string>>> entities;
    std::map<std::string, std::string> givenEntity;
    std::vector<std::map<std::string, std::string>> vectorOfEntities;
    std::map<std::string, std::string> mapOfAttackersInQueue;
    std::deque<std::pair<std::string, Point>> attackersInQueue;
    std::map<std::string, int> gameStats;
    std::string name;
    std::string value;
    bool gameReading = false;
    bool queueReading = false;
    int lineindex = 0;
    std::string entType;

    std::string test;
    while (getline(savefile, fileLine)) {
        lineindex++;
        if (fileLine.empty())
            continue;
        std::stringstream ss(fileLine);
        if (!gameReading) {
            int numvalue = -10;
            name.clear();
            ss >> name;
            ss >> numvalue;
            if (numvalue == -10) {
                ss.str(std::string());
                ss << "attribute on line " << lineindex << " in save file " << nameOfSave << " is incorrect. "
                   << "name= " << name << " value= " << value;
                throw syntaxErr(ss.str());
            }
            gameStats.emplace(name, numvalue);
            if (lineindex == 5)
                gameReading = true;
            continue;
        }
        if (!queueReading) {
            if (fileLine[0] == '#') {
                if (!givenEntity.empty()) {
                    vectorOfEntities.emplace_back(givenEntity);
                    givenEntity.clear();
                }
                if (!vectorOfEntities.empty()) {
                    entities.emplace(entType, vectorOfEntities);
                    vectorOfEntities.clear();
                }
                test.clear();
                ss >> entType; // getting rid of #
                entType.clear();
                ss >> entType; // value of type
                if (entType == "queue") {
                    queueReading = true;
                    continue;
                }
                ss >> test;
                if (!test.empty() || entType.empty()) {
                    ss.str(std::string());
                    ss << "attribute on line " << lineindex << " in save file " << nameOfSave << " is incorrect.";
                    throw syntaxErr(ss.str());
                }
                continue;
            }
            name.clear();
            value.clear();
            test.clear();
            ss >> name;
            ss >> value;
            ss >> test;
            if (!test.empty() || value.empty() || name.empty()) {
                ss.str(std::string());
                ss << "attribute on line " << lineindex << " in save file " << nameOfSave << " is incorrect. "
                   << "name= " << name << " value= " << value;
                throw syntaxErr(ss.str());
            }
            if (name == "type") {
                if (!givenEntity.empty()) {
                    vectorOfEntities.emplace_back(givenEntity);
                    givenEntity.clear();
                }
            }
            givenEntity.emplace(name, value);
        } else {
            name.clear();
            value.clear();
            test.clear();
            ss >> name;
            ss >> value;
            ss >> test;
            if (!test.empty() || name.empty() || value.empty()) {
                ss.str(std::string());
                ss << "attribute of queue on line" << lineindex << " in save file " << nameOfSave << " is incorrect. "
                   << "name= " << name << " value= " << value;
                throw syntaxErr(ss.str());
            }
            if (name == "name" || (name == "#" && value == "end")) {
                if (!mapOfAttackersInQueue.empty()) {
                    std::string attrName;
                    int attrX;
                    int attrY;
                    if (mapOfAttackersInQueue.count("name") == 0 || mapOfAttackersInQueue.count("x") == 0 ||
                        mapOfAttackersInQueue.count("y") == 0) {
                        ss.str(std::string());
                        ss << "one or more attributes in queue in save file " << nameOfSave << " is incorrect.";
                        throw syntaxErr(ss.str());
                    }
                    try {
                        attrName = mapOfAttackersInQueue.at("name");
                        attrX = std::stoi(mapOfAttackersInQueue.at("x"));
                        attrY = std::stoi(mapOfAttackersInQueue.at("y"));
                    } catch (std::exception &e) {
                        ss.str(std::string());
                        ss << "one or more attributes in queue in save file " << nameOfSave << " is incorrect.";
                        throw syntaxErr(ss.str());
                    }
                    attackersInQueue.emplace_back(std::make_pair(attrName, Point(attrX, attrY)));
                    mapOfAttackersInQueue.clear();
                }
                if (name == "#" && value == "end")
                    break;
            }
            mapOfAttackersInQueue.emplace(name, value);
        }
    }

    int i{};
    for (const auto &type: entities) {
        sendToLogFile(0, "loading entities typed " + type.first, "loadFromSave");
        i = 0;
        for (const auto &entity: type.second) {
            i++;
            sendToLogFile(0, "   index " + std::to_string(i), "loadFromSave");
            for (const auto &attr: entity) {
                sendToLogFile(0, "      attibute " + attr.first + " valued " + attr.second, "loadFromFile");
            }
        }
    }


    // load the safe file into a respective vectors of of attackers and towers
    if (entities.count("attackers") == 1) // entities.count("towers") == 1
    {
        for (const auto &utocnici: entities["attackers"]) {
            int x;
            int y;
            int hp;
            CEffects eff;
            std::stringstream ss;
            try {
                x = std::stoi(utocnici.at("x"));
                y = std::stoi(utocnici.at("y"));
                hp = std::stoi(utocnici.at("hp"));
                eff.m_slowEffect = std::stoi(utocnici.at("slw"));
            } catch (std::exception &e) {
                ss.str(std::string());
                ss << "one or more attributes is incorrect, saveFile might be corrupt";
                throw syntaxErr(ss.str());
            }
            if (utocnici.at("type") == BASICA)
                m_player->createNewAttacker(0, x, y, hp, eff);
            else if (utocnici.at("type") == FASTA)
                m_player->createNewAttacker(1, x, y, hp, eff);
            else if (utocnici.at("type") == CHARGERA)
                m_player->createNewAttacker(2, x, y, hp, eff);
            else {
                ss.str(std::string());
                ss << "one or more attributes is incorrect, saveFile might be corrupt";
                throw syntaxErr(ss.str());
            }
        }
    }
    if (entities.count("towers") == 1) {
        for (const auto &veze: entities["towers"]) {
            int x;
            int y;
            int hp;
            std::stringstream ss;
            try {
                x = std::stoi(veze.at("x"));
                y = std::stoi(veze.at("y"));
                hp = std::stoi(veze.at("hp"));
            } catch (std::invalid_argument &e) {
                ss.str(std::string());
                ss << "one or more attributes is incorrect, saveFile might be corrupt";
                throw syntaxErr(ss.str());
            }
            catch (std::out_of_range &e) {
                ss.str(std::string());
                ss << "one or more attributes is missing, saveFile might be corrupt";
                throw syntaxErr(ss.str());
            }
            if (veze.at("type") == BASICT)
                m_tower_manager->createNewTower(0, x, y, hp);
            else if (veze.at("type") == FASTT)
                m_tower_manager->createNewTower(1, x, y, hp);
            else if (veze.at("type") == STRONGT)
                m_tower_manager->createNewTower(2, x, y, hp);
            else if (veze.at("type") == SLOWET)
                m_tower_manager->createNewTower(3, x, y, hp);
            else {
                ss.str(std::string());
                ss << "one or more attributes is incorrect, saveFile might be corrupt";
                throw syntaxErr(ss.str());
            }
        }

    }


    // read the map
    m_gameMap.readMap(gameStats["map"]);
    // print towers and attackers on screen and start the game
    m_tower_manager->printTowers();
    m_player->printAttackers();
    m_player->setAttackersQueue(attackersInQueue);
    // set the game money and difficulty to the one specified in the save file
    if (gameStats.count("money") == 0 || gameStats.count("money") == 0 || gameStats.count("asfinished") == 0 ||
        gameStats.count("tsdestroyed") == 0)
        throw syntaxErr("one or more gamestats is invalid, saveFile might be corrupt");
    m_player->setCoins(gameStats["money"]);
    setDifficulty(gameStats["difficulty"]);
    m_player->setFinished(gameStats["asfinished"]);
    setTowersDestroyed(gameStats.at("tsdestroyed"));
    return true;
}

void CGame::highlightAttacker(int type) {
    int top = (int) m_gameMap.m_map.size() + 2;
    int defaultTop = top;
    int left = 2;
    int i = 0;
    for (const auto &ent: m_definitions.getAttacker()) {
        if (i == type)
            wattron(m_game_window, A_STANDOUT);
        mvwprintw(m_game_window, top, left, "%s", ent.first.c_str());
        top++;
        for (auto &hodnoty: ent.second) {
            top++;
            if (hodnoty.first == "symbol") {
                mvwprintw(m_game_window, top, left, "%s : %c", hodnoty.first.c_str(), hodnoty.second);
                continue;
            }
            mvwprintw(m_game_window, top, left, "%s : %d", hodnoty.first.c_str(), hodnoty.second);
        }
        top = defaultTop;
        left += (int) ent.first.length() + 4;
        if (i == type)
            wattroff(m_game_window, A_STANDOUT);
        i++;
    }
    wrefresh(m_game_window);
}

void CGame::drawAttackerDefs() {
    int top = (int) m_gameMap.m_map.size() + 2;
    int defaultTop = top;
    int left = 2;

    for (const auto &ent: m_definitions.getAttacker()) {
        mvwprintw(m_game_window, top, left, "%s", ent.first.c_str());
        top++;
        for (auto &hodnoty: ent.second) {
            top++;
            if (hodnoty.first == "symbol") {
                mvwprintw(m_game_window, top, left, "%s : %c", hodnoty.first.c_str(), hodnoty.second);
                continue;
            }
            mvwprintw(m_game_window, top, left, "%s : %d", hodnoty.first.c_str(), hodnoty.second);
        }
        top = defaultTop;
        left += (int) ent.first.length() + 4;
    }
    wrefresh(m_game_window);
}

// display the stats of the game and return to main menu
bool CGame::gameEnd(const char *msg) {
    CMenu endMenu;
    std::stringstream scorestring;
    std::stringstream sstowersDestroyed;
    std::stringstream coinsLeft;
    std::stringstream playersFinished;

    m_score = (m_towers_destroyed * 10 + m_player->getCoins() + m_player->getFinished() * 25) * (m_difficulty + 1);
    sstowersDestroyed << "towers destroyed: \t" << m_towers_destroyed << " * 10 = " << m_towers_destroyed * 10;
    coinsLeft << "coins left: \t\t" << m_player->getCoins();
    playersFinished << "players finished: \t" << m_player->getFinished() << " * 25 = " << m_player->getFinished() * 25;
    scorestring << "total score: \t" << m_score << " points!";

    endMenu.setMenu({msg, " ", sstowersDestroyed.str(), playersFinished.str(), coinsLeft.str(), " ", scorestring.str()},
                    {"QUIT", "RETURN TO MENU"});

    int retval = endMenu.show();
    if (retval == 0)
        return false;
    return true;
}

bool CGame::init(int level, const char *nameOfSave) {
    if (m_definitions.getTower().empty())
        try { m_definitions.loadDefinitions(); }
        catch (std::exception &e) {
            showError({"ERROR: ", "Failed during loading of definitions.", "Try to recompile the game."});
        }
    terminal term;
    int winheight = 25;
    int winwidth = 75;
    getmaxyx(stdscr, term.height, term.width); // zjisteni velikosti obrazovky
    int starty = (term.height - winheight) / 2; // vypocet pozice mapy
    int startx = (term.width - winwidth) / 2; // vypocet pozice mapy
    if (m_game_window == nullptr) {
        m_game_window = newwin(winheight, winwidth, starty, startx);
        m_gameMap.setWindow(m_game_window);
        initializeWindow();
    } else {
        delwin(m_game_window);
        m_game_window = newwin(winheight, winwidth, starty, startx);
        m_gameMap.setWindow(m_game_window);
        initializeWindow();
    }

    if (m_player == nullptr)
        m_player = new Player(&m_gameMap, m_definitions.getAttacker());
    else
        m_player->clearAttackers();

    if (m_tower_manager == nullptr)
        m_tower_manager = new Enemy(&m_gameMap, m_definitions.getTower(), m_difficulty);
    else
        m_tower_manager->clearTowers();

    if (std::strlen(nameOfSave) != 0) {
        try { loadFromSave(nameOfSave); }
        catch (std::exception &e) {
            showError({"ERROR: ", "Failed during loading of save file.", "Try to recompile the game."});
            return false;
        }
        m_gameMap.printMap();
    } else {
        if (m_gameMap.m_map.empty())
            loadMap(level);
        m_gameMap.printMap();
        drawTowers();
        m_player->setCoins(2500);
    }
    m_gameMap.setWindow(m_game_window);
    return true;
}

bool CGame::play() {
    initscr(); // inicializace ncurses a pameti
    noecho(); // zakaz vypisu na obrazovku
    cbreak(); // ctrlc se neukonci program


    while (true) {
        int choice = CMenu::mainMenu();

        if (choice == 0) { // choice is new game
            //new game
            if (!this->init(1)) {
                continue;
            }
            if (!this->start()) {
                if (this->gameEnd("You lost!"))
                    continue;
                else
                    return EXIT_FAILURE;
            } else {
                if (this->gameEnd("You won!"))
                    continue;
                else
                    return EXIT_FAILURE;
            }
        } else if (choice == 1) { // load game
            std::vector<std::string> saves;
            choice = CMenu::loadMenu(saves);
            std::string save;
            if (choice >= 0) {
                save = saves[choice];
            } else if (choice == -20)
                continue;

            this->init(1, save.c_str());
            if (!this->start()) {
                if (this->gameEnd("You lost!"))
                    continue;
                else
                    return EXIT_FAILURE;
            } else {
                if (this->gameEnd("You won!"))
                    continue;
                else
                    return EXIT_FAILURE;
            }
        } else if (choice == 2) {
            m_difficulty = CMenu::optionsMenu(m_difficulty);
            continue;
        } else if (choice == 3) {
            //exit
            break;
        }
    }
    return EXIT_SUCCESS;
}

bool CGame::pauseMenu(const char *msg) {
    CMenu pauseMenu({"GAME PAUSED", msg}, {"Resume", "Save and resume", "End and don't save"});

    int choice = pauseMenu.show();
    if (choice == 0) { // resume game
        return true;
    } else if (choice == 1) {
        if (!save(""))
            return this->pauseMenu("Save failed, try again.");
        return true; // game saved and resumed
    } else if (choice == 2) {
        return false; // quit game
    }
    return false;
}

void CGame::drawCurrentMoney() {
    int width = m_gameMap.getMapWidth();
    mvwprintw(m_game_window, 2, width + 2, "Money: %d ", m_player->getCoins());
}

bool CGame::performAttacks() {
    auto towersToAttack = m_player->getTowersToAttack();

    if (!towersToAttack.empty())
        m_towers_destroyed += m_tower_manager->damageTowers(towersToAttack);

    auto attackersToAttack = m_tower_manager->getAttackersToAttack();
    if (!attackersToAttack.empty())
        m_player->damageAttackers(attackersToAttack);

    for (const auto &attacker: attackersToAttack) {
        sendToLogFile(0, "performing attack on Attacker ID:" + std::to_string(std::get<0>(attacker)),
                      "CGame::performAttacks");
    }
    for (const auto &tower: towersToAttack) {
        sendToLogFile(0, "performing attack on Tower ID:" + std::to_string(tower.first), "CGame::performAttacks");
    }

    return true;
}

void CGame::setDifficulty(int def) {
    if (def > 2 || def < 0)
        throw logException("Wrong difficulty passed (allowed 0-2, passed " + std::to_string(def) + " )");
    m_difficulty = def;
}

void CGame::setTowersDestroyed(int nr) {
    m_towers_destroyed = nr;
}

void CGame::drawErrorMessage(const std::string &msg) {
    std::vector<std::string> error_msg;
    std::stringstream ss;
    auto it = msg.begin();
    int i = 1;
    int maxLen = errorTextLenght;
    while (it != msg.end()) {
        ss << *it;
        if (i % maxLen == 0) {
            error_msg.push_back(ss.str());
            ss.str("");
        }
        i++;
        ++it;
    }
    if (!ss.str().empty())
        error_msg.push_back(ss.str());

    wattron(m_game_window, A_STANDOUT);

    int posX = m_gameMap.getMapWidth() + 2;
    int posY = 5;
    mvwprintw(m_game_window, posY - 1, posX, "Error: ");
    for (const auto &message: error_msg) {
        mvwprintw(m_game_window, posY, posX, "%s", message.c_str());
        posY++;
    }
    wattroff(m_game_window, A_STANDOUT);
}

void CGame::cleanErrorMessage(int len) {

    int posX = m_gameMap.getMapWidth() + 2;
    int posY = 5;
    mvwprintw(m_game_window, posY - 1, posX, "       ");
    for (int i = 0; i <= len; i++) {
        mvwprintw(m_game_window, posY, posX + i % errorTextLenght, " ");
        if (((i + 1) % errorTextLenght) == 0)
            posY++;
    }
}

void CGame::showError(const std::vector<std::string> &strings) {
    CMenu error({strings}, {"OK"});
    error.show();
}


bool compareFiles(const char *lhs, const char *rhs) {
    std::ifstream lhs_file(lhs);
    std::ifstream rhs_file(rhs);
    std::string lhs_line, rhs_line;
    while (std::getline(lhs_file, lhs_line) && std::getline(rhs_file, rhs_line)) {
        if (lhs_line != rhs_line)
            return false;
    }
    return true;
}

void CGame::testRoutine() {
    try { m_definitions.loadDefinitions(); }
    catch (std::exception &e) {
        showError({"ERROR: ", "Failed during loading of definitions.", "Try to recompile the game."});
    }
    terminal term;
    initscr();
    noecho();
    int winheight = 25;
    int winwidth = 75;
    getmaxyx(stdscr, term.height, term.width); // zjisteni velikosti obrazovky
    int starty = (term.height - winheight) / 2; // vypocet pozice mapy
    int startx = (term.width - winwidth) / 2; // vypocet pozice mapy
    m_game_window = newwin(winheight, winwidth, starty, startx);
    m_gameMap.setWindow(m_game_window);
    initializeWindow();

    m_gameMap.readMap();


    auto testplayer = new Player(&m_gameMap, m_definitions.getAttacker());
    auto testAI = new Enemy(&m_gameMap, m_definitions.getTower(), 0);

    init(0, "test.txt");


    save("testsaved");

    init(0, "incorrectSave.txt");


    if (!compareFiles("assets/saves/test.txt", "assets/saves/testsaved.txt"))
        throw logException("initial trial failed, try to recompile");
    delwin(m_game_window);
    delete testplayer;
    delete testAI;
}

int CGame::getDifficulty() {
    return m_difficulty;
}
