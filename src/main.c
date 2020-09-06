 ////////////////////////////////////////
// {Pandemic CE} {0.0.1}
// Author: epsilon5, EverydayCode
// License: NA
// Description: Plague, Inc. like game for the TI-84 Plus CE
////////////////////////////////////////

//for the sprites
#include "sprites/sprites.h"

//OPTIX Graphics Libraries by epsilon5, stable release not available. Contact epsilon5 @ https://www.cemetech.net/downloads/users/epsilon5.
#include "sprites/optix.h"
/* Keep these headers */
#include <tice.h>

#include "contagion.h"

/* Standard headers - it's recommended to leave them included */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
/* Other available headers */
// including stdarg.h, setjmp.h, assert.h, ctype.h, float.h, iso646.h, limits.h, errno.h
/* Available libraries */
// including lib/ce/graphc.h, lib/ce/fileioc.h, and lib/ce/keypadc.h
// Sample: #include <lib/ce/graphc.h>
#include <graphx.h>
#include <fileioc.h>
#include <keypadc.h>
#include <intce.h>
#include <tice.h>
#include <math.h>


char *vname = NULL;

game_t game;

virus_t virus;

//going based on continent here
region_t region[7];

port_t port[22];
//functions

void SaveData(void) {
    ti_var_t slota;
    uint8_t i;
    ti_CloseAll();
    slota = ti_Open("CNTGNDAT", "w+");
    ti_Write(&game, sizeof(game), 1, slota);
    ti_Write(&virus, sizeof(virus), 1, slota);
    //ti_Write(&transport, sizeof(transport), 1, slota);
    for (i = 0; i < 7; i++) ti_Write(region[i].data, region[i].spr->width * region[i].spr->height, 1, slota);
    //don't need this line, everything else will be a constant and initialized
    //ti_Write(&region, sizeof(region), 1, slota);
    for (i = 0; i < 7; i++) free(region[i].spr);
}

void LoadData(void) {
    ti_var_t slota;
    uint8_t i;
    ti_CloseAll();
    if (slota = ti_Open("CNTGNDAT", "r")) {
        //need some check here to initialize everything
        //probably problematic to read directly to the pointer that we had last time, need to fix that
        ti_Read(&game, sizeof(game), 1, slota);
        ti_Read(&virus, sizeof(virus), 1, slota);
        //ti_Read(&transport, sizeof(transport), 1, slota);
        //11 kb of data...
        for (i = 0; i < 7; i++) ti_Read(region[i].data, region[i].spr->width * region[i].spr->height, 1, slota);
    }
}

//later a function to infect a specific point on the map will come in handy (for the updating, and for the planes and such)
//returns true if successful
bool InfectCoordinate(int x, int y) {
    uint8_t i;
    int infectsquare;
    for (i = 0; i < 7; i++) {
        if (x > region[i].x && y > region[i].y && (y - region[i].y < region[i].spr->height) && (x - region[i].x < region[i].spr->width)) {
            infectsquare = (y - region[i].y) * region[i].spr->width + (x - region[i].x);
            if (region[i].data[infectsquare] == 255) {
                region[i].data[infectsquare] = 224;
                return true;
            }
        }
    }
    return false;
}

