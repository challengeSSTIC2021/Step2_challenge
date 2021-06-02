// SSTIC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <stdint.h>
#include <random>
#include "includes/Stack.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlwapi.h>
#include <direct.h> 


#define NUM_DIRS 4
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define MIN_SIZE_MAZE 3
#define MAX_NAME_MAZE 128
#define MAX_NAME_PSEUDO 128
#define MAX_MONSTER 256
#define RATIO_MONSTRE 0.3
#define MAX_NUM_MAZE 32

#define PATH_MAZE "%s\\%s"
#define PATH_MAZE_MAZE "%s\\%s.maze"
#define PATH_MAZE_WILDCARD "%s\\*.maze"
#define PATH_MAZE_SCORE "%s\\%s.rank"

#define LEVEL_MONSTER 3
#define LEVEL_MULTI 2
#define LEVEL_CLASSIC 1

#define PERCENT_WALL_DEFAULT 5

const unsigned int MAX_STRING_SHORT = 16;
const unsigned int MAX_STRING_LONG = 256;

const unsigned int MAX_NB_SCORES = 128;



char PATH_CWD[MAX_PATH+1] = { 0 };


#pragma pack(1)
typedef struct Score {
    uint64_t value;
    char pseudo[MAX_NAME_PSEUDO];
}score_t;

#pragma pack(1)
typedef struct Scores {
    uint8_t nbScores;
    Score scores[MAX_NB_SCORES];
}scores_t;

#pragma pack(1)
typedef struct Monster {
    uint64_t score;
    uint16_t position;
    char display;
    BOOL alive;
} monster_t;

#pragma pack(1)
typedef struct Monsters {
    uint8_t nbMonsters;
    Monster monsters[MAX_MONSTER];
} monsters_t;


typedef struct maze maze_t;


#pragma pack(1)
typedef struct data_generic_ {
    char* data_wall;    
}data_generic_t;



#pragma pack(1)
typedef struct data_monster_ {
    Monsters monsters;
    char* data_wall;
    uint8_t wallProportion;        
}data_monster_t;

typedef union dataUnion {
    data_generic_t data_generic;
    data_monster_t data_monster;
}data_maze_u;


#pragma pack(1)
typedef struct maze {
    uint8_t width;
    uint8_t height;
    uint8_t level;    
    char name[MAX_NAME_MAZE]; //+1
    char creator[MAX_NAME_PSEUDO];    //Doit etre juste avant data
    data_maze_u data_maze;
    Scores highscores;    
}maze_t;






typedef enum menu_choice {

    MENU_REGISTER = 1,
    MENU_CREATE = 2,
    MENU_LOAD = 3,
    MENU_PLAY = 4,    
    MENU_REMOVE = 5,
    MENU_SCOREBOARD = 6,
    MENU_UPGRADE = 7,
    MENU_QUIT = 8
    

} menu_choice_t;

#pragma pack(1)
typedef struct player {
    uint64_t score;
    uint8_t posX;
    uint8_t posY;    
    char pseudo[MAX_NAME_PSEUDO];

} player_t;

typedef struct game {
    player_t player;
    maze_t* maze;
    
}game_t;


void myfflush() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (c == EOF) {
        exit(0);
    }
}



BOOL check_alpha(char* s) {
    for (size_t i = 0; i < strlen(s); i++) {
        //if (!( (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') )) { //|| (s[i] >= '0' && s[i] <= '9')
        if(s[i] == '*' || s[i] == '/' || s[i] == '\\' || s[i] > 128){
            return FALSE;
        }
    }
    if (s[0] == '.') {
        return FALSE;
    }
    return TRUE;
}

void free_data_maze(maze_t* maze) {

    if (maze->level == LEVEL_CLASSIC) {
        if(maze->data_maze.data_generic.data_wall){
            free(maze->data_maze.data_generic.data_wall);
        }
        maze->data_maze.data_generic.data_wall = NULL;
    }
    else {
        if (maze->data_maze.data_monster.data_wall) {
            free(maze->data_maze.data_monster.data_wall);
        }
        maze->data_maze.data_monster.data_wall = NULL;
    }


}



BOOL print_monster(maze_t* maze, uint64_t curPosition) {
    
    for (uint8_t i = 0; i < maze->data_maze.data_monster.monsters.nbMonsters; i++) {
        if (maze->data_maze.data_monster.monsters.monsters[i].position == curPosition && maze->data_maze.data_monster.monsters.monsters[i].alive) {
            printf("%c", maze->data_maze.data_monster.monsters.monsters[i].display);
            return TRUE;
        }
    }
    return FALSE;
}


void print_maze_classic(maze_t* maze, uint64_t positionPlayer) {

    for (uint64_t i = 0; i < maze->height; i++) {
        for (uint64_t j = 0; j < maze->width; j++) {
            uint64_t tmpPosition = i * (uint64_t)maze->width + j;
            if (tmpPosition == positionPlayer) {
                printf("x");
            }
            else{
                printf("%c", maze->data_maze.data_generic.data_wall[tmpPosition]);
            }
        }
        printf("\n");
    }
}


void print_maze_monster(maze_t * maze, uint64_t positionPlayer, BOOL newline) {

    for (uint64_t i = 0; i < maze->height; i++) {

        for (uint64_t j = 0; j < maze->width; j++) {
            uint64_t tmpPosition = i * (uint64_t)maze->width + j;
            if (tmpPosition == positionPlayer && positionPlayer != 0) {
                printf("x");
            }
            else if (!print_monster(maze, tmpPosition)) {
                printf("%c", maze->data_maze.data_monster.data_wall[tmpPosition]);
            }
        }
        if (newline) {
            printf("\n");
        }
    }
}

void reset_highscores(maze_t* maze) {
    maze->highscores.nbScores = 0;
    memset(maze->highscores.scores, 0, MAX_NB_SCORES * sizeof(score_t));
}

void print_maze(maze_t* maze, uint64_t positionPlayer) {
    
    //BUG OBLIGATOIRE ICI IL FAUT QUE CA PARTE EN CLASSIC SI LEVEL > LEVEL MAX
    if (maze->level == LEVEL_MONSTER || maze->level == LEVEL_MULTI) {
        
        print_maze_monster(maze, positionPlayer, TRUE);
        
    }
    else {        
        print_maze_classic(maze, positionPlayer);
        
    }

    printf("-*-*-*-*-*-*-*\n");
}


int get_user_string(char* user_value, int sizeMax) {

    int res = scanf_s("%s",user_value, sizeMax);
    myfflush();
    return res;
}


BOOL get_user_int(int* user_value) {
    int nbElement = scanf_s("%d", user_value);
    myfflush();
    return nbElement != 1;
}

