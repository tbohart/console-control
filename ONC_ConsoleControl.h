/*
 * Upgraded serial control
 * 
 * intuitive and easily expandable command system,
 * storage of variables in EEPROM, and more...
 *
 * 
 * By Tristan Bohart
 *
 * Created January 20th, 2022 while working for ONC
 *  
 * Last updated by Tristan on April 13th, 2022
 * 
 * 
 * Uses 5 (3 if no eeprom) console commands
 * Uses 0 eeprom variables
*/


#ifndef CONSOLECONTROL_h
#define CONSOLECONTROL_h


  

  // example function to be ran by command:
  // void printControls(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]) {}

  // example command registration:
  // registerCommand({"@help","prints available commands or specific command data","@help"DELIMITER"(<command>)",1,0,&printCommandHelp});

  //registration format: name (string), description (string), use (string), max parameters (0-255: number of parameters to take in), 
  //min parameters (0-255: number of parameters required to be present), &functionName (function pointer)


  // example eeprom variable registration
  // registerVariable({"relayState",0,MLR_STATE_ADDR});

  //format: name (string), type (0 = byte; 1 = double), address (an integer location in EEPROM)




/* -- -- -- -- -- -- Libraries -- -- -- -- -- -- */
  #ifndef NO_EEPROM
  #include <ONC_EEPROM.h> // standard eeprom interface, can be configured for different EEPROMS using definitions
  #endif
//




/* -- -- -- -- -- Configuration -- -- -- -- -- -- */

  // -- default console control definitions -- //
  #ifndef SERIAL_INTERFACE
  #define SERIAL_INTERFACE Serial1 // serial interface to send/recieve from (Serial,Serial1,etc.)
  #endif

  // #define NO_EEPROM (put this in your code to disable eeprom)

  //#define USE_DYNAMIC_VAR_ADDRESSES // enable this to automatically pack variables in as compactly as possible. However, the locations may change if you swap libraries/library orders, and most libraries won't know where to find their stuff.

  #ifndef MAX_COMMANDS 
  #define MAX_COMMANDS 10 // maximum commands registered at one time
  #endif

  #ifndef MAX_VARIABLES 
  #define MAX_VARIABLES 10 // maximum variables registered at one time
  #endif

  #ifndef MAX_PARAMETERS 
  #define MAX_PARAMETERS 6 // maximum parameters per command
  #endif

  #ifndef MAX_PARAMETER_LENGTH 
  #define MAX_PARAMETER_LENGTH 15 // maximum characters per parameter
  #endif

  #ifndef INPUT_BUFFER_SIZE 
  #define INPUT_BUFFER_SIZE 50 // maximum characters per command
  #endif

  #ifndef COMMAND_HISTORY_LENGTH
  #define COMMAND_HISTORY_LENGTH 5 // how many previous commands to remember (minimum zero)
  #endif

  #ifndef ENTER 
  #define ENTER '\r' // character detected when enter is pressed
  #endif

  #ifndef BACKSPACE 
  #define BACKSPACE '\b' // character detected when backspace is pressed on linux
  #endif

  #ifndef ESCAPE 
  #define ESCAPE 27 // character detected when escape is pressed
  #endif

  #ifndef DELETE
  #define DELETE 127 // character detected when backspace is pressed in putty
  #endif

  #ifndef LINE_FEED
  #define LINE_FEED 10 // character sometimes sent in addition to carriage return
  #endif

  #ifndef ENTRY_PREFIX 
  #define ENTRY_PREFIX '>' // thing to show up before command to signify user can input a command
  #endif

  #ifndef DELIMITER 
  #define DELIMITER "," // character by which to split the input strings
  #endif 

  #ifndef ESC_CODE_MS
  #define ESC_CODE_MS 2 // time to wait to see if an escape sequence is sent after an escape character is sent
  #endif

  #ifndef MAX_ESC_CODE_LENGTH
  #define MAX_ESC_CODE_LENGTH 2 // maximum characters in an escape sequence to be parsed (any number can be ignored)
  #endif
  
  #ifndef CONSOLE_CONTROL_TIMEOUT_MS
  #define CONSOLE_CONTROL_TIMEOUT_MS 30000
  #endif




  /* -- -- -- -- -- -- Data Types -- -- -- -- -- -- */

  // holds the information needed for a serial command
  struct Command {
    char* name;
    char* description;
    char* use;
    uint8_t maxParameters;
    uint8_t minParameters;
    void (*function)(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]);
  };


  #ifndef NO_EEPROM
  // allows storing/reading from eeprom based on variable name
  struct eepromVariable {
    char* name;
    uint8_t type; // 0 = byte; 1 = double
    int address;
    bool modified;
  };
  #endif


  // -- console control variables --/

  Command commands[MAX_COMMANDS]; // list to hold commands
  
  int commandNum = 0; // command counter



  #ifndef NO_EEPROM
  eepromVariable variables[MAX_VARIABLES]; // place to declare eeprom variables

  int variableNum = 0; // how many variables have been registered
  int nextAddress = 0; // if using automatic variable address assignment, the next available address



  static const char* typeNames[] = {"byte","double"}; //names of each variable type
  static const uint8_t typeSizes[] = {sizeof(uint8_t),sizeof(double)}; // sizes of variable types
  static const int typeNum = sizeof(typeNames)/sizeof(char*); // how many types there are

  #endif


  char historyBuffers[COMMAND_HISTORY_LENGTH+1][INPUT_BUFFER_SIZE]; // input buffer memory (for recalling past commands)
  int inputEnds[COMMAND_HISTORY_LENGTH+1]; // how many characters in each buffer actually have been entered

  char inputBuffer[INPUT_BUFFER_SIZE]; // the main input buffer
