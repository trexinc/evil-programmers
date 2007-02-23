Position Saver v3.4.1, a FAR plug-in
------------------------------------

  This plug-in was written for people who need to save file positions from the
internal editor/viewer for an unlimited period of time and no matter how much
files have been edited/viewed in the meanwhile.

  To start tracking a file all you need to do is execute the "Track" command
from plug-in's menu, this command will be available only for files that are not
tracked yet and will appear as the "Save" command for already tracked ones.

  All settings for the tracked files are kept in a separate file (by default
the plug-in writes the settings to "positionsaver.cfg" file in plug-in's
directory) which is of rather simple format so no need for me to explain it
here, just start tracking some file and see what the plug-in writes to the
configuration file. The path for this configuration file can be set from
plug-in's common configuration dialog. And if you did not understand yet, you
can manually edit the configuration file and the changes will be read
automatically.

  The plug-in has two work modes:

  1. Automatic
  In this mode the position of a tracked file will be automatically restored
when it is opened in the editor/viewer. And upon exiting, the saved position
will be updated with the current one. This version introduces a fully automatic
tracking with no need of user intervention. But if you want to force the
plug-in to write changes to the configuration file you can press F2 to force a
file save (in editor only) or execute the "Save" (also "Track") command from
plug-in's menu.
  When working in this mode you can tell the plug-in to automatically track a
file if its name corresponds to a list of file masks you can specify. To turn
this feature on check the "Check mask" checkbox in plug-in's editor/viewer
configuration dialog and enter the list of wanted file masks (separated by
commas) in the input line next to the checkbox. The plug-in understands
standard FAR file masks which may include directories and environment variables
(e.g. *.[ch],*.[ch]pp,%userprofile%\books\*.txt).

  2. Manual
  In this mode to restore or save the position of a tracked file you will have
to execute the corresponding ("Save" and "Restore") commands from plug-in's
menu. Changes will be written immediately to the configuration file.

  Switching between the two work modes is done from plug-in's editor/viewer
configuration dialog using the "Allow automatic save/restore" checkbox.

Notes:
------

  Because the plug-in never "forgets" a tracked file you have several options
to prevent the plug-in from tracking too many files including files that no
longer exist.
  Automatic: If you set the "Remove old" checkbox in plug-in's common
configuration dialog, every time you exit FAR the list of tracked files will be
checked for non existing files and remove the corresponding settings.
  Manual: Here you have three ways of doing things:
  1. Execute the "List tracked items" command from plug-in's menu to be able to
view and delete tracked items.
  2. Edit the configuration file manually.
  3. Execute the "Remove old" command from the plug-in menu to check the list
of tracked files for non existing files and remove the corresponding settings,
if a non existing file is opened in the editor its setting will not be removed.

Acknowledgments:
----------------
  This software uses Colorer Library SGML codes by Cail Lomecb
<ruiv@uic.nnov.ru>, http://www.uic.nnov.ru/~ruiv/
