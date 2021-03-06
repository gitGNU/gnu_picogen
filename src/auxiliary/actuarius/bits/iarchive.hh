//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2009  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef IARCHIVE_HH_INCLUDED_20090918
#define IARCHIVE_HH_INCLUDED_20090918

#include <stack>

//-----------------------------------------------------------------------------
// IArchive
//-----------------------------------------------------------------------------
namespace actuarius {
class IArchive {
        typedef std::string::iterator iterator_t;

        template <typename T> static void read_val (std::string const & from, T &to) {
                std::stringstream ss;
                ss << from;
                ss >> to;
        }
        static void read_val (std::string const & from, std::string &to) {
                to = from;
        }
public:

        enum { serialize = 0, deserialize = 1 };

        IArchive (std::istream &in)
        : content("")
        {
                path.push ("");

                std::string curr;
                while (getline (in, curr))
                        content += curr + '\n';

                doc = detail::parse<iterator_t> (
                 detail::block_match_t<iterator_t> (
                  detail::match_t<iterator_t>(content.begin(),content.begin()),
                  detail::match_t<iterator_t>(content.begin(),content.end())
                ));
                doc.set_as_top_level_node (true);
                optional.push(false);
        }

        IArchive (IArchive const & oa)
        : content(oa.content)
        , doc(oa.doc)
        , path(oa.path)
        {
                optional.push(false);
        }

        template <typename T>
         typename detail::disable_if<
                 detail::has_serialize_function<IArchive,T>,
                 IArchive
         >::type&
        operator & (nrp<T> val) {
                using namespace detail;
                path.push (path.top() + val.name + "/");

                if (detail::value_match_t<iterator_t>
                      value = doc.take_value (val.name)
                ) {
                        read_val (unescape_nonstring_terminal (value.value()),
                                val.value);
                } else if (!optional.top()) {
                        std::cerr << "warning: found nothing for "
                                  << path.top()
                                  << " (nrp)"
                                  << std::endl;
                }

                path.pop ();
                return *this;
        }

        template <typename T>
         typename detail::disable_if<
                 detail::has_serialize_function<IArchive,T>,
                 IArchive
         >::type&
        operator & (ref<T> val) {
                using namespace detail;
                path.push (path.top() + "?" + "/");

                if (detail::value_match_t<iterator_t>
                      value = doc.take_first_value()
                ) {
                        read_val(unescape_nonstring_terminal (value.value()),
                                val.value);
                } else if (!optional.top()) {
                        std::cerr << "warning: found nothing for "
                                  << path.top()
                                  << " (ref)"
                                  << std::endl;
                }

                path.pop ();
                return *this;
        }

        template <typename T> IArchive
        &operator & (nerp<T> val) {
                using namespace detail;
                path.push (path.top() + val.name + "/");
                if (value_match_t<iterator_t> value=doc.take_value (val.name)){

                        // Extract value (i.e. key for lookup).
                        const std::string key =
                                unescape_nonstring_terminal (value.value());

                        // Check if it exists.
                        if (val.enumDesc.exists (key.c_str())) {
                                val.value = val.enumDesc[key.c_str()];
                        } else if (!optional.top()) {
                                std::cerr << "warning: found nothing for "
                                          << path.top()
                                          << " for value '"
                                          << key
                                          << "' (nerp)"
                                          << std::endl;
                        }
                } else if (!optional.top()) {
                        std::cerr << "warning: found nothing for "
                                  << path.top()
                                  << " (nerp)"
                                  << std::endl;
                }

                path.pop ();
                return *this;
        }

        template <typename T> IArchive
        &operator & (ncrp<T> val) {
                path.push (path.top() + val.name + "/");

                while (detail::block_t<iterator_t>
                        child = doc.take_child (path.top())
                ) {
                        val.value.push_back (typename T::value_type());
                        IArchive ia (*this, child);
                        ia & make_nrp (val.name, val.value.back());
                }

                path.pop ();
                return *this;
        }

        template <typename T> IArchive
        &operator & (containerref<T> val) {
                path.push (path.top() + "?" + "/");

                while (detail::block_t<iterator_t>
                        child = doc.take_first_child ()
                ) {
                        val.value.push_back (typename T::value_type());
                        IArchive ia (*this, child, false);
                        ia & make_ref (val.value.back());
                }

                path.pop ();
                return *this;
        }

        template <typename T> IArchive
        &operator & (necrp<T> val) {
                path.push (path.top() + val.name + "/");

                while (detail::value_match_t<iterator_t>
                        value = doc.take_value (val.name)
                ) {
                        using namespace detail;
                        // Extract value (i.e. key for lookup).
                        const std::string key =
                                unescape_nonstring_terminal (value.value());

                        // Check if it exists.
                        if (val.enumDesc.exists (key.c_str())) {
                                val.value.push_back(val.enumDesc[key.c_str()]);
                        } else if (!optional.top()) {
                                std::cerr << "warning: found nothing for "
                                          << path.top()
                                          << " for value '"
                                          << key
                                          << "' (necrp)"
                                          << std::endl;
                        }
                }

                path.pop ();
                return *this;
        }

        template <typename T>
        typename detail::enable_if<
                detail::has_serialize_function<IArchive,T>,
                IArchive
        >::type
        &operator & (nrp<T> val) {
                path.push (path.top() + val.name + "/");

                if (detail::block_t<iterator_t>
                        child = doc.take_child (path.top())
                ) {
                        IArchive ia (*this, child, false);
                        val.value.serialize (ia);
                } else if (!optional.top()) {
                        std::cerr << "warning: found nothing for "
                                  << path.top()
                                  << " (nrp rec)"
                                  << std::endl;
                }

                path.pop ();
                return *this;
        }

