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

#include <sstream>
#include <iostream>
#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>

#include "joystick_test_widget.hpp"
#include "main.hpp"
#include "joystick.hpp"
#include "button_widget.hpp"
#include "joystick_map_widget.hpp"
#include "joystick_calibration_widget.hpp"

JoystickTestWidget::JoystickTestWidget(JoystickGui& gui, Joystick& joystick_, bool simple_ui) :
  Gtk::Window(),
  m_gui(gui),
  joystick(joystick_),
  m_simple_ui(simple_ui),
  label("<b>" + joystick.get_name() + "</b>\nDevice: " + joystick.get_filename(),
        Gtk::ALIGN_START, Gtk::ALIGN_START),
  axis_frame("Axes"),
  button_frame("Buttons"),
  mapping_button("Mapping"),
  calibration_button("Calibration"),
  close_button(Gtk::Stock::CLOSE),
  buttonbox(),
  stick1_widget(128, 128),
  stick2_widget(128, 128),
  stick3_widget(128, 128),
  rudder_widget(128, 32),
  throttle_widget(32, 128),
  left_trigger_widget(32, 128, true),
  right_trigger_widget(32, 128, true)
{
  set_title(joystick_.get_name());
  set_icon_from_file(Main::current()->get_data_directory() + "generic.png");
  label.set_use_markup(true);

  label.set_selectable(true);

  axis_frame.set_border_width(5);
  axis_table.set_border_width(5);
  axis_table.set_spacings(5);
  button_frame.set_border_width(5);
  button_table.set_border_width(5);
  button_table.set_spacings(8);
  buttonbox.set_border_width(5);

  // Vector to store value labels for each axis
  axis_value_labels.resize(joystick.get_axis_count());

  for(int i = 0; i < joystick.get_axis_count(); ++i)
  {
    std::ostringstream str;
    str << "Axis " << i << ": ";
    Gtk::Label& label = *Gtk::manage(new Gtk::Label(str.str()));

    Gtk::ProgressBar& progressbar = *Gtk::manage(new Gtk::ProgressBar());
    progressbar.set_fraction(0.5);

    // Create a label to show the axis value
    axis_value_labels[i] = Gtk::manage(new Gtk::Label("0"));
    axis_value_labels[i]->set_alignment(Gtk::ALIGN_END, Gtk::ALIGN_CENTER);
    axis_value_labels[i]->set_size_request(60, -1);  // Fixed width for consistent display
    
    //Each column must have at most 10 axes
    int x = (i/10)*3;  // *3 instead of *2 to make room for value labels
    int y = i%10;

    axis_table.attach(label, x, x+1, y, y+1, Gtk::SHRINK, Gtk::FILL);
    axis_table.attach(progressbar, x+1, x+2, y, y+1, Gtk::FILL|Gtk::EXPAND, Gtk::EXPAND);
    axis_table.attach(*axis_value_labels[i], x+2, x+3, y, y+1, Gtk::SHRINK, Gtk::FILL);

    axes.push_back(&progressbar);
  }

  for(int i = 0; i < joystick.get_button_count(); ++i)
  {
    int x = i / 10;
    int y = i % 10;

    std::ostringstream str;
    str << i;
    ButtonWidget& button = *Gtk::manage(new ButtonWidget(32, 32, str.str()));
    button_table.attach(button, x, x+1, y, y+1, Gtk::EXPAND, Gtk::EXPAND);
    buttons.push_back(&button);
  }

  alignment.set_padding(8, 8, 8, 8);
  alignment.add(label);
  m_vbox.pack_start(alignment, Gtk::PACK_SHRINK);

  buttonbox.add(mapping_button);
  buttonbox.add(calibration_button);
  buttonbox.add(close_button);

  test_hbox.pack_start(axis_frame,   Gtk::PACK_EXPAND_WIDGET);
  test_hbox.pack_start(button_frame, Gtk::PACK_EXPAND_WIDGET);
  m_vbox.pack_start(test_hbox, Gtk::PACK_SHRINK);
  m_vbox.pack_end(buttonbox, Gtk::PACK_SHRINK);

  add(m_vbox);

  stick_hbox.set_border_width(5);

  axis_callbacks.clear();
  raw_value_callbacks.clear();
  // Using manual loop instead of resize, as else all the signals
  // would be clones of each other, instead of individual signals
  for(int i = 0; i < (int)joystick.get_axis_count(); ++i) {
    axis_callbacks.push_back(sigc::signal<void, double>());
    raw_value_callbacks.push_back(sigc::signal<void, int>());
  }

  // Check if this is a Thrustmaster HOTAS Warthog or always add stick widget for first two axes
  bool is_thrustmaster = (joystick.get_name().find("Thrustmaster") != std::string::npos);
  bool has_at_least_two_axes = (joystick.get_axis_count() >= 2);
  
  // Always show main stick widget for first two axes if device has at least 2 axes
  if (has_at_least_two_axes) {
    stick_hbox.pack_start(stick1_widget, Gtk::PACK_EXPAND_PADDING);
    raw_value_callbacks[0].connect(sigc::mem_fun(stick1_widget, &AxisWidget::set_raw_x));
    raw_value_callbacks[1].connect(sigc::mem_fun(stick1_widget, &AxisWidget::set_raw_y));
  }

  // Continue with the regular joystick type handling
  switch(joystick.get_axis_count())
  {
  case 2: // Simple stick
    // Already handled above
    break;

  case 6: // Flightstick
  {
    Gtk::Table& table = *Gtk::manage(new Gtk::Table(2, 2));

    // Don't add stick1_widget again since we already did it above
    table.attach(rudder_widget,   0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
    table.attach(throttle_widget, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK);

    stick_hbox.pack_start(table, Gtk::PACK_EXPAND_PADDING);
    stick_hbox.pack_start(stick3_widget, Gtk::PACK_EXPAND_PADDING);

    // Don't connect stick1_widget again
    axis_callbacks[2].connect(sigc::mem_fun(rudder_widget, &RudderWidget::set_pos));
    axis_callbacks[3].connect(sigc::mem_fun(throttle_widget, &ThrottleWidget::set_pos));
    raw_value_callbacks[4].connect(sigc::mem_fun(stick3_widget, &AxisWidget::set_raw_x));
    raw_value_callbacks[5].connect(sigc::mem_fun(stick3_widget, &AxisWidget::set_raw_y));
    break;
  }

  case 8: // Dual Analog Gamepad + Analog Trigger
    // Don't add stick1_widget again
    stick_hbox.pack_start(stick2_widget, Gtk::PACK_EXPAND_PADDING);
    stick_hbox.pack_start(stick3_widget, Gtk::PACK_EXPAND_PADDING);
    stick_hbox.pack_start(left_trigger_widget, Gtk::PACK_EXPAND_PADDING);
    stick_hbox.pack_start(right_trigger_widget, Gtk::PACK_EXPAND_PADDING);

    // Don't connect stick1_widget again
    raw_value_callbacks[2].connect(sigc::mem_fun(stick2_widget, &AxisWidget::set_raw_x));
    raw_value_callbacks[3].connect(sigc::mem_fun(stick2_widget, &AxisWidget::set_raw_y));
    raw_value_callbacks[6].connect(sigc::mem_fun(stick3_widget, &AxisWidget::set_raw_x));
    raw_value_callbacks[7].connect(sigc::mem_fun(stick3_widget, &AxisWidget::set_raw_y));
    axis_callbacks[4].connect(sigc::mem_fun(left_trigger_widget, &ThrottleWidget::set_pos));
    axis_callbacks[5].connect(sigc::mem_fun(right_trigger_widget, &ThrottleWidget::set_pos));
    break;

  case 7: // Dual Analog Gamepad DragonRise Inc. Generic USB Joystick
    // Don't add stick1_widget again
    stick_hbox.pack_start(stick2_widget, Gtk::PACK_EXPAND_PADDING);
    stick_hbox.pack_start(stick3_widget, Gtk::PACK_EXPAND_PADDING);

    // Don't connect stick1_widget again
    raw_value_callbacks[3].connect(sigc::mem_fun(stick2_widget, &AxisWidget::set_raw_x));
    raw_value_callbacks[4].connect(sigc::mem_fun(stick2_widget, &AxisWidget::set_raw_y));
    raw_value_callbacks[5].connect(sigc::mem_fun(stick3_widget, &AxisWidget::set_raw_x));
    raw_value_callbacks[6].connect(sigc::mem_fun(stick3_widget, &AxisWidget::set_raw_y));
    break;

  case 27: // Playstation 3 Controller
    // Don't add stick1_widget again
    stick_hbox.pack_start(stick2_widget, Gtk::PACK_EXPAND_PADDING);
    // Not using stick3 for now, as the dpad is 4 axis on the PS3, not 2 (one for each direction)
    //stick_hbox.pack_start(stick3_widget, Gtk::PACK_EXPAND_PADDING);
    stick_hbox.pack_start(left_trigger_widget, Gtk::PACK_EXPAND_PADDING);
    stick_hbox.pack_start(right_trigger_widget, Gtk::PACK_EXPAND_PADDING);

    // Don't connect stick1_widget again
    raw_value_callbacks[2].connect(sigc::mem_fun(stick2_widget, &AxisWidget::set_raw_x));
    raw_value_callbacks[3].connect(sigc::mem_fun(stick2_widget, &AxisWidget::set_raw_y));
    //axis_callbacks[6].connect(sigc::mem_fun(stick3_widget, &AxisWidget::set_x_axis));
    //axis_callbacks[7].connect(sigc::mem_fun(stick3_widget, &AxisWidget::set_y_axis));
    axis_callbacks[12].connect(sigc::mem_fun(left_trigger_widget, &ThrottleWidget::set_pos));
    axis_callbacks[13].connect(sigc::mem_fun(right_trigger_widget, &ThrottleWidget::set_pos));
    break;

  default:
    // Do nothing here, we already added stick1_widget for the first two axes
    std::cout << "Using default circular display for first two axes" << std::endl;
    break;
  }

  // Always show the stick widgets unless simple_ui is enabled
  if (!m_simple_ui)
  {
    axis_vbox.pack_start(stick_hbox, Gtk::PACK_SHRINK);
  }

  axis_vbox.add(axis_table);
  axis_frame.add(axis_vbox);

  button_frame.add(button_table);

  joystick.axis_move.connect(sigc::mem_fun(this, &JoystickTestWidget::axis_move));
  joystick.button_move.connect(sigc::mem_fun(this, &JoystickTestWidget::button_move));

  calibration_button.signal_clicked().connect(sigc::mem_fun(this, &JoystickTestWidget::on_calibrate));
  mapping_button.signal_clicked().connect(sigc::mem_fun(this, &JoystickTestWidget::on_mapping));
  close_button.signal_clicked().connect(sigc::mem_fun(*this, &Gtk::Widget::hide));

  close_button.grab_focus();
}

void
JoystickTestWidget::axis_move(int number, int value)
{
  // Update progress bar
  axes.at(number)->set_fraction((value + 32767) / (double)(2*32767));

  // Update progress bar text with the value
  std::ostringstream str;
  str << value;
  axes.at(number)->set_text(str.str());
  
  // Update value label
  axis_value_labels.at(number)->set_text(str.str());
  
  // Update other widgets
  axis_callbacks[number](value / 32767.0);
  raw_value_callbacks[number](value);
}

void
JoystickTestWidget::button_move(int number, bool value)
{
  if (value)
    buttons.at(number)->set_down(true);
  else
    buttons.at(number)->set_down(false);
}

void
JoystickTestWidget::on_calibrate()
{
  m_gui.show_calibration_dialog();
}

void
JoystickTestWidget::on_mapping()
{
  m_gui.show_mapping_dialog();
}

/* EOF */
