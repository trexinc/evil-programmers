m4_include(`fm_version.m4')m4_dnl
.Language=English,English
.PluginContents=FARMail

@Contents
$^`#'FARMail `v'MAJOR.MINOR beta BETA#

 - ~About FARMail~@About@
 - ~How to use~@How@
 - ~Mailbox panel~@MailBoxPanel@
 - ~Mailbox settings~@MailboxSettings@
 - ~POP3 messages panel~@POP@
 - ~IMAP4 messages panel~@IMAP@
 - ~Mail sending options~@SendOptions@
 - ~Header information in message~@Header@
 - ~FastDownload~@FastDownload@
 - ~FastExpunge~@FastExpunge@
 - ~Plug-in commands~@UsingPlugins@
 - ~Program settings~@PluginSettings@
 - ~FAQ~@FAQ@
 - ~Technical information~@Tech@
 - ~Authors~@Authors@
 - ~License~@License@



@About
$^#About this program#

  FARMail is a FAR plug-in that allows You to work with electronic mail messages
from FAR. Its two main purposes are:

 - to administrate messages on POP3 and IMAP4 servers, that is to view,
delete and download messages for later offline processing;

 - to quickly send messages or files via SMTP without quitting FAR.

  Messages downloaded from POP3(IMAP4) servers are saved "as is", without any
processing. Outgoing messages can be sent "as is", encoded in base64 or
attached.



 ~Contents~@Contents@

@How
$^#How to work with FARMail#

 #Installation#
 #ÄÄÄÄÄÄÄÄÄÄÄÄ#

  1. Create subdirectory "FARMail" in "Far\Plugins\" directory.
  2. Copy files from FARMail archive to this subdirectory.
  3. Restart FAR.

 #Work#
 #ÄÄÄÄ#

  To start work You must at first ~enter valid mailbox(es)~@MailboxSettings@.
After that every time You select FARMail from plug-ins menu (or from Disks
menu), You will see a list of all mailboxes represented as folders in FAR file
panel. To select a mailbox simply "enter" it, that is - press ENTER (Note: You
must be connected to Internet). If You didn't specify Your login and/or
password, You will be prompted to enter them. If You work with IMAP4 server,
You will see a list of folders in Your mailbox, select the one You want. Then
You will see a list of messages on server, represented as files with names
starting from 1 and extensions specified in FARMail settings (".msg" by
default). You can see also message size, sender, date and subject. Now You can
view, move, copy or delete messages as files. There are also some special
functions, which will be described later.

  To send a message or file select files You want in a file panel
and copy (or move) file(s) to FARMail panel by pressing F5 or F6. It's not
necessary to enter a mailbox, mail will be sent automatically if You have
only one mailbox, otherwise You can select a mailbox from popup menu.
After a standard FAR prompt You will see a more complex dialog with
~mail sending options~@SendOptions@. Fill all necessary fields and press "Ok".
Your message will be sent. Of course You will not see it in mailbox panel.
Note: only files can be sent, not folders.

  If You send more than one file, You will be asked if You want to send
all files in one message. In this case all selected files will be sent attached
to one message.

  You can also send a message from FAR internal editor. Just select FARMail
from plug-ins menu, then select "Send message". Choose an appropriate server,
recipient etc. You can also write Your own scripts for new message
and reply or other functions you need using the Scripts sub plug-in.

  Another helpful features are:
  - ~FastDownload~@FastDownload@ - which allows You to quickly download
(copy/move) all messages from all the selected mailboxes.
  - ~FastExpunge~@FastExpunge@ - which allows You to quickly empty the
selected mailboxes.



 ~Contents~@Contents@


@MailBoxPanel
$^#Mailbox panle#

 This panel shows you all the mailboxes you have defined and allows you to
manage them. At the keybar you can see what functions are available in this
panel (FastDownload, Statistics and so on).



 ~Contents~@Contents@


@MailboxSettings
$^#Mailbox settings#

  To edit existing mailbox, press F4 on selected mailbox. To enter new
mailbox, press Shift-F4 in FARMail panel. To delete selected mailbox(es),
press F8.

"Mailbox options" dialog looks like this:

 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Mailbox options ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º  Mailbox       : Vasya Pupkin's mailbox                      º
 º  User Name     : Vasya Pupkin                                º
 º  E-Mail        : vasya@@hell.org                              º
 º  Organization  : Vasya & Co Ltd                              º
 º  [ ] Default mailbox                                          º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º  SMTP          [X] Needs authentication                       º
 º   Ã Server   : mail.hell.org                     Port:25     º
 º   À Login    : vasya       Password: ******       [ ] SSL     º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º  () POP3 / ( ) IMAP4                                         º
 º   Ã Server   : pop.hell.org                      Port:110    º
 º   Ã Login    : vasya                              [ ] SSL     º
 º   Ã Password : ******                                         º
 º   Ã Activity maintenance : 0 min.                             º
 º   Ã Keep [ ] messages' state (UIDL)                           º
 º   À Minimal TOP value: 0    lines                             º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º                      [ Ok ]  [ Cancel ]                       º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

Here You must enter:

#Mailbox# - Mailbox name, must be unique (Required filed);

#User Name# - User name, which will be used in outgoing messages;

#E-Mail# - User email address for outgoing messages (Required filed);

#Organization# - Organization name;

#Default# - If checked, this mailbox will be selected by default
in the mailboxes menu while sending;

#SMTP server# - SMTP server address and port (usually 25) (Required filed);

#Needs authentication# - turn this on if Your SMTP server needs
authorization before sending mail;

#Login# - Your login for SMTP server;

#Password# - Your password for SMTP server;

#SSL# - Check if SMTP or POP3/IMAP4 server supports SSL connection (available
only in FARMail versions compiled with SSL support).

