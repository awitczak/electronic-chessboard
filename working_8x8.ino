#include <Streaming.h>

// each column of sensors
#define col1 4
#define col2 5
#define col3 6
#define col4 7
#define col5 8
#define col6 9
#define col7 10
#define col8 11
// each row of sensors
#define row1 A0
#define row2 A1
#define row3 A2
#define row4 A3
#define row5 A4
#define row6 A5
#define row7 A6
#define row8 A7

#define BOARD_SIZE 8
#define EMPTY 0

// NEW VERSION
enum chess_pieces
{
  pawn,
  knight,
  bishop,
  rook,
  queen,
  king
};

// in order pawn/knight/bishop/rook/queen/king
const char pieces_W[] = { '1', '2', '3', '4', '5', '6' };
const char pieces_B[] = { 'a', 'b', 'c', 'd', 'e', 'f' };

enum moves
{
  normal,
  capture,
  short_castle,
  long_castle,
  pawn_promotion,
  pawn_promotion_capture
};

enum chess_colors
{
  black,
  white
};

enum chess_actions
{ 
  lifted,
  placed,
  reset
};

// variables used for the PGN generation
const int field_numbers[] = { 8, 7, 6, 5, 4, 3, 2, 1 };
const char field_letters[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
const String piece_letters[] = {"\0", "N", "B", "R", "Q", "K"};
const String move_letters[] = {"\0", "x", "O-O", "O-O-O", "="};
int move_iter = 0;
String move_str;

// buffer for received commands
String command;

char temp_piece;

// variable used for sending the square positions on which a move was performed
String squares_pos_str;

struct
{ 
  bool moveFound = false;

  // declaring starting position of the pieces
  bool initial_pos_reached = false;
  bool wrong_pos_found = false;

  // return to last correct position
  bool correct_pos_reached = false;

  // turn determination
  bool whose_turn = white;

  // castle possible?
  bool w_king_moved = false;
  bool b_king_moved = false;

} flags;

// user action history
const uint8_t N_actions = 4;

struct {
  uint8_t iter = 0;
  uint8_t action[N_actions] = { reset, reset, reset, reset };
  uint8_t color[N_actions] = { reset, reset, reset, reset };
  char piece[N_actions] = { '0', '0', '0', '0' };
  uint8_t pos_X[N_actions] = { EMPTY, EMPTY, EMPTY, EMPTY };
  uint8_t pos_Y[N_actions] = { EMPTY, EMPTY, EMPTY, EMPTY };
} action_data;

char pieces_pos[BOARD_SIZE][BOARD_SIZE] = {
  pieces_B[rook], pieces_B[knight], pieces_B[bishop], pieces_B[queen], pieces_B[king], pieces_B[bishop], pieces_B[knight], pieces_B[rook],
  pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn],
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn],
  pieces_W[rook], pieces_W[knight], pieces_W[bishop], pieces_W[queen], pieces_W[king], pieces_W[bishop], pieces_W[knight], pieces_W[rook],
};

char prev_pos[BOARD_SIZE][BOARD_SIZE] = {
  '1', '1', '1', '1', '1', '1', '1', '1', 
  '1', '1', '1', '1', '1', '1', '1', '1',  
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '1', '1', '1', '1', '1', '1', '1', '1', 
  '1', '1', '1', '1', '1', '1', '1', '1',
};

char curr_pos[BOARD_SIZE][BOARD_SIZE] = {
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
};

// variables holding the temporary state befora move is defined/not defined
char temp_prev_pos[BOARD_SIZE][BOARD_SIZE] = {
  '1', '1', '1', '1', '1', '1', '1', '1', 
  '1', '1', '1', '1', '1', '1', '1', '1',  
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '1', '1', '1', '1', '1', '1', '1', '1', 
  '1', '1', '1', '1', '1', '1', '1', '1',
};

