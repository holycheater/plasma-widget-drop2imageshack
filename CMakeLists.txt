project(plasma-drop2imageshack)

find_package(KDE4 REQUIRED)
find_package(CURL REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES}
    ${CURL_INCLUDE_DIR})

set(drop2imageshack_SRCS drop2imageshack.cpp imageuploader.cpp)

#kde4_add_ui_files(drop2imageshack_SRCS settings.ui theme.ui)
kde4_add_plugin(plasma_applet_drop2imageshack ${drop2imageshack_SRCS})
target_link_libraries(plasma_applet_drop2imageshack
    ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${CURL_LIBRARY})

install(TARGETS plasma_applet_drop2imageshack
    DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-drop2imageshack.desktop
    DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-drop2imageshack.notifyrc
    DESTINATION ${DATA_INSTALL_DIR}/plasma-drop2imageshack)
