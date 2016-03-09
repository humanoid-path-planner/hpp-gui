// Copyright (c) 2016, Joseph Mirabel
// Authors: Joseph Mirabel (joseph.mirabel@laas.fr)
//
// This file is part of hpp-gui.
// hpp-gui is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-gui is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-gui. If not, see <http://www.gnu.org/licenses/>.

#include <hpp/gui/color-map.hh>
#include <iostream>

// #define STRINGIFY(x) #x
// #define TOSTRING(x) STRINGIFY(x)

#define CHECK(test,msg) \
  do { \
    if (!(test)) { std::cerr << #test << ": " << msg << std::endl; errorCount++; } \
  } while (0)

using namespace hpp::gui;

int errorCount = 0;

int main (int, char**) {
  ColorMap c (3); // log2 ceil -> 8
  CHECK(c.remap (0) ==  0, "Value is " << c.remap (0));
  CHECK(c.remap (1) ==  4, "Value is " << c.remap (1));
  CHECK(c.remap (2) ==  2, "Value is " << c.remap (2));

  c = ColorMap (4); // log2 ceil -> 8
  CHECK(c.remap (0) ==  0, "Value is " << c.remap (0));
  CHECK(c.remap (1) ==  8, "Value is " << c.remap (1));
  CHECK(c.remap (2) ==  4, "Value is " << c.remap (2));
  CHECK(c.remap (3) == 12, "Value is " << c.remap (3));

  if (errorCount == 0) std::cout << "No error detected" << std::endl;
  else std::cout << errorCount << " error(s) detected" << std::endl;
  return 0;
}
