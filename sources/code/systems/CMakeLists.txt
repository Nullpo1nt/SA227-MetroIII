# specify the C standard
set(CMAKE_C_STANDARD 11)

add_library(systems STATIC plugin.c nws.c)
target_include_directories(systems PRIVATE $ENV{XPLANE_SDK}/CHeaders/XPLM $ENV{XPLANE_SDK}/CHeaders/Widgets)
target_compile_definitions(systems PRIVATE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)

set_target_properties(systems PROPERTIES PREFIX ""
                                         SUFFIX "")

if(WINDOWS)
    target_compile_definitions(systems PRIVATE -DAPL=0 -DIBM=1 -DLIN=0)
    set_target_properties(systems PROPERTIES OUTPUT_NAME "win.xpl")
elseif(APPLE)
    target_compile_definitions(systems PRIVATE -DAPL=1 -DIBM=0 -DLIN=0)
    set_target_properties(systems PROPERTIES OUTPUT_NAME "apl.xpl")
else()
    target_compile_definitions(systems PRIVATE -DAPL=0 -DIBM=0 -DLIN=1)
    target_compile_options(systems PRIVATE -fPIC -fvisibility=hidden)
    target_link_libraries(systems openal)
    target_link_options(systems PRIVATE )
    set_target_properties(systems PROPERTIES OUTPUT_NAME "lin.xpl")
endif()

target_link_libraries(systems m -m64 -static-libgcc -shared -Wl,--version-script=exports.txt)