//








/* -- -- -- -- -- -- Functions -- -- -- -- -- -- */




// registers a new command which can then be run via serial
// takes a Command object which stores all the necessary data
// the object can be represented by an initializer list
void registerCommand(Command command) {
  if (commandNum < MAX_COMMANDS) {
    commands[commandNum] = command;
    commandNum++;
  } else {
    SERIAL_INTERFACE.println("Out of space for commands. Change MAX_COMMANDS or register less commands.");
  }
}


#ifndef NO_EEPROM
// registers a new variable for use with EEPROM
// takes a eepromVariable object which stores all the necessary data
// the address field will be ignored (it is set by this function)
// the object can be represented by an initializer list
void registerVariable(eepromVariable variable) {
  
  if (variableNum < MAX_VARIABLES) {

    #ifdef USE_DYNAMIC_VAR_ADDRESSES
    variable.address = nextAddress;
    nextAddress += typeSizes[variable.type];
    #endif

    variable.modified = false;

    variables[variableNum] = variable;
    variableNum++;

  } else {

    SERIAL_INTERFACE.println("Out of space for variables. Change MAX_VARIABLES or register less variables.");
  }
}
#endif


#ifndef NO_EEPROM
void getVariable(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]);
void putVariable(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]);
void printVariables(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]);
#endif
void printCommandHelp(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]);
void printControls(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]);
// format: void functionName(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]);


// initialize the default commands (allows selective inclusion of default commands using macros)
void registerDefaultCommands() {
  #ifndef NO_EEPROM
  registerCommand({"@get","reads a variable from EEPROM","@get"DELIMITER"[<variable>]",1,1,&getVariable});
  registerCommand({"@put","stores a variable in EEPROM","@put"DELIMITER"[<variable>]"DELIMITER"[<value>]",2,2,&putVariable});
  registerCommand({"@variables","prints all eeprom variables and their types","@variables",0,0,&printVariables});
  #endif

  registerCommand({"@help","prints available commands or specific command data","@help"DELIMITER"(<command>)",1,0,&printCommandHelp});
  registerCommand({"@controls","Prints available console controls","@controls",0,0,&printControls});

}


