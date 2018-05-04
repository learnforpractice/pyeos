///
/// http://stackoverflow.com/questions/20991213/memory-mapped-files-managed-mapped-file-and-offset-pointer
///

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/container/scoped_allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace bip = boost::interprocess;
using mutex_type    = bip::named_mutex;

struct X
{
    char buf[100];
    double rate;
    uint32_t samples[1024];
};

template <typename T> using shared_alloc  = bip::allocator<T,bip::managed_mapped_file::segment_manager>;
template <typename T> using shared_vector = boost::container::vector<T, shared_alloc<T> >;
template <typename K, typename V, typename P = std::pair<K,V>, typename Cmp = std::less<K> >
                      using shared_map    = boost::container::flat_map<K, V, Cmp, shared_alloc<P> >;

using shared_string = bip::basic_string<char,std::char_traits<char>,shared_alloc<char> >;
using dataset_t     = shared_map<shared_string, shared_vector<X> >;

struct mutex_remove
{
    mutex_remove() { mutex_type::remove("ABCD"); }
    ~mutex_remove(){ mutex_type::remove("ABCD"); }
} remover;

static mutex_type mutex(bip::open_or_create,"ABCD");

static dataset_t& shared_instance()
{
    bip::scoped_lock<mutex_type> lock(mutex);
    static bip::managed_mapped_file seg(bip::open_or_create,"./demo.db", 1024*1024*10); // "50Gb ought to be enough for anyone"

    static dataset_t* _instance = seg.find_or_construct<dataset_t>
        ("DATA")
        (
         std::less<shared_string>(),
         dataset_t::allocator_type(seg.get_segment_manager())
        );

    static auto capacity = seg.get_free_memory();
    std::cerr << "Free space: " << capacity << "g\n";

    return *_instance;
}

int main()
{
    auto& db = shared_instance();
    {
       bip::scoped_lock<mutex_type> lock(mutex);
       auto alloc = db.get_allocator().get_segment_manager();

       std::cout << db.size() << '\n';

       for (int i = 0; i < 10; ++i)
       {
           std::string key_ = "item" + std::to_string(i);
           shared_string key(alloc);
           key.assign(key_.begin(), key_.end());
           auto value = shared_vector<X>(alloc);
           value.resize(1);
           auto entry = std::make_pair(key, value);

           db.insert(std::make_pair(key, value));
       }
    }

    shared_instance();

}
