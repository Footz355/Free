/****************************************************************************
 *   Copyright (c) 2021 Zheng Lei (realthunder) <realthunder.dev@gmail.com> *
 *                                                                          *
 *   This file is part of the FreeCAD CAx development system.               *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Library General Public            *
 *   License as published by the Free Software Foundation; either           *
 *   version 2 of the License, or (at your option) any later version.       *
 *                                                                          *
 *   This library  is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Library General Public License for more details.                   *
 *                                                                          *
 *   You should have received a copy of the GNU Library General Public      *
 *   License along with this library; see the file COPYING.LIB. If not,     *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,          *
 *   Suite 330, Boston, MA  02111-1307, USA                                 *
 *                                                                          *
 ****************************************************************************/

#ifndef APP_GROUP_PARAMS_H
#define APP_GROUP_PARAMS_H

/*[[[cog
import GroupParams
GroupParams.declare()
]]]*/

#include <Base/Parameter.h>

namespace App {
/** Convenient class to obtain App::GroupExtension/GeoFeatureGroupExtension related parameters

 * The parameters are under group "User parameter:BaseApp/Preferences/Group"
 *
 * This class is auto generated by GroupParams.py. Modify that file
 * instead of this one, if you want to add any parameter. You need
 * to install Cog Python package for code generation:
 * @code
 *     pip install cogapp
 * @endcode
 *
 * Once modified, you can regenerate the header and the source file,
 * @code
 *     python3 -m cogapp -r GroupParams.h GroupParams.cpp
 * @endcode
 *
 * You can add a new parameter by adding lines in GroupParams.py. Available
 * parameter types are 'Int, UInt, String, Bool, Float'. For example, to add
 * a new Int type parameter,
 * @code
 *     ParamInt(parameter_name, default_value, documentation, on_change=False)
 * @endcode
 *
 * If there is special handling on parameter change, pass in on_change=True.
 * And you need to provide a function implementation in GroupParams.cpp with
 * the following signature.
 * @code
 *     void GroupParams:on<parameter_name>Changed()
 * @endcode
 */
class AppExport GroupParams {
public:
    static ParameterGrp::handle getHandle();

    //@{
    /// Accessor for parameter ClaimAllChildren
    ///
    /// Claim all children objects in tree view. If disabled, then only claim
    /// children that are not claimed by other children.
    static const bool & getClaimAllChildren();
    static const bool & defaultClaimAllChildren();
    static void removeClaimAllChildren();
    static void setClaimAllChildren(const bool &v);
    static const char *docClaimAllChildren();
    //@}

    //@{
    /// Accessor for parameter KeepHiddenChildren
    ///
    /// Remember invisible children objects and restore only visible objects
    /// when the group is made visible.
    static const bool & getKeepHiddenChildren();
    static const bool & defaultKeepHiddenChildren();
    static void removeKeepHiddenChildren();
    static void setKeepHiddenChildren(const bool &v);
    static const char *docKeepHiddenChildren();
    //@}

    //@{
    /// Accessor for parameter ExportChildren
    ///
    /// Export visible children. Note, that once this option is enabled,
    /// the group object will be touched when its child toggles visibility.
    static const bool & getExportChildren();
    static const bool & defaultExportChildren();
    static void removeExportChildren();
    static void setExportChildren(const bool &v);
    static const char *docExportChildren();
    //@}

    //@{
    /// Accessor for parameter CreateOrigin
    ///
    /// Create all origin features when the origin group is created. If Disabled
    /// The origin features will only be created when the origin group is expanded
    /// for the first time.
    static const bool & getCreateOrigin();
    static const bool & defaultCreateOrigin();
    static void removeCreateOrigin();
    static void setCreateOrigin(const bool &v);
    static const char *docCreateOrigin();
    //@}

    // Auto generated code. See class document of GroupParams.
};
} // namespace App
//[[[end]]]

#endif // APP_GROUP_PARAMS_H
