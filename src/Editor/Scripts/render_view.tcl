namespace eval RenderView {

  ttk::frame .c.render_view -style Infos.TFrame -width 1600 -height 900
  ttk::frame .c.render_view.g
  frame .c.render_view.g.scene -width 1600 -height 900

  grid .c.render_view -column 1 -row 0 -rowspan 2 -sticky nwes -pady $::grid_pady -padx $::grid_padx
  grid .c.render_view.g -column 0 -row 0 -sticky nwes
  grid .c.render_view.g.scene -column 0 -row 1 -sticky nwes -pady $::grid_pady -padx $::grid_padx

  after 100 {
    set tk_hwnd [winfo id .c.render_view.g.scene]
    plp_get_scene $tk_hwnd
  }
}
