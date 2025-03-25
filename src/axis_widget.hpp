/*
**  jstest-gtk - A graphical joystick tester
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_JSTEST_GTK_AXIS_WIDGET_HPP
#define HEADER_JSTEST_GTK_AXIS_WIDGET_HPP

#include <gtkmm/drawingarea.h>
#include <gtkmm/alignment.h>

class AxisWidget : public Gtk::Alignment
{
private:
  Gtk::DrawingArea drawingarea;
  double x;
  double y;
  int raw_x;  // Raw value from -32767 to 32767
  int raw_y;  // Raw value from -32767 to 32767
  bool show_values;  // Flag to control whether to show values or not

public:
  AxisWidget(int width, int height, bool show_values = true);

  bool on_draw(const Cairo::RefPtr<Cairo::Context>& context);

  void set_x_axis(double x);
  void set_y_axis(double y);
  
  // New methods to set raw values
  void set_raw_x(int raw_x_value);
  void set_raw_y(int raw_y_value);

private:
  AxisWidget(const AxisWidget&);
  AxisWidget& operator=(const AxisWidget&);
};

#endif

/* EOF */
