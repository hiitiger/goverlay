#pragma once

namespace Storm {

    namespace priv
    {
        enum { K_MaxLevelNum = 16 };
        template<class K, class V>
        struct SkiplistNode {
            SkiplistNode* backword = nullptr;
            SkiplistNode* forward[K_MaxLevelNum] = { nullptr };

            std::pair<K, V> data;

            SkiplistNode() = default;

            SkiplistNode(const std::pair<K, V>& item)
                : data(item)
            {
            }
            SkiplistNode(std::pair<K, V>&& item)
                : data(std::move(item))
            {
            }
        };
    }

    template<class K, class V>
    class Skiplist
    {
    public:
        typedef std::pair<K, V> DataType;
        using Node = priv::SkiplistNode<K, V>;
    private:
        Node* header_ = nullptr;
        int level_ = 0;
        int size_ = 0;

        int randomLevel();

        Node* createNode();
        Node* createNode(const std::pair<K, V>& item);
        Node* createNode(std::pair<K, V>&& item);

        Node* findNode(const K& key) const;

        const Node* nodeAt(int index) const;
        Node* nodeAt(int index);

        Node* createHeader();
        void copyConstruct(Node* header);
        void destroy();

        void swap(Skiplist& other);

    public:
        typedef ptrdiff_t difference_type;
        typedef std::pair<K, V> value_type;
        typedef std::pair<K, V>* pointer;
        typedef std::pair<K, V>& reference;

        Skiplist();

        Skiplist(const Skiplist& other);
        Skiplist(Skiplist&& other);

        Skiplist& operator=(const Skiplist& other);
        Skiplist& operator=(Skiplist&& other);

        ~Skiplist();

        int size() const;    
        bool contains(const K& key) const;

        const V value(const K& key) const;
        const K key(const V& value) const;

        const K& keyAt(int index) const;
        const V& valueAt(int index) const;
        V& valueAt(int index);

        const V& operator[](const K& key)  const;
        V& operator[](const K& key);

        void insert(const K&, const V&);
        void insert(K&&, V&&);

        template<class ValueType>
        void insert(ValueType&&);
        
        void remove(const K& key);

        class iterator : public std::iterator<std::bidirectional_iterator_tag, V>
        {
            friend class const_iterator;
            using Node = typename Skiplist::Node;
            Node* node = nullptr;
        public:
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef ptrdiff_t difference_type;
            typedef std::pair<K, V> value_type;
            typedef std::pair<K, V>* pointer;
            typedef std::pair<K, V>& reference;

            iterator() = default;
            iterator(Node* n) :node(n){}
            iterator(const iterator&) = default;
            iterator(iterator&&) = default;
            ~iterator() = default;

            iterator& operator==(const iterator& other) { if (this != std::addressof(other)) iterator(other).swap(*this); return *this; }
            iterator& operator==(iterator&& other) { if (this != std::addressof(other)) iterator(std::move(other)).swap(*this); return *this; }

            void swap(iterator& other) { std::swap(node, other.node); }

            reference operator*() { return node->data; }
            pointer operator->() { return &(node->data); }

            bool operator==(const iterator &other) const { return node == other.node; }
            bool operator!=(const iterator &other) const { return node != other.node; }
            bool operator==(const const_iterator &other) const { return node == other.node; }
            bool operator!=(const const_iterator &other) const { return node != other.node; }

            iterator& operator++() { node = node->forward[0]; return *this; }
            iterator& operator++(int) { iterator r = *this; node = node->forward[0]; return r; }
            iterator& operator--() { node = node->backword; return *this; }
            iterator& operator--(int) { iterator r = *this; node = node->backword; return r; }

            iterator operator+(int j) const
            {
                iterator r = *this; if (j > 0) while (j--) ++r; else while (j++) --r; return r;
            }
            iterator operator-(int j) const { return operator+(-j); }
            iterator &operator+=(int j) { return *this = *this + j; }
            iterator &operator-=(int j) { return *this = *this - j; }

            const K& key() { return node->data.first; }
            V& value() { return node->data.second; }
        };


