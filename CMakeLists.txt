cmake_minimum_required(VERSION 3.18)

#set(includedirs "")
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(. REQUIRED portaudio-2.0 ncurses libxml-2.0)

project(seqr)
add_executable(seqr src/main.c src/seqr.c src/ui.c src/list.c src/wav.c src/vect.c src/song.c)
target_compile_options(seqr PUBLIC -Wfatal-errors)
target_include_directories(seqr PUBLIC /usr/include/libxml2/)

target_link_libraries(seqr portaudio ncurses xml2 pthread m)
#target_link_libraries(seqr PkgConfig::portaudio-2.0)