// takes the characters up to the next space or ending character in an input string
// made to be used in runSerialCommands()
// inputNum is the token index to find (0=first token, 1=second, etc.)
// if inputNum is lower than 0, the function will not do anything.
// stores the characters in 'token' if they exist, otherwise stores NULL in position 0
void getToken(char (&token)[MAX_PARAMETER_LENGTH], char (&input)[INPUT_BUFFER_SIZE], int tokenNum) {
  
  if (tokenNum < 0) return;

  if (input[0] == '\0') {
    token[0] = '\0';
    return;
  }

  //SERIAL_INTERFACE.println("got past empty check in getToken");
  //SERIAL_INTERFACE.println((int)input[0]);

  int i = 1;
  int start = 0;

  for (;i < INPUT_BUFFER_SIZE-1 && input[i] != '\0'; i++) {
    
    if ((input[i] == DELIMITER[0]) && (input [i-1] != DELIMITER[0])) {
    
      if (tokenNum == 0) break;
      else {
        tokenNum = tokenNum-1;
      }
    } else if ((input[i] != DELIMITER[0]) && (input [i-1] == DELIMITER[0])) {

      start = i;
    }
  }

  //SERIAL_INTERFACE.print(start);
  //SERIAL_INTERFACE.print('-');
  //SERIAL_INTERFACE.println(i);

  //SERIAL_INTERFACE.println(tokenNum);
  if (tokenNum > 0) { 
    token[0] = '\0';
    return;
  }

  //if (input[i] == '\0') i = i-1;

  
  int j = 0;
  
  for (; j < i-start && j < MAX_PARAMETER_LENGTH; j++) {
    
    token[j] = input[j+start];
  }

  token[j] = '\0';
  //SERIAL_INTERFACE.println(j);
  //SERIAL_INTERFACE.println(token);
}


// returns the index of the given command in the command list if it is valid, otherwise returns an index one past the end of the list
int findAndCheckCommandIndex(char (&command)[MAX_PARAMETER_LENGTH]) {
  int commandIndex = 0;
  for(; commandIndex < commandNum && strcmp(command,commands[commandIndex].name) != 0; commandIndex++);
  
  if (commandIndex == commandNum) {
    SERIAL_INTERFACE.print('\'');
    SERIAL_INTERFACE.print(command);
    SERIAL_INTERFACE.println("' is not a command. You can use the '@help' command to list all possible commands.");
  }

  return commandIndex;

}


#ifndef NO_EEPROM
// returns the index of the given variable in the variable list if it is valid, otherwise returns an index one past the end of the list
int findAndCheckVariableIndex(char (&variable)[MAX_PARAMETER_LENGTH]) {
  int variableIndex = 0;
  for(; variableIndex < variableNum && strcmp(variable,variables[variableIndex].name) != 0; variableIndex++);
  
  if (variableIndex == variableNum) {
    SERIAL_INTERFACE.print('\'');
    SERIAL_INTERFACE.print(variable);
    SERIAL_INTERFACE.println("' is not a variable. You can use the '@variables' command to list all variables.");
  }

  return variableIndex;

}
#endif

// copies the contents of one char buffer to another char buffer
void bufferCopy(char (&buffer1)[INPUT_BUFFER_SIZE], char (&buffer2)[INPUT_BUFFER_SIZE]) {

  for (int i = 0; i < INPUT_BUFFER_SIZE; i++) buffer1[i] = buffer2[i];
}


// shifts the buffer history up and copies the last used buffer to the new entry if necessary
// requires the index in the buffer list of the last used command
void incrementBufferHistory(int historyIndex) {

  bufferCopy(historyBuffers[0],inputBuffer);
  inputEnds[0] = inputEnds[historyIndex];

  for (int i = COMMAND_HISTORY_LENGTH; i > 0; i--) {
    bufferCopy(historyBuffers[i],historyBuffers[i-1]);
    inputEnds[i] = inputEnds[i-1];
  }


  /*if (historyIndex != 1) {

    if (historyIndex > 0) {
      bufferCopy(historyBuffers[0],historyBuffers[historyIndex]);
      inputEnds[0] = inputEnds[historyIndex];
    }
    

    for (int i = COMMAND_HISTORY_LENGTH; i > 0; i--) {
      bufferCopy(historyBuffers[i],historyBuffers[i-1]);
      inputEnds[i] = inputEnds[i-1];
    }
  }*/
}


// prints the buffer at the given history location to the current line after clearing it
void printHistoryBuffer(int historyIndex, int &inputIndex) {

  SERIAL_INTERFACE.print("\u001b[2K\r");
  
  SERIAL_INTERFACE.print(ENTRY_PREFIX);

  for (int i = 0; i < inputEnds[historyIndex]; i++) {
    SERIAL_INTERFACE.print(historyBuffers[historyIndex][i]);
  }

  inputIndex = inputEnds[historyIndex];

  //inputIndex = min(inputEnds[historyIndex],inputIndex);

  //SERIAL_INTERFACE.print("\u001b[");
  //SERIAL_INTERFACE.print(inputEnds[historyIndex] - inputIndex);
  //SERIAL_INTERFACE.print('D');
}


