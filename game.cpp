/*
 * Description: Console game that simulates "the storm" that appears in battle royale games
 * Input: keyboard input 
 *          w - move up     a - move left
 *          s - move down   d - move right
 *          enter - advance f - use short range wep (which is #)
 *          q - quit        u - shoot up with long range wep (which is !)
 *          h - shoot left  j - shoot below
 *          k - shoot right 
 * Output: Grid with players, obstacles, weapons, and storm
 *          Note -  In my version, the storm immediately destroyes obstacles and weapons (since they're much weaker)
 *                  but gives all of the players 2 ticks before destroying htem
 *                  Also, the short range weapon requires 2 shots to destory, 
 *                  and long range requires 1 shot to destory
 *                  There are 3 long range weapons and 5 short range weapons that spawn
 */

#include <iostream>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <string>

using namespace std;

#define curses
#define SAVELASTROUND true // if we want to save the last round set to true so it doesn't get autoerased

int GRIDX = 50;
int GRIDY = 14;

const int X = 0;
const int Y = 1;
const int COORDINATE = 2;
const int PLAYERCNT = 25;
const int NUM_SHORT_WEPS = 5;
const int NUM_LONG_WEPS = 3;
const int INT_TO_UPPER_ALPH = 65;   // num to add to 0 to make it represent 'A'
const bool ALIVE = true;
const bool DEAD = false;
const int ROUNDCOUNT = 100;

// uncomment when obstacles are needed
const int NUM_OF_OBSTACLES = 20; // declaring number of obstacles

/*
 * class_identifier: abstract class with virtual move functions and methods to set and get speed
 * constructors: none
 * public functions:    virtual void moveUp();
 *                      virtual void moveDown();
 *                      virtual void moveLeft();
 *                      virtual void moveRight();
 *                      void setSpeed()
 *                      int getSpeed() const
 * static members: none
 */

class move {
public:
    virtual void moveUp() = 0;
    virtual void moveDown() = 0;
    virtual void moveLeft() = 0;
    virtual void moveRight() = 0;
    void setSpeed(int newSpeed) {speed = newSpeed;}
    int getSpeed() const {return speed;}
private:
    int speed;
};

/*
 * class_identifier: initializes and allows changes to hp
 * constructors: health_t()
 * public functions:    int gethp() const
 *                      void sethp(int myhp)
 * static members: none
 */

class health_t {
public:
    health_t() {hp = 20;}               // default constructor
    int gethp() const {return hp;}      // getter for hp
    void sethp(int myhp) {hp = myhp;}   // setter for hp
private:
    int hp;
};

/*
 * class_identifier: initializes, prints, randomizes coordinates - both current and old x and y
 * constructors: coord_t()
 *               coord_t(int usrx, int usry)
 * public functions:    int getOldx() const
 *                      int getOldy() const
 *                      void setOldx(int usrOldx)
 *                      void setOldy(int usrOldy)
 *                      void print() const
 *                      void randomize()
 * static members: none
 */

class coord_t {
public:
    coord_t();
    coord_t(int usrx, int usry); // initializes x and oldx to usr x; same w/ y
    int getOldx() const {return oldx;}
    int getOldy() const {return oldy;}
    void rando();
    void setOldx(int usrOldx) {oldx = usrOldx;}
    void setOldy(int usrOldy) {oldy = usrOldy;}
    void print() const;
    void randomize();
public:
    int x;
    int y;
private:
    int oldx;
    int oldy;
};

// default constructor, setting vars to 0
coord_t::coord_t() {
    x = y = oldx = oldy = 0;
}

// constructor setting x and y to usr defined values, assuming they are appropriate
coord_t::coord_t(int usrx, int usry) {
    if (usrx <= GRIDX && usry <= GRIDY) {
        x = usrx;
        oldx = usrx;
        y = usry;
        oldy = usry;
    }
}
// prints coordinates
void coord_t::print() const {
#ifdef curses
    printw("[%i,%i]\n", x, y);
#else
    cout << "[" << x << "," << y << "]" << endl;
#endif
}

/*
 * function_identifier: assingns x and y to random positons within coord system
 * parameters: none
 * return value: none
 */
void coord_t::randomize() {
    oldx = x;
    oldy = y;
    
    x = rand() % GRIDX;    // mod operator to ensure x and y are within coord system
    y = rand() % GRIDY;
}

void coord_t::rando() {
    oldx = x;
    oldy = y;

    x = rand() % GRIDX;
    while (x <= GRIDX/2) x++; // ensures center is in the second half of grid
    y = rand() % GRIDY;
}
/*
 * class_identifier: declares and manipulates status, id, creationtime of all entities
 *                   this class is inherited by others frequently
 * constructors: ent_t()
 * public functions:    int getId() const
 *                      bool getStat() const
 *                      void setId(int usrId)
 *                      void setStat(bool usrStat)
 *                      void entprint()
 *                      void printCreationTime()
 * static members: entCnt
 */

class ent_t {
public:
    virtual char cprint();   // creating a virtual print function
    ent_t() {createEntity();}
    int getId() const {return id;}
    bool getStat() const {return status;}
    void setId(int usrId) {id = usrId;}
    void setStat(bool usrStat) {status = usrStat;}
    void entprint() const;
    void printCreationTime() const;
    coord_t pos;
    health_t hp;
    char symbol;
private:
    void createEntity();
protected:
    
private:
    int id;
    time_t creationTime;
    bool status;
    static int entCnt;
    
};

char ent_t::cprint() {
    // printw("%c", '^');
    return ' ';
}


/*
 * function_identifier: prints entity's infomation in both ncurses and not
 * parameters: none
 * return value: none
 */
