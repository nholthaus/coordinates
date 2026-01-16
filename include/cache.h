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

#ifndef cache_h__
#define cache_h__

//------------------------
//	INCLUDES
//------------------------

#include <algorithm>
#include <list>
#include <numeric>
#include <tuple>
#include <unordered_map>
#include <utility>

//	----------------------------------------------------------------------------
//	CLASS		Cache
//  ----------------------------------------------------------------------------
///	@brief		Least recently used cache.
///	@details	This container can be thought of as a hash map in which the
///				least recently accessed element will be removed if the total number
///				of elements surpasses a user-defined threshold.\n\n
///
///				For most purposes, this class is VERY similar to std::unordered_map. See the documentation
///				for that class if any of the member function documentation is unclear.\n\n
///
///				<b>Container Properties</b>\n\n
///
///				<b>Associative</b>: Elements in associative containers are referenced by their key and not by their absolute position in the container.\n
///				<b>Unordered</b>: Unordered containers organize their elements using hash tables that allow for fast access to elements by their key.\n
///				<b>Map</b>: Each element associates a key to a mapped value : Keys are meant to identify the elements whose main content is the mapped value.\n
///				<b>Unique keys</b>: No two elements in the container can have equivalent keys.\n
///				<b>Allocator-aware</b>: The container uses an allocator object to dynamically handle its storage needs.\n
///				<b>Size-constrained</b>: The container is extensible up to a certain maximum size, after which it will not grow.\n
///
/// @tparam		Key		Type of the key values. Each element in a Cache is uniquely identified by its
///						key value. Aliased as member type Cache::key_type.
/// @tparam		T		Type of the mapped value. Each element in an Cache is used to store some data as
///						its mapped value. Aliased as member type Cache::mapped_type.Note that this is
///						not the same as Cache::value_type(see below).
/// @tparam		Hash	A unary function object type that takes an object of type key type as
///						argument and returns a unique value of type size_t based on it. This can
///						either be a class implementing a function call operator or a pointer to a
///						function (see constructor for an example). This defaults to hash<Key>, which
///						returns a hash value with a probability of collision approaching
///						1.0/std::numeric_limits<size_t>::max(). The Cache object uses the hash values
///						returned by this function to organize its elements internally, speeding up
///						the process of locating individual elements. Aliased as member type Cache::hasher.
/// @param		Pred	A binary predicate that takes two arguments of the key type and returns a bool.
///						The expression pred(a,b), where pred is an object of this type and a and b
///						are key values, shall return true if a is to be considered equivalent to b.
///						This can either be a class implementing a function call operator or a pointer
///						to a function (see constructor for an example). This defaults to equal_to<Key>,
///						which returns the same as applying the equal-to operator (a==b). The Cache
///						object uses this expression to determine whether two element keys are equivalent.
///						No two elements in an Cache container can have keys that yield true using this
///						predicate. Aliased as member type Cache::key_equal.
/// @tparam		Alloc	Type of the allocator object used to define the storage allocation model.
///						By default, the allocator class template is used, which defines the simplest
///						memory allocation model and is value-independent. Aliased as member type
///						Cache::allocator_type.
//  ----------------------------------------------------------------------------
template<class Key, class T, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>, class Alloc = std::allocator<std::pair<const Key, T>>>
class Cache
{
public:
	class iterator;
	class const_iterator;

	//----------------------------------
	//	TYPEDEFS
	//----------------------------------

	typedef Key                                    key_type;
	typedef T                                      mapped_type;
	typedef Hash                                   hasher;
	typedef Pred                                   key_equal;
	typedef Alloc                                  allocator_type;
	typedef std::pair<const key_type, mapped_type> value_type;

	// NOTE:
	// `allocator_type` is provided as a "base" allocator. Internal containers require allocator
	// rebinding to their own value_type(s) (C++11+ allocator model).
	typedef std::allocator_traits<allocator_type>             allocator_traits;
	typedef allocator_traits::template rebind_alloc<key_type> list_allocator_type;

	typedef mapped_type&       reference;
	typedef const mapped_type& const_reference;
	typedef mapped_type*       pointer;
	typedef const mapped_type* const_pointer;

	typedef size_t         size_type;
	typedef std::ptrdiff_t difference_type;

	typedef std::list<key_type, list_allocator_type> list_type;

	typedef list_type::iterator                                                                   list_iterator_type;
	typedef list_type::const_iterator                                                             const_list_iterator_type;
	typedef std::pair<mapped_type, list_iterator_type>                                            cache_entry_type;
	typedef std::pair<const key_type, cache_entry_type>                                           map_value_type;
	typedef allocator_traits::template rebind_alloc<map_value_type>                               map_allocator_type;
	typedef std::unordered_map<key_type, cache_entry_type, hasher, key_equal, map_allocator_type> container_type;

	typedef std::reverse_iterator<iterator>       reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	static constexpr size_t DEFAULT_SIZE = 10;

public:
	//	----------------------------------------------------------------------------
	//	CLASS		const_iterator
	//  ----------------------------------------------------------------------------
	///	@brief		Cache immutable iterator type
	///	@details
	//  ----------------------------------------------------------------------------
	class const_iterator
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef const mapped_type               value_type;
		typedef const mapped_type*              pointer;
		typedef const mapped_type&              reference;
		typedef std::ptrdiff_t                  difference_type;

	public:
		const_iterator()
		    : m_cache(nullptr)
		    , m_listItr(const_list_iterator_type()) {};

		const_iterator(const Cache* cache, const_list_iterator_type listItr)
		    : m_cache(cache)
		    , m_listItr(listItr) {};
		const_iterator(const const_iterator&)            = default;
		const_iterator& operator=(const const_iterator&) = default;

		~const_iterator() {};

		const_iterator& operator++()
		{
			++m_listItr;
			return *this;
		}

		const_iterator operator++(int)
		{
			const_iterator temp = *this;
			++*this;
			return temp;
		}

		const_iterator& operator--()
		{
			--m_listItr;
			return *this;
		}

		const_iterator operator--(int)
		{
			const_iterator temp = *this;
			--*this;
			return temp;
		}

		friend bool operator==(const const_iterator& lhs, const const_iterator& rhs)
		{ return ((lhs.m_listItr == rhs.m_listItr) && (lhs.m_cache == rhs.m_cache)); }

		friend bool operator!=(const const_iterator& lhs, const const_iterator& rhs) { return !(lhs == rhs); }