//will be the whole bottom toolbar now
void RenderBottomToolbar(void) {
    char str1[100];
    region_t *r = &region[game.selected];
    //pass victory percentages and infection percentages per region
    gfx_SetTextScale(1, 1);
    gfx_SetColor(255);
    gfx_HorizLine(0, 209, 320);
    //needs to be virus name r->vname
    gfx_SetColor(0);
    //fill in the bottom of the screen
    gfx_FillRectangle(0, 210, 320, 30);
    //for the selected region
    if (game.viewmode == 1) sprintf(str1, "%s -> %s", region[port[game.initialport].region].name, region[port[game.endport].region].name);
    else if (game.viewmode == 0) strcpy(str1, region[game.selected].name);
    if (game.viewmode == 0) {
        //gfx_ScaledTransparentSprite_NoClip(r->outlinespr, r->x * 2, r->y * 2, 2, 2);
        gfx_SetColor(224);
        gfx_Rectangle(r->x * 2, r->y * 2, r->spr->width * 2, r->spr->height * 2);
        //gfx_Rectangle(r->x * 2 + 1, r->y * 2 + 1, r->spr->width * 2 - 2, r->spr->height * 2 - 2);
    //make an outline
        gfx_SetColor(0);
    }
    gfx_FillRectangle(160 - (gfx_GetStringWidth(str1) + 10) / 2, 197, gfx_GetStringWidth(str1) + 10, 13);
    gfx_PrintStringXY(str1, 160 - gfx_GetStringWidth(str1) / 2, 200);
    gfx_SetColor(255);
    gfx_HorizLine(160 - (gfx_GetStringWidth(str1) + 10) / 2, 197, gfx_GetStringWidth(str1) + 10);
    gfx_VertLine(160 - (gfx_GetStringWidth(str1) + 10) / 2, 197, 13);
    gfx_VertLine(160 + (gfx_GetStringWidth(str1) + 10) / 2, 197, 13);
    //x, y, width, x, height
    //gfx_FillRectangle(5,225,100,10);
    //gfx_FillRectangle(215,225,100,10);
    //draw the outlines on top
    //world
    gfx_SetColor(22);
    gfx_FillRectangle(216,225,game.wVictory,10);
    //virus
    gfx_SetColor(224);
    gfx_FillRectangle(5,225,game.vVictory,10);
    gfx_SetColor(255);
    gfx_Rectangle(5,225,100,10);
    gfx_Rectangle(216,225,100,10);
    gfx_VertLine(108, 210, 30);
    gfx_VertLine(212, 210, 30);
    //percent victory text on progress bars?
    sprintf(str1, "%s : %d%s", virus.name, game.vVictory, "%");
    gfx_PrintStringXY(str1,5,215);
    sprintf(str1, "%d%s : WORLD", game.wVictory, "%");
    gfx_PrintStringXY(str1,315 - gfx_GetStringWidth(str1),215);
    if (game.viewmode == 0) {
        sprintf(str1, "%d%s infected", r->squaresinfected * 100 / r->squares, "%");
        gfx_PrintStringXY(str1, 160 - gfx_GetStringWidth(str1) / 2, 215);
        sprintf(str1, "%d%s dead", r->squaresdead * 100 / r->squares, "%");
        gfx_PrintStringXY(str1, 160 - gfx_GetStringWidth(str1) / 2, 227);
    } else {
        strcpy(str1, "CONNECTION : ");
        gfx_PrintStringXY(str1, 160 - gfx_GetStringWidth(str1) / 2, 215);
        if (game.connectionmade) strcpy(str1, "success");
        else strcpy(str1, "failure");
        gfx_PrintStringXY(str1, 160 - gfx_GetStringWidth(str1) / 2, 227);
    }
    /*sprintf(str1, "Number : %d", r->squares);
    gfx_PrintStringXY(str1, 5, 230);
    sprintf(str1, "Infected : %d", r->squaresinfected);
    gfx_PrintStringXY(str1, 5, 215);
    sprintf(str1, "Percentage infected : %d", r->squaresinfected * 100 / r->squares);
    gfx_PrintStringXY(str1, 5, 200);*/
}

void RenderTopToolbar(void) {
    const char *viewmode[2] = {"Overview", "Transportation view"};
    //news is global- should update based on individual continent percentages
    //x, y, width, x, height
    char str1[100];
    uint8_t stringwidth;
    stringwidth = gfx_GetStringWidth(viewmode[game.viewmode]);
    gfx_SetColor(0);
    gfx_FillRectangle(0, 0, 320, 34);
    DispNews();
    gfx_FillRectangle(0, 7, 45, 8);
    gfx_SetColor(224);
    gfx_FillRectangle(5,5,38,11);
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY("NEWS:", 7, 7);
    gfx_SetColor(255);
    gfx_HorizLine(0, 33, 320);
    gfx_HorizLine(0, 20, 320);
    gfx_PrintStringXY("DNA Points: ",5,23);
    gfx_SetTextFGColor(224);
    gfx_PrintInt(game.dnapoints,1);
    gfx_SetTextFGColor(255);
    gfx_PrintStringXY("[graph]: mutate", 315 - gfx_GetStringWidth("[graph]: mutate"), 23);
    if (kb_Data[1] & kb_Mode) {
        gfx_SetColor(0);
        gfx_FillRectangle(155 - stringwidth / 2, 112, stringwidth + 10, 16);
        gfx_SetColor(255);
        gfx_Rectangle(155 - stringwidth / 2, 112, stringwidth + 10, 16);
        optix_WhiText();
        gfx_PrintStringXY(viewmode[game.viewmode], 160 - stringwidth / 2, 116);
    }
}