void ent_t::entprint() const {
#ifdef curses
    printw("Entity: %i/%i\nStatus: %i\nCreated: ", id, entCnt, status);
    printCreationTime();
#else
    cout << "Entity: " << id << "/" << entCnt << endl << "Status: ";
    if (status == 1) cout << "Alive";
    else if (status == 0) cout << "Dead";
    else cout << "undefined";
    cout << endl << "Created: ";
    printCreationTime();
#endif
}

// prints creation time in both ncurses and not
void ent_t::printCreationTime() const {
#ifdef curses
    printw("%s", ctime(&creationTime));
#else
    cout << ctime(&creationTime);
#endif
}

/*
 * function_identifier: initializes entity values, used for constructor
 * parameters: none
 * return value: none
 */
void ent_t::createEntity() {
    id = ++entCnt;
    entCnt += 1;
    creationTime = time(NULL); // current time
    status = ALIVE;
}

int ent_t::entCnt = 0;

// used in part II
class world_t {
private:
    ent_t *ent;
};

/*
 * class_identifier: sets type for obstacle and prints obstacle info
 * constructors: none
 * public functions:    string getId() const
 *                      void setType(string usrType)
 *                      void printObstacle() const
 * static members: none
 */

class obstacle_t : public ent_t {
public:
    string getId() const {return id;} // id getter
    void setType(string usrType) {id = usrType;} // id setter
    void printObstacle() const;
    char cprint ();
    health_t hp;
private:
    string id;
};

char obstacle_t::cprint() {
    // printw("%c", '@');      //  printing the obstacle
    return '@';
}

/*
 * class_identifier: creates, sets, and gets variables used by weapon
 * constructors:    weapon_t()
 *                  weapon_t(int, int, int, int, string)
 * public functions:    int getAmmo() const
 *                      int getDmg() const
 *                      string getModel() const
 *                      int getMagCap() const
 *                      int getMagAmmo() const
 *                      void setAmmo(int usrAmmo)
 *                      void setDmg(int usrDmg)
 *                      void setModel(string usrModel)
 *                      void setMagCap(int usrMC)
 *                      void setMagAmmo(int usrMA)
 *                      void reload()
 *                      bool isReloading()
 *                      void print()
 * static members: pCnt
 */

class weapon_t : public ent_t {
public:
    weapon_t();
    weapon_t(int, int, int, int, string);
    int getAmmo() const {return ammo;}
    int getDmg() const {return dmg;}
    string getModel() const {return model;}
    int getMagCap() const {return magcap;}
    int getMagAmmo() const {return magammo;}
    void setAmmo(int usrAmmo) {ammo = usrAmmo;}
    void setDmg(int usrDmg) {dmg = usrDmg;}
    void setModel(string usrModel) {model = usrModel;}
    void setMagCap(int usrMC) {magcap = usrMC;}
    void setMagAmmo(int usrMA) {magammo = usrMA;}
    void reload();
    void reloadClick() {reloading-=1;} // decreasing value of reloading by 1
    bool isReloading() const;
    void print() const;
    char cprint();
private:
    int magcap;     // how much ammo the magazine can hold
    int magammo;    // how much ammo is currently in active magazine
    int reloading;  // counter
    int ammo;       // how much ammo is left
    int dmg;        // damage per hit
    string model;   // name/ type of weapon
};

// default constructor
weapon_t::weapon_t() {
    magcap = 10;
    magammo = 10;
    reloading = 0;
    ammo = 20;
    dmg = 5;
    model = "";
}


// constructor with paramaters, setting member vars to usr values
weapon_t::weapon_t(int usrCap, int usrMA, int usrA, int usrDMG, string usrModel) {
    magcap = usrCap;
    magammo = usrMA;
    reloading = 0;
    ammo = usrA;
    dmg = usrDMG;
    model = usrModel;
}


/*
 * function_identifier: calcs ammo needed to reload, performs reload if enough by adding/subtracting
 *                      from necessary vars. Decrements reloading 5 times to simulate time
 * parameters: none
 * return value: none
 */
void weapon_t::reload() {
    int needed = magcap - magammo;  // amount of ammo needed from total ammo
    if ((ammo - needed) >= 0) {     // ensuring there's enough ammo
        reloading = 5;
        ammo -= needed;             // subtract from total
        magammo += needed;          // add to gun ammo
        for (int j = reloading; j > 0; j--) {   // loops 5 times, calling reloadclick() which decrements relaod
#ifdef curses
            printw("%i", reloading);            // printing info to show function works, will remove later
#else
            cout << reloading;
#endif
            reloadClick();
        }
    }
    // in Part II of assingment
}

// checks if weapon is reloading by tapping into reloading member var
bool weapon_t::isReloading() const {
    if (reloading > 0) return true;
    else return false;
}

/*
 * function_identifier: prints out weapon info in both curses and non-curses mode
 * parameters: map obj, player obj, direction (which key is pressed)
 * return value: none
 */
void weapon_t::print() const {
#ifdef curses
    printw("\nWeapon Information: \nmodel: %s\nmagcap: %i\nmagammo: %i\nreloading: %i\nammo: %i\ndmg: %i\n", model.c_str(), magcap, magammo, reloading, ammo, dmg);
#else
    cout << "Weapon Information: " << endl << "model: " << model << endl <<
    "magcap: " << magcap << endl << "magammo: " << magammo << endl <<
    "reloading: " << reloading << endl << "ammo: " << ammo << endl << "dmg: " << dmg << endl;
#endif
}

char weapon_t::cprint() {
    // printw("%c", '&');
    return '&';
}

/*
 * function_identifier: print obstacle information
 * parameters: none
 * return value: none
 */
void obstacle_t::printObstacle() const {
#ifdef curses
    printw("Obstacle id: %s", id.c_str());
#else
    cout << "Obstacle Id: " << id << endl;
#endif
    entprint();
    pos.print();
}