        class const_iterator : public std::iterator<std::bidirectional_iterator_tag, std::pair<K,V>>
        {
            friend class iterator;
            using Node = typename Skiplist::Node;
            Node* node = nullptr;
        public:
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef ptrdiff_t difference_type;
            typedef std::pair<K, V> value_type;
            typedef std::pair<K, V>* pointer;
            typedef std::pair<K, V>& reference;

            const_iterator() = default;
            const_iterator(Node* n) :node(n) {}
            const_iterator(const const_iterator&) = default;
            const_iterator(const_iterator&&) = default;
            ~const_iterator() = default;

            const_iterator& operator==(const const_iterator& other) { if (this != std::addressof(other)) const_iterator(other).swap(*this); return *this; }
            const_iterator& operator==(const_iterator&& other) { if (this != std::addressof(other)) const_iterator(std::move(other)).swap(*this); return *this; }

            void swap(const_iterator& other) { std::swap(node, other.node); }

            const reference operator*() const { return node->data; }
            const pointer operator->() const { return &(node->data); }

            bool operator==(const iterator &other) const { return node == other.node; }
            bool operator!=(const iterator &other) const { return node != other.node; }
            bool operator==(const const_iterator &other) const { return node == other.node; }
            bool operator!=(const const_iterator &other) const { return node != other.node; }

            const_iterator& operator++() { node = node->forward[0]; return *this; }
            const_iterator& operator++(int) { const_iterator r = *this; node = node->forward[0]; return r; }
            const_iterator& operator--() { node = node->backword; return *this; }
            const_iterator& operator--(int) { const_iterator r = *this; node = node->backword; return r; }

            const_iterator operator+(int j) const
            {
                const_iterator r = *this; if (j > 0) while (j--) ++r; else while (j++) --r; return r;
            }
            const_iterator operator-(int j) const { return operator+(-j); }
            const_iterator &operator+=(int j) { return *this = *this + j; }
            const_iterator &operator-=(int j) { return *this = *this - j; }

            const K& key() const { return node->data.first; }
            const V& value() const { return node->data.second; }
        };


        iterator begin() { return iterator(header_->forward[0]); }
        const_iterator begin() const { return const_iterator(header_->forward[0]); }
        const_iterator cbegin() const { return const_iterator(header_->forward[0]); }

        iterator end() { return iterator(header_); }
        const_iterator end() const { return const_iterator(header_); }
        const_iterator cend() const { return const_iterator(header_); }

        iterator find(const K& key) { return iterator(findNode(key)); }
        const_iterator find(const K& key) const { return const_iterator(findNode(key)); }

        iterator erase(iterator it);

    };


    template<class K, class V>
    using SkiplistNode_t = typename Skiplist<K, V>::Node;

    template<class K, class V>
    void Skiplist<K, V>::swap(Skiplist& other)
    {
        std::swap(header_, other.header_);
        std::swap(level_, other.level_);
        std::swap(size_, other.size_);
    }

    template<class K, class V>
    int Skiplist<K, V>::randomLevel()
    {
        int k = 0;
        while (rand() % 2 && k < (priv::K_MaxLevelNum - 1))
            k++;

        return k;
    }

    template<class K, class V>
    SkiplistNode_t<K, V>* Skiplist<K, V>::createNode()
    {
        Node* n = new Node();
        return n;
    }

    template<class K, class V>
    SkiplistNode_t<K, V>* Skiplist<K, V>::createNode(const std::pair<K, V>& item)
    {
        Node* n = new Node(item);
        return n;
    }

    template<class K, class V>
    SkiplistNode_t<K, V>* Skiplist<K, V>::createNode(std::pair<K, V>&& item)
    {
        Node* n = new Node(std::move(item));
        return n;
    }

    template<class K, class V>
    SkiplistNode_t<K, V>* Skiplist<K, V>::findNode(const K& key) const
    {
        Node* cur = header_;
        Node* next = header_;
        int k = level_;

        do
        {
            while ((next = cur->forward[k]) != header_ && (next->data.first < key))
            {
                cur = next;
            }
        } while (--k >= 0);

        if (next != header_ && next->data.first == key)
        {
            return next;
        }
        else
            return header_;
    }