void SetNews(const char str1[]) {
    strcpy(game.news, str1);
    game.newsloops = 30;
}

void DispNews(void) {
    if (game.newsloops > 0) {
        if (game.newsloops < 25) gfx_PrintStringXY(game.news, 45 - ((25 - game.newsloops) * 1.5), 7);
        else gfx_PrintStringXY(game.news, 45, 7);
        game.newsloops--;
    } else gfx_PrintStringXY("No new news.", 45, 7);
}

void UpdateNews(int vVictory, int wVictory) {
    //see mockup
    //it was overflowing when it was 40 ("TEST sees first
    //deaths in South America." or similar is a few too many characters)
    //well it is only one too big, that's 40 characters and it needs a null terminating byte too
    char str1[100];
    uint8_t i;
    int rPer;
    //eventaully change to user's virus name
    //display events until new event
    for (i = 0; i < 7; i++) {
        region_t *r = &region[i];
        rPer = r->squaresinfected * 100 / r->squares;
        if (vVictory > 50 && vVictory < 58) {
           sprintf(str1,"%s has infected half the world.",virus.name);
           SetNews(str1);
        } else if (vVictory > 1 && vVictory < 10) {
           sprintf(str1,"Scientists discover %s.",virus.name);
           SetNews(str1);
        } else if (vVictory > 75) {
            SetNews("The world is looking grim.");
        } else if (wVictory > 10 && wVictory < 20) {
            SetNews("Scientists start developing a cure.");
        } else if (wVictory > 80) {
            SetNews("Clinical trials underway for cure.");
        } else if (rPer > 5 && rPer < 15) {
            sprintf(str1, "%s discovered in %s.",virus.name,r->name);
            SetNews(str1);
        } else if (rPer > 50 && rPer < 75) {
            sprintf(str1, "%s seizes %s.",virus.name,r->name);
            SetNews(str1);
        } else if (r->squaresdead > 0 && r->squaresdead < 10) {
            sprintf(str1,"%s sees first deaths in %s.", virus.name,r->name);
            SetNews(str1);
        }
    }

    gfx_SetTextScale(1,1);
}