class trigger_t : public ent_t {
public:
    trigger_t(char symbol = '#');
    trigger_t(string, char);
    string whatIDo() const {return info;}
    void setWhatIDo(string usrin) {info = usrin;} // info setter
    char getId() const {return id;}
    void setId(char usrid) {id = usrid;}          // id setter
    char cprint();
    void setSymbol(char symb);
private:
    string info;
    char id;
    char symbol;
};

// default constructor, initializing info and id
// trigger_t::trigger_t() {
//     info = "";
//     id = '?';
// }

void trigger_t::setSymbol(char symb) {
    this->symbol = symb;
}

trigger_t::trigger_t(char symbol){
    this->symbol = symbol;
}

char trigger_t::cprint() {
    // printw("%c", '#');
    return symbol;
}

// constructor, initializing info and id to usr vars
trigger_t::trigger_t(string usrInfo, char usrid) {
    info = usrInfo;
    id = usrid;
}

int max(int x, int y, int z, int k) {
    int largest = x;
    if (y > largest) largest = y;
    if (z > largest) largest = z;
    if (k > largest) largest = k;
    return largest;
}
int max2(int x, int y) {
    int largest = x;
    if (y > largest) largest = y;
    return largest;
}

class blank : public ent_t{
    public:
        char cprint() {
            return '_';
        }
    private:
};




/*
 * class_identifier: creates map and adds entities to it
 * constructors: map_t()
 * public functions:    void initGrid()
 *                      void print() const
 *                      void clearScreen() const
 *                      void addObstacle(coord_t&)
 *                      void addPlayer(coord_t&, int)
 *                      void addTrigger(coord_t& c, char ch)
 *                      void updatePosition(ent_t)
 * static members: none
 */

class map_t : public ent_t {
public:
    map_t(int urows = 50, int ucols = 14);
    void initGrid();  // iniitialize grid to blanks
    void print() const;
    void dynamicPrint();
    void clearScreen() const;
    void addObstacle(coord_t&);
    void addPlayer(coord_t&, int);
    void dynAddEnt(ent_t* e, coord_t&);
    void addTrigger(coord_t& c, char ch);
    void updatePosition(ent_t, ent_t*);
    // for testing purposes
    int getRows() const {return rows;}
    int getCols() const {return cols;}
    friend void update(map_t &m,ent_t*e, ent_t*p, int& pU, int& pR, int& pD, int& pL);          // updates the map with storm
    // friend void secondUpdate(map_t &m, ent_t*e, ent_t*p);
    void calcRadius();      // calculates and returns radius
    // ~map_t();               // adding a destructor to deallocate the new grid at end of program
    int radius;
    int dXR;    // x dist to the right of center
    int dXL;    // x dist to the left of center
    int dYU;    // y dist up of center
    int dYB;    // y dist down of center
    coord_t centerCoord;
    char **grid;
    char cprint();
// private:
    ent_t*** egrid;    // creating the new grid(a 2-d array of ent_t pointers)
    int rows;
    int cols;
};

char map_t::cprint() {
    symbol = 's';
    return 's';
}

void map_t::dynAddEnt(ent_t* e, coord_t& c){
    egrid[c.y][c.x] = e;                        // storing the entity in the array
}

void map_t::dynamicPrint() {
    for (int i = 0; i < GRIDY; i++) {
        for (int j = 0; j < GRIDX; j++) {
            if (egrid[i][j] != nullptr) 
                printw("%c", egrid[i][j]->cprint());
            else 
                printw("%c", ' ');
        }
        printw("\n");
    }
}

// defualt paramater, intiializing the grid
map_t::map_t(int urows, int ucols) {
    this->rows = urows;
    this->cols = ucols;

    // dynamically allocating 2d array of ent_t pointers
    egrid = new ent_t**[this->rows];
    for (int i = 0; i < urows; i++)
        egrid[i] = new ent_t*[this->cols];

    // creating 2d pointer array
    grid = new char*[this->rows];  // creates array of pointers
    for (int i = 0; i < rows; i++) {
        grid[i] = new char[this->cols];
    }

    centerCoord.rando();       // creates a random center

    dXR = GRIDX - centerCoord.x;
    dXL = centerCoord.x;
    dYU = centerCoord.y;
    dYB = GRIDY - centerCoord.y - 1;

    calcRadius();
    initGrid();
    // grid[centerCoord.y][centerCoord.x] = ' ';    // print out location
}

// map_t::~map_t() {
//     for (int i = 0; i < rows; i++) {
//         for (int j = 0; j < cols; j++) {
//             delete newgrid[i][j];
//         }
//         delete [] newgrid[i];               // deallocating data in each row
//     }
//     delete [] newgrid;                      // deallocating the final 1d array of pointers
// }

void map_t::calcRadius() {
    this->radius = max(dXR, dXL, dYU, dYB);
    // this -> radius = max2(dYU, dYB);
}

//  adds player to coordinates passed into function
void map_t::addPlayer(coord_t& c, int pid) {
    grid[c.y][c.x] = (char)(pid+INT_TO_UPPER_ALPH);     // casting into a character
}

void map_t::addObstacle(coord_t& c) {
    grid[c.y][c.x] = '@';
}

void map_t::addTrigger(coord_t& c, char chr) {
    grid[c.y][c.x] = chr;
}

// initialize grid to blanks
void map_t::initGrid() {
    for (int i = 0; i < this->rows; i++) {
        for (int j = 0; j < this->cols; j++) {
            egrid[i][j] = nullptr;
        }
    }
}







/*
 * function_identifier: update position of entity by setting old position to ' ', and the new position to 'A'
 * parameters: none
 * return value: none
 */
