package require Tk

wm title . "PoulpeEdit : game level editor"

option add *tearOff 0

source [file join [file dirname [info script]] styles.tcl]
source [file join [file dirname [info script]] app_menu.tcl]

ttk::frame .c

grid .c -column 0 -row 0
grid columnconfigure . 0 -weight 1; grid rowconfigure . 0 -weight 1
grid columnconfigure . 1 -weight 1; grid rowconfigure . 1 -weight 1
grid columnconfigure . 2 -weight 1; grid rowconfigure . 2 -weight 1

source [file join [file dirname [info script]] stats.tcl]
source [file join [file dirname [info script]] render_view.tcl]
source [file join [file dirname [info script]] explorer.tcl]
source [file join [file dirname [info script]] level.tcl]
source [file join [file dirname [info script]] options.tcl]
