/***************************************************************************
 *   Copyright (c) 2010 Juergen Riegel <FreeCAD@juergen-riegel.net>        *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"
#ifndef _PreComp_
# include <Bnd_Box.hxx>
# include <gp_Dir.hxx>
# include <gp_Pln.hxx>
# include <BRep_Builder.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <BRepBndLib.hxx>
# include <BRepFeat_MakePrism.hxx>
# include <BRepBuilderAPI_MakeFace.hxx>
# include <Geom_Plane.hxx>
# include <Geom_Surface.hxx>
# include <TopoDS.hxx>
# include <TopoDS_Face.hxx>
# include <TopoDS_Wire.hxx>
# include <TopoDS_Solid.hxx>
# include <TopExp_Explorer.hxx>
# include <BRepAlgoAPI_Cut.hxx>
# include <BRepPrimAPI_MakeHalfSpace.hxx>
# include <BRepAlgoAPI_Common.hxx>
#endif

#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Placement.h>
#include <App/Document.h>
#include <Mod/Part/App/FeatureExtrusion.h>
#include <Mod/Part/App/PartParams.h>

#include "FeaturePocket.h"

FC_LOG_LEVEL_INIT("PartDesign", true, true)

using namespace PartDesign;

/* TRANSLATOR PartDesign::Pocket */

const char* Pocket::TypeEnums[]= {"Length","ThroughAll","UpToFirst","UpToFace","TwoLengths",NULL};

PROPERTY_SOURCE(PartDesign::Pocket, PartDesign::ProfileBased)

Pocket::Pocket()
{
    addSubType = FeatureAddSub::Subtractive;

    ADD_PROPERTY_TYPE(Type,((long)0),"Pocket",App::Prop_None,"Pocket type");
    Type.setEnums(TypeEnums);
    ADD_PROPERTY_TYPE(Length,(100.0),"Pocket",App::Prop_None,"Pocket length");
    ADD_PROPERTY_TYPE(Length2,(100.0),"Pocket",App::Prop_None,"P");
    ADD_PROPERTY_TYPE(UpToFace,(0),"Pocket",App::Prop_None,"Face where pocket will end");
    ADD_PROPERTY_TYPE(Offset,(0.0),"Pocket",App::Prop_None,"Offset from face in which pocket will end");
    static const App::PropertyQuantityConstraint::Constraints signedLengthConstraint = {-DBL_MAX, DBL_MAX, 1.0};
    Offset.setConstraints ( &signedLengthConstraint );

    // Remove the constraints and keep the type to allow to accept negative values
    // https://forum.freecadweb.org/viewtopic.php?f=3&t=52075&p=448410#p447636
    Length2.setConstraints(nullptr);

    ADD_PROPERTY_TYPE(TaperAngle,(0.0), "Pocket", App::Prop_None, "Sets the angle of slope (draft) to apply to the sides. The angle is for outward taper; negative value yields inward tapering.");
    ADD_PROPERTY_TYPE(TaperAngleRev,(0.0), "Pocket", App::Prop_None, "Taper angle of reverse part of pocketing.");
    ADD_PROPERTY_TYPE(InnerTaperAngle,(0.0), "Pocket", App::Prop_None, "Taper angle of inner holes.");
    ADD_PROPERTY_TYPE(InnerTaperAngleRev,(0.0), "Pocket", App::Prop_None, "Taper angle of the reverse part for inner holes.");

    ADD_PROPERTY_TYPE(UsePipeForDraft,(false), "Pocket", App::Prop_None, "Use pipe (i.e. sweep) operation to create draft angles.");
    ADD_PROPERTY_TYPE(Linearize,(false), "Pocket", App::Prop_None,
            "Linearize the resut shape by simplify linear edge and planar face into line and plane");
}

short Pocket::mustExecute() const
{
    if (Placement.isTouched() ||
        Type.isTouched() ||
        Length.isTouched() ||
        Length2.isTouched() ||
        Offset.isTouched() ||
        UpToFace.isTouched())
        return 1;
    return ProfileBased::mustExecute();
}

