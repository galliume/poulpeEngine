namespace eval MainUI {

  ttk::frame .c.level -style Infos.TFrame -width 380 -height 200
  ttk::frame .c.level.g
  ttk::label .c.level.g.level_label -text "Subido la marea" -background $::grid_background -foreground $::grid_foreground

  grid .c.level -column 2 -row 0 -sticky nwes -pady $::grid_pady -padx $::grid_padx
  grid .c.level.g -column 0 -row 0 -sticky nwes
  grid .c.level.g.level_label -column 0 -row 0 -sticky n -pady $::grid_pady -padx $::grid_padx
  grid [ttk::button .c.level.g.skybox_button -text "Yatangaki"] -column 0 -row 1 -sticky w

  # proc update_skybox {} {
  #   puts [plp_update_skybox]
  # }
}