char temp_pieces_pos[BOARD_SIZE][BOARD_SIZE] = {
  pieces_B[rook], pieces_B[knight], pieces_B[bishop], pieces_B[queen], pieces_B[king], pieces_B[bishop], pieces_B[knight], pieces_B[rook],
  pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn], pieces_B[pawn],
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  '0', '0', '0', '0', '0', '0', '0', '0', 
  pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn], pieces_W[pawn],
  pieces_W[rook], pieces_W[knight], pieces_W[bishop], pieces_W[queen], pieces_W[king], pieces_W[bishop], pieces_W[knight], pieces_W[rook],
};

void(* resetFunc) (void) = 0;

void setup() {
  // column setup
  pinMode(col1, OUTPUT);
  pinMode(col2, OUTPUT);
  pinMode(col3, OUTPUT);
  pinMode(col4, OUTPUT);
  pinMode(col5, OUTPUT);
  pinMode(col6, OUTPUT);
  pinMode(col7, OUTPUT);
  pinMode(col8, OUTPUT);

  // row setup
  pinMode(row1, INPUT);
  pinMode(row2, INPUT);
  pinMode(row3, INPUT);
  pinMode(row4, INPUT);
  pinMode(row5, INPUT);
  pinMode(row6, INPUT);
  pinMode(row7, INPUT);
  pinMode(row8, INPUT);

  // all columns on LOW initially, the sensors are off
  digitalWrite(col1, LOW);
  digitalWrite(col2, LOW);
  digitalWrite(col3, LOW);
  digitalWrite(col4, LOW);
  digitalWrite(col5, LOW);
  digitalWrite(col6, LOW);
  digitalWrite(col7, LOW);
  digitalWrite(col8, LOW);

  // debug
  Serial.begin(9600);
  BT_sendMSG("BT_CHESSBOARD connected!");
}

void loop() {
  // check if the chessboard should restart
  check_RESET();

  // waiting for initial board configuration
  if (!flags.initial_pos_reached) 
  {
    board_waitForPos(flags.initial_pos_reached, "Waiting for initial configuration!", "Game starts.");
  }

  while (!flags.moveFound) // until a move is registered
  { 
    // check if the chessboard should restart
    check_RESET();

    if (action_data.iter < N_actions) // when there are still actions to be performed
    { 
      // scan the sensors
      board_update();

      // check the changes
      action_check();

      // save changes from action_check
      save_data(temp_prev_pos, curr_pos);

      // look for a move
      move_check();
    }
    else
    { 
      //Serial.println("Move not recognized!");
      
      //Serial.println("RETURN TO: ");
      //board_print_data(pieces_pos);

      // waiting for last correct state
      flags.correct_pos_reached = false;
      flags.wrong_pos_found = false;
      board_waitForPos(flags.correct_pos_reached, "Return to the currently displayed position!", "The game can proceed.");
      
      // reset the actions
      delete_action_data();

      // set the new temp position to the previous correct position
      save_data(temp_prev_pos, prev_pos);

      // set the new temp pieces pos to the previous pieces pos
      save_data(temp_pieces_pos, pieces_pos);
    }   
  }
  
  // saving state after the latest move
  save_data(prev_pos, curr_pos);
  //Serial.println("\nSTATE SAVED AFTER MOVE: ");
  //board_print_data(prev_pos);
  save_data(pieces_pos, temp_pieces_pos);
  //Serial.println("\nPIECES SAVED AFTER MOVE: ");
  //board_print_data(pieces_pos);

  // copy prev_pos to temp_prev_pos
  save_data(temp_prev_pos, prev_pos);

  // sending the data to the computer
  BT_sendMoveFrame(move_str, squares_pos_str);

  // reset the flags
  flags.moveFound = false;
}

void check_RESET()
{
  // listening for commands from the computer
  if (Serial.available() > 0)
  {
    command = Serial.readString();
  }

  // reset command
  if (command == "RESET")
  { 
    command = "";
    resetFunc();
  }
}