        template <typename T>
        IArchive &operator & (inlineref<T> val) {
                val.value.serialize(*this);
                return *this;
        }

        template <typename T>
         typename detail::enable_if<
                 detail::has_serialize_function<IArchive,T>,
                 IArchive
         >::type&
        operator & (ref<T> val) {
                path.push (path.top() + "?" + "/");
                IArchive ia (*this, doc, false);
                val.value.serialize(ia);
                path.pop ();
                return *this;
        }

        IArchive&
        operator & (nrp<std::string> val) {
                using namespace detail;
                path.push (path.top() + val.name + "/");

                if (detail::value_match_t<iterator_t>
                      value = doc.take_value (val.name)
                ) {
                        val.value =
                                unescape_nonstring_terminal (value.value());
                } else if (!optional.top()) {
                        std::cerr << "warning: found nothing for "
                                  << path.top()
                                  << " (nrp)"
                                  << std::endl;
                }

                path.pop ();
                return *this;
        }

        template <typename CONT, typename ADVICE_TYPE>
        IArchive&
        operator & (npcrp<CONT,ADVICE_TYPE> val) {
                using namespace detail;
                path.push (path.top() + val.name + "/");

                if (detail::block_t<iterator_t>
                        block=doc.take_child(val.name)
                ) {
                        while (detail::block_t<iterator_t> child = block.take_first_child()) {
                                typename CONT::value_type value;

                                read_val(child.id().c_str(), value.*val.ptr);

                                IArchive ia (*this, child, false);
                                value.serialize (ia);
                                val.value.push_back (value);
                        }
                } else if (!optional.top()) {
                        std::cerr << "warning: found nothing for "
                                  << path.top()
                                  << " (npecrp)"
                                  << std::endl;
                }

                path.pop ();
                return *this;
        }

        template <typename CONT, typename PTR>
        IArchive&
        operator & (pcrp<CONT,PTR> val) {
                using namespace detail;
                path.push (path.top() + "?" + "/");

                while (detail::block_t<iterator_t> child = doc.take_first_child()) {
                        typename CONT::value_type value;

                        read_val(child.id().c_str(), value.*val.ptr);

                        IArchive ia (*this, child, false);
                        value.serialize (ia);
                        val.value.push_back (value);
                }

                path.pop ();
                return *this;
        }

        template <typename CONT, typename ADVICE_TYPE>
        IArchive&
        operator & (npecrp<CONT,ADVICE_TYPE> val) {
                using namespace detail;
                path.push (path.top() + val.name + "/");

                if (detail::block_t<iterator_t>
                        block=doc.take_child(val.name)
                ) {
                        while (detail::block_t<iterator_t> child = block.take_first_child()) {
                                // map id -> enum-value
                                if (!val.enumDesc.exists (child.id().c_str())){
                                        if (!optional.top()) {
                                                std::cerr
                                                  << "warning: found nothing for "
                                                  << path.top()
                                                  << " for value '"
                                                  << child.id()
                                                  << "' (necrp)"
                                                  << std::endl;
                                        }
                                        continue;
                                }
                                typename CONT::value_type value;
                                value.*val.ptr =
                                        val.enumDesc[child.id().c_str()];
                                IArchive ia (*this, child, false);
                                value.serialize (ia);
                                val.value.push_back (value);
                        }
                } else if (!optional.top()) {
                        std::cerr << "warning: found nothing for "
                                  << path.top()
                                  << " (npecrp)"
                                  << std::endl;
                }

                path.pop ();
                return *this;
        }

        template <typename CONT, typename ADVICE_TYPE>
        IArchive&
        operator & (pecrp<CONT,ADVICE_TYPE> val) {
                using namespace detail;
                path.push (path.top() + "?" + "/");

                while (detail::block_t<iterator_t> child = doc.take_first_child()) {
                        // map id -> enum-value
                        if (!val.enumDesc.exists (child.id().c_str())){
                                if (!optional.top()) {
                                        std::cerr
                                          << "warning: found nothing for "
                                          << path.top()
                                          << " for value '"
                                          << child.id()
                                          << "' (necrp)"
                                          << std::endl;
                                }
                                continue;
                        }
                        typename CONT::value_type value;
                        value.*val.ptr =
                                val.enumDesc[child.id().c_str()];
                        IArchive ia (*this, child, false);
                        value.serialize (ia);
                        val.value.push_back (value);
                }

                path.pop ();
                return *this;
        }


        // "iomanip" style things
        IArchive&
        operator& (push_optional rhs) {
                optional.push(rhs.optional);
                return *this;
        }
        IArchive&
        operator& (pop_optional_) {
                optional.pop();
                return *this;
        }


private:
        std::string content;
        detail::block_t<iterator_t> doc;
        std::stack<std::string> path;

        std::stack<bool> optional;

        IArchive (
                IArchive const & oa,
                detail::block_t<iterator_t> const &child,
                bool asChild=true)
        : content(oa.content)
        , doc()
        , path()
        {
                // In case this is a top-level-node we have an
                // anonymous top-level-node.  But if we take a
                // child,  then  that  child  does not have an
                // anonymous top level node,  hence  we create
                // a new node here, and insert that child into
                // that new node as its child.
                if (asChild)
                        doc.add_child (child);
                else
                        doc = child;
                path.push ("");

                optional.push(false);
        }
};
}



#endif // IARCHIVE_HH_INCLUDED_20090918
