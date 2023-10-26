static_assert( __cplusplus > 2020'00 );

#pragma once

/*!
 * @file
 * Program options library.
 *
 * The `Alepha::program_options` namespace defines a simple DSEL for adding commandline options
 * to a program.  Options are defined using `--"name"_option` operations and then "streaming"
 * an option handler into the option name, followed by streaming in any help text.  The result
 * type of `operator <<` between an option name and a handler is a `std::ostream &` which can
 * be used to build the option help string.  The help text can use a variable-expansion feature
 * to allow for options help to be dynamically kept in sync with program development.  The
 * variables `"!program-name!"` or `"!option-name!"` will expand to the text one would expect.
 * The variable `"!default!"` will expand to an example usage which initializes the option
 * as-if the option were never passed.
 *
 * An option handler can be a function or a variable.  If it is a function, that function
 * will be called when processing that option.  If the function takes a string argument, the
 * option will be parsed for an `=` and the text after that token will be passed as a string
 * argument.  If the function takes no arguments, then the option will not accept `=` arguments.
 *
 * If a `bool` variable is passed as an option handler, then the variable will be set if that
 * option is present.  A `"--no-"` form of the option will automatically be generated, as well.
 * That `"--no-"` form will clear the Boolean variable.  Options are processed from left to right.
 *
 * If a `std::vector< T >` variable is passed as an option handler, then each time the option
 * is encountered, its argument will be appended to that `std::vector`.  Parsing will use
 * `operator >> ( std::istream &, T & )`.
 *
 * If a single instance variable is passed as an option handler, then each time the option
 * is encountered, its argument will be parsed and replace the value stored in that variable.
 * Parsing of options is handled left-to-right.  Parsing of the argument string will use
 * `operator >> ( std::istream &, T & )`.
 *
 * If an `std::optional< T >` single instance variable is passed as an option handler, then each
 * time the option is encountered, its argument will be parsed and replace the value stored
 * in that variable.  If it is never encountered, the optional will not be modified.  This
 * avoids the need for dummy values and sentinal values in some cases.  Parsing of options
 * is handled left-to-right.  Parsing of the argument string will use
 * `operator >> ( std::istream &, T & )`.
 *
 * A `"--help"` and option and handler will be automatically generated.
 *
 * Example:
 *
 * ```
 * #include <Alepha/Options.h>
 *
 * int
 * main( int argc, const char **argv )
 * {
 *     using namespace Alepha::program_options;
 *
 *     //Let's define a few options: 
 *
 *     bool fooMode= false;
 *     // Note that `--no-foo-mode` will be provided for you.
 *     --"foo-mode"_option << fooMode << "Enable foo mode";
 *
 *     std::vector< std::string > fileList;
 *     --"process-file" << fileList
 *     << "Add the specified file to the list of files to process";
 *
 *     // `handleOptions` will return a vector of all program arguments
 *     // that it did not recognize.
 *     const std::vector< std::string > args= handleOptions( argc, argv, NonStrict );
 *
 *     // To permit use with other options parsing code, `Strict` vs `NonStrict`
 *     // arguments to `handleOptions` will determine whether it reports an error on
 *     // `--` options that it does not recognize.  The default is `Strict`, if
 *     // the parameter is omitted.
 * }
 * ```
 *
 * Errors are reported by throwing an exception.  The `.what()` observer will report
 * an informative message about the parsing error encountered.
 */

#include <Alepha/Alepha.h>

#include <string>
#include <typeindex>
#include <exception>
#include <stdexcept>
#include <optional>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <Alepha/Concepts.h>
#include <Alepha/string_algorithms.h>

#include <Alepha/IOStreams/Stream.h>

#include <Alepha/Utility/evaluation_helpers.h>

namespace Alepha::inline Cavorite  ::detail::  program_options
{
	inline namespace exports {}

	using namespace Utility::exports::evaluation_helpers;

	/*!
	 * User created unique symbols can be bound to options to build classes of options.
	 *
	 * The symbols are used to build domains of these classes, defining relationships between them.
	 * Examples of relationship include exclusivity, at-least-one-requirement, or chained dependencies.
	 *
	 * @note Domains must be global.
	 */
	struct DomainBase
	{
		std::type_index kind;
	};

	template< typename T >
	struct Domain : DomainBase
	{
		Domain() : DomainBase{ typeid( Domain ) } {}

		friend bool
		operator < ( const Domain &lhs, const Domain &rhs )
		{
			return std::less<>{}( &lhs, &rhs );
		}
	};

	namespace exports
	{
		class RepeatedProgramOptionError : public std::runtime_error
		{
			public:
				using std::runtime_error::runtime_error;
		};
	}

	struct exclusivity_tag;
	struct requirement_tag;
	struct pre_help_tag;

	namespace exports
	{
		/*!
		 * This is used to build groups of mutually exclusive options.
		 *
		 * If two or more options associated with the same exclusivity domain are seen in parsing the command-line, then
		 * the program options parsing will fail with an error.  An option may be in mulltiple exclusivity domains, and
		 * exclusivity domains may overlap by any arbitrary amount.  Just `operator <<` an instance of an exclusivity
		 * domain into the option definition.  That will cause that option to be added to the exclusivity domain.
		 *
		 * @note Exclusivity domains must be global instances, at this time.
		 * @note This must occur on the option line before the option handler or option variable.  (i.e., `<<`
		 * directly after the option string name or another domain.
		 */
		using ExclusivityDomain= Domain< exclusivity_tag >;

		using RequirementDomain= Domain< requirement_tag >;

		using PreHelpDomain= Domain< pre_help_tag >;
		inline const PreHelpDomain affectsHelp;
	}

	template< typename T >
	auto
	argumentFromString( const std::string &s, const std::string &argName, const std::string &fullOption )
	try
	{
		if constexpr( std::is_same_v< T, std::string > ) return s;
		else return boost::lexical_cast< T >( s );
	}
	catch( const boost::bad_lexical_cast &ex )
	{
		throw std::runtime_error( "Error parsing option `" + argName + "`, with parameter string: `" + s + "` (full option: `" + fullOption + "`)" );
	}

	inline namespace impl
	{
		struct ProgramOption;

		void checkArgument( const std::optional< std::string > &opt, const std::string &name );
	}

	class OptionBinding
	{
		public:
			std::string name;
			impl::ProgramOption *option;

			// The `operator <<` forms are used to define options.
			// These are not `std::ostream` operators directly,
			// except that the end of a chain will return the `std::ostream`
			// object used to construct the help for that option.

		private:
			// We have to self-call our operators, so this makes it syntactically simpler.
			auto &self() { return *this; }
			const auto &self() const { return *this; }

			using option_handler= std::function< void ( std::optional< std::string > ) >;
			[[nodiscard]] std::ostream &registerHandler( option_handler handler ) const;

			void setDefaultBuilder( std::function< std::string () > ) const;

			[[nodiscard]] const OptionBinding &bindDomain( const DomainBase & ) const;

		public:
			template< typename T >
			[[nodiscard]] const OptionBinding &
			operator << ( const Domain< T > &domain ) const
			{
				return bindDomain( domain ); // Pass to polymorphic handler for base
			}

			// This installs a custom handler that has to do its own string parsing.
			[[nodiscard]] std::ostream &operator << ( std::function< void ( std::string ) > core ) const;

			// This installs a custom handler that takes no arguments.
			[[nodiscard]] std::ostream &operator << ( std::function< void () > core ) const;

			// Handler generator -- parses the string arguments in an option and puts the at the end of the
			// specified `vector`.
			template< typename T >
			[[nodiscard]] std::ostream &
			operator << ( std::vector< T > &list ) const
			{
				return self() << [&list, name= name]( const std::string param )
				{
					for( const std::string &datum: parseCommas( param ) )
					{
						if constexpr( Integral< T > )
						{
							const auto parsedRange= parseRange< T >( argumentFromString< std::string >( datum, name, name + "=" + param ) );
							list.insert( back( list ), begin( parsedRange ), end( parsedRange ) );
						}
						else
						{
							list.push_back( argumentFromString< T >( datum, name, name + "=" + param ) );
						}
					}
				};
			}

			// Handler generator -- This builds a parser for the specified value, and installs the value to an optional
			// when the option and its argument are seen.
			template< typename T >
			[[nodiscard]] std::ostream &
			operator << ( std::optional< T > &value ) const
			{
				return self() << [&value, name= name]( const std::string datum )
				{
					value= argumentFromString< T >( datum, name, name + "=" + datum );
				};
			}

			// Boolean flag options are a special case of the value-binding system.
			// They generate `--no-` forms of the option as well.
			std::ostream &operator << ( bool &flag ) const;

			template< NotFunctional T >
			[[nodiscard]] std::ostream &
			operator << ( T &value ) const
			{
				// This is used in help generation to print out the "default" value chosen by the programmer, by referencing the
				// variable's value in C++ at runtime.
				auto defaultBuilder= [&value]
				{
					auto text= IOStreams::stringify( value );

					if( text.find_first_of( " \n\t" ) != std::string::npos )
					{
						text= '"' + text + '"';
					}
					return "=" + text;
				};
				setDefaultBuilder( defaultBuilder );
				return self() << [&value, name= name]( const std::string datum )
				{
					value= argumentFromString< T >( datum, name, name + "=" + datum );
				};
			}

			[[nodiscard]] std::ostream &
			operator << ( UnaryFunction auto handler ) const
			{
				using arg_type= get_arg_t< std::decay_t< decltype( handler ) >, 0 >;
				if constexpr( Vector< arg_type > )
				{
					// TODO: This should steal the impl from the vector form, above, and that should defer to this.

					using parse_type= typename arg_type::value_type;
					auto handler= [handler, name= name]( std::optional< std::string > argument )
					{
						impl::checkArgument( argument, name );

						const auto parsed= evaluate <=[&]
						{
							std::vector< parse_type > rv;
							for( const auto &value: parseCommas( argument.value() ) )
							{
								if constexpr( Integral< parse_type > )
								{
									const auto parsedRange= parseRange< parse_type >( argumentFromString< std::string >( value, name,
											name + "=" + argument.value() ) );
									rv.insert( back( rv ), begin( parsedRange ), end( parsedRange ) );
								}
								else rv.push_back( argumentFromString< parse_type >( value, name, name + "=" + argument.value() ) );
							}
							return rv;
						};
						handler( parsed );
					};
					return registerHandler( handler );
				}
				else
				{
					auto wrapped= [handler, name= name]( std::optional< std::string > argument )
					{
						impl::checkArgument( argument, name );

						const auto value= argumentFromString< arg_type >( argument.value(), name, name + "=" + argument.value() );
						return handler( value );
					};
					return registerHandler( wrapped );
				}
			}
	};

	void printString( const std::string &s, const std::size_t indent );

	struct OptionString { std::string name; };

	namespace exports::inline literals
	{
		OptionString operator ""_option( const char *const text, const std::size_t amount );
	}

	inline namespace impl
	{
		[[nodiscard]] OptionBinding operator --( OptionString option );
	}

	struct ProgramDescription
	{
		static std::string helpMessage() { return ""; }
		static std::optional< std::string > canonicalName() { return std::nullopt; }
	};

	namespace impl
	{
		[[noreturn]] void usage( const std::string &, const std::optional< std::string > & );
		[[nodiscard]] std::vector< std::string > handleOptions( const std::vector< std::string > &, std::function< void () > );
	}

	template< typename Supplement >
	[[noreturn]] void
	usageWrap()
	{
		impl::usage( Supplement::helpMessage(), Supplement::canonicalName() );
	}

	namespace exports
	{
		using DefaultSupplement= ProgramDescription;

		template< typename Supplement >
		auto
		handleOptions( const std::vector< std::string > &args )
		{
			return impl::handleOptions( args, usageWrap< Supplement > );
		}

		template< typename Supplement >
		auto
		handleOptions( const int argcnt, const char *const *const argvec )
		{
			return handleOptions< Supplement >( { argvec + 1, argvec + argcnt } );
		}

		inline auto
		handleOptions( const std::vector< std::string > &args )
		{
			return handleOptions< ProgramDescription >( args );
		}

		inline auto
		handleOptions( const int argcnt, const char *const *const argvec )
		{
			return handleOptions< ProgramDescription >( argcnt, argvec );
		}
	}
}

namespace Alepha::Cavorite::inline exports::inline program_options
{
	using namespace detail::program_options::exports;
}

namespace Alepha::Cavorite::inline exports::inline literals::inline option_literals
{
	using namespace detail::program_options::exports::literals;
}
