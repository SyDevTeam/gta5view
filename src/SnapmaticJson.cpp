/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2023 Syping
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include <boost/json/src.hpp>
#include <sstream>
#include "SnapmaticJson.h"

void serializer(std::ostream &os, const boost::json::value &jv, std::string *indent, bool do_indent)
{
    std::string indent_;
    if (!indent)
        indent = &indent_;
    switch (jv.kind()) {
    case boost::json::kind::object: {
        if (do_indent) {
            os << "{\n";
            indent->append(4, ' ');
        }
        else
            os << "{";
        auto const &obj = jv.get_object();
        if (!obj.empty()) {
            auto it = obj.begin();
            for (;;) {
                if (do_indent)
                    os << *indent << boost::json::serialize(it->key()) << ": ";
                else
                    os << boost::json::serialize(it->key()) << ":";
                serializer(os, it->value(), indent, do_indent);
                if (++it == obj.end())
                    break;
                if (do_indent)
                    os << ",\n";
                else
                    os << ",";
            }
        }
        if (do_indent) {
            os << "\n";
            indent->resize(indent->size() - 4);
            os << *indent << "}";
        }
        else
            os << "}";
        break;
    }
    case boost::json::kind::array: {
        if (do_indent) {
            os << "[\n";
            indent->append(4, ' ');
        }
        else
            os << "[";
        auto const &arr = jv.get_array();
        if (!arr.empty()) {
            auto it = arr.begin();
            for (;;) {
                if (do_indent)
                    os << *indent;
                serializer(os, *it, indent, do_indent);
                if (++it == arr.end())
                    break;
                if (do_indent)
                    os << ",\n";
                else
                    os << ",";
            }
        }
        if (do_indent) {
            os << "\n";
            indent->resize(indent->size() - 4);
            os << *indent << "]";
        }
        else
            os << "]";
        break;
    }
    case boost::json::kind::string: {
        os << boost::json::serialize(jv.get_string());
        break;
    }
    case boost::json::kind::uint64:
        os << jv.get_uint64();
        break;
    case boost::json::kind::int64:
        os << jv.get_int64();
        break;
    case boost::json::kind::double_:
        os << jv.get_double();
        break;
    case boost::json::kind::bool_:
        if (jv.get_bool())
            os << "true";
        else
            os << "false";
        break;
    case boost::json::kind::null:
        os << "null";
        break;
    }
}

std::string SnapmaticJson::serialize(const boost::json::value &jv, bool do_indent)
{
    std::ostringstream buffer;
    serializer(buffer, jv, nullptr, do_indent);
    return buffer.str();
}
