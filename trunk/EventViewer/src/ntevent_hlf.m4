include(`ev_ver.m4')dnl
.Language=English,English
.PluginContents=NT Events

@Contents
$ `#'NT Events MAJOR.MINOR build BUILD TYPE`#'

  NT Events plugin allows to list/backup/clear NT Event Logs.

  ~Keyboard reference~@Kbd@
  ~Configure~@Config@
  ~Columns~@Columns@
  ~About~@About@

@Kbd
$ #NT Events - keyboard reference#

  ~Logs panel~@Kbd1@
  ~Events panel~@Kbd2@

@Kbd1
$ #NT Events - keyboard reference - logs panel#

  Backup selected logs                                            #F5#
  Backup log under cursor                                   #Shift-F5#
  Connect to remote machine                                       #F6#
  Return to local machine                                   #Shift-F6#
  Clear selected logs                                             #F8#
  Clear log under cursor                                    #Shift-F8#

@Kbd2
$ #NT Events - keyboard reference - events panel#

  View event in viewer                                            #F3#
  View event data in viewer                                 #Shift-F3#
  View event in editor                                            #F4#
  View event data in editor                                 #Shift-F4#
  Copy selected events                                            #F5#
  Copy event under cursor                                   #Shift-F5#
  Connect to remote machine                                       #F6#
  Return to local machine                                   #Shift-F6#

@Config
$ #NT Events - configure#
  In this dialog you may change the following options:

  #Add to disks menu#      Enables adding the "event viewer" item
                         to the disks menu.

  #Disks menu hotkey#      Hotkey for the event viewer item in
                         the disks menu. You can enter a value
                         from 1 to 9 or leave this field empty
                         to autoassign.

  #Add to plugins menu#    Enables adding "Event viewer" item to the
                         plugins menu.

  #Browse .evt files#      When checked, *.evt files can be opened
                         by this plugin for browsing. Otherwise, the
                         plugin doesn't open them, leaving it
                         for other plugins.

  #Strip second extension# By default, plugin strips second extension
                         (err, war, inf, aus, auf) before copying.
                         This option allows to disable this behavior.

  #Scan forward#           Plugin puts oldest record first.

  #Scan backward#          Plugin puts newest record first.

  #Command line prefix#    Specifies the prefix for opening the nt
  #to start nt events#     events panel from the command line.

@Columns
$ #NT Events - columns#
  You can define log and event panels view mode
  in message file(ntevent.lng). Each panel has ten modes.

  Each mode is described by five lines:
  1. columns type;
  2. columns size;
  3. columns type in status line;
  4. columns size in status line;
  5. Full screen mode.

  You can use the following column names in the first and third
  lines:

  #C0# - Event id.
  #C1# - Event category.
  #C2# - Event generation time. Plugin adds 2 dots at the top.
  #C3# - Computer.
  #C4# - User.
  #C5# - Event generation time.
  #Z # - Event source.

  Full screen mode defined by the #1# symbol.

@About
$ #NT Events - about#

  Copyright (c) 2001 by ZG.

  E-mail: ~zg@@matrica.apollo.lv~@mailto:zg@@matrica.apollo.lv?subject=NT Events plugin@