void map_t::updatePosition(ent_t myEnt, ent_t* blank) {
    // grid[myEnt.pos.getOldy()][myEnt.pos.getOldx()] = ' ';
    // cout << "old x" << myEnt.pos.getOldx() <<"old y" << myEnt.pos.getOldy() ;
    
    egrid[myEnt.pos.getOldy()][myEnt.pos.getOldx()] = blank;    // make it point to the blank
    // printw("%c", egrid[myEnt.pos.getOldy()][myEnt.pos.getOldx()]->cprint());
    // grid[myEnt.pos.y][myEnt.pos.x] = 'A'; // supposing only entity whose position can be updated is the player until part II
    egrid[myEnt.pos.y][myEnt.pos.x] = &myEnt;
    myEnt.pos.setOldx(myEnt.pos.x);
    myEnt.pos.setOldy(myEnt.pos.y);
}

/*
 * function_identifier: erases printed array for both ncurses and not
 * parameters: none
 * return value: none
 */
void map_t::clearScreen() const {
#ifdef curses
    clear();
#else
    for (int i = 0; i < rows + 1; i++){
        cout << "\33[2K\033[A\r";
    }
    cout << flush;
#endif
}



/*
 * class_identifier: creates, changes, and stores player info
 * constructors: player_t()
 * public functions:    void print();
 *                      void setPid(int usrPid);
 *                      void setPname(string usrPname);
 *                      int getPid() const;
 *                      string getPname() const;
 *                      void moveUp();
 *                      void moveDown();
 *                      void moveRight();
 *                      void moveLeft();
 *                      void storeLocation(map_t);
 *                      void updateStatus(map_t);
 *                      void printStatus();
 *                      void chooseLastAlive();
 * static members:      lastAlive
 *                      playerStatus[PLAYERCNT]
 *                      playerLocation[PLAYERCNT][3]
 */

class player_t : public ent_t, public move {
public:
    player_t();
    void print();
    char cprint();
    void setPid(int usrPid) {pid = usrPid;}
    void setPname(string usrPname) {name = usrPname;}
    int getPid() const {return pid;}
    string getPname() const {return name;}
    void moveUp();
    void moveDown();
    void moveRight();
    void moveLeft();
    void storeLocation(map_t);
    void updateStatus(map_t);
    void printStatus() {printw("%i status: %i\n", pid, playerStatus[pid]);}
    void chooseLastAlive();
    void removePlayer();
public:
    weapon_t wep;
    static int lastAlive;                       // randomly chosen last char alive
    static bool playerStatus[PLAYERCNT];        // 1d array to store player if player is dead or alive
    static double playerLocation[PLAYERCNT][3]; // array common to all players to store location
                                                // stores: id, x, y
    health_t hp;
private:
    
    string name;
    int pid;
    static int pCnt;
};

int player_t::pCnt = 0;

double player_t::playerLocation[PLAYERCNT][3] = {{0}};     //initialize 2d array to 0s
bool player_t::playerStatus[PLAYERCNT] = {ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, 
ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, 
ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE};          //initialize all players to be alive
int player_t::lastAlive = 0;                              

// defualt constructor setting pid and name
player_t::player_t() {
    pid = pCnt++;
    string spid = to_string(pid);   // pid as string
    name = "Player " + spid;
}

// if player is inside of storm, their status changes to DEAD
void player_t::updateStatus(map_t m) {
    if (m.egrid[pos.y][pos.x]->symbol == 's') {         // checks if its in the storm
        this->playerStatus[this->pid] = DEAD;           // sets it to DEAD in that case
        
    }       
}

void player_t::removePlayer( ){
    this->playerStatus[this->pid] = DEAD;
}
// randomly selects a player that is alive
// this function is called when choosing a winner in case of draw
void player_t::chooseLastAlive() {
    bool selected = false;
    int pid = 0; 
    while (selected == false) {
        int pid = (rand()%(PLAYERCNT+1));
        if (this->playerStatus[pid] == ALIVE) {
            lastAlive = pid;
            return;
        }
    }
}

/*
 * function_identifier: stores player info into 2d array common to all players
 * parameters: none
 * return value: none
 */
void player_t::storeLocation(map_t m) {
    playerLocation[pid][0] = pid;           // storing the character
    playerLocation[pid][1] = pos.x;         // storing x   
    playerLocation[pid][2] = pos.y;         // storing y
}

/*
 * function_identifier: sets player's old position to current pos
 *                      and updated current position as long as it's within boundaries
 * parameters: none
 * return value: none
 */
void player_t::moveUp() {
    pos.setOldx(pos.x);         // copies current position to old so that updatePos() can make it blank
    pos.setOldy(pos.y);
    if (pos.y >= 1) pos.y -= 1; // checks that player doesn't leave boundaries
}

void player_t::moveDown() {
    pos.setOldx(pos.x);
    pos.setOldy(pos.y);
    if (pos.y < GRIDY-1) pos.y += 1;
}

void player_t::moveRight() {
    pos.setOldx(pos.x);
    pos.setOldy(pos.y);
    if (pos.x < GRIDX-1) pos.x += 1;
}

void player_t::moveLeft() {
    pos.setOldx(pos.x);
    pos.setOldy(pos.y);
    if (pos.x >= 1) pos.x -= 1;
}

/*
 * function_identifier: updates + formats player id and name in case it was
 *                      changed by a setter and prints player info
 * parameters: none
 * return value: none
 */
void player_t::print() {
    string spid = to_string(pid);   // in case pid changes after obj is created
    name = "Player " + spid;        // reflect changes to name
#ifdef curses
    printw("%s\n", name.c_str());
    entprint();
    printw("Player position: ");
    pos.print();
#else
    cout << name << endl;           // prints player name + id
    entprint();                     // prints metadata
    cout << "Player position: "; pos.print(); // prints position
#endif  
}

char player_t::cprint() {
    return (char)(pid+INT_TO_UPPER_ALPH);     // PID casted into a character
}

class empty_t: public ent_t {
    public: 
        char cprint() {return ' ';}
};

