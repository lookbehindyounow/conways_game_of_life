#include <signal.h> // signal
#include <stdint.h> // uint8_t
#include <stdio.h> // printf
#include <string.h> // strlen
#include <ctype.h>  // isdigit
#include <stdlib.h> // atoi, rand & exit
#include <stdbool.h> // bool
#include <unistd.h> // usleep
#include <time.h> // time

void handleSIGINT(int signal){ // before quitting program
    printf("\033[?25h"); // show cursor
    exit(0);
}

uint8_t validate(char* arg){ // validate input for uint8_t
    const uint64_t argLen=strlen(arg);
    if (argLen>3){
        return 0; // exit with error code
    }
    for (uint8_t c=0;c<argLen;++c){
        if (!isdigit(arg[c])){ // if any characters aren't 0-9
            return 0; // exit with error code
        }
    }
    uint16_t intArg=atoi(arg); // convert to at most 3-digit int
    if (intArg<1||intArg>255){ // if 0 or too big for uint8_t
        return 0; // exit with error code
    }
    return intArg; // return validated uint8_t
}

void printGrid(bool*** grids,uint8_t w,uint8_t h){ // display the most recent grid in the console
    for (uint8_t y=0;y<h;y+=2){ // y increments in twos because our pixels are 2 squares tall, using these: ▄ ▀ █
        for (uint8_t x=0;x<w;++x){
            if (grids[0][y][x]){ // upper square is on
                if (y<h-1 && grids[0][y+1][x]){ // lower square exists & both squares are on
                    printf("█"); // draw both squares
                } else{
                    printf("▀"); // draw upper square
                }
            } else if (y<h-1 && grids[0][y+1][x]){ // lower square exists & is on
                printf("▄");  // draw lower square
            } else{
                printf(" "); // draw neither squares
            }
        }
        printf("\n"); // next line
    }
}

void morph(bool*** grids,uint8_t w,uint8_t h,bool* gridAlive){
    // create neighbour count array & set previous grids
    uint8_t** neighbourSum=malloc(h*sizeof(uint8_t*)); // dynamically sized 2d uint8_t array, neighbourSum, is a pointer to a pointer to a uint8_t, here we will count how many neighbours are on for each square
    for (uint8_t y=0;y<h;++y){
        neighbourSum[y]=malloc(w*sizeof(uint8_t)); // dynamically sized 1d uint8_t array (row) is a pointer to a uint8_t but that's not needing declared here
        for (uint8_t x=0;x<w;++x){
            bool neighbours[3][3]={{true,true,true},{true,false,true},{true,true,true}}; // 2d 3×3 bool array for mask of which neighbouring squares to count, don't include self at (1,1)
            if (x==0){ // don't include squares left of self if at left border (*,0)
                neighbours[0][0]=false;
                neighbours[1][0]=false;
                neighbours[2][0]=false;
            }
            if (x==w-1){ // don't include squares right of self if at right border (*,2)
                neighbours[0][2]=false;
                neighbours[1][2]=false;
                neighbours[2][2]=false;
            }
            if (y==0){ // don't include squares below self if at bottom border (0,*)
                neighbours[0][0]=false;
                neighbours[0][1]=false;
                neighbours[0][2]=false;
            }
            if (y==h-1){ // don't include squares above self if at top border (2,*)
                neighbours[2][0]=false;
                neighbours[2][1]=false;
                neighbours[2][2]=false;
            }
            neighbourSum[y][x]=0; // start neighbour count at 0
            for (uint8_t y_=0;y_<3;++y_){
                for (uint8_t x_=0;x_<3;++x_){
                    if (neighbours[y_][x_] && grids[0][y+y_-1][x+x_-1]) ++neighbourSum[y][x]; // count neighbouring squares that exist & are on
                }
            }
            grids[2][y][x]=grids[1][y][x]; // set oldest grid to previous grid
            grids[1][y][x]=grids[0][y][x]; // set previous grid to most recent grid
        }
    }

    // derive new grid & determine state
    gridAlive[0]=false;
    gridAlive[1]=false;
    for (uint8_t y=0;y<h;++y){
        for (uint8_t x=0;x<w;++x){
            if (neighbourSum[y][x]==3||(grids[0][y][x]&&neighbourSum[y][x]==2)){
                grids[0][y][x]=true; // turn on squares that either have 3 active neighbours or are on & have 2 active neighbours (Conway's GoL rules)
            } else{
                grids[0][y][x]=false; // turn off rest
            }
            if (grids[0][y][x]!=grids[1][y][x]){
                gridAlive[0]=true; // if there's any change since last frame
            }
            if (grids[0][y][x]!=grids[2][y][x]){
                gridAlive[1]=true; // if there's any difference to oldest frame
            }
        }
    }
}

