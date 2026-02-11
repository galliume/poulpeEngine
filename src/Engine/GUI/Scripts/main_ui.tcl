namespace eval MainUI {

  ttk::frame .c.level -style Infos.TFrame -width 380 -height 200
  ttk::frame .c.level.g
  ttk::label .c.level.g.level_label -text "Main menu" -background $::grid_background -foreground $::grid_foreground

  grid .c.level -column 2 -row 0 -sticky nwes -pady $::grid_pady -padx $::grid_padx
  grid .c.level.g -column 0 -row 0 -sticky nwes
  grid .c.level.g.level_label -column 0 -row 0 -sticky n -pady $::grid_pady -padx $::grid_padx
  grid [ttk::button .c.level.g.quit_button -text "Quit game" -command MainUI::quit_game] -column 0 -row 1 -sticky w

  proc quit_game {} {
    
    if {[tk_messageBox -type yesno -icon warning -title "Leave ?" -message "Leave ?"] eq "yes"} {
      plp_quit_game
    }
  }
}