#POP3/IMAP4 server# - POP3 or IMAP4 server (may be the same as SMTP server) and port
(usually 110 for POP3 and 143 for IMAP4);

#Login# - Your login for POP3(IMAP4) server;

#Password# - Your password for POP3(IMAP4) server;

#Activity maintenance# - If more than 0, empty commands will be sent
to incoming mail server with this period.

#Keep messages' state (UIDL)# - Keeps states of messages (read, new & marked)
between connections. Convinient when you work directly with the POP3 server.

#Minimal TOP value# - If you have a buggy POP3 server (like mail.firemail.de) that
returns the whole message on "TOP 0" (instead of the header) then you should set
this value above 0 (this value will be used to get the message header).

  If login information is not present, You will be prompted for it on
connection. E-Mail must be in format "name@@domain" only.
Password is saved in registry slightly encrypted, but it's better
not to enter it if You are not sure that nobody else can work on Your
computer. Unencrypted passwords entered in version 1.0 are also valid,
they will be encrypted automatically first time You change something
in the mailbox. Note that it's not necessary to select a default mailbox.



 ~Contents~@Contents@


@FastDownload
$^#FastDownload#

  With this feature You can quickly download all messages from the selected
mailboxes.

  FastDownload can be executed in two ways:

  #1#. From FARMail main panel:
  When in main plug-in panel by pressing F5 - Copy or F6 - Move you will be
presented with a dialog asking you to where you want to download the messages
from the selected mailboxes. In this dialog You can choose between downloading
the messages to the Inbox file if such is defined (see
~Files and directories~@FilesDirs@) or to the specified directory. After
Okaying this dialog the messages will be downloaded and if you chose to move
they will also be deleted from the server.

  #2#. From the command line:

  The syntax is as follows:
  to copy: mail:fdc:"mailbox name" [dir]
  to move: mail:fdm:"mailbox name" [dir]

  Download messages from the specified mailbox to the directory "dir". If
"dir" isn't specified, downloading is done to the Inbox file if such is
defined (see ~Files and directories~@FilesDirs@).
  "dir" can be quoted.
  "mailbox name" must be quoted if it contains quotes or spaces. After quoting
it, all inside quotes should be prepended with "\" (e.g. "\"inside\"").

  #Note#: While this feature works both with POP3 and IMAP4 servers, on IMAP4
servers only the contents of the "INBOX" directory will be downloaded.



 ~Contents~@Contents@


@FastExpunge
$^#FastExpunge#

  With this feature You can quickly delete all messages from the selected
mailboxes.

  FastExpunge can be executed in two ways:

  #1#. From FARMail main panel:
  When in main plug-in panel by pressing Shift-F8 you will be presented with a
dialog asking you to confirm the action. After Okaying this dialog all the
messages in all the selected mailboxes will be deleted.

  #2#. From the command line:

  The syntax is as follows:
  mail:fx:"mailbox name"

  Delete all messages from the specified mailbox.
  "mailbox name" must be quoted if it contains quotes (e.g. ""name"").

  #Note#: While this feature works both with POP3 and IMAP4 servers, on IMAP4
servers only the contents of the "INBOX" directory will be deleted.



 ~Contents~@Contents@


@POP
$^#POP3 messages panel#

  This panel contains messages names and some information about these
messages. When You enter a mailbox, You automatically lock it. So You can't
see any new mail until You reconnect. Also any other mail clients can't
access locked messages. Message numbers don't change until You leave the
server. While connected, You can undelete all erased messages by pressing
F7. But this option works only before You leave server! When You leave, all
changes will be saved. You can also reconnect to server without leaving
panel by pressing Shift-F7 or Ctrl-R. Use Shift-F1 to edit address book,
Shift-F2 and Shift-F3 to edit/view only first lines of a message. You can
also run a Filter plug-in if such is installed by pressing Alt-F6.

  When You copy messages on disk, You can turn on the option "Assign
unique names". In this case files with same names will not be overwritten but
saved with a unique name.



 ~Contents~@Contents@


@IMAP
$^#IMAP4 messages panel#

  When You enter an IMAP4 mailbox, You see a list of all folders in Your
mailbox. You can create (F7), delete (F8) and rename (F6) folders
There can be a hierarchy of folders, but in this version all folders are
represented as a flat structure. There must be at least one folder INBOX,
which is the default folder for incoming mail.

  In the folders' panel there are fields:

