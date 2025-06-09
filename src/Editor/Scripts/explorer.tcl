namespace eval Explorer {

  ttk::frame .c.explorer -style Infos.TFrame -width 1280 -height 300
  ttk::label .c.explorer_label -text Explorer -background $::grid_background -foreground $::grid_foreground

  grid .c.explorer -column 0 -row 2 -columnspan 3 -sticky nwes -pady $::grid_pady -padx $::grid_padx
  grid .c.explorer_label -column 0 -row 2 -columnspan 3 -sticky nwes -pady $::grid_pady -padx $::grid_padx
}