// filters unwanted escape sequences from echoing, and implements escape sequences which are wanted
// returns whether an escape sequence was detected
// takes the current input position in the input buffer
bool catchEscapeSequence(int &historyIndex, int &inputIndex, bool allowHistory = false) {


  char escapeSequence[MAX_ESC_CODE_LENGTH];
  //int escapeIndex = 0;

  // read in sequence to parse
  for (int escapeIndex = 0; escapeIndex < MAX_ESC_CODE_LENGTH && SERIAL_INTERFACE.available(); escapeIndex++) {
    escapeSequence[escapeIndex] = SERIAL_INTERFACE.read();
    delay(ESC_CODE_MS);
  }

  // get rid of any remaining characters (in case of overly long sequence)
  while (SERIAL_INTERFACE.available()) {
    SERIAL_INTERFACE.read();
    delay(ESC_CODE_MS);
  }


  // parse escape sequences
  if (escapeSequence[0] == '[') {

    if (escapeSequence[1] == 'C' && inputIndex != inputEnds[historyIndex]) { // "[C" (move cursor right)
      SERIAL_INTERFACE.print("\u001b[C");

      inputIndex++;
      inputEnds[historyIndex] = max(inputEnds[historyIndex],inputIndex);
      
    }

    else if (escapeSequence[1] == 'D' && inputIndex > 0) { // "[D" (move cursor left)
      SERIAL_INTERFACE.print("\u001b[D");
      inputIndex--;
    }

    else if (escapeSequence[1] == 'A' && allowHistory && historyIndex < COMMAND_HISTORY_LENGTH) { // "[A" (up arrow, recall further back in history)
      
      if(historyIndex == 0) {
        bufferCopy(historyBuffers[0],inputBuffer);
      }
 
      historyIndex++;
      
      printHistoryBuffer(historyIndex,inputIndex);
      bufferCopy(inputBuffer,historyBuffers[historyIndex]);
    }

    else if (escapeSequence[1] == 'B' && allowHistory && historyIndex > 0) { // "[B" (down arrow, recall less far back in history)
      
      historyIndex--;
      printHistoryBuffer(historyIndex,inputIndex);
      bufferCopy(inputBuffer,historyBuffers[historyIndex]);
    }
  }
}




// allows the user to enter text
bool getSerialInput(int &historyIndex, bool allowHistory = false, bool timeout = true) {
  
  char inputChar;
  int inputIndex = 0;
  inputEnds[0] = 0;
  //inputBuffer[0] = '\0';

  bool finished = false;
  bool exit = false;

  int timeoutCounter = 0;
  
  //SERIAL_INTERFACE.println(historyIndex);

  // get any serial input into a local string where we can search it without clearing it
  while (inputEnds[historyIndex] < INPUT_BUFFER_SIZE-1 && !finished) {
  
    if (!SERIAL_INTERFACE.available()) {
      
      if (timeoutCounter < CONSOLE_CONTROL_TIMEOUT_MS) {
        
        timeoutCounter++; // note that a ms has passed without action
        delay(1);
        
        continue; // keep looping until characters found
      
      } else {

        exit = true;
        break;
      }

    }

    timeoutCounter = 0; // clear timeout counter if data found



    inputChar = SERIAL_INTERFACE.read();
    



    switch (inputChar) {
    case ESCAPE:
      
      delay(ESC_CODE_MS);

      // parse and remove any escape sequences before they enter buffer
      if (SERIAL_INTERFACE.available()) {

        catchEscapeSequence(historyIndex,inputIndex,allowHistory);
        break;
        
      // if escape character entered alone, exit console input mode
      } else {

        exit = true;
        //SERIAL_INTERFACE.println();
      }

    case ENTER:
      finished = true;
    
    case LINE_FEED:
      break;
    
    case BACKSPACE: // backspace on linux

    case DELETE: // backspace in putty

      //SERIAL_INTERFACE.println("huh?");

      if (inputIndex > 0) {
        if (inputIndex == inputEnds[historyIndex]) {
          SERIAL_INTERFACE.print("\b \b");

        } else {
          SERIAL_INTERFACE.print('\b');

          for (int i = inputIndex; i < inputEnds[historyIndex]; i++) {
            inputBuffer[i-1] = inputBuffer[i];
            SERIAL_INTERFACE.print(inputBuffer[i]);
          }
          //SERIAL_INTERFACE.print("HELLO");
          SERIAL_INTERFACE.print(" \u001b[");
          SERIAL_INTERFACE.print(inputEnds[historyIndex] - inputIndex + 1);
          SERIAL_INTERFACE.print('D');

        }

        inputEnds[historyIndex]--;
        inputIndex--;
      }
      break;

    default: // anything else
      SERIAL_INTERFACE.print(inputChar);
      inputEnds[historyIndex]++;
      
      if (inputIndex < inputEnds[historyIndex] - 1) {
        //SERIAL_INTERFACE.println(inputIndex);
        //SERIAL_INTERFACE.println("not last char");
        for (int i = inputEnds[historyIndex]; i > inputIndex; i--) inputBuffer[i] = inputBuffer[i-1];

        for (int i = inputIndex+1; i < inputEnds[historyIndex]; i++) SERIAL_INTERFACE.print(inputBuffer[i]);

        SERIAL_INTERFACE.print("\u001b[");
        SERIAL_INTERFACE.print(inputEnds[historyIndex] - inputIndex - 1);
        SERIAL_INTERFACE.print('D');
        //SERIAL_INTERFACE.println();
        //SERIAL_INTERFACE.println(inputIndex);

      }
      

      inputBuffer[inputIndex] = inputChar;
      inputIndex ++;
      

    }
  }

  inputBuffer[inputEnds[historyIndex]] = '\0';

  return exit;
}


