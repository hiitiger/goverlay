#include "stable.h"
#include "../third/pugixml.hpp"
#include "xmldocument.h"


namespace Storm{

    class XmlElementImpl
    {
    public:
        XmlElement* parent;
        std::wstring name;
        std::wstring text;
        std::vector<std::pair<std::wstring, std::wstring>> attributes;
        std::vector<XmlElement*> children;
    };

    XmlElement::XmlElement(const std::wstring& name)
        : dptr_(new XmlElementImpl())
    {
        dptr_->parent = nullptr;
        dptr_->name = name;
    }

    XmlElement::~XmlElement()
    {
        for (auto it = dptr_->children.begin(); it != dptr_->children.end(); ++it)
        {
            delete *it;
        }

        dptr_->children.clear();
    }

    XmlElement* XmlElement::parentElement() const
    {
        return dptr_->parent;
    }

    std::wstring XmlElement::name() const
    {
        return dptr_->name;
    }

    std::wstring XmlElement::text() const
    {
        return dptr_->text;
    }

    void XmlElement::setText(const std::wstring& text)
    {
        dptr_->text = text;
    }

    int XmlElement::childCount() const
    {
        return static_cast<int>(dptr_->children.size());
    }

    XmlElement* XmlElement::childAt(int index) const
    {
        if (index < 0 || index >= childCount())
        {
            return NULL;
        }
        return dptr_->children.at(index);
    }

    XmlElement* XmlElement::child(const std::wstring& name) const
    {
        auto it = std::find_if(dptr_->children.begin(), dptr_->children.end(), [&](XmlElement* elem) {return elem->name() == name; });
        if (it != dptr_->children.end())
        {
            return (*it);
        }
        else
        {
            return nullptr;
        }
    }

    XmlElement* XmlElement::addChild(const std::wstring& name)
    {
        XmlElement* child = new XmlElement(name);
        addChild(child);
        return child;
    }

    void XmlElement::addChild(XmlElement* child)
    {
        child->dptr_->parent = this;

        dptr_->children.push_back(child);
    }

    void XmlElement::deleteChild(XmlElement* child)
    {
        auto it = std::find(dptr_->children.begin(), dptr_->children.end(), child);
        if (it != dptr_->children.end())
        {
            delete child;
            dptr_->children.erase(it);
        }
    }

    const std::vector<XmlElement*> XmlElement::children() const
    {
        return dptr_->children;
    }

    int XmlElement::attributeCount() const
    {
        return static_cast<int>(dptr_->attributes.size());
    }

    bool XmlElement::hasAttribute(const std::wstring& attrName) const
    {
        return std::find_if(dptr_->attributes.begin(), dptr_->attributes.end(), [&](const std::pair<std::wstring, std::wstring>& attr) {return attr.first == attrName; }) != dptr_->attributes.end();
    }

    std::wstring XmlElement::attributeName(int index) const
    {
        if (index < 0 || index >= attributeCount())
        {
            return L"";
        }
        return dptr_->attributes.at(index).first;
    }

    std::wstring XmlElement::attributeValue(int index) const
    {
        if (index < 0 || index >= attributeCount())
        {
            return L"";
        }
        return dptr_->attributes.at(index).second;
    }


    std::wstring XmlElement::attributeValue(const std::wstring& attrName) const
    {
        for (auto it = dptr_->attributes.begin(); it != dptr_->attributes.end(); ++it)
        {
            if (it->first == attrName)
            {
                return it->second;
            }
        }
        return L"";
    }

    void XmlElement::setAttribute(const std::wstring& attrName, const std::wstring& attrValue)
    {
        auto it = std::find_if(dptr_->attributes.begin(), dptr_->attributes.end(), [&](const std::pair<std::wstring, std::wstring>& attr) {return attr.first == attrName; });
        if (it != dptr_->attributes.end())
        {
            it->second = attrValue;
        }
        else
        {
            dptr_->attributes.push_back(std::make_pair(attrName, attrValue));
        }
    }

    const std::vector<std::pair<std::wstring, std::wstring>> XmlElement::attributes() const
    {
        return dptr_->attributes;
    }

    class xmlwriter_wchar : public pugi::xml_writer
    {
    public:
        std::wstring& m_output;
        xmlwriter_wchar(std::wstring& output)
            : m_output(output)
        {

        }
        virtual void write(const void* data, size_t size)
        {
            m_output.append((const wchar_t*)(data), size);
        }
    };

    class xmlwriter_uft8 : public pugi::xml_writer
    {
    public:
        std::string& m_output;
        xmlwriter_uft8(std::string& output)
            : m_output(output)
        {

        }
        virtual void write(const void* data, size_t size)
        {
            m_output.append((const char*)(data), size);
        }
    };

    class xmlwriter_uft8_file : public pugi::xml_writer
    {
    public:
        File& m_file;
        xmlwriter_uft8_file(File& file)
            : m_file(file)
        {

        }
        virtual void write(const void* data, size_t size)
        {
            m_file.write((const uint8_t*)(data), size);
        }
    };

