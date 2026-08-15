//--------------------------------------------------------------------------------------------------
//
//	Coordinates: A compile-time c++23 coordinate conversion library based on `units`
//
//--------------------------------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------------------------------------------------------------
//
// Copyright (c) 2016 Nic Holthaus
//
//--------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//
/// @brief      
/// @details    
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef COORDINATES_CACHETEST_H
#define COORDINATES_CACHETEST_H

//#define TEST_OBJECT_OUTPUT	// for this to work, has to be defined before TestObject.h is included.
#include <gtest/gtest.h>
#include <cache.h>
#include <TestObject.h>
#include <iostream>
#include <type_traits>

#define COMMA ,	// this terrible kludge gets us around the gtest MACRO parsing limitations, see: http://stackoverflow.com/questions/13842468/comma-in-c-c-macro
// in hindsight, it would have been better to use double parenthesis.

/*extern template Cache<size_t, TestObject>;*/
using TestCache = Cache<size_t, TestObject>;
using TestMap = std::unordered_map<size_t, TestObject>;

namespace
{
	// The fixture for testing class Cache
	class cacheTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		cacheTest()
		{
			// You can do set-up work for each test here.
		}

		virtual ~cacheTest()
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:

		virtual void SetUp()
		{
			// Code here will be called immediately after the constructor (right
			// before each test).
		}

