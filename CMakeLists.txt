cmake_minimum_required(VERSION 3.7)

project(gta5view VERSION 1.11.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "gta5view C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FORCE_QT_VERSION "" CACHE STRING "Force Qt Version")
if(FORCE_QT_VERSION)
    set(QT_VERSION_MAJOR ${FORCE_QT_VERSION})
else()
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
endif()
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network Svg Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools QUIET)

set(RAGEPHOTO_STATIC ON CACHE BOOL "Build libragephoto as static library")
add_subdirectory(src/libragephoto)

if(WIN32)
    list(APPEND GTA5VIEW_LIBS
        dwmapi
    )
    list(APPEND GTA5VIEW_DEFINES
        -DUNICODE
        -D_UNICODE
        -DWIN32
    )
    list(APPEND GTA5VIEW_RESOURCES
        res/app.rc
    )
endif()
if(APPLE)
    list(APPEND GTA5VIEW_RESOURCES
        res/gta5view.icns
    )
    set(MACOSX_BUNDLE_BUNDLE_NAME gta5view)
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${gta5view_VERSION})
    set(MACOSX_BUNDLE_ICON_FILE gta5view.icns)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER de.syping.gta5view)
    set_source_files_properties(res/gta5view.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

list(APPEND GTA5VIEW_DEFINES
    -DGTA5SYNC_CMAKE
    -DGTA5SYNC_PROJECT
)

set(GTA5VIEW_SOURCES
    src/main.cpp
    src/AboutDialog.cpp
    src/AppEnv.cpp
    src/CrewDatabase.cpp
    src/DatabaseThread.cpp
    src/ExportDialog.cpp
    src/ExportThread.cpp
    src/GlobalString.cpp
    src/IconLoader.cpp
    src/ImportDialog.cpp
    src/JsonEditorDialog.cpp
    src/MapLocationDialog.cpp
    src/OptionsDialog.cpp
    src/PictureDialog.cpp
    src/PictureExport.cpp
    src/PictureWidget.cpp
    src/PlayerListDialog.cpp
    src/ProfileDatabase.cpp
    src/ProfileInterface.cpp
    src/ProfileLoader.cpp
    src/ProfileWidget.cpp
    src/SavegameCopy.cpp
    src/SavegameData.cpp
    src/SavegameDialog.cpp
    src/SavegameWidget.cpp
    src/SidebarGenerator.cpp
    src/SnapmaticEditor.cpp
    src/SnapmaticJson.cpp
    src/SnapmaticPicture.cpp
    src/SnapmaticWidget.cpp
    src/StandardPaths.cpp
    src/StringParser.cpp
    src/TranslationClass.cpp
    src/UserInterface.cpp
    src/anpro/imagecropper.cpp
    src/pcg/pcg_basic.c
    src/uimod/JSHighlighter.cpp
    src/uimod/UiModLabel.cpp
    src/uimod/UiModWidget.cpp
)

set(GTA5VIEW_HEADERS
    src/config.h
    src/AboutDialog.h
    src/AppEnv.h
    src/CrewDatabase.h
    src/DatabaseThread.h
    src/ExportDialog.h
    src/ExportThread.h
    src/GlobalString.h
    src/IconLoader.h
    src/ImportDialog.h
    src/JsonEditorDialog.h
    src/MapLocationDialog.h
    src/OptionsDialog.h
    src/PictureDialog.h
    src/PictureExport.h
    src/PictureWidget.h
    src/PlayerListDialog.h
    src/ProfileDatabase.h
    src/ProfileInterface.h
    src/ProfileLoader.h
    src/ProfileWidget.h
    src/SavegameCopy.h
    src/SavegameData.h
    src/SavegameDialog.h
    src/SavegameWidget.h
    src/SidebarGenerator.h
    src/SnapmaticEditor.h
    src/SnapmaticJson.h
    src/SnapmaticPicture.h
    src/SnapmaticWidget.h
    src/StandardPaths.h
    src/StringParser.h
    src/TranslationClass.h
    src/UserInterface.h
    src/anpro/imagecropper.h
    src/pcg/pcg_basic.h
    src/uimod/JSHighlighter.h
    src/uimod/UiModLabel.h
    src/uimod/UiModWidget.h
)

set(GTA5VIEW_INCLUDEDIR
    src
    src/anpro
    src/pcg
    src/uimod
)

set(GTA5VIEW_FORMS
    src/AboutDialog.ui
    src/ExportDialog.ui
    src/ImportDialog.ui
    src/JsonEditorDialog.ui
    src/MapLocationDialog.ui
    src/OptionsDialog.ui
    src/PictureDialog.ui
    src/PlayerListDialog.ui
    src/ProfileInterface.ui
    src/SavegameDialog.ui
    src/SavegameWidget.ui
    src/SnapmaticEditor.ui
    src/SnapmaticWidget.ui
    src/UserInterface.ui
)

set(GTA5VIEW_TRANSLATIONS
    res/gta5sync_de.ts
    res/gta5sync_en_US.ts
    res/gta5sync_fr.ts
    res/gta5sync_ko.ts
    res/gta5sync_ru.ts
    res/gta5sync_uk.ts
    res/gta5sync_zh_TW.ts
)

list(APPEND GTA5VIEW_RESOURCES
    res/global.qrc
)
set_property(SOURCE res/global.qrc PROPERTY AUTORCC_OPTIONS "-threshold;0;-compress;9")

if(Qt5LinguistTools_FOUND)
    qt5_add_translation(GTA5VIEW_QMFILES
        ${GTA5VIEW_TRANSLATIONS}
        res/qt5/qtbase_en_GB.ts
    )
    set(LINGUIST_FOUND TRUE)
elseif(Qt6LinguistTools_FOUND)
    qt6_add_translation(GTA5VIEW_QMFILES
        ${GTA5VIEW_TRANSLATIONS}
        res/qt6/qtbase_en_GB.ts
    )
    set(LINGUIST_FOUND TRUE)
else()
    set(GTA5VIEW_QMFILES
        res/gta5sync_de.qm
        res/gta5sync_en_US.qm
        res/gta5sync_fr.qm
        res/gta5sync_ko.qm
        res/gta5sync_ru.qm
        res/gta5sync_uk.qm
        res/gta5sync_zh_TW.qm
        res/qt${QT_VERSION_MAJOR}/qtbase_en_GB.qm
    )
endif()

option(QCONF_BUILD "System installation intended Qconf build" OFF)
if(QCONF_BUILD)
    list(APPEND GTA5VIEW_DEFINES
        -DGTA5SYNC_QCONF
    )
else()
    list(APPEND GTA5VIEW_RESOURCES
        res/img.qrc
        res/tr_g5p.qrc
        res/qt${QT_VERSION_MAJOR}/tr_qt.qrc
    )
endif()

option(FLATPAK_BUILD "Flatpak modifications and optimisations" OFF)
if(FLATPAK_BUILD)
    list(APPEND GTA5VIEW_DEFINES
        -DGTA5SYNC_FLATPAK
    )
endif()

option(RAGEPHOTO_ABI_WRAPPER "Use libragephoto's ABI wrapper" OFF)
if(RAGEPHOTO_ABI_WRAPPER)
    set(RAGEPHOTO_C_API ON CACHE BOOL "Build libragephoto with C API support" FORCE)
    list(APPEND GTA5VIEW_DEFINES
        -DRAGEPHOTO_USE_ABI_WRAPPER
    )
endif()

option(WITH_BOOST "Use linked Boost library" OFF)
if(WITH_BOOST)
    find_package(Boost REQUIRED
        COMPONENTS json
    )
    list(APPEND GTA5VIEW_LIBS
        Boost::json
    )
    list(APPEND GTA5VIEW_DEFINES
        -DGTA5SYNC_BOOST
    )
    list(APPEND GTA5VIEW_INCLUDEDIR
        ${Boost_INCLUDE_DIRS}
    )
else()
    list(APPEND GTA5VIEW_DEFINES
        -DBOOST_JSON_STANDALONE
    )
    list(APPEND GTA5VIEW_INCLUDEDIR
        src/json/include
    )
endif()

option(WITH_DONATE "Donate menu option and donation dialog" OFF)
if(WITH_DONATE)
    set(DONATE_ADDRESSES "" CACHE STRING "Donation addresses")
    list(APPEND GTA5VIEW_HEADERS
        src/anpro/QrCode.h
    )
    list(APPEND GTA5VIEW_SOURCES
        src/anpro/QrCode.cpp
    )
    list(APPEND GTA5VIEW_DEFINES
        -DGTA5SYNC_DONATE
    )
    list(APPEND GTA5VIEW_RESOURCES
        res/donate.qrc
    )
    if(DONATE_ADDRESSES)
        list(APPEND GTA5VIEW_DEFINES
            "-DGTA5SYNC_DONATE_ADDRESSES=\"${DONATE_ADDRESSES}\""
        )
    endif()
endif()

option(WITH_MOTD "Developer message system directed to users" OFF)
if(WITH_MOTD)
    set(MOTD_WEBURL "" CACHE STRING "Messages WebURL")
    list(APPEND GTA5VIEW_HEADERS
        src/MessageThread.h
    )
    list(APPEND GTA5VIEW_SOURCES
        src/MessageThread.cpp
    )
    list(APPEND GTA5VIEW_DEFINES
        -DGTA5SYNC_MOTD
    )
    if(MOTD_WEBURL)
        list(APPEND GTA5VIEW_DEFINES
            "-DGTA5SYNC_MOTD_WEBURL=\"${MOTD_WEBURL}\""
        )
    endif()
endif()

option(WITH_TELEMETRY "Hardware survey and basic telemetry system" OFF)
if(WITH_TELEMETRY)
    set(TELEMETRY_AUTHID "" CACHE STRING "Telemetry AuthID")
    set(TELEMETRY_AUTHPW "" CACHE STRING "Telemetry AuthPW")
    set(TELEMETRY_PUSHURL "" CACHE STRING "Telemetry PushURL")
    set(TELEMETRY_REGURL "" CACHE STRING "Telemetry RegURL")
    set(TELEMETRY_WEBURL "" CACHE STRING "Telemetry WebURL")
    list(APPEND GTA5VIEW_HEADERS
        src/TelemetryClass.h
        src/tmext/TelemetryClassAuthenticator.h
    )
    list(APPEND GTA5VIEW_SOURCES
        src/TelemetryClass.cpp
        src/tmext/TelemetryClassAuthenticator.cpp
    )
    list(APPEND GTA5VIEW_INCLUDEDIR
        src/tmext
    )
    list(APPEND GTA5VIEW_DEFINES
        -DGTA5SYNC_TELEMETRY
    )
    if(TELEMETRY_AUTHID AND TELEMETRY_AUTHPW AND TELEMETRY_PUSHURL AND TELEMETRY_REGURL)
        list(APPEND GTA5VIEW_DEFINES
            "-DGTA5SYNC_TELEMETRY_AUTHID=\"${TELEMETRY_AUTHID}\""
            "-DGTA5SYNC_TELEMETRY_AUTHPW=\"${TELEMETRY_AUTHPW}\""
            "-DGTA5SYNC_TELEMETRY_PUSHURL=\"${TELEMETRY_PUSHURL}\""
            "-DGTA5SYNC_TELEMETRY_REGURL=\"${TELEMETRY_REGURL}\""
        )
    endif()
    if(TELEMETRY_WEBURL)
        list(APPEND GTA5VIEW_DEFINES
            "-DGTA5SYNC_TELEMETRY_WEBURL=\"${TELEMETRY_WEBURL}\""
        )
    endif()
    if(WIN32)
        list(APPEND GTA5VIEW_LIBS
            d3d9
        )
    endif()
endif()

if(GTA5VIEW_APPVER)
    list(APPEND GTA5VIEW_DEFINES
        "-DGTA5SYNC_APPVER=\"${GTA5VIEW_APPVER}\""
    )
endif()
if(GTA5VIEW_BUILDCODE)
    list(APPEND GTA5VIEW_DEFINES
        "-DGTA5SYNC_BUILDCODE=\"${GTA5VIEW_BUILDCODE}\""
    )
endif()
if(GTA5VIEW_BUILDTYPE)
    list(APPEND GTA5VIEW_DEFINES
        "-DGTA5SYNC_BUILDTYPE=\"${GTA5VIEW_BUILDTYPE}\""
    )
endif()
if(GTA5VIEW_COMMIT)
    list(APPEND GTA5VIEW_DEFINES
        "-DGTA5SYNC_COMMIT=\"${GTA5VIEW_COMMIT}\""
    )
endif()

if(GTA5VIEW_INLANG)
    list(APPEND GTA5VIEW_DEFINES
        "-DGTA5SYNC_INLANG=\"${GTA5VIEW_INLANG}\""
    )
endif()
if(GTA5VIEW_LANG)
    list(APPEND GTA5VIEW_DEFINES
        "-DGTA5SYNC_LANG=\"${GTA5VIEW_LANG}\""
    )
endif()
if(GTA5VIEW_PLUG)
    list(APPEND GTA5VIEW_DEFINES
        "-DGTA5SYNC_PLUG=\"${GTA5VIEW_PLUG}\""
    )
endif()

add_executable(gta5view
    WIN32 MACOSX_BUNDLE
    ${GTA5VIEW_HEADERS}
    ${GTA5VIEW_SOURCES}
    ${GTA5VIEW_FORMS}
    ${GTA5VIEW_RESOURCES}
)

if(LINGUIST_FOUND AND QCONF_BUILD)
    add_custom_target(translations DEPENDS ${GTA5VIEW_QMFILES})
    add_dependencies(gta5view translations)
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16.0")
    target_precompile_headers(gta5view PRIVATE src/config.h)
endif()

if(Qt5Core_VERSION VERSION_GREATER_EQUAL "5.14.0")
    qt5_import_plugins(gta5view INCLUDE Qt5::QSvgPlugin)
endif()

target_compile_definitions(gta5view PRIVATE ${GTA5VIEW_DEFINES})
target_include_directories(gta5view PRIVATE ${GTA5VIEW_INCLUDEDIR} ${RAGEPHOTO_INCLUDE_DIRS})
target_link_libraries(gta5view PRIVATE Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Svg Qt${QT_VERSION_MAJOR}::Widgets ragephoto ${GTA5VIEW_LIBS})

install(TARGETS gta5view DESTINATION bin)
install(FILES res/de.syping.gta5view.desktop DESTINATION share/applications)
install(FILES res/de.syping.gta5view.metainfo.xml DESTINATION share/metainfo)
install(FILES res/de.syping.gta5view.xml DESTINATION share/mime/packages)
install(FILES res/gta5view-16.png DESTINATION share/icons/hicolor/16x16/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-24.png DESTINATION share/icons/hicolor/24x24/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-32.png DESTINATION share/icons/hicolor/32x32/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-48.png DESTINATION share/icons/hicolor/48x48/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-64.png DESTINATION share/icons/hicolor/64x64/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-96.png DESTINATION share/icons/hicolor/96x96/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-128.png DESTINATION share/icons/hicolor/128x128/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-256.png DESTINATION share/icons/hicolor/256x256/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-512.png DESTINATION share/icons/hicolor/512x512/apps RENAME de.syping.gta5view.png)
if(QCONF_BUILD)
    include(res/img.cmake)
    install(FILES ${GTA5VIEW_IMGFILES} DESTINATION share/gta5view/resources)
    install(FILES ${GTA5VIEW_QMFILES} DESTINATION share/gta5view/translations)
endif()