//temporary menu
//can be redone to be more neat
void MutationMenu() {
    int selected = 0;
    int cost = 0;
    //x is 40 to 280
    //y is 60 to 180
    //we don't need this, it's already done
    //the new menu stuff will do it all for us
    struct optix_menu_t *m = &optix_menu[optix_guidata.currmenu];
    const char *entries = "Transmission`Resistance`Civil Unrest`";
    const char *title = "Mutate";
    uint8_t maxlines = 3;
    uint8_t textspacing = 15;
    uint8_t width = 160;
    uint8_t xprint;
    uint8_t yprint;
    uint8_t numlines;
    uint8_t currline;
    uint8_t currentselection;
    uint8_t titlewidth = 12;
    char temp[2] = " ";
    bool enterpressed;
    /*numlines = optix_AddMenu(0, 0, 0, 0, 0, 0, 0, 0, entries, NULL);
    if (numlines < maxlines) maxlines = numlines;
    optix_DeleteLastMenu();
    currline = 0;
    currentselection = 0;
    enterpressed = false;
    xprint = 160 - width / 2;
    yprint = 120 - (titlewidth + maxlines * textspacing) / 2;
    optix_AddMenu(xprint, yprint + titlewidth, 0, 0, 1, numlines, 100, textspacing, entries, NULL);
    optix_guidata.currmenu = optix_guidata.nummenus - 1;
    m = &optix_menu[optix_guidata.currmenu];
    while (kb_AnyKey()) kb_Scan();
    while ((!m->enterpressed) || (kb_Data[6] & kb_Enter) || (kb_Data[1] & kb_2nd)) {
        optix_UpdateCurrMenu();
        gfx_SetColor(optix_guicolors.colora);
        gfx_FillRectangle(xprint - 1, yprint + titlewidth - 1, width + 2, maxlines * textspacing + 2);
        gfx_SetColor(optix_guicolors.colorb);
        gfx_Rectangle(xprint - 1, yprint + titlewidth - 1, width + 2, maxlines * textspacing + 2);
        //gfx_Rectangle(xprint + 1, yprint + titlewidth + 1, (width + 2 * padding) - 2, (maxlines * textspacing + (2 * padding)) - 2);
        //make some triangles (not now though, they're a little off and I like the box better)
        //gfx_FillTriangle(xprint - 5, yprint - 5 + textspacing, xprint - 5 + textspacing, yprint - 5 + textspacing, xprint - 5 + textspacing, yprint - 5);
        //gfx_FillTriangle(xprint - 5 + width, yprint - 5 + textspacing, xprint - 5 + width - textspacing, yprint - 5 + textspacing, xprint - 5 + width - textspacing, yprint - 5);
        gfx_FillRectangle(xprint - 1, yprint - 1, width + 2, titlewidth);
        optix_CusText(true);
        gfx_PrintStringXY(title, 160 - gfx_GetStringWidth(title) / 2, yprint + (titlewidth / 2 - 4));
        //indicate that scrolling can be done if applicable
        if (numlines > maxlines) {
            if (m->menumin > 0 && m->menumin < m->numoptions - maxlines) temp[0] = 18;
            else if (m->menumin > 0) temp[0] = 24;
            else if (m->menumin < m->numoptions - maxlines) temp[0] = 25;
            gfx_PrintStringXY(temp, xprint + width - gfx_GetStringWidth(temp), yprint + (titlewidth / 2 - 4));
        }
        optix_RenderMenu(optix_guidata.currmenu);
        gfx_SwapDraw();
    }
    optix_DeleteLastMenu();*/
    //gfx_SetDrawBuffer();
    do {
        kb_Scan();

        gfx_SetDrawBuffer();
        if (kb_Data[7] & kb_Up) selected--; gfx_SwapDraw();
        if (kb_Data[7] & kb_Down) selected++; gfx_SwapDraw();
        if (selected == -1) selected = 2; gfx_SwapDraw();
        if (selected == 3) selected = 0; gfx_SwapDraw();
        gfx_SetColor(0);
        gfx_FillRectangle(100,60,120,120);
        gfx_SetTextScale(2,2);
        optix_WhiText();
        gfx_PrintStringXY("MUTATE",160 - (gfx_GetStringWidth("MUTATE") / 2),70);
        gfx_SetTextScale(1,1);
        gfx_SetColor(255);
        gfx_HorizLine(100,90,120);
        //set text color with selected
        optix_WhiText();
        if (selected == 0) {
            gfx_SetTextFGColor(224);
            gfx_SetTextTransparentColor(0);
            cost = virus.vSpeed + 1;
        }
        gfx_PrintStringXY("TRANSMISSION ",105,100);
        gfx_PrintInt(virus.vSpeed + 1,1);
        optix_WhiText();
        if (selected == 1) {
            gfx_SetTextFGColor(224);
            gfx_SetTextTransparentColor(0);
            cost = virus.vResistance + 1;
        }
        gfx_PrintStringXY("RESISTANCE ",105,115);
        gfx_PrintInt(virus.vResistance + 1,1);
        optix_WhiText();
        if (selected == 2) {
            gfx_SetTextFGColor(224);
            gfx_SetTextTransparentColor(0);
            cost = virus.vUnrest + 1;
        }
        gfx_PrintStringXY("CIVIL UNREST ",105,130);
        gfx_PrintInt(virus.vUnrest + 1,1);
        gfx_SetTextFGColor(224);
        gfx_SetTextTransparentColor(0);
        gfx_PrintStringXY("COST: ",215 - (gfx_GetStringWidth("COST: 10")),170);
        //cost is upgrade + 1

        gfx_PrintInt(cost,1);
        optix_WhiText();
        gfx_SwapDraw();
        //enter and game points is greater than selected cost
        if (kb_Data[6] & kb_Enter && game.dnapoints >= cost) {
            if (selected == 0) virus.vSpeed++;
            if (selected == 1) virus.vResistance++;
            if (selected == 2) virus.vUnrest++;
            game.dnapoints = game.dnapoints - cost;
        }
    } while(kb_Data[6] != kb_Enter || kb_Data[6] != kb_Graph);

}

void Progress(void) {
    int totalInfected = 0;
    int totalDead = 0;

    for (int i = 0; i < 7; i++) {
        totalInfected += region[i].squaresinfected;
        totalDead += region[i].squaresdead;
    }

    game.vVictory = (totalInfected * 100 / 4410);
    //not working
    game.totaldead = (totalDead * 100 / 4410);
    //rework later
    if (game.vVictory % 20 == 0 && game.vVictory != 0 && game.dnapoints < game.vVictory / 20) {
        game.dnapoints++;
    }
    //the higher the virus speed, the higher the cure speed
    //start cure at percent dead?
    if (game.vVictory > 5 && game.wVictory < 100) if (randInt(0, 40 + virus.vResistance) == 0) game.wVictory++;
}

