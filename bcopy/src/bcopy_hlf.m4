include(`bc_ver.m4')dnl
.Language=English,English
.PluginContents=Background copy
.Options CtrlColorChar=\

@Contents
$  `#'Background copy MAJOR.MINOR build BUILD TYPE`#'

  \0C                                                                 \-
  \0CPlugin works ONLY in Windows NT 4.0, Windows 2000 and Windows XP.\-
  \0C                                                                 \-

  With this plugin you can:

  - copy/move/rename files and folders in the background;
  - delete/wipe files and folders in the background;
  - change file attributes in the background;
  - get info about current background jobs;
  - abort or temporary stop active background job;
  - eject/load removable media;
  - refresh SCSI adapters;
  - copy/move/delete/run/view/edit files with names containing
    unicode symbols;
  - unattended, "trying to complete as much as it can" work of
    copy/move/delete/wipe/attributes processes.

  ~Installation~@Install@
  ~Removal~@Uninstall@
  ~Configure~@Config@
  ~Error handling~@Errors@
  ~Network access~@Network@
  ~Copy/move~@Copy@
  ~Delete/wipe~@Delete@
  ~Attributes~@Attributes@
  ~Show name~@ShowName@
  ~Info~@Info@
  ~Macros~@Macros@
  ~Pipe~@Pipe@
  ~Thanks~@Thanks@
  ~About~@About@

@Install
$  #Background copy - installation#

  You have to be logged in as an administrator to install the
  Background Copy Service.

  1. Create a subdirectory in your FAR Manager plugin directory.
  2. Copy #Background Copy# plugin files there.
     Minimal working set is:
       bcopy.dll
       bcsvc.exe
       bcopy.hlf
       bcopy.lng
  3. Execute #bcsvc.exe /i# (service installs as "manual start") or
     #bcsvc.exe /a# (service installs as "automatic start").

@Uninstall
$  #Background copy - removal#

  If you want to remove the #Background Copy# service, simply run
#bcsvc.exe /r#. After this, you can delete the files, which belong
to #Background Copy#.

@Copy
$  #Background copy - copy or move files#

  If destination path ends with '\' or '..' or '.' it is treated as a
path. Otherwise it is treated as a file.

  Modes

  #Overwrite all existing files#       - if in the destination
                                       directory exists a file with
                                       the same name as in the source
                                       directory - it will be
                                       overwritten.
  #Append to all existing files#       - if in the destination
                                       directory exists a file with
                                       the same name as in the source
                                       directory - the source file
                                       will be appended to the
                                       destination file.
  #Skip all existing files#            - if in the destination
                                       directory exists a file with
                                       the same name as in the source
                                       directory - the copying of
                                       this file will be skipped.
  #Refresh old files#                  - if in the destination
                                       directory exists a file with
                                       the same name as in the source
                                       directory - it will be
                                       overwritten only if it is
                                       older than the file in the
                                       source directory.
  #Ask#                                - if in the destination
                                       directory exists a file with
                                       the same name as in the source
                                       directory - user will be asked
                                       for action.

  Options

  #Remove source files#                - move files, instead of
                                       copying.
  #Copy access rights#                 - copy file access information.
  #Delete source read-only files#      - delete source read-only files
                                       when moving.
  #Overwrite target read-only files#   - overwrite the target file even
                                       if it is read-only.
  #Abort on error#                     - on any error:
                                       #[x]# - stop the job.
                                       #[ ]# - continue the job.
                                       #[?]# - ask user for action.
  #Copy symbolic links contents#       - on symbolic link:
                                       #[x]# - copy contents.
                                       #[ ]# - relink.
                                       #[?]# - ask user for action.

@Delete
$  #Background copy - delete or wipe files#

  Wipe means renaming to a temporary name, overwriting with
  zero-bytes, truncating and, at last, deleting.

  Options

  #Delete read-only files# - clear read-only attribute before
                           deleting a file.
  #Abort on error#         - on any error:
                           #[x]# - stop the job.
                           #[ ]# - continue the job.
                           #[?]# - ask user for action.

@Attributes
$  #Background copy - change file attributes#

  With this command it is possible to change file attributes.
  Checkboxes used in the dialog can have the following 3 states:
   #[x]# - set the attribute for all items
   #[ ]# - clear the attribute for all items
   #[?]# - don't change the attribute

@ShowName
$  #Background copy - show name#

  This command shows the real Unicode-name of current file/folder
  in the panels, or the name of file loaded in FAR viewer/editor.

  Note that true-type font is required, such as #Lucida Console#.

@Config
$  #Background copy - configure#

  Here you can configure ~dialogs~@Config1@, ~plugin~@Config2@ and ~service~@Config3@.

@Config1
$  #Background copy - configure dialogs#

  Values in #Copy# group set default values for the ~Copy/move~@Copy@ dialog.

  Values in #Delete# group set default values for the ~Delete~@Delete@ dialog.

  Value in #Wipe# group sets default values for the ~Wipe~@Delete@ dialog.

  #Use system copy#   - use the same history as FAR's copy/move
  #history#             dialogs.

  #Show full#         - show full ~information~@Info@ about active
  #information#         background process.


@Config2
$  #Background copy - configure plugin#

  Press #Color# button to change color for ~error highlighting~@Errors@.

  #Autostart service#                - automatically start service
                                     before any file operation.

  #Add to Config menu#               - add "Background copy" item to
                                     the plugins' configuration menu.

  #Add to Viewer menu#               - add "Background copy" item to
                                     the viewer plugins' menu.

  #Add to Editor menu#               - add "Background copy" item to
                                     the editor plugins' menu.

  #Add to Dialog menu#               - add "Background copy" item to
                                     the dialog plugins' menu.

  #Show empty dialog#                - show dialog when job list is
                                     empty.

  #Autoselect single job#            - when only one job runs -
                                     automatically show its info.

  #Autoshow information dialog#      - automatically show information
                                     dialog when job starts.

  #Expand variables in copy dialog#  - expand environment variables,
                                     for example #%OS%# will be
                                     transformed into #Windows_NT#.

  #Check destination panel type#     - when checked, copy/move won't
                                     work with non-file passive
                                     panel.

  #Format size#                      - show files' size in bytes,
                                     kilobytes, megabytes,...

  #Show current time#                - show current time instead of
                                     job start time in the
                                     ~information dialog~@Info@.

  #Confirm abort#                    - ask confirmation for the job
                                     abort.

  #Refresh panels#                   - refresh Far panels after the
                                     ~information dialog~@Info@ have
                                     been closed. Refreshing only
                                     happens if the background
                                     process affects panels'
                                     contents.

  #Bring destination path to UNC#    - Convert destination path in the
                                     ~copy dialog~@Copy@ to UNC `format'.
                                     It affects only mapped network
                                     disks, ~symbolic links~@:HardSymLink@ and
                                     virtual subst-disks. Other UNC
                                     and local paths won't be
                                     converted.

@Config3
$  #Background copy - configure service#

  #Maximum errors per thread#   - the maximum allowed number of
                                errors to be written to the
                                application log per operation.

  #Number of listening threads# - number of listening threads.

  #Queue size#                  - number of simultaneously
                                working jobs.

  #Working thread priority#     - priority of threads which
                                do main work.

  #Listening thread priority#   - priority of listening threads.

  #Allow network connections#   - enable connecting from another
  #to the pipe#                   workstation to the ~pipe~@Pipe@.

  #Enter password#              - enter your password.
                                See ~network access~@Network@.

  #Clear password#              - clear your password.
                                See ~network access~@Network@.

@Config4
$  #Background copy - set password#

   Here you can enter your password. See ~network access~@Network@.

@Errors
$  #Background copy - error handling#

   The errors occurring during non-interactive operations are
written to the #Windows NT application log#. You can view and
manage NT event logs with #NT Event# plugin.

   You can select color for the ~information dialog~@Info@ error
highlighting (see ~plugin configuration~@Config2@).

@Network
$  #Background copy - network access#

   FAR Background Copy Service has three approaches to using the
network (each one has its own disadvantages so it is up to you
to choose the right one for you).

  1. Using current user's account.
     This is the default case. The service has LocalSystem account
     and accesses network and local resources using account of the
     user who initiated the service activity (copying, deleting etc.)

     Note: As this user logs off, system aborts all service's
           network connections. This "feature" does not exist in
           #Windows XP#.

  2. Using user defined logon session. The service itself has
     LocalSystem account but you must enter your password
     in the ~service config~@Config3@ dialog. Password is stored for
     each user individually. Service accesses the network and local
     resources using account of the user who initiated the service
     activity (copying, deleting etc.)

     Note: Password is stored using the LsaStorePrivateData function.
           See ~Platform SDK~@http://msdn.microsoft.com/@ for details.

  3. Using account of FarBCopy service (works with MS Windows
     Network only). Set account of the service through "Services"
     applet in the Control Panel. The service will run and access
     the network and local resources under this account.

     Note: The service can not access NetWare resources in this case.

@Pipe
$  #Background copy - pipe#

  You can manage service through the pipe #\\\\.\\pipe\\FARBCopy#.

  Copy example:
  #echo copy /0 "C:\\Program Files\\Far" C:\\TEMP\\>\\\\.\\pipe\\FARBCopy#

  Move example:
  #echo copy /0m "C:\\Program Files\\Far" C:\\TEMP\\>\\\\.\\pipe\\FARBCopy#

  Delete example:
  #echo copy /0 C:\\TEMP\\*.* >\\\\.\\pipe\\FARBCopy#

  Word #copy# must be lowercase. All keys also must be lowercase.

  Key list:
  #/o# - overwrite all existing files.
  #/a# - append to all existing files.
  #/r# - refresh old files.
  #/m# - remove source files.
  #/f# - abort on error.
  #/u# - copy access rights.
  #/1# - clear R/O attribute on source.
  #/2# - clear R/O attribute on destination.
  #/0# - calculate statistic.

@Info
$  #Background copy - info#

  Press #OK# button to close dialog.
  Press #Abort# button to abort current job.
  Press #Stop# button to temporary stop current job.
  Press #Continue# button to continue stopped job.
  Press #Gray +# to switch between starting and current time.
  Press #Gray -# to switch between formatted and unformatted sizes.

@InfoMenu
$  #Background copy - info menu#

  Jobs marked with asterisk are working. Other jobs are stopped.

  Press #Del# to abort job.
  Press #Space# to stop active job.
  Press #Space# to continue stopped job.
  Press #Enter# to show ~information dialog~@Info@.

@Abort
$  #Background copy - abort job#

  Job you've selected is paused now.

  Press #Abort# button to abort job immediately.
  Press #Esc# or #Continue# button to let the job continue.

  You may disable this confirmation in the ~plugin configuration~@Config2@.

@Macros
$  #Background copy - macros#

  All macro commands are stored under the registry branch
  #[HKEY_CURRENT_USER\\Software\\Far\\Plugins\\BCopy\\KeyMacros]#

  The macro commands are divided into 10 areas:

  Area           Usage
  컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  "Copy"         Copy dialog
  "Move"         Move dialog
  "Delete"       Delete dialog
  "Attributes"   Attributes dialog
  "Wipe"         Wipe dialog
  "Overwrite"    Overwrite dialog
  "Retry"        Retry dialog
  "Info"         Info dialog
  "Link"         Link dialog
  "InfoMenu"     Info menu
  컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

  Each macro command has a separate branch in the registry. The name
  of the branch is the name of the key to which the macro command is
  assigned.  Each macro command is described with the
  following parameters:

  Parameter             Type       Default   Description
  컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  "Sequence"            REG_SZ               Macro key combination
  "DisableOutput"       REG_DWORD  1         Disable screen output
                                             while playing macro
  "Description"         REG_SZ               Comment (not used)
  컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  "Sequence" is a mandatory parameter, other values can be omitted
  and use the default values.

@Thanks
$  #Special thanks to...#

  ...my wife and son who fill my life with meaning.

  ...~Alexey Yatsenko~@mailto:alv@@hotbox.ru@ for his constant interest, assistance, useful ideas and valuable advice.

@About
$  #Background copy - about#

  Copyright (c) 2000-2003 by ZG.

  E-mail: ~zg@@bmg.lv~@mailto:zg@@bmg.lv?subject=Background%20Copy%20plugin@
