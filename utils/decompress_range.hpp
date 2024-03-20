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



// class decompress_range_view : public std::ranges::view_base {
// // class decompress_range_view : public std::ranges::view_interface<decompress_range_view> {
// public:
//     std::shared_ptr<decompress_range> range;

//     explicit decompress_range_view(std::shared_ptr<decompress_range> range_ptr) : range(std::move(range_ptr)) {}
//     explicit decompress_range_view(std::string const& filename) : range(std::make_shared<decompress_range>(filename)) {}
//     decompress_range_view(decompress_range_view const& other) : range(std::make_shared<decompress_range>(*other.range)) {}

//     auto begin() { return range->begin(); }
//     auto end() { return range->end(); }

//     template<typename View>
//     auto operator|(View&& view) { return *range | std::forward<View>(view); }
// };

class decompress_range_owning_view : public std::ranges::owning_view<decompress_range_view> {
public:
    explicit decompress_range_owning_view(std::string const& filename) 
        : std::ranges::owning_view<decompress_range_view>(decompress_range_view(filename)) {}
};


// class decompress_range_owning_view : public std::ranges::owning_view<decompress_range_view> {
// private:
//     std::shared_ptr<decompress_range> range_;

// public:
//     explicit decompress_range_owning_view(std::string const& filename) 
//         : range_(std::make_shared<decompress_range>(filename)), 
//           std::ranges::owning_view<decompress_range_view>(decompress_range_view(range_)) {}
// };

// This is not necessary since decompress_range_view is view-compatible
// struct make_decompress_range_view : std::ranges::range_adaptor_closure<make_decompress_range_view>
// {
//     auto operator()(std::string const& filename) const { return decompress_range_view{filename}; }
// };
// constexpr make_decompress_range_view decompress_range_to_view;







// class decompress_iterator : public boost::iterator_facade<decompress_iterator, char, std::input_iterator_tag>
// {
// public:
//     using iterator_category = std::input_iterator_tag;
//     using value_type = char;
//     using difference_type = std::ptrdiff_t;
//     using pointer = const char*;
//     using reference = const char&;

//     decompress_iterator() : file(nullptr), end(true) {}

//     explicit decompress_iterator(std::unique_ptr<bio::filtering_istream>&& file, bool end = false)
//         : file(std::move(file)), end(end) {
//         if (!end) {
//             operator++();
//         }
//     }

//     friend class boost::iterator_core_access;

//     void increment() const {
//         if (file && !end) {
//             if (peek_char()) {
//                 end = false;
//             } else {
//                 end = true;
//             }
//         }
//     }

//     bool equal(const decompress_iterator& other) const { return end == other.end; }

//     reference operator*() const { return value; }

//     decompress_iterator& operator++() {
//         if (file && !end) {
//             if (file->get(value)) {
//                 end = false;
//             } else {
//                 end = true;
//             }
//         }
//         return *this;
//     }

//     decompress_iterator* operator++(int) {
//         increment();
//         return this;
//     }

//     friend bool operator==(const decompress_iterator& lhs, const decompress_iterator& rhs) {
//         return lhs.end == rhs.end;
//     }

//     friend bool operator!=(const decompress_iterator& lhs, const decompress_iterator& rhs) {
//         return !(lhs == rhs);
//     }

// private:
//     // const bio::filtering_istream* file;
//     std::unique_ptr<bio::filtering_istream> file;
//     mutable value_type value;
//     mutable bool end;
//     bool peek_char() const {
//         if (file) {
//             int c = file->peek();
//             return c != EOF;
//         }
//         return false;
//     }
// };





// class decompress_iterator : public std::iterator<std::input_iterator_tag, char> {
// public:
//     decompress_iterator() = default;

//     decompress_iterator(boost::iostreams::filtering_istream& stream)
//         : filter(&stream), current(), end() {
//         ++(*this);  // Initialize the iterator to the first valid value
//     }

//     decompress_iterator& operator++() {
//         if (filter && !(*filter)) {
//             filter = nullptr;  // Invalidate the iterator when the stream is done
//         } else {
//             current = char();
//             if ((*filter) >> current) {
//                 end = filter->peek();
//             } else {
//                 filter = nullptr;  // Invalidate the iterator on failure
//             }
//         }
//         return *this;
//     }

//     decompress_iterator operator++(int) {
//         decompress_iterator tmp(*this);
//         ++(*this);
//         return tmp;
//     }

//     const char& operator*() const {
//         return current;
//     }

//     friend bool operator==(const decompress_iterator& lhs, const decompress_iterator& rhs) {
//         return lhs.filter == rhs.filter;
//     }

