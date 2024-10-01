# Conway's Game of Life
complie with `clang -o gol gol.c`
<br>run with `./gol` with up to 5 options:
* width
* height
* fps
* flip trail
* frame skip

<p>- flip trail is how many frames you want it to continue after it's found a 2-frame stable equilibrium, counting starts after it's shown the last unique frame, default is 0
<br>- frame skip is how many of the first frames you want to skip; this is good for finding stable equilibriums with more than 2 frames, default is 0
<br>- width, height & fps are all 20 by default