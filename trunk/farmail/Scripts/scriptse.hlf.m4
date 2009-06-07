m4_include(`../FARMail/fm_version.m4')m4_dnl
.Language=English,English
.PluginContents=Scripts - FARMail sub plug-in

@Contents
$^`#'Scripts - FARMail sub plug-in `v'MAJOR.MINOR#

 - ~Usage - Main menu~@Usage@
 - ~Writing scripts~@Script@
 - ~Configuration~@Config@
 - ~Authors~@Authors@
 - ~License~@License@


@Usage
$^#Usage - Main menu#

  #1. Run script#
  Opens the ~Run script~@RunScript@ dialog.

  Now below the separator you can select/add/delete/update/relocate the
settings for the scripts you want to use. (~Script settings dialog~@Settings@)

  #Ins#
  Add a new script setting (max 26).

  #Del#
  Delete the settings for the script under cursor.

  #F4#
  Update the settings for the script under cursor.

  #Ctrl-Up, Ctrl-Dn#
  Move the item under cursor one position up or down.

  #Enter#
  Execute the script under cursor.



 ~Contents~@Contents@


@Settings
$^#Script settings dialog#

#Description# - The description that will be shown in plug-in's menu.

#Full path to help file# - The full path to the script file.



 ~Contents~@Contents@


@RunScript
$^#Run script dialog#

#Script filename# - enter here the full or relative path to the script file
You want to run, or just the file name if the script is located in the default
scripts directory (See ~Configuration~@Config@).

#Note:# To run a script located in the current directory you should write:
.\filename.ext



 ~Contents~@Contents@


@Script
$^#Script writing#

  - ~Syntax~@Syntax@
  - ~Functions~@Functions@

  You should note that to prevent encoding problems you should write all
scripts in the OEM codepage.



 ~Contents~@Contents@


@Syntax
$^#Syntax#

 #Variables#.
 There are two kinds of variables: of #integer# and #string# type.
You don't need to declare them manually, and You can use undefined
variables which are set by default to 0 (or "0" for strings).
All variables can be accessed anywhere in the script code.
 Strings are the same as in C/C++ with all the leading consequences.
 Integers are 32bit whole decimal numbers.


 #Assignment#.
 To assign a value, simply write #<variable>=<value>#.

   #i = 0#
   #j = "abc"#


 #Comments#.
 Line comments: Any text following "#rem #" (rem and a space) or "#//#" is
ignored.
 Block comments: Any text between #/*# and #*/# is ignored.

   #rem this is a comment#
   #// this is a comment#
   #/*this is#
   #a comment*/#


 #Constants#.
 There are two kinds of constants: integer and string.
A character in single quotes is an integer value. Examples:

   #123#, #'A'# - integer numbers;
   #"123"#, #"asd"# - string constants.


 Operator #[]#.
 You can access a single character in a string variable:
#<string variable>[<position of character>]# (starting index is 0).
Note that You can set this value. Examples:

   #i = str[2] + 4#
   #str[2] = 'a'#


 #Arithmetic expressions#.
 You can use '#++#', '#--#', '#+#', '#-#', '#*#', '#/#' and '#()#'
in expressions with integer values, and '#+#' with string
values:

   #i = -3*(j+k)*(1-l/2)#
   #str = str + "123"#
   #i = 5# -> #j = ++i + 1# -> #j==7, i==6#
   #i = 5# -> #j = i-- + 1# -> #j==6, i==4#

 You should know that strings and integers can be mixed within arithmetic
expressions, the type of the resulting value will be determined automatically
and when needed strings will be converted to numbers and numbers to strings.


 #Conditional expressions#.
 A conditional expression is an integer
value, 0 means false, any other value means true. It can be an arithmetic
expression, or a combination as below:

   #a1 == a2#    true if a1 is equal to a2
   #a1 != a2#    true if a1 is not equal to a2
   #a1 <= a2#    true if a1 is less or equal to a2
   #a1 < a2 #    true if a1 is less then a2
   #a1 >= a2#    true if a1 is more or equal to a2
   #a1 > a2 #    true if a1 is more then a2
   #!a1     #    not a1 (only if a1 is an integer value)
   #a1 || a2#    a1 or a2 (only if a1 and a2 are integer values)
   #a1 && a2#    a1 and a2 (only if a1 and a2 are integer values)


#Commands#.

 1. Conditional statement.

   #if# <condition is true>
     ...
   #else#
     ...
   #endif#

 2. Conditional loop.

   #while# <condition is true>
     ...
   #wend#

   You can use the #break# command to exit the loop immediately.
   You can use the #continue# command to jump immediately to the next
iteration.

 3. Defining and Calling sub-functions

   #sub# Name
     statements
     ...
   #endsub#

   #gosub# Name

   Just remember that all variables can be accessed anywhere in the script
code even those defined in sub-functions.

 ~Script writing~@Script@

@Functions
$^#Functions#

#1. Working with the editor#

 #Important note:# For best performance when working with the internal editor
it is recommended to first set the cursor position to the line you are about
to access and then access it by calling the appropriate function with line
number set to "-1".

 integer #nlines#().

   Parameters: none.
   Returns the number of lines in internal editor.
   There will always be at least one line.

 string #line#( integer i ).

   Parameters: i - number of line, -1 for current.
   Returns line i from internal editor.

 integer #setline#( string str, integer i ).

   Parameters: str - new line value, -1 for current;
               i - line number.
   Sets line i in internal editor to str.
   Returns 0.

 integer #delline#( integer i ).

   Parameters: i - line number, -1 for current;
   Deletes line i in internal editor.
   Returns 0.

 integer #insline#( integer i, integer eol ).

   Parameters: i - number of line, -1 for current;
               eol - end-of-line flag.
   Inserts a new empty new line at position i. If eol is
equal to 0, new line will be inserted at the beginning of
existing line. Else - at the end of line.
   Returns 0.
   Note that this function changes the cursor position.

 integer #setpos#(integer i, integer pos).

   Parameters: i - line number line, -1 for current;
               pos - position on the line, -1 for current.
   Set the cursor position to line i, position pos.
   Call setpos(-1) to get current line number.
   Call setpos(-1,-1) to get current position in the line.
   Returns zero when setting position.

 string #editorstate#().

   Parameters: none.
   Retrieves the state of the current editor.
   Returns the following string with each character corresponding to a specific
state of the editor:
   first  char (index 0) - '1' if file was modified, '0' otherwise;
   second char (index 1) - '1' if file was saved, '0' otherwise;
   third  char (index 2) - '1' if file is locked, '0' otherwise.

 integer #blktype#().

   Parameters: none.
   Returns the type of the selected block:
    - no selection - 0;
    - stream - 1;
    - vertical - 2.

 integer #blkstart#().

   Parameters: none.
   Return the number of the starting line of the selection.

 integet #setsel#(integer blktype, integer StartLine, integer StartPos, integer Width, integer Height).

   Parameters: blktype - no selection - 0
                       - stream - 1
                       - vertical - 2;
               StartLine - selection starting line;
               StartPos - selection starting position in that line, if -1 block
will be deselected;
               Width - width of the selected block, can be a negative value;
               Height - height of the selected block, must be >=1.
   Sets selection in the editor. To clear selection call setsel().
   Note: StartLine and StartPos for the first line and the first column is 0,0.
   Returns zero always.

 integer #selstart#(integer i).

   Parameters: i - line number, -1 for current.
   Returns the starting position of selection in the given line. If line has
no selection, returns -1.

 integer #selend#(integer i).

   Parameters: i - line number, -1 for current.
   Returns the ending position of selection in the given line. If the
selection area includes the end of line sequence of that line, returns -1.

#2. Working with strings#

 integer #strlen#( string str ).

   Parameters: str - string value.
   Returns the length of the string str.

 string #substr#( string str, integer delta, integer len ).

   Parameters: str - a string value; delta - start position in str;
               len - length of new string.
   Returns a substring of str, beginning at position delta, with
length of len characters.

 integer #strstr#( string str, string substr ).

   Parameters: str - a string value; substr - substring to search
               in str.
   Returns position of substr in str. If substr is not a substring
of str, returns -1.

 string #strlwr#( string str ).

   Parameters: str - a string value.
   Returns a string with uppercase letters converted to lowercase.

 string #strupr#( string str ).

   Parameters: str - a string value.
   Returns a string with lowercase letters converted to uppercase.

 string #sprintf# ( string format, ...).

   Parameters: format - a format string
               ... - any number of variables of any type
   Writes formatted output to a string. sprintf accepts a series of arguments,
applies to each a format specifier contained in the format string pointed to
by format, and outputs the formatted data to a string. sprintf applies the
first format specifier to the first argument, the second to the second, and so
on. There must be the same number of format specifiers as arguments.
   The format string is the same as in C/C++ (actually wvsprintf).
   Returns the formated string.

 string #ltrim#( string str ).

   Parameters: str - a string value.
   Returns a string containing a copy of a specified string with no leading
spaces and tabs.

 string #rtrim#( string str ).

   Parameters: str - a string value.
   Returns a string containing a copy of a specified string with no trailing
spaces and tabs.

 string #trim#( string str ).

   Parameters: str - a string value.
   Returns a string containing a copy of a specified string with no leading or
trailing spaces and tabs.

#3. Conversion#

 string #string#( integer i ).

   Parameters: i - an integer value.
   Converts i to a string value.
   Returns i converted to string type.

 string #char#( integer c ).

   Parameters: c - an integer value containing an ascii code.
   Makes a one character string from character c.
   Returns one character string.

 integer #integer#( string str ).

   Parameters: str - a string value.
   Converts str to an integer value.
   Returns str converted to integer type.

#4. User interaction#

 integer #message#( (integer|string)var, ... ).

   Parameters: any number of strings and integers.
   Displays a message with all the parameters as lines of text.
   Returns 0.

 string #usrinput#( string title, string message, string init[, string history] ).

   Parameters: title - dialog title;
               message - line of text displayed in the dialog;
               init - initial text written in the input line;
               history - name of input history.
   Displays a dialog with one line of text and one input line.
   Returns the contents of the input line, empty string on cancellation.

 integer #usrmenu#( (integer|string)var, ... )

   Parameters: any number of strings and integers.
   Displays a menu with all the parameters as menu items.
   Returns the number of the selected menu items or -1 on cancellation, menu
indexes start from 0.

 string #addressbook#()

   Parameters: none.
   Opens up the Address Book sub plug-in (if such installed) and returns the
selected item (full name and e-mail) or empty string on cancellation.

#5. Working with message specifics#

 string #header#( string msg_header, string kludge_name )

   Parameters: msg_header - the message header you need to process;
               kludge_name - the name of the kludge you want to extract from
the header (e.g. "From:", "To:" etc).
   Returns the wanted kludge or empty string if no such kludge found.
   The returned string will be decoded from Base64 or QP if needed, multiple
line kludges will be returned as one concatenated string.

 string #boundary#().

   Parameters: none.
   Returns a string containing a randomly generated MIME boundary in the form
of "--=_NextPart_XXXX_XXXX_XXXX.XXXX" where "X" stands for a hex number.

 string #encodeheader#(string text_to_encode, string encoding)

   Parameters: text_to_encode - the text you want to encode;
               encoding - the name of the charset encoding.
   Returns the the given text encoded in mime base64 in the wanted charset.
   The name of the encoding may be the name of one of the charsets defined in
FARMail (e.g. "koi8-r" etc.).
   The encoded text will be in the following form:
=?encoding_name?B?encoded_text_in_base64?=. It will also be split to several
such string as one exceeds 75 chars in length.

#6. Working with files#

 integer #filecount#( string path ).

   Parameters: path - a string value specifying a full path to a file or just
the file name if it is in the scripts directory.

   Returns the number of lines in file "filename", 0 on error.

 string #fileline#( string path, integer i ).

   Parameters: path - a string value specifying a full path to a file or just
the file name if it is in the scripts directory;
               i - an integer value specifying line number.
   Returns a string containing the line i from the given file.
   On any error returns an empty string.
   #Note:# 1.The maximum length of any line in the file should be less than
1000, for correct functionality of this function.
         2. Function is intended for one time usage, don't abuse it.

 integer #redirect#(string path, integer mode).

   Parameters: path - directory of file name;
               mode - work mode number.
   The purpose of this function is to copy (redirect) the contents of the
current editor to some other file. If the current editor is modal then
script execution will end with this function except in mode 4 or 5.
   If mode is even then path is taken as a directory where to create a file,
file name will be a unique eight figure number prepended with zeros.
   If mode is odd then path is taken as a file name to create.
   If mode equals 0 or 1 - current editor text is redirected to the wanted
file, the current editor is closed and the file is then opened in a new editor
where the script can continue.
   If mode equals 2 or 3 - current editor text is redirected to the wanted
file, the file is then opened in a new editor (if current editor is none modal)
which gets the focus so the script can continue.
   If mode equals 4 or 5 - current editor text is redirected to the wanted
file, the other panel is made to show the file.
   Returns 0.

 string/integer #getini#(string section, string key, string/integer default, string ini).

   Parameters: section - name of a section in the ini file;
               key - name of a key in the ini file;
               default - default value for that key;
               ini - path to the ini file.
   Returns the value of the given key in the given section of the given ini
file. If no such key/section/file exists the given default value is returned.
The type of the return value (integer/string) depends on the type of the given
default value. Empty string is returned on error.

 integer #setini#(string ini, string section, string key, string/integer value).

   Parameters: ini - path to the ini file;
               section - name of a section in the ini file;
               key - name of a key in the ini file;
               value - value for that key.
   Sets the value of the given key in the given section of the given ini
file. If no value is given then key is removed, if no key and value are given
then the section is removed.
   Returns zero on error or positive value in success.


#7. Miscellaneous#

 #exit#().

   Parameters: none.
   Stops script execution.

 integer #random#( integer i ).

   Parameters: i - an integer value.
   Returns a random number between 0 and (i-1).

 string #date#().

   Parameters: none.
   Returns a string containing current date in form dd.mm.yyyy.

 string #time#().

   Parameters: none.
   Returns a string containing current time in form hh:mm:ss.

 string #version#(integer i).

   Parameters: i - type of wanted program:
                - 0 - FARMail;
                - 1 - FAR;
                - 2 - Windows.
   Returns a string containing the name and version of the wanted program.

  The following two functions work with:
  1. Winamp 1.x, 2.x, 5.x.
  2. Winamp 3.x if it has the "Winamp 2.x Plugin Manager"
     component installed.
  3. STP, if "Emulate Winamp" option is checked.
  4. Appolo 37.
  5. Foobar2000. Restrictions:
     - can't distinguish between pause and play.
     - #winampsong# takes the title of the current
       song from the window title.

 integer #winampstate#()

   Parametrs: none.
   Returns: -1 - winamp not found.
             0 - winamp is stopped.
             1 - winamp is now playing some music composition.
             3 - winamp is paused.

 string #winampsong#()

   Parametrs: none.
   Returns the name of the current composition being played by winamp.

  Example of usage:

  #pos=nlines()-1#
  #wa=winampstate()#
  #if(wa==-1)#
    #insline(pos,1)#
    #setline("winamp not found",pos+1)#
  #else#
    #waname=winampsong()#
    #if(wa==0)#
      #insline(pos,1)#
      #setline("stopped: "+waname,pos+1)#
    #else#
      #if(wa==1)#
        #insline(pos,1)#
        #setline("playing: "+waname,pos+1)#
      #else#
        #insline(pos,1)#
        #setline("pause: "+waname,pos+1)#
      #endif#
    #endif#
  #endif#


 ~Script writing~@Script@


@Config
$^#Configuration#

#Script's dir# - enter here the full path to default directory to search in
for scripts. Leave empty to search in SCRIPTS sub folder in plug-in's folder.



 ~Contents~@Contents@


@Authors
$^#Authors#

  Currently the development of this plug-in is maintained by FARMail Group:

  #Alex Yaroslavsky#
  Homepage : ~http://code.google.com/p/evil-programmers/~@http://code.google.com/p/evil-programmers/@

  #Vadim Yegorov#
  E-mail   : at bmg.lv / zg
  Homepage : ~http://zg.times.lv/~@http://zg.times.lv/@

  Original idea and implementation by:

  #Serge Alexandrov#
  E-mail  : at zmail.ru / poseidon
  Homepage: ~http://alsea.euro.ru/~@http://alsea.euro.ru/@
            ~http://dpidb.genebee.msu.ru/users/poseidon/~@http://dpidb.genebee.msu.ru/users/poseidon/@



 ~Contents~@Contents@

@License
$ #License#

 Scripts sub-plugin for FARMail
 Copyright (C) COPYRIGHT FARMail Group
 Copyright (C) 1999,2000 Serge Alexandrov

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 ~Contents~@Contents@
