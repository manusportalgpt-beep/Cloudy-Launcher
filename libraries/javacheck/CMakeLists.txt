cmake_minimum_required(VERSION 3.15)
project(launcher Java)
find_package(Java 1.7 REQUIRED COMPONENTS Development)

include(UseJava)
set(CMAKE_JAVA_JAR_ENTRY_POINT JavaCheck)
# Java 7 was removed from modern JDK toolchains. The checker uses only
# Java 8-compatible language features, so keep the generated jar runnable
# on current JREs while allowing current javac versions to build it.
set(CMAKE_JAVA_COMPILE_FLAGS -target 8 -source 8 -Xlint:deprecation -Xlint:unchecked)

set(SRC
    JavaCheck.java
)

add_jar(JavaCheck ${SRC})
install_jar(JavaCheck "${JARS_DEST_DIR}")