#Folder name# - that is the name of the folder :)

#Fl# - this means "flags". If the folder is "marked" by server, You'll
see a "!" symbol. "Marked" means that the folder probably contains messages
that have been added since the last time the folder was selected.
Note that some servers don't give this information. If the folder
can't be selected, You'll see a "N" symbol.

#Ttl# - total number of messages in folder;

#Rcn# - number of recent messages;

#Uns# - number of unseen messages;

  Note that if Your server don't support IMAP4rev1 protocol, number of
messages will be always 0.

  When You select a folder, You get a list of all messages in the
folder. This list is similar to the list of messages on POP3 server.
But the feature of IMAP4 is that multiple mail clients can access
the same folder simultaneously. So messages can be deleted by other
mail clients, or new mail may come. In this case messages are
renumbered. On IMAP4 servers You can use the Ctrl-R
sequence to refresh message list without reconnecting.

  You can assign unique numbers while copying messages on disk - just
like with POP3 servers. You can run a filter plug-in if such is installed
by Alt-F6. To copy (move) message(s) to another folder on the same server,
press Shift-F5 (Shift-F6).

  You can also resume broken message download. To do this, turn on
the appropriate option in program settings. After that all downloaded
messages will be saved on disk, even if only a part of message was
downloaded. Broken messages will have the "hidden" attribute and a
special header field with necessary information. Then You can connect
again to Your server and copy this message onto itself (Note: don't
use unique names assign, and don't copy more than 1 message! This
feature works only when You copy 1 message onto itself. Also don't
change incomplete message!). You will be asked to resume or overwrite
message. Select "resume" and FARMail will try to resume download.



 ~Contents~@Contents@


@SendOptions
$^#Mail sending options#

 This is an example of send dialog:

 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Sending as message: SomeText.msg ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Sender    : Bob Dylan <bd@@music.org>                         º
 º Recipient : John Doe <jd@@hell.org>                       (?) º
 º CC        : dude@@serv.com,two@@serv.com                   (?) º
 º BCC       :                                              (?) º
 º Subject   :                                                  º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Send      : () Text as is (7 bit)     ( ) HTML as is         º
 º             ( ) Text in Base64         ( ) HTML in Base64     º
 º             ( ) Attached file          ( ) Extended format    º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Original text charset:       ³ Encode to:                     º
 º  1( ) OEM/Undefined          ³ 6( ) koi8-r                    º
 º  2() US-ASCII               ³ 7( ) Windows-1251              º
 º  3( ) koi8-r                 ³ 8() Do not encode             º
 º  4( ) Windows-1251           ³ 9( ) ISO-8859-5               º
 º  5( ) ISO-8859-5            ³                                º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º          [ Attached files: 0 ]  [ User headers: 0 ]           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º         [ Ok ]  [ Cancel ]  [ All ]  [ Analyze all ]          º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

#Sender# - User name and email of the sender, You, taken from the selected
mailbox settings. You can change it to whatever you want.

#Recipient# - User name and email of person You write to, must be in form User
<name@@domain>, name@@domain or <name@@domain>. You can select a user from
history list or from the Address book plug-in. You can enter multiple
recipients, divided by ",";

#(?)# - Select recipient(s) from Address book plug-in if such is installed;

#CC, BCC# - Carbon copy and blind carbon copy fields. Note that You'll
see these fields only if the appropriate checkbox in plug-in settings
is selected or you can temporarily activate them by pressing Ctrl-Shift-C.
You can enter multiple recipients, divided by ",";

#Subject# - Message subject;

#Send# - Here You can select the way Your message is sent.
FARMail tries to choose the appropriate method. Possible options are:
    1. #Text as is# - this means that file will be sent
   "as is", without any modifications. This must be a
   plain text file, with 7-bit or 8-bit characters
   (detected automatically). Use this option if You send a
   text file in 7-bit charset, or a text file in 8-bit
   charset and You are sure Your mail channel can transfer
   8-bit messages without modification.
    2. #Text in Base64# - this means that the file You send
   is a plain text file, but it can be modified on its way
   to recipient (for example, if it contains 8-bit
   characters etc). So the text will be encoded.
    3. #Attached file# - choose this option if the file You
   send is not a plain text file.
    4. #HTML as is# - choose this option if You send a
   message in form of HTML file.
    5. #HTML in Base64# - choose this option to send a
   HTML-message, encoded in Base64.
    6. #Extended# - for advanced users only.
   This works exactly as "Text as is" but permits You to have
   extended control of message structure. Meaning that the
   message body will be sent immediately after the header,
   without the separating empty line. The header generated by
   FARMail will contain only from,to,cc,subject,reply-to,
   x-mailer,date,mime-version and organization fields.

