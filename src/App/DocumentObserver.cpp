/***************************************************************************
 *   Copyright (c) 2008 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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
#include "PropertyGeo.h"

#ifndef _PreComp_
# include <sstream>
#endif

#include <boost_bind_bind.hpp>

#include <Base/Tools.h>
#include <App/DocumentObjectPy.h>
#include "Application.h"
#include "Document.h"
#include "DocumentObject.h"
#include "DocumentObserver.h"
#include "ComplexGeoData.h"
#include "GeoFeature.h"
#include "Link.h"

using namespace App;
namespace bp = boost::placeholders;


DocumentT::DocumentT()
{
}

DocumentT::DocumentT(Document* doc)
{
    document = doc->getName();
}

DocumentT::DocumentT(const std::string& name)
{
    document = name;
}

DocumentT::DocumentT(const DocumentT& doc)
{
    document = doc.document;
}

DocumentT::~DocumentT()
{
}

void DocumentT::operator=(const DocumentT& doc)
{
    if (this == &doc)
        return;
    document = doc.document;
}

void DocumentT::operator=(const Document* doc)
{
    document = doc->getName();
}

void DocumentT::operator=(const std::string& name)
{
    document = name;
}

Document* DocumentT::getDocument() const
{
    return GetApplication().getDocument(document.c_str());
}

const std::string &DocumentT::getDocumentName() const
{
    return document;
}

std::string DocumentT::getDocumentPython() const
{
    std::stringstream str;
    str << "App.getDocument(\"" << document << "\")";
    return str.str();
}

// -----------------------------------------------------------------------------

DocumentObjectT::DocumentObjectT()
{
}

DocumentObjectT::DocumentObjectT(const DocumentObjectT &other)
{
    *this = other;
}

DocumentObjectT::DocumentObjectT(DocumentObjectT &&other)
{
    *this = std::move(other);
}

DocumentObjectT::DocumentObjectT(const DocumentObject* obj)
{
    *this = obj;
}

DocumentObjectT::DocumentObjectT(const Property* prop)
{
    *this = prop;
}

DocumentObjectT::DocumentObjectT(const char *docName, const char *objName)
{
    if(docName)
        document = docName;
    if(objName)
        object = objName;
}

DocumentObjectT::~DocumentObjectT()
{
}

DocumentObjectT &DocumentObjectT::operator=(const DocumentObjectT& obj)
{
    if (this == &obj)
        return *this;
    object = obj.object;
    label = obj.label;
    document = obj.document;
    property = obj.property;
    return *this;
}

DocumentObjectT &DocumentObjectT::operator=(DocumentObjectT&& obj)
{
    if (this == &obj)
        return *this;
    object = std::move(obj.object);
    label = std::move(obj.label);
    document = std::move(obj.document);
    property = std::move(obj.property);
    return *this;
}

void DocumentObjectT::operator=(const DocumentObject* obj)
{
    if(!obj || !obj->getNameInDocument()) {
        object.clear();
        label.clear();
        document.clear();
        property.clear();
    } else {
        object = obj->getNameInDocument();
        label = obj->Label.getValue();
        document = obj->getDocument()->getName();
        property.clear();
    }
}

void DocumentObjectT::operator=(const Property *prop) {
    if(prop && prop->getName() && prop->getContainer()) {
        if (prop->getContainer()->isDerivedFrom(App::DocumentObject::getClassTypeId())) {
            auto obj = static_cast<App::DocumentObject*>(prop->getContainer());
            object = obj->getNameInDocument();
            label = obj->Label.getValue();
            document = obj->getDocument()->getName();
            property = prop->getName();
            return;
        }
        if (prop->getContainer()->isDerivedFrom(App::Document::getClassTypeId())) {
            auto doc = static_cast<App::Document*>(prop->getContainer());
            object.clear();
            label = doc->Label.getValue();
            document = doc->getName();
            property = prop->getName();
            return;
        }
    }
    object.clear();
    label.clear();
    document.clear();
    property.clear();
}

bool DocumentObjectT::operator==(const DocumentObjectT &other) const {
    // ignore label
    return document == other.document
        && object == other.object
        && property == other.property;
}

bool DocumentObjectT::operator<(const DocumentObjectT &other) const {
    if(getDocumentName() < other.getDocumentName())
        return true;
    if(getDocumentName() > other.getDocumentName())
        return false;
    if(getObjectName() < other.getObjectName())
        return true;
    if(getObjectName() > other.getObjectName())
        return false;
    return getPropertyName() < other.getPropertyName();
}

Document* DocumentObjectT::getDocument() const
{
    return GetApplication().getDocument(document.c_str());
}

const std::string& DocumentObjectT::getDocumentName() const
{
    return document;
}

std::string DocumentObjectT::getDocumentPython() const
{
    std::stringstream str;
    str << "FreeCAD.getDocument(\"" << document << "\")";
    return str.str();
}

DocumentObject* DocumentObjectT::getObject() const
{
    DocumentObject* obj = 0;
    Document* doc = getDocument();
    if (doc) {
        obj = doc->getObject(object.c_str());
    }
    return obj;
}

const std::string &DocumentObjectT::getObjectName() const
{
    return object;
}

const std::string &DocumentObjectT::getObjectLabel() const
{
    return label;
}

std::string DocumentObjectT::getObjectPython() const
{
    std::stringstream str;
    str << "FreeCAD.getDocument('" << document << "').getObject('" << object << "')";
    return str.str();
}

const std::string &DocumentObjectT::getPropertyName() const {
    return property;
}

std::string DocumentObjectT::getPropertyPython() const
{
    std::stringstream str;
    str << "FreeCAD.getDocument('" << document
        << "').getObject('" << object
        << "')";
    if(property.size())
        str << '.' << property;
    return str.str();
}

Property *DocumentObjectT::getProperty() const {
    if (object.empty()) {
        auto doc = getDocument();
        return doc ? doc->getPropertyByName(property.c_str()) : nullptr;
    }
    auto obj = getObject();
    return obj ? obj->getPropertyByName(property.c_str()) : nullptr;
}

// -----------------------------------------------------------------------------

SubObjectT::SubObjectT()
{}

SubObjectT::SubObjectT(const SubObjectT &other)
    :DocumentObjectT(other), subname(other.subname)
{
}

SubObjectT::SubObjectT(SubObjectT &&other)
    :DocumentObjectT(std::move(other)), subname(std::move(other.subname))
{
}

SubObjectT::SubObjectT(const DocumentObject *obj, const char *s)
    :DocumentObjectT(obj),subname(s?s:"")
{
}

SubObjectT::SubObjectT(const DocumentObject *obj)
    :DocumentObjectT(obj)
{
}

SubObjectT::SubObjectT(const DocumentObjectT& obj, const char *s)
    :DocumentObjectT(obj),subname(s?s:"")
{
}

SubObjectT::SubObjectT(const char *docName, const char *objName, const char *s)
    :DocumentObjectT(docName,objName), subname(s?s:"")
{
}

SubObjectT::SubObjectT(const std::vector<App::DocumentObject*> &objs, const char *s)
{
    if (objs.empty())
        return;

    static_cast<DocumentObjectT&>(*this) = objs.front();
    std::ostringstream ss;
    for (auto it = objs.begin()+1; it!=objs.end(); ++it) {
        auto obj = *it;
        if (!obj || !obj->getNameInDocument())
            continue;
        ss << obj->getNameInDocument() << ".";
    }
    if (s)
        ss << s;
    subname = ss.str();
}

SubObjectT::SubObjectT(std::vector<App::DocumentObject*>::const_iterator begin,
                       std::vector<App::DocumentObject*>::const_iterator end,
                       const char *s)
{
    if (begin == end)
        return;

    static_cast<DocumentObjectT&>(*this) = *begin;
    std::ostringstream ss;
    for (auto it = begin+1; it != end; ++it) {
        auto obj = *it;
        if (!obj || !obj->getNameInDocument())
            continue;
        ss << obj->getNameInDocument() << ".";
    }
    if (s)
        ss << s;
    subname = ss.str();
}

bool SubObjectT::operator<(const SubObjectT &other) const {
    if(getDocumentName() < other.getDocumentName())
        return true;
    if(getDocumentName() > other.getDocumentName())
        return false;
    if(getObjectName() < other.getObjectName())
        return true;
    if(getObjectName() > other.getObjectName())
        return false;
    if(getSubName() < other.getSubName())
        return true;
    if(getSubName() > other.getSubName())
        return false;
    return getPropertyName() < other.getPropertyName();
}

SubObjectT &SubObjectT::operator=(const SubObjectT& other)
{
    if (this == &other)
        return *this;
    static_cast<DocumentObjectT&>(*this) = other;
    subname = other.subname;
    return *this;
}

SubObjectT &SubObjectT::operator=(SubObjectT &&other)
{
    if (this == &other)
        return *this;
    static_cast<DocumentObjectT&>(*this) = std::move(other);
    subname = std::move(other.subname);
    return *this;
}

SubObjectT &SubObjectT::operator=(const DocumentObjectT &other)
{
    if (this == &other)
        return *this;
    static_cast<DocumentObjectT&>(*this) = other;
    subname.clear();
    return *this;
}

SubObjectT &SubObjectT::operator=(const DocumentObject *other)
{
    static_cast<DocumentObjectT&>(*this) = other;
    subname.clear();
    return *this;
}

bool SubObjectT::operator==(const SubObjectT &other) const {
    return static_cast<const DocumentObjectT&>(*this) == other
        && subname == other.subname;
}

void SubObjectT::setSubName(const char *s) {
    subname = s?s:"";
}

bool SubObjectT::isVisible() const
{
    auto obj = getObject();
    if (!obj)
        return false;
    int vis = obj->isElementVisibleEx(subname.c_str(), DocumentObject::GS_SELECT);
    if (vis >= 0)
        return vis > 0;
    auto sobj = getSubObject();
    return sobj && sobj->Visibility.getValue();
}

bool SubObjectT::normalize(NormalizeOptions options)
{
    bool noElement = options & NoElement;
    bool flatten = !(options & NoFlatten);
    bool keepSub = options & KeepSubName;
    bool convertIndex = options & ConvertIndex;

    std::ostringstream ss;
    std::vector<int> subs;
    auto obj = getObject();
    if(!obj)
        return false;
    auto objs = obj->getSubObjectList(subname.c_str(), &subs, flatten);
    if (objs.empty())
        return false;
    for (unsigned i=1; i<objs.size(); ++i) {
        // Keep digit only subname, as it maybe an index to an array, which does
        // not expand its elements as objects.
        const char *end = subname.c_str() + subs[i];
        const char *sub = end - 2;
        for(;;--sub) {
            if (sub < subname.c_str()) {
                sub = subname.c_str();
                break;
            }
            if (*sub == '.') {
                ++sub;
                break;
            }
        }
        bool _keepSub;
        if (!std::isdigit(sub[0]))
            _keepSub = keepSub;
        else if (!convertIndex)
            _keepSub = true;
        else {
            _keepSub = false;
            if (auto ext = objs[i-1]->getExtensionByType<App::LinkBaseExtension>(true)) {
                if (ext->getElementCountValue() && !ext->getShowElementValue()) {
                    // if the parent is a collapsed link array element, then we
                    // have to keep the index no matter what, because there is
                    // no sub-object corresponding to an array element.
                    _keepSub = true;
                }
            }
        }
        if (_keepSub)
            ss << std::string(sub, end);
        else
            ss << objs[i]->getNameInDocument() << ".";
    }
    if (objs.size() > 1 && objs.front()->getSubObject(ss.str().c_str()) != objs.back()) {
        // something went wrong
        return false;
    }
    if (!noElement)
        ss << getOldElementName();
    std::string sub = ss.str();
    if (objs.front() != obj || subname != sub) {
        *this = objs.front();
        subname = std::move(sub);
        return true;
    }
    return false;
}

SubObjectT App::SubObjectT::normalized(NormalizeOptions options) const
{
    SubObjectT res(*this);
    res.normalize(options);
    return res;
}

const std::string &SubObjectT::getSubName() const {
    return subname;
}

std::string SubObjectT::getSubNameNoElement(bool withObjName) const {
    if (!withObjName)
        return Data::ComplexGeoData::noElementName(subname.c_str());
    std::string res(getObjectName());
    res += ".";
    const char * element = Data::ComplexGeoData::findElementName(subname.c_str());
    if(element)
        return res.insert(res.size(), subname.c_str(), element - subname.c_str());
    return res;
}

const char *SubObjectT::getElementName() const {
    return Data::ComplexGeoData::findElementName(subname.c_str());
}

bool SubObjectT::hasSubObject() const {
    return Data::ComplexGeoData::findElementName(subname.c_str()) != subname.c_str();
}

bool SubObjectT::hasSubElement() const {
    auto element = getElementName();
    return element && element[0];
}

std::string SubObjectT::getNewElementName(bool fallback) const {
    const char *elementName = Data::ComplexGeoData::findElementName(subname.c_str());
    if (!elementName || !elementName[0])
        return std::string();
    std::string name = Data::ComplexGeoData::newElementName(elementName);
    if (name.size())
        return name;

    std::pair<std::string, std::string> element;
    auto obj = getObject();
    if(!obj)
        return std::string();
    GeoFeature::resolveElement(obj,subname.c_str(),element);
    if (!element.first.empty() || !fallback)
        return std::move(element.first);
    return std::move(element.second);
}

std::string SubObjectT::getOldElementName(int *index, bool fallback) const {
    const char *elementName = Data::ComplexGeoData::findElementName(subname.c_str());
    if (!elementName || !elementName[0])
        return std::string();
    std::string name = Data::ComplexGeoData::oldElementName(elementName);
    if (name.size())
        return name;

    std::pair<std::string, std::string> element;
    auto obj = getObject();
    if(!obj)
        return std::string();
    GeoFeature::resolveElement(obj,subname.c_str(),element);
    if(index) {
        std::size_t pos = element.second.find_first_of("0123456789");
        if(pos == std::string::npos)
            *index = -1;
        else {
            *index = std::atoi(element.second.c_str()+pos);
            element.second.resize(pos);
        }
    }
    if (!element.second.empty() || !fallback)
        return std::move(element.second);
    return std::move(element.first);
}

App::DocumentObject *SubObjectT::getSubObject() const {
    auto obj = getObject();
    if(obj)
        return obj->getSubObject(subname.c_str());
    return 0;
}

std::string SubObjectT::getSubObjectPython(bool force) const {
    if(!force && subname.empty())
        return getObjectPython();
    std::stringstream str;
    str << "(" << getObjectPython() << ", '"
        << Base::Tools::escapedUnicodeFromUtf8(normalized().subname.c_str()) << "')";
    return str.str();
}

std::vector<App::DocumentObject*> SubObjectT::getSubObjectList(bool flatten) const {
    auto obj = getObject();
    if(obj)
        return obj->getSubObjectList(subname.c_str(), nullptr, flatten);
    return {};
}

SubObjectT SubObjectT::getParent() const {
    const char *pos  = Data::ComplexGeoData::findElementName(subname.c_str());
    if (!pos || pos == subname.c_str())
        return SubObjectT();

    if(*pos != '.')
        --pos;
    if(--pos <= subname.c_str())
        return SubObjectT();

    bool found = false;
    for(;pos!=subname.c_str();--pos) {
        if(*pos != '.') {
            found = true;
            continue;
        }
        if(found)
            return SubObjectT(getDocumentName().c_str(),
                    getObjectName().c_str(), std::string(subname.c_str(), pos+1).c_str());
    }
    return SubObjectT(getDocumentName().c_str(), getObjectName().c_str(), nullptr);
}

SubObjectT SubObjectT::getChild(const App::DocumentObject *child) const
{
    if (getObjectName().empty())
        return SubObjectT(child);
    SubObjectT res = *this;
    res.setSubName(getSubNameNoElement() + child->getNameInDocument() + ".");
    return res;
}

std::string SubObjectT::getObjectFullName(const char *docName) const
{
    std::ostringstream ss;
    if (!docName || getDocumentName() != docName) {
        ss << getDocumentName();
        if (auto doc = getDocument()) {
            if (doc->Label.getStrValue() != getDocumentName())
                ss << "(" << doc->Label.getValue() << ")";
        }
        ss << "#";
    }
    ss << getObjectName();
    if (getObjectLabel().size() && getObjectLabel() != getObjectName())
        ss << " (" << getObjectLabel() << ")";
    return ss.str();
}

std::string SubObjectT::getSubObjectFullName(const char *docName) const
{
    if (subname.empty())
        return getObjectFullName(docName);
    std::ostringstream ss;
    if (!docName || getDocumentName() != docName) {
        ss << getDocumentName();
        if (auto doc = getDocument()) {
            if (doc->Label.getStrValue() != getDocumentName())
                ss << "(" << doc->Label.getValue() << ")";
        }
        ss << "#";
    }
    ss << getObjectName() << "." << subname;
    auto sobj = getSubObject();
    if (sobj && sobj->Label.getStrValue() != sobj->getNameInDocument())
        ss << " (" << sobj->Label.getValue() << ")";
    return ss.str();
}

PyObject *SubObjectT::getPyObject() const
{
    auto obj = getObject();
    if (!obj)
        return Py::new_reference_to(Py::Object());
    if (subname.empty())
        return obj->getPyObject();
    return Py::new_reference_to(Py::TupleN(
                Py::asObject(obj->getPyObject()), Py::String(subname)));
}

void SubObjectT::setPyObject(PyObject *pyobj)
{
    try {
        if (!pyobj)
            throw Base::ValueError("Invalid object");
        if (PyObject_TypeCheck(pyobj, &App::DocumentObjectPy::Type)) {
            this->operator=(static_cast<App::DocumentObjectPy*>(pyobj)->getDocumentObjectPtr());
            return;
        } else if (PySequence_Check(pyobj) && PySequence_Size(pyobj)==2) {
            Py::Sequence seq(pyobj);
            App::DocumentObject *obj;
            PropertyString tmp;
            if (PyObject_TypeCheck(seq[0].ptr(), &App::DocumentObjectPy::Type)) {
                obj = static_cast<App::DocumentObjectPy*>(seq[0].ptr())->getDocumentObjectPtr();
                tmp.setPyObject(seq[1].ptr());
                this->operator=(App::SubObjectT(obj, tmp.getValue()));
                return;
            }
        }
    } catch (Py::Exception &) {
        Base::PyException e;
    } catch (...) {
    }
    throw Base::ValueError("Expect either document object or tuple(obj, subname)");
}
// -----------------------------------------------------------------------------

class DocumentWeakPtrT::Private {
public:
    Private(App::Document* doc) : _document(doc) {
        if (doc) {
            connectApplicationDeletedDocument = App::GetApplication().signalDeleteDocument.connect(boost::bind
                (&Private::deletedDocument, this, bp::_1));
        }
    }

    void deletedDocument(const App::Document& doc) {
        if (_document == &doc)
            reset();
    }
    void reset() {
        connectApplicationDeletedDocument.disconnect();
        _document = nullptr;
    }

    App::Document* _document;
    typedef boost::signals2::scoped_connection Connection;
    Connection connectApplicationDeletedDocument;
};

DocumentWeakPtrT::DocumentWeakPtrT(App::Document* doc) noexcept
  : d(new Private(doc))
{
}

DocumentWeakPtrT::~DocumentWeakPtrT()
{
}

void DocumentWeakPtrT::reset() noexcept
{
    d->reset();
}

bool DocumentWeakPtrT::expired() const noexcept
{
    return (d->_document == nullptr);
}

App::Document* DocumentWeakPtrT::operator->() noexcept
{
    return d->_document;
}

// -----------------------------------------------------------------------------

class DocumentObjectWeakPtrT::Private {
public:
    Private(App::DocumentObject* obj) : object(obj), indocument(false) {
        set(obj);
    }
    void deletedDocument(const App::Document& doc) {
        // When deleting document then there is no way to undo it
        if (object && object->getDocument() == &doc) {
            reset();
        }
    }
    void createdObject(const App::DocumentObject& obj) noexcept {
        // When undoing the removal
        if (object == &obj) {
            indocument = true;
        }
    }
    void deletedObject(const App::DocumentObject& obj) noexcept {
        if (object == &obj) {
            indocument = false;
        }
    }
    void reset() {
        connectApplicationDeletedDocument.disconnect();
        connectDocumentCreatedObject.disconnect();
        connectDocumentDeletedObject.disconnect();
        object = nullptr;
        indocument = false;
    }
    void set(App::DocumentObject* obj) {
        object = obj;
        if (obj) {
            indocument = true;
            connectApplicationDeletedDocument = App::GetApplication().signalDeleteDocument.connect(boost::bind
            (&Private::deletedDocument, this, bp::_1));
            App::Document* doc = obj->getDocument();
            connectDocumentCreatedObject = doc->signalNewObject.connect(boost::bind
            (&Private::createdObject, this, bp::_1));
            connectDocumentDeletedObject = doc->signalDeletedObject.connect(boost::bind
            (&Private::deletedObject, this, bp::_1));
        }
    }
    App::DocumentObject* get() const noexcept {
        return indocument ? object : nullptr;
    }

    App::DocumentObject* object;
    bool indocument;
    typedef boost::signals2::scoped_connection Connection;
    Connection connectApplicationDeletedDocument;
    Connection connectDocumentCreatedObject;
    Connection connectDocumentDeletedObject;
};

DocumentObjectWeakPtrT::DocumentObjectWeakPtrT(App::DocumentObject* obj)
  : d(new Private(obj))
{
}

DocumentObjectWeakPtrT::~DocumentObjectWeakPtrT()
{

}

App::DocumentObject* DocumentObjectWeakPtrT::_get() const noexcept
{
    return d->get();
}

void DocumentObjectWeakPtrT::reset()
{
    d->reset();
}

bool DocumentObjectWeakPtrT::expired() const noexcept
{
    return !d->indocument;
}

DocumentObjectWeakPtrT& DocumentObjectWeakPtrT::operator= (App::DocumentObject* p)
{
    d->reset();
    d->set(p);
    return *this;
}

App::DocumentObject* DocumentObjectWeakPtrT::operator->() noexcept
{
    return d->get();
}

bool DocumentObjectWeakPtrT::operator== (const DocumentObjectWeakPtrT& p) const noexcept
{
    return d->get() == p.d->get();
}

bool DocumentObjectWeakPtrT::operator!= (const DocumentObjectWeakPtrT& p) const noexcept
{
    return d->get() != p.d->get();
}

// -----------------------------------------------------------------------------

DocumentObserver::DocumentObserver() : _document(nullptr)
{
    this->connectApplicationCreatedDocument = App::GetApplication().signalNewDocument.connect(boost::bind
        (&DocumentObserver::slotCreatedDocument, this, bp::_1));
    this->connectApplicationDeletedDocument = App::GetApplication().signalDeleteDocument.connect(boost::bind
        (&DocumentObserver::slotDeletedDocument, this, bp::_1));
}

DocumentObserver::DocumentObserver(Document* doc) : _document(nullptr)
{
    // Connect to application and given document
    this->connectApplicationCreatedDocument = App::GetApplication().signalNewDocument.connect(boost::bind
        (&DocumentObserver::slotCreatedDocument, this, bp::_1));
    this->connectApplicationDeletedDocument = App::GetApplication().signalDeleteDocument.connect(boost::bind
        (&DocumentObserver::slotDeletedDocument, this, bp::_1));
    attachDocument(doc);
}

DocumentObserver::~DocumentObserver()
{
    // disconnect from application and document
    this->connectApplicationCreatedDocument.disconnect();
    this->connectApplicationDeletedDocument.disconnect();
    detachDocument();
}

Document* DocumentObserver::getDocument() const
{
    return this->_document;
}

void DocumentObserver::attachDocument(Document* doc)
{
    if (_document != doc) {
        detachDocument();
        _document = doc;

        this->connectDocumentCreatedObject = _document->signalNewObject.connect(boost::bind
            (&DocumentObserver::slotCreatedObject, this, bp::_1));
        this->connectDocumentDeletedObject = _document->signalDeletedObject.connect(boost::bind
            (&DocumentObserver::slotDeletedObject, this, bp::_1));
        this->connectDocumentChangedObject = _document->signalChangedObject.connect(boost::bind
            (&DocumentObserver::slotChangedObject, this, bp::_1, bp::_2));
        this->connectDocumentRecomputedObject = _document->signalRecomputedObject.connect(boost::bind
            (&DocumentObserver::slotRecomputedObject, this, bp::_1));
        this->connectDocumentRecomputed = _document->signalRecomputed.connect(boost::bind
            (&DocumentObserver::slotRecomputedDocument, this, bp::_1));
    }
}

void DocumentObserver::detachDocument()
{
    if (this->_document) {
        this->_document = nullptr;
        this->connectDocumentCreatedObject.disconnect();
        this->connectDocumentDeletedObject.disconnect();
        this->connectDocumentChangedObject.disconnect();
        this->connectDocumentRecomputedObject.disconnect();
        this->connectDocumentRecomputed.disconnect();
    }
}

void DocumentObserver::slotCreatedDocument(const App::Document& /*Doc*/)
{
}

