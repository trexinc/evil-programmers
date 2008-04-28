*************************** Windows NT/2000/XP ONLY ***************************

Introduction

Background Copy is a plugin for FAR Manager to perform
file(s) view/edit/copy/move/delete/run with Unicode support.
Also copy/move/delete performs in background.

It consists of 2 parts:
- bcsvc.exe - the service that processes requests to perform copy/move/delete
- bcopy.dll - a FAR Manager plugin

Status
Background Copy is freeware. You can use the binaries and the source code for any
purpose you like.

Warranty
Background Copy is distributed "AS IS". No warranty of any kind is expressed or
implied. You use it at your own risk.

Contacts

E-mail: zg@bmg.lv

History

v1.3 build 57 xx-xx-2008
- "show info" in dialogs.
- x64 version

v1.3 build 56 01-03-2008
- token stuff removed.
- show name buffer overflow fixed.
- "/P" key to bcsvc.exe added.

v1.2 build 54 01-04-2004
- techinfo#3 description is corrected.
- macro processing is corrected.

v1.1 build 53 08-12-2003
- attributes set in symbolyc links.
- notifier in service.

v1.1 build 52 16-11-2003
- macros in info menu.
- "info" menu refreshed similarly "info" dialog.

v1.0 build 51 06-10-2003
- gcc 3.3.1.
- new macro support. Macros stored in "KeyMacros" subkey.
- service support CPS limit for copy operations.
- buffer overrun on plugin panels.
- default value TechRefreshInterval set to 250.
- refresh procedure for "info" dialog changed.
- fixed: ".." processing in FAR 1682 and higher.
- now you can limit queue size.
- bcopy.dll exports ShowInfoMenu function for use from external plugin.
- new TechPreload techinfo.
- set work pipe access permissions and audit through "FARBCopy.access" file.

v1.0 build 50 10-04-2003 release
- removed unneeded '"'-symbol from resources.
- some fixes in language and help files.

v0.9 build 49 22-03-2003
- name work(but worse) in quick view and tree.
- symbolic links support.
- dialog history bug.
- gcc 3.2.2.

v0.9 build 48 30-01-2003
- name work in viewer, editor and temporary panel.

v0.9 build 47 15-01-2003
- name feature,
- run feature.

v0.9 build 46 11-01-2003
- "read/write" pipe error shown.
- ampersand bugfixes.
- some code rewritten.

v0.9 build 45 10-01-2003
- "memory allocation" error shown.
- "get list" error shown.
- some code rewritten.
- option - Bring destination path to UNC.
- lng fix.

v0.9 build 44 30-12-2002
- eject/load current path more clever.

v0.9 build 43 30-12-2002
- eject/load current path.
- very long paths now cause error.

v0.9 build 42 18-12-2002
- settings for service threads priorities.

v0.9 build 41 06-12-2002
- macros added to Info dialog.

v0.9 build 40 20-11-2002
- custom macros. hotkeys added in build 31 - removed.
- fixed: rarely service may hang system.

v0.9 build 39 11-11-2002 FE
- fixed: network&subst bug.
- fixed: network&symlink bug.
- in overwrite/retry dialogs buttons without brackets.
- service sources now uses common ntdll.h.
- service now don't call ansi functions.
- handle not closed, when password readed/written.

v0.9 build 38 01-11-2002
- viewer&editor work better.
- fixed: oem characters bug in retry dialog.

v0.9 build 37 01-11-2002
- interactive retry.
- overwrite dialog incorectly shows file names with oem chars.
- potencial bug with move between different volumes.
- view&edit didn't work in temporary panel.

v0.9 build 36 01-10-2002
- work only in FAR 1.70 beta 5.
- panel refresh now work a bit faster.
- fixed: invalid colors in info menu in FAR 1.70 beta 5.
- Append speed increased.
- now it is possible copy/move/delete from temporary panel plugin.

v0.9 build 35 10-09-2002
- path passed from copy/move dialog now unquoted.
- fixed: CDROM to nul copying bug.

v0.9 build 34 23-08-2002
- information menu updates immediately after closing information dialog.
- fixed: state in information menu didn't updates sometimes.

