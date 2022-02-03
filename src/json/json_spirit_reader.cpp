//          Copyright John W. Wilkinson 2007 - 2009.
// Distributed under the MIT License, see accompanying file LICENSE.txt

// json spirit version 4.03

#include "json_spirit_reader.h"
#include "json_spirit_reader_template.h"

using namespace json_spirit;

bool json_spirit::read( const std::string& s, json_spirit::Value& value )
{
    return read_string( s, value );
}

void json_spirit::read_or_throw( const std::string& s, json_spirit::Value& value )
{
    read_string_or_throw( s, value );
}

bool json_spirit::read( std::istream& is, json_spirit::Value& value )
{
    return read_stream( is, value );
}

void json_spirit::read_or_throw( std::istream& is, json_spirit::Value& value )
{
    read_stream_or_throw( is, value );
}

bool json_spirit::read( std::string::const_iterator& begin, std::string::const_iterator end, json_spirit::Value& value )
{
    return read_range( begin, end, value );
}

void json_spirit::read_or_throw( std::string::const_iterator& begin, std::string::const_iterator end, json_spirit::Value& value )
{
    begin = read_range_or_throw( begin, end, value );
}

#ifndef BOOST_NO_STD_WSTRING

bool json_spirit::read( const std::wstring& s, json_spirit::wValue& value )
{
    return read_string( s, value );
}

void json_spirit::read_or_throw( const std::wstring& s, json_spirit::wValue& value )
{
    read_string_or_throw( s, value );
}

bool json_spirit::read( std::wistream& is, json_spirit::wValue& value )
{
    return read_stream( is, value );
}

void json_spirit::read_or_throw( std::wistream& is, json_spirit::wValue& value )
{
    read_stream_or_throw( is, value );
}

bool json_spirit::read( std::wstring::const_iterator& begin, std::wstring::const_iterator end, json_spirit::wValue& value )
{
    return read_range( begin, end, value );
}

void json_spirit::read_or_throw( std::wstring::const_iterator& begin, std::wstring::const_iterator end, json_spirit::wValue& value )
{
    begin = read_range_or_throw( begin, end, value );
}

#endif

bool json_spirit::read( const std::string& s, json_spirit::mValue& value )
{
    return read_string( s, value );
}

void json_spirit::read_or_throw( const std::string& s, json_spirit::mValue& value )
{
    read_string_or_throw( s, value );
}

bool json_spirit::read( std::istream& is, json_spirit::mValue& value )
{
    return read_stream( is, value );
}

void json_spirit::read_or_throw( std::istream& is, json_spirit::mValue& value )
{
    read_stream_or_throw( is, value );
}

bool json_spirit::read( std::string::const_iterator& begin, std::string::const_iterator end, json_spirit::mValue& value )
{
    return read_range( begin, end, value );
}

void json_spirit::read_or_throw( std::string::const_iterator& begin, std::string::const_iterator end, json_spirit::mValue& value )
{
    begin = read_range_or_throw( begin, end, value );
}

#ifndef BOOST_NO_STD_WSTRING

bool json_spirit::read( const std::wstring& s, json_spirit::wmValue& value )
{
    return read_string( s, value );
}

void json_spirit::read_or_throw( const std::wstring& s, json_spirit::wmValue& value )
{
    read_string_or_throw( s, value );
}

bool json_spirit::read( std::wistream& is, json_spirit::wmValue& value )
{
    return read_stream( is, value );
}

void json_spirit::read_or_throw( std::wistream& is, json_spirit::wmValue& value )
{
    read_stream_or_throw( is, value );
}

bool json_spirit::read( std::wstring::const_iterator& begin, std::wstring::const_iterator end, json_spirit::wmValue& value )
{
    return read_range( begin, end, value );
}

void json_spirit::read_or_throw( std::wstring::const_iterator& begin, std::wstring::const_iterator end, json_spirit::wmValue& value )
{
    begin = read_range_or_throw( begin, end, value );
}

#endif
