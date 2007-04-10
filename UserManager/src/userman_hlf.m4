include(`um_ver.m4')dnl
.Language=English,English
.PluginContents=User Manager
.Options CtrlColorChar=\

@Contents
$ `#'User Manager MAJOR.MINOR build BUILD TYPE`#'

 With this plugin you can:

 - manage access control lists for files, folders,
   registry keys and printers;
 - manage shares;
 - manage local and global users and groups;
 - manage user and group access permissions;
 - manage account rights.

 ~Plugins main menu~@MainMenu@
 ~Properties~@Properties@
 ~Apply to child objects~@Apply2children@
 ~Users and Groups~@Users@
 ~Account Rights~@Rights@
 ~Access control list~@ACL@

 ~About~@About@

@MainMenu
$ #User Manager - plugin main menu#

 This plugin adds one entry to the file panels plugins menu. You can open this
menu by pressing F11 in the file panels. Then you select the #User manager#
entry that you should see in the plugins menu. It will present you with a menu
containing the following four items:

 #Properties# - opens the ~properties~@Properties@ list of the selected object
in the active panel. Four object types are supported for now: files, folders,
registry keys and printers.

 #Apply to child objects# - ~applies~@Apply2children@ the default properties
of the selected container to all of its underlying objects. Two container
types are supported for now: folders and registry keys.

 #Users and Groups# - opens the ~users and groups~@Users@ list in the active
panel.

 #Account Rights# - opens the ~account rights~@Rights@ list in the active
panel.

@Properties
$ #User Manager - properties#

 When you open the properties list of an object, you'll see something similar
to this:

 \1B…ÕÕÕÕÕÕÕÕÕÕÕÕ\30 ACL:books.z\\ \1BÕ\-
 \1B∫\1Ex\1B                \1EName\1B      \-
 \1B∫\30..                         \-
 \1B∫\1FAccess Permissions\1B         \-
 \1B∫\1FAudit\1B                      \-
 \1B∫\1FShares\1B                     \-
 \1B∫ZG\\Administrator           \-

 There will be four items for folders and printers and three for files
and registry keys.

 #Permissions# - inside this folder you can view and manipulate object
access permissions.

 #Audit# - inside this folder you can view and manipulate object audit
polices.

 #Shares# - inside this folder you can view and manipulate folder or printer
shares. This item is available only for folders and printers.

 #Owner# - the last item shows the current object owner. In this example the
owner is #ZG\\Administrator#.

@Apply2children
$ #User Manager - apply to child objects#

 This operation can be applied only to containers. Each container contains not
only its own access permissions and audit polices but also the default access
permissions and audit polices for its underlying objects. This operation can
apply the default access permissions and audit polices of the selected
container to all of its underlying objects.

@Users
$ #User Manager - users and groups#

 In the root of the users and groups panel you can see a list of all local
or global users and a list of all local or global groups. Each group is
represented as a folder. When you enter a group, you'll see a list of all
its members.

 ~Users and groups list keyboard reference~@UsersKeys1@
 ~Group users list keyboard reference~@UsersKeys2@

@Rights
$ #User Manager - account rights#

 In the root of the account rights panel you can see a list of account rights
represented as folders. When you enter into a specific account right, you'll
see a list of all users and groups that have this account right assigned to
them.

@UsersKeys1
$ #User Manager - users and groups list keyboard reference#

 Switch between local and global users and groups                 #F3#
 Show user or group properties                                    #F4#
 Add an user                                                #Shift-F4#
 Add selected users and groups to the access control list
 on the passive panel                                             #F5#
 Add the user or group under cursor to the access control
 list on the passive panel                                  #Shift-F5#
 View users and groups on a remote machine over the network       #F6#
 Return to the local machine                                #Shift-F6#
 Add a group                                                      #F7#
 Delete selected users and groups                                 #F8#
 Delete the user or group under cursor                      #Shift-F8#

@UsersKeys2
$ #User Manager - group users list keyboard reference#

 Show user properties                                             #F4#
 Add an user                                                #Shift-F4#
 Add selected users to the access control list on the
 passive panel                                                    #F5#
 Add the user under cursor to the access control list
 on the passive panel                                       #Shift-F5#
 Remove selected users from the current group                     #F6#
 Remove the user under cursor from the current group        #Shift-F6#
 Delete selected users                                            #F8#
 Delete the user under cursor                               #Shift-F8#

@ACL
$ #User Manager - access control list#

 In various places of the plugin you can see an access control list as shown
below:

 \1B…ÕÕÕÕÕÕÕ\30 ACL:doll001.png\Audit \1BÕ—ÕÕÕÕÕÕ\-
 \1B∫\1Ex\1B            \1EName\1B              ≥\1EAudit\1B \-
 \1B∫\30..                             ≥      \-
 \1B∫*.ZG\\O                         ≥RWX   \-
 \1B∫+.ZG\\K                         ≥RWX   \-
 \1B∫-.ZG\\Administrator             ≥FULL  \-

 The symbol in the first position denotes the type of the access control
entry.

 #+# - allowed or succeeded entry.
 #-# - denied or failed entry.
 #*# - succeeded and failed entry.

 Keyboard reference:

 Change access control entry type                                 #F3#
 Edit access control entry in common form                         #F4#
 Edit access control entry in advanced form                   #Alt-F4#
 Delete selected access control entries                           #F8#
 Delete the access control entry under cursor                #Shift-8#
 Move the access control entry one position up                #Alt-Up#
 Move the access control entry one position down            #Alt-Down#

@ManageGroup
$ #User Manager - manage group#

 Here you can create a new or edit an existing user group.

@EditShare
$ #User Manager - edit share#

 Here you can change the comment and the maximum allowed connections of a
share.

@NewShare
$ #User Manager - new share#

 Here you should enter a name for the new share.

@EditAdvancedAccess
$ #User Manager - edit access permissions in the advanced mode#

 Here you can edit permissions in the advanced mode.

 Select all permissions                               #Shift-<Gray +>#
 Deselect all permissions                             #Shift-<Gray ->#

@EditCommonAccess
$ #User Manager - edit access permissions in the common mode#

 Here you can edit access permissions in the common mode.

@Config
$ #User Manager - configure#

 In this dialog you may change the following options:

  #Add to Disks menu#      Enables adding the "User manager" plugin
                         entry to the disks menu.

  #Disks menu hotkey#      Hotkey for the user manager entry in
                         the disks menu. You can enter a value
                         from 1 to 9 or leave this field empty
                         to autoassign.

  #Add to Plugins menu#    Enables adding "User manager" plugin entry
                         to the plugins menu.

  #Add to Config menu#     Enables adding "User manager" plugin entry
                          to the configuration menu.

  #Full user names#        If this option is enabled, user names will
                         be shown as #BUILTIN\\Administrators#.

  #Command line prefix to# Specifies the prefix for opening the file,
  #start the properties#   folder, registry key or device properties
  #list#                   list from the command line.

@ProcessChildren
$ #User Manager#

 Here you can setup options for the ~apply to child objects~@Apply2children@
operation.

 #Objects# - here you can select, what objects will be processed.

 #ACL types# - here you can select, which ACL types will be changed in the
processed objects.

@About
$ #User Manager - about#

 Copyright (c) 2001-2003 by ZG.

 E-mail: ~zg@@matrica.apollo.lv~@mailto:zg@@matrica.apollo.lv?subject=User Manager plugin@