		const mapped_type& operator*() const { return m_cache->peek(*m_listItr); }

		const mapped_type* operator->() const { return &m_cache->peek(*m_listItr); }

	private:
		const Cache*             m_cache;
		const_list_iterator_type m_listItr;

		friend class Cache;
		friend class iterator;
	};

	//	----------------------------------------------------------------------------
	//	CLASS		iterator
	//  ----------------------------------------------------------------------------
	///	@brief		Cache mutable iterator type
	///	@details
	//  ----------------------------------------------------------------------------
	class iterator
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef mapped_type                     value_type;
		typedef mapped_type*                    pointer;
		typedef mapped_type&                    reference;
		typedef std::ptrdiff_t                  difference_type;

	public:
		iterator()
		    : m_cache(nullptr)
		    , m_listItr(list_iterator_type()) {};

		iterator(Cache* cache, list_iterator_type listItr)
		    : m_cache(cache)
		    , m_listItr(listItr) {};
		iterator(const iterator&)            = default;
		iterator& operator=(const iterator&) = default;

		~iterator() {};

		iterator& operator++()
		{
			++m_listItr;
			return *this;
		}

		iterator operator++(int)
		{
			iterator temp = *this;
			++*this;
			return temp;
		}

		iterator& operator--()
		{
			--m_listItr;
			return *this;
		}

		iterator operator--(int)
		{
			iterator temp = *this;
			--*this;
			return temp;
		}

		friend bool operator==(const iterator& lhs, const iterator& rhs) { return ((lhs.m_listItr == rhs.m_listItr) && (lhs.m_cache == rhs.m_cache)); }

		friend bool operator!=(const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }

		mapped_type& operator*() const { return m_cache->peek(*m_listItr); }

		mapped_type* operator->() const { return &m_cache->peek(*m_listItr); }

		operator const_iterator() const { return const_iterator(m_cache, m_listItr); }

	private:
		Cache*             m_cache;
		list_iterator_type m_listItr;

		friend class Cache;
		friend class const_iterator;
	};

