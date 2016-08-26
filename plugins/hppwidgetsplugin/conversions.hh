#ifndef HPP_GUI_CONVERSIONS_HH
#define HPP_GUI_CONVERSIONS_HH

namespace hpp {
  namespace gui {
    template <typename Out>
    void fromHPP(const hpp::Transform__var& in, Out out[7])
    {
      for (int i = 0; i < 3; ++i) {
        out[i] = (Out)in.in()[i];
        out[4+i] = (Out)in.in()[3+i];
      }
      out[3] = (Out)in.in()[6];
    }
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_CONVERSIONS_HH