void DocumentObserver::slotDeletedDocument(const App::Document& /*Doc*/)
{
}

void DocumentObserver::slotCreatedObject(const App::DocumentObject& /*Obj*/)
{
}

void DocumentObserver::slotDeletedObject(const App::DocumentObject& /*Obj*/)
{
}

void DocumentObserver::slotChangedObject(const App::DocumentObject& /*Obj*/, const App::Property& /*Prop*/)
{
}

void DocumentObserver::slotRecomputedObject(const DocumentObject& /*Obj*/)
{
}

void DocumentObserver::slotRecomputedDocument(const Document& /*doc*/)
{
}


// -----------------------------------------------------------------------------

DocumentObjectObserver::DocumentObjectObserver()
{
}

DocumentObjectObserver::~DocumentObjectObserver()
{
}

DocumentObjectObserver::const_iterator DocumentObjectObserver::begin() const
{
    return _objects.begin();
}

DocumentObjectObserver::const_iterator DocumentObjectObserver::end() const
{
    return _objects.end();
}

void DocumentObjectObserver::addToObservation(App::DocumentObject* obj)
{
    _objects.insert(obj);
}

void DocumentObjectObserver::removeFromObservation(App::DocumentObject* obj)
{
    _objects.erase(obj);
}

void DocumentObjectObserver::slotCreatedDocument(const App::Document&)
{
}

void DocumentObjectObserver::slotDeletedDocument(const App::Document& Doc)
{
    if (this->getDocument() == &Doc) {
        this->detachDocument();
        _objects.clear();
        cancelObservation();
    }
}

void DocumentObjectObserver::slotCreatedObject(const App::DocumentObject&)
{
}

void DocumentObjectObserver::slotDeletedObject(const App::DocumentObject& Obj)
{
    std::set<App::DocumentObject*>::iterator it = _objects.find
        (const_cast<App::DocumentObject*>(&Obj));
    if (it != _objects.end())
        _objects.erase(it);
    if (_objects.empty())
        cancelObservation();
}

void DocumentObjectObserver::slotChangedObject(const App::DocumentObject&,
                                               const App::Property&)
{
}

void DocumentObjectObserver::cancelObservation()
{
}