void RenderView() {
    uint8_t i;
    //check if region of origin is not destination
    //if area of port is infected, percent chance that it infects random other port

    //view 0 - default
    //view 1 - transportation //sorry I changed it to fix an error on line 396 (said "game.viewmode" there) np you can build
    //if (game.viewmode == 0)
    if (game.viewmode == 1) {
        //get port data
    }
        //default view
}

void UpdateSelectedRegion(void) {
    //ripped from OPTIX
    int tryx;
    int tryy;
    uint8_t closestbutton;
    int closestbuttonscore;
    int buttonscore;
    uint8_t i;
    int buttonsensitivity = 3;
    region_t *c;
    i = 0;
    tryx = 0;
    tryy = 0;
    closestbuttonscore = 10000;
    closestbutton = 0;
    buttonscore = 0;
    if (game.canpress) {
        kb_Scan();
        if (kb_Data[7] & kb_Left) tryx--;
        if (kb_Data[7] & kb_Right) tryx++;
        if (kb_Data[7] & kb_Up) tryy--;
        if (kb_Data[7] & kb_Down) tryy++;
        if (tryx == 0 && tryy == 0) return;
        c = &region[game.selected];
        for (i = 0; i < 7; i++) {
            region_t *b = &region[i];
            if (tryx == -1) {
                if (b->x < c->x) buttonscore = (c->x - b->x) + (abs(b->y - c->y)) * buttonsensitivity;
                else buttonscore = 10000;
            } else if (tryx == 1) {
                if (b->x > c->x) buttonscore = (b->x - c->x) + (abs(b->y - c->y)) * buttonsensitivity;
                else buttonscore = 10000;
            } else if (tryy == -1) {
                if (b->y < c->y) buttonscore = (c->y - b->y) + (abs(b->x - c->x)) * buttonsensitivity;
                else buttonscore = 10000;
            } else if (tryy == 1) {
                if (b->y > c->y) buttonscore = (b->y - c->y) + (abs(b->x - c->x)) * buttonsensitivity;
                else buttonscore = 10000;
            } else closestbuttonscore = 10000;
            if (buttonscore < closestbuttonscore) {
                closestbutton = i;
                closestbuttonscore = buttonscore;
            }
        }
        if (closestbuttonscore < 5000) game.selected = closestbutton;
        game.canpress = false;
    }
}

void UpdateSimulation(void) {
    uint8_t x;
    uint8_t y;
    uint8_t i;
    int16_t infectdir;
    int hitx;
    int hity;
    region_t *r = &region[game.toupdate];
    x = 0;
    y = 0;
    infectdir = 1;
    //reset these
    r->squares = 0;
    r->squaresrecovered = 0;
    r->squaresinfected = 0;
    r->squaresdead = 0;
    for (x = 0; x < r->spr->width; x++) {
        //because this is so slow
        HandleKeys();
        for (y = 0; y < r->spr->height; y++) {
            if (r->data[y * r->spr->width + x] == 224) {
                //kill (eventually based on virus lethality, 0-100)
                /*if (randInt(0, 100 - 100) == 0) {
                    r->data[y * r->spr->width + x] = 0;
                    continue;
                }*/
                //infect randomly based on probability of infection, in a random direction
                //increase prob infection with dna points
                if (randInt(0,10 - virus.vSpeed) == 0) {
                    infectdir = ((y + randInt(-1, 1)) * r->spr->width) + x + randInt(-1, 1);
                    if (r->data[infectdir] == 255) {
                        r->data[infectdir] = 224;
                    } else if (r->data[infectdir] != 224) {
                        //check if that point is on any of the other regions
                        InfectCoordinate(x + r->x, y + r->y);
                    }
                }
                //kill
                if (randInt(0, 100 - virus.probdeath) == 0) r->data[y * r->spr->width + x] = 64;
                r->squares++;
                r->squaresinfected++;
            } else if (r->data[y * r->spr->width + x] == 64) {
                r->squares++;
                r->squaresinfected++;
                r->squaresdead++;
            } else if (r->data[y * r->spr->width + x] == 255) r->squares++;
        }
    }
    game.toupdate++;
    if (game.toupdate > 6) game.toupdate = 0;
}

