static_assert( __cplusplus > 2020'00 );

#include "../word_wrap.h"

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation_helpers.h>

namespace
{
	using namespace Alepha::Testing::literals::test_literals;;
	using Alepha::Testing::TableTest;
}

static auto init= Alepha::Utility::enroll <=[]
{
	"Does word wrap with no-indent do sensible things?"_test <=TableTest< Alepha::wordWrap >::Cases
	{
		{ "Three word with break after first?", { "Goodbye cruel world!", 12, 0 }, "Goodbye \ncruel world!" },
	};

	"word_wrap.noindent.single_word"_test <=TableTest< Alepha::wordWrap >::Cases
	{
		{ "Simple", { "Hello", 100, 0 }, "Hello" },
		{ "space should be kept", { "Hello ", 10, 0 }, "Hello " },
		{ "space should be dropped", { "Hello ", 5, 0 }, "Hello" },
		{ "all spaces should be dropped", { "Hello  ", 5, 0 }, "Hello" },
		{ "two spaces should be kept", { "Hello    ", 7, 0 }, "Hello  " },
		{ "too narrow, forces wrap anyway", { "Hello", 4, 0 }, "\nHello" },
		{ "too narrow, forces wrap anyway, drops space", { "Hello ", 4, 0 }, "\nHello" },
		{ "too narrow, forces wrap anyway, drops spaces", { "Hello    ", 4, 0 }, "\nHello" },
	};

	const std::string helloWorld= "Hello World";
	"word_wrap.noindent.two_words"_test <=TableTest< Alepha::wordWrap >::Cases
	{
		{ "Simple", { helloWorld, 100, 0 }, helloWorld },
		{ "Trailing space should be kept", { helloWorld + " ", 100, 0 }, helloWorld + " " },
		{ "Trailing spaces should be kept", { helloWorld + "     ", 100, 0 }, helloWorld + "     " },
		{ "Trailing spaces should be dropped", { helloWorld + "       ", helloWorld.size(), 0 }, helloWorld },
		{ "All but 2 trailing spaces dropped", { helloWorld + "       ", helloWorld.size() + 2, 0 },
				helloWorld + "  "},

		{ "Split line", { helloWorld, 8, 0 }, "Hello \nWorld" }, // TODO: Should we swallow trailing spaces?
	};

	"word_wrap.noindent.other_cases"_test <=TableTest< Alepha::wordWrap >::Cases
	{
		{ "Three words", { "Goodbye cruel world!", 12, 0 }, "Goodbye \ncruel world!" },
		{ "Three longer words", { "Goodbye terrible world!", 13, 0 }, "Goodbye \nterrible \nworld!" },
		{ "narrow terminal, complex text: Hamlet's \"To Be or Not To Be\"",
			{
				"To be, or not to be: "
				"that is the question: "
				"Whether 'tis nobler in the mind to suffer "
				"The slings and arrows of outrageous fortune, "
				"Or to take arms against a sea of troubles, "
				"And by opposing end them?  "
				"To die: to sleep; "
				"No more; and by a sleep to say we end "
				"The heart-ache and the thousand natural shocks "
				"That flesh is heir to, 'tis a consummation "
				"Devoutly to be wish'd.  To die, to sleep; "
				"To sleep: perchance to dream: ay, there's the rub;",
				20, 0
			},
			"To be, or not to be:\n"
			"that is the \n"
			"question: Whether \n"
			"'tis nobler in the \n"
			"mind to suffer The \n"
			"slings and arrows of\n"
			"outrageous fortune, \n"
			"Or to take arms \n"
			"against a sea of \n"
			"troubles, And by \n"
			"opposing end them?  \n"
			"To die: to sleep; No\n"
			"more; and by a sleep\n"
			"to say we end The \n"
			"heart-ache and the \n"
			"thousand natural \n"
			"shocks That flesh is\n"
			"heir to, 'tis a \n"
			"consummation \n"
			"Devoutly to be \n"
			"wish'd.  To die, to \n"
			"sleep; To sleep: \n"
			"perchance to dream: \n"
			"ay, there's the rub;"
		},
		{ "wider terminal, complex text: Hamlet's \"To Be or Not To Be\"",
			{
				"To be, or not to be: "
				"that is the question: "
				"Whether 'tis nobler in the mind to suffer "
				"The slings and arrows of outrageous fortune, "
				"Or to take arms against a sea of troubles, "
				"And by opposing end them?  "
				"To die: to sleep; "
				"No more; and by a sleep to say we end "
				"The heart-ache and the thousand natural shocks "
				"That flesh is heir to, 'tis a consummation "
				"Devoutly to be wish'd.  To die, to sleep; "
				"To sleep: perchance to dream: ay, there's the rub;",
				80, 0
			},
			"To be, or not to be: that is the question: Whether 'tis nobler in the mind to \n"
			"suffer The slings and arrows of outrageous fortune, Or to take arms against a \n"
			"sea of troubles, And by opposing end them?  To die: to sleep; No more; and by a \n"
			"sleep to say we end The heart-ache and the thousand natural shocks That flesh is\n"
			"heir to, 'tis a consummation Devoutly to be wish'd.  To die, to sleep; To sleep:\n"
			"perchance to dream: ay, there's the rub;"
		},
	};

	"word_wrap.indent"_test <=TableTest< Alepha::wordWrap >::Cases
	{
		{ "Two word indent, simple", { "Hello World!", 8, 2 }, "Hello \n  World!" },
		{ "Three word indent, simple", { "Hello Wonderful World!", 16, 4 }, "Hello Wonderful \n    World!" },
		{ "Three word indent, corner case", { "Hello Wonderful World!", 15, 4 }, "Hello Wonderful\n    World!" },
		{ "Two word indent, extra newline", { "Hello\n\nWorld!", 8, 2 }, "Hello\n  \n  World!" },
		{ "Two word indent, one newline", { "Hello\nWorld!", 8, 2 }, "Hello\n  World!" },
	};
};
