//function declarations
void UpdateSimulation(void);
void UpdateNews(int vVictory, int wVictory);
void InitializeSimulation(void);
void RenderSimulation(void);
uint8_t MainMenu(void);
void InitializeMap(void);
void RenderSelected(void);
void HandleKeys(void);
void SaveData(void);
void LoadData(void);
void DispNews(void);


//structs and things

//general game data, add more as needed
typedef struct {
    long ticksplayed;
    int numinfected;
    int numdead;
    int numrecovered;
    uint8_t selected;
    bool canpress;
    uint8_t toupdate;
    uint8_t cursorx;
    uint8_t cursory;
    bool cursoractive;
    //virus and world victory
    uint8_t vVictory;
    uint8_t wVictory;
    uint8_t totaldead;
    uint8_t dnapoints;
    //control view
    uint8_t viewmode;
    //news
    char news[100];
    uint8_t newsloops;
    uint8_t initialport;
    uint8_t endport;
    bool connectionmade;
} game_t;

//virus data, will need more later as well
typedef struct {
    //a number out of 100 probably
    uint8_t probinfection;
    uint8_t probdeath;
    //resistance is basically the same as probdeath
    uint8_t vResistance;
    uint8_t vSpeed;
    uint8_t vUnrest;
    char name[20];
} virus_t;

//region data (to be added later)
typedef struct {
    //"north america" is 13 characters long, plus 1 for the null terminating byte
    char name[14];
    uint8_t *data;
    uint16_t x;
    uint8_t y;
    gfx_sprite_t *spr;
    //gfx_sprite_t *outlinespr;
    uint16_t populationdensity;
    int squares;
    int squaresinfected;
    int squaresdead;
    int squaresrecovered;
    //each region has different probability of infection
    //increase probinfection with mutations
    uint8_t probinfection;
} region_t;

typedef struct {
    uint8_t x;
    uint8_t y;
    bool closed;
    uint8_t region;
    uint8_t closethreshold;
} port_t;