void UpdateTransportation(void) {
    //so we can draw lines later
    uint8_t probinfection;
    uint8_t i;
    //stop checking for closed ports
    game.initialport = randInt(0, 21);
    game.endport = randInt(0, 21);

    //this needs to be less common, but i think that fixes the problem
    probinfection = region[port[game.initialport].region].squaresinfected * 100 / region[port[game.initialport].region].squares;

    //close ports
    if (probinfection > port[game.initialport].closethreshold) port[game.initialport].closed = true;

    //too many if statements
    if (game.initialport != game.endport) {
       if (port[game.initialport].closed != true && port[game.endport].closed != true) {
           //if initial port is infected
           game.connectionmade = true;
           if (randInt(0, 100 - probinfection - virus.vUnrest) == 0) {
            InfectCoordinate(port[game.endport].x, port[game.endport].y);
           }
       } else {
           game.connectionmade = false;
       }
    }
}


void HandleKeys(void) {
    kb_Scan();
    if (!kb_AnyKey()) game.canpress = true;
    //if (kb_Data[1] == kb_Graph) game.mutate = true;
    /*if ((kb_Data[1] & kb_Yequ) && (game.selected > 0) && (game.canpress)) {
        game.selected--;
        game.canpress = false;
    }
    if ((kb_Data[1] & kb_Graph) && (game.selected < 6) && (game.canpress)) {
        game.selected++;
        game.canpress = false;
    }*/
    if (game.viewmode == 0) UpdateSelectedRegion();
    //if ((kb_Data[7] & kb_Left) && (game.cursorx > 0)) game.cursorx--;
    //if ((kb_Data[7] & kb_Right) && (game.cursorx < 160)) game.cursorx++;
    //if ((kb_Data[7] & kb_Up) && (game.cursory > 0)) game.cursory--;
    //dont go off screen - rework later
    //if ((kb_Data[7] & kb_Down) && (game.cursory < 200)) game.cursory++;
    //if (kb_Data[6] & kb_Enter) InfectCoordinate(game.cursorx, game.cursory);
    if (kb_Data[1] & kb_Mode && game.canpress) {
        game.viewmode++;
        if (game.viewmode > 1) game.viewmode = 0;
        game.canpress = false;
    }
}

void InitializePorts(void) {
    uint8_t i;
    uint8_t x[22] = {12, 28, 37, 54, 34, 30, 48, 84, 74, 94, 94, 68, 86, 112, 128, 138, 134, 132, 126, 146, 154, 146};
    uint8_t y[22] = {46, 50, 40, 24, 62, 72, 74, 84, 60, 80, 52, 44, 36, 54, 50, 46, 58, 66, 68, 84, 94, 70};
    uint8_t region[22] = {5, 5, 5, 4, 6, 6, 6, 1, 1, 1, 2, 3, 3, 2, 2, 2, 2, 7, 7, 7, 7, 7};
    uint8_t closethreshold[22] = {20, 70, 50, 30, 50, 90, 60, 60, 30, 40, 50, 40, 40, 50, 80, 40, 50, 40, 50, 90, 50, 40};
    for (i = 0; i < 22; i++) {
        port_t *p = &port[i];
        p->x = x[i];
        p->y = y[i];
        p->closethreshold = closethreshold[i];
        //because I'm an idiot
        p->region = region[i] - 1;
        p->closed = false;
    }
}


void InitializeMap(void) {
    uint8_t i;
    char *name[7] = {"Africa", "Asia", "Europe", "Greenland", "North America", "South America", "Oceania"};
    uint16_t x[7] = {117, 169, 127, 90, 0, 49, 252};
    uint8_t y[7] = {83, 37, 46, 35, 43, 114, 130};
    //store all the names to the structs
    for (i = 0; i < 7; i++) strcpy(region[i].name, name[i]);
    //store x and y coordinates
    for (i = 0; i < 7; i++) {
        //half size now so divide by 2, will make calcs easier
        region[i].x = x[i] / 2;
        region[i].y = y[i] / 2;
        //0/0 == -1? Meant to fix that
        region[i].squares = 1;
        region[i].squaresinfected = 0;
        region[i].squaresdead = 0;
    }
    //sprite things
    region[0].spr = africa;
    //region[0].outlinespr = africa_outlined;
    region[1].spr = asia;
    //region[1].outlinespr = asia_outlined;
    region[2].spr = europe;
    //region[2].outlinespr = europe_outlined;
    region[3].spr = greenland;
    //region[3].outlinespr = greenland_outlined;
    region[4].spr = northamerica;
    //region[4].outlinespr = northamerica_outlined;
    region[5].spr = southamerica;
    //region[5].outlinespr = southamerica_outlined;
    region[6].spr = oceania;
    //region[6].outlinespr = oceania_outlined;
    //accessing 3rd element because 1 and 2 are used for height and width
    region[0].data = &africa_data[2];
    region[1].data = &asia_data[2];
    region[2].data = &europe_data[2];
    region[3].data = &greenland_data[2];
    region[4].data = &northamerica_data[2];
    region[5].data = &southamerica_data[2];
    region[6].data = &oceania_data[2];
    //ports
}

