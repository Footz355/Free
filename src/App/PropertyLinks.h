/***************************************************************************
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2002     *
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


#ifndef APP_PROPERTYLINKS_H
#define APP_PROPERTYLINKS_H

// Std. configurations


#include <vector>
#include <string>
#include <memory>
#include <cinttypes>
#include "Property.h"

namespace Base {
class Writer;
}

namespace App
{
class DocumentObject;
class Document;

/** the general Link Poperty
 *  Main Purpose of this property is to Link Objects and Feautures in a document.
 */
class AppExport PropertyLink : public Property
{
    TYPESYSTEM_HEADER();

public:
    /**
     * A constructor.
     * A more elaborate description of the constructor.
     */
    PropertyLink();

    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyLink();

    /** Sets the property
     */
    virtual void setValue(App::DocumentObject *);

    /** This method returns the linked DocumentObject
     */
    App::DocumentObject * getValue(void) const;

    /** Returns the link type checked
     */
    App::DocumentObject * getValue(Base::Type t) const;

   /** Returns the link type checked
     */
    template <typename _type>
    inline _type getValue(void) const {
        return _pcLink ? dynamic_cast<_type>(_pcLink) : 0;
    }

    virtual PyObject *getPyObject(void);
    virtual void setPyObject(PyObject *);

    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);

    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);

    virtual unsigned int getMemSize (void) const{
        return sizeof(App::DocumentObject *);
    }
    virtual const char* getEditorName(void) const
    { return "Gui::PropertyEditor::PropertyLinkItem"; }

protected:
    App::DocumentObject *_pcLink;
};

class AppExport PropertyLinkList : public PropertyListsT<DocumentObject*>
{
    TYPESYSTEM_HEADER();
    typedef PropertyListsT<DocumentObject*> inherited;

public:
    /**
    * A constructor.
    * A more elaborate description of the constructor.
    */
    PropertyLinkList();

    /**
    * A destructor.
    * A more elaborate description of the destructor.
    */
    virtual ~PropertyLinkList();

    virtual void setSize(int newSize);
    virtual void setSize(int newSize, const_reference def);

    /** Sets the property
    */
    void setValues(const std::vector<DocumentObject*>&) override;

    void set1Value(int idx, DocumentObject * const &value, bool touch=false) override;

    virtual PyObject *getPyObject(void);

    virtual void Save(Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);

    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);

    virtual unsigned int getMemSize(void) const;

    DocumentObject *find(const char *, int *pindex=0) const;

protected:
    DocumentObject *getPyValue(PyObject *item) const override;

protected:
    mutable std::map<std::string, int> _nameMap;
};

/** the Link Poperty with sub elements
 *  This property links an object and a defined sequence of
 *  sub elements. These subelements (like Edges of a Shape)
 *  are stored as names, which can be resolved by the 
 *  ComplexGeoDataType interface to concrete sub objects.
 */
class AppExport PropertyLinkSub: public Property
{
    TYPESYSTEM_HEADER();

public:
    /**
     * A constructor.
     * A more elaborate description of the constructor.
     */
    PropertyLinkSub();

    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyLinkSub();

    /** Sets the property
     */
    void setValue(App::DocumentObject *,const std::vector<std::string> &SubList=std::vector<std::string>());

    /** This method returns the linked DocumentObject
     */
    App::DocumentObject * getValue(void) const;

    /// return the list of sub elements 
    const std::vector<std::string>& getSubValues(void) const;

    /// return the list of sub elements starts with a special string 
    std::vector<std::string> getSubValuesStartsWith(const char*) const;

    /** Returns the link type checked
     */
    App::DocumentObject * getValue(Base::Type t) const;

   /** Returns the link type checked
     */
    template <typename _type>
    inline _type getValue(void) const {
        return _pcLinkSub ? dynamic_cast<_type>(_pcLinkSub) : 0;
    }

    virtual PyObject *getPyObject(void);
    virtual void setPyObject(PyObject *);

    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);

    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);

    /// Return a copy of the property if any changes caused by importing external object 
    Property *CopyOnImportExternal(const std::map<std::string,std::string> &nameMap) const;

    virtual unsigned int getMemSize (void) const{
        return sizeof(App::DocumentObject *);
    }

protected:
    App::DocumentObject *_pcLinkSub;
    std::vector<std::string> _cSubList;

};

class AppExport PropertyLinkSubList: public PropertyLists
{
    TYPESYSTEM_HEADER();

public:
    typedef std::pair<DocumentObject*, std::vector<std::string> > SubSet;
    /**
     * A constructor.
     * A more elaborate description of the constructor.
     */
    PropertyLinkSubList();

    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyLinkSubList();

    virtual void setSize(int newSize);
    virtual int getSize(void) const;

    /** Sets the property.
     * setValue(0, whatever) clears the property
     */
    void setValue(DocumentObject*,const char*);
    void setValues(const std::vector<DocumentObject*>&,const std::vector<const char*>&);
    void setValues(const std::vector<DocumentObject*>&,const std::vector<std::string>&);

    /**
     * @brief setValue: PropertyLinkSub-compatible overload
     * @param SubList
     */
    void setValue(App::DocumentObject *lValue, const std::vector<std::string> &SubList=std::vector<std::string>());

    const std::vector<DocumentObject*> &getValues(void) const {
        return _lValueList;
    }

    const std::string getPyReprString();

    /**
     * @brief getValue emulates the action of a single-object link.
     * @return reference to object, if the link os to only one object. NULL if
     * the link is empty, or links to subelements of more than one documant
     * object.
     */
    DocumentObject* getValue() const;

    const std::vector<std::string> &getSubValues(void) const {
        return _lSubList;
    }

    void setSubListValues(const std::vector<SubSet>&);
    std::vector<SubSet> getSubListValues() const;

    virtual PyObject *getPyObject(void);
    virtual void setPyObject(PyObject *);

    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);

    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);

    /// Return a copy of the property if any changes caused by importing external object 
    Property *CopyOnImportExternal(const std::map<std::string,std::string> &nameMap) const;

    virtual unsigned int getMemSize (void) const;

private:
    //FIXME: Do not make two independent lists because this will lead to some inconsistencies!
    std::vector<DocumentObject*> _lValueList;
    std::vector<std::string>     _lSubList;
};
/** Link to an (sub)object in the same or different document
 */
class AppExport PropertyXLink : public PropertyLink
{
    TYPESYSTEM_HEADER();

public:
    PropertyXLink();

    virtual ~PropertyXLink();

    void setValue(App::DocumentObject *) override;
    void setValue(App::DocumentObject *, const char *subname, bool relative);
    void setValue(const char *filePath, const char *objectName, const char *subname, bool relative);
    const char *getSubName() const {return subName.c_str();}
    bool hasSubName() const {return !subName.empty();}

    App::Document *getDocument() const;
    const char *getDocumentPath() const;
    const char *getObjectName() const;
    bool isRestored() const;

    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);

    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);

    /// Return a copy of the property if any changes caused by importing external object 
    Property *CopyOnImportExternal(const std::map<std::string,std::string> &nameMap) const;

    virtual PyObject *getPyObject(void);
    virtual void setPyObject(PyObject *);

    class DocInfo;
    friend class DocInfo;
    typedef std::shared_ptr<DocInfo> DocInfoPtr;

    static bool hasXLink(const App::Document *doc);

protected:
    void unlink();
    void detach();

protected:
    std::string filePath;
    std::string objectName;
    std::string subName;
    std::string stamp;
    bool relativePath;

    DocInfoPtr docInfo;
};


} // namespace App


#endif // APP_PROPERTYLINKS_H