String array2D_ToStr(char fromThis[][BOARD_SIZE])
{
  String str;

  for (int y = 0; y < BOARD_SIZE; y++)
  {
    for (int x = 0; x < BOARD_SIZE; x++)
    { 
      str += (String) fromThis[y][x];
    }
  }
  return str;
}

void save_data(char toThis[][BOARD_SIZE], char fromThis[][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; i++) 
  {
    for (int j = 0; j < BOARD_SIZE; j++) 
    {
      toThis[i][j] = fromThis[i][j];
    }
  }
}


void BT_sendErrorNotification(String msg)
{
  String Msg;

  Msg = "0#" + msg;

  Serial.println(Msg);
}


void BT_sendMoveFrame(String pgn, String squares_pos)
{
  String pieces_pos_data;
  String dataframe;

  dataframe = "1#" + array2D_ToStr(pieces_pos) + "#" + pgn + "#" + squares_pos;

  //Serial << dataframe << endl;

  // send the data frame
  Serial.println(dataframe);
}

void BT_sendMSG(String msg)
{
  String Msg;

  Msg = "2#" + msg;

  // send the message
  Serial.println(Msg);
}

void print_action_data()
{
  Serial << "---------------" << endl;
  
  Serial << "ACTION: ";
  for (int x = 0; x < N_actions; x++)
  { 
    Serial << action_data.action[x] << " ";
  }
  Serial << endl;
  Serial << "COLOR : ";
  for (int x = 0; x < N_actions; x++)
  { 
    Serial << action_data.color[x] << " ";
  }
  Serial << endl;
  Serial << "PIECE : ";
  for (int x = 0; x < N_actions; x++)
  { 
    Serial << action_data.piece[x] << " ";
  }
  Serial << endl;

}

void board_print_data(char printThis[][BOARD_SIZE])
{ 
  Serial << "---------------" << endl;
  for (int y = 0; y < BOARD_SIZE; y++)
  {
    for (int x = 0; x < BOARD_SIZE; x++)
    { 
      Serial << printThis[y][x] << " ";
    }
    Serial << endl;
  }
}

void board_waitForPos(bool &flag, String before_msg, String after_msg)
{ 
  //Serial.println(before_msg);
  BT_sendErrorNotification(before_msg);
  while (!flag)
  { 
    check_RESET();

    board_update();
    // checking if every position matches the initial requirements
    for (int y = 0; y < BOARD_SIZE; y++)
    {
      for (int x = 0; x < BOARD_SIZE; x++)
      { 
        if (prev_pos[y][x] != curr_pos[y][x])
        {
          flags.wrong_pos_found = true;
        }
      }
    }
    if (!flags.wrong_pos_found)
    {
      //Serial.println(after_msg);
      BT_sendMSG(after_msg);

      flag = true;
    }
    // resetting the flag for the next iteration of board checking
    flags.wrong_pos_found = false;
  }
}

void board_update()
{
  // setting the columns to HIGH one at a time
  for (int y = 0; y < BOARD_SIZE; y++)
  { 
    int temp_col = y + col1; // due to pin assignments
    digitalWrite(temp_col, HIGH);

    // checking each row for a sensor signal
    for (int x = 0; x < BOARD_SIZE; x++)
    { 
      int temp_row = x + row1; // due to pin assignments

      if (analogRead(temp_row) < 500)
      { 
        curr_pos[x][y] = '1';
      }
      else
      { 
        curr_pos[x][y] = '0';
      }
    }
    digitalWrite(temp_col, LOW);
  }
}

bool color_check(char chess_piece)
{ 
  if ((int)chess_piece > 48 && (int)chess_piece < 55) return white;
  else return black;
}