public:
	//----------------------------------
	//	CONSTRUCTORS
	//----------------------------------

	/**
	 * @brief		constructor
	 * @details		constructs the Cache. For the range and initializer constructors, if the number
	 *				of elements to copy is greater than `n`, than only the first n elements will be
	 *				copied. If the Cache is constructor from a set of iterators which do not include
	 *				a key, then the default keys for the copied objects will be [0...n).
	 *				<b> Complexity:</b> Constant (Default, Move). Linear (avg) or quadratic (worst) for copy, range, initialize.\n
	 *				<b> Iterator Validity:</b> N/A.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong Guarantee. The Cache will not be created an all contained objects will be destroyed.\n
	 * @param[in]	n		maximum number of elements. Also used as the initial number of hash table buckets.
	 * @param[in]	hf		Hasher function object. A hasher is a function that returns an integral value based on the container object key passed to it as
	 * argument.
	 * @param[in]	eql		Comparison function object, that returns true if the two container object keys passed as arguments are to be considered equal.
	 * @param[in]	alloc	Allocator object to be used instead of constructing a new one. For class instantiations using their version of the default allocator
	 * class template, this parameter is not relevant.
	 */
	explicit Cache(size_type n = DEFAULT_SIZE, const hasher& hf = hasher(), const key_equal& eql = key_equal(), const allocator_type& alloc = allocator_type())
	    : m_container(n + 1, hf, eql, map_allocator_type(alloc))
	    , m_list(list_allocator_type(alloc))
	    , m_maxSize(n)
	{
	}

	explicit Cache(const allocator_type& alloc)
	    : m_container(map_allocator_type(alloc))
	    , m_list(list_allocator_type(alloc))
	    , m_maxSize(DEFAULT_SIZE)
	{
	}

	/**
	 * @brief		Range constructor.
	 * @details		Creates a Cache using by copying the first `n` elements in the range [first, last).
	 *				Order is preserved, so 'first' will be the top of the cache, and 'last' will be
	 *				the bottom.
	 *				<b> Complexity:</b> Linear with `n`.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee. The Cache will not be created an all contained objects will be destroyed.\n
	 * @param[in]	first	Iterator to the first element to copy
	 * @param[in]	last	Iterator to the one-past-the-last element to copy
	 * @param[in]	n		maximum number of elements. Also used as the initial number of hash table buckets.
	 * @param[in]	hf		Hasher function object. A hasher is a function that returns an integral value based on the container object key passed to it as
	 * argument.
	 * @param[in]	eql		Comparison function object, that returns true if the two container object keys passed as arguments are to be considered equal.
	 * @param[in]	alloc	Allocator object to be used instead of constructing a new one. For class instantiations using their version of the default allocator
	 * class template, this parameter is not relevant.
	 */
	template<class InputIterator>
	Cache(InputIterator         first,
	      InputIterator         last,
	      size_type             n     = DEFAULT_SIZE,
	      const hasher&         hf    = hasher(),
	      const key_equal&      eql   = key_equal(),
	      const allocator_type& alloc = allocator_type())
	    : m_container(n + 1, hf, eql, map_allocator_type(alloc))
	    , m_list(list_allocator_type(alloc))
	    , m_maxSize(n)
	{
		copyRange(first,
		          last,
		          typename std::is_same<typename std::iterator_traits<InputIterator>::value_type, value_type>::type(),
		          typename std::integral_constant < bool,
		          std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::bidirectional_iterator_tag> ||
		                  std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::random_access_iterator_tag> > ::type());
	}

	/**
	 * @brief		Copy constructor.
	 * @details		Makes a copy of Cache `other`
	 *				<b> Complexity:</b> linear with number of elements in Cache.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 * @param[in]	other Cache to copy
	 */
	Cache(const Cache& other)
	    : m_container(other.m_container)
	    , m_list(other.m_list)
	    , m_maxSize(other.m_maxSize)
	{ postCopyIteratorCorrection(); }

	/**
	 * @brief		Move constructor.
	 * @details		Moves container `other` to this container. The state of `other` after this operation
	 *				is undefined.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> All iterators are invalidated.\n
	 *				<b> Exception Safety:</b> Strong Guarantee.\n
	 * @param[in]	other	cache to move
	 */
	Cache(Cache&& other) noexcept
	    : m_container(std::move(other.m_container))
	    , m_list(std::move(other.m_list))
	    , m_maxSize(std::move(other.m_maxSize))
	{
	}

	/**
	 * @brief		Initializer List Constructor.
	 * @details		Constructs a cache from an initializer list. If n is not specified, it will default
	 *				to the initializer list size. If it is specified, than *only* the first n elements
	 *				will be copied from the initializer list(!)
	 *				<b> Complexity:</b> Linear.\n
	 *				<b> Iterator Validity:</b> N/A.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic Guarantee.\n
	 * @param[in]	il		initializer list containing key-value pairs
	 * @param[in]	n		maximum number of elements. Also used as the initial number of hash table buckets.
	 * @param[in]	hf		Hasher function object. A hasher is a function that returns an integral value based on the container object key passed to it as
	 * argument.
	 * @param[in]	eql		Comparison function object, that returns true if the two container object keys passed as arguments are to be considered equal.
	 * @param[in]	alloc	Allocator object to be used instead of constructing a new one. For class instantiations using their version of the default allocator
	 * class template, this parameter is not relevant.
	 */
	Cache(std::initializer_list<value_type> il,
	      size_type                         n     = 0,
	      const hasher&                     hf    = hasher(),
	      const key_equal&                  eql   = key_equal(),
	      const allocator_type&             alloc = allocator_type())
	    : m_container(n + 1, hf, eql, map_allocator_type(alloc))
	    , m_list(list_allocator_type(alloc))
	{
		if (n == 0)
			m_maxSize = il.size();
		else
			m_maxSize = n;

		copyRange(il.begin(), il.end(), std::true_type(), std::true_type());
	}

	//----------------------------------
	//	DESTRUCTORS
	//----------------------------------

	virtual ~Cache() {}

	//----------------------------------
	//	ASSIGNMENT OPERATORS
	//----------------------------------

	/**
	 * @brief		Copy Assignment.
	 * @details
	 *				<b> Complexity:</b> Linear with Cache size.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic Guarantee.\n
	 * @param[in]	other	Cache to copy.
	 * @returns		*this
	 */
	Cache& operator=(const Cache& other)
	{
		m_container = other.m_container;
		m_list      = other.m_list;
		m_maxSize   = other.m_maxSize;

		postCopyIteratorCorrection();

		return *this;
	}

	/**
	 * @brief		Move Assignment.
	 * @details		moves `other` into this container. The state of `other` is undefined after this
	 *				operation.
	 *				<b> Complexity:</b> Linear with Cache size.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic Guarantee.\n
	 * @param[in]	other	Cache to copy.
	 * @returns		*this
	 */
	Cache& operator=(Cache&& other) noexcept
	{
		m_container = std::move(other.m_container);
		m_list      = std::move(other.m_list);
		m_maxSize   = std::move(other.m_maxSize);

		return *this;
	}

	/**
	 * @brief		Initializer list assignment.
	 * @details		Assigns the contents of the initializer list il as the elements of the container object.
	 *				<b> Complexity:</b> Linear with `il` size.\n
	 *				<b> Iterator Validity:</b> N/A.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic Guarantee.\n
	 * @param[in]	il	An initializer_list object. The compiler will automatically construct such objects from initializer list declarators. Member type
	 * value_type is the type of the elements contained in the unordered_map, which is pair<const key_type,mapped_type>, where member type key_type is an alias
	 * of the first template parameter (the key type), and mapped_type is an alias of the second template parameter (the mapped type, T).
	 * @returns		* this
	 */
	Cache& operator=(std::initializer_list<value_type> il)
	{
		copyRange(il.begin(), il.end(), std::true_type(), std::true_type());

		return *this;
	}

	//----------------------------------
	//	CAPACITY
	//----------------------------------

	/**
	 * @brief		Return size of allocated storage capacity
	 * @details		Returns the size of the storage space currently allocated for the Cache,
	 *				expressed in terms of elements. This capacity is not necessarily equal to the
	 *				Cache size. It can be equal or greater, with the extra space allowing to
	 *				accommodate for growth.
	 *				Notice that this capacity imposes a limit on the size of the Cache. When this
	 *				capacity is exhausted and more is needed, the least-recently accessed element in
	 *				the cache will be destroyed. The theoretical limit on the size of a Cache is
	 *				given by member max_size.
	 *				The capacity of a Cache can be explicitly altered by calling member Cache::reserve.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> None.\n
	 *				<b> Exception Safety:</b> <b>No-throw guarantee</b>: this member function never throws exceptions..\n
	 * @sa			reserve(), max_size()
	 * @returns		size_type
	 */
	size_type capacity() const throw() { return m_maxSize; }

	/**
	 * @brief		Test whether container is empty.
	 * @details		The container is empty if it's size is 0.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> None.\n
	 *				<b> Exception Safety:</b> <b>No-throw guarantee</b>: this member function never throws exceptions..\n
	 * @sa			size()
	 * @returns		True if the container is empty, false otherwise.
	 */
	bool empty() const throw() { return m_container.empty(); }

	/**
	 * @brief		container size
	 * @details		Represents the number of elements currently stored in the container.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> N/A.\n
	 *				<b> Exception Safety:</b> No-throw guarantee: this member function never throws exceptions.\n
	 * @sa			capacity()
	 * @returns		the number of elements in the Cache
	 */
	size_type size() const throw() { return m_container.size(); }

	/**
	 * @brief		Maximum possible size of Cache.
	 * @details		Returns the theoretical maximum limit of the cache size due to system or library
	 *				implementation contraints. Notice that the Cache will not hold more than `cacapity()`
	 *				elements at any given time.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> None.\n
	 *				<b> Exception Safety:</b> No-throw guarantee: this member function never throws exceptions.\n
	 * @sa			capacity
	 * @returns		size_type
	 */
	size_type max_size() const throw() { return m_container.max_size(); }

	/**
	 * @brief		request a capacity change
	 * @details		Requests that the capacity be at least enough to contain n elements. If n is
	 *				greater than the current capacity, the function causes the container to rehash
	 *				its storage increasing its capacity to n.
	 *				<b> Complexity:</b> Linear with <i>n</i>.\n
	 *				<b> Iterator Validity:</b> All iterators are invalidated, but references and
	 *					pointers to individual elements remain valid..\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @param[in]	n		requested capacity of the container
	 */
	void reserve(size_type n)
	{
		if (n > m_maxSize)
		{
			m_maxSize = n;
			m_container.reserve(n + 1);
		}
	}

	//----------------------------------
	//	ITERATORS
	//----------------------------------

	/**
	 * @brief		returns an iterator to the front of the cache
	 * @details		The front is defined as the most-recently accessed element. Dereferencing the
	 *				iterator *does not* change the cache order.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> <b>No-throw guarantee</b>: this member function never throws exceptions.\n
	 * @returns		iterator to the most-recently accessed cache element
	 */
	iterator begin() noexcept { return iterator(this, m_list.begin()); }

	const_iterator begin() const throw() { return const_iterator(this, m_list.begin()); }

	/**
	 * @brief		returns a const_iterator to the front of the cache
	 * @details		The front is defined as the most-recently accessed element. Dereferencing the
	 *				iterator *does not* change the cache order.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> No-throw guarantee: this member function never throws exceptions.\n
	 * @returns		const_iterator to the most-recently accessed cache element
	 */
	const_iterator cbegin() const throw() { return const_iterator(this, m_list.begin()); }

	/**
	 * @brief		returns iterator to the reverse beginning
	 * @details		Returns a reverse iterator pointing to the last element in the container (i.e., its reverse beginning).
	 *				Reverse iterators iterate backwards: increasing them moves them towards the beginning of the container.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> No-throw guarantee: this member function never throws exceptions.\n
	 * @returns		iterator to the last element
	 */
	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

	const_reverse_iterator rbegin() const throw() { return static_cast<const_reverse_iterator>(const_iterator(end())); }

	/**
	 * @brief		returns const iterator to the reverse beginning
	 * @details		Returns a reverse iterator pointing to the last element in the container (i.e., its reverse beginning).
	 *				Reverse iterators iterate backwards: increasing them moves them towards the beginning of the container.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> No-throw guarantee: this member function never throws exceptions.\n
	 * @returns		const iterator to the last element
	 */
	const_reverse_iterator crbegin() const throw() { return const_reverse_iterator(cend()); }

	/**
	 * @brief		Return iterator to end of cache
	 * @details		The end of the cache is the one-past-most-recently-used element. It is not
	 *				dereferenceable.
	 * @returns		end iterator
	 */
	iterator end() noexcept { return iterator(this, m_list.end()); }

	const_iterator end() const throw() { return const_iterator(this, m_list.end()); }

	/**
	 * @brief		Return const_iterator to end of cache
	 * @details		The end of the cache is the one-past-most-recently-used element. It is not
	 *				dereferenceable.
	 * @returns		end iterator
	 */
	const_iterator cend() const throw() { return const_iterator(this, m_list.end()); }

	/**
	 * @brief		Return iterator to the reverse end of cache
	 * @details		The reverse end of the cache is the one-before-the-most-recently-used element. It is not
	 *				dereferenceable.
	 * @returns		reverse end iterator
	 */
	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

	const_reverse_iterator rend() const throw() { return const_reverse_iterator(begin()); }

	/**
	 * @brief		Return const iterator to the reverse end of cache
	 * @details		The reverse end of the cache is the one-before-the-most-recently-used element. It is not
	 *				dereferenceable.
	 * @returns		reverse end iterator
	 */
	const_reverse_iterator crend() const throw() { return const_reverse_iterator(begin()); }

	//----------------------------------
	//	ELEMENT ACCESS
	//----------------------------------

	/**
	 * @brief		Access element.
	 * @details		If k matches the key of an element in the container, the function returns a
	 *				reference to its mapped value. If k does not match the key of any element in the
	 *				container, the function inserts a new element with that key and returns a reference
	 *				to its mapped value. The key k marked as the most-recently accessed cache entry.
	 *				Notice that this always increases the container size by one,
	 *				even if no mapped value is assigned to the element (the element is constructed
	 *				using its default constructor), until the container size reaches its capacity.
	 *				A similar member function, Cache::at, has the same behavior when an element with
	 *				the key exists, but throws an exception when it does not.
	 *				<b> Complexity:</b> Average case - constant. Worst case - linear in container size.\n
	 *				<b> Iterator Validity:</b> Invalidates all iterators.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong Guarantee.\n
	 * @param[in]	k	key to look-up/insert.
	 * @sa			at()
	 * @returns		reference to the value located at the key.
	 */
	mapped_type& operator[](const key_type& k)
	{
		static_assert(std::is_default_constructible_v<mapped_type>, "Cache::mapped_type must be default constructible.");

		if (m_container.count(k))
			return accessElement(k);
		else
		{
			m_container.emplace(std::piecewise_construct, std::forward_as_tuple(k), std::make_tuple());
			m_list.push_front(k);
			m_container[k].second = m_list.begin();

			if (m_container.size() > m_maxSize)
			{
				m_container.erase(m_list.back());
				m_list.pop_back();
			}

			return m_container[k].first;
		}
	}

	// I didn't make an r-value version on operator[] because I don't think it would change the performance.

	/**
	 * @brief		Access element
	 * @details		Returns a reference to the mapped value of the element with key k in the Cache.
	 *				If k does not match the key of any element in the container, the function throws
	 *				an out_of_range exception. The key k marked as the most-recently accessed cache entry.
	 *				<b> Complexity:</b> Average case - constant. Worst case - linear in container size.\n
	 *				<b> Iterator Validity:</b> Invalidates all iterators.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b>  Throws if k is not a key present in the Cache.\n
	 * @param[in]	k key to access
	 * @returns		reference to the mapped type at the key location
	 */
	mapped_type& at(const key_type& k)
	{
		if (m_container.count(k))
			return accessElement(k);
		else
			throw std::out_of_range("Key does not exist in Cache");
	}

	/**
	 * @brief		Access element without changing cache order.
	 * @details		Returns a reference to the mapped value of the element with key k in the Cache
	 *				without changing the order of the cache.
	 *				If k does not match the key of any element in the container, the function throws
	 *				an out_of_range exception.
	 *				<b> Complexity:</b> Average case - constant. Worst case - linear in container size.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Throws if k is not a key present in the Cache.\n
	 * @param[in]	k key to access
	 * @returns		reference to the mapped type at the key location
	 */
	mapped_type& peek(const key_type& k)
	{
		if (m_container.count(k))
			return m_container[k].first;
		else
			throw std::out_of_range("Key does not exist in Cache");
	}

	const mapped_type& peek(const key_type& k) const
	{
		if (m_container.count(k))
			return m_container.at(k).first;
		else
			throw std::out_of_range("Key does not exist in Cache");
	}

	/**
	 * @brief		Access most-recently used element
	 * @details		Accesses the front of the cache, i.e. the most recently used element. Does not
	 *				alter the cache order.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> <b>No-throw guarantee</b>: this member function never throws exceptions.\n
	 * @sa			begin()
	 * @returns		mapped_type&
	 */
	mapped_type& front() noexcept { return *begin(); }

	const mapped_type& front() const throw() { return *cbegin(); }

	/**
	 * @brief		Access last element
	 * @details		Returns a reference to the least-recently-used element in the container. Calling
	 *				this function on an empty container causes undefined behavior.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> <b>No-throw guarantee</b>: this member function never throws exceptions.\n
	 * @sa			end()
	 * @returns		reference to last element
	 */
	mapped_type& back() noexcept { return *rbegin(); }

	const mapped_type& back() const throw() { return *crbegin(); }

	//----------------------------------
	//	ELEMENT LOOK-UP
	//----------------------------------

	/**
	 * @brief		Get iterator to element.
	 * @details		Searches the container for an element with k as key and returns an iterator to
	 *				it if found, otherwise it returns an iterator to Cache::end (the element past
	 *				the end of the container).
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> <b>No-throw guarantee</b>: this member function never throws exceptions.\n
	 * @param[in]	k	value of the key to find in the Cache.
	 * @returns		An iterator to the element, if the specified key value is found, or
	 *				Cache::end if the specified key is not found in the container.
	 */
	iterator find(const key_type& k) noexcept
	{
		auto itr = m_container.find(k);
		if (itr == m_container.end())
			return end();
		else
			return iterator(this, itr->second.second);
	}

	/**
	 * @brief		Count elements with a specific key
	 * @details		Searches the container for elements whose key is k and returns the number of
	 *				elements found. Because Cache containers do not allow for duplicate keys, this
	 *				means that the function actually returns 1 if an element with that key exists
	 *				in the container, and zero otherwise.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> <b>No-throw guarantee</b>: this member function never throws exceptions.\n
	 * @param[in]	k	Key value to be searched for.
	 * @returns		1 if an element with a key equivalent to k is found, or zero otherwise.
	 */
	size_type count(const key_type& k) const throw() { return m_container.count(k); }

	//----------------------------------
	//	MODIFIERS
	//----------------------------------

	/**
	 * @brief		Construct and insert element
	 * @details		Inserts a new element in the unordered_map if its key is unique. This new element
	 *				is constructed in place using args as the arguments for the element's constructor.
	 *				The emplaced element will be the first element of the cache.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @param[in]	args	Arguments forwarded to construct the new element (of type pair<const key_type, mapped_type>).
	 *						This can be one of:
	 *						-	Two arguments: one for the key, the other for the mapped value.
	 *						-	A single argument of a pair type with a value for the key as first member,
	 *							and a value for the mapped value as second.
	 *						-	piecewise_construct as first argument, and two additional arguments with
	 *							tuples to be forwarded as arguments for the key value and for the mapped
	 *							value respectively.
	 *						Piecewise initializer_list syntax is *not* supported.
	 * @returns		If the insertion takes place (because no other element existed with the same key),
	 *				the function returns a pair object, whose first component is an iterator to the
	 *				inserted element, and whose second component is true. Otherwise, the pair object
	 *				returned has as first component an iterator pointing to the element in the container
	 *				with the same key, and false as its second component.
	 */
	template<class... Args>
	std::pair<iterator, bool> emplace(Args&&... args)
	{
		auto ret = emplaceDispatch(m_list.begin(), std::forward<Args>(args)...);
		if (m_container.size() > m_maxSize)
		{
			m_container.erase(m_list.back());
			m_list.pop_back();
		}
		return ret;
	}

	/**
	 * @brief		Construct and insert element with hint
	 * @details		Inserts a new element in the unordered_map before `position` if its key is unique. This new element
	 *				is constructed in place using args as the arguments for the element's constructor. If the Cache
	 *				is full and an element is inserted to the end(), it will be immediately destroyed,
	 *				resulting in no change to the container.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic guarantee.\n
	 * @note		Emplacing to the end() of a full cache will result in the emplaced value being the
	 *				back of the cache, and the former value of `back()` being destroyed.
	 * @param[in]	position	Iterator to the position within the cache at which to emplace the item.
	 * @param[in]	args		Arguments forwarded to construct the new element (of type pair<const key_type, mapped_type>).
	 *							This can be one of:
	 *							-	Two arguments: one for the key, the other for the mapped value.
	 *							-	A single argument of a pair type with a value for the key as first member,
	 *								and a value for the mapped value as second.
	 *							-	piecewise_construct as first argument, and two additional arguments with
	 *								tuples to be forwarded as arguments for the key value and for the mapped
	 *								value respectively.
	 *							Piecewise initializer_list syntax is *not* supported.
	 * @returns		If the insertion takes place (because no other element existed with the same key),
	 *				the function returns a pair object, whose first component is an iterator to the
	 *				inserted element, and whose second component is true. Otherwise, the pair object
	 *				returned has as first component an iterator pointing to the element in the container
	 *				with the same key, and false as its second component.
	 */
	template<class... Args>
	std::pair<iterator, bool> emplace_hint(const_iterator position, Args&&... args)
	{
		auto ret = emplaceDispatch(position.m_listItr, std::forward<Args>(args)...);
		if (m_container.size() > m_maxSize)
		{
			m_container.erase(m_list.back());
			m_list.pop_back();
		}
		return ret;
	}

	/**
	 * @brief		Insert element
	 * @details		Copy element into the Cache. Each element is inserted only if its key is
	 *				not equivalent to the key of any other element already in the container (keys in
	 *				a Cache are unique).
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic guarantee.\n
	 * @param[in]	val		Object to be copied to (or moved as) the value of the new element. Member
	 *						type value_type is defined as std::pair<const key_type, mapped_type>.
	 * @returns		 a pair object whose first element is an iterator pointing either to the newly
	 *				inserted element in the container or to the element whose key is equivalent,
	 *				and a bool value indicating whether the element was successfully inserted or not.
	 */
	std::pair<iterator, bool> insert(const value_type& val)
	{
		auto ret = emplaceDispatch(m_list.begin(), std::forward<const value_type&>(val));
		if (m_container.size() > m_maxSize)
		{
			m_container.erase(m_list.back());
			m_list.pop_back();
		}
		return ret;
	}

	/**
	 * @brief		Insert element
	 * @details		Move element into the Cache. Each element is inserted only if its key is
	 *				not equivalent to the key of any other element already in the container (keys in
	 *				a Cache are unique).
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic guarantee.\n
	 * @param[in]	val		Object to be copied to (or moved as) the value of the new element. Member
	 *						type value_type is defined as std::pair<const key_type, mapped_type>.
	 * @returns		 a pair object whose first element is an iterator pointing either to the newly
	 *				inserted element in the container or to the element whose key is equivalent,
	 *				and a bool value indicating whether the element was successfully inserted or not.
	 */
	std::pair<iterator, bool> insert(value_type&& val)
	{
		auto ret = emplaceDispatch(m_list.begin(), std::move(val));
		if (m_container.size() > m_maxSize)
		{
			m_container.erase(m_list.back());
			m_list.pop_back();
		}
		return ret;
	}

	/**
	 * @brief		Insert element at position
	 * @details		Copy element into the Cache at the given position. Each element is inserted only if its key is
	 *				not equivalent to the key of any other element already in the container (keys in
	 *				a Cache are unique).
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic guarantee.\n
	 * @param[in]	pos		Iterator with the position within the cache where the new element will be
	 *						inserted (before). If the cache is full, inserting at the end will cause
	 *						the last element (the one just inserted) to be destroyed, resulting in
	 *						no net change to the container.
	 * @param[in]	val		Object to be copied to (or moved as) the value of the new element. Member
	 *						type value_type is defined as std::pair<const key_type, mapped_type>.
	 * @returns		an iterator to the newly inserted element, or the key position if it was already
	 *				in the cache.
	 */
	iterator insert(const_iterator pos, const value_type& val)
	{
		auto ret = emplaceDispatch(pos.m_listItr, std::forward<const value_type&>(val));
		if (m_container.size() > m_maxSize)
		{
			m_container.erase(m_list.back());
			m_list.pop_back();
		}
		return ret.first;
	}

	/**
	 * @brief		Insert element
	 * @details		Move element into the Cache. Each element is inserted only if its key is
	 *				not equivalent to the key of any other element already in the container (keys in
	 *				a Cache are unique).
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> No changes.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic guarantee.\n
	 * @param[in]	pos		Iterator with the position within the cache where the new element will be
	 *						inserted (before). If the cache is full, inserting at the end will cause
	 *						the last element (the one just inserted) to be destroyed, resulting in
	 *						no net change to the container.
	 * @param[in]	val		Object to be copied to (or moved as) the value of the new element. Member
	 *						type value_type is defined as std::pair<const key_type, mapped_type>.
	 * @returns		an iterator to the newly inserted element, or the key position if it was already
	 *				in the cache.
	 */
	iterator insert(const_iterator pos, value_type&& val)
	{
		auto ret = emplaceDispatch(pos.m_listItr, std::move(val));
		if (m_container.size() > m_maxSize)
		{
			m_container.erase(m_list.back());
			m_list.pop_back();
		}
		return ret.first;
	}

	/**
	 * @brief		Insert range of elements
	 * @details		Copies the range of elements into the container.  Order is preserved, so 'first'
	 *				will be the top of the cache, and 'last' will be the bottom. If the range is larger
	 *				than the Cache capacity, then only `Cache::capacity()` number of elements will be
	 *				inserted.
	 *				<b> Complexity:</b> Linear with size of range.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Basic guarantee.\n
	 * @param[in]	first	InputIterator to a `value_type` representing the beginning of the range
	 *						to be inserted.
	 * @param[in]	last	InputIterator to a `value_type` representing the end of the range
	 *						to be inserted.
	 */
	template<class InputIterator>
	void insert(InputIterator first, InputIterator last)
	{
		copyRange(first,
		          last,
		          typename std::is_same<typename std::iterator_traits<InputIterator>::value_type, value_type>::type(),
		          typename std::integral_constant < bool,
		          std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::bidirectional_iterator_tag> ||
		                  std::is_same_v < typename std::iterator_traits<InputIterator>::iterator_category,
		          std::random_access_iterator_tag >> ::type());
	}

	/**
	 * @brief		Insert range of elements
	 * @details		Copies the range of elements into the container.  Order is preserved, so 'first'
	 *				will be the top of the cache, and 'last' will be the bottom. If the list is larger
	 *				than the Cache capacity, then only `Cache::capacity()` number of elements will be
	 *				inserted.
	 *				<b> Complexity:</b> Linear with size of range.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @param[in]	il	initializer list containing the value to insert
	 */
	void insert(std::initializer_list<value_type> il) { copyRange(il.begin(), il.end(), std::true_type(), std::true_type()); }

	/**
	 * @brief		Erase element
	 * @details		Removes a single element from the Cache container. This effectively reduces the
	 *				container size by the number of elements removed, calling each element's destructor.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Only the erased iterators/references are invalidated.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @param[in]	position	iterator to an element in the Cache to erase
	 * @returns		an iterator pointing to the position immediately following the last of the
	 *				elements erased.
	 */
	iterator erase(const_iterator position)
	{
		m_container.erase(*(position.m_listItr));
		return iterator(this, m_list.erase(position.m_listItr));
	}

	/**
	 * @brief		Erase element
	 * @details		Removes a single element from the Cache container. This effectively reduces the
	 *				container size by the number of elements removed, calling each element's destructor.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Only the erased iterators/references are invalidated.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @param[in]	k	key value to be removed from the Cache.
	 * @returns		number of elements removed, which will be 1 if the key was present in the container,
	 *				0 zero if it was not found.
	 */
	size_type erase(const key_type& k)
	{
		if (m_container.count(k))
		{
			m_list.erase(m_container[k].second);
			return m_container.erase(k);
		}
		else
			return 0;
	}

	/**
	 * @brief		Erase range of elements.
	 * @details		Removes a range of elements from the Cache container. This effectively reduces the
	 *				container size by the number of elements removed, calling each element's destructor.
	 *				<b> Complexity:</b> Linear with the number of elements erased.\n
	 *				<b> Iterator Validity:</b>  Only the erased iterators/references are invalidated.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @param[in]	first	iterator to the first element to be erased.
	 * @param[in]	last	iterator to the one-past-the-last element to be erased.
	 * @returns		an iterator pointing to the position immediately following the last of the elements erased.
	 */
	iterator erase(const_iterator first, const_iterator last)
	{
		const_iterator position;
		for (position = first; position != last;)
		{
			position = erase(position);
		}
		return iterator(this, m_list.erase(position.m_listItr, position.m_listItr));    // constant time iterator from const_iterator
	}

	/**
	 * @brief		Shrink Cache size
	 * @details		Changes the Cache maximum capacity to <i>n</i> and reduces the Cache to size
	 *				by erasing the least-recently used elements. If n is greater than or equal to the
	 *				current capacity, this function does nothing.
	 *				<b> Complexity:</b> Linear with n.\n
	 *				<b> Iterator Validity:</b> Iterators to erased elements are invalidated. All others
	 *					remain valid.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @param[in]	n
	 * @returns		void
	 */
	void shrink(size_type n)
	{
		if (n < m_maxSize)
			m_maxSize = n;

		if (m_list.size() > m_maxSize)
		{
			for (auto itr = std::next(m_list.begin(), m_maxSize); itr != m_list.end();)
			{
				m_container.erase(*itr);
				itr = m_list.erase(itr);
			}
		}
	}

	/**
	 * @brief		clear content
	 * @details		All the elements in the Cache container are dropped: their destructors
	 *				are called, and they are removed from the container, leaving it with a size of 0.
	 *				<b> Complexity:</b> Linear with the size of the container.\n
	 *				<b> Iterator Validity:</b> All iterators, pointers, and references are invalidated.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> <b>No-throw guarantee</b>: this member function never throws exceptions.\n
	 */
	void clear() noexcept
	{
		m_container.clear();
		m_list.clear();
	}

	/**
	 * @brief		Swap content
	 * @details		Exchanges the content of the container by the content of <i>other</i>, which is another
	 *				Cache object containing elements of the same type. Sizes may differ.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> All iterators, pointers and references remain valid,
	 *					but now are referring to elements in the other container, and iterate in it.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong Guarantee.\n
	 * @param[in]	other	Cache to swap contents with this Cache.
	 */
	void swap(Cache& other) noexcept
	{
		std::swap(m_container, other.m_container);
		std::swap(m_list, other.m_list);
		std::swap(m_maxSize, other.m_maxSize);
	}

	template<class K, class Ty, class H, class P, class A>
	void swap(Cache<K, Ty, H, P, A>& lhs, Cache<K, Ty, H, P, A>& rhs)
	{ lhs.swap(rhs); }

	//----------------------------------
	//	OBSERVERS
	//----------------------------------

	/**
	 * @brief		Get hash function
	 * @details		Returns the hash function object used by the container.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @returns		hash function
	 */
	hasher hash_function() const { return hasher(); }

	/**
	 * @brief		Get key equivalence predicate
	 * @details		Returns the key equivalence comparison predicate used by the container.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @returns		key equivalence predicate
	 */
	key_equal key_eq() const { return key_equal(); }

	/**
	 * @brief		Get allocator
	 * @details		Returns the allocator object used to construct the container.
	 *				<b> Complexity:</b> Constant.\n
	 *				<b> Iterator Validity:</b> Unchanged.\n
	 *				<b> Data Races:</b> .\n
	 *				<b> Exception Safety:</b> Strong guarantee.\n
	 * @returns		allocator
	 */
	allocator_type get_allocator() const throw() { return allocator_type(); }

	//----------------------------------
	//	RELATIONAL OPERATORS
	//----------------------------------

	friend bool operator==(const Cache& lhs, const Cache& rhs)
	{
		if (lhs.size() != rhs.size())
			return false;

		auto litr = lhs.cbegin();
		auto ritr = rhs.cbegin();

		for (; litr != lhs.cend() && ritr != rhs.cend(); ++litr, ++ritr)
		{
			if (!(*litr == *ritr))
				return false;
		}

		return true;
	}

	friend bool operator!=(const Cache& lhs, const Cache& rhs) { return !(lhs == rhs); }