#Original text charset# - This is Your message encoding. FARMail tries to
detect it automatically if You have character frequency distribution table
installed or if the message is sent from the editor then the current editor
encoding is used. Your message's header will have the appropriate "charset"
parameter, and all the fields will be decoded if necessary. When changing this
option all the unchanged fields will be redecoded using the newly selected
charset as to appear correct on the screen, if for some reason You want to
redecode an already changed field You can set him unchanged by pressing Ctrl-L
in that field.
Possible values are:
    1. #OEM/Undefined# - this means that Your message is not
   in KOI8-R, Windows-1251 or US-ASCII charset, so the
   charset parameter will not be used. Choose this option
   for attached files or text files in code pages other
   then US-ASCII, KOI8-R or Win-1251 (OEM, for example).
    2. #US-ASCII# - this is only for 7-bit text messages.
    3. #koi8-r# - choose this option if Your message is
   written in KOI8-R encoding. An appropriate "charset"
   will be used.
    4. #Windows-1251# - choose this option if Your message
   is written in Windows code page. An appropriate "charset"
   will be used.
    5. If You have defined additional encodings
   in program settings, You can select here another charset.
   #Note:# Fields 3, 4 and 5 can be adjusted to Your liking.
           See ~Encodings~@Encodings@.

#Encode to# - this option shows how to encode Your message if
necessary. This may be useful, for example, if Your
message is in OEM codepage, and You want to encode it
in KOI8-R. Possible values are:
    1. #koi8-r#. Your message will be encoded to KOI8-R.
    2. #Windows-1251#.Your message will be encoded to
   Windows codpage.
    3. #Do not encode#. Select this option if You want to
   send Your message "as is".
    4. If You have defined additional encodings
   in program settings, You can select here another charset.
   #Note:# Fields 1, 2 and 4 can be adjusted to Your liking.
           See ~Encodings~@Encodings@.

  If You send multiple files in one message, this dialog will be more simple,
there will be only sender, recipients' addresses, subject and encoding. This
is because all files are sent attached to message and there is no text body to
encode. The "Encode to" setting is used to encode the "header" fields.

  Press "Ok" to proceed, "Cancel" to stop operation. If You send more than one
file, You will see also an "All" and "Analyze all" buttons. If You press
"All", all next files will be sent the same way. Choose "All" only if You are
sure You send identical files! If you press "Analyze all", every file will be
processed and sent separately.
  Note (for russian users): You must install character distribution table,
KOI8 and Windows-1251 codepages in order to work with different codepages.
FARMail searches for "KOI-8" and "CP-1251" substrings in these names. If on
Your computer their name is different or you want to use other codepages
instead, You can configure them in ~Encodings~@Encodings@.



 ~Contents~@Contents@


@Header
$^#Header information in message#


  If You don't want to enter a lot of information each time You send a
message, You can fill some necessary fields in message text. Just
write at the beginning of the first line:

 #%start%#

 And all next lines will be processed like a block of technical information,
up to the line:

 #%end%#

This block will not be sent to recipient. Inside it You can specify:

1. Recipient's address and message subject. The syntax is: #set <variable>=<value>#.
There are only 6 possible variables :-) You can write:

  set %from%  = <sender's address>
  set %reply% = <reply-to address>
  set %to%    = <list of recipients>
  set %cc%    = <list of carbon copy recipients>
  set %bcc%   = <list of blind carbon copy recipients>
  set %subj%  = <message subject>
  set %org%   = <organization>

2. The way the message will be sent. The syntax is: #set <variable>=<value>#.
There are only 3 possible variables:

  %how% - Can be any value between 1 and 6. It specifies the encoding method
for the message (e.g. 1 - Text as is, 3 - Attached file, 6 - Extended format).
  %charset% - Can be set to any of the defined charsets or empty for
OEM/Undefined. Selects the encoding in which the message is written (e.g.
koi8-r, us-ascii)
  %encodeto% - Can be set to any of the defined charsets or empty for not
encoding. Selects the encoding in which the message will be sent (e.g.
koi8-r).

3. Additional header lines. The syntax is: #header <field> = <value>#. You can
use defined variables %to%, %cc% etc. in header field. You can also create
header items that will span over several lines, by using the #appendheader#
directive. For example, such a construction:

 %start%
  set %to% = Vasya Pupkin <vasya@@hell.org>
  header X-Comment = line 1
  appendheader line 2
  appendheader line 3
  header X-MyComment-To = (sent to %to%)
 %end%

 will result in :

 ...
 To: Vasya Pupkin <vasya@@hell.org>
 X-Comment: line 1
  line 2
  line 3
 X-MyComment-To: (sent to Vasya Pupkin <vasya@@hell.org>)
 ...

4. Attached files. The syntax is: #attach <full filename>#. The file must
exist of course :-) Example: attach c:\myfile.exe.
If You want the attached files to have a content-id, for inside message linking
abilities. You need to specify the #related# directive somewhere in the
%start%-%end% block. The content-id of files attached this way will be equal
to their filenames.

5. All other lines are ignored. But if You want to put some comments,
it's a good idea to begin them with a ';' symbol.

You can redefine %start% and %end% directive names by indicating new names in
Windows registry, key HKCU\Software\Far\Plugins\Farmail\Common\StartBlock
and HKCU\...\EndBlock.

Note that to use this feature, You must turn on an appropriate option
in program settings.



 ~Contents~@Contents@


