#pragma once
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/adaptor/reversed.hpp>


namespace fcl {
    template <typename... T>
	auto zip(const T&... containers)
	    -> boost::iterator_range<boost::zip_iterator<decltype(boost::make_tuple(std::begin(containers)...))>> {
	    auto zip_begin = boost::make_zip_iterator(boost::make_tuple(std::begin(containers)...));
	    auto zip_end = boost::make_zip_iterator(boost::make_tuple(std::end(containers)...));
	    return boost::make_iterator_range(zip_begin, zip_end);
	}

	template <typename N>
	auto range(const N from, const N to)
	    -> decltype(boost::make_iterator_range(boost::counting_iterator<N>(from), boost::counting_iterator<N>(to))) {
	    return boost::make_iterator_range(boost::counting_iterator<N>(from), boost::counting_iterator<N>(to));
	}

	template
	auto rrange(const N from, const N downto)
	    -> {
	    return boost::adaptors::reverse(range(downto, from));
	}
}