void action_check()
{ 
  // checking each square for a change
  for (int y = 0; y < BOARD_SIZE; y++)
  {
    for (int x = 0; x < BOARD_SIZE; x++)
    { 
      // if a change occurs
      if (temp_prev_pos[y][x] != curr_pos[y][x]) 
      { 
        // lifting a piece
        if (temp_prev_pos[y][x] > curr_pos[y][x])
        { 
          temp_piece = temp_pieces_pos[y][x];
          temp_pieces_pos[y][x] = '0'; // clearing the field

          action_data.action[action_data.iter] = lifted;
          action_data.color[action_data.iter] = color_check(temp_piece);
          action_data.piece[action_data.iter] = temp_piece; // which piece moved?
          action_data.pos_X[action_data.iter] = x;
          action_data.pos_Y[action_data.iter] = y;
        }
        // placing a piece
        if (temp_prev_pos[y][x] < curr_pos[y][x])
        { 
          // the actual placing of a piece will be performed after a move is distinguished
          action_data.action[action_data.iter] = placed;
          action_data.color[action_data.iter] = color_check(temp_piece);
          action_data.piece[action_data.iter] = '0'; // not required
          action_data.pos_X[action_data.iter] = x;
          action_data.pos_Y[action_data.iter] = y;
        }
        // action occurred so increment the iter
        action_data.iter++;
      }
    }
  }
}

void delete_action_data()
{
  for (int i = 0; i < N_actions; i++)
  {
    action_data.action[i] = reset;
    action_data.color[i] = reset;
    action_data.piece[i] = '0';
  }
  // action iter reset
  action_data.iter = 0;
}

String save_squares_pos(int firstSquare_X, int firstSquare_Y, int secondSquare_X, int secondSquare_Y)
{
  String squares_pos;

  squares_pos = (String)firstSquare_X + (String)firstSquare_Y + (String)secondSquare_X  + (String)secondSquare_Y;

  return squares_pos;
}