    void saveToXmlDoc(pugi::xml_node& parent, XmlElement* elem)
    {
        pugi::xml_node root = parent.append_child(elem->name().c_str());
        pugi::xml_node textnode = root.append_child(pugi::node_pcdata);
        textnode.set_value(elem->text().c_str());
        
        const auto& attrs = elem->attributes();
        for (auto it = attrs.begin(); it != attrs.end(); ++it)
        {
            root.append_attribute(it->first.c_str()).set_value(it->second.c_str());
        }

        const auto& children = elem->children();
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            saveToXmlDoc(root, *it);
        }

    }
    void XmlElement::saveToWString(std::wstring& wstr)
    {
        pugi::xml_document xmlDoc;
        saveToXmlDoc(xmlDoc, this);
        xmlDoc.save(xmlwriter_wchar(wstr), L"\t", pugi::format_default, pugi::encoding_wchar);
    }

    void XmlElement::saveToUtf8(std::string& utf8)
    {
        pugi::xml_document xmlDoc;
        saveToXmlDoc(xmlDoc, this);
        xmlDoc.save(xmlwriter_uft8(utf8), L"\t", pugi::format_default, pugi::encoding_utf8);
    }

    void XmlElement::saveToFile(const std::wstring& filename)
    {
        pugi::xml_document xmlDoc;
        saveToXmlDoc(xmlDoc, this);
        File file(filename);
        file.open(File::ReadWrite | File::Truncate);
        xmlDoc.save(xmlwriter_uft8_file(file), L"\t", pugi::format_default, pugi::encoding_utf8);
    }

    //////////////////////////////////////////////////////////////////////////
    XmlDocument::XmlDocument()
        : rootElem_(nullptr)
    {

    }

    XmlDocument::~XmlDocument()
    {

    }

    void makeChildren(pugi::xml_node& elem, XmlElement* parent)
    {
        pugi::xml_node childElem = elem.first_child();
        while (childElem)
        {
            if (childElem.type() == pugi::node_element)
            {
                XmlElement* child = new XmlElement(childElem.name());

                child->setText(childElem.text().get());

                pugi::xml_attribute attr = childElem.first_attribute();
                while (attr)
                {
                    child->setAttribute(attr.name(), attr.value());
                    attr = attr.next_attribute();
                }

                parent->addChild(child);
                makeChildren(childElem, child);
            }

            childElem = childElem.next_sibling();
        }
    }

    bool XmlDocument::loadFromUtf8(const std::string& utf8Data)
    {
        pugi::xml_document xmlDoc;
        std::string data = utf8Data;
        pugi::xml_parse_result result = xmlDoc.load_buffer_inplace((void*)data.c_str(), data.size(), pugi::parse_default, pugi::encoding_utf8);
        bool ok = result.status == pugi::status_ok;
        if (ok)
        {
            pugi::xml_node root = xmlDoc.first_child();
            rootElem_.reset(new XmlElement(root.name()));
            rootElem_->setText(root.text().get());

            pugi::xml_attribute attr = root.first_attribute();
            while (attr)
            {
                rootElem_->setAttribute(attr.name(), attr.value());
                attr = attr.next_attribute();
            }

            makeChildren(root, rootElem_.get());
        }

        return ok;
    }

    bool XmlDocument::loadFromFile(const std::wstring& filename)
    {
        File file(filename);
        if (file.open(File::ReadOnly))
        {
            std::string data;
            data.resize((size_t)file.size());
            file.read(const_cast<uint8_t*>((const uint8_t*)data.c_str()), data.size());

            pugi::xml_document xmlDoc;
            pugi::xml_parse_result result = xmlDoc.load_buffer_inplace((void*)data.c_str(), data.size(), pugi::parse_default, pugi::encoding_auto);
            bool ok = result.status == pugi::status_ok;
            if (ok)
            {
                pugi::xml_node root = xmlDoc.first_child();
                rootElem_.reset(new XmlElement(root.name()));
                rootElem_->setText(root.text().get());

                pugi::xml_attribute attr = root.first_attribute();
                while (attr)
                {
                    rootElem_->setAttribute(attr.name(), attr.value());
                    attr = attr.next_attribute();
                }

                makeChildren(root, rootElem_.get());
            }

            return ok;
        }
        else
        {
            return false;
        }
    }

    bool XmlDocument::resetRootElement(const std::wstring& name)
    {
        rootElem_.reset(new XmlElement(name));
        return true;
    }

    XmlElement* XmlDocument::elementByPath(const std::wstring& path) const
    {
        std::vector<std::wstring> keyPath = Utils::split(path, L'/');
        keyPath.erase(std::remove(keyPath.begin(), keyPath.end(), L""), keyPath.end());

        XmlElement* elem = rootElem_.get();
        if (!elem || keyPath.size() == 0 || elem->name() != keyPath[0])
        {
            return nullptr;
        }

        for (int i = 1; i != keyPath.size(); ++i)
        {
            if (elem != nullptr)
            {
                elem = elem->child(keyPath[i]);
            }
            else
            {
                break;
            }
        }

        return elem;

    }

    XmlElement* XmlDocument::makeElementByPath(const std::wstring& path)
    {
        std::vector<std::wstring> keyPath = Utils::split(path, L'/');
        keyPath.erase(std::remove(keyPath.begin(), keyPath.end(), L""), keyPath.end());

        XmlElement* elem = rootElem_.get();
        if (!elem || keyPath.size() == 0 || elem->name() != keyPath[0])
        {
            return nullptr;
        }

        int i = 1;
        for (; i != keyPath.size(); ++i)
        {
            if (!keyPath[i].empty())
            {
                if (elem->child(keyPath[i]))
                {
                    elem = elem->child(keyPath[i]);
                }
                else
                {
                    break;
                }
            }
        }
        if (i == keyPath.size())
        {
            return elem;
        }
        else
        {
            for (; i != keyPath.size(); ++i)
            {
                elem = elem->addChild(keyPath[i]);
            }
            return elem;
        }
    }
}
