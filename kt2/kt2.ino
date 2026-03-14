
#define TRUE 1
#define FALSE 0


char * commafy (long num)
{
  char a[64] = "12345";
  static char c[64] = "## ";
  sprintf(a, "%lu", num);
  double groupf = strlen(a)/3.0;
  int groupi = groupf;
  int groupm = strlen(a) % 3;
  int e, s;

  strcpy(c, "");
    
  s = 0;
  if (groupm == 0)
    {
      e = 3;
    }
    
  else
    {
      e = groupm;
    }
        
  if (groupm != 0)
    {
      groupi += 1;
    }


  for (int i=0; i<groupi; i++)
    {
      //printf("s = %d, e = %d\n", s, e);
      //#print('[{}:{}] {}'.format(s,e, a[s:e]))
      if (i == 0)
        {
	  strncat(c, &a[s], e);
	  s += e;
	  e = 3;
        }
      else
        {
	  strcat(c, ",");
	  strncat(c, &a[s], e);
	  s += 3;
	  e = 3;
        }
    }
  return((char *)c);
}

void print_board(int board[12][12])
{
  Serial.printf("\n");
  for (int j = 2; j <= 9; j++)  {
    for (int i = 2; i <= 9; i++) Serial.printf("%3d",board[i][j]) ;
    Serial.printf("\n");
  }
}

void mainx(int argc, char * argv[])   
{

  int  board[12][12] ;
  int i,j,k,move,nextx,nexty;
  int m;
  int movex[9],movey[9];
  int movtrack[65];
  int curx[65],cury[65];
  int found ;
  long iterations ;

  int startx = 2 + 0;
  int starty = 2 + 0;

  Serial.printf("Start of mainX\n");
  if (argc == 3)
    {
      startx = 2 + atoi(argv[1]);
      starty = 2 + atoi(argv[2]);
    }

  Serial.printf("Start of mainX 2\n");
  Serial.printf("Starting point (x, y) (%d, %d)\n", startx-2, starty-2);

  //printf("long is %d bytes\n", sizeof(long));
  //iterations = 0xFFFFFFFFFFFFFFFF;
  //printf("max = %u\n", iterations);

  //This for is to run multiple times
  //for (m=0; m < 100; m++)
  {
    for (i = 0; i <= 64; i++) {
      movtrack[i] = 1;
      curx[i] = 0 ; cury[i] = 0 ;
    }
    //curx[1] = 2 ;  cury[1] = 2 ;
    curx[1] = startx ;  cury[1] = starty ;
    movex[1] =  1; movey[1] =  2;
    movex[2] =  2; movey[2] =  1;
    movex[3] =  2; movey[3] = -1;
    movex[4] =  1; movey[4] = -2;
    movex[5] = -1; movey[5] = -2;
    movex[6] = -2; movey[6] = -1;
    movex[7] = -2; movey[7] =  1;
    movex[8] = -1; movey[8] =  2;
    for (i = 0; i <= 11; i++) for (j = 0; j <= 11; j++) board[i][j] = 0;
    for (i = 0; i <= 11; i++) {
      board[i][0] = -1; board[i][1] = -1;
      board[i][10] = -1; board[i][11] = -1;
      board[0][i] = -1; board[1][i] = -1;
      board[10][i] = -1; board[11][i] = -1;
    }
    //board[2][2] = 1;
    board[startx][starty] = 1;
    iterations = 1;

    //Try starting with a different move
    //movtrack[2] = 2;

    /* initialization complete */
    int highest_move = 1;
    move = 2 ;
    time_t start_time = time(NULL);
    time_t curr_time;
    //printf("start time: %lu\n", start_time);
   
    while (move <= 64) {
      //if (move == 64) break;
      /*       printf("\r%7td",iterations++);  */
      iterations++;

      if (iterations == 0) Serial.printf("iterations rolled over\n");
      found = FALSE ;
      while ( !found ) {
	nextx = curx[move-1] + movex[movtrack[move]];
	nexty = cury[move-1] + movey[movtrack[move]];
	if (board[nextx][nexty] == 0) {         /* open square found */
	  board[nextx][nexty] = move ;
	  found = TRUE ;
	  curx[move] = nextx ; cury[move] = nexty ;
	  move = move + 1;
	}
	else {            /* square illegal or already occupied */
	  if (movtrack[move] < 8) {        /* try another move */
	    movtrack[move] = movtrack[move] + 1;
	  }
	  else {           /* out of moves, have to back up */
	    while (movtrack[move] == 8) {
	      /*                Serial.printf("."); */
	      movtrack[move] = 1;
	      board[curx[move-1]][cury[move-1]] = 0 ;
	      move = move - 1;
	      if (move == 1) {
		Serial.printf("No solution, %s iterations\n", commafy(iterations));
		exit(1);
	      }
		    
	      /*                Serial.printf("\r%2d",move); */
	    }
            movtrack[move] = movtrack[move] +1 ;
	  }
        }
      }
    }    /* end while */

    print_board(board);
    
    /* this value will be 8250733 when started from 2,2 (i.e., a corner) */
    //printf("%lu (%s) iterations\n",iterations, commafy(iterations));
    Serial.printf("\n Solved in %s iterations\n",commafy(iterations));
  } // end of the multi-run loop
  
  Serial.printf("Returning from mainx\n");
}


void setup() {
  
  //WiFi.mode(WIFI_OFF);
  Serial.begin(115200);

  delay(1000);
  Serial.printf("Let's get started\n");

  uint32_t start = micros();
  mainx(0, NULL);
  uint32_t end = micros();
  double et = (end-start) / 1000000.0;
  //Serial.printf("ET = %dmS  %lfS\n", end-start, et);
  Serial.printf("ET = %lf seconds\n", et);

}

void loop() {
  delay(2000);
  //Serial.printf("in the loop\n");
}

