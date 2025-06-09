package require Tk

wm title . "PoulpeEdit : game level editor"

option add *tearOff 0

menu .menubar
. configure -menu .menubar

set m .menubar
menu $m.file
menu $m.edit
$m add cascade -menu $m.file -label File
$m add cascade -menu $m.edit -label Edit

$m.file add command -label "Close" -command "close_plp_edit"

grid [ttk::frame .c -padding "3 3 12 12"] -column 0 -row 0 -sticky nwes
grid columnconfigure . 0 -weight 1; grid rowconfigure . 0 -weight 1

grid [ttk::button .skybox_button -text "bluesky skybox" -command "update_skybox"] -column 0 -row 0 -sticky w
grid [ttk::button .test_callback -text "Test C++ callback" -command "call_back"] -column 1 -row 0 -stick w

proc call_back {} {
  puts [test_callback arg1 arg2]
}

proc update_skybox {} {
  puts [test_update_skybox]
}

proc close_plp_edit {} {
  exit 0
}