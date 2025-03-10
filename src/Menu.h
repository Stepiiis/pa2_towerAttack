#pragma once
#include "Game.h"
#include <vector>
#include <string>
#include <set>
#include <ncurses.h>
#include <experimental/filesystem>
#include "HelpStructs.h"


struct CMenu{
    /// default constructor. has to be called setMenu to set values of menus to be showed
    CMenu() = default;

    /// overloaded constructior which can set the text and options values upon instantiation
    /// @param text text to be shown on top of the menu
    /// @param options options to choose from
    CMenu(std::vector<std::string> text, std::vector<std::string> options, int defaultOption = 0);

    /// if parameter wait is true, it will wait for input before showing menu
    /// @returns the number of option that was chosen
    /// @param wait specifies if it should show the wait menu
    int show(bool wait = false);

    /// sets the value of text and options vectors
    /// @param text text to be shown on top of the menu
    /// @param options options to choose from
    void setMenu(std::vector<std::string> text, std::vector<std::string> options); // sets the menu


    /// Shows the main menu and returns the value chosen
    /// @returns 0 - new game, 1 - load game, 2 - options, 3 - exit
    static int mainMenu();

    /// Shows save menu with abilitiy to type name of save
    /// @returns string which corresponds the text written on screen
    static std::string saveMenu();

    /// shows all saves which are currently in /assets/saves
    /// @param save_names read write reference vector of loaded saves in /assets/saves. All names of saves loaded from directory are rturned there
    /// @returns index inside the vector save_names which specifies which save to load
    static int loadMenu(std::vector<std::string>& save_names);

    /// shows menu which allows user to change difficulty
    /// @param dif current difficulty - adjusted according to choice inside menu
    static int optionsMenu(int dif);

    /// prints a picture on screen on specified coordinate
    static void printShrek(WINDOW* menu_win, int posY, int posX);
    static void clearShrek(WINDOW* menu_win, int posY, int posX);

private:
    std::vector<std::string> _text; // vector of text to be shown on screen
    std::vector<std::string> _options; // vector of choices to be able to chose from
    int _chosen = 0 ; // value currently chosen

};