v0.9 build 33 22-08-2002
- service usage cosmetic bug.
- ask is a default action.
- new button "Close" in overwrite dialog.
- "ask" communication protected by unique id.
- listen threads priority increased.

v0.9 build 32 16-08-2002
- fixed: wipe config wasn't exist.
- Ask feature.

v0.9 build 31 15-08-2002
- Alt-Del in wipe dialog.
- Ctrl-A in attribute dialog.
- F8 in delete dialog.
- F5/F6 in copy/move dialog.
- fixed: when current time less than start time - plugin traps.
- dead code removed.
- fixed: somtimes folder time didn't copy and service reported about error.

v0.9 build 30 04-05-2002 RC
- info menu now willn't close after info dialog closing.
- fixed: invalid error reporting when delete.

v0.9 build 29 28-04-2002
- smart macros. see "macros" directory for details.
  thanks a lot to Alexey Yatsenko <alv@hotbox.ru>.
- changes in helpfiles
- fixed: help in wipe dialog.
- info menu: show percents.

v0.9 build 28 11-04-2002
- fixed: many cosmetics bugs in info menu.
- file changed to item.
- fixed: strings don't get out dialogs.
- CPS calculared more clear.
- fixed: wipe more abortable.

v0.9 build 27 10-04-2002
- autorefresh.
- fixed: delete info AV. introduced in build 26.

v0.9 build 26 06-04-2002
- eject/load redesigned.
- new option - show current time.
- you can define color for errors.
- fixed: cosmetic bugs in wipe and attributes dialog.
- wipe: calculated in CPS, updated more frequently.
- fixed: incorrect state restored after abort cancelation.
- fixed: del in info menu didn't ask confirmation.
- labels changed in abort confirmation dialog.
- fixed: service manager handle didn't close.
- fixed: ok button in info dialog now default.
- info menu rewritten. now it is live.
- fixed: pause AV. introduced in build 25.
- removed support of synchronous stop.
- fixed: wipe sometimes didn't remove temporary file.
- FILETYPE_DELETE and FILETYPE_UNKNOWN were the same.

v0.9 build 25 10-03-2002
- add width to "maximum errors per thread" option.
- fixed: year overflow...
- confirm abort option.
- service does not display the critical-error-handler message box.
- option "Clear R/O attribute from CD files" reseived from FAR API.
- wipe.
- fixes in documentation.
- environment variables and oem.
- memory leak.
- now you can eject/load A: and B: disks.
- service now responce when build file list.
- managing from info menu.
- more convenient labels in info menu.
- rename directory routine moved into plugin.
- copy with rename.
- move work more fine.

v0.8 build 24 25-09-2001 beta4
- titles in menus.
- changes in documentation.
- fixed: bug in configure service.

v0.7 build 23 22-09-2001 beta3
- new default options.
- changer SYSTEM account detecting algorithm.
- 'Now service is building the file list...' message.

v0.6 build 22 18-09-2001
- now service write username into event log database.
- 'Show full info' option now affected into delete and attribute processing.
- copy speed optimization.
- fixed: compress bug in w2k (maybe).
- autoshow information dialog feature.
  thanks a lot to Alexey Yatsenko <alv@hotbox.ru>.
- fixed: security bug. introduced in build 21.

v0.6 build 21 07-09-2001
- enchangemets in time prediction.
- format size.
- cosmetic bugs.
- copy speed optimization.
- now you can really disable network connections to the named pipe.

v0.6 build 20 04-09-2001
- time bug fixed.
- plugin: add attribute processing.
- plugin: now plugin support "Clear R/O attribute from CD files" FAR option.
- service: add attribute processing.
- fixed: network access from pipe bug.

v0.6 build 19 25-07-2001
- changes in network access with manually set account.
- changes in internal structure.
- fixed: didn't work Stop/Continue. introduced in build 17.
- fixed: "disable network connections" sometimes didn't work.

v0.6 build 18 16-07-2001
- internal build for testing purposes.

v0.6 build 17 14-07-2001
- fixed: ampersand in file names.
- fixed: hidden/system attribute bug.
- now you can disable network connections to the named pipe.
- fixed: you can copy/move to nul/prn/lptx/comx.
- fixed: filename in copy/move/delete dialog now truncated.

