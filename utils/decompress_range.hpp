#pragma once

#include <iostream>
#include <ranges>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace bio = boost::iostreams;

class decompress_iterator : public boost::iterator_facade<decompress_iterator, char, std::input_iterator_tag>
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = char;
    using difference_type = std::ptrdiff_t;
    using pointer = const char*;
    using reference = const char&;

    decompress_iterator() : file(nullptr), end(true) {}

    explicit decompress_iterator(bio::filtering_istream& file, bool end = false) : file(&file), end(end) {
        if (!end) operator++();
    }

    friend class boost::iterator_core_access;

    void increment() {
        if (!file || end) return;
        end = peek_char();
    }

    bool equal(const decompress_iterator& other) const { return end == other.end; }

    reference operator*() const { return value; }

    decompress_iterator& operator++() {
        if (file && !end) {
            end = file->get(value) ? false : true;
        }
        return *this;
    }

    decompress_iterator operator++(int) {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    friend bool operator==(const decompress_iterator& lhs, const decompress_iterator& rhs) { return lhs.end == rhs.end; }
    friend bool operator!=(const decompress_iterator& lhs, const decompress_iterator& rhs) { return !(lhs == rhs); }

private:
    bio::filtering_istream* file;
    value_type value;
    bool end;

    bool peek_char() const {
        std::cout << file << std::endl;
        if (file) {
            int c = file->peek();
            std::cout << c << std::endl;
            return c != EOF;
        }
        return false;
    }
};


class decompress_range
{
public:
    bio::file_source file;
    bio::filtering_istream filter;

    explicit decompress_range(std::string const& filename)
    : file(filename, std::ios_base::in | std::ios_base::binary) {
        filter.push(bio::zstd_decompressor());
        filter.push(file);
    }

    decompress_iterator begin() { return decompress_iterator(filter); }
    decompress_iterator end() { return decompress_iterator(); }

    operator std::ranges::subrange<decompress_iterator>() { return {begin(), end()}; }
};


class decompress_range_view : public std::ranges::view_base {
public:
    std::unique_ptr<decompress_range> range;

    explicit decompress_range_view(std::string const& filename) : range(std::make_unique<decompress_range>(filename)) {}

    auto begin() { return range->begin(); }
    auto end() { return range->end(); }

    template<typename View>
    auto operator|(View&& view) { return *range | std::forward<View>(view); }
};


class decompress_range_owning_view : public std::ranges::owning_view<decompress_range_view> {
public:
    explicit decompress_range_owning_view(std::string const& filename) 
        : std::ranges::owning_view<decompress_range_view>(decompress_range_view(filename)) {}
};

