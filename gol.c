#include <signal.h> // signal
#include <stdint.h> // uint8_t
#include <stdio.h> // printf
#include <string.h> // strlen
#include <ctype.h>  // isdigit
#include <stdlib.h> // atoi, rand & exit
#include <stdbool.h> // bool
#include <unistd.h> // usleep
#include <time.h> // time

#define maxArgs 5

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

void morph(bool*** grids,uint8_t w,uint8_t h,bool* gridRepeat,uint8_t gridCount){
    // create neighbour count array & set previous grids
    uint8_t** neighbourSum=malloc(h*sizeof(uint8_t*)); // dynamically sized 2d uint8_t array, neighbourSum, is a pointer to a pointer to a uint8_t, here we will count how many neighbours are on for each square
    for (uint8_t y=0;y<h;++y){
        neighbourSum[y]=malloc(w*sizeof(uint8_t)); // dynamically sized 1d uint8_t array (row) is a pointer to a uint8_t but that's not needing declared here
        for (uint8_t x=0;x<w;++x){
            bool neighbours[3][3]={ // set both 3s to 5s for 5×5 neighbour square
                // {true,true,true,true,true},
                // {true,true,true,true,true},
                // {true,true,false,true,true},
                // {true,true,true,true,true},
                // {true,true,true,true,true}}; // 2d 5×5 bool array for mask of which neighbouring squares to count, don't include self at (2,2)
                {true,true,true},
                {true,false,true},
                {true,true,true}}; // 2d 3×3 bool array for mask of which neighbouring squares to count, don't include self at (1,1)
            if (x==0){ // don't include squares left of self if at left border (*,0)
                neighbours[0][0]=false; // include for 5x5 neighbour square
                neighbours[1][0]=false;
                neighbours[2][0]=false;
            //     neighbours[3][0]=false;
            //     neighbours[4][0]=false;
            //     neighbours[0][1]=false;
            //     neighbours[1][1]=false;
            //     neighbours[2][1]=false;
            //     neighbours[3][1]=false;
            //     neighbours[4][1]=false;
            // }
            // else if (x==1){ // don't include squares left of self if at left border (*,0)
            //     neighbours[0][0]=false;
            //     neighbours[1][0]=false;
            //     neighbours[2][0]=false;
            //     neighbours[3][0]=false;
            //     neighbours[4][0]=false;
            // }
            // if (x==w-2){ // don't include squares right of self if at right border (*,2)
            //     neighbours[0][4]=false;
            //     neighbours[1][4]=false;
            //     neighbours[2][4]=false;
            //     neighbours[3][4]=false;
            //     neighbours[4][4]=false;
            }
            if (x==w-1){ // don't include squares right of self if at right border (*,2)
                neighbours[0][2]=false; // don't include for 5x5 neighbour square
                neighbours[1][2]=false;
                neighbours[2][2]=false;
                // neighbours[0][3]=false;
                // neighbours[1][3]=false;
                // neighbours[2][3]=false;
                // neighbours[3][3]=false;
                // neighbours[4][3]=false;
                // neighbours[0][4]=false;
                // neighbours[1][4]=false;
                // neighbours[2][4]=false;
                // neighbours[3][4]=false;
                // neighbours[4][4]=false;
            }
            if (y==0){ // don't include squares below self if at bottom border (0,*)
                neighbours[0][0]=false; // include for 5x5 neighbour square
                neighbours[0][1]=false;
                neighbours[0][2]=false;
            //     neighbours[0][3]=false;
            //     neighbours[0][4]=false;
            //     neighbours[1][0]=false;
            //     neighbours[1][1]=false;
            //     neighbours[1][2]=false;
            //     neighbours[1][3]=false;
            //     neighbours[1][4]=false;
            // }
            // if (y==1){ // don't include squares below self if at bottom border (1,*)
            //     neighbours[0][0]=false;
            //     neighbours[0][1]=false;
            //     neighbours[0][2]=false;
            //     neighbours[0][3]=false;
            //     neighbours[0][4]=false;
            // // }
            // if (y==h-2){ // don't include squares above self if at top border (2nd top,*)
            //     neighbours[4][0]=false;
            //     neighbours[4][1]=false;
            //     neighbours[4][2]=false;
            //     neighbours[4][3]=false;
            //     neighbours[4][4]=false;
            }
            if (y==h-1){ // don't include squares above self if at top border (top,*)
                neighbours[2][0]=false; // don't include for 5x5 neighbour square
                neighbours[2][1]=false;
                neighbours[2][2]=false;
                // neighbours[3][0]=false;
                // neighbours[3][1]=false;
                // neighbours[3][2]=false;
                // neighbours[3][3]=false;
                // neighbours[3][4]=false;
                // neighbours[4][0]=false;
                // neighbours[4][1]=false;
                // neighbours[4][2]=false;
                // neighbours[4][3]=false;
                // neighbours[4][4]=false;
            }
            neighbourSum[y][x]=0; // start neighbour count at 0
            for (uint8_t y_=0;y_<3;++y_){ // change 3 to 5 for 5x5 neighbour square
                for (uint8_t x_=0;x_<3;++x_){ // change 3 to 5 for 5x5 neighbour square
                    if (neighbours[y_][x_] && grids[0][y+y_-1][x+x_-1]) ++neighbourSum[y][x]; // count neighbouring squares that exist & are on; change -1 to -2 for 5x5 neighbour square
                }
            }
            for (uint8_t i=gridCount-2;i<gridCount-1;i--){
                grids[i+1][y][x]=grids[i][y][x]; // set previous grids
            }
        }
    }

    // derive new grid & determine state
    for (uint8_t i=0;i<gridCount-1;i++){
        gridRepeat[i]=true;
    }
    for (uint8_t y=0;y<h;++y){
        for (uint8_t x=0;x<w;++x){
            if (neighbourSum[y][x]==3||(grids[0][y][x]&&neighbourSum[y][x]==2)){
                grids[0][y][x]=true; // turn on squares that either have 3 active neighbours or are on & have 2 active neighbours (Conway's GoL rules)
            } else{
                grids[0][y][x]=false; // turn off rest
            }
            for (uint8_t i=0;i<gridCount-1;i++){
                if (grids[0][y][x]!=grids[i+1][y][x]){
                    gridRepeat[i]=false; // if there's any change since nth last frame
                }
            }
        }
    }
}

