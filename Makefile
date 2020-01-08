all: isophotes

isophotes: isophotes.cc
	$(CXX) -o $@ $< -lpng
