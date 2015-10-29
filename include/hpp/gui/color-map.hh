#ifndef HPP_GUI_COLORMAP_HH
#define HPP_GUI_COLORMAP_HH

#include <QColor>

namespace hpp {
  namespace gui {
    class ColorMap {
      public:
        static QColor interpolate (std::size_t nbColors, std::size_t index)
        {
          return QColor::fromHslF((qreal)index / (qreal)nbColors, 1, 0.5);
        }

        ColorMap (std::size_t nbColors) :
          nbColors_ (nbColors),
          currentIndex_ (0)
      {}

        QColor getColor (std::size_t index) const {
          return ColorMap::interpolate(nbColors_, index);
        }

        void getColor (std::size_t index, float color[4]) const {
          QColor c = getColor(index);
          color[0] = (float)c.redF();
          color[1] = (float)c.greenF();
          color[2] = (float)c.blueF();
          color[3] = (float)c.alphaF();
        }

        QColor nextColor () {
          QColor color = getColor (currentIndex_);
          currentIndex(currentIndex_ + 1);
          return color;
        }

        void currentIndex (std::size_t index) {
          currentIndex_ = index % nbColors_;
        }

      private:
        std::size_t nbColors_;
        std::size_t currentIndex_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_COLORMAP_HH
