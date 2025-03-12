TowerAttack
=============

## Introduction

### What is Tower Attack?
Tower attack is a game where the player has control of spawning attackers and tries to conquer the battlefield 
by destroying all towers and leaving through the exit door.


#### Points on map legend
- ' ' - Empty places where Towers can be placed and Attackers can move
- 'F' - fast shooting tower
- 'I' - default tower
- 'T' - high damage tower
- 'S' - tower which shoots damage and gives slow buff
- '#' - wall
- 'o','O' - watter, only fast attacker can move through it
- '$" - basic attacker
- '@' - slower attacker with higher damage
- '%' - faster attacker which avoids perimeter of towers
- '<=' - entries and exits from map.


## Controls
- DELETE confirms the selected option in menus
- 'a','s' a 'd' chooses different attackers
- '1', '2', '3' chooses different towers
- SPACE spawns chosen attacker on chosen spawn lane
- 'q' pressed during game brings up pause menu where player can save the game
- ESC used to return from menu where player can save the game (because i am using ncurses, this can take up to 1-2 seconds)

## Implementace

### Pohyb útočníků
- Pro hledání cesty útočníků je použit algoritmus Breadth First Search se specializací pro každý druh útočníka.
- Výpočet cesty se provede při každém kroku.
- Pokud má útočník slowEffect, bude po dobu platnosti jeho pohyb ~1.5x zpomalen dokud efekt nevyprší.
- Útočník se pohybuje pouze pokud je na dalším políčku, kam se má posunout podle vypočítané cesty, volno. Pokud je obsazeno, stojí na místě.
- Slower attacker je pomalejší, ale má vyšší damage než základní útočník. Vždy směřuje směrem na nejbližší věž, nebo východ.
- Faster attacker je rychlejší, ale má nižší HP a damage. Vždy směřuje směrem k východu a při cestě se vyhíbá permieteru věží.
- Tyto specifikace fungují pomocípolymorfních tříd a jejich virtuálních metod, které specializují který prvek můžou navšítit.

### "AI" protihráče
- Věže jsou vytvářeny vždy na začátku hry podle dané obtížnosti.
- Pokud je daná obtížnost "easy", jde pouze o náhodné vytvoření věží na mapě.
- pokud je daná obtížnost "medium", věže jsou vytvořeny v okolí oblasti kudy by mohli útočníci chodit.
- pokud je daná obtížnost "hard", pouze se zvýší počet věží a použije se stejný algoritmus jako "medium".
- Pro zjištění přibližné cesty útočníků je použit algoritmus BFS od výstupu k vstupům.

### Interakce mezi entitami
- Nejprve je u každé entity zkontrolován perimetr, který je stanoven v definičním souboru a pokud nenašla entitu na kterou měla focus, 
zaútočí na novou nejližší nepřátelskou entitu a uloží si jeho ID jako současný focus. 
- útoky nejdřív provedou útočníci, které případně mohou i věže zničit.
- Poté zbylé věže provedou útoky na útočníky.

### Načítání z definice
- Definice mapy je uložena v souboru map1.txt a soubor je postupně načítán a vyvtářeno dvourozměrné pole, které je poté vykresleno na obrazovku
- Definice útočníků a věží jsou uloženy ve vlastních kontejnerech std::map, které mají klíče vždy názaev typu a hodnotou
uvnitř mapy je další mapa, která obsahuje dané parametry a klíčem je jejich název.
- K definicím přisupují konstruktory specifických entit pomocí reference a pouze si vyberou která data chtějí použít.
- Bližší info ve třídě CDefinitions.

### Ukládání a načítání hry
- Hra je načítána podobným způsobem jako načítá z definice. Mapa v mapě v mapě... 
- Funkce vykonávající tuto funkcionalitu jsou: 
CGame::loadFromSave(const std::string &path) a CGame::save()