v0.6 build 16 06-07-2001
- SCSI refresh.
- changed bcopy.lng.
- fixed R/O bug (maybe).
- enchanged copy dialog.
- fixed very small bug in append.

v0.6 build 15 30-06-2001 beta2
- now menu displayed in viewer/editor.
- more convenient configure.

v0.5 build 14 26-06-2001
- new network.txt and network.rus.
  thanks a lot to Alexey Yatsenko <alv@hotbox.ru>.
- russian .hlf file. thanks a lot to Alexey Yatsenko <alv@hotbox.ru>.
- fixed directory creating routine.
- added new config option - "Check passive panel type".
- many tech options moved into config menu.
- fixed bug in remove directory stuff.
- added configure into plugin menu.
- changed method to get real file name from FAR.
- now you can use variables (for example %TMP%) in copy dialog.
- fixed: can't copy files into share with oem chars in name.
- fixed: oem chars in mapped and substituted disks.

v0.5 build 13 21-06-2001
- substs are resolved now.
- fixed problem with UNC names in w2k.
- fixed problem with CD-RW (maybe).
- fixed: log non-error.
- additionals info menu behaviours.

v0.5 build 12 22-05-2001
- help work in menu.
- fixed finish time bug.
- changes in bcopy.lng.
- calculated pause time.
- copied access rights in "copy" command.
- some networks improvements. see network.txt for details.

v0.5 build 11 22-05-2001
- internal build for testing purposes.

v0.5 build 10 20-05-2001
- internal build for testing purposes.

v0.5 build 09 18-05-2001
- now you can turn error logging off.
- now you can set number of listening threads from 1 to 16.
- startup synchronization.
- fixed bug with critical section init.
- copy directory attributes and times.
- copy access rights.
- rearrange options.
- CloeseHandle after OpenThreadToken.

v0.5 build 08 14-05-2001 beta1
- now in service four listening threads.
- sometime info dialog closes independently.
- pause in info dialog.
- info in viewer/editor.
- bug in info menu fixed.

v0.5 build 07 13-05-2001
- stop all threads before service stopping/shutdowning.
- service autostart before operations.
- now service correctly reports service manager about supporting features.
- now you not needed to change service account to access to network.
  thanks to Alexey Yatsenko <alv@hotbox.ru>.
- some support for russian language.
- russian .lng file. thanks a lot to Alexey Yatsenko <alv@hotbox.ru>.
- all paths converts into UNC paths before using.
- now you can set maximum number of errors per operation, written into
  error log. default value 20.
- in info dialog, if option 'Show full info' set, you can see number of
  errors, occurred in file operation processing.
- now service before each connect performs heap validation.
- fix in EventLog support.
- remove info list size limitation. It was 128 entries.
- some code optimization.
- change CopyFile to CopyFileEx.
- change memory manager.
- sources changed for FAR 1.70.3 build 591.
- skip copy/move if passive panel supported by plugin.
- fix in rename stuff.

v0.0.6 12-04-2001 Alfa3
- now you can manage service throw the pipe.
- memory leak in CopyThread.
- fix append+move bug.

v0.0.5 24-01-2001 Alfa2
- directory renaming.
- support junctions in subdirectories.
- improve info dialog. New copy option - 'Show full info'.
- remove selection after delete initiate.
- fix bug with showing Info.
- add eject/load drive.
- in config: add 'Use system copy history', remove 'Remove source files'.
- check pipe existing.
- add 'Move' to menu.

v0.0.4 10-01-2001 Alfa1
- view/edit.
- some junction stuff.
- remove selection.
- check FAR Version.
- log into EventLog message database.

v0.0.3 07-01-2001
- info work now, when appending.
- help ;)).
- info autoclose.
- config.
- info. stop.
- ignore plugin panels.
- more heuristic destination type detection.
- destination path length limited in edit control.
- delete function is implemented.

v0.0.2 31-12-2000
- dynamic caption.
- directory junction(untested).
- create unexisted destination dirs.
- clear r/o.
- src==dest - error...
- fail on error.
- don't copy dirs into themselves.
- log errors.
- move.

v0.0.1 26-12-2000
- first version.