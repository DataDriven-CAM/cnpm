#pragma once

#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <filesystem>
#include <functional>
#include <cstring>

#include "Url.h"

#include <coroutine>
#include <stdexcept>
#include <thread>

#ifdef __WIN32__
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "c_plus_plus_serializer.h" 

namespace sylvanmats::reading{

	template<typename T>
	class generator;

	namespace detail
	{
		template<typename T>
		class generator_promise
		{
		public:

			using value_type = std::remove_reference_t<T>;
			using reference_type = std::conditional_t<std::is_reference_v<T>, T, T&>;
			using pointer_type = value_type*;

			generator_promise() = default;

			generator<T> get_return_object() noexcept;

			constexpr std::suspend_always initial_suspend() const { return {}; }
			constexpr std::suspend_always final_suspend() const noexcept { return {}; }
 
			template<
				typename U = T,
				std::enable_if_t<!std::is_rvalue_reference<U>::value, int> = 0>
			std::suspend_always yield_value(std::remove_reference_t<T>& value) noexcept
			{
				m_value = std::addressof(value);
				return {};
			}

			std::suspend_always yield_value(std::remove_reference_t<T>&& value) noexcept
			{
				m_value = std::addressof(value);
				return {};
			}

			void unhandled_exception()
			{
				m_exception = std::current_exception();
			}

			void return_void()
			{
			}

			reference_type value() const noexcept
			{
				return static_cast<reference_type>(*m_value);
			}

			// Don't allow any use of 'co_await' inside the generator coroutine.
			template<typename U>
			std::suspend_never await_transform(U&& value) = delete;

			void rethrow_if_exception()
			{
				if (m_exception)
				{
					std::rethrow_exception(m_exception);
				}
			}

		private:

			pointer_type m_value;
			std::exception_ptr m_exception;

		};

        struct generator_sentinel {};

		template<typename T>
		class generator_iterator
		{
			using coroutine_handle = std::coroutine_handle<generator_promise<T>>;

		public:

			using iterator_category = std::input_iterator_tag;
			// What type should we use for counting elements of a potentially infinite sequence?
			using difference_type = std::ptrdiff_t;
			using value_type = typename generator_promise<T>::value_type;
			using reference = typename generator_promise<T>::reference_type;
			using pointer = typename generator_promise<T>::pointer_type;

			// Iterator needs to be default-constructible to satisfy the Range concept.
			generator_iterator() noexcept
				: m_coroutine(nullptr)
			{}
			
			explicit generator_iterator(coroutine_handle coroutine) noexcept
				: m_coroutine(coroutine)
			{}

			friend bool operator==(const generator_iterator& it, generator_sentinel) noexcept
			{
				return !it.m_coroutine || it.m_coroutine.done();
			}

			friend bool operator!=(const generator_iterator& it, generator_sentinel s) noexcept
			{
				return !(it == s);
			}

			friend bool operator==(generator_sentinel s, const generator_iterator& it) noexcept
			{
				return (it == s);
			}

			friend bool operator!=(generator_sentinel s, const generator_iterator& it) noexcept
			{
				return it != s;
			}

			generator_iterator& operator++()
			{
				m_coroutine.resume();
				if (m_coroutine.done())
				{
					m_coroutine.promise().rethrow_if_exception();
				}

				return *this;
			}

			// Need to provide post-increment operator to implement the 'Range' concept.
			void operator++(int)
			{
				(void)operator++();
			}

			reference operator*() const noexcept
			{
				return m_coroutine.promise().value();
			}

			pointer operator->() const noexcept
			{
				return std::addressof(operator*());
			}

		private:

			coroutine_handle m_coroutine;
		};
	}

	template<typename T>
	class [[nodiscard]] generator
	{
	public:

		using promise_type = detail::generator_promise<T>;
		using iterator = detail::generator_iterator<T>;

		generator() noexcept
			: m_coroutine(nullptr)
		{}

		generator(generator&& other) noexcept
			: m_coroutine(other.m_coroutine)
		{
			other.m_coroutine = nullptr;
		}

		generator(const generator& other) = delete;

		~generator()
		{
			if (m_coroutine)
			{
				m_coroutine.destroy();
			}
		}

		generator& operator=(generator other) noexcept
		{
			swap(other);
			return *this;
		}