int main(int argc,char** argv){ // main has to take an int (no. of args) & a char*[] (array of char pointers (strings))
    signal(SIGINT,handleSIGINT); // catch ctrl+c
    printf("\033[?25l"); // hide cursor

    // validate input
    if (argc>6){ // if more than 5 args (program name is an arg)
        printf("takes up to 5 optional args: width, height, fps, flip trail & frame skip\n");
        printf("\033[?25h"); // show cursor
        return 1; // exit with error code
    }
    uint8_t args[5];
    uint8_t defaults[5]={20,20,20,0,0};
    for (int i=0;i<5;++i){
        if (i<argc-1){ // if argument given
            args[i]=validate(argv[i+1]);
            if (args[i]==0){
                printf("args have to be ints from 1-255");
                printf("\033[?25h"); // show cursor
                return 1; // exit with error code
            }
        } else{
            args[i]=defaults[i];
        }
    }
    const uint8_t w=args[0];
    const uint8_t h=args[1];
    const uint8_t fps=args[2];
    const uint8_t flipTrail=args[3];
    const uint8_t frameSkip=args[4];

    // bool grid[33][37]={ // this is the starter grid from the alpha phoenix yt video on Conway's GoL
    //     {0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,1,0,0,0,0,0,0},
    //     {0,0,0,0,0,1,0,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
    //     {0,0,0,1,0,0,1,1,1,1,1,1,1,1,0,0,1,0,1,1,0,1,1,0,1,0,0,1,1,1,0,1,0,0,0,0,0},
    //     {0,1,0,0,1,1,0,1,0,1,0,0,0,1,1,0,1,1,0,1,0,0,0,0,1,0,1,0,0,1,1,0,0,0,1,0,0},
    //     {0,0,1,1,1,1,0,0,1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,0,0,0,0},
    //     {0,1,1,0,1,0,0,0,1,1,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,1,0,0,0,0,0,1},
    //     {0,1,1,0,1,1,0,1,0,1,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,0,0,0},
    //     {0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,0,0,0},
    //     {1,0,1,1,1,1,0,1,1,0,1,0,1,0,0,1,0,0,1,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,1,0,0},
    //     {0,1,0,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1},
    //     {1,1,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0},
    //     {1,1,0,0,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,0,0,0,0,1,1,1,1,0,1,1,0,0,0,0},
    //     {0,1,1,1,0,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0},
    //     {0,1,1,1,0,1,0,0,1,1,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0},
    //     {1,1,0,0,0,1,1,1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,0,0},
    //     {1,1,1,0,1,0,0,1,0,0,1,1,0,0,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0},
    //     {0,1,1,1,0,1,0,0,1,0,0,0,1,1,1,0,0,1,0,1,1,1,0,0,0,0,1,1,1,0,1,1,1,1,1,0,0},
    //     {1,1,1,0,1,0,0,1,0,0,1,1,0,0,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0},
    //     {1,1,0,0,0,1,1,1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,0,0},
    //     {0,1,1,1,0,1,0,0,1,1,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0},
    //     {0,1,1,1,0,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0},
    //     {1,1,0,0,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,0,0,0,0,1,1,1,1,0,1,1,0,0,0,0},
    //     {1,1,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0},
    //     {0,1,0,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1},
    //     {1,0,1,1,1,1,0,1,1,0,1,0,1,0,0,1,0,0,1,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,1,0,0},
    //     {0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,1,0,1,1,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,0,0,0},
    //     {0,1,1,0,1,1,0,1,0,1,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,0,0,0},
    //     {0,1,1,0,1,0,0,0,1,1,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,1,0,0,0,0,0,1},
    //     {0,0,1,1,1,1,0,0,1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,0,0,0,0},
    //     {0,1,0,0,1,1,0,1,0,1,0,0,0,1,1,0,1,1,0,1,0,0,0,0,1,0,1,0,0,1,1,0,0,0,1,0,0},
    //     {0,0,0,1,0,0,1,1,1,1,1,1,1,1,0,0,1,0,1,1,0,1,1,0,1,0,0,1,1,1,0,1,0,0,0,0,0},
    //     {0,0,0,0,0,1,0,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
    //     {0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,1,0,0,0,0,0,0}
    // };

    // create grids
    bool*** grids=malloc(24);
    srand(time(NULL)); // seed random number generator with current time

    // repeats upon finish
    while (true){
        for (uint8_t i=0;i<3;i++){ // for each grid
            grids[i]=malloc(h*sizeof(bool*)); // dynamically sized 2d bool array
            for (uint8_t y=0;y<h;++y){ // for each row
                grids[i][y]=malloc(w*sizeof(bool)); // dynamically sized 2d bool array
                if (i==0){ // for first grid
                    for (uint8_t x=0;x<w;++x){ // for each column
                        grids[0][y][x]=rand()%2; // random distribution of on/off
                        // grids[0][y][x]=grid[y][x];
                    }
                }
            }
        }

        // main loop for one run through of the game
        bool gridAlive[2]={true,true};
        uint8_t flips=0;
        uint64_t frame=0;
        while (true){
            printGrid(grids,w,h); // show most recent grid
            if (!gridAlive[0]) break; // end program if no squares have changed
            if (!gridAlive[1]){ // if flipping between two grid states
                if (flips++>flipTrail) break; // iterate flips & end program after some amount
            }
            morph(grids,w,h,gridAlive); // morph grid with Conway's GoL rules & overwrite oldest grid
            if (++frame>frameSkip) usleep(1e6/fps); // skip frame delay for first n frames
            printf("\033[%uA",(h+1)/2); // move cursor back to start for next frame to overwrite previous
        }
        break;
        printf("\033[%uA",(h+1)/2); // move cursor back to start for next game
    }
    printf("\033[?25h"); // show cursor
    return 0;
}
