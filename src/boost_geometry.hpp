#ifndef FREECAD_GEOMETRY_HPP_WORKAROUND
#define FREECAD_GEOMETRY_HPP_WORKAROUND

// Workaround for boost >= 1.74
#define BOOST_ALLOW_DEPRECATED_HEADERS
#include <boost/geometry.hpp>
#undef BOOST_ALLOW_DEPRECATED_HEADERS

#endif // #ifndef FREECAD_GEOMETRY_HPP_WORKAROUND
