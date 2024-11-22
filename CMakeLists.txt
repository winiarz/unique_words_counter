
include_directories("include")

add_executable(generator
               data_generator/main.cpp)

add_executable(counter
               source/main.cpp
			   source/WordsRangesContainer.cpp
			   source/compressWord.cpp
			   source/sort.cpp)

