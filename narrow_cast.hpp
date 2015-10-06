#pragma once
#include <numeric>

namespace fcl {
template< typename to, typename from >
	to num_narrow_cast( from value ) {
	    static_assert( std::numeric_limits< to >::max() < std::numeric_limits< from >::max(),
	        "narrow_cast used in non-narrowing context" );

	    return static_cast< to >( from %
	        ( static_cast< from >( std::numeric_limits< to >::max() ) + 1 ) ) );
	}

	#include <cassert>
	#include <stdexcept>

	//Copied from
	// * Bjarne Stroustrup. The C++ Programming Language (4th edition). 2013.
	//   ISBN: 978-0-321-56384-2. Chapter 11.5: Explicit type conversion. page 299.
	template< class to, class from >
	to narrow_cast(from v) {
	    auto r = static_cast<to>(v);
	    if (static_cast<from>(r) != v) {
	        throw std::runtime_error("narrow_cast<>() failed");
	    }

	    return r;
	}
}
