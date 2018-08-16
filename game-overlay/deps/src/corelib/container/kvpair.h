#pragma once

namespace Storm {

    template <class K, class V >
    class KVPair
    {
    public:
        K first;
        V second;

        KVPair() = default;
        KVPair(const K& k, const V& v) : first(k), second(v) { ; }
        KVPair(K&& k, V&& v) : first(std::move(k)), second(std::move(v)) { ; }

        KVPair(const KVPair& kv) : first(kv.first), second(kv.second) { ; }
        KVPair(KVPair&& kv) : first(std::move(kv.first)), second(std::move(kv.second)) { ; }
        
        KVPair& operator=(const KVPair& kv) 
        {
            if (this != &kv)
            {
                first = kv.first;
                second = kv.second;
            }
            return *this;
        }

        KVPair& operator=(KVPair&& kv)
        {
            if (this != &kv)
            {
                first = std::move(kv.first);
                second = std::move(kv.second);
            }
            return *this;
        }
        const K& key() const { return this->first; }
        const V& value() const { return this->second; }

        V& value() { return this->second; }

        bool operator == (const KVPair& kv) const{ return this->first == kv.first; }
        bool operator != (const KVPair& kv) const{ return this->first != kv.first; }
        bool operator  < (const KVPair& kv) const{ return this->first < kv.first; }
        bool operator <= (const KVPair& kv) const{ return this->first <= kv.first; }
        bool operator  > (const KVPair& kv) const{ return this->first > kv.first; }
        bool operator >= (const KVPair& kv) const{ return this->first >= kv.first; }
    };

    template<class K, class V>
    bool operator==(const KVPair<K, V>& kv, const K& key){
        return kv.key() == key;
    }

    template<class K, class V>
    bool operator!=(const KVPair<K, V>& kv, const K& key){
        return kv.key() != key;
    }

    template<class K, class V>
    bool operator<(const KVPair<K, V>& kv, const K& key){
        return kv.key() < key;
    }

    template<class K, class V>
    bool operator<=(const KVPair<K, V>& kv, const K& key){
        return kv.key() <= key;
    }

    template<class K, class V>
    bool operator>(const KVPair<K, V>& kv, const K& key){
        return kv.key() > key;
    }

    template<class K, class V>
    bool operator>=(const KVPair<K, V>& kv, const K& key){
        return kv.key() >= key;
    }

    template<class K, class V>
    bool operator==(const K& key, const KVPair<K, V>& kv) {
        return key == kv.key();
    }

    template<class K, class V>
    bool operator!=(const K& key, const KVPair<K, V>& kv) {
        return key != kv.key();
    }

    template<class K, class V>
    bool operator<(const K& key, const KVPair<K, V>& kv) {
        return key < kv.key();
    }

    template<class K, class V>
    bool operator<=(const K& key, const KVPair<K, V>& kv) {
        return key <= kv.key();
    }

    template<class K, class V>
    bool operator>(const K& key, const KVPair<K, V>& kv) {
        return key > kv.key();
    }

    template<class K, class V>
    bool operator>=(const K& key, const KVPair<K, V>& kv) {
        return key >= kv.key();
    }
}