		virtual void TearDown()
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}

		// Objects declared here can be used by all tests in the test case for Foo.
	};

	// The fixture for testing iterators.
	// These tests assume that the CircularQueue members work!
	class cacheIteratorTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		cacheIteratorTest()
		{

		}

		virtual ~cacheIteratorTest()
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

	};

	TEST_F(cacheIteratorTest, defaultConstructor)
	{
		TestObject::clearfunctioncalls();
		EXPECT_TRUE(std::is_default_constructible<TestCache::iterator>::value);
		EXPECT_TRUE(std::is_default_constructible<TestCache::const_iterator>::value);
	}

	TEST_F(cacheIteratorTest, copyConstructor)
	{
		TestObject::clearfunctioncalls();
 		EXPECT_TRUE(std::is_copy_constructible<TestCache::iterator>::value);
		EXPECT_TRUE(std::is_copy_constructible<TestCache::const_iterator>::value);
	}

	TEST_F(cacheIteratorTest, copyAssignment)
	{
		TestObject::clearfunctioncalls();
 		EXPECT_TRUE(std::is_copy_assignable<TestCache::iterator>::value);
		EXPECT_TRUE(std::is_copy_assignable<TestCache::const_iterator>::value);
	}

	TEST_F(cacheIteratorTest, equal)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 1, 'a' } };
			EXPECT_EQ(1, test.size());

			auto b1 = test.begin();
			auto b2 = test.begin();
			auto b3 = test.cbegin();

			EXPECT_EQ(b1, b2);
			EXPECT_EQ(b1, b3);
			EXPECT_EQ(b2, b3);
			EXPECT_EQ(b3, b2);
			EXPECT_EQ(b3, b1);
		}

		EXPECT_EQ(1, TestObject::getconstructorcalls());
		EXPECT_EQ(2, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheIteratorTest, notEqual)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 1, 'a' } };
			EXPECT_EQ(1, test.size());

			auto b = test.begin();
			auto e = test.end();

			EXPECT_NE(b, e);

			auto cb = test.cbegin();
			auto ce = test.cend();

			EXPECT_NE(cb, ce);

			EXPECT_NE(b, ce);
			EXPECT_NE(cb, e);
		}

		EXPECT_EQ(1, TestObject::getconstructorcalls());
		EXPECT_EQ(2, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheIteratorTest, prefixIncrement)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };

			EXPECT_EQ(4, cache.size());

			TestCache::iterator itr = cache.begin();
			EXPECT_STREQ("a", (itr)->getString().c_str());
			EXPECT_STREQ("b", (++itr)->getString().c_str());
			EXPECT_STREQ("b", (itr)->getString().c_str());

			TestCache::const_iterator citr = cache.cbegin();
			EXPECT_STREQ("a", (citr)->getString().c_str());
			EXPECT_STREQ("b", (++citr)->getString().c_str());
			EXPECT_STREQ("b", (citr)->getString().c_str());
		}

		EXPECT_EQ(4, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(6, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheIteratorTest, postfixIncrement)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };

			EXPECT_EQ(4, cache.size());

			TestCache::iterator itr = cache.begin();
			EXPECT_STREQ("a", (itr)->getString().c_str());
			EXPECT_STREQ("a", (itr++)->getString().c_str());
			EXPECT_STREQ("b", (itr)->getString().c_str());

			TestCache::const_iterator citr = cache.cbegin();
			EXPECT_STREQ("a", (citr)->getString().c_str());
			EXPECT_STREQ("a", (citr++)->getString().c_str());
			EXPECT_STREQ("b", (citr)->getString().c_str());
		}

		EXPECT_EQ(4, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(6, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheIteratorTest, prefixDecrement)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };

			EXPECT_EQ(4, cache.size());

			TestCache::iterator itr = --cache.end();
			EXPECT_STREQ("d", (itr)->getString().c_str());
			EXPECT_STREQ("c", (--itr)->getString().c_str());
			EXPECT_STREQ("c", (itr)->getString().c_str());

			TestCache::const_iterator citr = --cache.cend();
			EXPECT_STREQ("d", (citr)->getString().c_str());
			EXPECT_STREQ("c", (--citr)->getString().c_str());
			EXPECT_STREQ("c", (citr)->getString().c_str());
		}

		EXPECT_EQ(4, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(6, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheIteratorTest, postfixDecrement)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };

			EXPECT_EQ(4, cache.size());

			TestCache::iterator itr = --cache.end();
			EXPECT_STREQ("d", (itr)->getString().c_str());
			EXPECT_STREQ("d", (itr--)->getString().c_str());
			EXPECT_STREQ("c", (itr)->getString().c_str());

			TestCache::const_iterator citr = --cache.cend();
			EXPECT_STREQ("d", (citr)->getString().c_str());
			EXPECT_STREQ("d", (citr--)->getString().c_str());
			EXPECT_STREQ("c", (citr)->getString().c_str());
		}

		EXPECT_EQ(4, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(6, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheIteratorTest, dereference)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 1, 'a' } };
			EXPECT_EQ(1, test.size());
			EXPECT_STREQ("a", test.begin()->getString().c_str());
			EXPECT_STREQ("a", (*test.begin()).getString().c_str());

			*test.begin() = 'b';

			EXPECT_STREQ("b", test[1].getString().c_str());

			EXPECT_STREQ("b", test.cbegin()->getString().c_str());
			EXPECT_STREQ("b", (*test.cbegin()).getString().c_str());
		}

		EXPECT_EQ(1, TestObject::getconstructorcalls());
		EXPECT_EQ(2, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(1, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(5, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheIteratorTest, convertToConst)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 1, 'a' } };
			EXPECT_EQ(1, test.size());

			auto begin = test.begin();
			EXPECT_STREQ("a", begin->getString().c_str());

			TestCache::const_iterator cbegin = begin;
			EXPECT_STREQ("a", cbegin->getString().c_str());
		}

		EXPECT_EQ(1, TestObject::getconstructorcalls());
		EXPECT_EQ(2, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(2, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheIteratorTest, swap)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			std::string output[4] = { "d", "c", "b", "a" };

			EXPECT_EQ(4, cache.size());

			auto a = cache.begin();
			auto b = cache.end();
			auto begin = cache.begin();
			auto end = cache.end();

			EXPECT_EQ(a, begin);
			EXPECT_EQ(b, end);

			std::swap(a, b);

			EXPECT_EQ(a, end);
			EXPECT_EQ(b, begin);
		}

		EXPECT_EQ(4, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, defaultConstructor)
	{
		TestObject::clearfunctioncalls();
		EXPECT_TRUE(std::is_default_constructible<TestCache>::value);

		// just allocate memory, no construction
		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(0, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());

		// w/allocator
		{
			auto alloc = std::allocator<std::pair<const size_t, TestObject>>();
			TestCache test(alloc);
		}

		// just allocate memory, no construction
		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(0, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, rangeConstructorMap)
	{
		TestObject::clearfunctioncalls();
		{
			// map size < n
			std::unordered_map<size_t, TestObject> map({ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } });

			// don't expect std library construction. Annoyingly, the
			// performance is pretty different between different compilers
			TestObject::getconstructorcalls();
			TestObject::getdestructorcalls();
			TestObject::getdefaultconstructorcalls();
			TestObject::getmoveconstructorcalls();
			TestObject::getcopyconstructorcalls();
			TestObject::getmoveassignmentcalls();
			TestObject::getcopyassignmentcalls();
			TestObject::gettestfunctioncalls();
			TestObject::getstringfunctioncalls();

			// extra scope to support memory leak detection
			{
				TestCache test(map.begin(), map.end());

				EXPECT_EQ(4, test.size());
				EXPECT_STREQ("d", test[3].getString().c_str());
				EXPECT_STREQ("d", map[3].getString().c_str());
			}

			EXPECT_STREQ("d", map[3].getString().c_str());
			EXPECT_STREQ("c", map[2].getString().c_str());
			EXPECT_STREQ("b", map[1].getString().c_str());
			EXPECT_STREQ("a", map[0].getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(6, TestObject::getstringfunctioncalls());

			// map size > n
			{
				auto first = map.begin();
				auto second = (++map.begin());

				TestCache test(map.begin(), map.end(), 2);

				EXPECT_EQ(2, test.size());

				// order should be preserved IFF the iterator is bidirectional
				if(std::is_same<typename std::iterator_traits<decltype(first)>::iterator_category, std::bidirectional_iterator_tag>::value)
				{
					EXPECT_STREQ(first->second.getString().c_str(), std::next(test.begin(), 0)->getString().c_str());
					EXPECT_STREQ(second->second.getString().c_str(), std::next(test.begin(), 1)->getString().c_str());
					EXPECT_EQ(4, TestObject::getstringfunctioncalls());
				}
			}

			EXPECT_STREQ("d", map[3].getString().c_str());
			EXPECT_STREQ("c", map[2].getString().c_str());
			EXPECT_STREQ("b", map[1].getString().c_str());
			EXPECT_STREQ("a", map[0].getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(2, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(4, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());

	}

	TEST_F(cacheTest, rangeConstructorValue)
	{
		TestObject::clearfunctioncalls();
		{
			// vector size < n
			std::vector<TestObject> vec({ 'a', 'b', 'c', 'd' });

			// don't expect std library construction. Annoyingly, the
			// performance is pretty different between different compilers
			TestObject::getconstructorcalls();
			TestObject::getdestructorcalls();
			TestObject::getdefaultconstructorcalls();
			TestObject::getmoveconstructorcalls();
			TestObject::getcopyconstructorcalls();
			TestObject::getmoveassignmentcalls();
			TestObject::getcopyassignmentcalls();
			TestObject::gettestfunctioncalls();
			TestObject::getstringfunctioncalls();

			// extra scope to support memory leak detection
			{
				TestCache test(vec.begin(), vec.end());

				EXPECT_EQ(4, test.size());
				for (int i = 0; i < test.size(); ++i)
				{
					EXPECT_STREQ(vec[i].getString().c_str(), (std::next(test.begin(),i))->getString().c_str());
				}
			}

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(8, TestObject::getstringfunctioncalls());

			// vector size > n
			{
				TestCache test(vec.begin(), vec.end(), 2);

				EXPECT_EQ(2, test.size());
				for (int i = 0; i < test.size(); ++i)
				{
					EXPECT_STREQ(vec[i].getString().c_str(), (std::next(test.begin(), i))->getString().c_str());
				}
			}

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(2, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(4, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, copyConstructor)
	{
		TestObject::clearfunctioncalls();
		EXPECT_TRUE(std::is_copy_constructible<TestCache>::value);

		{
			// vector size < n
			std::vector<TestObject> vec({ 'a', 'b', 'c', 'd' });

			// don't expect std library construction. Annoyingly, the
			// performace is pretty different between different compilers
			TestObject::getconstructorcalls();
			TestObject::getdestructorcalls();
			TestObject::getdefaultconstructorcalls();
			TestObject::getmoveconstructorcalls();
			TestObject::getcopyconstructorcalls();
			TestObject::getmoveassignmentcalls();
			TestObject::getcopyassignmentcalls();
			TestObject::gettestfunctioncalls();
			TestObject::getstringfunctioncalls();

			TestCache cache(vec.begin(), vec.end());

			EXPECT_STREQ("c", cache[2].getString().c_str());

			// don't really care how many this block takes, we're not making
			// cache in the way that will be most efficient once the whole interface
			// is implemented
			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// extra scope to support memory leak detection
			{
				TestCache test(cache);

				EXPECT_EQ(4, test.size());
				EXPECT_STREQ("c", test[2].getString().c_str());
			}

			// Anything but n copies here is bad.
			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());

	}

	TEST_F(cacheTest, moveConstructor)
	{
		TestObject::clearfunctioncalls();
		EXPECT_TRUE(std::is_move_constructible<TestCache>::value);

		{
			// vector size < n
			std::vector<TestObject> vec({ 'a', 'b', 'c', 'd' });

			// don't expect std library construction. Annoyingly, the
			// performace is pretty different between different compilers
			TestObject::getconstructorcalls();
			TestObject::getdestructorcalls();
			TestObject::getdefaultconstructorcalls();
			TestObject::getmoveconstructorcalls();
			TestObject::getcopyconstructorcalls();
			TestObject::getmoveassignmentcalls();
			TestObject::getcopyassignmentcalls();
			TestObject::gettestfunctioncalls();
			TestObject::getstringfunctioncalls();

			TestCache cache(vec.begin(), vec.end());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			// extra scope to support memory leak detection
			{
				TestCache test(std::move(cache));

				EXPECT_EQ(4, test.size());
				EXPECT_STREQ("c", test[2].getString().c_str());
			}

			// 0 move constructors on TestObject, because the internal container is being moved
			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, initializerListConstructor)
	{
		TestObject::clearfunctioncalls();
		// extra scope to support memory leak detection
		{
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' }, { 4, 'e' } };
			EXPECT_EQ(5, test.size());
			char ch = 'a';
			for (int i = 0; i < test.size(); ++i)
			{
				EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
			}
		}

		// 0 move constructors on TestObject, because the internal container is being moved
		EXPECT_EQ(5, TestObject::getconstructorcalls());
		EXPECT_EQ(10, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(5, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(5, TestObject::getstringfunctioncalls());

		// n < initializer list size
		{
			TestCache test({ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' }, { 4, 'e' } }, 2);

			EXPECT_EQ(2, test.size());
			char ch = 'a';
			for (int i = 0; i < test.size(); ++i)
			{
				EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
			}
		}

		// 0 move constructors on TestObject, because the internal container is being moved
		EXPECT_EQ(5, TestObject::getconstructorcalls());
		EXPECT_EQ(7, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(2, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(2, TestObject::getstringfunctioncalls());

		{
			EXPECT_THROW((TestCache{ { 0 COMMA 'a' } COMMA{ 1 COMMA 'b' } COMMA{ 2 COMMA TestObject::CopyConstructor } }), std::bad_alloc);

			EXPECT_EQ(3, TestObject::getconstructorcalls());
			EXPECT_EQ(3, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(0, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, destructor)
	{
		TestObject::clearfunctioncalls();
		EXPECT_TRUE(std::is_destructible<TestObject>::value);
	}

	TEST_F(cacheTest, copyAssignment)
	{
		TestObject::clearfunctioncalls();
		EXPECT_TRUE(std::is_copy_assignable<TestCache>::value);

		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			{
				TestCache test = cache;

				EXPECT_EQ(4, test.size());
				EXPECT_STREQ("c", test[2].getString().c_str());
				EXPECT_STREQ("c", cache[2].getString().c_str());

				EXPECT_EQ(0, TestObject::getconstructorcalls());
				EXPECT_EQ(0, TestObject::getdestructorcalls());
				EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
				EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
				EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
				EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
				EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
				EXPECT_EQ(0, TestObject::gettestfunctioncalls());
				EXPECT_EQ(2, TestObject::getstringfunctioncalls());
			}

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, moveAssignment)
	{
		TestObject::clearfunctioncalls();
		EXPECT_TRUE(std::is_move_assignable<TestCache>::value);

		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			{
				TestCache test = std::move(cache);

				EXPECT_EQ(4, test.size());
				EXPECT_STREQ("c", test[2].getString().c_str());

				EXPECT_EQ(0, TestObject::getconstructorcalls());
				EXPECT_EQ(0, TestObject::getdestructorcalls());
				EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
				EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
				EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
				EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
				EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
				EXPECT_EQ(0, TestObject::gettestfunctioncalls());
				EXPECT_EQ(1, TestObject::getstringfunctioncalls());
			}

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(0, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, capacity)
	{
		TestObject::clearfunctioncalls();
		TestCache test1;
		TestCache test2(35);
		TestCache test3(52387);

		EXPECT_EQ(10, test1.capacity());
		EXPECT_EQ(35, test2.capacity());
		EXPECT_EQ(52387, test3.capacity());

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(0, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, empty)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache empty;
			TestCache notEmpty{ { 1, 'a' } };

			EXPECT_TRUE(empty.empty());
			EXPECT_FALSE(notEmpty.empty());
		}

		EXPECT_EQ(1, TestObject::getconstructorcalls());
		EXPECT_EQ(2, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, size)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache empty;
			TestCache notEmpty{ { 1, 'a' } };

			EXPECT_EQ(0, empty.size());
			EXPECT_EQ(1, notEmpty.size());
		}

		EXPECT_EQ(1, TestObject::getconstructorcalls());
		EXPECT_EQ(2, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, reserve)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache c{ { 1, 'a' } };

			EXPECT_EQ(1, c.size());
			EXPECT_EQ(1, c.capacity());

			// nothing happens
			c.reserve(0);

			EXPECT_EQ(1, c.size());
			EXPECT_EQ(1, c.capacity());

			c.reserve(3);

			EXPECT_EQ(1, c.size());
			EXPECT_EQ(3, c.capacity());
		}

		EXPECT_EQ(1, TestObject::getconstructorcalls());
		EXPECT_EQ(2, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, max_size)
	{
		TestObject::clearfunctioncalls();
		{
			// As far as I can tell, this is un-important and untestable.
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(0, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, begin)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 1, 'a' } };
			EXPECT_EQ(1, test.size());

			TestCache::iterator itr = test.begin();
			EXPECT_STREQ("a", (*itr).getString().c_str());

			TestCache::const_iterator citr = test.begin();
			EXPECT_STREQ("a", (*citr).getString().c_str());

			// test mutability
			typedef TestCache::iterator it;
			typedef std::iterator_traits<it>::pointer ptr;
			typedef std::remove_pointer<ptr>::type iterator_type;
			EXPECT_FALSE(std::is_const<iterator_type>::value);
		}

		EXPECT_EQ(1, TestObject::getconstructorcalls());
		EXPECT_EQ(2, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(2, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, cbegin)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 1, 'a' }, { 2, 'b' } };
			EXPECT_EQ(2, test.size());
			EXPECT_STREQ("a", (*test.cbegin()).getString().c_str());

			// test mutability
			typedef TestCache::const_iterator c_it;
			typedef std::iterator_traits<c_it>::pointer c_ptr;
			typedef std::remove_pointer<c_ptr>::type c_iterator_type;
			EXPECT_TRUE(std::is_const<c_iterator_type>::value);
		}

		EXPECT_EQ(2, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(2, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(1, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, rbegin)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 1, 'a' }, { 2, 'b' } };
			EXPECT_EQ(2, test.size());

			TestCache::reverse_iterator itr = test.rbegin();
			EXPECT_STREQ("b", (*itr).getString().c_str());

			TestCache::const_reverse_iterator citr = test.rbegin();
			EXPECT_STREQ("b", (*citr).getString().c_str());

			// test mutability
			typedef TestCache::reverse_iterator r_it;
			typedef std::iterator_traits<r_it>::pointer r_ptr;
			typedef std::remove_pointer<r_ptr>::type reverse_iterator_type;
			EXPECT_FALSE(std::is_const<reverse_iterator_type>::value);
		}

		EXPECT_EQ(2, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(2, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(2, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, crbegin)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 1, 'a' }, { 2, 'b' } };
			EXPECT_EQ(2, test.size());
			EXPECT_STREQ("b", (*test.crbegin()).getString().c_str());

			// test mutability
			typedef TestCache::const_iterator c_it;
			typedef std::iterator_traits<c_it>::pointer c_ptr;
			typedef std::remove_pointer<c_ptr>::type c_iterator_type;
			EXPECT_TRUE(std::is_const<c_iterator_type>::value);
		}

		EXPECT_EQ(2, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(2, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(1, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, end)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			std::string output[4] = { "a", "b", "c", "d" };

			EXPECT_EQ(4, cache.size());

			int i = 0;
			for (TestCache::iterator itr = cache.begin(); itr != cache.end(); itr++)
			{
				EXPECT_STREQ(output[i].c_str(), itr->getString().c_str());
				++i;
			}

			i = 0;
			for (TestCache::const_iterator itr = cache.begin(); itr != cache.end(); itr++)
			{
				EXPECT_STREQ(output[i].c_str(), itr->getString().c_str());
				++i;
			}
		}

		EXPECT_EQ(4, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(8, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, cend)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			std::string output[4] = { "a", "b", "c", "d" };

			EXPECT_EQ(4, cache.size());

			int i = 0;
			for (auto itr = cache.cbegin(); itr != cache.cend(); itr++)
			{
				EXPECT_STREQ(output[i].c_str(), itr->getString().c_str());
				++i;
			}
		}

		EXPECT_EQ(4, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(4, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, rend)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			std::string output[4] = { "d", "c", "b", "a" };

			EXPECT_EQ(4, cache.size());

			int i = 0;
			for (TestCache::reverse_iterator itr = cache.rbegin(); itr != cache.rend(); itr++)
			{
				EXPECT_STREQ(output[i].c_str(), itr->getString().c_str());
				++i;
			}

			i = 0;
			for (TestCache::const_reverse_iterator itr = cache.rbegin(); itr != cache.rend(); itr++)
			{
				EXPECT_STREQ(output[i].c_str(), itr->getString().c_str());
				++i;
			}
		}

		EXPECT_EQ(4, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(8, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, crend)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache cache{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			std::string output[4] = { "d", "c", "b", "a" };

			EXPECT_EQ(4, cache.size());

			int i = 0;
			for (auto itr = cache.crbegin(); itr != cache.crend(); itr++)
			{
				EXPECT_STREQ(output[i].c_str(), itr->getString().c_str());
				++i;
			}
		}

		EXPECT_EQ(4, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(4, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, operatorBracket)
	{
		TestObject::clearfunctioncalls();
		// extra scope for memory leak detection
		{
			TestCache test(2);

			EXPECT_EQ(2, test.capacity());

			// insert w/ bracket
			test[0] = 'a';

			EXPECT_STREQ("a", test[0].getString().c_str());
			EXPECT_STREQ("a", test.begin()->getString().c_str());
			EXPECT_EQ(1, test.size());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(1, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(1, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			test[1] = 'b';

			EXPECT_STREQ("b", test[1].getString().c_str());
			EXPECT_STREQ("b", test.begin()->getString().c_str());
			EXPECT_EQ(2, test.size());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(1, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(1, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			test[2] = 'c';	// this pops 'a' because we exceed capacity

			EXPECT_STREQ("c", test[2].getString().c_str());
			EXPECT_STREQ("c", test.begin()->getString().c_str());
			EXPECT_EQ(2, test.size());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(1, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(1, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(2, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());

		// TEST CACHE ORDER USING []
		// extra scope for memory leak detection
		{
			int i = 0;
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			std::string output1[4] = { "a", "b", "c", "d" };
			EXPECT_EQ(4, test.capacity());

			i = 0;
			for (auto itr = test.begin(); itr != test.end(); ++itr)
			{
				EXPECT_EQ(output1[i], itr->getString().c_str());
				++i;
			}

			EXPECT_STREQ("c", test[2].getString().c_str());
			std::string output2[4] = { "c", "a", "b", "d" };

			i = 0;
			for (auto itr = test.begin(); itr != test.end(); ++itr)
			{
				EXPECT_EQ(output2[i], itr->getString().c_str());
				++i;
			}

			EXPECT_STREQ("a", test[0].getString().c_str());
			std::string output3[4] = { "a", "c", "b", "d" };

			i = 0;
			for (auto itr = test.begin(); itr != test.end(); ++itr)
			{
				EXPECT_EQ(output3[i], itr->getString().c_str());
				++i;
			}

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(14, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, at)
	{
		TestObject::clearfunctioncalls();
		// TEST CACHE ORDER USING at
		// extra scope for memory leak detection
		{
			int i = 0;
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			std::string output1[4] = { "a", "b", "c", "d" };
			EXPECT_EQ(4, test.capacity());

			i = 0;
			for (auto itr = test.begin(); itr != test.end(); ++itr)
			{
				EXPECT_EQ(output1[i], itr->getString().c_str());
				++i;
			}

			EXPECT_STREQ("c", test.at(2).getString().c_str());
			std::string output2[4] = { "c", "a", "b", "d" };

			i = 0;
			for (auto itr = test.begin(); itr != test.end(); ++itr)
			{
				EXPECT_EQ(output2[i], itr->getString().c_str());
				++i;
			}

			EXPECT_STREQ("a", test.at(0).getString().c_str());
			std::string output3[4] = { "a", "c", "b", "d" };

			i = 0;
			for (auto itr = test.begin(); itr != test.end(); ++itr)
			{
				EXPECT_EQ(output3[i], itr->getString().c_str());
				++i;
			}

			EXPECT_THROW(test.at(4), std::out_of_range);

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(14, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, peek)
	{
		TestObject::clearfunctioncalls();
		// TEST CACHE ORDER (UNCHANGED) USING peek
		// extra scope for memory leak detection
		{
			int i = 0;
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			std::string output[4] = { "a", "b", "c", "d" };
			EXPECT_EQ(4, test.capacity());

			i = 0;
			for (auto itr = test.begin(); itr != test.end(); ++itr)
			{
				EXPECT_EQ(output[i], itr->getString().c_str());
				++i;
			}

			EXPECT_STREQ("c", test.peek(2).getString().c_str());

			i = 0;
			for (auto itr = test.begin(); itr != test.end(); ++itr)
			{
				EXPECT_EQ(output[i], itr->getString().c_str());
				++i;
			}

			EXPECT_STREQ("a", test.peek(0).getString().c_str());

			i = 0;
			for (auto itr = test.begin(); itr != test.end(); ++itr)
			{
				EXPECT_EQ(output[i], itr->getString().c_str());
				++i;
			}

			EXPECT_THROW(test.peek(4), std::out_of_range);

			const TestCache ctest;
			EXPECT_THROW(ctest.peek(0), std::out_of_range);

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(14, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, front)
	{
		TestObject::clearfunctioncalls();
		{
			int i = 0;
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			EXPECT_EQ(4, test.capacity());

			EXPECT_STREQ("a", test.front().getString().c_str());

			EXPECT_STREQ("c", test[2].getString().c_str());
			EXPECT_STREQ("c", test.front().getString().c_str());

			EXPECT_STREQ("a", test[0].getString().c_str());
			EXPECT_STREQ("a", test.front().getString().c_str());

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(5, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, back)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			EXPECT_EQ(4, test.capacity());

			EXPECT_STREQ("d", test.back().getString().c_str());

			EXPECT_STREQ("d", test[3].getString().c_str());
			EXPECT_STREQ("c", test.back().getString().c_str());

			EXPECT_STREQ("b", test[1].getString().c_str());
			EXPECT_STREQ("c", test.back().getString().c_str());

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(5, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(4, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, find)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			EXPECT_EQ(4, test.capacity());

			// it's in the container
			EXPECT_STREQ("b", test.find(1)->getString().c_str());

			// it's not in the container
			EXPECT_EQ(test.end(), test.find(4));
		}
	}

	TEST_F(cacheTest, count)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };
			EXPECT_EQ(4, test.capacity());

			// it's in the container
			EXPECT_EQ(1, test.count(1));

			// it's not in the container
			EXPECT_EQ(0, test.count(4));
		}
	}

	TEST_F(cacheTest, emplace)
	{
		TestObject::clearfunctioncalls();
		bool keyConvertible = std::is_convertible<typename std::decay<decltype(0)>::type, TestCache::key_type>::value;
		bool valConvertible = std::is_convertible<typename std::decay<decltype('a')>::type, TestCache::mapped_type>::value;
		EXPECT_TRUE(keyConvertible);
		EXPECT_TRUE(valConvertible);

		using emplaceRetVal_t = ::std::pair<TestCache::iterator, bool>;
		emplaceRetVal_t ret;

		{
			TestCache c(5);

			// r-value key/val args
			ret = c.emplace(0, 'a');
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("a", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value key/val args already in
			ret = c.emplace(0, 'b');
			EXPECT_EQ(emplaceRetVal_t(c.begin(), false), ret);
			EXPECT_STREQ("a", c.begin()->getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// l-val key-val args
			size_t key = 1;
			char val('c');

			ret = c.emplace(key, val);
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("c", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// l-val key-val args already in
			val = 'd';

			ret = c.emplace(key, val);
			EXPECT_EQ(emplaceRetVal_t(c.begin(), false), ret);
			EXPECT_STREQ("c", c.begin()->getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value key/val pair
			ret = c.emplace(TestCache::value_type(2, 'e'));
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("e", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(1, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value key/val pair already in
			ret = c.emplace(TestCache::value_type(2, 'f'));
			EXPECT_EQ(emplaceRetVal_t(c.begin(), false), ret);
			EXPECT_STREQ("e", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// l-value key/val pair
			TestCache::value_type p(3, 'g');
			ret = c.emplace(p);
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("g", c.begin()->getString().c_str());

			// make sure we didn't break p
			EXPECT_STREQ("g", p.second.getString().c_str());
			EXPECT_EQ(3, p.first);

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// l-value key/val pair already in
			TestCache::value_type p2(3, 'h');
			ret = c.emplace(p2);
			EXPECT_EQ(emplaceRetVal_t(c.begin(), false), ret);
			EXPECT_STREQ("g", c.begin()->getString().c_str());

			// make sure we didn't break p
			EXPECT_STREQ("h", p2.second.getString().c_str());
			EXPECT_EQ(3, p2.first);

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// r-value piecewise
			ret = c.emplace(std::piecewise_construct, std::forward_as_tuple(4), std::forward_as_tuple('i'));
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("i", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value piecewise, already in
			ret = c.emplace(std::piecewise_construct, std::forward_as_tuple(4), std::forward_as_tuple('j'));
			EXPECT_EQ(emplaceRetVal_t(c.begin(), false), ret);
			EXPECT_STREQ("i", c.begin()->getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			EXPECT_EQ(5, c.size());

			// r-value key/val (pops oldest)
			ret = c.emplace(5, 'k');
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("k", c.front().getString().c_str());
			EXPECT_STREQ("c", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// l-val key-val args (pops oldest)
			key = 6;
			val ='l';

			ret = c.emplace(key, val);
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("l", c.front().getString().c_str());
			EXPECT_STREQ("e", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// r-value key/val pair (pops oldest)
			ret = c.emplace(TestCache::value_type(7, 'm'));
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("m", c.front().getString().c_str());
			EXPECT_STREQ("g", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(1, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// l-value key/val pair (pops oldest)
			TestCache::value_type q(8, 'n');
			ret = c.emplace(q);
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("n", c.front().getString().c_str());
			EXPECT_STREQ("i", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// r-value piecewise (pops oldest)
			ret = c.emplace(std::piecewise_construct, std::forward_as_tuple(9), std::forward_as_tuple('o'));
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("o", c.front().getString().c_str());
			EXPECT_STREQ("k", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());

		// EXCEPTION SAFETY
		char ch;
		TestCache test(4);
		test.insert({ { 0, 'a' }, { 1, 'b' }, { 2, 'c' } });

		EXPECT_EQ(3, TestObject::getconstructorcalls());
		EXPECT_EQ(3, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(3, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		// r-value
		EXPECT_THROW(test.emplace(3 COMMA TestObject::Constructor), std::bad_alloc);

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		// l-value
		size_t failt = 3;
		TestObject fail('f', TestObject::CopyConstructor | TestObject::CopyAssignment);
		EXPECT_THROW(test.emplace(failt, fail), std::bad_alloc);

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		// r-value pair
		EXPECT_THROW(test.emplace(TestCache::value_type(5, TestObject::Constructor)), std::bad_alloc);

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		// l-value pair
		TestCache::value_type copyFail(8, TestObject::CopyConstructor);
		EXPECT_THROW(test.emplace(copyFail), std::bad_alloc);

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		// r-value piecewise
		EXPECT_THROW(test.emplace(std::piecewise_construct, std::forward_as_tuple(5), std::forward_as_tuple(TestObject::Constructor)), std::bad_alloc);

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		TestObject::clearfunctioncalls();
	}

	TEST_F(cacheTest, emplaceHint)
	{
		TestObject::clearfunctioncalls();
		bool keyConvertible = std::is_convertible<typename std::decay<decltype(0)>::type, TestCache::key_type>::value;
		bool valConvertible = std::is_convertible<typename std::decay<decltype('a')>::type, TestCache::mapped_type>::value;
		EXPECT_TRUE(keyConvertible);
		EXPECT_TRUE(valConvertible);

		using emplaceRetVal_t = ::std::pair<TestCache::iterator, bool>;
		emplaceRetVal_t ret;

		{
			TestCache c(5);

			// r-value key/val args
			ret = c.emplace_hint(c.end(),0, 'a');
			EXPECT_EQ(emplaceRetVal_t(--c.end(), true), ret);
			EXPECT_STREQ("a", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value key/val args already in
			ret = c.emplace_hint(c.end(),0, 'b');
			EXPECT_EQ(emplaceRetVal_t(--c.end(), false), ret);
			EXPECT_STREQ("a", c.begin()->getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// l-val key-val args
			size_t key = 1;
			char val('c');

			ret = c.emplace_hint(c.begin(),key, val);	// begin on purpose
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("c", c.front().getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// l-val key-val args already in
			val = 'd';

			ret = c.emplace_hint(c.end(),key, val);
			EXPECT_EQ(emplaceRetVal_t(c.begin(), false), ret);
			EXPECT_STREQ("a", c.back().getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value key/val pair
			ret = c.emplace_hint(c.end(), TestCache::value_type(2, 'e'));
			EXPECT_EQ(emplaceRetVal_t(--c.end(), true), ret);
			EXPECT_STREQ("e", c.back().getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(1, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value key/val pair already in
			ret = c.emplace_hint(c.end(), TestCache::value_type(2, 'f'));
			EXPECT_EQ(emplaceRetVal_t(--c.end(), false), ret);
			EXPECT_STREQ("e", c.back().getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// l-value key/val pair
			TestCache::value_type p(3, 'g');
			ret = c.emplace_hint(c.end(),p);
			EXPECT_EQ(emplaceRetVal_t(--c.end(), true), ret);
			EXPECT_STREQ("g", c.back().getString().c_str());

			// make sure we didn't break p
			EXPECT_STREQ("g", p.second.getString().c_str());
			EXPECT_EQ(3, p.first);

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// l-value key/val pair already in
			TestCache::value_type p2(3, 'h');
			ret = c.emplace_hint(c.end(),p2);
			EXPECT_EQ(emplaceRetVal_t(--c.end(), false), ret);
			EXPECT_STREQ("g", c.back().getString().c_str());

			// make sure we didn't break p
			EXPECT_STREQ("h", p2.second.getString().c_str());
			EXPECT_EQ(3, p2.first);

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// r-value piecewise
			ret = c.emplace_hint(c.end(),std::piecewise_construct, std::forward_as_tuple(4), std::forward_as_tuple('i'));
			EXPECT_EQ(emplaceRetVal_t(--c.end(), true), ret);
			EXPECT_STREQ("i", c.back().getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value piecewise, already in
			ret = c.emplace_hint(c.end(),std::piecewise_construct, std::forward_as_tuple(4), std::forward_as_tuple('j'));
			EXPECT_EQ(emplaceRetVal_t(--c.end(), false), ret);
			EXPECT_STREQ("i", c.back().getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			EXPECT_EQ(5, c.size());

			// r-value key/val (pops oldest)
			ret = c.emplace_hint(--c.end(),5, 'k');
			EXPECT_EQ(emplaceRetVal_t(--c.end(), true), ret);
			EXPECT_STREQ("c", c.front().getString().c_str());
			EXPECT_STREQ("k", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// l-val key-val args (pops oldest)
			key = 6;
			val = 'l';

			ret = c.emplace_hint(--c.end(),key, val);
			EXPECT_EQ(emplaceRetVal_t(--c.end(), true), ret);
			EXPECT_STREQ("c", c.front().getString().c_str());
			EXPECT_STREQ("l", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// r-value key/val pair (pops oldest)
			ret = c.emplace_hint(--c.end(), TestCache::value_type(7, 'm'));
			EXPECT_EQ(emplaceRetVal_t(--c.end(), true), ret);
			EXPECT_STREQ("c", c.front().getString().c_str());
			EXPECT_STREQ("m", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(1, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// l-value key/val pair (pops oldest)
			TestCache::value_type q(8, 'n');
			ret = c.emplace_hint(--c.end(), q);
			EXPECT_EQ(emplaceRetVal_t(--c.end(), true), ret);
			EXPECT_STREQ("c", c.front().getString().c_str());
			EXPECT_STREQ("n", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// r-value piecewise (pops oldest)
			ret = c.emplace_hint(--c.end(),std::piecewise_construct, std::forward_as_tuple(9), std::forward_as_tuple('o'));
			EXPECT_EQ(emplaceRetVal_t(--c.end(), true), ret);
			EXPECT_STREQ("c", c.front().getString().c_str());
			EXPECT_STREQ("o", c.back().getString().c_str());
			EXPECT_EQ(5, c.size());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(8, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, insert)
	{
		TestObject::clearfunctioncalls();
		using emplaceRetVal_t = ::std::pair<TestCache::iterator, bool>;
		using cacheEntry = TestCache::value_type;
		emplaceRetVal_t ret;

		{
			TestCache c;

			// const l-value insert
			TestCache::value_type a(0, 'a');
			ret = c.insert(a);
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("a", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// const l-value insert, already in
			TestCache::value_type b(0, 'z');
			ret = c.insert(b);
			EXPECT_EQ(emplaceRetVal_t(c.begin(), false), ret);
			EXPECT_STREQ("a", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value insert
			ret = c.insert({ 1, 'b' });
			EXPECT_EQ(emplaceRetVal_t(c.begin(), true), ret);
			EXPECT_STREQ("b", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(1, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value insert, already-in
			ret = c.insert({ 1, 'y' });
			EXPECT_EQ(emplaceRetVal_t(c.begin(), false), ret);
			EXPECT_STREQ("b", c.begin()->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// const l-value insert @ position
			TestCache::value_type e(2, 'c');
			auto itr = c.insert(c.end(), e);
			EXPECT_EQ(--c.end(), itr);
			EXPECT_STREQ("c", (--c.end())->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// const l-value insert, already in
			TestCache::value_type f(2, 'x');
			itr = c.insert(c.end(), f);
			EXPECT_EQ(--c.end(), itr);
			EXPECT_STREQ("c", (--c.end())->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value insert
			itr = c.insert(c.end(), { 3, 'd' });
			EXPECT_EQ(--c.end(), itr);
			EXPECT_STREQ("d", (--c.end())->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(1, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// r-value insert, already-in
			itr = c.insert(c.end(), { 3, 'w' });
			EXPECT_EQ(--c.end(), itr);
			EXPECT_STREQ("d", (--c.end())->getString().c_str());

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(1, TestObject::getstringfunctioncalls());

			// make the order of c known
			c[3]; c[2]; c[1]; c[0];

			// copy range pair (first 4)
			TestCache c2;
			c2.insert(c.begin(), std::next(c.begin(), 4));

			EXPECT_EQ(4, c.size());
			EXPECT_EQ(4, c2.size());

			// order should be preserved
			for (int i = 0; i < c2.size(); ++i)
			{
				EXPECT_STREQ(std::next(c.begin(), i)->getString().c_str(), std::next(c2.begin(), i)->getString().c_str());
			}

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(8, TestObject::getstringfunctioncalls());

			// copy range value (first 4)
			std::vector<char> vec{ 'e', 'f', 'g', 'h' };

			c2.insert(vec.begin(), vec.end());

			EXPECT_EQ(8, c2.size());

			// order should be preserved
			char ch = 'a';
			for (int i = 0; i < c2.size(); ++i)
			{
				EXPECT_EQ((char)(ch + i), (char)c2[i].getString().c_str()[0]);
			}

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(8, TestObject::getstringfunctioncalls());

			// insert initializer list
			c2.insert({{ 8, 'i' }, { 9, 'j' }});

			EXPECT_EQ(10, c2.size());
			ch = 'a';
			for (int i = 0; i < c2.size(); ++i)
			{
				EXPECT_EQ((char)(ch + i), (char)c2[i].getString().c_str()[0]);
			}

			EXPECT_EQ(2, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(2, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(10, TestObject::getstringfunctioncalls());

			// INSERT > MAX SIZE TESTS

			std::vector<TestObject> output(c2.begin(), c2.end());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(10, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			// l-value (pops oldest)
			TestCache::value_type k{ 10, 'k' };
			c2.insert(k);

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			// r-value (pops oldest)
			c2.insert({ 11, 'l' });

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(1, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			// l-value @ pos (pops oldest)
			TestCache::value_type m{ 12, 'm' };
			c2.insert(c2.end(), m);

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(1, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			// r-value @ pos (pops oldest)
			c2.insert(c2.end(), { 13, 'n' });

			EXPECT_EQ(1, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(1, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			// range (pops oldest)
			std::vector<TestObject> vec2{ 'o', 'p' };
			c2.insert(vec2.begin(), vec2.end());

			EXPECT_EQ(2, TestObject::getconstructorcalls());
			EXPECT_EQ(TestObject::getcopyconstructorcalls(), TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			// initializer (pops oldest)
			c2.insert({ {16, 'q'}, {17, 'r'} });

			EXPECT_EQ(2, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(2, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(34, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());

		// EXCEPTION SAFETY
		char ch;
		TestCache test(4);
		test.insert({ { 0, 'a' }, { 1, 'b' }, { 2, 'c' } });

		EXPECT_EQ(3, TestObject::getconstructorcalls());
		EXPECT_EQ(3, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(3, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		// r-value
		EXPECT_THROW((test.insert(TestCache::value_type(3, TestObject::Constructor))), std::bad_alloc);

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		// l-value
		TestCache::value_type copyFail(3, TestObject::CopyConstructor);
		EXPECT_THROW(test.insert(copyFail), std::bad_alloc);

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		// initializer
		EXPECT_THROW(test.insert({ 3, TestObject::Constructor }), std::bad_alloc);

		EXPECT_EQ(3, test.size());
		ch = 'a';
		for (int i = 0; i < test.size(); ++i)
		{
			EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
		}

		TestObject::clearfunctioncalls();
	}

	TEST_F(cacheTest, erase)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };

			EXPECT_EQ(4, test.size());

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			// erase by iterator
			auto ret = test.erase(test.find(2));

			EXPECT_STREQ("d", ret->getString().c_str());
			EXPECT_EQ(3, test.size());

			EXPECT_STREQ("a", (std::next(test.begin(), 0))->getString().c_str());
			EXPECT_STREQ("b", (std::next(test.begin(), 1))->getString().c_str());
			EXPECT_STREQ("d", (std::next(test.begin(), 2))->getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(4, TestObject::getstringfunctioncalls());

			// erase by key
			size_t num = test.erase(0);

			EXPECT_EQ(1, num);
			EXPECT_EQ(2, test.size());

			EXPECT_STREQ("b", (std::next(test.begin(), 0))->getString().c_str());
			EXPECT_STREQ("d", (std::next(test.begin(), 1))->getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(1, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// erase by key, not in container
			num = test.erase(0);

			EXPECT_EQ(0, num);
			EXPECT_EQ(2, test.size());

			EXPECT_STREQ("b", (std::next(test.begin(), 0))->getString().c_str());
			EXPECT_STREQ("d", (std::next(test.begin(), 1))->getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(2, TestObject::getstringfunctioncalls());

			// erase by range
			ret = test.erase(test.begin(), test.end());

			EXPECT_EQ(0, test.size());
			EXPECT_EQ(ret, test.end());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(0, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, shrink)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };

			EXPECT_EQ(4, test.size());

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			// does nothing
			test.shrink(5);

			EXPECT_EQ(4, test.size());
			EXPECT_EQ(4, test.capacity());

			char ch = 'a';
			for (int i = 0; i < test.size(); ++i)
			{
				EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
			}

			test.shrink(2);

			EXPECT_EQ(2, test.size());
			EXPECT_EQ(2, test.capacity());

			ch = 'a';
			for (int i = 0; i < test.size(); ++i)
			{
				EXPECT_EQ(char(ch + i), (char)(std::next(test.begin(), i))->getString().c_str()[0]);
			}

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(2, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(6, TestObject::getstringfunctioncalls());

		}
	}

	TEST_F(cacheTest, clear)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache test{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }, { 3, 'd' } };

			EXPECT_EQ(4, test.size());

			EXPECT_EQ(4, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(4, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			test.clear();

			EXPECT_EQ(0, test.size());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(4, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());
		}
	}

	TEST_F(cacheTest, swap)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache c1{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' }};
			TestCache c2{ { 3, 'd' }, { 4, 'e' }, { 5, 'f' } };

			EXPECT_EQ(3, c1.size());
			EXPECT_EQ(3, c2.size());

			EXPECT_EQ(6, TestObject::getconstructorcalls());
			EXPECT_EQ(6, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(6, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(0, TestObject::getstringfunctioncalls());

			c1.swap(c2);

			EXPECT_EQ(3, c1.size());
			EXPECT_EQ(3, c2.size());

			EXPECT_STREQ("d", c1[3].getString().c_str());
			EXPECT_STREQ("e", c1[4].getString().c_str());
			EXPECT_STREQ("f", c1[5].getString().c_str());
			EXPECT_STREQ("a", c2[0].getString().c_str());
			EXPECT_STREQ("b", c2[1].getString().c_str());
			EXPECT_STREQ("c", c2[2].getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(6, TestObject::getstringfunctioncalls());

			swap(c1, c2);

			EXPECT_EQ(3, c1.size());
			EXPECT_EQ(3, c2.size());

			EXPECT_STREQ("a", c1[0].getString().c_str());
			EXPECT_STREQ("b", c1[1].getString().c_str());
			EXPECT_STREQ("c", c1[2].getString().c_str());
			EXPECT_STREQ("d", c2[3].getString().c_str());
			EXPECT_STREQ("e", c2[4].getString().c_str());
			EXPECT_STREQ("f", c2[5].getString().c_str());

			EXPECT_EQ(0, TestObject::getconstructorcalls());
			EXPECT_EQ(0, TestObject::getdestructorcalls());
			EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
			EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
			EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
			EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
			EXPECT_EQ(0, TestObject::gettestfunctioncalls());
			EXPECT_EQ(6, TestObject::getstringfunctioncalls());
		}

		EXPECT_EQ(0, TestObject::getconstructorcalls());
		EXPECT_EQ(6, TestObject::getdestructorcalls());
		EXPECT_EQ(0, TestObject::getdefaultconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveconstructorcalls());
		EXPECT_EQ(0, TestObject::getcopyconstructorcalls());
		EXPECT_EQ(0, TestObject::getmoveassignmentcalls());
		EXPECT_EQ(0, TestObject::getcopyassignmentcalls());
		EXPECT_EQ(0, TestObject::gettestfunctioncalls());
		EXPECT_EQ(0, TestObject::getstringfunctioncalls());
	}

	TEST_F(cacheTest, observers)
	{
		TestObject::clearfunctioncalls();
		{
			TestCache c;

			// just test that these don't crash
			c.hash_function();
			c.key_eq();
			c.get_allocator();
		}
	}

	TEST_F(cacheTest, relationalOperators)
	{
		TestObject::clearfunctioncalls();
		{
			char ch;
			TestCache c1{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' } };
			TestCache c2{ { 0, 'a' }, { 1, 'b' }, { 2, 'c' } };
			TestCache c3{ { 3, 'd' }, { 4, 'e' }, { 5, 'f' } };

			EXPECT_EQ(c1, c2);
			EXPECT_NE(c2, c3);

			// verify cache order was not affected
			ch = 'a';
			for (int i = 0; i < c1.size(); ++i)
			{
				EXPECT_EQ(char(ch + i), (char)(std::next(c1.begin(), i))->getString().c_str()[0]);
			}

			ch = 'a';
			for (int i = 0; i < c2.size(); ++i)
			{
				EXPECT_EQ(char(ch + i), (char)(std::next(c2.begin(), i))->getString().c_str()[0]);
			}

			ch = 'd';
			for (int i = 0; i < c3.size(); ++i)
			{
				EXPECT_EQ(char(ch + i), (char)(std::next(c3.begin(), i))->getString().c_str()[0]);
			}
		}
		TestObject::clearfunctioncalls();
	}
}  // namespace
#endif //COORDINATES_CACHETEST_H