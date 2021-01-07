 # the compiler: gcc for C program, define as g++ for C++
  CXX = g++

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -g -Wall

  # the build target executable:
  TARGET_ONE = tcp_echo_server
  TARGET_TWO = tcp_echo_client

  all: $(TARGET_ONE) $(TARGET_TWO)

  $(TARGET_ONE): $(TARGET_ONE).cpp
	$(CXX) $(CFLAGS) -o $(TARGET_ONE) $(TARGET_ONE).cpp
  $(TARGET_TWO): $(TARGET_TWO).cpp
	$(CXX) $(CFLAGS) -o $(TARGET_TWO) $(TARGET_TWO).cpp

  clean:
	$(RM) $(TARGET_ONE) $(TARGET_TWO)