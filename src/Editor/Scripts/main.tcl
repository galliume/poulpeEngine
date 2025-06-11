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

set grid_padx 2
set grid_pady 2
set grid_border_width 1
set grid_relief ridge
set grid_background "slate grey"
set grid_foreground white
set render_view_foreground "dark slate gray"

ttk::style configure Infos.TFrame -background $grid_background -borderwidth $grid_border_width -relief $grid_relief
ttk::style configure RenderView.TFrame -background $render_view_foreground -borderwidth $grid_border_width -relief $grid_relief

ttk::frame .c
ttk::frame .c.stats -style Infos.TFrame -width 300 -height 420
ttk::label .c.stats_label -text Stats -background $grid_background -foreground $grid_foreground

ttk::frame .c.render_view -style Infos.TFrame -width 600 -height 420
ttk::frame .c.render_view.g
ttk::label .c.render_view.g.render_view_label -text Render -background $grid_background -foreground $grid_foreground
frame .c.render_view.g.scene -container 1 -width 800 -height 600

ttk::frame .c.explorer -style Infos.TFrame -width 1280 -height 300
ttk::label .c.explorer_label -text Explorer -background $grid_background -foreground $grid_foreground

ttk::frame .c.level -style Infos.TFrame -width 380 -height 200
ttk::frame .c.level.g
ttk::label .c.level.g.level_label -text Levels -background $grid_background -foreground $grid_foreground

ttk::frame .c.options -style Infos.TFrame -width 380 -height 400
ttk::label .c.options_label -text Options -background $grid_background -foreground $grid_foreground

grid .c -column 0 -row 0
grid columnconfigure . 0 -weight 1; grid rowconfigure . 0 -weight 1
grid columnconfigure . 1 -weight 1; grid rowconfigure . 1 -weight 1
grid columnconfigure . 2 -weight 1; grid rowconfigure . 2 -weight 1

grid .c.stats -column 0 -row 0 -rowspan 2 -sticky nwes -pady $grid_pady -padx $grid_padx
grid .c.stats_label -column 0 -row 0 -sticky nwes -pady $grid_pady -padx $grid_padx
grid [ttk::button .c.test_callback -text "Test C++ callback" -command "call_back"] -column 0 -row 0 -stick w

grid .c.render_view -column 1 -row 0 -rowspan 2 -sticky nwes -pady $grid_pady -padx $grid_padx
grid .c.render_view.g -column 0 -row 0 -sticky nwes
grid .c.render_view.g.render_view_label -column 0 -row 0 -sticky nwes -pady $grid_pady -padx $grid_padx
grid .c.render_view.g.scene -column 0 -row 1 -sticky nwes -pady $grid_pady -padx $grid_padx

bind .c.render_view.g.scene <Map> {
  after idle {
    set tk_hwnd [winfo id .c.render_view.g.scene]
    puts "Main HWND: $tk_hwnd"
    
    set children [winfo children .c.render_view.g.scene]
    puts "Children: $children"
    
    plp_get_scene $tk_hwnd
  }
}

grid .c.level -column 2 -row 0 -sticky nwes -pady $grid_pady -padx $grid_padx
grid .c.level.g -column 0 -row 0 -sticky nwes
grid .c.level.g.level_label -column 0 -row 0 -sticky n -pady $grid_pady -padx $grid_padx
grid [ttk::button .c.level.g.skybox_button -text "bluesky skybox" -command "update_skybox"] -column 0 -row 1 -sticky w

grid .c.options -column 2 -row 1 -sticky nwes -pady $grid_pady -padx $grid_padx
grid .c.options_label -column 2 -row 1 -sticky nwes -pady $grid_pady -padx $grid_padx

grid .c.explorer -column 0 -row 2 -columnspan 3 -sticky nwes -pady $grid_pady -padx $grid_padx
grid .c.explorer_label -column 0 -row 2 -columnspan 3 -sticky nwes -pady $grid_pady -padx $grid_padx


# ttk::style configure Danger.TFrame -background red -borderwidth 5 -relief raised
# grid [ttk::frame .c -width 200 -height 200 -padding "3 3 12 12" -style Danger.TFrame] -column 0 -row 0 -sticky nwes
# grid columnconfigure . 0 -weight 1; grid rowconfigure . 0 -weight 1

# grid [ttk::button .skybox_button -text "bluesky skybox" -command "update_skybox"] -column 0 -row 0 -sticky w
# grid [ttk::button .test_callback -text "Test C++ callback" -command "test_callback"] -column 1 -row 0 -stick w

proc call_back {} {
  puts [plp_test_callback arg1 arg2]
}

proc update_skybox {} {
  puts [plp_update_skybox]
}

proc close_plp_edit {} {
  exit 0
}