//
// Copyright (c) CNRS
// Author: Joseph Mirabel
//

#ifndef HPP_GUI_CONVERSIONS_HH
#define HPP_GUI_CONVERSIONS_HH

#include <gepetto/gui/windows-manager.hh>
#include <hpp/common-idl.hh>

namespace hpp {
namespace gui {
inline void fromHPP(const hpp::floatSeq_var& in, osgVector3& v) {
  typedef gepetto::viewer::WindowsManager::value_type type;
  const hpp::floatSeq& t(in.in());
  v.set((type)t[0], (type)t[1], (type)t[2]);
}

inline void fromHPP(const hpp::Transform__slice* t, osgVector3& v) {
  typedef gepetto::viewer::WindowsManager::value_type type;
  v.set((type)t[0], (type)t[1], (type)t[2]);
}

inline void fromHPP(const hpp::Transform__var& in, osgVector3& v) {
  fromHPP(in.in(), v);
}

inline void fromHPP(const hpp::Transform__slice* t, osgQuat& q) {
  typedef gepetto::viewer::WindowsManager::value_type type;
  q.set((type)t[3], (type)t[4], (type)t[5], (type)t[6]);
}

inline void fromHPP(const hpp::Transform__var& in, osgQuat& q) {
  fromHPP(in.in(), q);
}

inline void fromHPP(const hpp::Transform__slice* in,
                    gepetto::viewer::Configuration& c) {
  fromHPP(in, c.position);
  fromHPP(in, c.quat);
}

inline void fromHPP(const hpp::Transform__var& in,
                    gepetto::viewer::Configuration& c) {
  fromHPP(in.in(), c);
}

inline void fromHPP(const hpp::TransformSeq& in,
                    std::vector<gepetto::viewer::Configuration>& c) {
  c.resize(in.length());
  for (std::size_t i = 0; i < in.length(); ++i)
    fromHPP(in[(CORBA::ULong)i], c[i]);
}

}  // namespace gui
}  // namespace hpp

#endif  // HPP_GUI_CONVERSIONS_HH