int get_user_move(char *move) {    
   
    int nbElement = scanf_s("%c", move, 1);
    myfflush();    
    if (nbElement != 1) {
        return nbElement;
    }
    else if (*move !=  'z' && *move != 'q' && *move != 's' && *move != 'd' && *move != 'x') {        
        return 1;
    }
    return 0;

}



bool register_user(player_t* player) {

    printf("Pseudo :\n");
    
     if (get_user_string(player->pseudo, sizeof(player->pseudo)) != 1) {
         printf("Problem while getting pseudo, maximum size is %zd\n", sizeof(player->pseudo));
         return FALSE;
    }

    return TRUE;

}




int directions[] = { 0, 1, 2, 3 };

void init_maze(char* data_wall, uint8_t width, uint8_t height)
{    

    for (uint64_t y = 0; y < height; y++)
    {
        for (uint64_t x = 0; x < width; x++)
        {
            data_wall[y * width + x] = '#';
        }
    }
   
}

void shuffle(int* array, int n)
{
    if (n > 1)
    {
        for (int i = 0; i < n - 1; i++)
        {
            int j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

void carve_maze_generic(char * data_wall, uint8_t width, uint8_t height, Stack* x_stack, Stack* y_stack ) {

    init_maze(data_wall, width, height);
    data_wall[1 * width + 1] = ' ';
    data_wall[height * width] = 0;



    while (!stack_isempty(x_stack))
    {
        shuffle(directions, NUM_DIRS);

        int x = stack_pop(x_stack);
        int y = stack_pop(y_stack);

        for (int i = 0; i < NUM_DIRS; i++)
        {
            switch (directions[i])
            {
            case 0: // up
            {
                if (y - 2 <= 0) continue;

                if (data_wall[(y - 2) * width + x] != ' ')
                {
                    data_wall[(y - 2) * width + x] = ' ';                    
                    data_wall[(y - 1) * width + x] = ' ';                    
                    stack_push(x_stack, x);
                    stack_push(y_stack, y - 2);
                }
                break;
            }
            case 1: // right
            {
                if (x + 2 >= width - 1) continue;

                if (data_wall[y * width + x + 2] != ' ')
                {
                    data_wall[y * width + x + 2] = ' ';                    
                    data_wall[y * width + x + 1] = ' ';                    
                    stack_push(x_stack, x + 2);
                    stack_push(y_stack, y);
                }
                break;
            }
            case 2: // down
            {
                if (y + 2 >= height - 1) continue;

                if (data_wall[(y + 2) * width + x] != ' ')
                {
                    data_wall[(y + 2) * width + x] = ' ';                    
                    data_wall[(y + 1) * width + x] = ' ';                    
                    stack_push(x_stack, x);
                    stack_push(y_stack, y + 2);
                }
                break;
            }
            case 3: // left
            {
                if (x - 2 <= 0) continue;

                if (data_wall[y * width + x - 2] != ' ')
                {
                    data_wall[y * width + x - 2] = ' ';                    
                    data_wall[y * width + x - 1] = ' ';                    
                    stack_push(x_stack, x - 2);
                    stack_push(y_stack, y);
                }
                break;
            }
            }
        }
    }

    data_wall[(height - 2) * width + width - 1] = 'o';
}

BOOL carve_maze_multi(char * data_wall, uint8_t width, uint8_t height, uint8_t wallProportion) {


    int* tmpWalls = NULL;
    uint64_t indexTmpWalls = 0;
    size_t sizeData = (size_t)height * (size_t)width;
    tmpWalls = (int*)calloc(sizeData, sizeof(int));

    if (tmpWalls == NULL) {
        return FALSE;
    }

    for (uint8_t i = 0; i < width; i++) {
        for (uint8_t j = 0; j < height; j++) {

            if (i == (uint8_t)0 || i == width - (uint8_t)1) {
                continue;
            }
            if (j == (uint8_t)0 || j == height - (uint8_t)1) {
                continue;
            }
            if (data_wall[j * width + i] == '#') {
                tmpWalls[indexTmpWalls++] = (int)(j * (uint64_t)width + i);
            }
        }
    }

    shuffle(tmpWalls, (int)indexTmpWalls);
    
    double double_nbWallsToDestroy = (double)indexTmpWalls * ((double)wallProportion / 100.0);
    uint64_t nbWallsToDestroy = (uint64_t)double_nbWallsToDestroy;
    printf("Will destroy %ju walls\n", nbWallsToDestroy);

    for (uint64_t i = 0; i < min(nbWallsToDestroy, indexTmpWalls); i++) {
        data_wall[tmpWalls[i]] = ' ';        
    }

    free(tmpWalls);
    return TRUE;

}

BOOL carve_maze_monsters(char * data_wall, Monsters * monsters, uint8_t width, uint8_t height) {

    int* tmpPositions = NULL;
    int indexTmpPositions = 0;

    size_t sizeData = (size_t)height * (size_t)width;

    tmpPositions = (int*)calloc(sizeData, sizeof(int));


    if (tmpPositions == NULL) {
        return FALSE;
    }

    for (uint8_t i = 0; i < width; i++) {
        for (uint8_t j = 0; j < height; j++) {

            if ((uint8_t)i == 0 || i == width - (uint8_t)1) {
                continue;
            }
            if ((uint8_t)j == 0 || j == height - (uint8_t)1) {
                continue;
            }
            //Starting position
            if ((uint8_t)j == 1 && i == 1) {
                continue;
            }

            if (data_wall[j * width + i] == ' ') {
                tmpPositions[indexTmpPositions++] = (int)(j * (uint64_t)width + i);
            }
        }
    }

    shuffle(tmpPositions, indexTmpPositions);
    uint8_t realNbMonsters = 0;
    for (uint64_t i = 0; i < monsters->nbMonsters; i++) {
        monsters->monsters[i].position = (uint16_t)tmpPositions[i];
        realNbMonsters++;
    }

    monsters->nbMonsters = realNbMonsters;
    free(tmpPositions);

    return TRUE;
}


BOOL carve_maze(maze_t* maze, Stack* x_stack, Stack* y_stack)
{  
        
    char* data_wall = NULL;

    data_wall = (char*)calloc((uint64_t)maze->width * (uint64_t)maze->height + 1, sizeof(char));
    if (data_wall == NULL) {
        return FALSE;
    }

    if (maze->level == LEVEL_CLASSIC) {        
        maze->data_maze.data_generic.data_wall = data_wall;        
     
    }
    else if(maze->level == LEVEL_MULTI){
        maze->data_maze.data_monster.data_wall = data_wall;
     
    }
    else if (maze->level == LEVEL_MONSTER) {
        maze->data_maze.data_monster.data_wall = data_wall;
     
    }

 

    carve_maze_generic(data_wall, maze->width, maze->height, x_stack, y_stack);
    
    if (maze->level >= LEVEL_MULTI) {
        if (!carve_maze_multi(data_wall, maze->width, maze->height, maze->data_maze.data_monster.wallProportion)) {
            free(data_wall);
            return FALSE;
        }
    }    

    if (maze->level == LEVEL_MONSTER) {
        if (!carve_maze_monsters(data_wall, &maze->data_maze.data_monster.monsters, maze->width, maze->height)) {
            free(data_wall);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL generate_maze(maze_t * maze)
{
    

    BOOL res = TRUE;
    Stack* x_stack = NULL;
    Stack* y_stack = NULL;
    
    x_stack = stack_create(32);
    if (x_stack == NULL) {
        return FALSE;
    }

    y_stack = stack_create(32);
    if (y_stack == NULL) {
        free(x_stack);
        return FALSE;
    }

    // start in top left
    int x = 1;
    int y = 1;

    stack_push(x_stack, x);
    stack_push(y_stack, y);

    if (!carve_maze(maze, x_stack, y_stack)) {
        res = FALSE;     
    }

    
    free(x_stack);
    free(y_stack);


    return res;
   
}

BOOL create_custom_maze_generic(maze_t * maze) {

    int width = 0;
    int height = 0;


    while (width % 2 == 0 || width < MIN_SIZE_MAZE || width >= UINT8_MAX) {
        printf("Width odd and greater than %d: ", MIN_SIZE_MAZE);
        get_user_int(&width);
    }
    maze->width = (uint8_t)width;

    while (height % 2 == 0 || height < MIN_SIZE_MAZE || height >= UINT8_MAX) {
        printf("Height odd and greater than %d: ", MIN_SIZE_MAZE);
        get_user_int(&height);

    }
    maze->height = (uint8_t)height;

    return TRUE;

}

BOOL create_custom_maze_multi(maze_t* maze) {

    int proportion = -1;

    while (proportion < 0 || proportion > 100) {
        printf("Enter the percentage of wall to remove, default is %d%%: ", PERCENT_WALL_DEFAULT);
        get_user_int(&proportion);
    }
    maze->data_maze.data_monster.wallProportion = (uint8_t)proportion;
    
    return TRUE;

}


BOOL create_custom_maze_monster(maze_t* maze) {


    int nbMonsters = 0;
    int nbMonstersMax = 0;



    nbMonstersMax = maze->width + maze->height - 6;


    nbMonsters = maze->width * maze->height;
    while (nbMonsters > nbMonstersMax || nbMonsters < 0 || nbMonsters > MAX_MONSTER) {
        printf("Number of traps between %d and %d: ", 0, min(nbMonstersMax, MAX_MONSTER));
        get_user_int(&nbMonsters);
    }

    int valueScoreMonster = 0;
    maze->data_maze.data_monster.monsters.nbMonsters = (uint8_t)nbMonsters;

    printf("Score value for traps: ");

    if (get_user_int(&valueScoreMonster)) {
        return FALSE;
    }

    for (int i = 0; i < maze->data_maze.data_monster.monsters.nbMonsters; i++) {
        maze->data_maze.data_monster.monsters.monsters[i].display = '^';
        maze->data_maze.data_monster.monsters.monsters[i].position = 0;
        maze->data_maze.data_monster.monsters.monsters[i].alive = TRUE;

        
        maze->data_maze.data_monster.monsters.monsters[i].score = (uint64_t)valueScoreMonster;
    }

    return TRUE;
}



BOOL create_custom_maze(maze_t * maze) {


    create_custom_maze_generic(maze);


    if (maze->level >= LEVEL_MULTI) {
        create_custom_maze_multi(maze);
    }

    if (maze->level == LEVEL_MONSTER) {
        create_custom_maze_monster(maze);
    }


    return TRUE;

}




void apply_user_move(maze_t* maze, player_t * player, char move) {

    player->score+=1;

    uint8_t newX = player->posX;
    uint8_t newY = player->posY;
    //printf("oldX %d oldY %d\n", newX, newY);
    switch (move) {

    case 'd':
        newX += 1;
        break;
    case 'q':
        newX -= 1;
        break;

    case 's':
        newY += 1;
        break;

    case 'z':
        newY -= 1;
        break;

    default:
        //printf("Should never happen this move : %c %d\n", move, move);
        break;
    }    
    
    //Pas besoin de check par rapport a 0 car c'est du unsigned
    if (newX >= maze->width || newY >= maze->height) {
        return;
    }
    uint64_t newPosition = newY * (uint64_t)maze->width + newX;

    char* data_wall = NULL;
    if (maze->level == LEVEL_CLASSIC) {
        data_wall = maze->data_maze.data_generic.data_wall;
    }
    else if (maze->level == LEVEL_MULTI) {
        data_wall = maze->data_maze.data_monster.data_wall;
    }
    else if (maze->level == LEVEL_MONSTER) {
        data_wall = maze->data_maze.data_monster.data_wall;
    }
    else {
        return;
    }


    if (maze->level >= LEVEL_MONSTER) {        
        Monsters* monsters = &maze->data_maze.data_monster.monsters;
        for (uint64_t i = 0; i < monsters->nbMonsters; i++) {
            if (monsters->monsters[i].position == newPosition && monsters->monsters[i].alive) {
               player->score += monsters->monsters[i].score;
               monsters->monsters[i].alive = FALSE;
            }
        }        
    }    

    if (data_wall[newY * maze->width + newX] != '#'){        
        player->posX = newX;
        player->posY = newY;        
    }



}


void generate_random_maze_generic(maze_t* maze) {

    uint64_t width;
    uint64_t height;

    width = (11 + (uint64_t)rand()) % 256;
    if (width % 2 == 0) {
        if (width == 4) {
            width += 1;
        }
        else {
            width -= 1;
        }
    }

    height = (11 + (uint64_t)rand()) % 256;
    if (height % 2 == 0) {
        if (height == 4) {
            height += 1;
        }
        else {
            height -= 1;
        }
    }

    
    maze->width = (uint8_t)  width;
    maze->height = (uint8_t)height;
}

void generate_random_maze_multi(maze_t* maze) {

    do {
        maze->data_maze.data_monster.wallProportion = (uint8_t) (rand() % 10);
    } while (maze->data_maze.data_monster.wallProportion == 0);

}

void generate_random_maze_monster(maze_t* maze) {
    int nbMonstersMax;

    uint64_t scoreMonster = (uint64_t)(rand() % ((maze->width * maze->height) / 10));

    nbMonstersMax = (int)(((maze->width * maze->height) / 4) * RATIO_MONSTRE);
    maze->data_maze.data_monster.monsters.nbMonsters = (uint8_t)(rand() % min(nbMonstersMax, MAX_MONSTER));
    for (int i = 0; i < maze->data_maze.data_monster.monsters.nbMonsters; i++) {
        maze->data_maze.data_monster.monsters.monsters[i].display = '^';
        maze->data_maze.data_monster.monsters.monsters[i].position = 0;
        maze->data_maze.data_monster.monsters.monsters[i].alive = TRUE;
        maze->data_maze.data_monster.monsters.monsters[i].score = scoreMonster;
    }
}


BOOL create_random_maze(maze_t* maze) {
        

    generate_random_maze_generic(maze);
    

    if (maze->level >= LEVEL_MULTI) {
        generate_random_maze_multi(maze);
    }

    if (maze->level == LEVEL_MONSTER) {
        generate_random_maze_monster(maze);
    }

    return TRUE;
    
}

BOOL save_maze_generic(maze_t* maze, HANDLE hFile) {


    uint8_t lenCreator = (uint8_t)strlen(maze->creator);
    DWORD dwBytesWritten;

    BOOL bErrorFlag = WriteFile(
        hFile,           // open file handle
        &lenCreator,      // start of data to write
        sizeof(lenCreator),  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure


    bErrorFlag &= WriteFile(
        hFile,           // open file handle
        maze->creator,      // start of data to write
        lenCreator,  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure


    bErrorFlag &= WriteFile(
        hFile,           // open file handle
        &maze->level,      // start of data to write
        sizeof(maze->level),  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure


    bErrorFlag &= WriteFile(
        hFile,           // open file handle
        &maze->width,      // start of data to write
        sizeof(maze->width),  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure

    bErrorFlag &= WriteFile(
        hFile,           // open file handle
        &maze->height,      // start of data to write
        sizeof(maze->height),  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure

    uint64_t sizeData_ = (uint64_t)maze->height * (uint64_t)maze->width;
    char* data_wall = NULL;
    if (maze->level == LEVEL_CLASSIC) {
        data_wall = maze->data_maze.data_generic.data_wall;
    }
    else if (maze->level == LEVEL_MULTI) {
        data_wall = maze->data_maze.data_monster.data_wall;
    }
    else if (maze->level == LEVEL_MONSTER) {
        data_wall = maze->data_maze.data_monster.data_wall;
    }
    DWORD sizeData = (DWORD)sizeData_;
    bErrorFlag &= WriteFile(
        hFile,           // open file handle
        data_wall,      // start of data to write
        sizeData,  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure

    return bErrorFlag;
}

BOOL save_maze_monsters(maze_t* maze, HANDLE hFile) {

    DWORD dwBytesWritten;

    BOOL bErrorFlag = WriteFile(
        hFile,           // open file handle
        &maze->data_maze.data_monster.monsters.nbMonsters,      // start of data to write
        sizeof(maze->data_maze.data_monster.monsters.nbMonsters),  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure

    for (int i = 0; i < maze->data_maze.data_monster.monsters.nbMonsters; i++) {

        bErrorFlag &= WriteFile(
            hFile,           // open file handle
            &maze->data_maze.data_monster.monsters.monsters[i].score,      // start of data to write
            sizeof(maze->data_maze.data_monster.monsters.monsters[i].score),  // number of bytes to write
            &dwBytesWritten, // number of bytes that were written
            NULL);            // no overlapped structure

        bErrorFlag &= WriteFile(
            hFile,           // open file handle
            &maze->data_maze.data_monster.monsters.monsters[i].position,      // start of data to write
            sizeof(maze->data_maze.data_monster.monsters.monsters[i].position),  // number of bytes to write
            &dwBytesWritten, // number of bytes that were written
            NULL);            // no overlapped structure

        bErrorFlag &= WriteFile(
            hFile,           // open file handle
            &maze->data_maze.data_monster.monsters.monsters[i].display,      // start of data to write
            sizeof(maze->data_maze.data_monster.monsters.monsters[i].display),  // number of bytes to write
            &dwBytesWritten, // number of bytes that were written
            NULL);            // no overlapped structure

    }

    return bErrorFlag;
}

BOOL create_maze_score(maze_t* maze) {

    DWORD dwBytesWritten;
    char pathScore[MAX_STRING_LONG] = { 0 };
    sprintf_s(pathScore, MAX_STRING_LONG, PATH_MAZE_SCORE, PATH_CWD, maze->name);


    reset_highscores(maze);


    HANDLE hFile = CreateFile(pathScore,                // name of the write
        GENERIC_WRITE,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        CREATE_NEW,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        //printf("Problem creating file %s, err code %ld \n", pathMaze, GetLastError());
        return FALSE;
    }

    BOOL bErrorFlag = WriteFile(
        hFile,           // open file handle
        &maze->highscores.nbScores,      // start of data to write
        sizeof(maze->highscores.nbScores),  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure

    CloseHandle(hFile);

    return bErrorFlag;

}
bool save_maze(maze_t* maze) {


    char pathMaze[MAX_NAME_MAZE + sizeof(PATH_CWD) + sizeof(PATH_MAZE_MAZE)] = { 0 };
    sprintf_s(pathMaze, MAX_NAME_MAZE + sizeof(PATH_CWD) + sizeof(PATH_MAZE_MAZE), PATH_MAZE_MAZE, PATH_CWD, maze->name);
    

    HANDLE hFile = CreateFile(pathMaze,                // name of the write
        GENERIC_WRITE,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        CREATE_NEW,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        //printf("Problem creating file %s, err code %ld \n", pathMaze, GetLastError());
        return FALSE;
    }


    BOOL bErrorFlag = save_maze_generic(maze, hFile);
    
    if (maze->level == LEVEL_MONSTER) {
        bErrorFlag &= save_maze_monsters(maze, hFile);
    }
    
    CloseHandle(hFile);

    return bErrorFlag;
}

BOOL load_maze_data_wall(maze_t* maze, IStream* stm) {

    DWORD dwBytesRead = 0;
    uint64_t sizeData = (uint64_t)maze->width * (uint64_t)maze->height;


    char* data_wall = (char*)calloc(sizeData + (uint64_t)1, sizeof(char));


    if (data_wall == NULL) {
        return FALSE;    
    }

    if (maze->level >= LEVEL_MULTI) {
        maze->data_maze.data_monster.data_wall = data_wall;
    }
    else {
        maze->data_maze.data_generic.data_wall = data_wall;
    }


    data_wall[sizeData] = 0;

    if (S_OK != stm->Read(data_wall, (ULONG)sizeData, &dwBytesRead))
    {
        free(data_wall);
        data_wall = NULL;
        return FALSE;
    }

    return TRUE;
}

BOOL load_maze_data_classic(maze_t* maze, IStream *stm) {


    DWORD dwBytesRead = 0;    
    uint8_t lenCreator = 0;
    
    if (S_OK != stm->Read(&lenCreator, sizeof(lenCreator), &dwBytesRead))
    {
        return FALSE;
    }

    if (S_OK != stm->Read(maze->creator, lenCreator, &dwBytesRead))
    {
        return FALSE;
    }

    if (S_OK != stm->Read(&maze->level, sizeof(maze->level), &dwBytesRead))
    {
        return FALSE;
    }


    if (S_OK != stm->Read(&maze->width, sizeof(maze->width), &dwBytesRead))
    {
        return FALSE;
    }

    if (S_OK != stm->Read(&maze->height, sizeof(maze->height), &dwBytesRead))
    {
        return FALSE;
    }    
    if (!load_maze_data_wall(maze, stm)) {
        return FALSE;
    }


    return TRUE;

}


BOOL load_maze_data_monster(maze_t* maze, IStream* stm) {


    DWORD dwBytesRead = 0;
    //uint64_t sizeData = (uint64_t)maze->width * (uint64_t)maze->height;

    data_monster_t* data_monsters = &maze->data_maze.data_monster;
       

    if (S_OK != stm->Read(&data_monsters->monsters.nbMonsters, sizeof(data_monsters->monsters.nbMonsters), &dwBytesRead))
    {
        return FALSE;
    }
    
    for (int i = 0; i < data_monsters->monsters.nbMonsters; i++) {
        if (S_OK != stm->Read(&data_monsters->monsters.monsters[i].score, sizeof(data_monsters->monsters.monsters[i].score), &dwBytesRead))
        {
            return FALSE;
        }
        

        if (S_OK != stm->Read(&data_monsters->monsters.monsters[i].position, sizeof(data_monsters->monsters.monsters[i].position), &dwBytesRead))
        {
            return FALSE;
        }
        

        if (S_OK != stm->Read(&data_monsters->monsters.monsters[i].display, sizeof(data_monsters->monsters.monsters[i].display), &dwBytesRead)) {

            return FALSE;
        }
        
    }
       

    return TRUE;

}



maze_t* load_maze_by_name(const char* pathMaze) {
       


    IStream* stm;
    if (S_OK != SHCreateStreamOnFileA(pathMaze, STGM_READ, &stm)) {
        //printf("Terminal failure: Unable to create stream from file %s\n GetLastError=%lx\n", pathMaze, GetLastError());        
        return NULL;
    }

    maze_t* maze = (maze_t*)calloc(1, sizeof(maze_t));

    if (maze == NULL) {
        //printf("Problem with malloc maze\n");
        return NULL;
    }

    if (!load_maze_data_classic(maze, stm)) {
        free(maze);
        maze = NULL;
        //printf("Terminal failure: Unable to read from file.\n GetLastError=%lu\n", GetLastError());
        stm->Release();
        return NULL;
    }        

    if (maze->level == LEVEL_MULTI) {
        /*if (!load_maze_data_monster(maze, stm)) {
            free_data_maze(maze);
            free(maze);
            maze = NULL;
            return NULL;
        }
        */
    }
    //BUG VOLONTAIRE
    else if (maze->level >= LEVEL_MONSTER) {

        if (!load_maze_data_monster(maze, stm)) {
            free_data_maze(maze);
            free(maze);            
            maze = NULL;
            return NULL;
        }
    }    
    
    stm->Release();
    
    return maze;

}
int get_list_maze(char listMaze[MAX_NUM_MAZE][MAX_STRING_LONG]) {


    char dirMaze[MAX_PATH+1];
    sprintf_s(dirMaze, MAX_PATH, PATH_MAZE_WILDCARD, PATH_CWD);

    
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(dirMaze, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        return 0;
    }

    // List all the files in the directory with some info about them.
    int numChoiceMax = 0;
    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //printf("  %s   <DIR>\n", ffd.cFileName);
        }
        else
        {
            //printf("  %s   FILE\n", ffd.cFileName);
            strcpy_s(listMaze[numChoiceMax], MAX_STRING_LONG-1, ffd.cFileName);
            numChoiceMax+=1;
        }
    } while (FindNextFile(hFind, &ffd) != 0 && numChoiceMax < MAX_NUM_MAZE);


    FindClose(hFind);
    return numChoiceMax;
}

void display_list_maze(char listMaze[MAX_NUM_MAZE][MAX_STRING_LONG] , int sizeMaze) {
    printf("List of existing mazes\n");
    for (int i = 0; i < sizeMaze; i++) {
        printf("%d -> %s\n", i+1, listMaze[i]);
        
    }
}


BOOL load_highscores_from_file(maze_t* maze) {


    char pathScore[MAX_STRING_LONG];
    sprintf_s(pathScore, MAX_STRING_LONG, PATH_MAZE_SCORE, PATH_CWD, maze->name);


    
    IStream* stm;
    if (S_OK != SHCreateStreamOnFileA(pathScore, STGM_READ, &stm)) {
        //printf("Terminal failure: Unable to create stream from file %s.\n GetLastError=%lx\n", pathScore, GetLastError());
        return FALSE;
    }

    DWORD nbBytesRead;


    if (S_OK != stm->Read(&maze->highscores.nbScores, sizeof(maze->highscores.nbScores), &nbBytesRead))
    {
        stm->Release();
        return FALSE;
    }  


    for (int i = 0; i < maze->highscores.nbScores; i++) {
        uint8_t pseudoLen;
        if (S_OK != stm->Read(&pseudoLen, sizeof(pseudoLen), &nbBytesRead)) {
            stm->Release();
            return FALSE;
        }
        if (S_OK != stm->Read(&maze->highscores.scores[i].pseudo, pseudoLen, &nbBytesRead)) {
            stm->Release();
            return FALSE;
        }
        if (S_OK != stm->Read(&maze->highscores.scores[i].value, sizeof(maze->highscores.scores[i].value), &nbBytesRead)) {
            stm->Release();
            return FALSE;
        }
    }
 

    stm->Release();

    return TRUE;
}



BOOL load_highscores(maze_t* maze) {


    reset_highscores(maze);
    if (load_highscores_from_file(maze)) {
        return TRUE;
    }
    
    
    return FALSE;
    


}



maze_t* load_maze() {
    char listMaze[MAX_NUM_MAZE][MAX_STRING_LONG] = { 0 };
    int sizeListMaze = get_list_maze(listMaze);
    if (sizeListMaze == 0) {
        printf("There is no maze to load. Please create one before.\n");
        return NULL;
    }
    display_list_maze(listMaze, sizeListMaze);
    
    int choice_int = -1;
    char choice_string[MAX_NAME_MAZE] = { 0 };
    char pathMaze[MAX_PATH + 1] = { 0 };
    char nameMaze[MAX_NAME_MAZE + 1] = { 0 };
    WIN32_FIND_DATA ffd;
    HANDLE hFile;
    
    do {
        printf("Which maze do you want ? send its identifier or its name (w or wo extension).\n You can send -1 to come back to the main menu.\n");
        get_user_string(choice_string, MAX_NAME_MAZE);
        choice_int = atoi(choice_string);
        if (choice_int == -1)
            return NULL;
        if (choice_int > 0 && choice_int <= sizeListMaze) {            
            memcpy_s(nameMaze, MAX_NAME_MAZE, listMaze[choice_int - 1], strlen(listMaze[choice_int - 1]) -5);
            sprintf_s(pathMaze, MAX_PATH, PATH_MAZE, PATH_CWD, listMaze[choice_int - 1]);
            break;
        }

        
        if (check_alpha(choice_string)) {            
        
            sprintf_s(pathMaze, MAX_PATH, PATH_MAZE, PATH_CWD, choice_string);
            hFile = FindFirstFile(pathMaze, &ffd);
            if (strlen(choice_string) > 5 && INVALID_HANDLE_VALUE != hFile) {
                memcpy_s(nameMaze, MAX_NAME_MAZE, choice_string, strlen(choice_string) - 5);
                FindClose(hFile);
                hFile = NULL;
                break;
            }
            if (hFile != INVALID_HANDLE_VALUE) {
                FindClose(hFile);
                hFile = NULL;
            }
            sprintf_s(pathMaze, MAX_PATH, PATH_MAZE_MAZE, PATH_CWD, choice_string);
            hFile = FindFirstFile(pathMaze, &ffd);
            if (INVALID_HANDLE_VALUE  != hFile) {
                strcpy_s(nameMaze, MAX_NAME_MAZE, choice_string);
                FindClose(hFile);
                hFile = NULL;
                break;
            }
        }
        else {
            printf("Name of maze contains forbidden characters\n");
        }
        
    } while (TRUE);
       
    
    maze_t* maze = load_maze_by_name(pathMaze);

    if (maze == NULL)
        return NULL;

    memset(maze->name, 0, sizeof(maze->name));
    strcpy_s(maze->name, MAX_NAME_MAZE, nameMaze);    
    return maze;
    
}

int get_user_char(char * res) {

    int tmp = scanf_s("%c", res, 1);
    myfflush();
    return tmp;
}





maze_t * menu_create_maze(player_t player) {


    char listMaze[MAX_NUM_MAZE][MAX_STRING_LONG] = { 0 };
    int sizeListMaze = get_list_maze(listMaze);
    if (sizeListMaze >= MAX_NUM_MAZE) {
        printf("There is already too many mazes, please remove one before creating a new one.\n");
        return NULL;
    }

    maze_t* maze = (maze_t*)calloc(1, sizeof(maze_t));

    if (maze == NULL) {
        return NULL;
    }

    int level_value;
    
    
    
    do {        
        printf("Choose the level of your maze :\n\t1. Classic maze\n\t2. Maze multipass \n\t3. Maze multipass with traps\n");
        
    } while (get_user_int(&level_value)|| level_value < LEVEL_CLASSIC || level_value > LEVEL_MONSTER);
    
    maze->level = (uint8_t)level_value;
        

    printf("Random maze or custom maze ? r/c ");
    char user_custom_random = 0;
    
    while (TRUE) {
        get_user_char(&user_custom_random);
        if (user_custom_random == 'r') {
            if (!create_random_maze(maze)) {
                free(maze);
                return NULL;
            }
            break;
        }
        else if (user_custom_random == 'c') {
            if (!create_custom_maze(maze)) {
                free(maze);
                return NULL;
            }
            break;
        } 
    }
    //Contains the data_wall generation
    if (!generate_maze(maze)) {        
        free(maze);
        return NULL;
    }


    maze->highscores.nbScores = 0;
    
    print_maze(maze, (uint64_t) maze->width+(uint64_t)1);
    

    printf("Do you want to save this maze ? y/n ");
    char user_yes_no = 0;    
    get_user_char(&user_yes_no);    
    if (user_yes_no == 'y') {
        while (TRUE) {
            printf("What the name of the maze to save ?");
            if (get_user_string(maze->name, MAX_NAME_MAZE) == 1 && check_alpha(maze->name)) {

                WIN32_FIND_DATA ffd;
                
                char pathMaze[MAX_NAME_MAZE + sizeof(PATH_CWD) + sizeof(PATH_MAZE_MAZE)] = { 0 };
                sprintf_s(pathMaze, MAX_NAME_MAZE + sizeof(PATH_CWD) + sizeof(PATH_MAZE_MAZE), PATH_MAZE_MAZE, PATH_CWD, maze->name);
                HANDLE hFind = FindFirstFile(pathMaze, &ffd);
                if (INVALID_HANDLE_VALUE == hFind)
                {
                    break;
                }
                else {
                    printf("A maze with this name already exists\n");
                    FindClose(hFind);
                }
                
            }
            else {
                printf("Bad name, maybe too long or containing non alphabetical chars?\n");
            }
        }

        strcpy_s(maze->creator, MAX_NAME_PSEUDO, player.pseudo);
        if (!save_maze(maze) || !create_maze_score(maze)) {
            printf("Problem while saving maze\n");
        }
        else {
            printf("Maze saved as %s\n", maze->name);
        }
    }
    else {
        printf("I forgot the maze \n");
        
        free_data_maze(maze);
        free(maze);
        return NULL;
    }
    

    return maze;
    
}

//Consider that scores are ordered
//Bug inside but where ?
BOOL update_highscores(Score newScore, Scores* highscores) {    
    for (uint64_t i = 0; i < MAX_NB_SCORES; i++) {
        if (highscores->nbScores <= i) {
            highscores->nbScores += 1;
            memcpy(&highscores->scores[i], &newScore, sizeof(Score));
            return TRUE;
        }
        else if (newScore.value < highscores->scores[i].value) {

            //Shift all scores to the right
            memcpy(&highscores->scores[i + 1], &highscores->scores[i], (((uint64_t)MAX_NB_SCORES) - i - (uint64_t)1) * sizeof(Score));

            //set the new ith score
            memcpy(&highscores->scores[i], &newScore, sizeof(Score));

            if (highscores->nbScores < MAX_NB_SCORES) {
                highscores->nbScores++;            
            }
            return TRUE;
        }

    }

    return FALSE;

}

BOOL save_highscores(maze_t *maze) {

    char pathScore[MAX_STRING_LONG];
    sprintf_s(pathScore, MAX_STRING_LONG, PATH_MAZE_SCORE, PATH_CWD, maze->name);
    

    HANDLE hFile = CreateFile(pathScore,                // name of the write
        GENERIC_WRITE,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        CREATE_ALWAYS,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {        
        return FALSE;
    }

    DWORD dwBytesWritten;

    
    BOOL bErrorFlag = WriteFile(
        hFile,           // open file handle
        &maze->highscores.nbScores,      // start of data to write
        sizeof(maze->highscores.nbScores),  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure

    for (uint8_t i = 0; i < maze->highscores.nbScores; i++) {


        uint8_t pseudoLen = (uint8_t) strlen(maze->highscores.scores[i].pseudo);
        
        
        bErrorFlag &= WriteFile(
            hFile,           // open file handle
            &pseudoLen,      // start of data to write
            sizeof(pseudoLen),  // number of bytes to write
            &dwBytesWritten, // number of bytes that were written
            NULL);

        bErrorFlag &= WriteFile(
            hFile,           // open file handle
            &maze->highscores.scores[i].pseudo,      // start of data to write
            pseudoLen,  // number of bytes to write
            &dwBytesWritten, // number of bytes that were written
            NULL);

        bErrorFlag &= WriteFile(
        hFile,           // open file handle
        &maze->highscores.scores[i].value,      // start of data to write
        sizeof(maze->highscores.scores[i].value),  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);

    }
    


    CloseHandle(hFile);
    return TRUE;
}


void show_scoreboard(maze_t *maze) {


    if (maze->highscores.nbScores == 0) {
        printf("There is no Scoreboard for %s yet\n", maze->name);
        return;
    }
    
    printf("Scoreboard for %s (created by %s)\n", maze->name, maze->creator);

    
    
    printf("Rank.\tScore\tpseudo\n");

    for (uint8_t i = 0; i < maze->highscores.nbScores; i++) {
        printf("%d.\t%ju\t%s\n", i+1, maze->highscores.scores[i].value, maze->highscores.scores[i].pseudo);
    }

    printf("-*-*-*-*-*-*-*\n");


}


BOOL update_monsters_positions(maze_t* maze) {
    char user_yes_no = 0;
    printf("Do you want to update traps positions y/n?");
    get_user_char(&user_yes_no);
    if (user_yes_no == 'y') {
        uint64_t sizeData = (uint64_t) maze->width * (uint64_t)maze->height;

        printf("Please enter new data respecting the format, for example, current data maze is :\n");

        for (uint64_t i = 0; i < maze->data_maze.data_monster.monsters.nbMonsters; i++) {
            maze->data_maze.data_monster.monsters.monsters[i].alive = TRUE;
        }
        print_maze_monster(maze, -1, FALSE);
        printf("\n-*-*-*-*-*-*-*\n");

        fgets(maze->data_maze.data_monster.data_wall,sizeData+1, stdin);
        myfflush();
        uint64_t nbMonstersInserted = 0;
        if (strlen(maze->data_maze.data_monster.data_wall) != sizeData) {
            printf("size of input need to be %zd, given is %zd\n", sizeData, strlen(maze->data_maze.data_monster.data_wall));
            return FALSE;
        }
        for (uint64_t i = 0; i < sizeData; i++) {
            if (!(maze->data_maze.data_monster.data_wall[i] == '#' || maze->data_maze.data_monster.data_wall[i] == ' ' || maze->data_maze.data_monster.data_wall[i] == '^' || maze->data_maze.data_monster.data_wall[i] == 'o')) {
                printf("Only #, SPACE, o,  and ^ are allowed.\n");
                return FALSE;
            }
            if (maze->data_maze.data_monster.data_wall[i] == '^') {
                nbMonstersInserted++;
            }
        }
        if (nbMonstersInserted != maze->data_maze.data_monster.monsters.nbMonsters) {
            printf("Number of traps need to be %d, input = %ju\n", maze->data_maze.data_monster.monsters.nbMonsters, nbMonstersInserted);
            return FALSE;
        }
        nbMonstersInserted = 0;
        for (uint64_t i = 0; i < sizeData; i++) {
            if (maze->data_maze.data_monster.data_wall[i] == '^') {
                maze->data_maze.data_monster.data_wall[i] = ' ';
                
                maze->data_maze.data_monster.monsters.monsters[nbMonstersInserted].position = i;
                nbMonstersInserted++;
            }
            if (maze->data_maze.data_monster.data_wall[i] == 'o') {
                maze->data_maze.data_monster.data_wall[i] = ' ';
            }


        }

        maze->data_maze.data_monster.data_wall[(maze->height - 2) * maze->width + maze->width - 1] = 'o';
        return TRUE;
    }
    return FALSE;
}

BOOL upgrade_monsters(maze_t* maze) {
    char user_yes_no = 0;

    printf("Do you want to upgrade to level multipass with trap ? y/n: ");
    get_user_char(&user_yes_no);
    if (user_yes_no == 'y') {        
        maze->level = LEVEL_MONSTER;
        create_custom_maze_monster(maze);
        carve_maze_monsters(maze->data_maze.data_monster.data_wall, &maze->data_maze.data_monster.monsters, maze->width, maze->height);
        print_maze(maze, (uint64_t)maze->width + (uint64_t)1);
        return TRUE;
    }
    else {
        return FALSE;
    }
}

BOOL upgrade_multipass(maze_t* maze) {

    char user_yes_no = 0;
    printf("Do you want to upgrade to level multipass ? y/n: ");
    get_user_char(&user_yes_no);
    if (user_yes_no == 'y') {
        maze->level = LEVEL_MULTI;
        maze->data_maze.data_monster.data_wall = maze->data_maze.data_generic.data_wall;
        memset(&maze->data_maze.data_monster.monsters, 0, sizeof(maze->data_maze.data_monster.monsters));
        maze->data_maze.data_monster.monsters.nbMonsters = 0;

        create_custom_maze_multi(maze);
        carve_maze_multi(maze->data_maze.data_monster.data_wall, maze->width, maze->height, maze->data_maze.data_monster.wallProportion);
        print_maze(maze, (uint64_t) maze->width + (uint64_t)1);
        return TRUE;
    }
    else {
        return FALSE;
    }

}
BOOL remove_maze(maze_t* maze) {
    char pathMaze[MAX_NAME_MAZE + sizeof(PATH_CWD) + sizeof(PATH_MAZE_MAZE)] = { 0 };
    sprintf_s(pathMaze, MAX_NAME_MAZE + sizeof(PATH_CWD) + sizeof(PATH_MAZE_MAZE), PATH_MAZE_MAZE, PATH_CWD, maze->name);

    char pathScore[MAX_NAME_MAZE + sizeof(PATH_CWD) + sizeof(PATH_MAZE_SCORE)] = { 0 };
    sprintf_s(pathScore, MAX_NAME_MAZE + sizeof(PATH_CWD) + sizeof(PATH_MAZE_SCORE), PATH_MAZE_SCORE, PATH_CWD, maze->name);

    BOOL bError = TRUE;
    bError &= (remove(pathMaze) == 0);
    bError &= (remove(pathScore) == 0);

    return bError;
}

BOOL resave_maze(maze_t* maze) {

    char user_yes_no = 0;
    printf("Do you want to save this maze ? y/n ");
    get_user_char(&user_yes_no);
    if (user_yes_no != 'y') {
        return TRUE;
    }


    if (!remove_maze(maze)) {
        printf("Problem while removing maze\n");
        return FALSE;
    }

    
    if (!save_maze(maze) || !create_maze_score(maze)) {
        printf("Problem while saving maze\n");
    }
    
    printf("Maze upgraded !\n");

    return TRUE;
}


DWORD WINAPI AlarmThread(LPVOID lpParameter) {
    DWORD sleepMs = (DWORD)lpParameter;

    ::Sleep(sleepMs);
    ::TerminateProcess(::GetCurrentProcess(), 1);
    

    return 1;
}

bool InitChallenge(DWORD sleepMs) {
    HANDLE hThread = ::CreateThread(NULL, 0, &AlarmThread, (LPVOID)sleepMs, 0, NULL);
    if (hThread == NULL) {
        return false;
    }

    ::setvbuf(stdout, NULL, _IONBF, 0);

    return true;
}


int main()
{

    bool userRegistered = FALSE;

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    
    if (_getcwd(PATH_CWD, MAX_PATH) == NULL) {
        printf("Problem getting working dir\n");
        return 0;
    }
    
        
    if (!InitChallenge(500*1000)) {
        return -1;
    }

    
    srand((unsigned int)time(NULL));
       
    
        int choice;        
        bool firstTime;
        game_t game = { 0 };
        

        do
        {
            printf("Menu\n\n1. Register\n2. Create maze\n3. Load maze\n4. Play maze\n5. Remove maze\n6. View scoreboard\n7. Upgrade\n8. Exit\n");
            int status = get_user_int(&choice);                  

            if (status) {
                printf("This is not a number !! GoodBye\n");
                break;
            }       

            switch (choice)
            {        
            
            case MENU_REGISTER:

                if (register_user(&game.player)) {
                    userRegistered = TRUE;
                }
                else {
                    printf("Problem while registering. Please retry\n");
                    userRegistered = FALSE;
                }
                break;
            case MENU_CREATE:
                if (!userRegistered) {
                    printf("Need to register before playing\n");
                    break;
                }
                if (game.maze) {
                    free_data_maze(game.maze);
                    free(game.maze);                    
                    game.maze = NULL;
                }

                game.maze = menu_create_maze(game.player);
                    
                

                break;
            case MENU_LOAD:
                if (!userRegistered) {
                    printf("Need to register before playing\n");
                    break;
                }
                if (game.maze) {
                    free_data_maze(game.maze);
                    free(game.maze);                    
                    game.maze = NULL;                    
                }

                game.maze = load_maze();
                
                if (game.maze == NULL) {
                    printf("Problem loading maze\n");
                    break;
                }               
                if (!load_highscores(game.maze)) {
                    printf("Problem loading highscores\n");
                }

                break;

            case MENU_PLAY:
                if (!userRegistered) {
                    printf("Need to register before playing\n");
                    break;
                }

                if (game.maze == NULL) {
                    printf("Need to load a maze before playing\n");
                    break;
                }

                if (game.maze->level == LEVEL_MONSTER) {
                
                    for (uint64_t i = 0; i < game.maze->data_maze.data_monster.monsters.nbMonsters; i++) {
                        game.maze->data_maze.data_monster.monsters.monsters[i].alive = TRUE;
                    }
                }


                game.player.posX = 1;
                game.player.posY = 1;
                game.player.score = UINT64_MAX;

            
                firstTime = TRUE;
                while (!(game.player.posX == (game.maze->width - 1) && game.player.posY == (game.maze->height - 2))) {
                    
                    print_maze(game.maze, (uint64_t)game.player.posY * (uint64_t)game.maze->width + (uint64_t)game.player.posX);                    
                    char move;
                    if (firstTime) {
                        printf("Use zqsd to move and x to exit\n");
                        firstTime = FALSE;
                    }                    
                    while (get_user_move(&move)) {
                        get_user_move(&move);
                    }
                    if (move == 'x') {
                        goto endgame;
                        break;
                    }
                    apply_user_move(game.maze,&(game.player), move);
                    printf("Current score %ju\n", game.player.score);
                }
                

;
                printf("YOU WIN with a score of %zd\n", game.player.score);

                Score newScore;
                newScore.value = game.player.score;
                memcpy(newScore.pseudo, (char *)game.player.pseudo, sizeof(newScore.pseudo));                

                
                if (update_highscores(newScore, &game.maze->highscores)) {
                    save_highscores(game.maze);
                }

                endgame:
                break;
            case MENU_REMOVE:
                if (!userRegistered) {
                    printf("Need to register before removing a maze\n");
                    break;
                }

                if(game.maze == NULL)
                    game.maze = load_maze();

                if(game.maze != NULL){
                    printf("Are you sure to remove %s y/n ?", game.maze->name);
                    char user_yes_no = 0;
                    get_user_char(&user_yes_no);
                    if (user_yes_no == 'y') {
                        remove_maze(game.maze);
                        free(game.maze);
                        game.maze = NULL;
                    }
                }
                

                break;
            case MENU_SCOREBOARD:
                if (game.maze == NULL) {
                    printf("Need to load a maze before show its scoreboard\n");
                    break;
                }
                show_scoreboard(game.maze);
                break;
            case MENU_UPGRADE:
                if (game.maze == NULL) {
                    printf("Need to load a maze before upgrading it\n");
                    break;
                }
                if (game.maze->level == LEVEL_MONSTER) {                    
                    if (update_monsters_positions(game.maze)) {
                        printf("Trap positions have been updated\n");
                        resave_maze(game.maze);
                        char pathMaze[MAX_PATH];
                        sprintf_s(pathMaze, MAX_PATH, PATH_MAZE_MAZE, PATH_CWD, game.maze->name);

                        free(game.maze);
                        game.maze = NULL;
                        game.maze = load_maze_by_name(pathMaze);
                    }
                    else {
                        printf("Problem during the modification of trap positions. Please retry\n");
                    }

                    break;
                }

                print_maze(game.maze, (uint64_t) game.maze->width+ (uint64_t)1);
                
                if (game.maze->level == LEVEL_MULTI) {
                    if (upgrade_monsters(game.maze)) {
                        resave_maze(game.maze);
                    }
                }
                else {
                    if(!upgrade_multipass(game.maze)){
                        break;
                    }
                    upgrade_monsters(game.maze);
                    resave_maze(game.maze);

                }
                break;

            case MENU_QUIT: 
                printf("Goodbye\n");            
            
                break;
            default: printf("Wrong Choice. Enter again\n");
                break;
            }
        

        } while (choice != MENU_QUIT );


    return 0;
}






