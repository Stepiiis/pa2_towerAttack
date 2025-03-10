#include "Menu.h"


CMenu::CMenu(std::vector<std::string> text, std::vector<std::string> options, int defaultOption) {
    _text = std::move(text);
    _options = std::move(options);
    _chosen = defaultOption;
}

int CMenu::show(bool wait) {
    terminal term;
    int height = 25;
    int width = 75;
    int posX = (width / 2) - (int) (this->_text.size() / 2) - 10;

    auto style = A_STANDOUT;
    getmaxyx(stdscr, term.height, term.width); // zjisteni velikosti obrazovky
    int starty = (term.height - height) / 2; // vypocet pozice mapy
    int startx = (term.width - width) / 2; // vypocet pozice mapy
    auto menu_win = newwin(height, width, starty, startx);
    refresh();
    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);
    wrefresh(menu_win);
    int posY = 5;
    auto text = this->_text.begin();
    while (text != this->_text.end()) {
        mvwprintw(menu_win, posY, posX, "%s", (*text).c_str());
        wrefresh(menu_win);
        posY++;
        text++;
    }
    wrefresh(menu_win);
    int keypress = 0;
    int choice = _chosen;
    if (wait) {
        posY++;
        mvwprintw(menu_win, posY, posX, "Press any key to continue");
        printShrek(menu_win, posY + 1, posX);
        refresh();
        wgetch(menu_win);
        mvwprintw(menu_win, posY, posX, "                         ");
        clearShrek(menu_win, posY + 1, posX);
    }
    posY++;
    int temp = posY;
    while (true) {
        posY = temp;
        if (keypress == 27) {
            delwin(menu_win);
            touchwin(stdscr);
            refresh();
            return -20;
        }
        if (keypress == KEY_UP) {
            choice = (choice - 1);
            if (choice == -1)
                choice = (int) this->_options.size() - 1;
            else
                choice %= (int) this->_options.size();
        } else if (keypress == KEY_DOWN) {
            choice = (choice + 1) % (int) this->_options.size();
        } else if (keypress == KEY_DC) {
            break;
        }
//        wmove(menu_win,0,0);
//        wprintw(menu_win, "%d %d %d", choice, menu._options.size()), (-1%3);
        auto option = this->_options.begin();
        while (option != this->_options.end()) {
            if (choice == std::distance(this->_options.begin(), option)) {
                wattron(menu_win, style);
                mvwprintw(menu_win, posY, posX, "%s", (*option).c_str());
                wattroff(menu_win, style);
            } else {
                mvwprintw(menu_win, posY, posX, "%s", (*option).c_str());
            }
            posY++;
            option++;
        }
        wrefresh(menu_win);
        keypress = wgetch(menu_win);
    }
    delwin(menu_win);
    touchwin(stdscr);
    refresh();
    move(0, 0);
    return choice;
}

void CMenu::setMenu(std::vector<std::string> text, std::vector<std::string> options) {
    _text = std::move(text);
    _options = std::move(options);
}

int CMenu::mainMenu() {

    CMenu main({
                       "Welcome to",
                       "Tower Attack 2",
                       "Electric Boogaloo"
               }, {
                       "New game",
                       "Load game",
                       "Options",
                       "Exit"
               });

    return main.show(true);
}

// return values eiter positive which specify which index of saves choose
// or negative which specify error handling
// -10 - new game
// -20 - back to menu
int CMenu::loadMenu(std::vector<std::string> &save_names) {
    const std::experimental::filesystem::path path{"assets/saves/"};
    for (const auto &saves: std::experimental::filesystem::directory_iterator(path)) {
        save_names.emplace_back(saves.path().filename().string());
//        std::cout << saves.path().filename().string() << std::endl;
    }
    CMenu loadMenu;

    if (!save_names.empty()) {
        loadMenu.setMenu({
                                 "Please select a save file.",
                                 "Confirm with delete, or press ESC to go back."
                         }, {save_names});
        return loadMenu.show();
    }
    loadMenu.setMenu({
                             "No save files found",
                             "Please create a new game"
                     }, {
                             "New game", "Back to main menu"
                     });

    int choice = loadMenu.show();
    int retval = choice == 0 ? -10 : -20;
    return retval;
}


void CMenu::printShrek(WINDOW *menu_win, int posY, int posX) {
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY++, posX, "      @     |    @       ");
    mvwprintw(menu_win, posY++, posX, "      @     |    @  @    ");
    mvwprintw(menu_win, posY++, posX, "      @     |    @  @    ");
    mvwprintw(menu_win, posY++, posX, "            |            ");
    mvwprintw(menu_win, posY++, posX, "------------+------------");
    mvwprintw(menu_win, posY++, posX, "            |            ");
    mvwprintw(menu_win, posY++, posX, "    @   @   |   @        ");
    mvwprintw(menu_win, posY++, posX, "    @   @   |   @        ");
    mvwprintw(menu_win, posY, posX, "    @   @   |   @ @@@@@  ");
}

