#ifndef CAS_h__
#define CAS_h__

//------------------------
//	INCLUDES
//------------------------
#include <atomic>

/**
 * @brief		atomically compares and exchanges <i>val</i> with <i>a</i> if <i>val</i> is greater than <i>a</i>.
 * @details
 * @param[in]	a atomic value to compare and exchange
 * @param[in]	val value to be exchanged into the atomic if greater than the current atomic value.
 * @returns		value stored in <i>a</i> at the end of the operation.
 */
template<typename T>
T compare_exchange_if_greater(std::atomic<T>& a, T val)
{
    T old = a.load();
    while (val > old && !a.compare_exchange_weak(old, val)) {}
    return a;
}

/**
 * @brief		atomically compares and exchanges <i>val</i> with <i>a</i> if <i>val</i> is less than <i>a</i>.
 * @details
 * @param[in]	a atomic value to compare and exchange
 * @param[in]	val value to be exchanged into the atomic if less than the current atomic value.
 * @returns		value stored in <i>a</i> at the end of the operation.
 */
template<typename T>
T compare_exchange_if_lessthan(std::atomic<T>& a, T val)
{
    T old = a.load();
	while (val < old && !a.compare_exchange_weak(old, val)) {}
    return a;
}

#endif // CAS_h__