cmake_minimum_required(VERSION 3.7)
#project name  
PROJECT(latency_test)  

SET(GSTREAMER_DEV D://gstreamer//1.0//x86_64)
#head file path  
INCLUDE_DIRECTORIES(  
${GSTREAMER_DEV}/include/glib-2.0  
${GSTREAMER_DEV}/lib/glib-2.0/include
${GSTREAMER_DEV}/include/gstreamer-1.0
${GSTREAMER_DEV}/include
)  
LINK_DIRECTORIES(${GSTREAMER_DEV}/lib)
#source directory  
AUX_SOURCE_DIRECTORY(. DIR_SRCS)  
#set environment variable  

SET(TEST_MATH  
${DIR_SRCS}  
)  
#set extern libraries  
SET(LIBRARIES  
${GSTREAMER_DEV}/lib/gstreamer-1.0.lib
${GSTREAMER_DEV}/lib/glib-2.0.lib
${GSTREAMER_DEV}/lib/gobject-2.0.lib  
)  
#add executable file  
ADD_EXECUTABLE(f_latency_test ${TEST_MATH})  
#add link library  
TARGET_LINK_LIBRARIES(f_latency_test ${LIBRARIES}) 