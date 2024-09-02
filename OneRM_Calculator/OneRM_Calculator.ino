#include <Key.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// Bad practice use all caps for define statements
//number of rows on the keypad
#define numRows 4
//number of columns on the keypad
#define numCols 4

// Capitals for enum consts
enum mode
{
  Onerm,
  Reps,
  Weight
};

char keymap[numRows][numCols]=
{
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

byte rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3
byte colPins[numCols]= {5,4,3,2}; //Columns 0 to 3

Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

LiquidCrystal_I2C asdf(0x27, 16, 2);
int CursorCol = 2;
int CursorRow = 0;

// Assumed rpe10

void displayMessage(const char *message)
{
  asdf.setCursor(2, 0);
  asdf.print("                                                                  ");
  asdf.setCursor(2, 0);
  asdf.print(message);
  asdf.setCursor(2, 1);
  CursorCol = 2;
  CursorRow = 1;
}

mode& operator++(mode& m, int i) {
  switch (m) {
    case Onerm:
      m = Reps;
      break;
    case Reps:
      m = Weight;
      break;
    case Weight:
      // Handle wrap-around or error
      m = Onerm;
      break;
  }
  return m;
}

mode operator+(mode m, int i) {
  switch (m) {
    case Onerm:
      m = Reps;
      break;
    case Reps:
      m = Weight;
      break;
    case Weight:
      // Handle wrap-around or error
      m = Onerm;
      break;
  }
  return m;
}

const char* modeToString(enum mode inputmode)
{
  switch (inputmode)
  {
  case Onerm:
    return "1RM";
    break;
  case Reps:
    return "Reps";
    break;
  case Weight:
    return "Weight";
    break;
  }
}
struct datatable
{
  private:
  char input[100];
  float floatinput;

  // I really should've put these in an array
  
  float onerm;
  float reps;
  float weight;
  // Variable, not #defined
  mode MODE;

  float *ModePtr;

  // currently selected input
  mode Stage;

  //ptr to var for current input
  float *StagePtr;
  
  bool inputOn = false;

  // true means 'A' or enter cannot be pressed, false means it can
  bool toggleEnter = false;

  public:
  
  // Clears all values, called on init
  void clear()
  {
    strcpy(input, "");
    floatinput = 0;
    onerm = 0;
    reps = 0;
    weight = 0;
    // Clear also cycles mode since I'm out of buttons
    MODE++;
    Stage = MODE;
    Stage++;
    setStagePtr();

    switch (MODE)
    {
    case Onerm:
      ModePtr = &onerm;
      break;
    case Reps:
      ModePtr = &reps;
      break;
    case Weight:
      ModePtr = &weight;
      break;
    default:
      // If you try to put a mode that's not in the enum
      exit(1);
      break;
    }

    inputOn = false;
    toggleEnter = false;

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Mode: %s. Press A.", modeToString(MODE));
    displayMessage(buffer);
  }

  void next()
  {
    if (toggleEnter)
    {
      return;
    }
    if (inputOn)
    {
      if (!checkInput())
    {
      displayMessage("Invalid input. Press A.");
      inputOn = false;
      return;
    }
      if (Stage + 1 == MODE)
      {
        set();
        showFinalScreen();
      }
      else
      {
        set();
        Stage++;
        setStagePtr();
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Enter %s: ", modeToString(Stage));
        displayMessage(buffer);
      }
    }
    else
    {
      char buffer[100];
      snprintf(buffer, sizeof(buffer), "Enter %s: ", modeToString(Stage));
      displayMessage(buffer);
      inputOn = true;
    }
    
  }

  void showFinalScreen()
  {
    calculateMissing();
    inputOn = false;
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%s is %d. #", modeToString(MODE), (int)(*ModePtr + 0.5f));
    displayMessage(buffer);
    toggleEnter = true;
  }

  bool checkInput()
  {
    if (strcmp(input, "0") != 0 && atof(input) == 0)
    {
      return false;
    }
    return true;
  }

// Sets and resets a value based on stage
  void set()
  {
    floatinput = atof(input);
    *StagePtr = floatinput;
    floatinput = 0;
    strcpy(input, "");
  }

// Calculate missing value with Epley formula (mainly since it's the simplest)
  void calculateMissing()
  {
    switch (MODE)
    {
      case Onerm:
        onerm = weight * (1 + reps / 30);

        break;
      case Reps:
        reps = ((onerm / weight) - 1)*30;
        break;
      case Weight:
        weight = onerm / (1 + reps / 30);
        break;
    }
  }

  // Getter and setter for 'input'
const char* getInput() {
    return input;
}

void setInput(const char* newInput) {
    strncpy(input, newInput, sizeof(input)-1);
    input[strlen(input)] = '\0';
}

// Getter and setter for 'floatinput'
float getFloatInput() {
    return floatinput;
}

void setFloatInput(float newFloatInput) {
    floatinput = newFloatInput;
}

bool getInputOn() {
    return inputOn;
}

// Getter and setter for 'onerm'
float getOnerm() {
    return onerm;
}

void setOnerm(float newOnerm) {
    onerm = newOnerm;
}

// Getter and setter for 'reps'
float getReps() {
    return reps;
}

void setReps(float newReps) {
    reps = newReps;
}

// Getter and setter for 'weight'
float getWeight() {
    return weight;
}

void setWeight(float newWeight) {
    weight = newWeight;
};

void setMode(mode newMode) {
    MODE = newMode;
}

mode getMode() {
    return MODE;
}

private:
void setStagePtr()
{
  switch (Stage)
  {
    case Onerm:
      StagePtr = &onerm;
      break;
    case Reps:
      StagePtr = &reps;
      break;
    case Weight:
      StagePtr = &weight;
      break;
    default:
    exit(1);
    break;
  }
}
};
datatable Datatable;

void setup()
{
  asdf.init();
  asdf.clear();
  asdf.cursor_on();

  asdf.backlight();
  asdf.setCursor(CursorCol, CursorRow);

  Datatable.setMode(Weight);
  Datatable.clear();
  
}
void loop()
{
  char keypressed = myKeypad.getKey();
  if (keypressed == NO_KEY)
  {
    return;
  }
  
  switch (keypressed)
  {
  // backspace
  case '*':
    if (strcmp(Datatable.getInput(), "") == 0 || !Datatable.getInputOn())
    {
      break;
    }
    if (CursorCol == 2 && CursorRow > 0)
    {
      CursorCol = 13;
      CursorRow--;
    }
    else if (CursorCol > 2)
    {
      CursorCol--;
    }
    asdf.setCursor(CursorCol, CursorRow);
    asdf.print(' ');
    asdf.setCursor(CursorCol, CursorRow);
    char tempInput[100];
    strncpy(tempInput, Datatable.getInput(), sizeof(tempInput) - 1);
    tempInput[sizeof(tempInput) - 1] = '\0';
    tempInput[strlen(tempInput) - 1] = '\0'; // Remove last character
    Datatable.setInput(tempInput);
    break;
  // clear
  case '#':
    Datatable.clear();
    break;

  // enter
  case 'A':
    Datatable.next();
    break;
  
  default:
  if (!Datatable.getInputOn())
  {
    break;
  }
    
    if (CursorCol < 14)
    {
      CursorCol++;
      asdf.print(keypressed);
    }
    else if (CursorRow < 1)
    {
      CursorCol = 2;
      CursorRow++;
      asdf.setCursor(CursorCol, CursorRow);
      asdf.print(keypressed);
      CursorCol++;
    }
    char newInput[100];
    snprintf(newInput, sizeof(newInput), "%s%c", Datatable.getInput(), keypressed);
    Datatable.setInput(newInput);
    break;
  }
  }