// puts parameters from the input buffer into a parameter array
// returns true if there were enough parameters
// otherwise returns false
bool getParametersFromInput(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH], int &commandIndex) {
  int parameterIndex = 0;

  for (; parameterIndex < commands[commandIndex].maxParameters; parameterIndex++) {
    
    getToken(parameters[parameterIndex], inputBuffer, parameterIndex+1);
    

    if (parameters[parameterIndex][0] == '\0' && parameterIndex < commands[commandIndex].minParameters) {
      SERIAL_INTERFACE.println("Too few parameters!");
      SERIAL_INTERFACE.print("Correct format: ");
      SERIAL_INTERFACE.println(commands[commandIndex].use);
      //SERIAL_INTERFACE.println(commands[commandIndex].minParameters);
      
      return false;
    }
  }

  return true;
}




// starts once incoming serial data is detected
// press escape to exit
// executes commands incoming on the serial port
void runSerialCommands() {
  if (!SERIAL_INTERFACE.available()) {
    return;
  }
  bool exit = false;
  int historyIndex = 0;

  
  // keep in console mode until exited
  while (!exit) {

    SERIAL_INTERFACE.print(ENTRY_PREFIX);

    exit = getSerialInput(historyIndex,true); // have the user enter a string

    SERIAL_INTERFACE.println();

    // skip command handling if exit trigged
    if (exit) continue;


    // identify the command being executed
    char command[MAX_PARAMETER_LENGTH];
    getToken(command, inputBuffer,0);


    int commandIndex = findAndCheckCommandIndex(command);

    // get ready to execute command if valid
    if (commandIndex != commandNum) {

      char parameters[MAX_PARAMETERS][MAX_PARAMETER_LENGTH];

      
      incrementBufferHistory(historyIndex); // shift history positions

      // run command
      if (getParametersFromInput(parameters,commandIndex)) {
        commands[commandIndex].function(parameters);
      }
    } else {
      incrementBufferHistory(historyIndex); // shift history positions

    }
    

    historyIndex = 0;
  }
}


#ifndef NO_EEPROM


// checks if a variable at a given address has been changed
// if clearFlag is set, it will also set the modified flag to false afterwards
bool variableModified(int address, bool clearFlag = false) {
  int variableIndex = 0;
  for(; variableIndex < variableNum && variables[variableIndex].address != address; variableIndex++);
  
  if (variableIndex == variableNum) {
    //SERIAL_INTERFACE.println("isVariableModified() being called on nonexistant variable!");
    return false;
  }

  bool modified = variables[variableIndex].modified;

  if (clearFlag) variables[variableIndex].modified = false;

  return modified;
}


