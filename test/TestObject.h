#include <iostream>
#include <string>



//	----------------------------------------------------------------------------
//	CLASS
//  ----------------------------------------------------------------------------
///	@brief
///	@details
//  ----------------------------------------------------------------------------
class TestObject
{
public:

	enum ExceptionFlags
	{
		None = 0,
		Constructor = 1,
		CopyConstructor = 2,
		MoveConstructor = 4,
		CopyAssignment = 8,
		MoveAssignment = 16,
	};

	inline friend ExceptionFlags operator|(const ExceptionFlags& lhs, const ExceptionFlags& rhs)
	{
		return static_cast<ExceptionFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

public:

    friend std::ostream& operator<< (std::ostream& os, const TestObject& t);

	TestObject() : str(""), exceptions(None)
    {
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::constructor" << std::endl;
#endif
        defaultConstructorCalls++;
    };

	TestObject(ExceptionFlags exceptions) : str(""), exceptions(exceptions)
	{
#ifdef TEST_OBJECT_OUTPUT
		std::cout << "    TestObject::constructor" << std::endl;
#endif
		constructorCalls++;
		if (exceptions & Constructor)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
			throw std::bad_alloc();
		}
	};
	
    TestObject(char c, ExceptionFlags exceptions = None)
        : str(&c,1), exceptions(exceptions)
    {
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::constructor" << std::endl;
#endif
        constructorCalls++;
        if (exceptions & Constructor)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
            throw std::bad_alloc();
        }
    };

	TestObject(std::string s, ExceptionFlags exceptions = None) : str(s), exceptions(exceptions)
    {
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::constructor" << std::endl;
#endif
        constructorCalls++;
		if (exceptions & Constructor)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
			throw std::bad_alloc();
		}
    };

	TestObject(const char* c, ExceptionFlags exceptions = None) : str(c), exceptions(exceptions)
    {
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::constructor" << std::endl;
#endif
        constructorCalls++;
		if (exceptions & Constructor)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
			throw std::bad_alloc();
		}
    };

    ~TestObject()
    {
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::destructor" << std::endl;
#endif
        destructorCalls++;
    };

    TestObject(const TestObject& other) : str(other.str), exceptions(other.exceptions)
    {
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::copy constructor" << std::endl;
#endif
        if (exceptions & CopyConstructor)
        {
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
            throw std::bad_alloc();
        }
		copyConstructorCalls++;
    }

    TestObject(TestObject&& other) : str(std::move(other.str)), exceptions(std::move(other.exceptions))
    {
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::move constructor" << std::endl;
#endif
        if (exceptions & MoveConstructor)
        {
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
            throw std::bad_alloc();
        }
		moveConstructorCalls++;
    }

    TestObject& operator=(char c)
    {
        str = c;
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::copy assign from char" << std::endl;
#endif
		if (exceptions & CopyAssignment)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
			throw std::bad_alloc();
		}
		copyAssignmentCalls++;
        return *this;
    }

    TestObject& operator=(const char* c)
    {
		if (exceptions & CopyAssignment)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
			throw std::bad_alloc();
		}
        str = c;
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::copy assign from c-string" << std::endl;
#endif
		copyAssignmentCalls++;
        return *this;
    }

    TestObject& operator=(const std::string& s)
    {
		if (exceptions & CopyAssignment)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
			throw std::bad_alloc();
		}
        str = s;
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::copy assign from string" << std::endl;
#endif
        copyAssignmentCalls++;		
        return *this;
    }

    TestObject& operator=(std::string&& s)
    {
		if (exceptions & MoveAssignment)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
			throw std::bad_alloc();
		}
        str = std::move(s);
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::move assign from string" << std::endl;
#endif
        moveAssignmentCalls++;		
        return *this;
    }

    TestObject& operator=(const TestObject& t)
    {
		std::cout << (int)exceptions;
		if (exceptions & CopyAssignment)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
			throw std::bad_alloc();
		}
        str = t.str;
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::copy assign from TestObject" << std::endl;
#endif
        copyAssignmentCalls++;		
        return *this;
    }

    TestObject& operator=(TestObject&& t)
    {
		exceptions = std::move(t.exceptions);
		if (exceptions & MoveAssignment)
		{
#ifdef TEST_OBJECT_OUTPUT
			std::cout << "    TestObject::bad alloc" << std::endl;
#endif
			throw std::bad_alloc();
		}
        str = std::move(t.str);
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::move assign from TestObject" << std::endl;
#endif
        moveAssignmentCalls++;
        return *this;
    }

    bool operator==(const TestObject& rhs) const /*noexcept*/
    {
        return this->str == rhs.str;
    }

    bool operator==(const std::string& str) const /*noexcept*/
    {
        return this->str == str;
    }

    bool operator==(const char* cstr) const /*noexcept*/
    {
        return this->str == cstr;
    }

    bool operator!=(const TestObject& rhs) const
    {
        return !(this->str == rhs.str);
    }

    bool operator!=(const std::string& str) const /*noexcept*/
    {
        return !(this->str == str);
    }

    bool operator!=(const char* cstr) const /*noexcept*/
    {
        return !(this->str == cstr);
    }

    void testFunction()
    {
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::member function" << std::endl;
#endif
        testFunctionCalls++;
    }

    std::string getString() const
    {
#ifdef TEST_OBJECT_OUTPUT
        std::cout << "    TestObject::getString function" << std::endl;
#endif
        getStringCalls++;
        return str;
    }

    // These functions return the numbers of test object function calls SINCE THE LAST TIME THEY
    // WERE REQUESTED

    static unsigned int getdefaultconstructorcalls()
    {
        static unsigned int prev = 0;
        int value = defaultConstructorCalls - prev;
        prev = defaultConstructorCalls;
        return value;
    }

    static unsigned int getconstructorcalls()
    {
        static unsigned int prev = 0;
        int value = constructorCalls - prev;
        prev = constructorCalls;
        return value;
    }

    static unsigned int getcopyconstructorcalls()
    {
        static unsigned int prev = 0;
        int value = copyConstructorCalls - prev;
        prev = copyConstructorCalls;
        return value;
    }

    static unsigned int getmoveconstructorcalls()
    {
        static unsigned int prev = 0;
        int value = moveConstructorCalls - prev;
        prev = moveConstructorCalls;
        return value;
    }

    static unsigned int getcopyassignmentcalls()
    {
        static unsigned int prev = 0;
        int value = copyAssignmentCalls - prev;
        prev = copyAssignmentCalls;
        return value;
    }

    static unsigned int getmoveassignmentcalls()
    {
        static unsigned int prev = 0;
        int value = moveAssignmentCalls - prev;
        prev = moveAssignmentCalls;
        return value;
    }

    static unsigned int getdestructorcalls()
    {
        static unsigned int prev = 0;
        int value = destructorCalls - prev;
        prev = destructorCalls;
        return value;
    }

    static unsigned int gettestfunctioncalls()
    {
        static unsigned int prev = 0;
        int value = testFunctionCalls - prev;
        prev = testFunctionCalls;
        return value;
    }

    static unsigned int getstringfunctioncalls()
    {
        static unsigned int prev = 0;
        int value = getStringCalls - prev;
        prev = getStringCalls;
        return value;
    }

    static void clearfunctioncalls()
    {
        // just call them all to update the prev
        getdefaultconstructorcalls();
        getconstructorcalls();
        getcopyconstructorcalls();
        getmoveconstructorcalls();
        getcopyassignmentcalls();
        getmoveassignmentcalls();
        getdestructorcalls();
        gettestfunctioncalls();
        getstringfunctioncalls();
    }


private:

    std::string str;
	ExceptionFlags exceptions;

    static unsigned int defaultConstructorCalls;
    static unsigned int constructorCalls;
    static unsigned int copyConstructorCalls;
    static unsigned int moveConstructorCalls;
    static unsigned int copyAssignmentCalls;
    static unsigned int moveAssignmentCalls;
    static unsigned int destructorCalls;
    static unsigned int testFunctionCalls;
    static unsigned int getStringCalls;
};

std::ostream& operator<< (std::ostream& os, const TestObject& t)
{
    return os << t.str;
}

unsigned int TestObject::defaultConstructorCalls	= 0;
unsigned int TestObject::constructorCalls			= 0;
unsigned int TestObject::copyConstructorCalls		= 0;
unsigned int TestObject::moveConstructorCalls		= 0;
unsigned int TestObject::copyAssignmentCalls		= 0;
unsigned int TestObject::moveAssignmentCalls		= 0;
unsigned int TestObject::destructorCalls			= 0;
unsigned int TestObject::testFunctionCalls			= 0;
unsigned int TestObject::getStringCalls				= 0;
