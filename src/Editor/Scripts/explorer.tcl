namespace eval Explorer {

  ttk::notebook .c.tab_explorer -width 1280 -height 300
  grid .c.tab_explorer -column 0 -row 2 -columnspan 3

  ttk::frame .c.ambient -style Infos.TFrame -width 1280 -height 300
  ttk::frame .c.music -style Infos.TFrame -width 1280 -height 300
  ttk::frame .c.texture -style Infos.TFrame -width 1280 -height 300

  tk::listbox .c.ambient.ambient_sounds -height 10 -width 1280
  tk::listbox .c.music.musics -height 10 -width 1280
  image create photo img -file "./assets/texture/mpoulpe.png" -height 680
  label .c.texture.textures -image img

  grid .c.ambient.ambient_sounds -column 0 -row 0 -sticky news -padx $::grid_padx -pady $::grid_pady
  grid .c.music.musics -column 0 -row 0 -sticky news -padx $::grid_padx -pady $::grid_pady
  grid .c.texture.textures -column 0 -row 0 -sticky news -padx $::grid_padx -pady $::grid_pady

  bind .c.ambient.ambient_sounds <Double-Button-1> { plp_play_sound [.c.ambient.ambient_sounds curselection] }

  .c.tab_explorer add .c.ambient -text "Ambient Sounds"
  .c.tab_explorer add .c.music -text "Musics"
  .c.tab_explorer add .c.texture -text "Textures"

  after 100 {
    plp_list_ambient_sound
  }
}
