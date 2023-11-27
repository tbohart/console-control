/*
 * 
 * Demonstration of use of the ConsoleControl library for custom commands without eeprom
 *
 * By Tristan Bohart
 * Created January 31st, 2022 while working for ONC
 * 
 * Last updated February 18th, 2022
*/




/* -- -- -- -- -- -- OPTIONAL CONFIGURATION -- -- -- -- -- -- -- */
  // put definitions here to override the defaults in the library
  // the overrides must be defined before the library is included
  #define ENTRY_PREFIX "enter command here: " // text to show up before command to signify user can input a command


  // disable eeprom functionality in ConsolControl
  // this will stop the eeprom-related functions and objects from compiling
  // This avoids atof() and atoi() which results in much smaller flash memory used if the functions
  // are not used elsewhere
  #define NO_EEPROM true 
//



/* -- -- -- -- -- -- CONSOLE CONTROL LIBRARY -- -- -- -- -- -- -- */
  #include <ONC_ConsoleControl.h> // include the library which handles all the serial functionality

//








/* functions to be run by commands (must be declared before they are registered as commands) */

// declare the function which will be run by command "test1"
void test1(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]) {
  Serial1.println("Test1 = Success!");
}

// declare the function which will be run by command "test2"
void otherTest(char (&parameters)[MAX_PARAMETERS][MAX_PARAMETER_LENGTH]) {
  
  Serial1.print("Test2 = Success! First parameter: ");
  
  Serial1.print(parameters[0]);

  if (parameters[1][0] == '\0') { // if the fist character is NULL (\0), the string is empty and no parameter was passed
    Serial1.println(", no second parameter entered.");
  
  } else {
    Serial1.print(", Second Parameter: ");
    Serial1.println(parameters[1]);
    
    //double twoTimes = 2*atof(parameters[1]);

    //Serial1.print("Two times second parameter: ");
    //Serial1.println(twoTimes,10);
  }
}






// single-run initialization
void setup() {
  
  // initialize the serial we have chosen to use with this library
  Serial1.begin(9600);
  

  // add the commands which come packaged with the library
  registerDefaultCommands();


  // add some custom commands we can run via serial
  registerCommand({"@test1","testing adding commands","@test1",0,0,&test1}); // register a command
  registerCommand({"@test2","testing adding commands","@test2,[<anything>],(<#.##>)",2,1,&otherTest}); // register another command
  /* 
   * command format: name (string), description (string), usage (string), maximum parameters (0-255), minimum parameters (0-255), &functionName
   * 
   * 
   * usage format: [] = required, () = optional, <> = non-literal, {} = default, <#> = integer, <#.##> = decimal
   * examples:
   * "@get,[<variable>],[<value>]" -> two parameters, both required, both non-literal (replace the text inside <> with something else)
   * "@setRelay,({on}/off)" -> one optional parameter, with default value "on"
   * "@setDutyCycleMinutes,[<#>]" -> one required parameter, which is an integer
   * 
  */ 

}




// main loop
void loop() {

  // run this to check if the user is inputting anything;
  // once it detects input, it will enter console mode until the user presses escape
  // warning: will block other execution until exited by pressing escape
  runSerialCommands();
  

}