void move_check()
{ 
  // whose turn determination
  if (flags.whose_turn == white)
  {
    if (action_data.color[0] == black && action_data.action[0] == lifted) // initial action has to be 'lifted' | in this case if a black piece is lifted first on white turn, it has to be a capture
    { 
      if (action_data.color[1] == white && action_data.action[1] == lifted)
      {
        if (action_data.action[2] == placed && action_data.pos_X[2] == action_data.pos_X[0] && action_data.pos_Y[2] == action_data.pos_Y[0]) // wait for placement and correct position (the capturing piece needs to land on the correct square)
        { 
          // check if it is a pawn promotion with capture
          if (action_data.piece[1] == pieces_W[pawn] && action_data.pos_Y[2] == 0) // if placed on the 8th rank = promotion
          { 
            // placing the chosen piece on the given square
            temp_pieces_pos[action_data.pos_Y[0]][action_data.pos_X[0]] = pieces_W[queen];

            // printing the move
            move_print(pawn_promotion_capture, action_data.piece[1], action_data.pos_Y[0], action_data.pos_X[0], action_data.pos_X[1]);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = black;
            delete_action_data();
              
            // change the flags
            flags.moveFound = true;
          }
          else 
          {
            // placing the white piece on the position of the black piece
            temp_pieces_pos[action_data.pos_Y[0]][action_data.pos_X[0]] = action_data.piece[1];

            // printing the move
            move_print(capture, action_data.piece[1], action_data.pos_Y[0], action_data.pos_X[0], action_data.pos_X[1]);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[2], action_data.pos_Y[2], action_data.pos_X[0], action_data.pos_Y[0]);

            flags.whose_turn = black;
            delete_action_data();
            
            // change the flags
            flags.moveFound = true;
          }
        }
      }
    }
    else if (action_data.color[0] == white && action_data.action[0] == lifted) // if first white is lifted, then we can check other possibilities
    {
      if (action_data.color[1] == black && action_data.action[1] == lifted) // it has to be a capture
      {
        if (action_data.action[2] == placed && action_data.pos_X[2] == action_data.pos_X[1] && action_data.pos_Y[2] == action_data.pos_Y[1]) // wait for placement and correct position (the capturing piece needs to land on the correct square)
        {
          // check if it is a pawn promotion with capture
          if (action_data.piece[0] == pieces_W[pawn] && action_data.pos_Y[2] == 0) // if placed on the 8th rank = promotion
          { 
            // placing the chosen piece on the given square
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = pieces_W[queen];

            // printing the move
            move_print(pawn_promotion_capture, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], action_data.pos_X[0]);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = black;
            delete_action_data();
              
            // change the flags
            flags.moveFound = true;
          }
          else
          {
            // placing the white piece on the position of the black piece
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];

            // printing the move
            move_print(capture, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], action_data.pos_X[0]);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[2], action_data.pos_Y[2]);

            flags.whose_turn = black;
            delete_action_data();
            
            // change the flags
            flags.moveFound = true;
          }
        }
      }
      else if (action_data.color[1] == white && action_data.action[1] == placed) // this might be a normal move / castling attempt / promotion
      { 
        // check if it is a pawn promotion
        if (action_data.piece[0] == pieces_W[pawn])
        {
          if (action_data.pos_Y[1] == 0) // if placed on the 8th rank = promotion
          {
            // placing the chosen piece on the given square
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = pieces_W[queen];

            // printing the move
            move_print(pawn_promotion, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], 0);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = black;
            delete_action_data();
            
            // change the flags
            flags.moveFound = true;
          }
          else if ((action_data.pos_X[1] != action_data.pos_X[0]) || (action_data.pos_Y[1] != action_data.pos_Y[0])) // normal move
          {
            // placing the white piece on the new square
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];
            
            // printing the move
            move_print(normal, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], 0);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = black;
            delete_action_data();
            
            // change the flags
            flags.moveFound = true;
          }
        }
        // check if it is a king move
        else if (action_data.piece[0] == pieces_W[king])
        { 
          // if castling available and if king placed on the long-castle destination
          if (!flags.w_king_moved && action_data.pos_X[1] == 2 && action_data.pos_Y[1] == 7)
          {
            if (action_data.piece[2] == pieces_W[rook]) // if next a rook is lifted
            {
              if (action_data.action[3] == placed && action_data.pos_X[3] == 3 && action_data.pos_Y[3] == 7) // wait for the rook placement
              {
                // placing the white king on the new square
                temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];

                // placing the white rook on the new square
                temp_pieces_pos[action_data.pos_Y[3]][action_data.pos_X[3]] = action_data.piece[2];
                  
                // printing the move
                move_print(long_castle, 0, 0, 0, 0);

                // saving the positions of the two squares between which the move occurred
                squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

                flags.whose_turn = black;
                delete_action_data();
                  
                // change the flags
                flags.moveFound = true;
                flags.w_king_moved = true;
              }
            }
          }
          // if castling available and if king placed on the short-castle destination
          else if (!flags.w_king_moved && action_data.pos_X[1] == 6 && action_data.pos_Y[1] == 7)
          {
            if (action_data.piece[2] == pieces_W[rook]) // if next a rook is lifted
            {
              if (action_data.action[3] == placed && action_data.pos_X[3] == 5 && action_data.pos_Y[3] == 7) // wait for the rook placement
              {
                // placing the white king on the new square
                temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];

                // placing the white rook on the new square
                temp_pieces_pos[action_data.pos_Y[3]][action_data.pos_X[3]] = action_data.piece[2];
                  
                // printing the move
                move_print(short_castle, 0, 0, 0, 0);

                // saving the positions of the two squares between which the move occurred
                squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

                flags.whose_turn = black;
                delete_action_data();
                  
                // change the flags
                flags.moveFound = true;
                flags.w_king_moved = true;
              }
            }
          }
          else if ((action_data.pos_X[1] != action_data.pos_X[0]) || (action_data.pos_Y[1] != action_data.pos_Y[0])) // normal king move
          {
            // placing the white piece on the new square
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];

            // printing the move
            move_print(normal, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], 0);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = black;
            delete_action_data();
            
            // change the flags
            flags.moveFound = true;
            flags.w_king_moved = true;
          }
        }
        else if ((action_data.pos_X[1] != action_data.pos_X[0]) || (action_data.pos_Y[1] != action_data.pos_Y[0])) // a normal move occurred
        { 
          // placing the white piece on the new square
          temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];
          
          // printing the move
          move_print(normal, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], 0);

          // saving the positions of the two squares between which the move occurred
          squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

          flags.whose_turn = black;
          delete_action_data();
          
          // change the flags
          flags.moveFound = true;
        }
      }
    }
  }
  else if (flags.whose_turn == black)
  {
    if (action_data.color[0] == white && action_data.action[0] == lifted) // initial action has to be 'lifted' | in this case if a white piece is lifted first on black turn, it has to be a capture
    {
      if (action_data.color[1] == black && action_data.action[1] == lifted)
      {
        if (action_data.action[2] == placed && action_data.pos_X[2] == action_data.pos_X[0] && action_data.pos_Y[2] == action_data.pos_Y[0]) // wait for placement and correct position (the capturing piece needs to land on the correct square)
        { 
          // check if it is a pawn promotion with capture
          if (action_data.piece[1] == pieces_B[pawn] && action_data.pos_Y[2] == 7) // if placed on the 1st rank = promotion
          { 
            // placing the chosen piece on the given square
            temp_pieces_pos[action_data.pos_Y[0]][action_data.pos_X[0]] = pieces_B[queen];

            // printing the move
            move_print(pawn_promotion_capture, action_data.piece[1], action_data.pos_Y[0], action_data.pos_X[0], action_data.pos_X[1]);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = white;
            delete_action_data();
              
            // change the flags
            flags.moveFound = true;
          }
          else 
          {
            // placing the black piece on the position of the white piece
            temp_pieces_pos[action_data.pos_Y[0]][action_data.pos_X[0]] = action_data.piece[1];

            // printing the move
            move_print(capture, action_data.piece[1], action_data.pos_Y[0], action_data.pos_X[0], action_data.pos_X[1]);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[2], action_data.pos_Y[2], action_data.pos_X[0], action_data.pos_Y[0]);

            flags.whose_turn = white;
            delete_action_data();
            
            // change the flags
            flags.moveFound = true;
          }
        }
      }
    }
    else if (action_data.color[0] == black && action_data.action[0] == lifted) // if first black is lifted, then we can check other possibilities
    {
      if (action_data.color[1] == white && action_data.action[1] == lifted) // it has to be a capture
      {
        if (action_data.action[2] == placed && action_data.pos_X[2] == action_data.pos_X[1] && action_data.pos_Y[2] == action_data.pos_Y[1]) // wait for placement and correct position (the capturing piece needs to land on the correct square)
        {
          // check if it is a pawn promotion with capture
          if (action_data.piece[0] == pieces_B[pawn] && action_data.pos_Y[2] == 7) // if placed on the 1st rank = promotion
          { 
            // placing the chosen piece on the given square
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = pieces_B[queen];

            // printing the move
            move_print(pawn_promotion_capture, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], action_data.pos_X[0]);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = white;
            delete_action_data();
              
            // change the flags
            flags.moveFound = true;
          }
          else 
          {
            // placing the black piece on the position of the white piece
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];

            // printing the move
            move_print(capture, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], action_data.pos_X[0]);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[2], action_data.pos_Y[2], action_data.pos_X[0], action_data.pos_Y[0]);

            flags.whose_turn = white;
            delete_action_data();
            
            // change the flags
            flags.moveFound = true;
          }
        }
      }
      else if (action_data.color[1] == black && action_data.action[1] == placed) // this might be a normal move / castling attempt / promotion
      { 
        // check if it is a pawn promotion
        if (action_data.piece[0] == pieces_B[pawn])
        {
          if (action_data.pos_Y[1] == 0) // if placed on the 1st rank = promotion
          {
            // placing the chosen piece on the given square
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = pieces_B[queen];

            // printing the move
            move_print(pawn_promotion, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], 0);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = white;
            delete_action_data();
            
            // change the flags
            flags.moveFound = true;
          }
          else if ((action_data.pos_X[1] != action_data.pos_X[0]) || (action_data.pos_Y[1] != action_data.pos_Y[0])) // normal move
          {
            // placing the black piece on the new square
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];
            
            // printing the move
            move_print(normal, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], 0);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = white;
            delete_action_data();
            
            // change the flags
            flags.moveFound = true;
          }
        }
        // check if it is a king move
        else if (action_data.piece[0] == pieces_B[king])
        { 
          // if castling available and if king placed on the long-castle destination
          if (!flags.b_king_moved && action_data.pos_X[1] == 2 && action_data.pos_Y[1] == 0)
          {
            if (action_data.piece[2] == pieces_B[rook]) // if next a rook is lifted
            {
              if (action_data.action[3] == placed && action_data.pos_X[3] == 3 && action_data.pos_Y[3] == 0) // wait for the rook placement
              {
                // placing the black king on the new square
                temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];

                // placing the black rook on the new square
                temp_pieces_pos[action_data.pos_Y[3]][action_data.pos_X[3]] = action_data.piece[2];
                  
                // printing the move
                move_print(long_castle, 0, 0, 0, 0);

                // saving the positions of the two squares between which the move occurred
                squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

                flags.whose_turn = white;
                delete_action_data();
                  
                // change the flags
                flags.moveFound = true;
                flags.b_king_moved = true;
              }
            }
          }
          // if castling available and if king placed on the short-castle destination
          else if (!flags.b_king_moved && action_data.pos_X[1] == 6 && action_data.pos_Y[1] == 0)
          {
            if (action_data.piece[2] == pieces_B[rook]) // if next a rook is lifted
            {
              if (action_data.action[3] == placed && action_data.pos_X[3] == 5 && action_data.pos_Y[3] == 0) // wait for the rook placement
              {
                // placing the black king on the new square
                temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];

                // placing the black rook on the new square
                temp_pieces_pos[action_data.pos_Y[3]][action_data.pos_X[3]] = action_data.piece[2];
                  
                // printing the move
                move_print(short_castle, 0, 0, 0, 0);

                // saving the positions of the two squares between which the move occurred
                squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

                flags.whose_turn = white;
                delete_action_data();
                  
                // change the flags
                flags.moveFound = true;
                flags.b_king_moved = true;
              }
            }
          }
          else if ((action_data.pos_X[1] != action_data.pos_X[0]) || (action_data.pos_Y[1] != action_data.pos_Y[0])) // normal king move
          {
            // placing the white piece on the new square
            temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];

            // printing the move
            move_print(normal, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], 0);

            // saving the positions of the two squares between which the move occurred
            squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

            flags.whose_turn = white;
            delete_action_data();

            // change the flags
            flags.moveFound = true;
            flags.b_king_moved = true;
          }
        }
        else if ((action_data.pos_X[1] != action_data.pos_X[0]) || (action_data.pos_Y[1] != action_data.pos_Y[0])) // a normal move occurred
        { 
          // placing the white piece on the new square
          temp_pieces_pos[action_data.pos_Y[1]][action_data.pos_X[1]] = action_data.piece[0];

          // printing the move
          move_print(normal, action_data.piece[0], action_data.pos_Y[1], action_data.pos_X[1], 0);

          // saving the positions of the two squares between which the move occurred
          squares_pos_str = save_squares_pos(action_data.pos_X[0], action_data.pos_Y[0], action_data.pos_X[1], action_data.pos_Y[1]);

          flags.whose_turn = white;
          delete_action_data();
          
          // change the flags
          flags.moveFound = true;
        }
      }
    }
  }
}

