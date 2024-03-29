cmake_minimum_required(VERSION 3.7)

project(gta5view LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FORCE_QT_VERSION "" CACHE STRING "Force Qt Version")
if(FORCE_QT_VERSION)
    set(QT_VERSION_MAJOR ${FORCE_QT_VERSION})
else()
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
endif()
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network Svg Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools QUIET)

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
    set(MACOSX_BUNDLE_BUNDLE_VERSION 1.10.2)
    set(MACOSX_BUNDLE_ICON_FILE gta5view.icns)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER de.syping.gta5view)
    set_source_files_properties(res/gta5view.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

list(APPEND GTA5VIEW_DEFINES
    -DGTA5SYNC_CMAKE
    -DGTA5SYNC_PROJECT
)

set(GTA5VIEW_SOURCES
    main.cpp
    AboutDialog.cpp
    AppEnv.cpp
    CrewDatabase.cpp
    DatabaseThread.cpp
    ExportDialog.cpp
    ExportThread.cpp
    GlobalString.cpp
    IconLoader.cpp
    ImportDialog.cpp
    JsonEditorDialog.cpp
    MapLocationDialog.cpp
    OptionsDialog.cpp
    PictureDialog.cpp
    PictureExport.cpp
    PictureWidget.cpp
    PlayerListDialog.cpp
    ProfileDatabase.cpp
    ProfileInterface.cpp
    ProfileLoader.cpp
    ProfileWidget.cpp
    RagePhoto.cpp
    SavegameCopy.cpp
    SavegameData.cpp
    SavegameDialog.cpp
    SavegameWidget.cpp
    SidebarGenerator.cpp
    SnapmaticEditor.cpp
    SnapmaticPicture.cpp
    SnapmaticWidget.cpp
    StandardPaths.cpp
    StringParser.cpp
    TranslationClass.cpp
    UserInterface.cpp
    anpro/imagecropper.cpp
    pcg/pcg_basic.c
    uimod/JSHighlighter.cpp
    uimod/UiModLabel.cpp
    uimod/UiModWidget.cpp
)

set(GTA5VIEW_HEADERS
    config.h
    wrapper.h
    AboutDialog.h
    AppEnv.h
    CrewDatabase.h
    DatabaseThread.h
    ExportDialog.h
    ExportThread.h
    GlobalString.h
    IconLoader.h
    ImportDialog.h
    JsonEditorDialog.h
    MapLocationDialog.h
    OptionsDialog.h
    PictureDialog.h
    PictureExport.h
    PictureWidget.h
    PlayerListDialog.h
    ProfileDatabase.h
    ProfileInterface.h
    ProfileLoader.h
    ProfileWidget.h
    RagePhoto.h
    SavegameCopy.h
    SavegameData.h
    SavegameDialog.h
    SavegameWidget.h
    SidebarGenerator.h
    SnapmaticEditor.h
    SnapmaticPicture.h
    SnapmaticWidget.h
    StandardPaths.h
    StringParser.h
    TranslationClass.h
    UserInterface.h
    anpro/imagecropper.h
    pcg/pcg_basic.h
    uimod/JSHighlighter.h
    uimod/UiModLabel.h
    uimod/UiModWidget.h
)

set(GTA5VIEW_INCLUDEDIR
    anpro
    pcg
    uimod
)

set(GTA5VIEW_FORMS
    AboutDialog.ui
    ExportDialog.ui
    ImportDialog.ui
    JsonEditorDialog.ui
    MapLocationDialog.ui
    OptionsDialog.ui
    PictureDialog.ui
    PlayerListDialog.ui
    ProfileInterface.ui
    SavegameDialog.ui
    SavegameWidget.ui
    SnapmaticEditor.ui
    SnapmaticWidget.ui
    UserInterface.ui
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
    res/template.qrc
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

option(WITH_DONATE "Donate menu option and donation dialog" OFF)
if(WITH_DONATE)
    set(DONATE_ADDRESSES "" CACHE STRING "Donation addresses")
    list(APPEND GTA5VIEW_HEADERS
        anpro/QrCode.h
    )
    list(APPEND GTA5VIEW_SOURCES
        anpro/QrCode.cpp
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
        MessageThread.h
    )
    list(APPEND GTA5VIEW_SOURCES
        MessageThread.cpp
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
        TelemetryClass.h
        tmext/TelemetryClassAuthenticator.h
    )
    list(APPEND GTA5VIEW_SOURCES
        TelemetryClass.cpp
        tmext/TelemetryClassAuthenticator.cpp
    )
    list(APPEND GTA5VIEW_INCLUDEDIR
        tmext
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
    target_precompile_headers(gta5view PRIVATE config.h)
endif()

if(Qt5Core_VERSION VERSION_GREATER_EQUAL "5.14.0")
    qt5_import_plugins(gta5view INCLUDE Qt5::QSvgPlugin)
endif()

target_compile_definitions(gta5view PRIVATE ${GTA5VIEW_DEFINES})
target_include_directories(gta5view PRIVATE ${GTA5VIEW_INCLUDEDIR})
target_link_libraries(gta5view PRIVATE Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Svg Qt${QT_VERSION_MAJOR}::Widgets ${GTA5VIEW_LIBS})

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