//     friend bool operator!=(const decompress_iterator& lhs, const decompress_iterator& rhs) {
//         return !(lhs == rhs);
//     }

// private:
//     boost::iostreams::filtering_istream* filter;
//     char current;
//     char end;  // To detect the end of the stream
// };


// // class decompress_range
// // {
// // public:
// //     std::unique_ptr<bio::filtering_istream> const filter;

// //     explicit decompress_range(std::unique_ptr<bio::filtering_istream> inputStream)
// //         : filter(std::move(inputStream))
// //     {
// //         // Assuming that the zstd_decompressor should be added in the constructor
// //         filter->push(bio::zstd_decompressor());
// //     }

// //     decompress_iterator begin() const { return decompress_iterator(*filter); }
// //     decompress_iterator end() const { return decompress_iterator(); }

// //     operator std::ranges::subrange<decompress_iterator>() const { return {begin(), end()}; }
// // };


// class decompress_range {
// public:
//     decompress_range(bio::filtering_istream stream)
//         : filter(std::make_unique<bio::filtering_istream>(std::move(stream))) {}

//     auto begin() const {
//         return decompress_iterator(*filter);
//     }

//     auto end() const {
//         return decompress_iterator();
//     }

// private:
//     std::unique_ptr<boost::iostreams::filtering_istream> filter;
// };

// class decompress_range_view : public std::ranges::view_base {
// public:
//     const decompress_range range;

//     explicit decompress_range_view(boost::iostreams::filtering_istream inputStream)
//         : range(std::move(inputStream)) {}

//     auto begin() const { return range.begin(); }
//     auto end() const { return range.end(); }

//     template <typename View>
//     auto operator|(View&& view) const {
//         return std::views::all(range) | std::forward<View>(view);
//     }
// };

// struct make_decompress_range_view : std::ranges::range_adaptor_closure<make_decompress_range_view> {
//     auto operator()(boost::iostreams::filtering_istream inputStream) const {
//         return decompress_range_view{std::move(inputStream)};
//     }
// };

// constexpr make_decompress_range_view decompress_range_to_view;




/*


class decompress_iterator : public std::iterator<std::input_iterator_tag, char> {
public:
    decompress_iterator() = default;

    decompress_iterator(boost::iostreams::filtering_istream& stream)
        : filter(&stream), current(), end() {
        ++(*this);  // Initialize the iterator to the first valid value
    }

    decompress_iterator& operator++() {
        if (filter && !(*filter)) {
            filter = nullptr;  // Invalidate the iterator when the stream is done
        } else {
            current = char();
            if ((*filter) >> current) {
                end = filter->peek();
            } else {
                filter = nullptr;  // Invalidate the iterator on failure
            }
        }
        return *this;
    }

    decompress_iterator operator++(int) {
        decompress_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    const char& operator*() const {
        return current;
    }

    friend bool operator==(const decompress_iterator& lhs, const decompress_iterator& rhs) {
        return lhs.filter == rhs.filter;
    }

    friend bool operator!=(const decompress_iterator& lhs, const decompress_iterator& rhs) {
        return !(lhs == rhs);
    }

private:
    boost::iostreams::filtering_istream* filter;
    char current;
    char end;  // To detect the end of the stream
};

class Stream {
public:
    Stream() = default;

    Stream(std::shared_ptr<boost::iostreams::filtering_istream> stream)
        : filterStream(std::move(stream)) {}

    // Copy constructor
    Stream(const Stream& other)
        : filterStream(other.filterStream) {}

    auto begin() const {
        return decompress_iterator(*filterStream);
    }

    auto end() const {
        return decompress_iterator();
    }

private:
    std::shared_ptr<boost::iostreams::filtering_istream> filterStream;
};

class decompress_range {
public:
    decompress_range(Stream stream)
        : stream(std::move(stream)) {}

    auto begin() const {
        return stream.begin();
    }

    auto end() const {
        return stream.end();
    }

private:
    Stream stream;
};

class decompress_range_view : public std::ranges::view_base {
public:
    const decompress_range range;

    explicit decompress_range_view(Stream stream)
        : range(std::move(stream)) {}

    auto begin() const { return range.begin(); }
    auto end() const { return range.end(); }

    template <typename View>
    auto operator|(View&& view) const {
        return std::views::all(range) | std::forward<View>(view);
    }
};

struct make_decompress_range_view : std::ranges::range_adaptor_closure<make_decompress_range_view> {
    auto operator()(std::shared_ptr<boost::iostreams::filtering_istream> stream) const {
        return decompress_range_view{Stream(std::move(stream))};
    }
};

constexpr make_decompress_range_view decompress_range_to_view;

*/

