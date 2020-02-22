#ifndef DRAGAZO_NVEC_H
#define DRAGAZO_NVEC_H

#include <utility>
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace detail
{
	template<std::size_t> using size_t_t = std::size_t; // maps all numbers to std::size_t - used for var args

	template<typename T, typename I, typename Allocator> class nvec;

	// represents a sizeof...(I)-dimensional flattened array of T - DO NOT USE THIS DIRECTLY!!
	// I must be [0, sizeof...(I)) in ascending order - otherwise undefined behavior.
	template<typename T, typename Allocator, std::size_t ...I>
	class nvec<T, std::index_sequence<I...>, Allocator>
	{
	private: // -- data -- //

		static_assert(sizeof...(I) != 0);
		friend class nvec;

		std::vector<T, Allocator> arr;             // the raw flattened array
		std::array<std::size_t, sizeof...(I)> dim; // the lengths of each dimension

	public: // -- types -- //

		typedef T         value_type;
		typedef Allocator allocator_type;

		typedef std::size_t    size_type;
		typedef std::ptrdiff_t difference_type;

		typedef typename std::vector<T, Allocator>::reference       reference;
		typedef typename std::vector<T, Allocator>::const_reference const_reference;

		typedef typename std::vector<T, Allocator>::pointer       pointer;
		typedef typename std::vector<T, Allocator>::const_pointer const_pointer;

		typedef typename std::vector<T, Allocator>::iterator       iterator;
		typedef typename std::vector<T, Allocator>::const_iterator const_iterator;

		typedef typename std::vector<T, Allocator>::reverse_iterator       reverse_iterator;
		typedef typename std::vector<T, Allocator>::const_reverse_iterator const_reverse_iterator;

	public: // -- ctor / dtor / asgn -- //

		// creates an empty array
		nvec() { dim = { (I, 0)... }; }

		// creates an array with the specified initial dimensions.
		// if any of the specified dimensions is zero the result is empty.
		explicit nvec(size_t_t<I> ...init_dim) { resize(init_dim...); }
		explicit nvec(size_t_t<I> ...init_dim, const value_type &value) { resize(init_dim..., value); }

		// creates a copy of the other flat array, with the same dimensions and elements
		nvec(const nvec &other) : arr(other.arr), dim(other.dim) {}
		// creates a new flat array with the resources of other - other is guaranteed empty after this operation
		nvec(nvec &&other) noexcept : arr(std::move(other.arr)), dim(std::move(other.dim)) {}

		// copies the contents of other to this array - this changes both our dimensions and stored elements - self-assignment is safe
		nvec &operator=(const nvec &other) { arr = other.arr; dim = other.dim; return *this; }
		// moves the contents of other into this object - other is guaranteed empty after this operation - self-assignment is no-op
		nvec &operator=(nvec &&other) noexcept(noexcept(arr = std::move(other.arr)))
		{
			if (&other != this)
			{
				arr = std::move(other.arr);
				dim = std::move(other.dim);
				other.clear();
			}
			return *this;
		}

	public: // -- conversion -- //

		// accesses the nvec as a flattened standard array
		const std::vector<T, Allocator> &flat() const noexcept { return arr; }

		// converts this nvec into a (flattened) standard array by copying its internal representation
		std::vector<T, Allocator> to_flat() const& { return arr; }
		// converts this nvec into a (flattend) standard array by moving from its internal representation.
		// this nvec is guaranteed to be empty after this operation.
		std::vector<T, Allocator> to_flat() && noexcept { auto t = std::move(arr); clear(); return t; }

		// takes another nvec of any dimensionality and assigns/reshapes its content into this nvec with the specified dimensions.
		// throws std::invalid_argument if the total number of elements would differ before and after.
		template<std::size_t ...J>
		void reshape_from(const nvec<T, std::index_sequence<J...>, Allocator> &other, size_t_t<I> ...new_dim)
		{
			if ((... * new_dim) != other.arr.size()) throw std::invalid_argument("reshape_from(): new and old sizes differ");
			if (other.arr.empty()) clear(); else { arr = other.arr; dim = { new_dim... }; }
		}
		// allows for conversion from standard vectors to nvecs.
		void reshape_from(const std::vector<T, Allocator> &other, size_t_t<I> ...new_dim)
		{
			if ((... * new_dim) != other.size()) throw std::invalid_argument("reshape_from(): new and old sizes differ");
			if (other.empty()) clear(); else { arr = other; dim = { new_dim... }; }
		}
		// special case for converting standard vectors into 1D nvecs (dimensions implied by context and bounds check can be omitted)
		template<int _ = 0, std::enable_if_t<_ == 0 && (sizeof...(I) == 1), int> = 0>
		void reshape_from(const std::vector<T, Allocator> &other) { arr = other; dim[0] = arr.size(); }

		// performs the same reshape_from() operation as other overloads, by moves from the source nvec.
		// other is guaranteed to be empty after this operation.
		template<std::size_t ...J>
		void reshape_from(nvec<T, std::index_sequence<J...>, Allocator> &&other, size_t_t<I> ...new_dim)
		{
			if ((... * new_dim) != other.arr.size()) throw std::invalid_argument("reshape_from(): new and old sizes differ");
			if constexpr (std::is_same_v<std::remove_reference_t<decltype(other)>, nvec>) { if (&other == this) return; }
			if (other.arr.empty()) clear(); else { arr = std::move(other.arr); dim = { new_dim... }; other.clear(); }
		}
		// move semantics overload for standard vecs.
		// other is left in a valid but unspecified state after this operation.
		void reshape_from(std::vector<T, Allocator> &&other, size_t_t<I> ...new_dim)
		{
			if ((... * new_dim) != other.size()) throw std::invalid_argument("reshape_from(): new and old sizes differ");
			if (other.empty()) clear(); else { arr = std::move(other); dim = { new_dim... }; }
		}
		// move semantics overload for standard vecs into 1D nvecs.
		// other is left in a valid but unspecified state after this operation.
		template<int _ = 0, std::enable_if_t<_ == 0 && (sizeof...(I) == 1), int> = 0>
		void reshape_from(std::vector<T, Allocator> &&other) { arr = std::move(other); dim[0] = arr.size(); }

	public: // -- utility -- //

		// adds a new (hyper) row to the nvec. because nvec is stored in row major order all pre-existing items and their indexes are unaltered.
		// equivalent to resize(d1 + 1, d2, d3, ...) where dn is the current size of each dimension.
		// note that if this nvec is currently empty then the result is also empty.
		void new_row() { if (dim[0]) arr.resize(arr.size() + arr.size() / dim[0]++); }
		void new_row(const value_type &value) { if (dim[0]) arr.resize(arr.size() + arr.size() / dim[0]++, value); }

		// resizes the flattened array to the specified dimensions.
		// if any of the dimensions is zero, this is equivalent to clear().
		// shrinking the total length of the array destroys objects at the end.
		// growing the total length of the array constructs new objects at the end (default inserted (no value param), or copied from provided value).
		// objects present in both ranges are still present and are not moved (row-major flattened array structure).
		void resize(size_t_t<I> ...new_dim)
		{
			arr.resize((... * new_dim));
			if (arr.empty()) dim = { (I, 0)... }; else dim = { new_dim... };
		}
		void resize(size_t_t<I> ...new_dim, const value_type &value)
		{
			arr.resize((... * new_dim), value);
			if (arr.empty()) dim = { (I, 0)... }; else dim = { new_dim... };
		}

		// as resize() for changing dimensions, but the total number of objects must be the same
		// if total size differs, throws std::invalid_argument
		void reshape(size_t_t<I> ...new_dim)
		{
			if ((... * new_dim) != arr.size()) throw std::invalid_argument("reshape(): new and old sizes differ");
			if (!arr.empty()) dim = { new_dim... };
		}

		// destroys all contained objects and sets (all) dimensions to 0 - the container is empty after this operation.
		void clear() noexcept
		{
			arr.clear();
			dim = { (I, 0)... };
		}

		// hints to reserve space for at least new_cap elements
		void reserve(std::size_t new_cap) { arr.reserve(new_cap); }
		// gets the current capacity of the array (not the same as size)
		std::size_t capacity() const noexcept { return arr.capacity(); }

	public: // -- query -- //

		// returns the total size (total number of elements)
		std::size_t size() const noexcept { return arr.size(); }

		// returns the size of dimension P
		template<std::size_t P, std::enable_if_t<(P < sizeof...(I)), int> = 0>
		std::size_t size() const noexcept { return dim[P]; }
		// returns the size of dimension p. equivalent to size<p>() except that p is not required to be a constant expression.
		// as a consequence, p is bounds checked at runtime
		std::size_t size(std::size_t p) const { return p >= sizeof...(I) ? throw std::out_of_range("dimension index out of bounds") : dim[p]; }
		// as size(p) except that bounds checking is not performed
		std::size_t size_unchecked(std::size_t p) const { return dim[p]; }

		// returns true iff the array is empty (which means there are no objects and all dimensions are zero)
		bool empty() const noexcept { return arr.empty(); }

		// returns the flattened index of the location with no bounds checking whatsoever
		std::size_t flat_index(size_t_t<I> ...index) const noexcept
		{
			std::size_t res = 0;
			int _[] = { (res = res * dim[I] + index, 0)... };
			return res;
		}
		// returns the flattened index of the location with additional bounds checking for each dimension
		std::size_t flat_index_bounded(size_t_t<I> ...index) const
		{
			if ((... || (index >= dim[I]))) throw std::out_of_range("index out of bounds");
			return flat_index(index...);
		}

	public: // -- access -- //

		// gets the element at the specified flattened index without bounds checking
		decltype(auto) operator[](std::size_t index) { return arr[index]; }
		decltype(auto) operator[](std::size_t index) const { return arr[index]; }

		// gets the ekement at the specified flattened index with additional bounds checking
		decltype(auto) at(std::size_t index) { return arr.at(index); }
		decltype(auto) at(std::size_t index) const { return arr.at(index); }

		// gets the element at the specified location with no bounds checking whatsoever
		decltype(auto) operator()(size_t_t<I> ...index) { return arr[flat_index(index...)]; }
		decltype(auto) operator()(size_t_t<I> ...index) const { return arr[flat_index(index...)]; }

		// gets the element at the specified location with additional bounds checking for each dimension
		template<int _ = 0, std::enable_if_t<_ == 0 && (sizeof...(I) > 1), int> = 0>
		decltype(auto) at(size_t_t<I> ...index) { return arr[flat_index_bounded(index...)]; }
		template<int _ = 0, std::enable_if_t<_ == 0 && (sizeof...(I) > 1), int> = 0 >
		decltype(auto) at(size_t_t<I> ...index) const { return arr[flat_index_bounded(index...)]; }

		// returns the first element in the flattened array
		decltype(auto) front() { return arr.front(); }
		decltype(auto) front() const { return arr.front(); }

		// returns the last element in the flattened array
		decltype(auto) back() { return arr.back(); }
		decltype(auto) back() const { return arr.back(); }

	public: // -- iteration -- //

		// iterates through all items in the flattened array
		decltype(auto) begin() { return arr.begin(); }
		decltype(auto) begin() const { return arr.begin(); }
		decltype(auto) cbegin() const { return arr.cbegin(); }

		decltype(auto) end() { return arr.end(); }
		decltype(auto) end() const { return arr.end(); }
		decltype(auto) cend() const { return arr.cend(); }

		decltype(auto) rbegin() { return arr.rbegin(); }
		decltype(auto) rbegin() const { return arr.rbegin(); }
		decltype(auto) crbegin() const { return arr.crbegin(); }

		decltype(auto) rend() { return arr.rend(); }
		decltype(auto) rend() const { return arr.rend(); }
		decltype(auto) crend() const { return arr.crend(); }

	public: // -- comparison -- //

		// returns true iff the arrays have the same dimensions and the same contents
		friend bool operator==(const nvec &a, const nvec &b) { return a.dim == b.dim && a.arr == b.arr; }
		// returns true iff the arrays have different domensions or different contents
		friend bool operator!=(const nvec &a, const nvec &b) { return a.dim != b.dim || a.arr != b.arr; }

	public: // -- swap -- //

		// swaps the contents of a and b
		friend void swap(const nvec &a, const nvec &b)
		{
			using std::swap;
			swap(a.arr, b.arr);
			swap(a.dim, b.dim);
		}
	};
}

// user-level alias for a dynamic D-dimensional flattened array of T
template<typename T, std::size_t D, typename Allocator = std::allocator<T>>
using nvec = detail::nvec<T, std::make_index_sequence<D>, Allocator>;

#endif