void CMenu::clearShrek(WINDOW *menu_win, int posY, int posX) {
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY++, posX, "                         ");
    mvwprintw(menu_win, posY, posX, "                         ");
}

void cleanPos(WINDOW *okno, int posX, int posY, int len) {
    wattroff(okno, A_STANDOUT);
    for (int i = 0; i < len; ++i)
        mvwprintw(okno, posY, posX + i, " ");
    wattron(okno, A_STANDOUT);
}

std::string CMenu::saveMenu() {
    int delkaJmena;
    std::string saveName;
    std::vector<std::string> message{"Enter the save name please.", "The name can only be one word.",
                                     "Allowed are both uppercase/lowercase letters and numbers.", " ",
                                     "If you dont want to save the game,",
                                     "please leave the field empty and press ESC"};
    std::string option = "Press ESC to save.";
    std::string errorZnak = "Character not allowed, press backspace please.";
    std::string errorLength = "Character limit reached, press backspace please.";
    std::set<int> allowed;
    for (int i = 48; i <= 57; ++i)
        allowed.emplace(i);
    for (int i = 65; i <= 90; ++i)
        allowed.emplace(i);
    for (int i = 97; i <= 122; ++i)
        allowed.emplace(i);
    bool error = false;
    terminal term;
    int height = 25;
    int width = 75;
    int posX = (width / 2) - (int) (message[0].size() / 2) - 10;
    auto style = A_STANDOUT;
    getmaxyx(stdscr, term.height, term.width); // zjisteni velikosti obrazovky
    int starty = (term.height - height) / 2; // vypocet pozice mapy
    int startx = (term.width - width) / 2; // vypocet pozice mapy
    auto menu_win = newwin(height, width, starty, startx);

    refresh();
    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);
    wrefresh(menu_win);
    int posY = 5;
    auto text = message.begin();
    while (text != message.end()) {
        mvwprintw(menu_win, posY, posX, "%s", (*text).c_str());
        wrefresh(menu_win);
        posY++;
        text++;
    }
    wrefresh(menu_win);
    wattron(menu_win, style);
    mvwprintw(menu_win, posY + 3, posX, "%s", option.c_str());
    wattroff(menu_win, style);
    posY++;
    wattron(menu_win, A_STANDOUT);
    int keypress = 0;
    while (true) {
        wtimeout(menu_win, 50);
        keypress = wgetch(menu_win);
        if (keypress != ERR) {
            if (keypress == 27) // Code for ESC in NCurses
            {
                if (saveName.empty())
                    break;
                if (error)
                    continue;
                break;
            } else if (keypress == KEY_BACKSPACE) {
                size_t length = saveName.size();
                if (length == 0)
                    continue;
                wattroff(menu_win, A_STANDOUT);
                mvwprintw(menu_win, posY, posX + length - 1, " ");
                wattron(menu_win, A_STANDOUT);
                saveName.pop_back();
                if (error) {
                    cleanPos(menu_win, posX, posY + 4, (int) std::max(errorLength.size(), errorZnak.size()));
                    error = false;
                }
                wrefresh(menu_win);
            } else if (allowed.count(keypress) != 1) {
                wattroff(menu_win, A_STANDOUT);
                mvwprintw(menu_win, posY + 4, posX, "%s", errorZnak.c_str());
                wattron(menu_win, A_STANDOUT);
                error = true;
                wrefresh(menu_win);
            } else {
                if (saveName.size() < 25) {
                    saveName.push_back((char) keypress);
                    mvwprintw(menu_win, posY, posX, "%s", saveName.c_str());
                    if (error) {
                        cleanPos(menu_win, posX, posY + 4, (int) std::max(errorLength.size(), errorZnak.size()));
                        error = false;
                    }
                    wrefresh(menu_win);
                } else {
                    wattroff(menu_win, A_STANDOUT);
                    mvwprintw(menu_win, posY + 4, posX, "%s", errorLength.c_str());
                    wattron(menu_win, A_STANDOUT);
                    wrefresh(menu_win);
                    error = true;
                }
            }

        }
    }

    wattroff(menu_win, A_STANDOUT);
    delwin(menu_win);
    return saveName;
}

int CMenu::optionsMenu(int difficulty) {
    CMenu options(
            {"Please choose difiiculty"},
            {"Easy", "Normal", "Hard"}, difficulty
    );
    return options.show();
}