empty_t e;

void updatePos(map_t &map, player_t &p){
    map.egrid[p.pos.getOldy()][p.pos.getOldx()] = &e;
    map.egrid[p.pos.y][p.pos.x] = &p;
    p.pos.setOldx(p.pos.x);
    p.pos.setOldy(p.pos.y);
}

/*
 * function_identifier: moves player on map, depending on key user has pressed
 * parameters: map_t &map, player_t &p, obstacle_t*o, trigger_t *shortWep, trigger_t* longWep, int direction, bool& haveShort, bool& haveLong
 * return value: none
 */
void makemove(map_t &map, player_t &p, obstacle_t*o, trigger_t *shortWep, trigger_t* longWep, int direction, bool& haveShort, bool& haveLong) {
#ifdef curses
    bool obstacle = false;
    bool player = false;
    
    if (direction == 119) {                                             // checking W
        if (p.pos.y <= 0) {                                             // prevent going out of bounds
            return;
        }
        for (int i = 0; i < NUM_OF_OBSTACLES; i++){                     // looking at obstacles around
            if (map.egrid[p.pos.y - 1][p.pos.x] == (o+i)) {
                obstacle = true;
            }
        }
        for (int i = 0; i< PLAYERCNT; i++) {                            // looking around for players
            if (map.egrid[p.pos.y - 1][p.pos.x] == (&p+i)) {
                player = true;
            }
        }

        if (!obstacle && !player) {                                    // if no player or obstacle
            for (int i = 0; i < NUM_SHORT_WEPS; i++) {                 // check if the next move is a short weapon
                if (map.egrid[p.pos.y - 1][p.pos.x] == (shortWep+i)){
                    haveShort = true;                                   // make it known that player has short wep
                    break;
                }
            }
            for (int i = 0; i < NUM_LONG_WEPS; i++) {                   // check if next move picks up long weapon
                if (map.egrid[p.pos.y - 1][p.pos.x] == (longWep+i)){
                    haveLong = true;                                    // make it known player has long wep
                    break;
                }
            }
            p.moveUp();                                                 // move player
            updatePos(map, p);                                          // update player's position
        }
    } else if (direction == 115) {                                      // Checking S
        if (p.pos.y >= GRIDY-1) {                                       // prevent going out of bounds
            return;
        }
        for (int i = 0; i < NUM_OF_OBSTACLES; i++){
            if (map.egrid[p.pos.y + 1][p.pos.x] == (o+i)) {
                obstacle = true;
            }
        }
        
         for (int i = 0; i< PLAYERCNT; i++) {
            if (map.egrid[p.pos.y + 1][p.pos.x] == (&p+i)) {
                player = true;
            }
        }
        if (!obstacle && !player) {
            for (int i = 0; i < NUM_SHORT_WEPS; i++) {
                if (map.egrid[p.pos.y + 1][p.pos.x] == (shortWep+i)){
                    haveShort = true;
                    break;
                }
            }
            for (int i = 0; i < NUM_LONG_WEPS; i++) {
                if (map.egrid[p.pos.y + 1][p.pos.x] == (longWep+i)){
                    haveLong = true;
                    break;
                }
            }
            p.moveDown();
            updatePos(map, p);
        }
    } else if (direction == 100) {                                  // checking D
        for (int i = 0; i < NUM_OF_OBSTACLES; i++){
            if (map.egrid[p.pos.y][p.pos.x+1] == (o+i)) {
                obstacle = true;
            }
        }
        for (int i = 0; i< PLAYERCNT; i++) {
            if (map.egrid[p.pos.y][p.pos.x+1] == (&p+i)) {
                player = true;
            }
        }
        if (!obstacle && !player) {
            for (int i = 0; i < NUM_SHORT_WEPS; i++) {
                if (map.egrid[p.pos.y][p.pos.x+1] == (shortWep+i)){
                    haveShort = true;
                    break;
                }
            }
            for (int i = 0; i < NUM_LONG_WEPS; i++) {
                if (map.egrid[p.pos.y][p.pos.x+1] == (longWep+i)){
                    haveLong = true;
                    break;
                }
            }
            p.moveRight();
            updatePos(map, p);
        }
    } else if (direction == 97) {                                       // checking A
        for (int i = 0; i < NUM_OF_OBSTACLES; i++){
            if (map.egrid[p.pos.y][p.pos.x - 1] == (o+i)) {
                obstacle = true;
            }
        }
        for (int i = 0; i< PLAYERCNT; i++) {
            if (map.egrid[p.pos.y][p.pos.x - 1] == (&p+i)) {
                player = true;
            }
        }
        if (!obstacle && !player){
            for (int i = 0; i < NUM_SHORT_WEPS; i++) {
                if (map.egrid[p.pos.y][p.pos.x-1] == (shortWep+i)){
                    haveShort = true;
                    break;
                }
            }
            for (int i = 0; i < NUM_LONG_WEPS; i++) {
                if (map.egrid[p.pos.y][p.pos.x-1] == (longWep+i)){
                    haveLong = true;
                    break;
                }
            }
            p.moveLeft();
            updatePos(map, p);
        }
    } else if (direction == 102 && haveShort) {
        for (int i = 0; i < NUM_OF_OBSTACLES; i++){
            // if the obstacle is above, below, left, or right of us
            
            if (map.egrid[p.pos.y-1][p.pos.x] == (o+i)){            // OBS is ABOVE US
                if ((o+i)->hp.gethp()>0){
                    (o+i)->hp.sethp((o+i)->hp.gethp() - 20);        // decrease the hp by 10 of that obstacle
                    break;
                } else {
                    map.egrid[p.pos.y-1][p.pos.x] = nullptr;        // make it not point to anything(delete the obstacle)
                    break;
                } 
            }   
            else if (map.egrid[p.pos.y+1][p.pos.x] == (o+i)) {      // OBS is BELOW US
                if ((o+i)->hp.gethp()>0){
                    (o+i)->hp.sethp((o+i)->hp.gethp() - 20);        // decrease the hp by 10 of that obstacle
                    break;
                } else {
                    map.egrid[p.pos.y+1][p.pos.x] = nullptr;        // make it not point to anything(delete the obstacle)
                    break;
                } 
            }
            else if (map.egrid[p.pos.y][p.pos.x - 1] == (o+i)) {    // OBS is LEFT of US
                if ((o+i)->hp.gethp()>0){
                    (o+i)->hp.sethp((o+i)->hp.gethp() - 20);        // decrease the hp by 10 of that obstacle
                    break;
                } else {
                    map.egrid[p.pos.y][p.pos.x - 1] = nullptr;        // make it not point to anything(delete the obstacle)
                    break;
                } 
            } 
            else if (map.egrid[p.pos.y][p.pos.x + 1] == (o+i)) {
                if ((o+i)->hp.gethp()>0){
                    (o+i)->hp.sethp((o+i)->hp.gethp() - 20);        // decrease the hp by 10 of that obstacle
                    break;
                } else {
                    map.egrid[p.pos.y][p.pos.x + 1] = nullptr;        // make it not point to anything(delete the obstacle)
                    break;
                } 
            } 
            
            else {
                continue;
            }
        }

        // ------------------------------- PLAYERS -------------------------------
        for (int i = 0; i < PLAYERCNT; i++){
            // if the obstacle is above, below, left, or right of us
            if (map.egrid[p.pos.y-1][p.pos.x] == (&p+i)){            // PLAYER is ABOVE US
                if ((&p+i)->hp.gethp()>0){
                    (&p+i)->hp.sethp((&p+i)->hp.gethp() - 20);        // decrease the hp by 10 of that obstacle
                    break;
                } else {
                    (&p+i)->removePlayer();
                    map.egrid[p.pos.y-1][p.pos.x] = &e;        // make it not point to anything(delete the obstacle)
                    
                    break;
                } 
            }   
            else if (map.egrid[p.pos.y+1][p.pos.x] == (&p+i)) {      // PLAYER is BELOW US
                if ((&p+i)->hp.gethp()>0){
                    (&p+i)->hp.sethp((&p+i)->hp.gethp() - 20);        // decrease the hp by 10 of that obstacle
                    break;
                } else {
                    (&p+i)->removePlayer();
                    map.egrid[p.pos.y+1][p.pos.x] = &e;        // make it not point to anything(delete the obstacle)
                         // make it not point to anything(delete the obstacle)
                    
                    break;
                } 
            }
            else if (map.egrid[p.pos.y][p.pos.x - 1] == (&p+i)) {    // PLAYER is LEFT of US
                if ((&p+i)->hp.gethp()>0){
                    (&p+i)->hp.sethp((&p+i)->hp.gethp() - 20);        // decrease the hp by 10 of that obstacle
                    break;
                } else {
                    (&p+i)->removePlayer();
                    map.egrid[p.pos.y][p.pos.x - 1] = &e;        // make it not point to anything(delete the obstacle)
                    
                    break;
                } 
            } 
            else if (map.egrid[p.pos.y][p.pos.x + 1] == (&p+i)) {
                if ((&p+i)->hp.gethp()>0){
                    (&p+i)->hp.sethp((&p+i)->hp.gethp() - 20);        // decrease the hp by 10 of that obstacle
                    
                    break;
                } else {
                    (&p+i)->removePlayer();
                    map.egrid[p.pos.y][p.pos.x + 1] = &e;        // make it not point to anything(delete the obstacle)
                    
                    break;
                } 
            } else {
                continue;
            }
        }
    }
    else if (direction == 117 && haveLong) {             // checking that player has a long range weapon
        bool foundObs = false;
        bool foundPlayer = false;
        for (int i = p.pos.y; i >= 1; i--) {                // CHECKING OBSTACLE ABOVE
            for (int j = 0; j < NUM_OF_OBSTACLES; j++){
                if (map.egrid[i][p.pos.x] == (o+j)) {        // if there is an obstacle ANYWHERE above the player
                    map.egrid[i][p.pos.x] = &e;
                    foundObs = true;
                    break;
                }   
                if (i==1 && map.egrid[i-1][p.pos.x] == (o+j)){
                    map.egrid[i-1][p.pos.x] = &e;
                    foundObs = true;
                    break;
                }
                if (foundObs) break;                               // break out of double for loop
            }
            if (foundObs) break;  
            for (int j = 0; j < PLAYERCNT; j++){
                if (map.egrid[i-1][p.pos.x] == (&p+j)) {        // if there is an player ANYWHERE above the player
                    (&p+j)->removePlayer();
                    map.egrid[i-1][p.pos.x] = &e;
                    foundPlayer = true;
                    break;
                }   
                if (foundPlayer) break;                               // break out of double for loop
            }
            if (foundPlayer) break;   
        }
    } else if (direction == 106 && haveLong) {                // BELOW
        bool foundObs = false;
        bool foundPlayer = false;

         for (int i = p.pos.y; i <= (GRIDY-2); i++) {             // CHECKING OBSTACLE
            for (int j = 0; j < NUM_OF_OBSTACLES; j++){
                if (map.egrid[i][p.pos.x] == (o+j)) {        // if there is an obstacle ANYWHERE above the player
                    map.egrid[i][p.pos.x] = &e;
                    foundObs = true;
                    break;
                }
                if (i==(GRIDY-1) && map.egrid[i+1][p.pos.x] == (o+j)){
                    map.egrid[i+1][p.pos.x] = &e;
                    foundObs = true;
                    break;
                }   
                if (foundObs) break;                            // break out of double for loop
            }
            if (foundObs) break;
             for (int j = 0; j < PLAYERCNT; j++){
                if (map.egrid[i+1][p.pos.x] == (&p+j)) {        // if there is an player ANYWHERE above the player
                    (&p+j)->removePlayer();
                    map.egrid[i+1][p.pos.x] = &e;
                    foundPlayer = true;
                    break;
                }   
                if (foundPlayer) break;                               // break out of double for loop
            }
            if (foundPlayer) break;
        }
    } else if (direction == 107 && haveLong ) {                           // right
        bool foundObs = false;
        bool foundPlayer = false;
        
        for (int i = p.pos.x; i <= GRIDX; i++) {             // CHECKING OBSTACLE
            for (int j = 0; j < NUM_OF_OBSTACLES; j++){
                if (map.egrid[p.pos.y][i] == (o+j)) {        // if there is an obstacle ANYWHERE above the player
                    map.egrid[p.pos.y][i] = &e;
                    foundObs = true;
                    break;
                }                                  // break out of double for loop
                if (foundObs) break;
            }
            if (foundObs) break;
            for (int k = 0; k < PLAYERCNT; k++) {
                if (map.egrid[p.pos.y][i+1] == (&p+k)) {
                        (&p+k)->removePlayer();
                        map.egrid[p.pos.y][i+1] = &e;
                        foundPlayer = true;
                        break;
                }
                if (foundPlayer) break;
            }

            if (foundPlayer) break;
           
        }
        
    } else if (direction == 104 && haveLong) {                              // left
        bool foundObs = false;
        bool foundPlayer = false;
        for (int i = p.pos.x; i >= 0; i--) {                    // CHECKING OBSTACLE
            for (int j = 0; j < NUM_OF_OBSTACLES; j++){
                if (map.egrid[p.pos.y][i] == (o+j)) {           // if there is an obstacle ANYWHERE above the player
                    map.egrid[p.pos.y][i] = &e;
                    foundObs = true;
                    break;
                }   
                if (foundObs) break;                               // break out of double for loop
            }
            if (foundObs) break;
            for (int j = 0; j < PLAYERCNT; j++){
                if (map.egrid[p.pos.y][i-1] == (&p+j)) {        // if there is an player ANYWHERE above the player
                    (&p+j)->removePlayer();
                    map.egrid[p.pos.y][i-1] = &e;
                    foundPlayer = true;
                    break;
                }   
                if (foundPlayer) break;                               // break out of double for loop
            }
            if (foundPlayer) break;  
        }
        
    }
#endif
}