@UsingPlugins
$^#Plug-in commands#

 Starting with FARMail v3.0 you can use external modules (plug-ins) to extend
program capabilities. Currently available modules that are shipped with
FARMail are:
 1. ~Address Book~@<FMP\AddressBook\>@ - A simple address book to keep recipients' e-mails in.
 2. ~Filter~@<FMP\Filter\>@ - Spam filter.
 3. ~Helper~@<FMP\Helper\>@ - Assists in different mail writing tasks.
 4. ~Scripts~@<FMP\Scripts\>@ - A scripting language to assists You with mail processing.


 #Installing plug-ins#

 Create a folder named "FMP" in the folder where You installed FARMail. Then
create there a sub folder for every plug-in You want to install.

 A file with extension "fmp" is the module itself, a file with extension "fml"
is the language file for the module and files with extension "hlf" are the
help files for the module.



 ~Contents~@Contents@


@PluginSettings
$^#Program settings#

 - ~Interface settings~@Interface@
 - ~Files and directories~@FilesDirs@
 - ~Connection~@Connection@
 - ~Encodings~@Encodings@
 - ~States~@States@
 - ~Other~@Other@
 - ~Plug-ins configuration~@PlugIns@



 ~Contents~@Contents@


@Interface
$^#Interface settings#

 ÉÍÍÍÍÍÍÍÍÍ FARMail options: interface ÍÍÍÍÍÍÍÍÍ»
 º [x] Add to Disks menu                        º
 º   1 Disk menu hotkey ('1'-'9')               º
 º [x] Add to Plug-ins menu                     º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Select panel to edit panel modes for:        º
 º Mailboxes                                   º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º  Mode 0   ³ Column types                     º
 º  Mode 1   ³ N,C0                             º
 º  Mode 2   ³ Column widths                    º
 º  Mode 3   ³ 20,0                             º
 º  Mode 4   ³ Status line column types         º
 º  Mode 5   ³                                  º
 º  Mode 6   ³ Status line column widths        º
 º  Mode 7   ³                                  º
 º  Mode 8   ³                                  º
 º  Mode 9   ³ [ ] Fullscreen                   º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º              { Ok }  [ Cancel ]              º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

#Add to Disks menu# - Check this option if You want to see
FARMail in Disks menu;

#Disk menu hotkey# - It's a "disks" menu hotkey :) Enter
'0' to autoassign;

#Add to Plug-ins menu# - Check this option if You want to see
FARMail in plug-ins menu in panels;

#Edit panel modes# - Here You can choose what columns You want to see in all
available FarMail panels. In this list You should select the panel type for 
which You want to configure the column types. There exist the following four
panel types:

	#Mailboxes#
	FarMail mailboxes panel

	#IMAP4 folders#
	IMAP4 folders panel

	#POP3 Mailbox#
	POP3 messages panel

	#IMAP4 Mailbox#
	IMAP4 messages panel

  For each panel mode you can set the following:        

  #Column types#

  In this field the type of shown columns is set. In accordance with the
type of the panel the following column name are available:

	#"Mailbox" panel#

	#N#  - mailbox name.
	#C0# - name of the POP3/IMAP4 server.

	#"IMAP4 folders" panel#

	#N#  - IMAP4 folder name.
	#C0# - IMAP4 folder flags.
	#C1# - total number of messages in the folder.
	#C2# - number of new messages in the folder.
	#C3# - number of unread messages in the folder.

	#"POP3 Mailbox" and "IMAP4 Mailbox" panels#

	#N#  - message file name.
	#S#  - message file size.
	#C0# - sender.
	#C1# - date sent.
	#C2# - subject.

	Also now You can use standard FAR column types, for example date and time
types (D,T,DM,DC,DA). Message date and time are stored as file creation,
modification and last access dates.

  #Column widths# - Widths of all columns, separated by ','.
Enter 0 to autoassign.

  #Status line column types# - Here You can choose what columns You want to see
in message panel status line. Enter column names separated by ',', the names
are the same as for "column types".

  #Status line column widths# - Widths of all columns in status line, separated
by ','. Enter 0 to autoassign.

  #Show panel in full size# - Check this option if You want to see messages
panel in full screen mode;



 ~Program settings~@PluginSettings@ ~Contents~@Contents@


@FilesDirs
$^#Files and directories#

 ÉÍÍÍÍÍÍÍÍÍÍ FARMail options: files and directories ÍÍÍÍÍÍÍÍÍÍÍ»
 º Fixed length of message name        : 3                     º
 º Message files extension             : msg                   º
 º Ú Incoming ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ º
 º ³[x] Assign new names to downloaded messages              ³ º
 º ³Format: %subj:80%                                        ³ º
 º ³[x] Save incoming messages to Inbox                      ³ º
 º ³Filename:  c:\download\mail\boxes\inbox.mbx              ³ º
 º ³[x] Take file date and time from message                 ³ º
 º ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ º
 º Ú Outgoing ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ º
 º ³[x] Save outgoing messages                               ³ º
 º ³( ) Directory: c:\download\mail\sent                     ³ º
 º ³() Outbox:    c:\download\mail\boxes\outbox.mbx         ³ º
 º ³[x] Save Message-Id of outgoing messages                 ³ º
 º ³Template:  250 Message received: %s                      ³ º
 º ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ º
 º                     [ Ok ]  [ Cancel ]                      º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