    template<class K, class V>
    const SkiplistNode_t<K, V>* Skiplist<K, V>::nodeAt(int index) const
    {
        const Node* c = header_;
        while (index >= 0)
        {
            if (c)
            {
                c = c->forward[0];
                index -= 1;
            }
            else
            {
                break;
            }
        }

        return c;
    }

    template<class K, class V>
    SkiplistNode_t<K, V>* Skiplist<K, V>::nodeAt(int index)
    {
        Node* c = header_;
        while (index >= 0)
        {
            if (c)
            {
                c = c->forward[0];
                index -= 1;
            }
            else
            {
                break;
            }
        }

        return c;
    }

    template<class K, class V>
    SkiplistNode_t<K, V>* Skiplist<K, V>::createHeader()
    {
        Node* h = createNode();
        h->backword = h;
        h->forward[0] = h;
        return h;
    }

    template<class K, class V>
    void Skiplist<K, V>::copyConstruct(Node* header)
    {
        if (header)
        {
            Node* cur = header;
            while (cur = cur->forward[0])
            {
                insert(cur->data.first, cur->data.second);
            }
        }
    }

    template<class K, class V>
    void Skiplist<K, V>::destroy()
    {
        if (header_)
        {
            Node* c = header_;
            Node* n = c->forward[0];
            while (n != header_)
            {
                c = n;
                n = n->forward[0];
                delete c;
            }

            delete header_;
            header_ = nullptr;
        }

        level_ = 0;
        size_ = 0;
    }

    template<class K, class V>
    Skiplist<K, V>::Skiplist()
        : header_(createHeader())
    {
    }

    template<class K, class V>
    Skiplist<K, V>::Skiplist(const Skiplist& other)
        : header_(createHeader())
    {
        copyConstruct(other.header_);
    }

    template<class K, class V>
    Skiplist<K, V>::Skiplist(Skiplist&& other)
        : header_(std::move(other.header_))
        , level_(other.level_)
        , size_(other.size_)
    {
        other.header_ = createHeader();
        other.level_ = 0;
        other.size_ = 0;
    }

    template<class K, class V>
    Skiplist<K, V>& Skiplist<K, V>::operator=(const Skiplist& other)
    {
        if (this != &other)
        {
            Skiplist(other).swap(*this);
        }
        return *this;
    }

    template<class K, class V>
    Skiplist<K, V>& Skiplist<K, V>::operator=(Skiplist&& other)
    {
        if (this != &other)
        {
            Skiplist(std::move(other)).swap(*this);
        }
        return *this;
    }

    template<class K, class V>
    Skiplist<K, V>::~Skiplist()
    {
        destroy();
    }

    template<class K, class V>
    int Skiplist<K, V>::size() const
    {
        return size_;
    }

    template<class K, class V>
    bool Skiplist<K, V>::contains(const K& key) const
    {
        return findNode(key) != header_;
    }

    template<class K, class V>
    const V Skiplist<K, V>::value(const K& key) const
    {
        const Node* n = findNode(key);
        if (n && n != header_)
        {
            return n->data.second;
        }
        else
            return V();
    }

    template<class K, class V>
    const K Skiplist<K, V>::key(const V& value) const
    {
        const Node* c = header_;
        while (c = c->forward[0])
        {
            if (c->data.second == value)
            {
                return c->data.first;
            }
        }

        return K();
    }

    template<class K, class V>
    const K& Skiplist<K, V>::keyAt(int index) const
    {
        const Node* n = nodeAt(index);
        DAssert(n);
        return n->data.first;
    }

    template<class K, class V>
    const V& Skiplist<K, V>::valueAt(int index) const
    {
        const Node* n = nodeAt(index);
        DAssert(n);
        return n->data.second;
    }

    template<class K, class V>
    V& Skiplist<K, V>::valueAt(int index)
    {
        Node* n = nodeAt(index);
        DAssert(n);
        return n->data.second;
    }

    template<class K, class V>
    const V& Skiplist<K, V>::operator[](const K& key) const
    {
        const Node* n = findNode(key);
        DAssert(n != header_);
        return n->data.second;
    }