// ends curses lib and interface
void endCurses() {
#ifdef curses
    refresh();
    getch();    // asks for one final character
    endwin();   // closes ncurses lib
#endif
}

// initializes curses lib and prints out initial statements/instructions
void initCurses() {
#ifdef curses
    initscr();
    keypad(stdscr, TRUE); // accepts arrow keys
    noecho();
    raw(); //prevents exiting with commands
#endif
}

/*
 * function_identifier: checks when player steps on trigger
 * parameters: player obj, trigger obj
 * return value: true if player is on object, false otherwise
 */

bool winRnd(player_t p, trigger_t t) {
    // looking to see if player steps on trigger
    if ((p.pos.x == t.pos.x) && (p.pos.y == t.pos.y)) return true;
    else return false;
}

/*
 * function_identifier: counts and returns number of players alive
 * parameters: player_t *p
 * return value: number alive
 */

int numAlive(player_t *p) {
    int alive = 0;
    for (int i = 0; i < PLAYERCNT; i++) {
        if (p[0].playerStatus[i] == ALIVE)
            alive++;
    }
    return alive;
}

/*
 * function_identifier: searches and returns the id of last player alive
 * parameters: player_t *p
 * return value: player id
 */

int whoAlive(player_t *p) {
    int pid;
    for (int i = 0; i < PLAYERCNT; i++) {
    if (p[0].playerStatus[i] == ALIVE)
        pid = i;
    }
    return pid;
}