int main(int argc,char** argv){ // main has to take an int (no. of args) & a char*[] (array of char pointers (strings))
    signal(SIGINT,handleSIGINT); // catch ctrl+c
    printf("\033[?25l"); // hide cursor

    // validate input
    if (argc>maxArgs+1){ // if more than [maxArgs+1] args (program name is an arg)
        printf("takes up to %u optional args: width, height, fps, frame skip & grid count\n",maxArgs);
        printf("\033[?25h"); // show cursor
        return 1; // exit with error code
    }
    uint8_t args[maxArgs];
    uint8_t defaults[maxArgs]={20,20,20,0,3};
    for (uint8_t i=0;i<maxArgs;++i){
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
    const uint8_t frameSkip=args[3];
    const uint8_t gridCount=args[4];

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
    bool*** grids=malloc(gridCount*sizeof(bool**));
    srand(time(NULL)); // seed random number generator with current time

    // repeats upon finish
    while (true){
        for (uint8_t i=0;i<gridCount;i++){ // for each grid
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
        bool* gridRepeat=malloc((gridCount-1)*sizeof(bool));
        uint8_t* flips=malloc((gridCount-2)*sizeof(uint8_t));
        for (uint8_t i=0;i<gridCount-2;i++){
            gridRepeat[i]=false;
            flips[i]=0;
        }
        bool flopped=false;
        uint64_t frame=0;
        while (true){
            printGrid(grids,w,h); // show most recent grid
            if (gridRepeat[0]) break; // end program if no squares have changed
            for (uint8_t i=1;i<gridCount-1;i++){
                if (gridRepeat[i]){
                    if (++flips[i-1]>fps){ // iterate flips & end program after 1s
                        flopped=true;
                        break;
                    }
                }
            }
            if (flopped) break;
            morph(grids,w,h,gridRepeat,gridCount); // morph grid with Conway's GoL rules & overwrite oldest grid
            if (++frame>(frameSkip!=1?frameSkip:1e5)) usleep(1e6/fps); // skip frame delay for first n frames
            printf("\033[%uA",(h+1)/2); // move cursor back to start for next frame to overwrite previous
        }
        // break; // uncomment this line for it to run just once
        printf("\033[%uA",(h+1)/2); // move cursor back to start for next game
    }
    printf("\033[?25h"); // show cursor
    return 0;
}
