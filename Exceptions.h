static_assert( __cplusplus > 2020'00 );

#pragma once

#include <exception>

namespace Alepha::inline Cavorite  ::detail::  exceptions
{
	inline namespace exports
	{
		class MessageHolder;
		class Throwable;

		class Exception;

		class Condition;
		class Notification;
		class Error;
		class Violation;
		class CriticalError;
	}

	class exports::Throwable
	{
		public:
			virtual ~Throwable()= default;

			// TODO: Not all throwables will be derived from `std::exception`, eventually.
			virtual const char *message() const noexcept { return dynamic_cast< const std::exception * >( this )->what(); }
	};

	class exports::MessageHolder
		: virtual private Throwable
	{
		private:
			std::string msg;

		public:
			explicit MessageHolder( std::string msg ) : msg( std::move( msg ) ) {}

			const char *message() const noexcept final { return msg.c_str(); } {}
	};

	class exports::Exception : public virtual std::exception, public virtual Throwable {};

	class exports::Condition { public: virtual ~Condition()= default; }
	class exports::Notification { public: virtual ~Notification()= default; }
	class exports::Error : public virtual Exception { public: virtual ~Error()= default; }
	class exports::Violation : public virtual Throwable { public: virtual ~Violation()= default; }
	class exports::CriticalError : public virtual Throwable { public: virtual ~CriticalError()= default; }

	namespace exports
	{
		struct EndingCondition : virtual Condition {};
		struct FinishedCondition : virtual EndingCondition {};

		struct FlushCondition : virtual Condition {};

		struct FileNotFoundError
			: virtual Error, private virtual MessageHolder
		{
			explicit FileNotFoundError( std::string m ) : MessageHolder( std::move( m ) ) {}
		};

		struct Cancellation : virtual Notification {};
	}
}

namespace Alepha::Cavorite::inline exports::inline exceptions
{
	using namespace detail::exceptions::exports;
}