/*
 * function_identifier: checks if winner exists, and decides who it is
 * parameters: player_t *p, map_t m, int lastAlive
 * return value: true if there is a winner, false if no winner yet
 */

bool checkVictor(player_t *p, map_t m, int lastAlive) {
    if (numAlive(p)==1) {
        printw("Victory Royale!\n");
        printw("Player '%c' wins!\n", whoAlive(p)+INT_TO_UPPER_ALPH);
        printw("Game Over!\n");
        return true;
    } else if (numAlive(p) == 0) {
        printw("Victory Royale!\n");
        printw("player '%c' nearly took the L, but won!\n", lastAlive+INT_TO_UPPER_ALPH);
        printw("Game Over!\n");
        return true;
    }
    return false;
}

// returns whether or not the current cell contains a player
// used for determining what the storm destroys
bool isPlayer(ent_t*** egrid, player_t*p, int x, int y){
    for (int i = 0; i < PLAYERCNT; i++){
        if (egrid[y][x] == (p+i)) return true;
    }
    return false;
}

/*
 * function_identifier: advances the storm posiiton on the map
 * parameters: map_t &m, ent_t*e, player_t*p, int& pU, int& pR, int& pD, int& pL
 * return value: none
 */
void update(map_t &m, ent_t*e, player_t*p, int& pU, int& pR, int& pD, int& pL) {
    int performedUp = 0; int performedDown = 0; int performedRight = 0; int performedLeft = 0;
    
    if(m.dXR == m.radius) {                                     // remove right
        pR++;
        for (int i = 0; i < m.rows; i++) {
            if (pR > 2) {
                m.egrid[i][m.centerCoord.x + m.dXR+2] = e;              // perform the second round of the storm, to damage the chars that weren't initially
            }
            if (isPlayer(m.egrid, p,  m.centerCoord.x + m.dXR, i)) {   // if there is a player
                    continue;                                           
            } else {
                m.egrid[i][m.centerCoord.x + m.dXR] = e;               // destroy it
            } 
        }
        m.dXR -= 1;
    }
    if (m.dXL == m.radius) {                                    // remove left
        pL++;
        for (int i = 0; i < m.rows; i++) {
            if (pL>2) {
                m.egrid[i][m.centerCoord.x - (2+m.dXL)] = e;  
            }
            if (isPlayer(m.egrid, p,  m.centerCoord.x - m.dXL, i)) {
                continue;
            }
            else {
             m.egrid[i][m.centerCoord.x - m.dXL] = e;
            }
        }
        m.dXL -= 1;
    }
    if (m.dYU == m.radius) {                                    // remove up
        pU++;
        for (int i = 0; i < m.cols; i++) {
            if (pU>2) {
                   m.egrid[m.centerCoord.y - (m.dYU+2)][i] = e;  
            }
            if (isPlayer(m.egrid, p,  i, m.centerCoord.y - m.dYU)){
                continue;
            } else {
                m.egrid[m.centerCoord.y - m.dYU][i] = e;
         
            }
        }
        m.dYU -= 1;
    }
    if (m.dYB == m.radius) {                                    // remove down
    pD++;
        for (int i = 0; i < m.cols; i++) {
            if (pD>2) {
                m.egrid[2+ m.centerCoord.y + m.dYB][i] = e;  
            }
            if (isPlayer(m.egrid, p,  i, m.centerCoord.y + m.dYB)) {
                continue;
            } else {
                m.egrid[m.centerCoord.y + m.dYB][i] = e;
         
            }
        }
        m.dYB -= 1;
    }
    m.radius -= 1;
}