// reads a variable from eeprom memory
// uses one parameter, the variable name
void getVariable(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]) {
  //SERIAL_INTERFACE.println("get function is under construction. come back later.");

  int variableIndex = findAndCheckVariableIndex(parameters[0]);

  if (variableIndex == variableNum) return;

  SERIAL_INTERFACE.print(parameters[0]);
  SERIAL_INTERFACE.print("\u2192");

  switch (variables[variableIndex].type) {
    case 0:
      uint8_t byte;
      eepromGet(variables[variableIndex].address,byte);
      SERIAL_INTERFACE.println(byte);
      break;
    case 1:
      double decimal;
      eepromGet(variables[variableIndex].address,decimal);
      SERIAL_INTERFACE.println(decimal,10);
      break;
    default:
      SERIAL_INTERFACE.println("Invalid variable type! \n\r Check the 'variables' list definition in the code ASAP.");

  //SERIAL_INTERFACE.print("address:");
  //SERIAL_INTERFACE.println(storedAddresses[variableIndex]);
  }
}



// writes a variable to the eeprom
// uses two parameters, the variable name and the value to write
void putVariable(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]) {
  //SERIAL_INTERFACE.println("put function is under construction. come back later.");

  int variableIndex = findAndCheckVariableIndex(parameters[0]);

  if (variableIndex == variableNum) return;

  SERIAL_INTERFACE.print(parameters[0]);
  SERIAL_INTERFACE.print("\u2190");


  switch (variables[variableIndex].type) {
    case 0:
      uint8_t byte;
      byte = atoi(parameters[1]);
      eepromPut(variables[variableIndex].address,byte);
      SERIAL_INTERFACE.println(byte);
      break;
    case 1:
      double decimal;
      decimal = atof(parameters[1]);
      eepromPut(variables[variableIndex].address,decimal);
      SERIAL_INTERFACE.println(decimal,10);
      break;
    default:
      SERIAL_INTERFACE.println("Invalid variable type! \n\r Check the 'variables' list definition in the code ASAP.");
  }

  variables[variableIndex].modified = true;
}
#endif


// prints help on commands
// if given no parameters, will print a list of possible commands
// given a command as a parameter, it will tell what the command does and its parameter format
void printCommandHelp(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]) {
  //SERIAL_INTERFACE.println("help function is under construction. come back later.");

  if (parameters[0][0] == NULL) {
    SERIAL_INTERFACE.println("Available commands: ");

    for (int i = 0; i < commandNum; i++) {
      SERIAL_INTERFACE.println(commands[i].name);
    }

    SERIAL_INTERFACE.println("\nFor additional information on a given command, type '@help"DELIMITER"<command>'");
    SERIAL_INTERFACE.println("For help using the console, type '@controls'");
    SERIAL_INTERFACE.println("command usage format: [] = required, () = optional, <> = non-literal, {} = default");

  } else {
    //SERIAL_INTERFACE.println("found parameter:");
    //SERIAL_INTERFACE.println(parameters[0]);
    //SERIAL_INTERFACE.println(parameters[1]);
    int commandIndex = findAndCheckCommandIndex(parameters[0]);

    if (commandIndex != commandNum) {
      SERIAL_INTERFACE.print("Name: ");
      SERIAL_INTERFACE.println(commands[commandIndex].name);
      SERIAL_INTERFACE.print("Description: ");
      SERIAL_INTERFACE.println(commands[commandIndex].description);
      SERIAL_INTERFACE.print("Use: ");
      SERIAL_INTERFACE.println(commands[commandIndex].use);
    }
    
  }

}


#ifndef NO_EEPROM
// prints the name and type of every eeprom variable
void printVariables(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]) {

  SERIAL_INTERFACE.println("EEPROM Variables: ");

  for (int variableIndex = 0; variableIndex < variableNum; variableIndex++) {

    SERIAL_INTERFACE.print(variables[variableIndex].name);
    SERIAL_INTERFACE.print(" (");
    SERIAL_INTERFACE.print(typeNames[variables[variableIndex].type]);
    
    if (variables[variableIndex].modified) {
      SERIAL_INTERFACE.println(") - Modified ");
    } else {
      SERIAL_INTERFACE.println(')');
    }
  }
}
#endif



// prints the available controls to help users understand how to navigate/use the console
void printControls(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]) {

  SERIAL_INTERFACE.println("Console Controls:");
  SERIAL_INTERFACE.println("Press Escape to exit console mode");
  SERIAL_INTERFACE.println("Press the up or down arrows to move in the command history");
  SERIAL_INTERFACE.println("Left, right, and backspace are all supported when entering commands");
}


// end of header
#endif
