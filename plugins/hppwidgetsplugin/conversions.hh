//
// Copyright (c) CNRS
// Author: Joseph Mirabel
//

#ifndef HPP_GUI_CONVERSIONS_HH
#define HPP_GUI_CONVERSIONS_HH

#include <gepetto/gui/windows-manager.hh>

namespace hpp {
  namespace gui {
    inline void fromHPP(const hpp::floatSeq_var& in, osgVector3& v)
    {
      typedef graphics::WindowsManager::value_type type;
      const hpp::floatSeq& t (in.in());
      v.set((type)t[0], (type)t[1], (type)t[2]);
    }

    inline void fromHPP(const hpp::Transform__var& in, osgVector3& v)
    {
      typedef graphics::WindowsManager::value_type type;
      const hpp::Transform__slice* t (in.in());
      v.set((type)t[0], (type)t[1], (type)t[2]);
    }

    inline void fromHPP(const hpp::Transform__var& in, osgQuat& q)
    {
      typedef graphics::WindowsManager::value_type type;
      const hpp::Transform__slice* t (in.in());
      q.set((type)t[3], (type)t[4], (type)t[5], (type)t[6]);
    }

    inline void fromHPP(const hpp::Transform__var& in, graphics::Configuration& c)
    {
      fromHPP(in, c.position);
      fromHPP(in, c.quat);
    }

  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_CONVERSIONS_HH
