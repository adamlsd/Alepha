static_assert( __cplusplus > 2020'00 );

#include "ProgramOptions.h"

#include <set>
#include <exception>

#include <Alepha/Console.h>
#include <Alepha/word_wrap.h>
#include <Alepha/StaticValue.h>
#include <Alepha/error.h>

namespace Alepha::Cavorite  ::detail::  program_options
{
	namespace
	{
		namespace C
		{
			const bool debug= false;
			const bool debugMatching= false or C::debug;
			const bool debugExclusions= false or C::debug;
		}

		using namespace std::literals::string_literals;

		struct OptionMissingArgumentError
			: virtual std::runtime_error
		{
			using std::runtime_error::runtime_error;
		};

		// Print the string with wrapping to the terminal and the specified indent
		// for subsequent lines.
		void
		printString( const std::string &s, const std::size_t indent )
		{
			const std::size_t width= getConsoleWidth();
			std::cout << wordWrap( s, width, indent ) << std::endl;
		}
	}

	struct impl::ProgramOption
	{
		std::function< void ( std::optional< std::string > ) > handler;
		std::ostringstream help;
		std::function< std::string () > defaultBuilder= [] { return ""; };

		std::map< std::type_index, std::set< const DomainBase * > > domains;
	};

	namespace
	{
		StaticValue< std::map< std::string, impl::ProgramOption > > programOptions;

		std::vector< std::string >
		allOptionNames()
		{
			std::vector< std::string > rv;
			for( const auto &[ name, _ ]: programOptions() ) rv.push_back( name );
			return rv;
		}

		struct ExclusivityEntry
		{
			std::optional< std::string > previous;
		};

		StaticValue< std::map< const DomainBase *, ExclusivityEntry > > mutuallyExclusiveOptions;

		// The required options have to live in a single global collection.  There's only one
		// set of program options per execution, so this entire list has to be searched.
		StaticValue< std::map< const DomainBase *, std::vector< std::string > > > requiredOptions;
	}

	void
	impl::checkArgument( const std::optional< std::string > &argument, const std::string &name )
	{
		if( argument.has_value() ) return;
		throw OptionMissingArgumentError( '`' + name + "` requires an argument." );
	}

	const OptionBinding &
	OptionBinding::bindDomain( const DomainBase &domain ) const
	{
		if( domain.kind == typeid( RequirementDomain ) )
		{
			requiredOptions()[ &domain ].push_back( name );
		}

		option->domains[ domain.kind ].insert( &domain );
		return *this;
	}

	std::ostream &
	OptionBinding::operator << ( std::function< void () > core ) const
	{
		// So that users do not have to implement their own checking for argument absent,
		// we do it for them.
		auto handler= [core, name= name]( const std::optional< std::string > s )
		{
			if( s.has_value() )
			{
				throw std::runtime_error( "`--" + name + "` takes no arguments, but `" + s.value()
						+ " was provided." );
			}
			return core();
		};
		return registerHandler( handler );
	}

	std::ostream &
	OptionBinding::operator << ( std::function< void ( std::string ) > core ) const
	{
		// So that users do not have to implement their own checking for argument present,
		// we do it for them.
		auto handler= [core, name= name]( const std::optional< std::string > argument )
		{
			impl::checkArgument( argument, name );
			return core( argument.value() );
		};
		return registerHandler( handler );
	}

	void
	OptionBinding::setDefaultBuilder( std::function< std::string () > builder ) const
	{
		option->defaultBuilder= builder;
	}

	std::ostream &
	OptionBinding::registerHandler( std::function< void ( std::optional< std::string > ) > handler ) const
	{
		option->handler= handler;
		return option->help;
	}

	namespace
	{
		std::string
		buildIncompatibleHelpText( const std::string &name, const auto &domains, const auto &exclusivityMembers )
		{
			if( not domains.contains( typeid( ExclusivityDomain ) )
					or domains.at( typeid( ExclusivityDomain ) ).empty() )
			{
				return "";
			}

			std::set< std::string > incompatibles;
			for( const auto &domain: domains.at( typeid( ExclusivityDomain ) ) )
			{
				std::transform( exclusivityMembers.lower_bound( domain ),
						exclusivityMembers.upper_bound( domain ),
						std::inserter( incompatibles, end( incompatibles ) ),
						[]( const auto &item ) { return item.second; } );
			}
			incompatibles.erase( name );
			if( incompatibles.empty() ) return "";
			std::ostringstream oss;
			oss << "\nIncompatible with: \n\n";
			bool first= true;
			for( const auto &incompat: incompatibles )
			{
				if( not first ) oss << ", ";
				first= false;
				oss << '`' << incompat << '`';
			}
			oss << std::endl;
			return std::move( oss ).str();
		}

		void
		printAllOptionsHelp( const std::optional< std::string > canonicalProgramName )
		{
			const auto longestOption= std::max_element( begin( programOptions() ), end( programOptions() ),
					[]( const auto &lhs, const auto &rhs )
					{
						return lhs.first.size() < rhs.first.size();
					} );
			// Account for the `:` and the ` ` in the output table format.
			const std::size_t alignmentWidth= longestOption->first.size() + 2;

			//
			std::multimap< const DomainBase *, std::string > exclusivityMembers;
			for( const auto &[ name, def ]: programOptions() )
			{
				if( not def.domains.contains( typeid( ExclusivityDomain ) ) ) continue;
				for( const auto &excl: def.domains.at( typeid( ExclusivityDomain ) ) )
				{
					exclusivityMembers.emplace( excl, name );
				}
			}

			const std::size_t width= getConsoleWidth();
			std::cout << StartWrap{ width };
			std::cout << "Options:" << std::endl << std::endl;
			std::cout << EndWrap;

			// Inspect and print each option.
			for( const auto &[ name, def ]: programOptions() )
			{
				std::cout << StartWrap{ width, alignmentWidth };
				const auto &[ _, helpText, defaultBuilder, domains ]= def;
				// How much unused of the max width there will be
				const std::size_t padding= alignmentWidth - name.size() - 2;

				VariableMap substitutions=
				{
					// This uses a GNU extension, but it's fine.  We can always make this
					// portable, later.
					{ "program-name"s, lambaste<=::program_invocation_short_name },
					{ "option-name"s, lambaste<=name },
					{ "default"s, [&defaultBuilder= defaultBuilder, &name= name]
						{
							return "Default is `" + name + defaultBuilder() + "`";
						} },
				};
				if( canonicalProgramName.has_value() )
				{
					substitutions[ "canonical-name"s ]= lambaste<=canonicalProgramName.value();
				}

				std::string substitutionTemplate= name + ": " + std::string( padding, ' ' )
						+ helpText.str() + "\n";

				// Append the incompatibility text, when we see mutually-exclusive options.
				substitutionTemplate+= buildIncompatibleHelpText( name, domains, exclusivityMembers );

				std::cout << expandVariables( substitutionTemplate, substitutions, '!' );
				std::cout << EndWrap;
				std::cout << std::endl;
			}


			// Check for required options, and print a summary of those:
			if( not requiredOptions().empty() ) for( const auto &[ _, group ]: requiredOptions() )
			{
				std::ostream &oss= std::cout;
				oss << StartWrap{ width };
				oss << "At least one of the options in this group are required: ";
				bool first= true;
				for( const auto &required: group )
				{
					if( not first ) oss << ", ";
					first= false;
					oss << '`' << required << '`';
				}

				oss << std::endl;
			}
		}
	}

	// The options which set boolean flags can be 
	std::ostream &
	OptionBinding::operator << ( bool &flag ) const
	{
		--OptionString{ "no-" + name.substr( 2 ) }
			<< [&flag] { flag= false; } << "Disable `" + name + "`.  See that option for more details.";
		return self() << [&flag] { flag= true; };
	}

	OptionString
	literals::operator ""_option( const char *const text, const std::size_t amount )
	{
		return { std::string( text, text + amount ) };
	}

	OptionBinding
	impl::operator --( const OptionString option )
	{
		const auto name= "--" + option.name;
		if( programOptions().contains( name ) )
		{
			throw RepeatedProgramOptionError( "Option `" + name + "` was already registered." );
		}
		return OptionBinding{ name, &programOptions()[ name ] };
	}

	[[noreturn]] void
	impl::usage( const std::string &helpMessage, const std::optional< std::string > &canonicalName )
	{
		if( not helpMessage.empty() )
		{
			VariableMap substitutions
			{
				// Another use of the GNUism.
				{ "program-name"s, lambaste<=::program_invocation_short_name },
			};

			if( canonicalName.has_value() ) substitutions[ "canonical-name"s ]= lambaste<=canonicalName.value();
			std::cout << wordWrap( expandVariables( helpMessage, substitutions, '!' ), getConsoleWidth() )
					<< std::endl << std::endl;
		}

		printAllOptionsHelp( canonicalName );
		::exit( EXIT_SUCCESS );
	}

	std::vector< std::string >
	impl::handleOptions( const std::vector< std::string > &args, const std::function< void () > usageFunction )
	{
		--"help"_option << usageFunction << "Print this help message (program usage).";

		// The unprocessed program arguments will be collected into this vector
		std::vector< std::string > rv;

		const auto &opts= programOptions();

		// The arguments end at the first `--` token (by itself), or when there's no more.
		const auto endOfArgs= std::find( begin( args ), end( args ), "--" );

		// Because `--help` needs to expand certain variables, options which can affect it need to be processed
		// before handling `--help`
		const bool helpRequested= std::find( begin( args ), endOfArgs, "--help" ) != endOfArgs;

		// Each time a required domain is seen, we put that requirement into this set.
		// If all required options are passed, then this set should match the list of
		// required option domains.
		std::set< const DomainBase * > requiredOptionsSeen;

		const std::vector< std::string > argsToProcess{ begin( args ), endOfArgs };

		// An option that requires an argument might have been type-o'ed as `--option arg`
		// instead of `--option=arg`.  By tracking the next option, we can print helpful
		// diagnostics in the error messages.
		auto next= begin( argsToProcess );

		for( const auto &param: argsToProcess )
		try
		{
			++next;
			// Because `--help` has a special relationship with the rest of the options,
			// we skip it in this pass.
			if( helpRequested and param == "--help" ) continue;

			// Match up each argument.
			const bool matched= evaluate <=[&]
			{
				// TODO: Make this into direct map lookups.
				// It requires modifying `--` options as passed, to strip `=` before
				// doing a map lookup.
				for( const auto &[ name, def ]: opts )
				{
					if( C::debugMatching ) error() << "Attempting to match `" << name << "` to `" << param << "`" << std::endl;

					const auto &handler= def.handler;
					std::optional< std::string > argument;
					if( param == name ) argument= std::nullopt;
					else if( param.starts_with( name ) and "=:"s.find( param.at( name.size() ) ) != std::string::npos )
					{
						argument= param.substr( name.size() + 1 );
					}
					else continue;

					// Skip options that do not affect help, when we're doing a `--help` run.
					if( helpRequested and not def.domains.contains( typeid( PreHelpDomain ) ) ) return true;

					// Exclusivity has to be handled as a running concern across options...
					if( def.domains.contains( typeid( ExclusivityDomain ) ) )
					{
						const auto &exclusions= def.domains.at( typeid( ExclusivityDomain ) );
						if( C::debugExclusions )
						{
							error() << "I see " << exclusions.size() << " mutual exclusions against `"
							<< name << "`" << std::endl;
						}
						for( const auto &exclusion: exclusions )
						{
							// Look up this domain, and see if something from it was used.
							auto &other= mutuallyExclusiveOptions()[ exclusion ].previous;
							if( other.has_value() and other != name )
							{
								throw std::runtime_error{ "Options `" + other.value() + "` and `"
										+ name + "` are mutually exclusive." };
							}
							else other= name; // If nothing was there, record that this name was now used.
						}
					}

					// If the option was required, mark that we took it.
					if( def.domains.contains( typeid( RequirementDomain ) ) )
					{
						for( const auto &domain: def.domains.at( typeid( RequirementDomain ) ) )
						{
							requiredOptionsSeen.insert( domain );
						}
					}
					handler( argument );
					return true;
				}
				return false;
			};
			if( C::debugMatching and not matched ) error() << "No match for `" << param << "` was found." << std::endl;
			if( matched ) continue;
			rv.push_back( param );

			if( param.starts_with( "--" ) )
			{
				// TODO: 
				throw std::runtime_error( "`" + param + "` is an unrecognized option." );
			}
		}
		catch( const OptionMissingArgumentError &e )
		{
			if( next == end( argsToProcess ) or next->starts_with( "--" ) ) throw;
			throw std::runtime_error( e.what() + " did you mean: `"s + param + "=" + *next + "`?" );
		}

		if( endOfArgs != end( args ) ) std::copy( endOfArgs + 1, end( args ), back_inserter( rv ) );

		if( helpRequested ) programOptions().at( "--help" ).handler( std::nullopt );

		// If we're not doing a help-run, then we need to validate the required
		// options were all passed.
		if( requiredOptions().size() != requiredOptionsSeen.size() )
		{
			for( auto [ required, opts ]: requiredOptions() )
			{
				if( requiredOptionsSeen.contains( required ) ) continue;

				std::ostringstream oss;
				oss << "Required option missing.  At least one of ";
				bool first= true;
				for( const auto &name: opts )
				{
					if( not first ) oss << ", ";
					first= false;
					oss << '`' << name << '`';
				}
				oss << " must be passed.";

				throw std::runtime_error( oss.str() );
			}

			throw std::runtime_error{ "A required option was missing, and it couldn't be identified." };
		}

		return rv;
	}
}
