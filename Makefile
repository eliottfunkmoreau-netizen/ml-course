CXX      := clang++
CXXFLAGS := -std=c++20 -O3 -mcpu=native -flto -Wall -Wextra -Iinclude
HEADERS  := $(wildcard include/quant/*.hpp)

all: build/quant build/quant_test build/quant_feed build/quant_history build/quant_calendar build/quant_valprof build/quant_trend build/quant_dash

build/quant: src/main.cpp $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@ -pthread

build/quant_feed: src/feed.cpp $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@

build/quant_history: src/history.cpp $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@

build/quant_calendar: src/calendar_run.cpp $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@

build/quant_valprof: src/valprof_run.cpp $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@

build/quant_trend: src/trend_run.cpp $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@

build/quant_dash: src/dash.cpp $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@

build/quant_test: tests/test_main.cpp $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@

test: build/quant_test
	./build/quant_test

clean:
	rm -rf build

.PHONY: all test clean
