package require Tk

. configure -bg "#ff00ff" 

wm withdraw .
wm overrideredirect . 1
wm attributes . -topmost 1
wm title . "Main UI"
wm geometry . 400x300+$game_menu_x+$game_menu_y
wm attributes . -transparentcolor "#ff00ff"
wm attributes . -alpha 0.5
wm deiconify .

option add *tearOff 0

source [file join [file dirname [info script]] styles.tcl]

ttk::frame .c

grid .c -column 0 -row 0
grid columnconfigure . 0 -weight 1; grid rowconfigure . 0 -weight 1

source [file join [file dirname [info script]] main_ui.tcl]

proc update_ui_pos {x y} {
  set ui_x [expr {$x + 20}]
  set ui_y [expr {$y + 50}]

  wm geometry . 400x300+$ui_x+$ui_y
  update
}