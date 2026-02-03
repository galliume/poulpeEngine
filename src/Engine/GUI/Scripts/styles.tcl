set grid_padx 2
set grid_pady 2
set grid_border_width 1
set grid_relief ridge
set grid_background "slate grey"
set grid_foreground white
set render_view_foreground "dark slate gray"

ttk::style configure Infos.TFrame -background $grid_background -borderwidth $grid_border_width -relief $grid_relief
ttk::style configure RenderView.TFrame -background $render_view_foreground -borderwidth $grid_border_width -relief $grid_relief