void InitializeSimulation(void) {
    //continent data will go here
    virus.probinfection = 99;
    virus.probdeath = 10;
    //virus.vSpeed = 25;
    game.toupdate = 0;
}


void DrawMap(void) {
    uint8_t i;
    for (i = 0; i < 7; i++) gfx_ScaledTransparentSprite_NoClip(region[i].spr, region[i].x * 2, region[i].y * 2, 2, 2);
}

uint8_t MainMenu(void) {
    bool keypress;
    uint8_t keypressed;
    uint8_t i = 0;
    char *menutext[3] = {"RESTART", "PLAY", "QUIT"};
    keypressed = 3;
    keypress = false;
    //partial redraw is our friend here, drawing the map takes a lot of time
    gfx_FillScreen(18);
    DrawMap();
    gfx_Blit(1);
    while (kb_AnyKey()) kb_Scan();
    while ((keypressed == 3) || (keypress == true)) {
        kb_Scan();
        //update star positions
        //draw everything
        //left
        if (kb_Data[1] & kb_Yequ) {
            keypress = true;
            keypressed = 0;
        }
        //right
        if (kb_Data[1] & kb_Graph || kb_Data[6] & kb_Clear) {
            keypress = true;
            keypressed = 2;
        }
        //center
        if (kb_Data[1] & kb_Zoom) {
            keypress = true;
            keypressed = 1;
        }
        //logo, cool
        gfx_SetTextScale(2, 2);
        optix_WhiText();
        gfx_SetTextFGColor(224);
        gfx_PrintStringXY("CONTAGION", 160 - gfx_GetStringWidth("CONTAGION") / 2, 7);
        gfx_SetColor(224);
        gfx_HorizLine(50, 30, 220);
        gfx_SetTextScale(1, 1);
        optix_WhiText();
        //going to do something different here, make some tabs at the bottom
        for (i = 0; i < 3; i++) {
            //first rectangle
            if (i == keypressed) gfx_SetColor(224);
            else gfx_SetColor(0);
            gfx_FillRectangle(i * 107, 230, 107, 10);
            //second rectangle, 10 in
            gfx_FillRectangle(i * 107 + 10, 220, 87, 10);
            //draw a triangle on either side
            gfx_FillTriangle(i * 107, 230, i * 107 + 10, 220, i * 107 + 10, 230);
            gfx_FillTriangle(i * 107 + 107, 230, i * 107 + 97, 230, i * 107 + 97, 220);
            //outline everything depending on what's pressed
            gfx_SetColor(255);
            gfx_Line(i * 107, 240, i * 107, 230);
            gfx_Line(i * 107, 230, i * 107 + 10, 220);
            gfx_Line(i * 107 + 10, 220, i * 107 + 96, 220);
            gfx_Line(i * 107 + 106, 230, i * 107 + 96, 220);
            gfx_Line(i * 107 + 106, 240, i * 107 + 106, 230);
            gfx_PrintStringXY(menutext[i], (i * 107 + 53) - gfx_GetStringWidth(menutext[i]) / 2, 226);
        }
        //line gets cut off so drawing it here
        gfx_Line(319, 240, 319, 230);
        if (!kb_AnyKey()) keypress = false;
        gfx_SwapDraw();
    }
    return keypressed;
}

void RenderTransportation(void) {
    //loop through and show all the ports
    uint8_t i;
    gfx_SetColor(7);
    if (game.connectionmade == true) {
        gfx_Line(port[game.initialport].x * 2, port[game.initialport].y * 2, port[game.endport].x * 2, port[game.endport].y * 2);
        gfx_Line(port[game.initialport].x * 2 - 1, port[game.initialport].y * 2, port[game.endport].x * 2 - 1, port[game.endport].y * 2);
    }
    for (i = 0; i < 22; i++) {
        if (port[i].closed) gfx_SetColor(0);
        else gfx_SetColor(7);
        gfx_FillCircle(port[i].x * 2, port[i].y * 2, 3);
    }
}


