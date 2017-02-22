#pragma once

namespace Ariadne {

class Ability {
public:
	enum class AssertionResult {
		CAN_INVOKE,
		TOO_FAR,
		CANNOT_INTERACT,
		NOT_POINTING
	};

	Ability(const std::function<void(void)>& body_, const std::function<AssertionResult(void)>& assertion_ = []() { return AssertionResult::CAN_INVOKE; }) :
		body(body_),
		assertion(assertion_) {}

	bool canInvoke() const {
		return limit != 0 && assertion() == AssertionResult::CAN_INVOKE;
	}

	AssertionResult getAssertionResult() const {
		return assertion();
	}

	void setLimit(const int limit_) {
		limit = limit_;
	}

	int getLimit() const {
		return limit;
	}

	void invoke() {
		if (limit > 0) {
			--limit;
		}
		body();
	}

private:
	int limit = -1;
	const std::function<void(void)> body;
	const std::function<AssertionResult(void)> assertion;
};

}
