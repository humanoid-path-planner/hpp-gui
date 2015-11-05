#ifndef HPP_GUI_META_HH
#define HPP_GUI_META_HH

#include <QString>

namespace hpp {
  namespace gui {
    template <typename T> struct Traits;

    template <> struct Traits <QString> {
      typedef CORBA::String_var CORBA_t;
      static inline CORBA_t to_corba (const QString& s) { return (const char*)s.toLocal8Bit().data(); }
    };
    template <> struct Traits <std::string> {
      typedef CORBA::String_var CORBA_t;
      static inline CORBA_t to_corba (const std::string& s) { return s.c_str(); }
    };
  }
}

#endif // HPP_GUI_META_HH