#Fixed length# - Fixed length of file names. File names will have leading
zeros if necessary.

#Message file extension# - Extension for messages on server, 'msg' by default;

#Assign new names..# - if You turn on this option, messages copied to Your
disk from server will have different names, based on format string below.
In this field You can define Your own name format using standard characters
and special variables:
%from[:length]% - the sender's name;
%subj[:length]% - message subject;
%date[:length]% - message date;
%curdate[:format(max 50)]% - current date where format is any string that may contain
the following keywords: D - 2 digit day of the month, M - 2 digit month of the year,
Y - 4 digit year, S - 2 digit short year. Note that sometimes subject and sender
names may be too long to fit in the maximum length of a filename, so it is
recommended to write the maximum possible variable length after a ':' symbol,
for example %from:50% etc.
As an additional feature You can specify folders in this field, for example:
%from:50%\%curdate:Y-M-D%\%subj:30% (%date%).

#Save incomming messages to Inbox# - if You turn on this option, messages
downloaded using the ~FastDownload~@FastDownload@ feature will be saved to the
specified mailbox file (Unix mailbox).

#Take file date...# - turn on this option to set file date and time
to message date and time (for incoming mail).

#Save outgoing messages# - turn on this option to save all outgoing
messages to the specified directory or mailbox file (Unix mailbox).

#Save Message-Id of outgoing messages# - Works only on SMTP severs that return
the Message-Id after sending the message. The #Template# field should contain
a template that FARMail will use to extract the Message-Id from server's
response, "%s" marks the location of Message-Id in the response string. To
check if Your SMTP server supports this feature see the response of Your
server, after a message has been sent, in FARMail logs.



 ~Program settings~@PluginSettings@ ~Contents~@Contents@


@Connection
$^#Connection#

 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ FARMail options: connection ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Connection timeout: 60  sec.                                  º
 º [ ] Write session log                                         º
 º Ú IMAP4 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ º
 º ³Quick view:         0    bytes                             ³ º
 º ³[x] Try to resume broken message download                  ³ º
 º ³IMAP4 mail folders:   "*"                                 ³ º
 º ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ º
 º Ú POP3 ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ º
 º ³Quick view:         0    lines                             ³ º
 º ³[ ] Display zero size messages                             ³ º
 º ³[ ] Disable TOP and LIST                                   ³ º
 º ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ º
 º                      [ Ok ]  [ Cancel ]                       º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

#Connection timeout# - Timeout period for blocking winsock operations.

#Write session log# - When turned on, FARMail will generate 3 log files
in its root directory: farmail.pop for POP3-sessions, farmail.imap for
IMAP4-sessions and farmail.out for SMTP sessions.

 #IMAP4:#

#Quick view# - this is the number of first bytes of message text to download
when using quick view or quick edit functions. If You set these values to 0,
only message header will be downloaded.

#Try to resume...# - if You turn on this option, incomplete messages
will be saved on disk with a possibility to resume download.

#IMAP4 mail folders# - a pattern (or a list of patterns divided by ';') with
folder names where mail can be found. Usually You don't need anything
but "INBOX", so that's the default setting. You can write here "*" to get
a list of all folders, "INBOX";"##news*" to get the INBOX folder and all folders
beginning with "##news" etc. Note that patterns must be in quotes.

 #POP3:#

#Quick view# - this is the number of first lines of message text to download
when using quick view or quick edit functions. If You set these values to 0,
only message header will be downloaded.

#Display zero size messages# - On some POP3 servers, after deleting messages
the server still lists them but with message size zero, so if you do not want
to display those none existing messages deselect this checkbox.

#Disable TOP and LIST# - With this option on, You will see no information
about the messages on the server, but the mailbox will be opened very quickly.



 ~Program settings~@PluginSettings@ ~Contents~@Contents@


@Encodings
$^#Encodings#

 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ FARMail options: encodings ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º           Charsets:                                                  º
 ºÚÄÄÄÄÂÄÄÄÄÂÄÄÄ                                                        º
 º³1()³A()³  OEM/Undefined | Do not encode                            º
 º³    ³    ³                                                           º
 º³    ³    ³ Frequently used:      FAR Tables:                         º
 º³2( )³B( )³  koi8-r                KOI-8 (E-Mail russian)            º
 º³3( )³C( )³  Windows-1251          CP-1251 (Windows Cyrillic)        º
 º³    ³    ³                                                           º
 º³    ³    ³ Additional:                                               º
 º³4( )³D( )³  ISO-8859-5            ISO 8859-5                        º
 º³5( )³E( )³  ISO-8859-2            ISO 8859-2                        º
 º³6( )³F( )³                                                          º
 ºÀÄÂÄÄÁÄÂÄÄÁÄÄÄ                                                        º
 º  ³    ÀÄÄÄÄ Default charset for Outgoing messages                    º
 º  ÀÄÄÄÄÄÄÄÄÄ Default charset for Incoming messages                    º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º                         [ Ok ]  [ Cancel ]                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

