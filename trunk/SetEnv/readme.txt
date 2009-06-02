SETENV variable[=string] [/PID processid ...] [/IM imagename ...]
                         [/P [depth]] [/V] [/U] [/W]

Description:
  Sets the value of an environment variable for any process or processes

Parameters list:
  variable   Specifies the environment-variable name.
  string     Specifies a series of characters to assign to the variable.
  processid  Specifies the process ID of the process.
  imagename  Specifies the image name of the process.
  /P depth   Specifies processing only chains of parental processes
             with optional limitation of depth.
  /V         Specifies that verbose information be displayed in the output.
  /U         Specifies that output to a pipe or file to be Unicode.
  /W         Specifies to wait for pressing any key after execution.

NOTE:
  Wildcards '*', '?' can be used to specify imagename and processid.

Examples:
  SETENV TMP=c:\temp /IM Far.exe /PID 716 2???
  SETENV "path=%path%;d:\tools" /IM *
