It is recommended to assign macros to the frequently used plugin commands,
such as background copying/moving, viewing info, etc.
"Macros" folder contains batch files with self-explaining names. They are
intended to set up the macro sequences for executing plugin commands.

1. Assign hotkey "b" to the Background Copy plugin using F4 key in the
plugins list.

2. Run necessary .cmd-files. For example, BGmove.cmd assigns background
move command to CtrlShiftF6 key. You can assign macro to another key (f.e.
CtrlAltF12) by executing "BGmove.cmd CtrlAltF12" (without quotes). We
recommend to start these files by FAR because its language settings and
current user (if /u parameter was used to start FAR) will be taken into
account in this case.

3. Restart FAR to apply changes.