void RenderCursor(void) {
    gfx_SetColor(224);
    //gfx_SetColor(0);
    gfx_Circle(game.cursorx * 2, game.cursory * 2, 5);
    gfx_Circle(game.cursorx * 2, game.cursory * 2, 4);
}

void StartGame(void) {
    bool success = false;
    char str1[100];
    game.cursorx = 80;
    game.cursory = 60;
    vname = optix_GetStringInput("Name?", 10, 150, 8);
    //GetStringInput("Name?");
    strcpy(virus.name, vname);
    while (kb_AnyKey()) kb_Scan();
    while (!success) {
        kb_Scan();
        if ((kb_Data[7] & kb_Left) && (game.cursorx > 0)) game.cursorx -= 2;
        if ((kb_Data[7] & kb_Right) && (game.cursorx < 160)) game.cursorx += 2;
        if ((kb_Data[7] & kb_Up) && (game.cursory > 0)) game.cursory -= 2;
        //dont go off screen - rework later
        if ((kb_Data[7] & kb_Down) && (game.cursory < 120)) game.cursory += 2;
        if (kb_Data[6] & kb_Enter) success = InfectCoordinate(game.cursorx, game.cursory);
        gfx_FillScreen(18);
        optix_WhiText();
        DrawMap();
        gfx_SetColor(0);
        gfx_FillRectangle(0, 0, 320, 13);
        sprintf(str1, "Choose an initial location to infect. x:%d y:%d", game.cursorx, game.cursory);
        gfx_PrintStringXY(str1, 5, 2);
        RenderCursor();
        gfx_SetColor(255);
        gfx_HorizLine(0, 13, 320);
        gfx_SwapDraw();

    }
    free(vname);
}


void main(void) {
    char str1[20];
    uint8_t keypressed;
    uint8_t x;
    uint8_t y;
    uint8_t i;
    srand(rtc_Time());
    gfx_Begin();
    gfx_SetDraw(1);
    gfx_SetTransparentColor(0);
    optix_WhiText();
    optix_SetDefaultColors();
    InitializeSimulation();
    InitializeMap();
    InitializePorts();
    LoadData();
    //do FPS things
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;
    keypressed = 3;
    x = randInt(0, 160);
    y = randInt(0, 120);

    while (keypressed != 2) {
        keypressed = MainMenu();
        //because I am tired of the memory management menu
        if (keypressed == 0) {
            ti_Delete("CNTGNDAT");
            optix_Message("ALERT", "~DELETION `All of your Pandemic CE data has been deleted. The program will now quit. `~EXITING `Please press [ENTER]. Some text... `newline starts here. Some text... `~This is a title! `more text", 10, 150, 6);
            optix_Menu("TEST MENU", "Play`Statistics`Options`Reset`Game mode`Credits`About`Quit`", 14, 100, 4);
            break;
        }
        //need something here for keypressed == 0, will be a submenu of some kind
        if (keypressed == 1) {
            if (game.ticksplayed == 0) StartGame();
            while (!(kb_Data[6] & kb_Clear)) {
                kb_Scan();
                if (kb_Data[1] & kb_Graph && game.canpress) {
                    MutationMenu();
                    game.canpress = false;
                }
                //we don't need this anymore, was used to infect random coordinates as a test
                /*if (randInt(0, 10) == 10) {
                    x = randInt(0, 160);
                    y = randInt(0, 120);
                }*/
                HandleKeys();
                UpdateSimulation();
                UpdateTransportation();
                gfx_FillScreen(18);
                DrawMap();
                //RenderCursor();
                if (game.viewmode == 1) RenderTransportation();
                Progress();
                if (game.newsloops == 0) UpdateNews(game.vVictory, game.wVictory);
                //testing some stuff (disable for now), seems to mostly work
                /*InfectCoordinate(x, y);
                gfx_SetColor(224);
                gfx_Circle(x * 2, y * 2, 5);*/
                //more FPS
                RenderBottomToolbar();
                RenderTopToolbar();
                sprintf(str1, "FPS : %d", 32768 / timer_1_Counter);
                //gfx_PrintStringXY(str1, 160 - gfx_GetStringWidth(str1) / 2, 2);
                gfx_SwapDraw();
                //increment the ticks played
                game.ticksplayed += timer_1_Counter;
                timer_1_Counter = 0;
            }
        }
        if (keypressed == 2) {
            SaveData();
            break;
        }
    }
    gfx_End();
}
