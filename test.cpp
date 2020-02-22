#include <iostream>
#include <iomanip>
#include <utility>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include "nvec.h"

#define assert_throws(exp, t) try { exp; std::cerr << "didn't throw\n"; assert(false); } catch (const t&) {} catch (...) { std::cerr << "threw wrong type\n"; assert(false); }

int main()
{
	{
		nvec<int, 2> a1;
		assert(a1.empty());
		assert(a1.size<0>() == 0);
		assert(a1.size<1>() == 0);
		assert(a1.size() == 0);

		a1.resize(3, 4);
		assert(a1.size<0>() == 3);
		assert(a1.size<1>() == 4);
		assert(a1.size() == 12);

		assert(a1.flat_index(0, 0) == 0);
		assert(a1.flat_index(1, 3) == 7);
		assert(a1.flat_index(0, 2) == 2);
		assert(a1.flat_index(2, 0) == 8);

		a1.resize(9, 21);
		assert(a1.size<0>() == 9);
		assert(a1.size<1>() == 21);
		assert(a1.size() == 189);

		assert(a1.flat_index(0, 0) == 0);
		assert(a1.flat_index(1, 3) == 24);
		assert(a1.flat_index(0, 2) == 2);
		assert(a1.flat_index(2, 0) == 42);

		a1.resize(4, 2);
		assert(a1.size<0>() == 4);
		assert(a1.size<1>() == 2);
		assert(a1.size() == 8);

		assert(a1.flat_index(0, 0) == 0);
		assert(a1.flat_index(1, 1) == 3);
		assert(a1.flat_index(0, 1) == 1);
		assert(a1.flat_index(2, 0) == 4);

		a1.clear();
		assert(a1.empty());
		assert(a1.size<0>() == 0);
		assert(a1.size<1>() == 0);
		assert(a1.size() == 0);
	}

	{
		nvec<int, 4> a2(3, 4, 5, 6);
		assert(a2.size<0>() == 3);
		assert(a2.size<1>() == 4);
		assert(a2.size<2>() == 5);
		assert(a2.size<3>() == 6);
		assert(a2.size() == 360);

		assert(a2.flat_index(0, 0, 0, 0) == 0);
		assert(a2.flat_index(1, 2, 3, 4) == 202);
		assert(a2.flat_index(0, 0, 4, 2) == 26);
		assert(a2.flat_index(2, 0, 0, 1) == 241);
		assert(a2.flat_index(0, 3, 1, 0) == 96);
		assert(a2.flat_index(0, 3, 2, 0) == 102);

		a2.resize(2, 3, 0, 5);
		assert(a2.empty());
		assert(a2.size<0>() == 0);
		assert(a2.size<1>() == 0);
		assert(a2.size<2>() == 0);
		assert(a2.size<3>() == 0);
		assert(a2.size() == 0);

		a2.resize(1, 1, 1, 1);
		assert(a2.flat_index_bounded(0, 0, 0, 0) == 0);
		assert_throws(a2.flat_index_bounded(1, 0, 0, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 1, 0, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 0, 1, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 0, 0, 1), std::out_of_range);
		assert_throws(a2.flat_index_bounded(2, 0, 0, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 2, 0, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 0, 2, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 0, 0, 2), std::out_of_range);
		assert_throws(a2.at(1, 0, 0, 0), std::out_of_range);
		assert_throws(a2.at(0, 1, 0, 0), std::out_of_range);
		assert_throws(a2.at(0, 0, 1, 0), std::out_of_range);
		assert_throws(a2.at(0, 0, 0, 1), std::out_of_range);
		assert_throws(a2.at(2, 0, 0, 0), std::out_of_range);
		assert_throws(a2.at(0, 2, 0, 0), std::out_of_range);
		assert_throws(a2.at(0, 0, 2, 0), std::out_of_range);
		assert_throws(a2.at(0, 0, 0, 2), std::out_of_range);

		a2.resize(10, 10, 10, 10);
		assert(a2.flat_index_bounded(0, 0, 0, 0) == 0);
		assert(a2.flat_index_bounded(9, 0, 0, 0) == 9000);
		assert(a2.flat_index_bounded(0, 9, 0, 0) == 900);
		assert(a2.flat_index_bounded(0, 0, 9, 0) == 90);
		assert(a2.flat_index_bounded(0, 0, 0, 9) == 9);
		assert(a2.flat_index_bounded(9, 9, 9, 9) == 9999);
		assert_throws(a2.at(10000), std::out_of_range);
		assert_throws(a2.at(10001), std::out_of_range);
		assert_throws(a2.at(10002), std::out_of_range);
		assert_throws(a2.at(20000), std::out_of_range);
		assert_throws(a2.flat_index_bounded(10, 0, 0, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 10, 0, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 0, 10, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 0, 0, 10), std::out_of_range);
		assert_throws(a2.flat_index_bounded(11, 0, 0, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 11, 0, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 0, 11, 0), std::out_of_range);
		assert_throws(a2.flat_index_bounded(0, 0, 0, 11), std::out_of_range);
		assert_throws(a2.at(10, 0, 0, 0), std::out_of_range);
		assert_throws(a2.at(0, 10, 0, 0), std::out_of_range);
		assert_throws(a2.at(0, 0, 10, 0), std::out_of_range);
		assert_throws(a2.at(0, 0, 0, 10), std::out_of_range);
		assert_throws(a2.at(11, 0, 0, 0), std::out_of_range);
		assert_throws(a2.at(0, 11, 0, 0), std::out_of_range);
		assert_throws(a2.at(0, 0, 11, 0), std::out_of_range);
		assert_throws(a2.at(0, 0, 0, 11), std::out_of_range);
	}

	{
		nvec<int, 1> d(6);
		assert(d.size() == 6);
		assert(d.size<0>() == 6);
		assert(d.size(0) == 6);
		assert(d.size_unchecked(0) == 6);

		d.at(0) = 67; // make sure this isn't ambiguous between (flat) at(i) and at(...i)

		d.new_row();
		assert(d.size() == 7);
		assert(d.size<0>() == 7);
		assert(d.size(0) == 7);
		assert(d.size_unchecked(0) == 7);

		d.new_row();
		assert(d.size() == 8);
		assert(d.size<0>() == 8);
		assert(d.size(0) == 8);
		assert(d.size_unchecked(0) == 8);
	}

	{
		nvec<int, 2> c1(3, 5);
		nvec<int, 2> c2;
		assert(c1.size<0>() == 3 && c1.size<1>() == 5 && c1.size() == 15);
		assert(c2.empty() && c2.size<0>() == 0 && c2.size<1>() == 0 && c2.size() == 0);

		for (std::size_t i = 0; i < c1.size(); ++i) c1[i] = i;
		c2 = c1;
		assert(c2.size() == c1.size());
		for (std::size_t i = 0; i < c2.size(); ++i) { assert(c2[i] == i); }

		for (std::size_t i = 0; i < c1.size(); ++i) std::cout << c1[i] << ' ';
		std::cout << '\n';
		for (int i : c1) std::cout << i << ' ';
		std::cout << '\n';
		for (std::size_t i = 0; i < c2.size<0>(); ++i) for (std::size_t j = 0; j < c2.size<1>(); ++j) std::cout << c2(i, j) << ' ';
		std::cout << '\n';
		for (int &i : c2) std::cout << i << ' ';
		std::cout << "\n\n";

		c1.clear();
		assert(c1.empty() && c1.size<0>() == 0 && c1.size<1>() == 0 && c1.size() == 0);
		c1 = std::move(c2);
		assert(c2.empty() && c2.size<0>() == 0 && c2.size<1>() == 0 && c2.size() == 0);
		assert(!c1.empty() && c1.size<0>() == 3 && c1.size<1>() == 5 && c1.size() == 15);
		for (std::size_t i = 0; i < c1.size(); ++i) { assert(c1[i] == i); }

		using std::swap;

		swap(c1, c2);
		assert(c1.empty() && c1.size<0>() == 0 && c1.size<1>() == 0 && c1.size() == 0);
		assert(!c2.empty() && c2.size<0>() == 3 && c2.size<1>() == 5 && c2.size() == 15);
		for (std::size_t i = 0; i < c2.size(); ++i) { assert(c2[i] == i); }

		swap(c1, c2);
		assert(c2.empty() && c2.size<0>() == 0 && c2.size<1>() == 0 && c2.size() == 0);
		assert(!c1.empty() && c1.size<0>() == 3 && c1.size<1>() == 5 && c1.size() == 15);
		for (std::size_t i = 0; i < c1.size(); ++i) { assert(c1[i] == i); }

		swap(c1, c1);
		assert(!c1.empty() && c1.size<0>() == 3 && c1.size<1>() == 5 && c1.size() == 15);
		for (std::size_t i = 0; i < c1.size(); ++i) { assert(c1[i] == i); }

		c1 = c1;
		assert(!c1.empty() && c1.size<0>() == 3 && c1.size<1>() == 5 && c1.size() == 15);
		for (std::size_t i = 0; i < c1.size(); ++i) { assert(c1[i] == i); }

		c1 = std::move(c1);
		assert(!c1.empty() && c1.size<0>() == 3 && c1.size<1>() == 5 && c1.size() == 15);
		for (std::size_t i = 0; i < c1.size(); ++i) { assert(c1[i] == i); }

		nvec<int, 2> cc1(c1);
		assert(!c1.empty());
		assert(!cc1.empty() && cc1.size<0>() == 3 && cc1.size<1>() == 5 && cc1.size() == 15);
		for (std::size_t i = 0; i < cc1.size(); ++i) { assert(cc1[i] == i); }

		nvec<int, 2> cc2(std::move(cc1));
		assert(cc1.empty());
		assert(!cc2.empty() && cc2.size<0>() == 3 && cc2.size<1>() == 5 && cc2.size() == 15);
		for (std::size_t i = 0; i < cc2.size(); ++i) { assert(cc2[i] == i); }
	}

	{
		nvec<bool, 3> bool_arr(8, 8, 8);
		assert(bool_arr.size<0>() == 8);
		assert(bool_arr.size<1>() == 8);
		assert(bool_arr.size<2>() == 8);

		assert(bool_arr.size(0) == 8);
		assert(bool_arr.size(1) == 8);
		assert(bool_arr.size(2) == 8);

		for (std::size_t i = 0; i < bool_arr.size<0>(); ++i)
			for (std::size_t j = 0; j < bool_arr.size<1>(); ++j)
				for (std::size_t k = 0; k < bool_arr.size<2>(); ++k)
					bool_arr(i, j, k) = true;

		bool_arr.reshape(64, 1, 8);
		assert(bool_arr.size<0>() == 64);
		assert(bool_arr.size<1>() == 1);
		assert(bool_arr.size<2>() == 8);

		assert(bool_arr.size(0) == 64);
		assert(bool_arr.size(1) == 1);
		assert(bool_arr.size(2) == 8);

		bool_arr.reshape(1, 512, 1);
		assert(bool_arr.size<0>() == 1);
		assert(bool_arr.size<1>() == 512);
		assert(bool_arr.size<2>() == 1);

		assert(bool_arr.size(0) == 1);
		assert(bool_arr.size(1) == 512);
		assert(bool_arr.size(2) == 1);

		assert_throws(bool_arr.reshape(1, 511, 1), std::invalid_argument);
		assert_throws(bool_arr.reshape(1, 513, 1), std::invalid_argument);

		assert(bool_arr.size<0>() == 1);
		assert(bool_arr.size<1>() == 512);
		assert(bool_arr.size<2>() == 1);
		bool_arr.reshape(2, 8, 32);
		assert(bool_arr.size<0>() == 2);
		assert(bool_arr.size<1>() == 8);
		assert(bool_arr.size<2>() == 32);
		assert(bool_arr.size() == 512);

		bool_arr.new_row();
		assert(bool_arr.size<0>() == 3);
		assert(bool_arr.size<1>() == 8);
		assert(bool_arr.size<2>() == 32);
		assert(bool_arr.size() == 768);

		bool_arr.new_row(false);
		assert(bool_arr.size<0>() == 4);
		assert(bool_arr.size<1>() == 8);
		assert(bool_arr.size<2>() == 32);
		assert(bool_arr.size() == 1024);
	}

	{
		nvec<int, 2> eq1(3, 5);
		std::iota(eq1.begin(), eq1.end(), 0);
		nvec<int, 2> eq2 = eq1;

		assert(!eq1.empty() && eq1.size<0>() == 3 && eq1.size<1>() == 5 && eq1.size() == 15);
		for (std::size_t i = 0; i < eq1.size(); ++i) { assert(eq1[i] == i); }

		assert(!eq2.empty() && eq2.size<0>() == 3 && eq2.size<1>() == 5 && eq2.size() == 15);
		for (std::size_t i = 0; i < eq2.size(); ++i) { assert(eq2[i] == i); }

		assert(eq1 == eq2 && eq2 == eq1);
		assert(!(eq1 != eq2) && !(eq2 != eq1));

		assert(eq1.flat() == eq2.flat() && eq2.flat() == eq1.flat());

		eq1.resize(5, 3);

		assert(eq1 != eq2 && eq2 != eq1);
		assert(!(eq1 == eq2) && !(eq2 == eq1));

		assert(eq1.flat() == eq2.flat() && eq2.flat() == eq1.flat());

		eq1.reserve(75);
		assert(!eq1.empty() && eq1.size<0>() == 5 && eq1.size<1>() == 3 && eq1.size() == 15);
		eq1.capacity();

		assert(eq1 != eq2 && eq2 != eq1);
		assert(!(eq1 == eq2) && !(eq2 == eq1));

		assert(eq1.flat() == eq2.flat() && eq2.flat() == eq1.flat());
	}

	{
		std::vector<int> vec(22);
		for (std::size_t i = 0; i < vec.size(); ++i) vec[i] = i;
		assert(vec.size() == 22);

		nvec<int, 1> nvec1;
		assert(nvec1.size() == 0);
		nvec1.reshape_from(vec);
		assert(nvec1.size() == 22);

		assert(nvec1.flat() == vec);

		std::vector<int> vec2 = std::move(nvec1).to_flat();
		assert(nvec1.empty());
		assert(nvec1.size() == 0);
		assert(nvec1.size<0>() == 0);

		nvec<int, 2> nvec2;
		assert(nvec2.size() == 0 && nvec2.empty());

		assert_throws(nvec2.reshape_from(std::move(vec2), 11, 3), std::invalid_argument);
		nvec2.reshape_from(std::move(vec2), 11, 2);

		assert(nvec2.size() == 22);
		for (std::size_t i = 0; i < nvec2.size(); ++i) { assert(nvec2[i] == i); }

		nvec<int, 3> nvec3;
		nvec3.reshape_from(nvec2, 11, 2, 1);
		assert(nvec3.flat() == nvec2.flat());

		nvec2.resize(3, 0);
		assert(nvec2.empty());
		assert(nvec2.size() == 0);
		assert(nvec2.size<0>() == 0);
		assert(nvec2.size<1>() == 0);

		nvec3.resize(3, 0, 1, -56);
		assert(nvec3.empty());
		assert(nvec3.size() == 0);
		assert(nvec3.size<0>() == 0);
		assert(nvec3.size<1>() == 0);
		assert(nvec3.size<2>() == 0);

		nvec3.reshape(32, 0, 128);
		assert(nvec3.empty());
		assert(nvec3.size() == 0);
		assert(nvec3.size<0>() == 0);
		assert(nvec3.size<1>() == 0);
		assert(nvec3.size<2>() == 0);
	}

	{
		nvec<int, 2> f, g;
		f.reshape_from(g, 0, 2);
		assert(f.size<0>() == 0);
		assert(f.size<1>() == 0);
	}
	{
		nvec<int, 2> f, g;
		f.reshape_from(std::move(g), 0, 2);
		assert(f.size<0>() == 0);
		assert(f.size<1>() == 0);
	}

	std::cout << "all tests completed\n";
	std::cin.get();
	return 0;
}