void move_print(uint8_t whichMove, char whichPiece, uint8_t y, uint8_t x, uint8_t prev_x)
{ 
  // necessary conversion for the correct piece letter
  int temp_whichPiece;
  if (color_check(whichPiece)) // if white
  {
    temp_whichPiece = whichPiece - 48;
  }
  else temp_whichPiece = whichPiece - 96;

  move_str = "";
  if (flags.whose_turn == white)
  { 
    move_iter++;
    if (whichPiece == pieces_W[pawn] && whichMove == pawn_promotion) // pawn promotion
    {
      move_str += move_iter;
      move_str += ". ";
      move_str += field_letters[x];
      move_str += field_numbers[y];
      move_str += move_letters[whichMove];
      move_str += "Q";
      move_str += " ";

      // Serial << move_str;
    }
    else if (whichPiece == pieces_W[pawn] && whichMove == pawn_promotion_capture) // pawn promotion capture
    { 
      move_str += move_iter;
      move_str += ". ";
      move_str += field_letters[prev_x];
      move_str += move_letters[1];
      move_str += field_letters[x];
      move_str += field_numbers[y];
      move_str += move_letters[4];
      move_str += "Q";
      move_str += " ";

      // Serial << move_str;
    }
    else if (whichPiece == pieces_W[pawn] && whichMove == capture) // pawn capture
    {
      move_str += move_iter;
      move_str += ". ";
      move_str += field_letters[prev_x];
      move_str += move_letters[whichMove];
      move_str += field_letters[x];
      move_str += field_numbers[y];
      move_str += " ";

      // Serial << move_str;
    }
    else if (whichMove == short_castle || whichMove == long_castle) // castling
    {
      move_str += move_iter;
      move_str += ". ";
      move_str += move_letters[whichMove];
      move_str += " ";

      // Serial << move_str;
    }
    else 
    {
      move_str += move_iter;
      move_str += ". ";
      move_str += piece_letters[temp_whichPiece-1];
      move_str += move_letters[whichMove];
      move_str += field_letters[x];
      move_str += field_numbers[y];
      move_str += " ";

      // Serial << move_str;
    }
  }
  else if (flags.whose_turn == black)
  { 
    if (whichPiece == pieces_B[pawn] && whichMove == pawn_promotion) // pawn promotion
    {
      move_str += field_letters[x];
      move_str += field_numbers[y];
      move_str += move_letters[whichMove];
      move_str += "Q";
      move_str += " ";

      // Serial << move_str;
    }
    else if (whichPiece == pieces_B[pawn] && whichMove == pawn_promotion_capture) // pawn promotion capture
    { 
      move_str += field_letters[prev_x];
      move_str += move_letters[1];
      move_str += field_letters[x];
      move_str += field_numbers[y];
      move_str += move_letters[4];
      move_str += "Q";
      move_str += " ";

      // Serial << move_str;
    }
    else if (whichPiece == pieces_B[pawn] && whichMove == capture) // pawn capture
    {
      move_str += field_letters[prev_x];
      move_str += move_letters[whichMove];
      move_str += field_letters[x];
      move_str += field_numbers[y];
      move_str += ' ';

      // Serial << move_str;
    }
    else if (whichMove == short_castle || whichMove == long_castle) // castling
    {
      move_str += move_letters[whichMove];
      move_str += ' ';

      // Serial << move_str;
    }
    else 
    {
      move_str += piece_letters[temp_whichPiece-1];
      move_str += move_letters[whichMove];
      move_str += field_letters[x];
      move_str += field_numbers[y];
      move_str += ' ';

      // Serial << move_str;
    }
  }
}