		iterator begin()
		{
			if (m_coroutine)
			{
				m_coroutine.resume();
				if (m_coroutine.done())
				{
					m_coroutine.promise().rethrow_if_exception();
				}
			}

			return iterator{ m_coroutine };
		}

		detail::generator_sentinel end() noexcept
		{
			return detail::generator_sentinel{};
		}

		void swap(generator& other) noexcept
		{
			std::swap(m_coroutine, other.m_coroutine);
		}

	private:

		friend class detail::generator_promise<T>;

		explicit generator(std::coroutine_handle<promise_type> coroutine) noexcept
			: m_coroutine(coroutine)
		{}

		std::coroutine_handle<promise_type> m_coroutine;

	};

	template<typename T>
	void swap(generator<T>& a, generator<T>& b)
	{
		a.swap(b);
	}

	namespace detail
	{
		template<typename T>
		generator<T> generator_promise<T>::get_return_object() noexcept
		{
			using coroutine_handle = std::coroutine_handle<generator_promise<T>>;
			return generator<T>{ coroutine_handle::from_promise(*this) };
		}
	}

	template<typename FUNC, typename T>
	generator<std::invoke_result_t<FUNC&, typename generator<T>::iterator::reference>> fmap(FUNC func, generator<T> source)
	{
		for (auto&& value : source)
		{
			co_yield std::invoke(func, static_cast<decltype(value)>(value));
		}
	}

    class WebGetter{
    protected:
            char buf[1'000'000];
            int port = 443;
            SSL *ssl;
            int sock;
            std::string acceptValue;
    public:
        WebGetter(std::string acceptValue="*/*"): acceptValue (acceptValue) {};
        WebGetter(const WebGetter& orig) = delete;
        virtual ~WebGetter() = default;

        bool operator()(std::string& urlStr, std::function<void(std::istream& content)> apply);
        bool operator()(std::string& urlStr, std::ostream& ss);
    protected:
        bool get(std::string& urlStr, std::ostream& ss);
        generator<std::pair<int, char *>> read(){
            int len=32768;
                len=SSL_read(ssl, buf, 32768);
                if(len>=0)buf[len]=0;
                //printf("%s",buf);
                //fprintf(fp, "%s",buf);
            co_yield std::make_pair(len, buf);
        }

        int RecvPacket(std::ostream& ss)
        {
            int len=100;
            bool foundCRLFCRLF=false;
            unsigned int countCRLF=0;
            unsigned int count=0;
            do {
                generator<std::pair<int, char *>> rs = read();
                for(auto it=rs.begin();it !=rs.end();it++){
                    len=(*it).first;
                    if(len>0){
                        for(int i=0;i<len;i++){
//                            if(countCRLF!=4)std::cout<<(*it).second[i];
                            if(countCRLF==4)ss<<bits((*it).second[i]);
                            else {
                                if((countCRLF==0|| countCRLF==2) && (*it).second[i]=='\r')countCRLF++;
                                else if((countCRLF==1|| countCRLF==3) && (*it).second[i]=='\n')countCRLF++;
                                else countCRLF=0;
                            }
                        }
                    }
                }
            } while (len>0);
            if (len < 0) {
                int err = SSL_get_error(ssl, len);
            if (err == SSL_ERROR_WANT_READ)
                    return 0;
                if (err == SSL_ERROR_WANT_WRITE)
                    return 0;
                if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL || err == SSL_ERROR_SSL)
                    return -1;
            }
            return 0;
        }

        int SendPacket(const char *buf)
        {
            int len = SSL_write(ssl, buf, std::strlen(buf));
            if (len < 0) {
                int err = SSL_get_error(ssl, len);
                switch (err) {
                case SSL_ERROR_WANT_WRITE:
                    return 0;
                case SSL_ERROR_WANT_READ:
                    return 0;
                case SSL_ERROR_ZERO_RETURN:
                case SSL_ERROR_SYSCALL:
                case SSL_ERROR_SSL:
                default:
                    return -1;
                }
            }
            return 0;
        }

        void log_ssl()
        {
            int err;
            while (err = ERR_get_error()) {
                char *str = ERR_error_string(err, 0);
                if (!str)
                    return;
                printf(str);
                printf("\n");
                fflush(stdout);
            }
        }

    };
}
