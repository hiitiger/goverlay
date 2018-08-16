#pragma once


namespace Storm{

    class XmlElementImpl;

    class XmlElement
    {
        STORM_NONCOPYABLE(XmlElement)
    public:
        explicit XmlElement(const std::wstring& name);
        ~XmlElement();

        XmlElement* parentElement() const;

        std::wstring name() const;
        std::wstring text() const;
        void setText(const std::wstring& text);

        int childCount() const;
        XmlElement* childAt(int index) const;
        XmlElement* child(const std::wstring& name) const;
        XmlElement* addChild(const std::wstring& name);
        void addChild(XmlElement* child);
        void deleteChild(XmlElement* child);

        const std::vector<XmlElement*> children() const;

        int attributeCount() const;
        std::wstring attributeName(int index) const;
        std::wstring attributeValue(int index) const;
        std::wstring attributeValue(const std::wstring& attrName) const;
        bool hasAttribute(const std::wstring& attrName) const;
        void setAttribute(const std::wstring& attrName, const std::wstring& attrValue);

        const std::vector<std::pair<std::wstring, std::wstring>> attributes() const;

        void saveToWString(std::wstring& wstr);
        void saveToUtf8(std::string& utf8);
        void saveToFile(const std::wstring& filename);

    private:
        std::unique_ptr<XmlElementImpl> dptr_;
    };


    class XmlDocument
    {
        STORM_NONCOPYABLE(XmlDocument)
    public:
        XmlDocument();
        ~XmlDocument();

        bool loadFromUtf8(const std::string& utf8Data);
        bool loadFromFile(const std::wstring& file);
        bool resetRootElement(const std::wstring& name);

        XmlElement* rootElement() const { return rootElem_.get(); }

        XmlElement* elementByPath(const std::wstring& path) const;
        XmlElement* makeElementByPath(const std::wstring& path);

        bool valid() const { return rootElement() != nullptr; }

    private:
        std::unique_ptr<XmlElement> rootElem_;
    };

}