#Default charset# - For incoming mail: if message on server doesn't have a
"charset" or "X-Sun-Text-Type" field, default charset will be used; for
outgoing mail: default encoding in outgoing messages.

#Frequently used charsets# - here You can configure the default
encodings. Just write #correct# charset name (for example, koi8-r) in the
"Charsets" field, and select the FAR encoding table in "FAR Tables" field.

#Additional charsets# - here You can specify up to 3 additional encodings.
Just write #correct# charset name (for example, ISO-8859-5) in "Charsets"
field, and select the FAR encoding table in "FAR Tables" field.



 ~Program settings~@PluginSettings@ ~Contents~@Contents@


@States
$^#States#

 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ FARMail options: states ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º  New message                 ³ [x] Read only                º
 º  Read message                ³ [ ] Archive                  º
 º  Marked message              ³ [ ] Hidden                   º
 º  Deleted message             ³ [ ] System                   º
 º                              ³ [ ] Compressed               º
 º                              ³ [ ] Encrypted                º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º [ ] Use attribute highlighting                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º                     [ Ok ]  [ Cancel ]                      º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

 Here you can select what attributes will be set for messages with the above
states. New, read and marked states are available only in mailboxes with "Keep
messages' state" option checked.

 #Use attribute highlighting# - This option selects how Far will manage
highlighting of files in plug-in's panel. Normally it is done by file mask in
conjunction with attributes but can be done by attributes only (see Far's
help).



 ~Program settings~@PluginSettings@ ~Contents~@Contents@


@Other
$^#Other#

 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ FARMail options: other ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º [ ] Use CC/BCC fields                                       º
 º [x] Search for header information in message                º
 º [x] Confirm binary                                          º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º                     [ Ok ]  [ Cancel ]                      º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

#Use CC/BCC fields# - select this option if You want to use CC and BCC fields
in the send dialog.

#Search for header...# - turn on this option if You want FARMail to
check for %start%..%end% block in Your outgoing messages.

#Confirm binary# - turn on this option if You want to be warned before sending
a binary file or a file containing non-text symbols from the editor or from the file panel
when header information was found in the file being sent. The warning will appear before
the mail sending dialog.  



 ~Program settings~@PluginSettings@ ~Contents~@Contents@


@PlugIns
$^#Plug-ins configuration#

 Here you see the list of FARMail sub plug-ins that You can configure.



 ~Program settings~@PluginSettings@ ~Contents~@Contents@


@FAQ
$^#Frequently asked questions#

 Q: I can't connect to my server..
 A: Did You enter all the information correctly?

 Q: I receive strange errors while connecting..
 A: Don't use plug-in versions before 1.2.

 Q: But it still doesn't work!!
 A: Hm.. Ok, so we must check the session log. Turn on the
    appropriate checkbox in settings. You will get 3 files in
    plug-in's directory- farmail.out for SMTP sessions, farmail.pop
    for POP3 sessions, farmail.imap for IMAP4 sessions. May be they
    contain the answer.

 Q: Many people work on my PC and I don't want to let them check my
    mail!
 A: Simply don't enter Your password in mailbox settings.

 Q: Is it hard to decode my password by someone else with access to
    my registry?
 A: No, absolutely not!! What is more, there are more simple ways
    to find out the real password than decoding.. I warn You one
    more time: DO NOT SAVE YOUR PASSWORD IN REGISTRY IF YOU'RE NOT
    SURE YOUR COMPUTER IS INACCESSIBLE TO ANYBODY ELSE!

 Q: Why must I connect to incoming mail server to send a message?
 A: There is no need. You may send a message to the mailboxes
    panel without connecting to POP3 or IMAP4 server. You may also
    send Your message from FAR editor.

 Q: I was reading a mail for some minutes and when going back to
    messages panel I receive a "data transmission error" message..
 A: Enter an appropriate activity maintenance period in mailbox
    settings. Or simply reconnect.

 Q: When I enter an activity maintenance period for a mail server
    on localhost, my computer hangs up..
 A: It seems You have an old plug-in version.

 Q: When I delete a message on IMAP4 server, all next messages
    change their numbers, but on POP3 servers it works ok!
 A: Yes, renumbering (and multiple clients access) is a feature
    of IMAP4.

 Q: So, if I have 2 messages, after deleting message number 1,
    message number 2 will become number 1??
 A: Right.

 Q: But if any other mail client deletes message number 1, which is
    from spam@@suckerfinder.org, then I try to delete the same
    message number 1 - I'll delete my second, very important message
    from president@@mycountry.gov????!!!!!
 A: No, messages are deleted by their UIDs, not numbers. And message
    UIDs are read every time You refresh messages panel. So if
    You see a message from spam@@suckerfinder.org on the panel, You
    can delete it without hesitating :)

 Q: Charset autodetection doesn't work!
 A: Do You have language distribution table installed? (Dist.Rus.reg
    for russian users)

 Q: I can't change my message's encoding, I receive message
    "selected charset not installed"!
 A: Check out "..\far\addons\tables" directory.

 Q: I still receive this &%&%ing message!!
 A: Maybe you need to configure the default encodings. You can do
    it from plug-in's configuration dialog.

 Q: How can I extract attached files or read text in base64 or
    quoted-printable with this plug-in?
 A: No way. You can use external mail viewers or import messages
    in Your favorite email program.
    A FAR plug-in mail viewer can be downloaded from
    ~http://trexinc.sourceforge.net~@http://trexinc.sourceforge.net/mailview.php@.
    A console mail viewer can be downloaded from
    ~http://alsea.euro.ru~@http://alsea.euro.ru@.

 Q: I found a BUG!!
 A: Check Your settings, read this help file :)

 Q: &%&%, I found a BUG!!
 A: Do You have the last version? Check out the my webpage for
    latest info.

 Q: Yes, I have the last version and it has a lot of bugs!
 A: Well, send me a detailed description, log files, FAR and plug-in
    versions, operating system name.


 ~Contents~@Contents@


