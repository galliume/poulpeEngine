namespace eval Options {

  ttk::frame .c.options -style Infos.TFrame -width 380 -height 400
  ttk::label .c.options_label -text Options -background $::grid_background -foreground $::grid_foreground

  grid .c.options -column 2 -row 1 -sticky nwes -pady $::grid_pady -padx $::grid_padx
  grid .c.options_label -column 2 -row 1 -sticky nwes -pady $::grid_pady -padx $::grid_padx

}
