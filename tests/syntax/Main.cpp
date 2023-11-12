#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int argc, char* argv[]) {
	doctest::Context context;
	context.applyCommandLine(argc, argv);
	return context.run();
}

class TestListener : public doctest::IReporter {
	std::ostream& out;

public:
	explicit TestListener(const doctest::ContextOptions& options) :
		out(*options.cout) {
	}

	void report_query(const doctest::QueryData&) override {
	}

	void test_run_start() override {
	}

	void test_run_end(const doctest::TestRunStats&) override {
	}

	void test_case_start(const doctest::TestCaseData& test_case) override {
		out << "Running test: " << test_case.m_name << '\n';
	}

	void test_case_reenter(const doctest::TestCaseData&) override {
	}

	void test_case_end(const doctest::CurrentTestCaseStats&) override {
	}

	void test_case_exception(const doctest::TestCaseException&) override {
	}

	void subcase_start(const doctest::SubcaseSignature&) override {
	}

	void subcase_end() override {
	}

	void log_assert(const doctest::AssertData&) override {
	}

	void log_message(const doctest::MessageData&) override {
	}

	void test_case_skipped(const doctest::TestCaseData&) override {
	}
};

REGISTER_LISTENER("test_listener", 1, TestListener);

// Workaround because DocTest doesn't have an API for this yet. Also it must be implemented in the file that defines
// DOCTEST_CONFIG_IMPLEMENT as described in https://github.com/doctest/doctest/issues/345.
const char* get_current_test_name() {
	return doctest::detail::g_cs->currentTest->m_name;
}