    template<class K, class V>
    V& Skiplist<K, V>::operator[](const K& key)
    {
        Node* n = findNode(key);
        DAssert(n != header_);
        return n->data.second;
    }


    template<class K, class V>
    template<class ValueType>
    void Skiplist<K, V>::insert(ValueType&& pair)
    {
        Node* update[priv::K_MaxLevelNum] = { nullptr };
        Node* cur = header_;
        Node* next = header_;
        int k = level_;

        do
        {
            while ((next = cur->forward[k]) != header_ && (next->data.first < pair.first))
            {
                cur = next;
            }
            update[k] = cur;

        } while (--k >= 0);

        if (next != header_ )
        {
            if (next->data.first == pair.first)
            {
                next->data.second = pair.second;
                return;
            }
            if (next->data.first < pair.first)
            {
                next = header_;
            }
        }

        k = randomLevel();

        if (k > level_)
        {
            k = ++level_;
            update[k] = header_;
            header_->forward[k] = header_;
        }

        Node* n = createNode(std::forward<ValueType>(pair));
        if (next != header_)
        {
            n->backword = next->backword;
            next->backword = n;
        }
        else
        {
            n->backword = cur;
            header_->backword = n;
        }

        size_ += 1;

        do
        {
            n->forward[k] = update[k]->forward[k];
            update[k]->forward[k] = n;

        } while (--k >= 0);
    }

    template<class K, class V>
    void Skiplist<K, V>::insert(const K& key, const V& value)
    {
        return insert(std::make_pair(key, value));
    }

    template<class K, class V>
    void Skiplist<K, V>::insert(K&& key, V&& value)
    {
        return insert(std::make_pair(std::move(key), std::move(value)));
    }


    template<class K, class V>
    void Skiplist<K, V>::remove(const K& key)
    {
        Node* update[priv::K_MaxLevelNum] = { nullptr };
        Node* cur = header_;
        Node* next = header_;
        int k = level_;

        do
        {
            while ((next = cur->forward[k])!=header_ && (next->data.first < key))
            {
                cur = next;
            }
            update[k] = cur;

        } while (--k >= 0);

        if (next != header_ && next->data.first == key)
        {
            for (int i = level_; i >= 0; --i)
            {
                if (update[i]->forward[i] == next)
                {
                    update[i]->forward[i] = next->forward[i];
                }
            }

            if (next->forward[0])
            {
                next->forward[0]->backword = next->backword;
            }

            if (next == header_->backword)
            {
                header_->backword = next->backword;
            }

            delete next;

            size_ -= 1;

            for (int i = level_; i >= 0; --i)
            {
                if (header_->forward[i] == nullptr)
                {
                    level_ -= 1;
                }
                else
                    break;
            }
        }
    }

    template<class K, class V>
    typename Skiplist<K, V>::iterator Skiplist<K, V>::erase(iterator it)
    {
        if (it == end())
        {
            return it;
        }

        Node* update[priv::K_MaxLevelNum] = { nullptr };
        Node* cur = header_;
        Node* next = header_;
        int k = level_;

        do
        {
            while ((next = cur->forward[k]) && (next->data.first < it.key()))
            {
                cur = next;
            }
            update[k] = cur;

        } while (--k >= 0);

        if (next != header_ && next->data.first == it.key())
        {
            for (int i = level_; i >= 0; --i)
            {
                if (update[i]->forward[i] == next)
                {
                    update[i]->forward[i] = next->forward[i];
                }
            }

            if (next->forward[0])
            {
                next->forward[0]->backword = next->backword;
            }

            if (next == header_->backword)
            {
                header_->backword = next->backword;
            }

            it+=1;

            delete next;

            size_ -= 1;

            for (int i = level_; i >= 0; --i)
            {
                if (header_->forward[i] == nullptr)
                {
                    level_ -= 1;
                }
                else
                    break;
            }
        }

        return it;
    }

    template<class K, class V>
    std::ostream& operator<<(std::ostream& os, const Skiplist<K, V>& src)
    {
        for (auto c: src)
        {
            os << "{" << c.first << ":" << c.second << "}" << "\t";
        }

        return os;
    }

}
