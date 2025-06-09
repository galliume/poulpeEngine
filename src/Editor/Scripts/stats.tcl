namespace eval Stats {

  ttk::frame .c.stats -style Infos.TFrame -width 300 -height 420
  ttk::label .c.stats_label -text Stats -background $::grid_background -foreground $::grid_foreground

  grid .c.stats -column 0 -row 0 -rowspan 2 -sticky nwes -pady $::grid_pady -padx $::grid_padx
  grid .c.stats_label -column 0 -row 0 -sticky nwes -pady $::grid_pady -padx $::grid_padx
  grid [ttk::button .c.test_callback -text "Test C++ callback" -command "Stats::call_back"] -column 0 -row 0 -stick w

  proc call_back {} {
    puts [plp_test_callback arg1 arg2]
  }
}
