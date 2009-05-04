/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**  
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**  
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtkmm/stock.h>

#include "joystick_map_widget.hpp"

JoystickMapWidget::JoystickMapWidget()
  : label("Change the order of axis and button. The order applies directly to the "
          "joystick kernel driver, so it will work in any game."),
    up_button(Gtk::Stock::GO_UP),
    down_button(Gtk::Stock::GO_DOWN)
{
  set_border_width(5);

  label.set_line_wrap();
  pack_start(label, Gtk::PACK_SHRINK);
  buttonbox.add(up_button);
  buttonbox.add(down_button);
  add(buttonbox);
}

/* EOF */