@Tech
$^#Technical information#

  #1. POP3 connection.#

  Each time You "enter" a mailbox, POP3 transaction is established. Program
sends USER and PASS commands. When You leave a mailbox, QUIT command is sent.
STAT, LIST and TOP are used to retrieve messages list, RETR to download
messages, DELE to delete and RSET to undelete messages. TOP is used also for
quick view/quick edit. UIDL is used to keep track of messages states. Well,
nothing special :)

  #2. SMTP connection.#

  SMTP session is established each time You send a message. Program sends
HELO command with name of the local host (returned by gethostname() function)
or EHLO and AUTH if the server needs authorization. PLAIN and LOGIN SASL
mechanisms are supported.
Then MAIL command is sent with E-Mail from mailbox settings as parameter,
RCPT with recipient's email, DATA with message body and QUIT. FARMail
generates message header with fields:

  Date                     : current date and time;
  From and Reply-To        : user name and email;
  To                       : recipients' names and e-mails, divided by ",";
  CC                       : carbon copy recipients, divided by ",";
  BCC                      : this field is not sent;
  Subject                  : message subject;
  X-Mailer                 : FARMail :)
  MIME-Version             : 1.0
  Content-Type             : text/plain for text messages "as is"
                             and in Base64 encoding;
                             text/html for HTML files;
                             application/octet-stream; name=.. for
                             attached files;
                             multipart/mixed when sending multiple
                             files in one message;
                             charset parameter is equal to
                             "us-ascii", "Koi-8r", "Windows-1251"
                             or not used;
  Content-Transfer-Encoding: 7(8)bit for plain text; Base64;
  Content-Disposition      : attachment; filename=.. only for
                             attached files
  Organization             : Organization name (if any);

  Message body for plain text 7/8 bit is generated directly from file,
but lines of text are no more then 1000 characters. If first character
in line is '.', this character is duplicated. So, if You send an
executable file, for example, "as is", its code will be corrupted.
Base64 encoding generates 76 symbols in one line.


  #3. IMAP4 connection.#

  FARMail actually uses features of IMAP4rev1 protocol, like STATUS
command. Correct work with "pure" IMAP4 servers is not guaranteed. Sessions begin
with LOGIN command, then FARMail sends CAPABILITY and checks for "IMAP4"
response, not "IMAP4rev1", because some "IMAP4"-servers implement IMAP4rev1
features. LIST and STATUS requests are used to build a list of folders,
SELECT enters a folder, FETCH is used to get message information, STORE
and EXPUNGE to delete messages. Note that all actions except deleting use
message numbers, but deleting uses message UIDs. Quick view/edit functions
use FETCH command with BODY[TEXT]<0.n> argument.


  #4. Unix Mailbox file format used in FARMail#

  FARMail uses the simplest format of Unix Mailbox, where the start of every
message is marked with "From<space><crlf>", the word From followed by a
space followed by a new line. The messages are written "as is" without any
processing besides adding ">" to every line that starts with "From<space>".
New messages are appended at the end of the mailbox file.



 ~Contents~@Contents@


@Authors
$^#Authors#

  Currently the development of this plug-in is maintained by FARMail Group:

  #Alex Yaroslavsky#
  Homepage : ~http://code.google.com/p/evil-programmers/~@http://code.google.com/p/evil-programmers/@

  #Vadim Yegorov#
  E-mail   : at bmg.lv / zg
  Homepage : ~http://zg.times.lv/~@http://zg.times.lv/@

  In the years of 1999 and 2000 the plug-in was developed by its original
author:

  #Serge Alexandrov#
  E-mail  : at zmail.ru / poseidon
  Homepage: ~http://alsea.euro.ru/~@http://alsea.euro.ru/@
            ~http://dpidb.genebee.msu.ru/users/poseidon/~@http://dpidb.genebee.msu.ru/users/poseidon/@



 ~Contents~@Contents@

@License
$ #License#

 FARMail plugin for FAR Manager
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