App::DocumentObjectExecReturn *Pocket::execute(void)
{
    std::string method(Type.getValueAsString());                

    // Handle legacy features, these typically have Type set to 3 (previously NULL, now UpToFace),
    // empty FaceName (because it didn't exist) and a value for Length
    if (method == "UpToFace" &&
        (UpToFace.getValue() == NULL && Length.getValue() > Precision::Confusion()))
        Type.setValue("Length");

    // Validate parameters
    double L = Length.getValue();
    if ((method == "Length") && (L < Precision::Confusion()))
        return new App::DocumentObjectExecReturn("Length too small");
    double L2 = 0;
    if ((method == "TwoLengths")) {
        L2 = Length2.getValue();
        if (std::abs(L2) < Precision::Confusion())
            return new App::DocumentObjectExecReturn("Second length too small");
    }

    Part::Feature* obj = 0;
    TopoShape profileshape;
    try {
        obj = getVerifiedObject();
        profileshape = getVerifiedFace();
    } catch (const Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }

    // if the Base property has a valid shape, fuse the prism into it
    TopoShape base;
    base = getBaseShape(true, true);

    // get the Sketch plane
    Base::Placement SketchPos    = obj->Placement.getValue();
    Base::Vector3d  SketchVector = getProfileNormal();

    // turn around for pockets
    SketchVector *= -1;

    try {
        this->positionByPrevious();
        TopLoc_Location invObjLoc = this->getLocation().Inverted();

        base.move(invObjLoc);

        gp_Dir dir(SketchVector.x,SketchVector.y,SketchVector.z);
        dir.Transform(invObjLoc.Transformation());

        if (profileshape.isNull())
            return new App::DocumentObjectExecReturn("Pocket: Creating a face from sketch failed");
        profileshape.move(invObjLoc);

        if (method == "UpToFirst" || method == "UpToFace") {
            if (base.isNull())
                return new App::DocumentObjectExecReturn("Pocket: Extruding up to a face is only possible if the sketch is located on a face");

            // Note: This will return an unlimited planar face if support is a datum plane
            TopoShape supportface = getSupportFace();
            supportface.move(invObjLoc);

            if (Reversed.getValue())
                dir.Reverse();

            // Find a valid face or datum plane to extrude up to
            TopoShape upToFace;
            if (method == "UpToFace") {
                getUpToFaceFromLinkSub(upToFace, UpToFace);
                upToFace.move(invObjLoc);
            }
            getUpToFace(upToFace, base, supportface, profileshape, method, dir);
            addOffsetToFace(upToFace, dir, Offset.getValue());

            // BRepFeat_MakePrism(..., 2, 1) in combination with PerForm(upToFace) is buggy when the
            // prism that is being created is contained completely inside the base solid
            // In this case the resulting shape is empty. This is not a problem for the Pad or Pocket itself
            // but it leads to an invalid SubShape
            // The bug only occurs when the upToFace is limited (by a wire), not for unlimited upToFace. But
            // other problems occur with unlimited concave upToFace so it is not an option to always unlimit upToFace
            // Check supportface for limits, otherwise Perform() throws an exception
            if (!supportface.hasSubShape(TopAbs_WIRE))
                supportface = TopoShape();
#if 0
            BRepFeat_MakePrism PrismMaker;
            PrismMaker.Init(base.getShape(), profileshape.getShape(), supportface, dir, 0, 1);
            PrismMaker.Perform(upToFace);

            if (!PrismMaker.IsDone())
                return new App::DocumentObjectExecReturn("Pocket: Up to face: Could not extrude the sketch!");

            TopoShape prism(0,getDocument()->getStringHasher());
            prism.makEShape(PrismMaker,{base,profileshape});
#else
            TopoShape prism(0,getDocument()->getStringHasher());
            profileshape.reTagElementMap(-getID(), getDocument()->getStringHasher());
            PrismMode mode = PrismMode::CutFromBase;
            generatePrism(prism, method, base, profileshape, supportface, upToFace, dir, mode, Standard_True);
#endif
            // DO NOT assign id to the generated prism, because this prism is
            // actually the final result. We obtain the subtracted shape by cut
            // this prism with the original base. Assigning a minus self id here
            // will mess up with preselection highlight. It is enough to re-tag
            // the profile shape above.
            //
            // prism.Tag = -this->getID();

            // And the really expensive way to get the SubShape...
            try {
                TopoShape result(0,getDocument()->getStringHasher());
                result.makECut({base,prism});
                // FIXME: In some cases this affects the Shape property: It is set to the same shape as the SubShape!!!!
                result = refineShapeIfActive(result);
                this->AddSubShape.setValue(result);
            }catch(Standard_Failure &) {
                return new App::DocumentObjectExecReturn("Pocket: Up to face: Could not get SubShape!");
            }

            if (NewSolid.getValue())
                prism = this->AddSubShape.getShape();
            else
                prism = refineShapeIfActive(prism);
            this->Shape.setValue(getSolid(prism));

        } else {
            TopoShape prism(0,getDocument()->getStringHasher());

            Part::Extrusion::ExtrusionParameters params;
            params.dir = dir;
            params.solid = true;
            params.taperAngleFwd = this->TaperAngle.getValue() * M_PI / 180.0;
            params.taperAngleRev = this->TaperAngleRev.getValue() * M_PI / 180.0;
            params.innerTaperAngleFwd = this->InnerTaperAngle.getValue() * M_PI / 180.0;
            params.innerTaperAngleRev = this->InnerTaperAngleRev.getValue() * M_PI / 180.0;
            if (L2 == 0.0 && Midplane.getValue()) {
                params.lengthFwd = L/2;
                params.lengthRev = L/2;
                if (params.taperAngleRev == 0.0)
                    params.taperAngleRev = params.taperAngleFwd;
            } else {
                params.lengthFwd = L;
                params.lengthRev = L2;
            }
            if (std::fabs(params.taperAngleFwd) >= Precision::Angular()
                    || std::fabs(params.taperAngleRev) >= Precision::Angular()
                    || std::fabs(params.innerTaperAngleFwd) >= Precision::Angular()
                    || std::fabs(params.innerTaperAngleRev) >= Precision::Angular()) {
                if (fabs(params.taperAngleFwd) > M_PI * 0.5 - Precision::Angular()
                        || fabs(params.taperAngleRev) > M_PI * 0.5 - Precision::Angular())
                    return new App::DocumentObjectExecReturn(
                            "Magnitude of taper angle matches or exceeds 90 degrees");
                if (fabs(params.innerTaperAngleFwd) > M_PI * 0.5 - Precision::Angular()
                        || fabs(params.innerTaperAngleRev) > M_PI * 0.5 - Precision::Angular())
                    return new App::DocumentObjectExecReturn(
                            "Magnitude of inner taper angle matches or exceeds 90 degrees");
                if (Reversed.getValue())
                    params.dir.Reverse();
                std::vector<TopoShape> drafts;
                params.usepipe = this->UsePipeForDraft.getValue();
                params.linearize = this->Linearize.getValue();
                Part::Extrusion::makeDraft(params, profileshape, drafts, getDocument()->getStringHasher());
                if (drafts.empty())
                    return new App::DocumentObjectExecReturn("Pocket with draft angle failed");
                prism.makECompound(drafts,0,false);
            } else
                generatePrism(prism, profileshape, method, dir, L, L2,
                            Midplane.getValue(), Reversed.getValue());

            if (prism.isNull())
                return new App::DocumentObjectExecReturn("Pocket: Resulting shape is empty");

            // set the subtractive shape property for later usage in e.g. pattern
            prism = refineShapeIfActive(prism);
            this->AddSubShape.setValue(prism);

            prism.Tag = -this->getID();

            // Cut the SubShape out of the base feature
            TopoShape result(0,getDocument()->getStringHasher());
            try {
                if (NewSolid.getValue())
                    result = prism;
                else
                    result.makECut({base,prism});
            }catch(Standard_Failure &){
                return new App::DocumentObjectExecReturn("Pocket: Cut out of base feature failed");
            }
            // we have to get the solids (fuse sometimes creates compounds)
            auto solRes = this->getSolid(result);
            if (solRes.isNull())
                return new App::DocumentObjectExecReturn("Pocket: Resulting shape is not a solid");

            solRes = refineShapeIfActive(solRes);
            remapSupportShape(solRes.getShape());
            this->Shape.setValue(getSolid(solRes));
        }

        return App::DocumentObject::StdReturn;
    }
    catch (Standard_Failure& e) {

        if (std::string(e.GetMessageString()) == "TopoDS::Face" &&
            (method == "UpToFirst" || method == "UpToFace"))
            return new App::DocumentObjectExecReturn("Could not create face from sketch.\n"
                "Intersecting sketch entities or multiple faces in a sketch are not allowed "
                "for making a pocket up to a face.");
        else
            return new App::DocumentObjectExecReturn(e.GetMessageString());
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }
}

void Pocket::setupObject()
{
    ProfileBased::setupObject();
    UsePipeForDraft.setValue(Part::PartParams::getUsePipeForExtrusionDraft());
    Linearize.setValue(Part::PartParams::getLinearizeExtrusionDraft());
}