protected:
	/// construct class from a tuple of arguments. http://stackoverflow.com/questions/14897527/constructor-arguments-from-tuple
	template<std::size_t... Is>
	struct indices
	{
	};

	template<std::size_t N, std::size_t... Is>
	struct build_indices : build_indices<N - 1, N - 1, Is...>
	{
	};

	template<std::size_t... Is>
	struct build_indices<0, Is...> : indices<Is...>
	{
	};

	template<typename Ty>
	struct fromTuple
	{
		template<typename... Args>
		static Ty construct(std::tuple<Args...>&& args)
		{ return construct(std::move(args), build_indices<sizeof...(Args)>{}); }

		template<typename... Args>
		static Ty construct(std::initializer_list<Args...> args)
		{ return construct(std::move(args), build_indices<sizeof...(Args)>{}); }

	private:
		template<std::size_t... Is, typename... Args>
		static Ty construct(std::tuple<Args...>&& args, indices<Is...>)
		{ return Ty(std::get<Is>(args)...); }

		template<std::size_t... Is, typename... Args>
		static Ty construct(std::initializer_list<Args...> args, indices<Is...>)
		{ return Ty{std::get<Is>(args)...}; }
	};

	/// Overload for iterators that match <key_type, mapped_type> with a bidirectional iterator (i.e. another Cache)
	template<class InputIterator>
	void copyRange(InputIterator first, InputIterator last, std::true_type isKeyValueItr, std::true_type isBidirectional)
	{
		// only copy the first n
		InputIterator realLast = last;
		if (std::distance(first, last) > static_cast<difference_type>(m_maxSize))
			realLast = std::next(first, m_maxSize);

		// use reverse iterators to preserve cache order.
		auto rlast  = std::reverse_iterator<InputIterator>(first);
		auto rfirst = std::reverse_iterator<InputIterator>(realLast);

		for (auto itr = rfirst; itr != rlast; ++itr)
		{
			auto listPos = m_list.insert(m_list.begin(), itr->first);
			m_container.emplace(std::piecewise_construct,
			                    std::forward_as_tuple(itr->first),
			                    std::forward_as_tuple(std::piecewise_construct, std::forward_as_tuple(itr->second), std::forward_as_tuple(std::move(listPos))));
		}
	}

	/// Overload for iterators that match <key_type, mapped_type> with a forward iterator (i.e. unordered_map, map)
	template<class InputIterator>
	void copyRange(InputIterator first, InputIterator last, std::true_type isKeyValueItr, std::false_type isBidirectional)
	{
		// only copy the first n
		InputIterator realLast = last;
		if (std::distance(first, last) > static_cast<difference_type>(m_maxSize))
			realLast = std::next(first, m_maxSize);

		// Order is not preserved in this case because the underlying container doesn't have an inherent order.
		for (auto itr = first; itr != realLast; ++itr)
		{
			auto listPos = m_list.insert(m_list.begin(), itr->first);
			m_container.emplace(std::piecewise_construct,
			                    std::forward_as_tuple(itr->first),
			                    std::forward_as_tuple(std::piecewise_construct, std::forward_as_tuple(itr->second), std::forward_as_tuple(std::move(listPos))));
		}
	}

	/// Overload for iterators that only match mapped_type (i.e. vector, list)
	template<class InputIterator>
	    requires std::is_constructible_v<mapped_type, typename std::iterator_traits<InputIterator>::value_type>
	void copyRange(InputIterator first, InputIterator last, std::false_type, std::true_type)
	{
		// only copy the first n
		InputIterator realLast = last;
		if (std::distance(first, last) > static_cast<difference_type>(m_maxSize))
			realLast = std::next(first, m_maxSize);

		// use reverse iterators to preserve cache order.
		auto rlast  = std::reverse_iterator<InputIterator>(first);
		auto rfirst = std::reverse_iterator<InputIterator>(realLast);

		size_t i = std::distance(rfirst, rlast) + size() - 1;
		for (auto itr = rfirst; itr != rlast; std::advance(itr, 1))
		{
			auto listPos = m_list.insert(m_list.begin(), i);
			m_container.emplace(std::piecewise_construct,
			                    std::forward_as_tuple(i),
			                    std::forward_as_tuple(std::piecewise_construct, std::forward_as_tuple(*itr), std::forward_as_tuple(std::move(listPos))));
			--i;
		}
	}

	/// Fixes the container iterators after a copy operation
	void postCopyIteratorCorrection()
	{
		// fix list iterators (they point to the other.m_list, not m_list)
		for (auto itr = m_list.begin(); itr != m_list.end(); std::advance(itr, 1))
		{
			m_container[*itr].second = itr;
		}
	}

	/// Returns an element reference and moves it to the front of the cache.
	mapped_type& accessElement(const key_type& k)
	{
		m_list.splice(m_list.begin(), m_list, m_container[k].second);
		m_container[k].second = m_list.begin();
		return m_container[k].first;
	}

	/// emplace for universal (l-value/r-value) key/value arguments
	template<class K, class M>
	    requires std::is_convertible_v<std::decay_t<K>, key_type> && std::is_convertible_v<std::decay_t<M>, mapped_type>
	std::pair<iterator, bool> emplaceDispatch(const_list_iterator_type itr, K&& k, M&& m)
	{
		list_iterator_type listPos;
		try
		{
			list_iterator_type mutableItr = m_list.erase(itr, itr);    // constant-time remove const.
			if (!m_container.count(k))
			{
				listPos  = m_list.insert(itr, k);
				auto ret = std::pair<iterator, bool>(iterator(this, listPos), true);
				m_container.emplace(std::forward<K>(k), std::make_pair(std::forward<M>(m), std::move(listPos)));
				return ret;
			}
			else
				return std::pair<iterator, bool>(iterator(this, m_container[k].second), false);
		}
		catch (...)
		{
			m_list.erase(listPos);
			throw;
		}
	}

	/// emplace for universal (l-value/r-value) key/value pair
	std::pair<iterator, bool> emplaceDispatch(const_list_iterator_type itr, const value_type& p)
	{
		list_iterator_type listPos;
		try
		{
			if (!m_container.count(p.first))
			{
				listPos  = m_list.insert(itr, p.first);
				auto ret = std::pair<iterator, bool>(iterator(this, listPos), true);
				m_container.emplace(
				        std::piecewise_construct,
				        std::forward_as_tuple(p.first),
				        std::forward_as_tuple(std::piecewise_construct, std::forward_as_tuple(p.second), std::forward_as_tuple(std::move(listPos))));
				return ret;
			}

			return std::pair<iterator, bool>(iterator(this, m_container[p.first].second), false);
		}
		catch (...)
		{
			m_list.erase(listPos);
			throw;
		}
	}

	/// emplace for r-value pair
	std::pair<iterator, bool> emplaceDispatch(const_list_iterator_type itr, value_type&& p)
	{
		list_iterator_type listPos;
		try
		{
			if (!m_container.count(p.first))
			{
				listPos  = m_list.insert(itr, std::move(p.first));
				auto ret = std::pair<iterator, bool>(iterator(this, listPos), true);
				m_container.emplace(
				        std::piecewise_construct,
				        std::forward_as_tuple(p.first),
				        std::forward_as_tuple(std::piecewise_construct, std::forward_as_tuple(std::move(p.second)), std::forward_as_tuple(std::move(listPos))));
				return ret;
			}

			return std::pair<iterator, bool>(iterator(this, m_container[p.first].second), false);
		}
		catch (...)
		{
			m_list.erase(listPos);
			throw;
		}
	}

	/// piecewise-constructed emplace
	template<class KeyArgs, class ValArgs>
	std::pair<iterator, bool> emplaceDispatch(const_list_iterator_type itr, std::piecewise_construct_t, KeyArgs&& k, ValArgs&& v)
	{
		list_iterator_type listPos;
		try
		{
			key_type key = fromTuple<key_type>::construct(std::move(k));
			if (!m_container.count(key))
			{
				listPos  = m_list.insert(itr, key);
				auto ret = std::pair<iterator, bool>(iterator(this, listPos), true);
				m_container.emplace(std::piecewise_construct,
				                    std::forward_as_tuple(key),
				                    std::forward_as_tuple(std::piecewise_construct, std::move(v), std::forward_as_tuple(std::move(listPos))));
				return ret;
			}

			return std::pair<iterator, bool>(iterator(this, m_container[key].second), false);
		}
		catch (...)
		{
			m_list.erase(listPos);
			throw;
		}
	}

private:
	container_type m_container;    ///< Stores the cached data.
	list_type      m_list;         ///< Stores the most recently used keys, in order of most recent (begin) to least recent (end).
	size_type      m_maxSize;      ///< Maximum number of elements the cache will store before deleting the oldest one.
};

#endif    // cache_h__