#pragma once
#include <type_traits>
#include <fstream>

namespace michelemei {

template<typename Head, typename ...Tail>
struct _last {
	using value = typename _last<Tail...>::value;
};
template<typename Head>
struct _last<Head> {
	using value = Head;
};
/// Returns the last element
template<typename Head, typename ...Tail>
using last_t = typename _last<Head, Tail...>::value;

template<typename Head, typename ...Tail>
struct _first {
	using value = Head;
};
// Returns the first element
template<typename ...Tail>
using first_t = typename _first<Tail...>::value;

template<bool Condition, typename True = void, typename False = void>
struct _if {};
template<typename True, typename False>
struct _if<true, True, False> {
	using type = True;
};
template<typename True, typename False>
struct _if<false, True, False> {
	using type = False;
};
/// If statement for static types
/// @brief Returns True type if Condition is true, False otherwise
template<bool Condition, typename True, typename False>
using if_t = typename _if<Condition, True, False>::type;

/// A list type defintion
struct _list_t {
	using type = void;
};

template<typename ...Tail>
struct first_or_list_type {
	static_assert(sizeof...(Tail) > 0, "You are looking for the first element of an empty parameter pack!");
	using type = if_t<sizeof...(Tail) == 1, first_t<Tail...>, _list_t>;
};

/// A base class for composite (, usefull for std::is_base_of usage)
struct _composite_base {};

/// A compile-time implementation of Composite Pattern
template<size_t Id, typename ...Ts>
class _composite : public _composite_base, public std::integral_constant<size_t, Id>
{
	// get_size()
	template<typename T>
	static constexpr std::enable_if_t<!std::is_base_of_v<_composite_base, T>, size_t> get_size() {
		return sizeof(T);
	}
	template<typename T>
	static constexpr std::enable_if_t<std::is_base_of_v<_composite_base, T>, size_t> get_size() {
		return T::footprint;
	}

	// offset
	template<typename I, typename Head, typename ...Tail>
	struct _offset
		: std::integral_constant<size_t,
		I::value == Head::value ? 0 : Head::footprint + _offset<I, Tail...>::value> {
	};
	template<typename I, typename Head>
	struct _offset<I, Head>
		: std::integral_constant<size_t, 0> {
	};
	template<typename Head, typename ...Tail>
	struct _composite_offset
		: std::integral_constant<size_t,
		_composite_offset<Head>::value +
		Head::type::template offset<Tail...>> {
	};
	template<typename Head>
	struct _composite_offset<Head>
		: std::integral_constant<size_t, _offset<Head, Ts...>::value> {
	};

	// _get(void*)
	template<typename Head, typename ...Tail>
	static constexpr char * _get(void* ptr) {
		return (static_cast<char *>(ptr) + offset<Head, Tail...>);
	}
public:
	static_assert(sizeof...(Ts) > 0, "???");

	/// _composite type
	/// @brief If template variadic parameter Ts is greater than 1, type is an _list_type otherwise it assume the first (and unique) type
	using type = typename first_or_list_type<Ts...>::type;

	/// Get the memory footprint (aka "size")
	static constexpr size_t footprint = ((get_size<Ts>()) + ...);

	/// Get the item's offset
	template<typename Head, typename ...Tail>
	static constexpr size_t offset = _composite_offset<Head, Tail...>::value;

	template<typename Head, typename ...Tail>
	static constexpr const typename last_t<Head, Tail...>::type& get(void* ptr) {
		return *(static_cast<typename last_t<Head, Tail...>::type *>(static_cast<void*>(_get<Head, Tail...>(ptr))));
	}

	template<typename Head, typename ...Tail>
	static constexpr void set(void* ptr, typename last_t<Head, Tail...>::type&& value) {
		*(static_cast<typename last_t<Head, Tail...>::type *>(static_cast<void*>(_get<Head, Tail...>(ptr)))) = value;
	}
};
#define atom(...) michelemei::_composite<__COUNTER__,__VA_ARGS__>
#define pack(...) using data = michelemei::_composite<__COUNTER__,__VA_ARGS__>

template<typename Mask>
struct stencil {
	explicit stencil(void *_ptr = nullptr) : ptr(_ptr), manage_ptr(ptr == nullptr) {
		if (ptr == nullptr) {
			ptr = malloc(Mask::data::template footprint);
		} // else nothing to do
	}

	explicit stencil(std::istream& is) : stencil(nullptr) {
		// @todo: manage errors
		is.read(static_cast<char*>(ptr), Mask::data::template footprint);
	}

	stencil(const stencil& rhv) : stencil(nullptr) {
		memcpy(ptr, rhv.ptr, Mask::template footprint);
	}

	stencil(stencil&& rhv) noexcept {
		ptr = rhv.ptr;
		manage_ptr = rhv.ptr;
		rhv.ptr = nullptr;
		rhv.manage_ptr = false;
	}

	// @todo: implement!
	stencil& operator=(const stencil& rhv) = delete;

	// @todo: implement!
	stencil& operator=(stencil&& rhv) = delete;

	virtual ~stencil() {
		if (manage_ptr) {
			delete static_cast<char*>(ptr);
		} // else nothing to do
	}

	template<typename Head, typename ...Tail>
	const typename last_t<Head, Tail...>::type& get() const {
		return Mask::data::template get<Head, Tail...>(ptr);
	}

	template<typename Head, typename ...Tail>
	void set(typename last_t<Head, Tail...>::type&& value) {
		Mask::data::template set<Head, Tail...>(ptr, std::forward<typename last_t<Head, Tail...>::type>(value));
	}

	void write(std::ostream& os) const {
		os.write(static_cast<char*>(ptr), Mask::data::template footprint);
	}

private:
	void* ptr;
	bool manage_ptr;
};

}