/*
 * function_identifier: "client code" where objects are created and added to the game
 *                       there is also a section to test methods of all the classes
 * parameters: none
 * return value: 0
 */

int main(int argc, char* argv[]) {
    // pre-game initialization ---------------------------------------------
    initCurses();
    int round = 0;
    bool haveShortWep = false;              // don't have short
    bool haveLongWep = false;               // or long weapon initially
    int pU = 0; int pL = 0; int pR = 0; int pD = 0;

    // changes size of map to custom value
    if (argc == 3) {
        GRIDX = atoi(argv[1]);
        GRIDY = atoi(argv[2]);
    }

    srand(time(NULL));                      // creates random seed rand() function

    map_t map(GRIDY, GRIDX);                // generating map and random center coord
    player_t p[PLAYERCNT];                  // 25 player objects
    obstacle_t o[NUM_OF_OBSTACLES];
    trigger_t shortWep[NUM_SHORT_WEPS];
    trigger_t longWep[NUM_LONG_WEPS];
    for (int i = 0; i < NUM_LONG_WEPS; i++)
        longWep[i].setSymbol('!');          // setting the long rage weapon symbol

    for (int i = 0; i < NUM_OF_OBSTACLES; i++) {
        o[i].pos.randomize();               // randomize the obstacles
        map.dynAddEnt(&(o[i]), o[i].pos);   // add them to the map
    }
    // initializing game arrays---------------------------------------------
    for (int i = 0; i < PLAYERCNT; i++) {
        p[i].pos.randomize();               // sets player to random position
        map.dynAddEnt(&(p[i]), p[i].pos);
    }
    for (int i = 0; i<NUM_SHORT_WEPS; i++){
        shortWep[i].pos.randomize();
        map.dynAddEnt(&(shortWep[i]), shortWep[i].pos);
    }
    for (int i = 0; i<NUM_LONG_WEPS; i++){
        longWep[i].pos.randomize();
        map.dynAddEnt(&(longWep[i]), longWep[i].pos);
    }
    
    // main game loop start ------------------------------------------------
    char input = ' ';

    // //printing game info
    printw("Center: (%i, %i)\n", map.centerCoord.x, map.centerCoord.y);
    printw("Victor's Battle Royale!\n");
    printw("Use wasd to move, q to quit - # is the short range weapon ! is the long range\n");
    map.dynamicPrint();
    
    // main game loop, terminated by press of 'q'
    while (input != 'q') {
        input = getch();
        p[0].chooseLastAlive();
        int lastAlive = p[0].lastAlive;
        // only move if player is alive
        if (p[0].playerStatus[0] == ALIVE) {
            makemove(map, p[0], o, shortWep, longWep, input, haveShortWep, haveLongWep);     // updates map and player obj based on usr input
        }

        if (input == '\n') {
            map.clearScreen();
            printw("Center: (%i, %i)\n", map.centerCoord.x, map.centerCoord.y);
            printw("Victor's Battle Royale!\n");
            printw("Use wasd to move, q to quit - # is the short range weapon ! is the long range\n");
            
            update(map, &map, p, pU, pR, pD, pL);
            
            map.dynamicPrint();
            
            // updates status of all players (either dead or alive) after the map gets updated with new storm iteration
            for (int i = 0; i < PLAYERCNT; i++){
                p[i].updateStatus(map);
                // printw("%i ", p[0].playerStatus[i]);
            }
            printw("Round %i Complete. Press Enter to Continue\n", (round+1));
            // only increments round if user presses enter
            round++;
        // ensures immediate termination rather than waiting for loop to end
        } else if (input == 'q'){
            break;
        // user input validation
        } else if ( input != 'w' && input != 'd' && input != 'a' && 
                    input != 's' && input != 'f' && input != 'u' &&
                    input != 'k' && input != 'j' && input != 'h'){
            printw("Error! Only Press Enter.\n");
            break;
        }
        // checking for victory status
        if (checkVictor(p, map, lastAlive)){
            endCurses();
            return 0;
        };  
    } 
    // end main game loop ----------------------------------------------------

    endCurses();
    return 0;
}

